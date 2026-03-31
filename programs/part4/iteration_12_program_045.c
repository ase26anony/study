/* Target: haifa-sched.cc free_sched_context coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

#define HOT __attribute__((hot))
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#define NOINLINE __attribute__((noinline))

/* Helper functions for inlining */
ALWAYS_INLINE unsigned int hash_mix(unsigned int a, unsigned int b, unsigned int c) {
    a ^= b; a -= (b << 14) | (b >> 18);
    b ^= c; b -= (c << 11) | (c >> 21);
    c ^= a; c -= (a << 16) | (a >> 16);
    return c;
}

ALWAYS_INLINE int compute_index(int x, int y, int mod) {
    int t = (x * 1103515245 + 12345) ^ (y * 1664525 + 1013904223);
    return (t & 0x7FFFFFFF) % mod;
}

/* Complex control flow with irreducible CFG using computed goto */
HOT NOINLINE unsigned int irreducible_cfg(int n, int *data) {
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5, &&L6, &&L7 };
    unsigned int sum = 0;
    int i = 0;
    
    L0:
    if (i >= n) goto end;
    sum += data[i] * 3;
    i++;
    goto *labels[data[i-1] & 7];
    
    L1:
    sum ^= (sum << 13) | (sum >> 19);
    data[i % n] = sum & 0xFF;
    goto *labels[(sum >> 3) & 7];
    
    L2:
    {
        int temp = data[(i * 17) % n];
        sum += temp * temp;
        i += 2;
        goto *labels[(temp + i) & 7];
    }
    
    L3:
    sum = (sum * 1103515245 + 12345) & 0x7FFFFFFF;
    if (sum & 1) goto L5;
    goto L7;
    
    L4:
    {
        volatile int barrier;
        asm volatile ("# Memory barrier" : "=m"(barrier) : : "memory");
        sum += data[sum % n];
        goto L0;
    }
    
    L5:
    for (int j = 0; j < 3; j++) {
        sum += data[(i + j) % n] * j;
        if (sum & (1 << j)) {
            sum ^= data[j % n];
        }
    }
    goto L2;
    
    L6:
    switch (sum & 3) {
        case 0: sum += hash_mix(sum, i, n); break;
        case 1: sum ^= (data[i % n] << 8) | data[(i+1) % n]; break;
        case 2: sum = (sum * 6364136223846793005ULL) >> 32; break;
        case 3: sum = ~sum; break;
    }
    goto L4;
    
    L7:
    if (sum % 7 == 0) {
        sum += compute_index(i, sum, n);
        goto L1;
    } else {
        sum -= compute_index(sum, i, n);
        goto L6;
    }
    
    end:
    return sum;
}

/* Loop with carried dependencies and vectorization candidate */
HOT NOINLINE void vectorizable_loop(int *restrict a, int *restrict b, 
                                    int *restrict c, int n) {
    #pragma omp simd simdlen(8)
    for (int i = 0; i < n; i++) {
        int t = a[i] * 3 + b[i];
        // Artificial dependency chain
        for (int j = 0; j < 3; j++) {
            t = (t << 3) | (t >> 29);
            t ^= 0x5A827999;
        }
        c[i] = t + i;
    }
}

/* Deeply nested if-else chains with memory operations */
HOT NOINLINE unsigned int complex_branching(int *arr, int size) {
    unsigned int result = 0xDEADBEEF;
    int *ptr = arr;
    int *end = arr + size;
    
    while (ptr < end) {
        int val = *ptr;
        
        if (val < 0) {
            if (val < -1000) {
                result ^= (result << 7) | (result >> 25);
                result += val * 3;
            } else if (val < -100) {
                result = (result * 1664525 + 1013904223) & 0x7FFFFFFF;
                for (int k = 0; k < 4; k++) {
                    result ^= ptr[k % size];
                }
            } else {
                result += hash_mix(result, val, size);
            }
        } else if (val == 0) {
            switch (result & 7) {
                case 0: result = ~result; break;
                case 1: result ^= 0xAAAAAAAA; break;
                case 2: result = (result >> 16) | (result << 16); break;
                case 3: result += 0x55555555; break;
                case 4: result *= 3; break;
                case 5: result ^= result >> 12; break;
                case 6: result = (result & 0x0F0F0F0F) << 4; break;
                case 7: result = (result & 0xF0F0F0F0) >> 4; break;
            }
        } else if (val < 100) {
            for (int i = 0; i < (val & 3); i++) {
                result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
                result ^= arr[result % size];
            }
        } else if (val < 1000) {
            int temp = val;
            do {
                result += temp;
                temp = (temp * 6364136223846793005ULL) >> 32;
            } while (temp > 10);
        } else {
            // Memory barrier to force scheduling constraints
            asm volatile ("# Heavy dependency" : : : "memory");
            result = (result + val) * 0x9E3779B9;
        }
        
        ptr++;
        
        // Additional nested condition
        if ((ptr - arr) % 128 == 0) {
            result ^= arr[(ptr - arr) % size];
        }
    }
    
    return result;
}

