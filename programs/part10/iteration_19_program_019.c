/* test_cache_coverage.c - Cover GCC i386 driver cache detection cases */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Prevent compiler from optimizing away our benchmarks */
#define COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Different architecture targets to trigger specific cache descriptor cases */
#ifdef TEST_PENTIUM3
/* Targets cases: 0x0a, 0x0c, 0x0d, 0x0e, 0x21, 0x24 */
__attribute__((target("arch=pentium3")))
#endif
#ifdef TEST_PENTIUM4
/* Targets cases: 0x2c, 0x39-0x3e, 0x41-0x45, 0x49 (non-Xeon-MP) */
__attribute__((target("arch=pentium4")))
#endif
#ifdef TEST_NOCONA
/* Targets cases: 0x49 (non-Xeon-MP), 0x60, 0x66-0x68 */
__attribute__((target("arch=nocona")))
#endif
#ifdef TEST_K8
/* Targets cases: 0x78-0x87 */
__attribute__((target("arch=k8")))
#endif
#ifdef TEST_CORE2
/* Targets cases: 0x48, 0x4e */
__attribute__((target("arch=core2")))
#endif
static void benchmark_cache(int iterations, int buffer_size_kb) {
    volatile int result = 0;
    int i, j;
    
    /* Allocate buffer larger than expected cache */
    int elements = (buffer_size_kb * 1024) / sizeof(int);
    int *buffer = (int*)malloc(elements * sizeof(int));
    
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    
    /* Initialize with pseudo-random pattern */
    srand(42);
    for (i = 0; i < elements; i++) {
        buffer[i] = rand();
    }
    
    /* Cache-thrashing benchmark */
    for (j = 0; j < iterations; j++) {
        int stride = 17; /* Prime number to avoid simple patterns */
        int index = 0;
        
        for (i = 0; i < elements * 4; i++) {
            index = (index + stride) % elements;
            buffer[index] = buffer[index] * 3 + 1;
            COMPILER_BARRIER();
        }
    }
    
    /* Compute final result to prevent optimization */
    for (i = 0; i < elements; i += 64) { /* Cache line sized steps */
        result ^= buffer[i];
    }
    
    /* Use result to prevent dead code elimination */
    printf("Benchmark result: %d\n", result);
    
    free(buffer);
}

/* Multi-versioned function using target_clones */
#ifdef USE_MULTIVERSIONING
__attribute__((target_clones("default,arch=pentium3,arch=pentium4,arch=nocona,arch=k8,arch=core2")))
#endif
static void multiarch_benchmark() {
    /* Different buffer sizes to stress different cache levels */
    benchmark_cache(10, 8);   /* L1 cache sizes */
    benchmark_cache(5, 128);  /* Small L2 */
    benchmark_cache(3, 1024); /* Large L2 */
    benchmark_cache(2, 4096); /* Very large L2/L3 */
}

int main() {
    clock_t start, end;
    double cpu_time_used;
    
    printf("Cache Detection Coverage Test\n");
    printf("=============================\n");
    
    /* Run benchmarks with different optimization hints */
    
    /* Test 1: Generic x86 - may use runtime detection */
    printf("\n[Test 1: Generic x86]\n");
    start = clock();
    benchmark_cache(20, 512);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Time: %f seconds\n", cpu_time_used);
    
    /* Test 2: Multi-arch if supported */
#ifdef USE_MULTIVERSIONING
    printf("\n[Test 2: Multi-architecture]\n");
    start = clock();
    multiarch_benchmark();
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Time: %f seconds\n", cpu_time_used);
#endif
    
    /* Test 3: Matrix multiplication - cache intensive */
    printf("\n[Test 3: Matrix Multiplication]\n");
    {
        const int N = 256;
        volatile int sum = 0;
        int *A = (int*)malloc(N * N * sizeof(int));
        int *B = (int*)malloc(N * N * sizeof(int));
        int *C = (int*)malloc(N * N * sizeof(int));
        
        if (A && B && C) {
            /* Initialize matrices */
            for (int i = 0; i < N * N; i++) {
                A[i] = i % 100;
                B[i] = (i + 1) % 100;
            }
            
            /* Perform multiplication - stresses cache hierarchy */
            for (int i = 0; i < N; i++) {
                for (int k = 0; k < N; k++) {
                    for (int j = 0; j < N; j++) {
                        C[i * N + j] += A[i * N + k] * B[k * N + j];
                    }
                }
                COMPILER_BARRIER();
            }
            
            /* Use result */
            for (int i = 0; i < N * N; i += 128) {
                sum ^= C[i];
            }
            printf("Matrix result checksum: %d\n", sum);
        }
        
        free(A); free(B); free(C);
    }
    
    /* Test 4: Linked list traversal - pointer chasing */
    printf("\n[Test 4: Linked List Traversal]\n");
    {
        const int LIST_SIZE = 100000;
        struct Node {
            int value;
            struct Node* next;
        };
        
        struct Node* nodes = (struct Node*)malloc(LIST_SIZE * sizeof(struct Node));
        if (nodes) {
            /* Create linked list with random permutation */
            int* permutation = (int*)malloc(LIST_SIZE * sizeof(int));
            for (int i = 0; i < LIST_SIZE; i++) permutation[i] = i;
            
            /* Fisher-Yates shuffle */
            for (int i = LIST_SIZE - 1; i > 0; i--) {
                int j = rand() % (i + 1);
                int temp = permutation[i];
                permutation[i] = permutation[j];
                permutation[j] = temp;
            }
            
            /* Link nodes according to permutation */
            for (int i = 0; i < LIST_SIZE - 1; i++) {
                nodes[permutation[i]].value = permutation[i];
                nodes[permutation[i]].next = &nodes[permutation[i + 1]];
            }
            nodes[permutation[LIST_SIZE - 1]].value = permutation[LIST_SIZE - 1];
            nodes[permutation[LIST_SIZE - 1]].next = NULL;
            
            /* Traverse list many times */
            volatile int list_sum = 0;
            struct Node* current = &nodes[permutation[0]];
            
            start = clock();
            for (int iter = 0; iter < 100; iter++) {
                current = &nodes[permutation[0]];
                while (current) {
                    list_sum ^= current->value;
                    current = current->next;
                    COMPILER_BARRIER();
                }
            }
            end = clock();
            
            printf("List traversal checksum: %d\n", list_sum);
            cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
            printf("Time: %f seconds\n", cpu_time_used);
            
            free(permutation);
            free(nodes);
        }
    }
    
    return 0;
}
