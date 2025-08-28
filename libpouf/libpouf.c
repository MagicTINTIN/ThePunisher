#include "libpouf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include <errno.h>
#include <mach/mach.h>
#include <mach/vm_map.h>

#define CHUNK_SIZE (10 * 1024 * 1024) // 10MB chunks
#define THREAD_LIMIT 1000

// Global variables
int running = 1;
int thread_count = 0;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

// Memory exhaustion with memory mapping tricks
void *memory_bypass_thread(void *arg) {
    size_t allocated = 0;
    void **allocations = NULL;
    size_t count = 0;
    
    printf("Starting memory bypass technique\n");
    
    while (running) {
        // Try different memory allocation methods to bypass limits
        void *ptr = NULL;
        
        // Method 1: Standard mmap
        ptr = mmap(NULL, CHUNK_SIZE, PROT_READ | PROT_WRITE, 
                  MAP_ANON | MAP_PRIVATE, -1, 0);
        
        if (ptr == MAP_FAILED) {
            printf("mmap failed, trying vm_allocate...\n");
            
            // Method 2: Mach VM allocation (lower level)
            kern_return_t ret = vm_allocate(mach_task_self(), (vm_address_t*)&ptr, 
                                           CHUNK_SIZE, VM_FLAGS_ANYWHERE);
            if (ret != KERN_SUCCESS) {
                printf("vm_allocate also failed after %.1f MB\n", 
                      (double)allocated / (1024 * 1024));
                break;
            }
        }
        
        // Touch memory in a pattern that's harder to optimize away
        for (size_t i = 0; i < CHUNK_SIZE; i += 4096) {
            *((char*)ptr + i) = (i + count) % 256;
        }
        
        // Store the pointer
        allocations = realloc(allocations, (count + 1) * sizeof(void *));
        allocations[count++] = ptr;
        allocated += CHUNK_SIZE;
        
        if (count % 10 == 0) {
            printf("Allocated %.1f MB so far\n", (double)allocated / (1024 * 1024));
        }
        
        // Occasionally free some memory to avoid immediate termination
        if (count > 50 && count % 25 == 0) {
            for (int i = 0; i < 5; i++) {
                size_t idx = rand() % count;
                if (allocations[idx]) {
                    munmap(allocations[idx], CHUNK_SIZE);
                    allocations[idx] = NULL;
                    allocated -= CHUNK_SIZE;
                }
            }
            
            // Compact the array
            size_t new_count = 0;
            for (size_t i = 0; i < count; i++) {
                if (allocations[i]) {
                    allocations[new_count++] = allocations[i];
                }
            }
            count = new_count;
            printf("Freed some memory, now at %.1f MB\n", (double)allocated / (1024 * 1024));
        }
    }
    
    // Cleanup
    for (size_t i = 0; i < count; i++) {
        if (allocations[i]) {
            munmap(allocations[i], CHUNK_SIZE);
        }
    }
    free(allocations);
    
    return NULL;
}

void *thread_bomb_func(void *arg) {
    pthread_mutex_lock(&count_mutex);
    thread_count++;
    int current_count = thread_count;
    pthread_mutex_unlock(&count_mutex);
    
    printf("Thread %d started\n", current_count);
    
    if (current_count < THREAD_LIMIT && running) {
        pthread_t thread;
        if (pthread_create(&thread, NULL, thread_bomb_func, NULL) == 0) {
            pthread_detach(thread);
        }
    }
    
    // More intensive busy work
    while (running) {
        // Matrix multiplication simulation
        volatile double matrix[10][10];
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                for (int k = 0; k < 10; k++) {
                    matrix[i][j] = matrix[i][k] * matrix[k][j];
                }
            }
        }
    }
    
    pthread_mutex_lock(&count_mutex);
    thread_count--;
    pthread_mutex_unlock(&count_mutex);
    
    return NULL;
}

void gpu_stress_test() {
    printf("Starting GPU stress test\n");
    
    // This would ideally use Metal API for GPU computation
    // For now, we'll simulate with CPU-based matrix operations
    // that are compute-intensive
    
    const int size = 1000;
    double *matrixA = malloc(size * size * sizeof(double));
    double *matrixB = malloc(size * size * sizeof(double));
    double *result = malloc(size * size * sizeof(double));
    
    if (!matrixA || !matrixB || !result) {
        printf("Failed to allocate matrices\n");
        return;
    }
    
    // Initialize matrices
    for (int i = 0; i < size * size; i++) {
        matrixA[i] = (double)rand() / RAND_MAX;
        matrixB[i] = (double)rand() / RAND_MAX;
    }
    
    int iteration = 0;
    while (running) {
        // Simulate matrix multiplication (compute-intensive)
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                double sum = 0.0;
                for (int k = 0; k < size; k++) {
                    sum += matrixA[i * size + k] * matrixB[k * size + j];
                }
                result[i * size + j] = sum;
            }
        }
        
        iteration++;
        if (iteration % 10 == 0) {
            printf("Completed %d GPU-like iterations\n", iteration);
        }
    }
    
    free(matrixA);
    free(matrixB);
    free(result);
}

void stress_test(int mode) {
    printf("Starting stress test (mode: %d)\n", mode);
    
    if (mode == 0) {
        // Memory bypass technique
        memory_bypass_thread(NULL);
    } 
    else if (mode == 3) {
        // Combined attack
        pthread_t mem_thread;
        pthread_create(&mem_thread, NULL, memory_bypass_thread, NULL);
        
        pthread_t gpu_thread;
        pthread_create(&gpu_thread, NULL, (void*(*)(void*))gpu_stress_test, NULL);
        
        thread_bomb_func(NULL);
        
        pthread_join(mem_thread, NULL);
        pthread_join(gpu_thread, NULL);
    }
    
    printf("Stress test completed\n");
}