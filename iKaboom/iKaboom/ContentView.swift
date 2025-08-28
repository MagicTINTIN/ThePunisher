import SwiftUI
import Foundation
#if os(macOS)
import AppKit
#endif

// declare this in your bridging header instead:
// extern "C" void havefun(int);
@_silgen_name("havefun")
func havefun(_ x: Int32)

struct ContentView: View {
    @State private var isRunning = false
    @State private var output = ""

    var body: some View {
        VStack(spacing: 12) {
            HStack {
                Button(action: runFun) {
                    Text(isRunning ? "Generating fun..." : "Press here to have fun")
                        .foregroundColor(.white)
                        .padding()
                }
                .background(Color.red)
                .cornerRadius(6)
                .disabled(isRunning)
            }
            .padding(.top)

            // Scrollable area to display captured stdout
            TextEditor(text: $output)
                .font(.system(.body, design: .monospaced))
                .frame(minHeight: 200)
                .padding()
                .overlay(RoundedRectangle(cornerRadius: 6).stroke(Color.secondary))
                .padding()
        }
        .padding()
    }

    private func runFun() {
        isRunning = true
        output = ""

        DispatchQueue.global(qos: .userInitiated).async {
            captureStdoutAndRun {
                // call the C/C++ function that prints with printf
                havefun(2)
            } onOutput: { chunk in
                // append output progressively on main thread
                DispatchQueue.main.async {
                    output += chunk
                    // auto-scroll: you can set selection or track it if needed
                }
            } onComplete: { success, error in
                DispatchQueue.main.async {
                    isRunning = false
                    if let err = error {
                        output += "\n\n[Error capturing stdout: \(err.localizedDescription)]"
                    } else if !success {
                        output += "\n\n[Capture finished with unknown state]"
                    }
                }
            }
        }
    }

    /// Redirects stdout to a pipe, runs the `work` closure, streams output into `onOutput`,
    /// then restores stdout and calls `onComplete`.
    private func captureStdoutAndRun(work: () -> Void,
                                     onOutput: @escaping (String) -> Void,
                                     onComplete: @escaping (Bool, Error?) -> Void) {
        // Create a pipe
        var fds: [Int32] = [0, 0]
        if pipe(&fds) != 0 {
            onComplete(false, NSError(domain: "captureStdout", code: 1, userInfo: [NSLocalizedDescriptionKey: "pipe() failed"]))
            return
        }
        let readFD = fds[0]
        let writeFD = fds[1]

        // Save original stdout
        let origStdout = dup(STDOUT_FILENO)
        if origStdout == -1 {
            // cleanup
            close(readFD)
            close(writeFD)
            onComplete(false, NSError(domain: "captureStdout", code: 2, userInfo: [NSLocalizedDescriptionKey: "dup() failed"]))
            return
        }

        // Redirect stdout to write end of the pipe
        if dup2(writeFD, STDOUT_FILENO) == -1 {
            close(readFD); close(writeFD); close(origStdout)
            onComplete(false, NSError(domain: "captureStdout", code: 3, userInfo: [NSLocalizedDescriptionKey: "dup2() failed"]))
            return
        }

        // We'll keep writeFD so we can close it later to signal EOF. Do NOT close writeFD now.
        // Set up a FileHandle for the read side and a readability handler to stream data.
        let readHandle = FileHandle(fileDescriptor: readFD, closeOnDealloc: true)

        // readabilityHandler runs on a run loop. Use it to stream bytes.
        readHandle.readabilityHandler = { fh in
            let data = fh.availableData
            if data.count > 0 {
                let s = String(data: data, encoding: .utf8) ?? String(decoding: data, as: UTF8.self)
                onOutput(s)
            } else {
                // EOF reached
                fh.readabilityHandler = nil
                try? fh.close()
            }
        }

        // Run the printing work synchronously (it will write into stdout -> pipe)
        var workError: Error?
        do {
            work()
        } catch {
            workError = error
        }

        // Ensure libc buffers are flushed
        fflush(stdout)

        // Restore original stdout
        if dup2(origStdout, STDOUT_FILENO) == -1 {
            // attempt cleanup, but still try to close write fd
            close(origStdout)
            close(writeFD)
            onComplete(false, NSError(domain: "captureStdout", code: 4, userInfo: [NSLocalizedDescriptionKey: "dup2 restore failed"]))
            return
        }
        close(origStdout)

        // Close the pipe write end to signal EOF to the reader
        close(writeFD)

        // wait briefly for readabilityHandler to receive EOF and close the read handle.
        // Poll until readabilityHandler is cleared (we can just sleep a tiny bit)
        // Note: readabilityHandler will clear when EOF is reached (it closes the FileHandle).
        // Give it a short timeout.
        let timeout: TimeInterval = 2.0
        let start = Date()
        while Date().timeIntervalSince(start) < timeout {
            // If the read FD is closed, the FileHandle's descriptor is -1; break.
            if readHandle.fileDescriptor == -1 { break }
            usleep(10_000) // 10ms
        }

        // Ensure file handle is closed
        try? readHandle.close()

        // Done
        onComplete(workError == nil, workError)
    }
}