/* Mixed instruction types with artificial dependencies */
HOT NOINLINE unsigned int instruction_mix(int n) {
    unsigned int a = 1, b = 2, c = 3, d = 4;
    unsigned int sum = 0;
    
    for (int i = 0; i < n; i++) {
        // Integer arithmetic
        a = a * 1103515245 + 12345;
        b = (b << 13) | (b >> 19);
        c = c ^ (c >> 17);
        d = d + (d << 2) + (d << 4);
        
        // Bitwise operations
        unsigned int t = (a & b) | (c & ~d);
        t = t ^ (t << 15);
        t = t * 0x85EBCA6B;
        
        // Conditional operations
        if (t & 1) {
            sum += t;
            asm volatile ("# Conditional barrier" : : : "memory");
        } else if (t & 2) {
            sum ^= t;
        } else {
            sum = (sum * 3 + t) & 0x7FFFFFFF;
        }
        
        // Memory operation simulation
        volatile unsigned int mem = t;
        sum += mem;
        
        // Function call to inline candidate
        sum = hash_mix(sum, a, b);
        
        // Switch statement
        switch (i & 7) {
            case 0: sum += compute_index(i, sum, n); break;
            case 1: sum ^= compute_index(sum, i, n); break;
            case 2: sum = (sum >> 1) | (sum << 31); break;
            case 3: sum = ~sum; break;
            case 4: sum *= 0xCC9E2D51; break;
            case 5: sum = (sum + 0x61C88647) & 0x7FFFFFFF; break;
            case 6: sum ^= sum >> 16; break;
            case 7: sum = (sum & 0x55555555) << 1 | (sum & 0xAAAAAAAA) >> 1; break;
        }
    }
    
    return sum;
}

/* Main orchestrator */
int main() {
    const int sizes[] = {1024, 2048, 4096, 8192};
    const int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    unsigned int final_checksum = 0;
    
    // Warm-up phase
    printf("Warm-up phase...\n");
    for (int iter = 0; iter < 3; iter++) {
        for (int s = 0; s < num_sizes; s++) {
            int size = sizes[s];
            int *data1 = (int*)malloc(size * sizeof(int));
            int *data2 = (int*)malloc(size * sizeof(int));
            int *data3 = (int*)malloc(size * sizeof(int));
            
            // Initialize with pseudo-random data
            for (int i = 0; i < size; i++) {
                data1[i] = (i * 1103515245 + 12345) & 0x7FFF;
                data2[i] = (i * 1664525 + 1013904223) & 0x7FFF;
                data3[i] = 0;
            }
            
            // Call complex functions
            final_checksum ^= irreducible_cfg(size, data1);
            vectorizable_loop(data1, data2, data3, size);
            final_checksum ^= complex_branching(data1, size);
            final_checksum += instruction_mix(size / 4);
            
            free(data1);
            free(data2);
            free(data3);
        }
    }
    
    // Timed execution phase
    printf("Timed execution phase...\n");
    clock_t start = clock();
    
    #pragma omp parallel for schedule(dynamic)
    for (int s = 0; s < num_sizes * 2; s++) {
        int size = sizes[s % num_sizes] * (1 + (s / num_sizes));
        int *data1 = (int*)malloc(size * sizeof(int));
        int *data2 = (int*)malloc(size * sizeof(int));
        int *data3 = (int*)malloc(size * sizeof(int));
        
        // Different initialization pattern
        unsigned int seed = s * 9973;
        for (int i = 0; i < size; i++) {
            seed = seed * 1103515245 + 12345;
            data1[i] = (seed >> 16) & 0x7FFF;
            seed = seed * 1664525 + 1013904223;
            data2[i] = (seed >> 16) & 0x7FFF;
            data3[i] = 0;
        }
        
        // Mix of scheduling-intensive operations
        unsigned int local_sum = 0;
        for (int rep = 0; rep < 2; rep++) {
            local_sum ^= irreducible_cfg(size, data1);
            vectorizable_loop(data1, data2, data3, size);
            local_sum += complex_branching(data1, size);
            local_sum ^= instruction_mix(size / 8);
            
            // Additional OpenMP parallel region inside
            #pragma omp parallel for reduction(+:local_sum)
            for (int i = 0; i < 4; i++) {
                local_sum += compute_index(i, local_sum, size);
            }
        }
        
        #pragma omp atomic
        final_checksum ^= local_sum;
        
        free(data1);
        free(data2);
        free(data3);
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Final checksum: 0x%08X\n", final_checksum);
    printf("Execution time: %.3f seconds\n", elapsed);
    
    return 0;
}
