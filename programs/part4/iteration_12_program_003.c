/* sched_stress.c - Stress test for GCC's Haifa scheduler context cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <omp.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100000
#define MAX_DEPTH 8

/* Always inline helper functions */
static inline int __attribute__((always_inline)) 
compute_hash(int a, int b) {
    return (a ^ (b << 1)) + (a >> 3) * b;
}

static inline int __attribute__((always_inline))
bitwise_mix(int x, int y) {
    int result = 0;
    result = (x & 0xFF) | ((y & 0xFF) << 8);
    result ^= (x >> 8) | ((y >> 8) << 16);
    
    /* Artificial scheduling barrier */
    asm volatile("" : "+r" (result) : : "memory");
    
    return result;
}

/* Hot function with complex control flow */
__attribute__((hot))
void complex_control_flow(int *data, int size, int *result) {
    int i, j, k;
    volatile int barrier = 0; /* Prevent optimization */
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i++) {
        int temp = data[i];
        
        /* Deep if-else chain */
        if (temp < 0) {
            temp = -temp;
            if (temp > 100) {
                temp = compute_hash(temp, i);
            } else if (temp > 50) {
                temp = bitwise_mix(temp, i);
            } else {
                for (j = 0; j < (i % 5); j++) {
                    temp += j * 3;
                }
            }
        } else if (temp == 0) {
            temp = 1;
        } else {
            switch (temp % 7) {
                case 0: temp <<= 1; break;
                case 1: temp >>= 1; break;
                case 2: temp ^= 0x55AA55AA; break;
                case 3: temp = compute_hash(temp, temp); break;
                case 4: temp = bitwise_mix(temp, ~temp); break;
                case 5: temp *= 3; break;
                case 6: temp = (temp * 13) / 7; break;
            }
        }
        
        /* Memory operation with pointer arithmetic */
        int *ptr = &data[(i + 1) % size];
        *ptr += temp;
        
        /* Artificial dependency chain */
        for (k = 0; k < (temp % 4); k++) {
            barrier = *ptr;
            asm volatile("" : "+r" (barrier) : : "memory");
        }
        
        result[i] = temp;
    }
}

/* Function with irreducible control flow using computed goto */
__attribute__((noinline))
void irreducible_cfg(int *arr, int n) {
    if (n <= 0) return;
    
    /* Labels for computed goto */
    static void *labels[] = {
        &&L0, &&L1, &&L2, &&L3, &&L4,
        &&L5, &&L6, &&L7, &&L8, &&L9
    };
    
    int i = 0;
    int state = arr[0] % 10;
    
    L0: arr[i] += 1;
    L1: arr[i] ^= 0x12345678;
    L2: arr[i] = compute_hash(arr[i], i);
    L3: arr[i] *= 3;
    L4: arr[i] = bitwise_mix(arr[i], arr[(i+1)%n]);
    
    /* Complex state transitions */
    state = (state * 7 + 3) % 10;
    i = (i + 1) % n;
    
    /* Computed goto creates irreducible CFG */
    goto *labels[state];
    
    L5: arr[i] -= 5;
    L6: arr[i] |= 0xF0F0F0F0;
    L7: arr[i] = arr[i] * arr[(i+2)%n];
    L8: arr[i] = compute_hash(arr[i], state);
    L9: return; /* Actually unreachable but needed for label */
}

/* Vectorization candidate with carried dependency */
void vectorizable_loop(int *a, int *b, int *c, int n) {
    int i;
    int carry = 1;
    
    /* Loop with carried dependency - stresses scheduler */
    #pragma omp simd
    for (i = 0; i < n; i++) {
        a[i] = b[i] + c[i];
        
        /* Artificial carried dependency */
        a[i] += carry;
        carry = a[i] % 256;
        
        /* Mix in helper function calls */
        if (i % 3 == 0) {
            a[i] = compute_hash(a[i], i);
        } else if (i % 3 == 1) {
            a[i] = bitwise_mix(a[i], b[i]);
        }
        
        /* Memory clobber to prevent reordering */
        asm volatile("" : : "r"(a[i]), "r"(b[i]), "r"(c[i]) : "memory");
    }
}

