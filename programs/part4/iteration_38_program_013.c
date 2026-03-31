/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Single accumulator with loop-carried dependency */
int loop_carried_accumulator(int n, int* restrict a, int* restrict b, int* restrict c) {
    volatile int sum = 0;  /* volatile prevents optimization of carried dependency */
    for (int i = 1; i < n; i++) {
        /* Multiple operations with distance-1 dependencies */
        a[i] = a[i-1] * b[i] + c[i];  /* Distance-1: a[i-1] used in iteration i */
        sum = sum + a[i] * 3;          /* Distance-1: sum carried across iterations */
        /* Additional operations to create scheduling complexity */
        b[i] = (b[i] << 2) | (c[i] & 0xF);
        c[i] = c[i-1] + (i % 256);     /* Another distance-1 dependency */
    }
    /* Memory barrier to preserve dependencies */
    asm volatile("" : : "r"(sum) : "memory");
    return sum;
}

/* Function 2: Multiple interleaved accumulators */
int multiple_accumulators(int n, int* restrict x, int* restrict y, int* restrict z) {
    volatile int sum1 = 0, sum2 = 0;
    for (int i = 1; i < n; i++) {
        /* Two separate distance-1 dependencies */
        x[i] = x[i-1] + y[i] * z[i];   /* Distance-1: x[i-1] */
        sum1 = sum1 + x[i];            /* Distance-1: sum1 */
        
        y[i] = y[i-1] ^ (z[i] << 1);   /* Distance-1: y[i-1] */
        sum2 = sum2 ^ y[i];            /* Distance-1: sum2 */
        
        /* Complex operation to increase ILP requirements */
        z[i] = (z[i-1] * 7 + z[i]) & 0xFF;  /* Distance-1: z[i-1] */
    }
    asm volatile("" : : "r"(sum1), "r"(sum2) : "memory");
    return sum1 + sum2;
}

/* Function 3: Nested loops with inner loop carried dependency */
int nested_loop_dependency(int n, int m, int* restrict mat, int* restrict vec) {
    volatile int total = 0;
    /* Outer loop - will likely be unrolled */
    for (int i = 0; i < n; i += 2) {
        /* Inner loop with carried dependency */
        int acc = 0;
        for (int j = 1; j < m; j++) {
            /* Distance-1 dependency in inner loop */
            acc = acc + mat[i * m + j] * vec[j-1];  /* vec[j-1] creates distance-1 */
            mat[i * m + j] = (mat[i * m + j-1] + acc) >> 1;  /* mat[][j-1] distance-1 */
        }
        total += acc;
        /* Prevent outer loop from being optimized away */
        asm volatile("" : : "r"(acc) : "memory");
    }
    return total;
}

/* Function 4: Complex recurrence with multiple dependencies */
int complex_recurrence(int n, int* restrict p, int* restrict q, int* restrict r) {
    volatile int state1 = p[0], state2 = q[0];
    for (int i = 1; i < n; i++) {
        /* Multiple interleaved distance-1 dependencies */
        int temp1 = state1 * 3 + p[i-1];      /* p[i-1] distance-1 */
        int temp2 = state2 ^ q[i-1];          /* q[i-1] distance-1 */
        
        p[i] = (temp1 + r[i]) & 0xFFFF;
        q[i] = (temp2 - r[i-1]) & 0xFFFF;     /* r[i-1] distance-1 */
        
        state1 = state1 + p[i] * 5;           /* state1 distance-1 */
        state2 = state2 ^ q[i] * 7;           /* state2 distance-1 */
        
        r[i] = r[i-1] + (i & 0xFF);           /* r[i-1] distance-1 */
    }
    asm volatile("" : : "r"(state1), "r"(state2) : "memory");
    return state1 + state2;
}

/* Function 5: Loop with unknown trip count (prevents unrolling) */
int variable_trip_count(int n, int* restrict arr1, int* restrict arr2) {
    volatile int result = 0;
    /* n is unknown at compile time, forces modulo scheduling analysis */
    for (int i = 1; i < n; i++) {
        /* Classic distance-1 pattern */
        arr1[i] = arr1[i-1] * 2 - arr2[i];    /* arr1[i-1] distance-1 */
        arr2[i] = arr2[i-1] + arr1[i] / 3;    /* arr2[i-1] distance-1 */
        result += arr1[i] ^ arr2[i];
    }
    asm volatile("" : : "r"(result) : "memory");
    return result;
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int n, int* a, int* b, int* c) {
    for (int i = 0; i < n; i++) {
        a[i] = (i * 13) & 0xFF;
        b[i] = (i * 17) & 0xFF;
        c[i] = (i * 19) & 0xFF;
    }
}

int main() {
    const int N = 1024;
    const int M = 512;
    
    /* Allocate and initialize arrays */
    int* a = (int*)malloc(N * sizeof(int));
    int* b = (int*)malloc(N * sizeof(int));
    int* c = (int*)malloc(N * sizeof(int));
    int* x = (int*)malloc(N * sizeof(int));
    int* y = (int*)malloc(N * sizeof(int));
    int* z = (int*)malloc(N * sizeof(int));
    int* mat = (int*)malloc(N * M * sizeof(int));
    int* vec = (int*)malloc(M * sizeof(int));
    int* p = (int*)malloc(N * sizeof(int));
    int* q = (int*)malloc(N * sizeof(int));
    int* r = (int*)malloc(N * sizeof(int));
    int* arr1 = (int*)malloc(N * sizeof(int));
    int* arr2 = (int*)malloc(N * sizeof(int));
    
    if (!a || !b || !c || !x || !y || !z || !mat || !vec || 
        !p || !q || !r || !arr1 || !arr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize all arrays */
    init_arrays(N, a, b, c);
    init_arrays(N, x, y, z);
    init_arrays(N, p, q, r);
    init_arrays(N, arr1, arr2);
    
    for (int i = 0; i < M; i++) {
        vec[i] = (i * 23) & 0xFF;
    }
    for (int i = 0; i < N * M; i++) {
        mat[i] = (i * 29) & 0xFF;
    }
    
    /* Call all test functions to ensure they're compiled */
    int total = 0;
    
    total += loop_carried_accumulator(N, a, b, c);
    total += multiple_accumulators(N, x, y, z);
    total += nested_loop_dependency(16, M, mat, vec);  /* Small outer loop */
    total += complex_recurrence(N, p, q, r);
    
    /* Variable trip count - use different values */
    total += variable_trip_count(N/2, arr1, arr2);
    total += variable_trip_count(N/4, arr1 + N/2, arr2 + N/2);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(x); free(y); free(z);
    free(mat); free(vec);
    free(p); free(q); free(r);
    free(arr1); free(arr2);
    
    return 0;
}
