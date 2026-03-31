/* Test program to trigger free_sched_context in GCC's Haifa scheduler */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

#define HOT __attribute__((hot))
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#define NOINLINE __attribute__((noinline))

/* Helper functions for inlining */
ALWAYS_INLINE unsigned int compute_hash(unsigned int x) {
    /* Mix of arithmetic and bitwise ops for ILP */
    x = (x ^ (x >> 16)) * 0x85ebca6b;
    x = (x ^ (x >> 13)) * 0xc2b2ae35;
    return x ^ (x >> 16);
}

ALWAYS_INLINE int complex_condition(int a, int b, int c) {
    /* Deep conditional chain */
    if (a > b) {
        if (b < c) {
            return a * c - b;
        } else if (a == c) {
            return (a << 3) | (b & 0xF);
        } else {
            return (a ^ b ^ c) + (a & b & c);
        }
    } else if (a == b) {
        return (c * 7) ^ (a * 13);
    } else {
        if (c > 100) {
            return (b - a) * c;
        } else if (c < 0) {
            return (a + b) * (-c);
        } else {
            return (a * b) + (b * c) + (c * a);
        }
    }
}

/* Function with irreducible control flow using computed goto */
HOT NOINLINE void irreducible_cfg(int *arr, int n, int *result) {
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5 };
    
    int sum = 0;
    int i = 0;
    int state = 0;
    
    /* Complex loop with computed goto */
    while (i < n) {
        void *target = labels[state % 6];
        goto *target;
        
    L0:
        sum += arr[i] * 2;
        state = (state + arr[i]) % 6;
        i++;
        continue;
    L1:
        sum -= arr[i] * 3;
        state = (state ^ arr[i]) % 6;
        i++;
        continue;
    L2:
        sum ^= arr[i];
        state = (state * 17 + 1) % 6;
        i++;
        continue;
    L3:
        sum = (sum << 3) | (arr[i] & 0xFF);
        state = (state + i) % 6;
        i++;
        continue;
    L4:
        sum = complex_condition(sum, arr[i], i);
        state = (state * 31) % 6;
        i++;
        continue;
    L5:
        sum = compute_hash(sum + arr[i]);
        state = (state + 5) % 6;
        i++;
        continue;
    }
    
    *result = sum;
}

/* Function with nested loops and memory dependencies */
HOT void nested_loop_scheduler(int *A, int *B, int *C, int n) {
    /* Outer loop with carried dependency */
    for (int i = 1; i < n; i++) {
        /* Inner loop with independent operations */
        for (int j = 0; j < n; j++) {
            /* Mix of memory ops and arithmetic */
            int temp = A[j] * B[(i + j) % n];
            
            /* Artificial scheduling barrier */
            asm volatile("" : "+r"(temp) : : "memory");
            
            C[j] = temp + complex_condition(i, j, A[j]);
            
            /* Another memory dependency */
            if (j > 0) {
                C[j] += C[j-1] & 0x3FF;
            }
        }
        
        /* Switch statement inside outer loop */
        switch (i % 7) {
            case 0:
                A[i] = B[i] * 3;
                break;
            case 1:
                A[i] = B[i] ^ 0xAAAA;
                break;
            case 2:
                A[i] = compute_hash(B[i]);
                break;
            case 3:
                A[i] = complex_condition(A[i], B[i], i);
                break;
            case 4:
                A[i] = (A[i] << 1) | (B[i] >> 31);
                break;
            case 5:
                A[i] = A[i] + B[i] * 2;
                break;
            default:
                A[i] = A[i] - B[i];
        }
    }
}

/* Vectorization candidate with OpenMP */
HOT void vectorizable_loop(int *in1, int *in2, int *out, int n) {
    #pragma omp simd safelen(16)
    for (int i = 0; i < n; i++) {
        /* Simple stride-1 operations for vectorization */
        int a = in1[i];
        int b = in2[i];
        
        /* Multiple independent operations */
        int t1 = a * b;
        int t2 = a + b;
        int t3 = a ^ b;
        int t4 = (a << 2) | (b >> 30);
        
        /* Combine results with function call */
        out[i] = compute_hash(t1 + t2 + t3 + t4);
    }
}

