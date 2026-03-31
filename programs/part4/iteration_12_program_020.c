/* haifa_scheduler_test.c
 * Designed to trigger free_sched_context in GCC's Haifa scheduler
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -finline-functions -funswitch-loops -fprofile-generate -o test_gen haifa_scheduler_test.c
 * Run: ./test_gen
 * Recompile with: gcc -O3 -fschedule-insns -fschedule-insns2 -finline-functions -funswitch-loops -fprofile-use -o test_use haifa_scheduler_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100000
#define WARMUP_ITERATIONS 1000

/* Helper functions marked for inlining */
__attribute__((always_inline)) static inline int compute_hash(int x, int y) {
    /* Complex bitwise operations to create ILP opportunities */
    int h = x ^ y;
    h = (h >> 16) ^ (h << 16);
    h = h * 0x5bd1e995;
    h = h ^ (h >> 15);
    return h;
}

__attribute__((always_inline)) static inline int scramble_bits(int val) {
    /* Multiple independent operations for scheduler */
    int a = val * 0xcc9e2d51;
    int b = (val >> 17) | (val << 15);
    int c = val ^ 0x85ebca6b;
    /* Memory barrier to force ordering */
    asm volatile("" ::: "memory");
    return (a + b) ^ c;
}

__attribute__((hot)) void complex_control_flow(int *data, int size) {
    /* Irreducible control flow with computed goto */
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5 };
    
    int sum = 0;
    int i = 0;
    
    /* Nested loops with switch inside */
    for (int outer = 0; outer < 10; outer++) {
        int state = outer % 6;
        goto *labels[state];
        
        L0:
            while (i < size) {
                data[i] = compute_hash(data[i], i);
                i++;
                if (i % 7 == 0) goto *labels[1];
                if (i % 13 == 0) goto *labels[3];
            }
            continue;
        
        L1:
            for (int j = 0; j < size; j += 2) {
                data[j] = scramble_bits(data[j]);
                /* Memory operations with dependencies */
                if (j > 0) data[j] += data[j-1];
            }
            continue;
        
        L2:
            do {
                data[i % size] ^= 0xdeadbeef;
                i = (i * 1103515245 + 12345) & 0x7fffffff;
            } while (i % 100 != 0);
            continue;
        
        L3:
            /* Deep if-else chain */
            for (int k = 0; k < size; k++) {
                if (data[k] < 0) {
                    data[k] = -data[k];
                } else if (data[k] < 100) {
                    data[k] = compute_hash(data[k], k);
                } else if (data[k] < 1000) {
                    data[k] = scramble_bits(data[k]);
                } else if (data[k] < 10000) {
                    data[k] = data[k] >> 4;
                } else {
                    data[k] = data[k] & 0xffff;
                }
            }
            continue;
        
        L4:
            /* Switch with multiple cases */
            for (int m = 0; m < size; m++) {
                switch (data[m] % 8) {
                    case 0: data[m] += m; break;
                    case 1: data[m] -= m; break;
                    case 2: data[m] ^= m; break;
                    case 3: data[m] |= m; break;
                    case 4: data[m] &= m; break;
                    case 5: data[m] = compute_hash(data[m], m); break;
                    case 6: data[m] = scramble_bits(data[m]); break;
                    case 7: data[m] *= m; break;
                }
            }
            continue;
        
        L5:
            /* Mixed operations with asm barriers */
            for (int n = 0; n < size; n++) {
                int temp = data[n];
                asm volatile("" : "+r"(temp) : : "memory");
                temp = temp * 0x5bd1e995;
                asm volatile("" ::: "memory");
                temp = (temp >> 17) | (temp << 15);
                data[n] = temp ^ n;
            }
            continue;
    }
}

__attribute__((hot)) void vectorizable_loop(int *a, int *b, int *c, int size) {
    /* Loop designed for auto-vectorization */
    #pragma omp simd
    for (int i = 0; i < size; i++) {
        a[i] = b[i] + c[i];
        a[i] = a[i] * 3 - b[i];
        a[i] = compute_hash(a[i], i);
    }
    
    /* Second loop with carried dependency */
    for (int i = 1; i < size; i++) {
        a[i] += a[i-1] * 0x9e3779b9;
    }
}

__attribute__((hot)) void nested_loop_pattern(int *data, int size) {
    /* Multiple nested loops with varying bounds */
    for (int i = 0; i < size / 4; i++) {
        int acc = 0;
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 4; k++) {
                acc = compute_hash(acc, data[i * 4 + k]);
                acc = scramble_bits(acc);
            }
            /* Conditional inside inner loop */
            if (acc % 2 == 0) {
                data[i] ^= acc;
            } else {
                data[i] += acc;
            }
        }
    }
}

__attribute__((hot)) void parallel_region(int *data, int size) {
    /* OpenMP parallel region to trigger different scheduling */
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < size; i++) {
        int val = data[i];
        /* Complex operation chain */
        val = val * 0xcc9e2d51;
        val = (val >> 16) ^ val;
        val = compute_hash(val, i);
        val = scramble_bits(val);
        data[i] = val;
    }
}

int main() {
    /* Allocate and initialize data */
    int *data1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data3 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand();
        data2[i] = rand();
        data3[i] = rand();
    }
    
    clock_t start, end;
    double cpu_time_used;
    
    printf("Starting scheduler stress test...\n");
    
    /* Warm-up phase */
    printf("Warm-up phase...\n");
    for (int iter = 0; iter < WARMUP_ITERATIONS; iter++) {
        complex_control_flow(data1, ARRAY_SIZE / 2);
    }
    
    /* Main test phase */
    printf("Main test phase...\n");
    start = clock();
    
    long long total_checksum = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Alternate between different patterns */
        switch (iter % 5) {
            case 0:
                complex_control_flow(data1, ARRAY_SIZE);
                break;
            case 1:
                vectorizable_loop(data2, data1, data3, ARRAY_SIZE);
                break;
            case 2:
                nested_loop_pattern(data3, ARRAY_SIZE);
                break;
            case 3:
                parallel_region(data1, ARRAY_SIZE);
                break;
            case 4:
                /* Mix all patterns */
                complex_control_flow(data2, ARRAY_SIZE / 2);
                vectorizable_loop(data3, data2, data1, ARRAY_SIZE);
                nested_loop_pattern(data1, ARRAY_SIZE / 4);
                break;
        }
        
        /* Compute checksum for verification */
        if (iter % 100 == 0) {
            for (int i = 0; i < ARRAY_SIZE; i += 16) {
                total_checksum += data1[i] + data2[i] + data3[i];
            }
        }
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    /* Final checksum computation */
    long long final_checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_checksum += data1[i] ^ data2[i] ^ data3[i];
    }
    
    printf("Test completed in %.2f seconds\n", cpu_time_used);
    printf("Intermediate checksum: %lld\n", total_checksum);
    printf("Final checksum: %lld\n", final_checksum);
    printf("Data1[0]=%d, Data2[0]=%d, Data3[0]=%d\n", data1[0], data2[0], data3[0]);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    
    return 0;
}
