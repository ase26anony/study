/* Test program to trigger free_sched_context in GCC's Haifa scheduler */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <omp.h>

#define ARRAY_SIZE 1024
#define MAX_DEPTH 8
#define ITERATIONS 1000

/* Helper functions marked for inlining */
static inline int __attribute__((always_inline)) 
compute_hash(int a, int b, int c) {
    /* Complex bitwise operations creating ILP opportunities */
    int t1 = (a ^ b) & (b | c);
    int t2 = (a << 3) | (b >> 2);
    int t3 = (c * 7) ^ (t1 & 0xFF);
    return (t1 + t2) ^ t3;
}

static inline int __attribute__((always_inline))
process_element(int x, int *state) {
    /* Memory operation with pointer arithmetic */
    int old = *state;
    *state = x ^ old;
    
    /* Artificial scheduling barrier */
    asm volatile("" ::: "memory");
    
    return (x * old) + (*state);
}

/* Hot function with complex control flow */
__attribute__((hot, noinline))
void complex_control_flow(int *data, int size, int *result) {
    int i, j, k;
    int temp[MAX_DEPTH];
    int state = 0x5A5A5A5A;
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i++) {
        /* Deep if-else chain */
        if (data[i] < 0) {
            temp[0] = -data[i];
            for (j = 1; j < MAX_DEPTH; j++) {
                temp[j] = compute_hash(temp[j-1], i, j);
            }
        } else if (data[i] == 0) {
            /* Switch statement with multiple cases */
            switch (i % 5) {
                case 0: temp[0] = data[i] + 1; break;
                case 1: temp[0] = data[i] * 2; break;
                case 2: temp[0] = data[i] | 0xFF; break;
                case 3: temp[0] = data[i] ^ state; break;
                case 4: temp[0] = ~data[i]; break;
                default: temp[0] = 0;
            }
            for (j = 1; j < MAX_DEPTH/2; j++) {
                temp[j] = temp[j-1] * 3;
            }
        } else {
            temp[0] = process_element(data[i], &state);
            for (j = 1; j < MAX_DEPTH; j += 2) {
                temp[j] = temp[j-1] + j;
                if (j+1 < MAX_DEPTH) {
                    temp[j+1] = temp[j] ^ 0xAA;
                }
            }
        }
        
        /* Compute final value with mixed operations */
        int sum = 0;
        for (k = 0; k < MAX_DEPTH; k++) {
            sum += temp[k];
            /* Memory dependency chain */
            asm volatile("" : "+r"(sum) : : "memory");
        }
        result[i] = sum;
    }
}

/* Function with irreducible control flow using computed goto */
__attribute__((noinline))
void irreducible_cfg(int *data, int size) {
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5 };
    int i = 0;
    int state = 0;
    
    /* Computed goto creates irreducible control flow */
    goto *labels[data[0] % 6];
    
L0:
    state = data[i] * 3;
    i = (i + 1) % size;
    goto *labels[(state + i) % 6];
    
L1:
    state ^= data[i];
    i = (i + 2) % size;
    goto *labels[(state >> 2) % 6];
    
L2:
    state += data[i] | 0x55;
    i = (i + 3) % size;
    if (state > 1000) goto L4;
    goto *labels[(state * 7) % 6];
    
L3:
    state = (state << 1) | (data[i] & 1);
    i = (i + 1) % size;
    goto *labels[state % 6];
    
L4:
    state -= data[i];
    i = (i + 4) % size;
    goto *labels[abs(state) % 6];
    
L5:
    state = (state * state) % 1000;
    i = (i + 1) % size;
    if (i == 0) return;
    goto *labels[data[i] % 6];
}

