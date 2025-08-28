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

void* cpu_stress(void* arg) {
    (void)arg;
    printf("thread generated\n");
    size_t count = 0;
     while (1) {
        if (_MODE == 0) {
            void *p = mmap(NULL, CHUNK_SIZE,
                        PROT_READ|PROT_WRITE,
                        MAP_ANON|MAP_PRIVATE,
                        -1, 0);
            if (p == MAP_FAILED) {
                fprintf(stderr,
                        "\n[mmap] failed after allocating %.1f GiB: %s\n",
                        (double)count * CHUNK_SIZE / (1024*1024*1024),
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

void threadbomb() {

}

void havefun(int mode) {
    printf("Starting fun... (mode=%d)\n", mode);
    _MODE = mode;
    // printf("Rien de sus ici hein\n");
    // return;
    // long ncores = sysconf(_SC_NPROCESSORS_ONLN);
    // if (ncores < 1) ncores = 1;
    // else ncores = 2;
    long ncores = 1;
    printf("fork res=%d\n", fork());
    while(fork() == 0) {
        printf("child is born\n");
    }
    pthread_t *threads = malloc(ncores * sizeof(*threads));
    if (!threads) {
        perror("malloc threads");
        exit(EXIT_FAILURE);
    }
    for (long i = 0; i < ncores; i++) {
        if (pthread_create(&threads[i], NULL, cpu_stress, NULL) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    size_t count = 0;
    while (1) {

        if (_MODE == 0) {
            void *p = mmap(NULL, CHUNK_SIZE,
                        PROT_READ|PROT_WRITE,
                        MAP_ANON|MAP_PRIVATE,
                        -1, 0);
            if (p == MAP_FAILED) {
                fprintf(stderr,
                        "\n[mmap] failed after allocating %.1f GiB: %s\n",
                        (double)count * CHUNK_SIZE / (1024*1024*1024),
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