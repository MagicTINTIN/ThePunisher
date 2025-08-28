import SwiftUI
import Foundation
import Metal
import Accelerate

// C function declarations
@_silgen_name("stress_test")
func stress_test(_ mode: Int32)

@_silgen_name("gpu_stress_test")
func gpu_stress_test()

struct ContentView: View {
    @State private var isRunning = false
    @State private var output = ""
    @State private var selectedMode = 0
    
    var body: some View {
        VStack(spacing: 16) {
            Text("Advanced iPad Stress Test")
                .font(.title)
                .padding()
            
            Picker("Test Mode", selection: $selectedMode) {
                Text("Memory exhaust").tag(0)
                Text("GPU Stress").tag(1)
                Text("Core Animation").tag(2)
                Text("Combined gpu/memory").tag(3)
                Text("PAF").tag(4)
            }
            .pickerStyle(SegmentedPickerStyle())
            .padding(.horizontal)
            .disabled(isRunning)
            
            Button(action: {
                runStressTest(mode: Int32(selectedMode))
            }) {
                Text(isRunning ? "Testing in progress..." : "Start Stress Test")
                    .foregroundColor(.white)
                    .padding()
                    .frame(maxWidth: .infinity)
            }
            .background(isRunning ? Color.gray : Color.red)
            .cornerRadius(10)
            .padding(.horizontal)
            .disabled(isRunning)
            
            // Output display
            ScrollView {
                Text(output)
                    .font(.system(size: 12, design: .monospaced))
                    .frame(maxWidth: .infinity, alignment: .leading)
                    .padding()
            }
            .frame(minHeight: 200)
            .background(Color.black)
            .foregroundColor(.green)
            .cornerRadius(8)
            .padding(.horizontal)
            
            Text("Warning: this is fun!")
                .font(.caption)
                .foregroundColor(.red)
                .padding()
        }
        .padding()
    }
    
    private func runStressTest(mode: Int32) {
        isRunning = true
        output = ""
        
        DispatchQueue.global(qos: .userInitiated).async {
            if mode == 1 {
                // GPU stress test
                gpu_stress_test()
            } else if mode == 2 {
                // Core Animation stress (SwiftUI implementation)
                self.coreAnimationStressTest()
            } else {
                // Regular stress test
                DispatchQueue.global(qos: .userInitiated).async {
                    while mode == 4
                    {
                        DispatchQueue.global(qos: .userInitiated).async {
                            self.coreAnimationStressTest()
                        }
                        usleep(5_000_000)
                    }
                }
                
                DispatchQueue.global(qos: .userInitiated).async {
                    if mode == 4 {
                        for _ in 0..<3 {
                            DispatchQueue.global(qos: .userInitiated).async {
                                stress_test(3)
                            }
                        }
                    }
                }

                DispatchQueue.global(qos: .userInitiated).async {
                    if mode == 4 {
                        stress_test(3)
                    }
                    else {
                        stress_test(mode)
                    }
                }
            }
            
            DispatchQueue.main.async {
                isRunning = false
                output += "\nTest completed without crash (unexpected)"
            }
        }
    }
    
    private func coreAnimationStressTest() {
        // Create a massive number of layers/views to stress Core Animation
        DispatchQueue.main.async {
            for i in 0..<15000 {
                let view = UIView(frame: CGRect(x: 0, y: 0, width: 10, height: 10))
                view.backgroundColor = UIColor(red: CGFloat.random(in: 0...1),
                                              green: CGFloat.random(in: 0...1),
                                              blue: CGFloat.random(in: 0...1),
                                              alpha: 1)
                view.layer.cornerRadius = 5
                view.transform = CGAffineTransform(rotationAngle: CGFloat.random(in: 0...CGFloat.pi))
                
                // Add to main window
                if let window = UIApplication.shared.windows.first {
                    window.addSubview(view)
                }
                
                // Animate
                UIView.animate(withDuration: 0.5, delay: 0, options: [.repeat, .autoreverse]) {
                    view.frame.origin = CGPoint(x: CGFloat.random(in: 0...1000),
                                               y: CGFloat.random(in: 0...1000))
                    view.transform = view.transform.rotated(by: CGFloat.random(in: 0...CGFloat.pi))
                }
                
                if i % 100 == 0 {
                    self.output += "\nCreated \(i) animated views"
                }
            }
        }
    }
}
