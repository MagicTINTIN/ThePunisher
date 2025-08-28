import SwiftUI
import UniformTypeIdentifiers
#if os(macOS)
import AppKit
#endif

struct ContentView: View {
    @State private var isRunning: Bool = false

    var body: some View {
        VStack(spacing: 12) {
            Form {
                HStack {
                    Button(action: runFun) {
                        Text(isRunning ? "Generating fun..." : "Press here to have fun")
                            .foregroundColor(.white)
                            .padding()
                    }
                    .background(Color.red)
                    .cornerRadius(2)
                    .padding()
                    .disabled(isRunning)
                }
            }
            .padding()
        }
    }

    private func runFun() {

        isRunning = true
        havefun()
        isRunning = false
    }
}
