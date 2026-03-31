/* haifa-sched-trigger.c
 * Designed to trigger GCC's Haifa scheduler context allocation and cleanup
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -finline-functions -funswitch-loops -frandom-seed=1 haifa-sched-trigger.c -o haifa-test
 * For FDO: gcc -O2 -fprofile-generate haifa-sched-trigger.c -o haifa-prof
 *           ./haifa-prof
 *           gcc -O3 -fschedule-insns2 -fprofile-use haifa-sched-trigger.c -o haifa-final
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

#define HOT __attribute__((hot))
#define ALWAYS_INLINE __attribute__((always_inline))
#define NOINLINE __attribute__((noinline))

/* Helper functions designed for inlining */
ALWAYS_INLINE static inline uint32_t mix_bits(uint32_t x) {
    x ^= x >> 16;
    x *= 0x85ebca6b;
    x ^= x >> 13;
    x *= 0xc2b2ae35;
    x ^= x >> 16;
    return x;
}

ALWAYS_INLINE static inline int complex_condition(int a, int b, int c) {
    int t = (a & b) | (c & ~a);
    t = t ^ (t >> 3);
    t = t * 1103515245 + 12345;
    return t & 1;
}

/* Function with irreducible control flow using computed goto */
NOINLINE HOT uint64_t irreducible_cfg(int *arr, int n) {
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5 };
    
    uint64_t sum = 0;
    int i = 0;
    int state = 0;
    
    /* Create complex control flow with computed goto */
    void *next_label = labels[state];
    goto *next_label;
    
L0:
    for (; i < n; i++) {
        /* Mix operations to create dependencies */
        arr[i] = mix_bits(arr[i]) + i;
        sum += arr[i];
        
        /* Switch state based on complex condition */
        state = complex_condition(arr[i], i, sum) * 3;
        if (state >= 6) state = 0;
        next_label = labels[state];
        goto *next_label;
        
L1:
        /* Different computation path */
        arr[i] ^= 0xAAAAAAAA;
        sum -= arr[i] * 2;
        state = (arr[i] & 3) + 1;
        next_label = labels[state];
        goto *next_label;
        
L2:
        /* Memory intensive path */
        if (i > 0) {
            arr[i] += arr[i-1];
            asm volatile("" : : "r"(arr[i]) : "memory");
        }
        sum ^= arr[i];
        state = (sum & 5);
        next_label = labels[state];
        goto *next_label;
        
L3:
        /* Another path with arithmetic */
        arr[i] = arr[i] * 1103515245 + 12345;
        sum += arr[i] >> 4;
        state = 4;
        next_label = labels[state];
        goto *next_label;
        
L4:
        /* Path with barrier */
        asm volatile("mfence" : : : "memory");
        arr[i] = ~arr[i];
        sum -= arr[i];
        state = 5;
        next_label = labels[state];
        goto *next_label;
        
L5:
        /* Final path before loop continues */
        arr[i] = (arr[i] << 3) | (arr[i] >> 29);
        sum ^= (sum << 1) | (sum >> 63);
        state = 0;
        next_label = labels[state];
        if (++i < n) goto *next_label;
    }
    
    return sum;
}

/* Function with deeply nested conditionals and loops */
NOINLINE HOT int nested_control_flow(int *data, int size) {
    int result = 0;
    int i, j, k;
    
    for (i = 0; i < size; i++) {
        /* Outer loop with switch */
        switch (i & 7) {
            case 0:
                for (j = 0; j < 8; j++) {
                    data[i] += mix_bits(j);
                    if (complex_condition(data[i], j, result)) {
                        for (k = 0; k < 4; k++) {
                            data[i] ^= (k << (j & 3));
                            asm volatile("" : : "r"(data[i]) : "memory");
                        }
                    } else {
                        do {
                            data[i] = (data[i] * 3 + 1) >> 1;
                            result ^= data[i];
                        } while (data[i] & 1);
                    }
                }
                break;
                
            case 1:
            case 3:
                while (data[i] > 0) {
                    data[i] = (data[i] & 1) ? (data[i] * 3 + 1) : (data[i] >> 1);
                    result += data[i];
                    if (data[i] & 2) {
                        data[i] ^= result;
                        continue;
                    }
                    /* Nested if-else chain */
                    if (result > 1000) {
                        if (data[i] < 500) {
                            data[i] *= 2;
                        } else if (data[i] < 1000) {
                            data[i] /= 2;
                        } else {
                            data[i] ^= 0xFFFF;
                        }
                    } else {
                        data[i] |= 0x80000000;
                    }
                }
                break;
                
            case 2:
            case 4:
                for (j = i; j > 0 && j < size; j += (i & 3) - 1) {
                    int temp = data[j];
                    data[j] = mix_bits(data[j] ^ data[i]);
                    result += temp - data[j];
                    /* Inline assembly to create scheduling barrier */
                    asm volatile("" : : "r"(data[j]), "r"(result) : "memory");
                }
                break;
                
            default:
                /* Complex arithmetic with dependencies */
                int x = data[i];
                for (j = 0; j < 16; j++) {
                    x = (x << 1) | ((x >> 31) & 1);
                    x ^= 0xEDB88320;
                    result ^= x;
                    x = x * 0xCC9E2D51;
                }
                data[i] = x;
                break;
        }
    }
    
    return result;
}