/* Function with nested loops and OpenMP */
__attribute__((hot))
void parallel_region(int *data, int size) {
    int i, j;
    
    #pragma omp parallel for private(j) schedule(dynamic)
    for (i = 0; i < size; i++) {
        int sum = 0;
        
        /* Inner loop with varying bounds */
        for (j = 0; j < (i % 16) + 1; j++) {
            sum += data[(i + j) % size];
            
            /* Inline function calls at multiple sites */
            if (j % 2 == 0) {
                sum = compute_hash(sum, j);
            } else {
                sum = bitwise_mix(sum, data[j]);
            }
        }
        
        /* Switch statement inside parallel region */
        switch (sum % 5) {
            case 0: data[i] = sum << 2; break;
            case 1: data[i] = sum >> 2; break;
            case 2: data[i] = sum ^ 0xAAAAAAAA; break;
            case 3: data[i] = compute_hash(sum, data[i]); break;
            case 4: data[i] = bitwise_mix(sum, ~data[i]); break;
        }
    }
}

/* Recursive function with tail calls */
int __attribute__((noinline))
recursive_compute(int *arr, int idx, int depth) {
    if (depth >= MAX_DEPTH || idx >= ARRAY_SIZE) {
        return arr[idx % ARRAY_SIZE];
    }
    
    /* Complex recursive pattern */
    int a = recursive_compute(arr, idx * 2, depth + 1);
    int b = recursive_compute(arr, idx * 2 + 1, depth + 1);
    
    /* Mix operations */
    int result = compute_hash(a, b);
    
    /* Memory operation */
    arr[idx % ARRAY_SIZE] = result;
    
    /* Tail-like call but with computation after */
    int next = (idx + 1) % ARRAY_SIZE;
    return bitwise_mix(result, recursive_compute(arr, next, depth + 1));
}

/* Main orchestrator */
int main() {
    int i, j;
    clock_t start, end;
    unsigned long long checksum = 0;
    
    /* Allocate and initialize data */
    int *data1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data3 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *results = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!data1 || !data2 || !data3 || !results) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand();
        data2[i] = rand();
        data3[i] = rand();
        results[i] = 0;
    }
    
    printf("Starting scheduler stress test...\n");
    start = clock();
    
    /* Warm-up phase - trigger optimization heuristics */
    for (j = 0; j < ITERATIONS / 100; j++) {
        complex_control_flow(data1, ARRAY_SIZE / 4, results);
    }
    
    /* Main test phase with different patterns */
    for (i = 0; i < 10; i++) {
        /* Vary array sizes to trigger different scheduling paths */
        int size = ARRAY_SIZE / (1 << (i % 4));
        
        /* Call different functions with complex control flow */
        complex_control_flow(data1, size, results);
        
        /* Trigger irreducible CFG */
        irreducible_cfg(data2, size);
        
        /* Vectorization attempts */
        vectorizable_loop(data3, data1, data2, size);
        
        /* Parallel region */
        parallel_region(data1, size);
        
        /* Recursive computation */
        checksum += recursive_compute(data3, 1, 0);
        
        /* Mix up data */
        for (j = 0; j < size; j++) {
            data1[j] = compute_hash(data1[j], data2[j]);
            data2[j] = bitwise_mix(data2[j], data3[j]);
            data3[j] = data1[j] ^ data2[j] ^ data3[j];
        }
    }
    
    /* Compute final checksum for verification */
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += data1[i];
        checksum += data2[i];
        checksum += data3[i];
        checksum += results[i];
        
        /* Prevent optimization */
        asm volatile("" : "+r" (checksum) : : "memory");
    }
    
    end = clock();
    
    printf("Test completed.\n");
    printf("Checksum: %llu\n", checksum);
    printf("Time: %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(results);
    
    return 0;
}
