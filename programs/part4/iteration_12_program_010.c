/* Complex control flow patterns to stress GCC's Haifa scheduler */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <omp.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000
#define MAX_DEPTH 8

/* Always inline helper functions */
static inline int __attribute__((always_inline)) 
compute_hash(int a, int b) {
    return (a ^ b) + (a << 3) + (b >> 2);
}

static inline int __attribute__((always_inline))
bit_reverse(int x) {
    x = ((x >> 1) & 0x55555555) | ((x & 0x55555555) << 1);
    x = ((x >> 2) & 0x33333333) | ((x & 0x33333333) << 2);
    x = ((x >> 4) & 0x0F0F0F0F) | ((x & 0x0F0F0F0F) << 4);
    x = ((x >> 8) & 0x00FF00FF) | ((x & 0x00FF00FF) << 8);
    return (x >> 16) | (x << 16);
}

/* Hot function with complex control flow */
__attribute__((hot))
void complex_control_flow(int *data, int size, int depth) {
    int i, j, k;
    volatile int barrier = 0; /* Force memory dependencies */
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i++) {
        /* Deep if-else chain */
        if (data[i] & 1) {
            if (data[i] & 2) {
                if (data[i] & 4) {
                    data[i] = compute_hash(data[i], i);
                } else {
                    data[i] = bit_reverse(data[i]);
                }
            } else if (data[i] & 8) {
                /* Switch statement with multiple cases */
                switch (data[i] % 7) {
                    case 0: data[i] += i; break;
                    case 1: data[i] -= i * 2; break;
                    case 2: data[i] ^= 0xDEADBEEF; break;
                    case 3: data[i] = data[i] * 3 + 1; break;
                    case 4: data[i] = (data[i] << 1) | (data[i] >> 31); break;
                    case 5: data[i] = compute_hash(data[i], depth); break;
                    default: data[i] = bit_reverse(data[i] ^ i); break;
                }
            }
        }
        
        /* Memory operations with pointer arithmetic */
        int *ptr = &data[i];
        *ptr += *(ptr + (i % 16));
        
        /* Artificial scheduling barrier */
        asm volatile("" : "+r" (barrier) : : "memory");
    }
    
    /* Irreducible control flow using computed goto */
    if (depth > 0) {
        static void *labels[] = { &&label0, &&label1, &&label2, &&label3 };
        goto *labels[depth % 4];
        
    label0:
        for (j = 0; j < size / 2; j++) {
            data[j] = compute_hash(data[j], data[size - j - 1]);
        }
        goto end;
        
    label1:
        #pragma omp simd
        for (j = 0; j < size; j++) {
            data[j] = data[j] * 2 - 1;
        }
        goto end;
        
    label2:
        for (j = 0; j < size; j += 4) {
            /* Unrolled loop with dependencies */
            data[j] = bit_reverse(data[j]);
            data[j+1] = compute_hash(data[j], data[j+1]);
            data[j+2] = data[j] + data[j+1];
            data[j+3] = data[j+2] ^ data[j+3];
        }
        goto end;
        
    label3:
        /* Tight inner loop with carried dependency */
        int acc = 0;
        for (j = 0; j < size; j++) {
            acc = compute_hash(acc, data[j]);
            data[j] = acc;
        }
        goto end;
        
    end:
        /* Recursive call with reduced depth */
        if (depth > 1) {
            complex_control_flow(data, size, depth - 1);
        }
    }
}

/* Function with loops suitable for vectorization */
__attribute__((hot))
void vectorizable_loops(float *a, float *b, float *c, int n) {
    int i;
    
    /* Loop with stride-1 access pattern */
    #pragma omp parallel for simd
    for (i = 0; i < n; i++) {
        a[i] = b[i] * c[i] + 1.0f;
    }
    
    /* Loop with reduction */
    float sum = 0.0f;
    #pragma omp simd reduction(+:sum)
    for (i = 0; i < n; i++) {
        sum += a[i] * b[i];
        /* Memory clobber to force scheduling decisions */
        asm volatile("" : : : "memory");
    }
    
    /* Dependent loop chain */
    for (i = 1; i < n; i++) {
        a[i] = a[i-1] * 0.99f + b[i];
    }
    
    /* Mixed integer/float operations */
    for (i = 0; i < n; i++) {
        int idx = (int)(a[i] * 100) % n;
        c[i] = b[idx] + (float)compute_hash(i, idx);
    }
}

