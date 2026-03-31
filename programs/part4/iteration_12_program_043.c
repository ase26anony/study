/* Target: haifa-sched.cc free_sched_context cleanup block */
/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -finline-functions -funswitch-loops -frandom-seed=1 -fprofile-generate -o scheduler_test scheduler_test.c */
/* After running: gcc -O3 -fschedule-insns2 -fprofile-use -finline-functions -funswitch-loops -o scheduler_test_opt scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define HOT __attribute__((hot))
#define ALWAYS_INLINE __attribute__((always_inline))
#define NOINLINE __attribute__((noinline))

/* Helper functions for inlining */
ALWAYS_INLINE static unsigned int hash_mix(unsigned int a, unsigned int b, unsigned int c) {
    a -= b; a -= c; a ^= (c >> 13);
    b -= c; b -= a; b ^= (a << 8);
    c -= a; c -= b; c ^= (b >> 13);
    a -= b; a -= c; a ^= (c >> 12);
    b -= c; b -= a; b ^= (a << 16);
    c -= a; c -= b; c ^= (b >> 5);
    a -= b; a -= c; a ^= (c >> 3);
    b -= c; b -= a; b ^= (a << 10);
    c -= a; c -= b; c ^= (b >> 15);
    return c;
}

ALWAYS_INLINE static int compute_branch(int x, int y) {
    /* Complex branching pattern to stress scheduler */
    if (x > y) {
        return (x * y) ^ (x - y);
    } else if (x < y) {
        return (x + y) | (x << 3);
    } else {
        return (x & y) + (x | y) - (x ^ y);
    }
}

/* Function with irreducible control flow using computed goto */
NOINLINE HOT static unsigned int irreducible_cfg(int n, int *arr) {
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5 };
    unsigned int sum = 0;
    int i = 0;
    
    L0:
    if (i >= n) goto END;
    sum += arr[i] * 3;
    i++;
    goto *labels[(arr[i-1] * 17) % 6];
    
    L1:
    sum ^= (arr[i] << 2);
    i += 2;
    goto *labels[(sum * 13) % 6];
    
    L2:
    sum = (sum >> 1) | (sum << 31);
    i++;
    goto *labels[(arr[i] * 19) % 6];
    
    L3:
    sum += compute_branch(arr[i], sum & 0xFF);
    i += (sum & 1) + 1;
    goto *labels[(i * 23) % 6];
    
    L4:
    sum = hash_mix(sum, arr[i], i);
    i++;
    goto *labels[(sum * 29) % 6];
    
    L5:
    sum -= arr[i] * 7;
    i += 3;
    goto *labels[(arr[i-1] * 31) % 6];
    
    END:
    return sum;
}

/* Function with nested loops and complex dependencies */
NOINLINE HOT static long process_matrix(int size, int *matrix) {
    long total = 0;
    volatile long barrier = 0;
    
    /* Outer loop with switch inside */
    for (int i = 0; i < size; i++) {
        switch (i % 5) {
            case 0:
                for (int j = 0; j < size; j++) {
                    int idx = i * size + j;
                    matrix[idx] = (matrix[idx] * 3 + 7) & 0xFF;
                    total += matrix[idx];
                    
                    /* Memory dependency chain */
                    asm volatile("" : "+r" (matrix[idx]) : : "memory");
                }
                break;
                
            case 1:
                for (int j = size - 1; j >= 0; j--) {
                    int idx = i * size + j;
                    matrix[idx] ^= (total & 0xFF);
                    total -= matrix[idx];
                    
                    /* Artificial scheduling barrier */
                    asm volatile("" : : : "memory");
                }
                break;
                
            case 2:
                for (int j = 0; j < size; j += 2) {
                    int idx1 = i * size + j;
                    int idx2 = i * size + j + 1;
                    matrix[idx1] = compute_branch(matrix[idx1], matrix[idx2]);
                    matrix[idx2] = hash_mix(matrix[idx1], matrix[idx2], j);
                    total += matrix[idx1] - matrix[idx2];
                }
                break;
                
            case 3:
                /* Do-while loop with early exit */
                int j = 0;
                do {
                    int idx = i * size + j;
                    matrix[idx] = (matrix[idx] << 3) | (matrix[idx] >> 5);
                    total ^= matrix[idx];
                    j++;
                } while (j < size && (matrix[i * size + j - 1] & 1));
                break;
                
            case 4:
                /* While loop with complex condition */
                j = 0;
                while (j < size && total < 1000000) {
                    int idx = i * size + j;
                    matrix[idx] += (total % 256);
                    total *= (matrix[idx] + 1);
                    j += (matrix[idx] % 3) + 1;
                }
                break;
        }
        
        /* Mix in some function calls */
        barrier = total;
        total = hash_mix(total, i, barrier);
    }
    
    return total;
}