/* Function with do-while and while loops */
HOT void mixed_loops(int *data, int n, int threshold) {
    int i = 0;
    
    /* do-while with early exit */
    do {
        if (data[i] > threshold) {
            /* Complex computation in hot path */
            data[i] = complex_condition(data[i], i, threshold);
            
            /* Nested while */
            int j = 0;
            while (j < 10) {
                data[i] ^= (data[i] << j);
                j++;
            }
        } else {
            data[i] = compute_hash(data[i]);
        }
        
        i++;
    } while (i < n && data[i-1] != 0);
    
    /* Another while loop */
    while (i < n) {
        /* Memory operations with pointer arithmetic */
        int *ptr = &data[i];
        *ptr = (*ptr * 31) & 0x7FFF;
        
        /* Artificial asm barrier */
        asm volatile("" : : "r"(ptr) : "memory");
        
        i += 2;
    }
}

/* Main orchestrator */
int main() {
    const int sizes[] = {1024, 2048, 4096, 8192};
    const int num_sizes = sizeof(sizes)/sizeof(sizes[0]);
    
    unsigned long long total_checksum = 0;
    
    /* Warm-up phase */
    printf("Starting warm-up...\n");
    for (int iter = 0; iter < 3; iter++) {
        for (int s = 0; s < num_sizes; s++) {
            int n = sizes[s] / (iter + 1);
            
            /* Allocate with different alignments */
            int *A = (int*)aligned_alloc(64, n * sizeof(int));
            int *B = (int*)aligned_alloc(32, n * sizeof(int));
            int *C = (int*)aligned_alloc(16, n * sizeof(int));
            int *D = (int*)malloc(n * sizeof(int));
            
            /* Initialize with pattern */
            for (int i = 0; i < n; i++) {
                A[i] = (i * 37) & 0xFFF;
                B[i] = (i * 73) ^ 0xABCD;
                C[i] = 0;
                D[i] = i;
            }
            
            /* Call different scheduling-intensive functions */
            irreducible_cfg(A, n, &C[0]);
            nested_loop_scheduler(A, B, C, n > 100 ? 100 : n);
            vectorizable_loop(A, B, D, n);
            mixed_loops(C, n, 1000);
            
            /* Compute checksum */
            for (int i = 0; i < n; i++) {
                total_checksum ^= (unsigned long long)A[i] << 32;
                total_checksum ^= B[i];
                total_checksum = compute_hash(total_checksum);
                total_checksum += C[i] * D[i];
            }
            
            free(A);
            free(B);
            free(C);
            free(D);
        }
    }
    
    /* Main timed phase */
    printf("Main computation phase...\n");
    clock_t start = clock();
    
    #pragma omp parallel for reduction(^:total_checksum)
    for (int trial = 0; trial < 10; trial++) {
        for (int s = 0; s < num_sizes; s++) {
            int n = sizes[s];
            
            int *A = (int*)aligned_alloc(64, n * sizeof(int));
            int *B = (int*)aligned_alloc(64, n * sizeof(int));
            int *C = (int*)aligned_alloc(64, n * sizeof(int));
            
            /* Different initialization patterns */
            for (int i = 0; i < n; i++) {
                A[i] = (i * (trial + 1) * 97) & 0xFFFF;
                B[i] = (i ^ (trial * 0x1234)) * 31;
                C[i] = 0;
            }
            
            /* Stress scheduler with all patterns */
            int temp_result;
            irreducible_cfg(A, n, &temp_result);
            
            nested_loop_scheduler(A, B, C, n > 500 ? 500 : n);
            
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                A[i] = complex_condition(A[i], B[i], C[i]);
            }
            
            vectorizable_loop(A, B, C, n);
            mixed_loops(C, n, 2000);
            
            /* Local checksum */
            unsigned long long local_sum = 0;
            for (int i = 0; i < n; i += 8) {
                for (int j = 0; j < 8 && (i+j) < n; j++) {
                    local_sum = compute_hash(local_sum + A[i+j] + B[i+j] * 3);
                    local_sum ^= C[i+j] << (j * 4);
                }
            }
            
            total_checksum ^= local_sum;
            
            free(A);
            free(B);
            free(C);
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Final checksum: 0x%016llX\n", total_checksum);
    printf("Time elapsed: %.3f seconds\n", elapsed);
    printf("Test completed successfully.\n");
    
    return 0;
}