/* Function with switch-based state machine */
__attribute__((hot))
int state_machine(int *data, int size, int init_state) {
    int state = init_state;
    int result = 0;
    int i;
    
    for (i = 0; i < size; i++) {
        /* Complex switch with fall-through cases */
        switch (state) {
            case 0:
                data[i] = bit_reverse(data[i]);
                state = 1;
                break;
            case 1:
                data[i] = compute_hash(data[i], result);
                state = (data[i] & 1) ? 2 : 3;
                break;
            case 2:
                result += data[i];
                data[i] = result;
                state = 0;
                /* Fall through intentionally */
            case 3:
                result -= data[i];
                data[i] = bit_reverse(result);
                state = (i % 5 == 0) ? 1 : 2;
                break;
            case 4:
                #pragma omp simd
                for (int j = 0; j < 8; j++) {
                    data[(i + j) % size] ^= 0xAA;
                }
                state = 0;
                break;
            default:
                state = 0;
                break;
        }
        
        /* Inline assembly for scheduling barriers */
        asm volatile("" : "+r" (state), "+r" (result) : : "memory");
    }
    
    return result;
}

/* Main orchestrator function */
int main() {
    int i, iter;
    clock_t start, end;
    double total_time = 0.0;
    
    /* Allocate and initialize data */
    int *int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_a = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float *float_b = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float *float_c = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    srand(42);
    for (i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand();
        float_a[i] = (float)rand() / RAND_MAX;
        float_b[i] = (float)rand() / RAND_MAX;
        float_c[i] = (float)rand() / RAND_MAX;
    }
    
    printf("Starting scheduler stress test...\n");
    
    /* Warm-up phase */
    printf("Warm-up phase...\n");
    for (iter = 0; iter < ITERATIONS / 10; iter++) {
        complex_control_flow(int_data, ARRAY_SIZE / 2, 3);
        vectorizable_loops(float_a, float_b, float_c, ARRAY_SIZE / 4);
    }
    
    /* Main test phase with varying parameters */
    printf("Main test phase...\n");
    start = clock();
    
    for (iter = 0; iter < ITERATIONS; iter++) {
        int depth = (iter % MAX_DEPTH) + 1;
        int size = ARRAY_SIZE / (1 + (iter % 4));
        
        /* Alternate between different complex functions */
        if (iter % 3 == 0) {
            complex_control_flow(int_data, size, depth);
        } else if (iter % 3 == 1) {
            vectorizable_loops(float_a, float_b, float_c, size);
        } else {
            state_machine(int_data, size, iter % 5);
        }
        
        /* Mix in some OpenMP parallel regions */
        if (iter % 7 == 0) {
            #pragma omp parallel for
            for (i = 0; i < size; i++) {
                int_data[i] = compute_hash(int_data[i], omp_get_thread_num());
            }
        }
    }
    
    end = clock();
    total_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    /* Compute checksum for verification */
    unsigned long long checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += (unsigned long long)int_data[i];
        checksum += (unsigned long long)(float_a[i] * 1000);
    }
    
    printf("Test completed in %.2f seconds\n", total_time);
    printf("Final checksum: %llu\n", checksum);
    printf("Array[0] = %d, Array[%d] = %d\n", 
           int_data[0], ARRAY_SIZE-1, int_data[ARRAY_SIZE-1]);
    
    /* Cleanup */
    free(int_data);
    free(float_a);
    free(float_b);
    free(float_c);
    
    return 0;
}