/* Vectorization candidate with OpenMP */
NOINLINE HOT void vectorizable_loop(int *a, int *b, int *c, int n) {
    int i;
    
    /* Candidate for auto-vectorization */
    #pragma omp simd
    for (i = 0; i < n; i++) {
        a[i] = b[i] * 3 + c[i] * 7;
    }
    
    /* Another loop with carried dependency */
    for (i = 1; i < n; i++) {
        a[i] += a[i-1] * 2;
        /* Memory clobber to force scheduling constraints */
        asm volatile("" : : "r"(a[i]) : "memory");
    }
    
    /* Parallel loop */
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        int j;
        for (j = 0; j < 8; j++) {
            c[i] ^= (a[i] << j) | (b[i] >> j);
        }
        c[i] = mix_bits(c[i]);
    }
}

/* Function with tight inner loop and carried dependencies */
NOINLINE HOT uint64_t tight_inner_loop(int *arr, int n) {
    uint64_t acc = 0x123456789ABCDEF0ULL;
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Outer loop */
        int val = arr[i];
        
        /* Tight inner loop with carried dependency */
        for (j = 0; j < 32; j++) {
            val = (val & 1) ? ((val >> 1) ^ 0xEDB88320) : (val >> 1);
            acc = (acc << 1) | (val & 1);
            /* Inline asm to prevent optimization */
            asm volatile("" : "+r"(val), "+r"(acc) : : "memory");
        }
        
        arr[i] = val;
        
        /* Another inner loop with different pattern */
        for (j = 0; j < 8; j++) {
            acc ^= (uint64_t)arr[i] << (j * 8);
            acc = (acc >> 1) | (acc << 63);
        }
    }
    
    return acc;
}

/* Main orchestrator */
int main() {
    const int sizes[] = {1024, 2048, 4096, 8192};
    const int num_sizes = sizeof(sizes)/sizeof(sizes[0]);
    uint64_t total_checksum = 0;
    int iter;
    
    /* Warm-up phase */
    printf("Starting warm-up phase...\n");
    for (iter = 0; iter < 3; iter++) {
        for (int s = 0; s < num_sizes; s++) {
            int size = sizes[s];
            int *arr1 = malloc(size * sizeof(int));
            int *arr2 = malloc(size * sizeof(int));
            int *arr3 = malloc(size * sizeof(int));
            
            if (!arr1 || !arr2 || !arr3) {
                fprintf(stderr, "Memory allocation failed\n");
                return 1;
            }
            
            /* Initialize with pseudo-random data */
            for (int i = 0; i < size; i++) {
                arr1[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
                arr2[i] = mix_bits(i);
                arr3[i] = arr1[i] ^ arr2[i];
            }
            
            /* Call different complex functions */
            total_checksum ^= irreducible_cfg(arr1, size);
            total_checksum += nested_control_flow(arr2, size);
            vectorizable_loop(arr1, arr2, arr3, size);
            total_checksum ^= tight_inner_loop(arr3, size);
            
            free(arr1);
            free(arr2);
            free(arr3);
        }
    }
    
    /* Main timed phase */
    printf("Starting main computation phase...\n");
    clock_t start = clock();
    
    for (iter = 0; iter < 10; iter++) {
        for (int s = 0; s < num_sizes; s++) {
            int size = sizes[s];
            int *arr1 = malloc(size * sizeof(int));
            int *arr2 = malloc(size * sizeof(int));
            int *arr3 = malloc(size * sizeof(int));
            
            /* Different initialization pattern */
            for (int i = 0; i < size; i++) {
                arr1[i] = (i * iter + s) ^ 0x55555555;
                arr2[i] = mix_bits(arr1[i]);
                arr3[i] = arr1[i] + arr2[i];
            }
            
            /* Mix of function calls with different characteristics */
            switch (iter & 3) {
                case 0:
                    total_checksum += irreducible_cfg(arr1, size);
                    total_checksum ^= nested_control_flow(arr2, size/2);
                    break;
                case 1:
                    vectorizable_loop(arr1, arr2, arr3, size);
                    total_checksum += tight_inner_loop(arr1, size);
                    break;
                case 2:
                    total_checksum ^= irreducible_cfg(arr2, size);
                    total_checksum += nested_control_flow(arr3, size);
                    break;
                case 3:
                    vectorizable_loop(arr2, arr3, arr1, size);
                    total_checksum ^= tight_inner_loop(arr3, size);
                    break;
            }
            
            free(arr1);
            free(arr2);
            free(arr3);
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Final checksum: 0x%016llX\n", (unsigned long long)total_checksum);
    printf("Computation time: %.3f seconds\n", elapsed);
    printf("Test completed successfully.\n");
    
    return 0;
}
