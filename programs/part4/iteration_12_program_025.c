/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -finline-functions -funswitch-loops -frandom-seed=1 -fopenmp */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>

#define N 1024
#define M 512
#define ITERS 1000

/* Always inline helper functions */
static inline int __attribute__((always_inline)) compute_hash(int x, int y) {
    return ((x ^ y) * 16777619) ^ ((x & y) * 1103515245);
}

static inline int __attribute__((always_inline)) scramble_bits(int val) {
    val = (val ^ (val >> 16)) * 0x45d9f3b;
    val = (val ^ (val >> 16)) * 0x45d9f3b;
    return val ^ (val >> 16);
}

/* Hot function with complex control flow */
__attribute__((hot)) void complex_scheduler_stress(int *arr, int size, int seed) {
    int i, j, k;
    volatile int barrier = 0; /* Force memory dependencies */
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i++) {
        int temp = arr[i];
        
        /* Deep if-else chain */
        if (temp < 0) {
            temp = -temp * 3 + seed;
        } else if (temp < 100) {
            temp = temp * temp - seed;
        } else if (temp < 1000) {
            temp = (temp >> 3) | (temp << 29);
        } else {
            temp = scramble_bits(temp);
        }
        
        /* Switch statement with multiple cases */
        switch (temp % 8) {
            case 0: temp += compute_hash(i, seed); break;
            case 1: temp -= scramble_bits(seed); break;
            case 2: temp ^= 0xDEADBEEF; break;
            case 3: temp = (temp * 13) & 0x7FFFFFFF; break;
            case 4: temp = ~temp; break;
            case 5: temp = temp / 3; break;
            case 6: temp = (temp << 4) | (temp >> 28); break;
            case 7: temp = compute_hash(temp, i); break;
        }
        
        /* Memory operation with pointer arithmetic */
        int *ptr = &arr[i];
        *ptr = temp;
        
        /* Artificial scheduling barrier */
        asm volatile("" : : "r"(ptr) : "memory");
        
        /* Inner loop with carried dependency */
        for (j = 0; j < (i % 16); j++) {
            arr[i] += compute_hash(arr[i], j);
        }
    }
    
    /* Irreducible control flow using computed goto */
    void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
    
    i = seed % 5;
    goto *labels[i];
    
L0:
    for (k = 0; k < size / 2; k++) {
        arr[k] = scramble_bits(arr[k] ^ arr[size - k - 1]);
    }
    goto END;
    
L1:
    #pragma omp simd
    for (k = 0; k < size; k++) {
        arr[k] = (arr[k] * 3) / 2;
    }
    goto END;
    
L2:
    for (k = 1; k < size; k++) {
        arr[k] += arr[k-1]; /* Create dependency chain */
    }
    goto END;
    
L3:
    /* Mixed operations */
    for (k = 0; k < size; k++) {
        int x = arr[k];
        x = (x & 0x55555555) << 1 | (x & 0xAAAAAAAA) >> 1;
        x = x * 1103515245 + 12345;
        arr[k] = x & 0x7FFFFFFF;
    }
    goto END;
    
L4:
    /* Another asm barrier */
    asm volatile("" : : : "memory");
    for (k = 0; k < size; k++) {
        arr[k] = compute_hash(arr[k], k);
    }
    
END:
    barrier = 1;
}

/* Function with tight inner loop and vectorization candidate */
__attribute__((hot)) void vectorizable_loop(int *a, int *b, int *c, int n) {
    int i;
    
    #pragma omp parallel for simd schedule(static)
    for (i = 0; i < n; i++) {
        /* Independent operations suitable for vectorization */
        int t1 = a[i] * 3 + b[i];
        int t2 = (a[i] << 2) | (b[i] >> 3);
        int t3 = scramble_bits(a[i] ^ b[i]);
        
        /* Complex expression with multiple dependencies */
        c[i] = (t1 * t2 + t3) & 0xFF;
        c[i] += compute_hash(c[i], i);
        
        /* Memory clobber to force scheduling constraints */
        if (i % 32 == 0) {
            asm volatile("" : : : "memory");
        }
    }
}

/* Function with switch inside loop */
void switch_in_loop(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        int val = arr[i];
        
        /* Complex switch with fallthrough */
        switch (val % 7) {
            case 0:
                val = scramble_bits(val);
                /* Fall through */
            case 1:
                val += compute_hash(val, i);
                break;
            case 2:
                val = (val * val) & 0xFFFF;
                /* Fall through */
            case 3:
                val ^= 0x12345678;
                break;
            case 4:
                val = ~val;
                /* Fall through */
            case 5:
                val = val * 7 + 3;
                break;
            case 6:
                val = (val << 1) | (val >> 31);
                break;
        }
        
        /* Nested if-else */
        if (val > 1000) {
            val /= 3;
        } else if (val > 100) {
            val *= 2;
        } else {
            val += 5;
        }
        
        arr[i] = val;
    }
}

/* Main orchestrator */
int main() {
    int *array1 = malloc(N * sizeof(int));
    int *array2 = malloc(M * sizeof(int));
    int *array3 = malloc(N * sizeof(int));
    
    /* Initialize with pseudo-random values */
    srand(42);
    for (int i = 0; i < N; i++) {
        array1[i] = rand() % 10000;
        array3[i] = rand() % 10000;
    }
    for (int i = 0; i < M; i++) {
        array2[i] = rand() % 10000;
    }
    
    clock_t start, end;
    double total_time = 0;
    long long checksum = 0;
    
    /* Warm-up loop to trigger optimization heuristics */
    printf("Warm-up phase...\n");
    for (int iter = 0; iter < ITERS / 10; iter++) {
        complex_scheduler_stress(array1, N, iter);
        vectorizable_loop(array1, array3, array2, M);
        switch_in_loop(array2, M);
    }
    
    /* Main timed execution */
    printf("Main execution phase...\n");
    start = clock();
    
    for (int iter = 0; iter < ITERS; iter++) {
        /* Vary parameters to trigger different scheduling paths */
        int size = (iter % 2 == 0) ? N : M;
        int *arr = (iter % 2 == 0) ? array1 : array2;
        
        complex_scheduler_stress(arr, size, iter);
        
        if (iter % 3 == 0) {
            vectorizable_loop(array1, array3, array2, M);
        }
        
        if (iter % 4 == 0) {
            switch_in_loop(array3, N);
        }
        
        /* Mix in some OpenMP parallel regions */
        #pragma omp parallel for
        for (int i = 0; i < N; i++) {
            array1[i] = scramble_bits(array1[i] ^ array3[i]);
        }
    }
    
    end = clock();
    total_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    /* Compute final checksum for verification */
    for (int i = 0; i < N; i++) {
        checksum += array1[i];
        checksum += array3[i];
    }
    for (int i = 0; i < M; i++) {
        checksum += array2[i];
    }
    
    printf("Execution time: %.3f seconds\n", total_time);
    printf("Final checksum: %lld\n", checksum);
    printf("Expected range: 0 to %lld\n", (long long)N * 20000LL + (long long)M * 10000LL);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