/* Vectorization candidate with OpenMP */
NOINLINE static void vectorizable_loop(int *a, int *b, int *c, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Simple stride-1 operations for vectorization */
        a[i] = b[i] * 3 + c[i] * 7;
        c[i] = a[i] ^ b[i];
        b[i] = (b[i] << 2) | (b[i] >> 30);
        
        /* Add some branching to complicate scheduling */
        if (a[i] > 1000) {
            a[i] = a[i] / 3;
        } else {
            a[i] = a[i] * 2;
        }
    }
}

/* Complex function with all patterns combined */
NOINLINE HOT static unsigned long stress_scheduler(int iterations, int *data, int size) {
    unsigned long checksum = 0xDEADBEEF;
    int *temp = malloc(size * sizeof(int));
    
    if (!temp) return 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Copy and transform data */
        memcpy(temp, data, size * sizeof(int));
        
        /* Process with irreducible CFG */
        checksum ^= irreducible_cfg(size, temp);
        
        /* Process matrix-style */
        int matrix_size = (size > 100) ? 10 : 5;
        int *matrix = temp;
        long matrix_result = process_matrix(matrix_size, matrix);
        checksum += matrix_result;
        
        /* Vectorizable section */
        int vec_size = size / 2;
        if (vec_size > 0) {
            vectorizable_loop(temp, temp + vec_size, data, vec_size);
            
            /* Accumulate results */
            for (int i = 0; i < vec_size; i++) {
                checksum = hash_mix(checksum, temp[i], data[i]);
            }
        }
        
        /* Deeply nested if-else chain */
        int x = checksum & 0xFF;
        if (x < 64) {
            if (x < 32) {
                if (x < 16) {
                    if (x < 8) checksum <<= 1;
                    else checksum >>= 1;
                } else {
                    if (x < 24) checksum ^= 0xAAAA;
                    else checksum |= 0x5555;
                }
            } else {
                if (x < 48) {
                    if (x < 40) checksum += 0x1234;
                    else checksum -= 0x4321;
                } else {
                    if (x < 56) checksum *= 3;
                    else checksum /= 3;
                }
            }
        } else {
            if (x < 128) {
                if (x < 96) {
                    if (x < 80) checksum = ~checksum;
                    else checksum = checksum & 0xFFFF;
                } else {
                    if (x < 112) checksum = checksum | 0xFF00;
                    else checksum = checksum ^ 0xFF00;
                }
            } else {
                if (x < 192) {
                    if (x < 160) checksum = (checksum << 8) | (checksum >> 24);
                    else checksum = (checksum >> 8) | (checksum << 24);
                } else {
                    if (x < 224) checksum += checksum;
                    else checksum = checksum % 65537;
                }
            }
        }
    }
    
    free(temp);
    return checksum;
}

/* Main driver with warm-up and verification */
int main() {
    const int small_size = 100;
    const int medium_size = 500;
    const int large_size = 1000;
    
    int *data_small = malloc(small_size * sizeof(int));
    int *data_medium = malloc(medium_size * sizeof(int));
    int *data_large = malloc(large_size * sizeof(int));
    
    if (!data_small || !data_medium || !data_large) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (int i = 0; i < large_size; i++) {
        int val = rand() % 1000;
        if (i < small_size) data_small[i] = val;
        if (i < medium_size) data_medium[i] = val;
        data_large[i] = val;
    }
    
    printf("Starting scheduler stress test...\n");
    
    /* Warm-up phase */
    printf("Warm-up phase...\n");
    unsigned long warmup_result = 0;
    for (int i = 0; i < 10; i++) {
        warmup_result ^= stress_scheduler(2, data_small, small_size);
    }
    printf("Warm-up checksum: %lu\n", warmup_result);
    
    /* Main test with different sizes to trigger different scheduling paths */
    printf("\nRunning main tests...\n");
    
    clock_t start = clock();
    
    unsigned long result1 = stress_scheduler(50, data_small, small_size);
    printf("Small dataset result: %lu\n", result1);
    
    unsigned long result2 = stress_scheduler(20, data_medium, medium_size);
    printf("Medium dataset result: %lu\n", result2);
    
    unsigned long result3 = stress_scheduler(10, data_large, large_size);
    printf("Large dataset result: %lu\n", result3);
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("\nTotal time: %.3f seconds\n", elapsed);
    
    /* Final verification checksum */
    unsigned long final_checksum = result1 ^ result2 ^ result3;
    printf("Final verification checksum: %lu\n", final_checksum);
    
    /* Cleanup */
    free(data_small);
    free(data_medium);
    free(data_large);
    
    return 0;
}