/* Loop with carried dependency for scheduling stress */
__attribute__((hot))
void tight_inner_loop(int *a, int *b, int *c, int n) {
    int i;
    int acc = 0;
    
    /* Loop with carried dependency */
    for (i = 1; i < n; i++) {
        a[i] = b[i] + c[i];
        /* Create artificial dependency chain */
        b[i] = a[i-1] * 2;
        c[i] = b[i-1] + i;
        
        /* Mix in function calls */
        acc = compute_hash(acc, a[i], b[i]);
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Final reduction */
    for (i = 0; i < n; i++) {
        acc ^= a[i];
    }
    a[0] = acc;
}

/* Vectorization candidate with OpenMP */
__attribute__((hot))
void vectorizable_loop(int *src, int *dst, int n) {
    int i;
    
    #pragma omp simd safelen(16)
    for (i = 0; i < n; i++) {
        /* Simple stride-1 operations good for vectorization */
        dst[i] = src[i] * 3 + 7;
        dst[i] ^= 0xAAAAAAAA;
        dst[i] = (dst[i] << 2) | (dst[i] >> 30);
    }
    
    /* Additional loop with reduction */
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (i = 0; i < n; i++) {
        sum += dst[i];
    }
    dst[0] = sum;
}

/* Function with switch and loops */
__attribute__((noinline))
void switch_with_loops(int *data, int size, int mode) {
    int i, j;
    
    switch (mode) {
        case 0:
            /* While loop */
            i = 0;
            while (i < size) {
                data[i] = compute_hash(data[i], i, mode);
                i += 1 + (data[i] & 3);  /* Variable increment */
            }
            break;
            
        case 1:
            /* Do-while loop */
            i = 0;
            do {
                data[i] = process_element(data[i], &data[(i+1)%size]);
                i = (i * 2 + 1) % size;
            } while (i != 0);
            break;
            
        case 2:
            /* Nested for loops */
            for (i = 0; i < size; i++) {
                int val = data[i];
                for (j = 0; j < 4; j++) {
                    val = (val << 4) | (val >> 28);
                    val ^= 0xCCCCCCCC >> (j * 4);
                }
                data[i] = val;
            }
            break;
            
        default:
            /* Mixed loop types */
            for (i = 0; i < size; i++) {
                int k = i;
                while (k > 0) {
                    data[i] += data[k % size];
                    k /= 2;
                }
            }
    }
}

/* Main orchestrator */
int main() {
    int *data1 = malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = malloc(ARRAY_SIZE * sizeof(int));
    int *result = malloc(ARRAY_SIZE * sizeof(int));
    int i, iter;
    clock_t start, end;
    long long checksum = 0;
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand() % 1000;
        data2[i] = rand() % 1000;
    }
    
    printf("Starting scheduler stress test...\n");
    start = clock();
    
    /* Warm-up phase */
    for (iter = 0; iter < ITERATIONS / 10; iter++) {
        complex_control_flow(data1, ARRAY_SIZE / 4, result);
    }
    
    /* Main test phase with different patterns */
    for (iter = 0; iter < ITERATIONS; iter++) {
        int mode = iter % 4;
        
        /* Alternate between different functions */
        switch (mode) {
            case 0:
                complex_control_flow(data1, ARRAY_SIZE, result);
                break;
            case 1:
                tight_inner_loop(data1, data2, result, ARRAY_SIZE);
                break;
            case 2:
                vectorizable_loop(data1, result, ARRAY_SIZE);
                break;
            case 3:
                switch_with_loops(data1, ARRAY_SIZE, iter % 3);
                break;
        }
        
        /* Occasionally trigger irreducible CFG */
        if (iter % 50 == 0) {
            irreducible_cfg(data1, ARRAY_SIZE);
        }
        
        /* Update checksum for verification */
        for (i = 0; i < ARRAY_SIZE; i += 64) {
            checksum ^= (long long)data1[i] << (i % 32);
            checksum += result[i];
        }
    }
    
    end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Test completed in %.2f seconds\n", elapsed);
    printf("Final checksum: 0x%016llX\n", checksum);
    
    /* Verification */
    if (checksum != 0) {
        printf("Result is non-zero - computation performed.\n");
    }
    
    free(data1);
    free(data2);
    free(result);
    
    return 0;
}
