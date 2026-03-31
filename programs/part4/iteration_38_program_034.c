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
        b[i] = b[i-1] + sum;           /* Distance-1: b[i-1] used in iteration i */
        
        /* Add memory barrier to preserve dependencies */
        asm volatile("" : : "r"(sum) : "memory");
    }
    return sum;
}

/* Function 2: Multiple interleaved accumulators */
int multiple_accumulators(int n, int* restrict x, int* restrict y, int* restrict z) {
    volatile int acc1 = 0;
    volatile int acc2 = 0;
    
    for (int i = 1; i < n; i++) {
        /* Two separate distance-1 dependencies */
        x[i] = y[i-1] + z[i];      /* Distance-1: y[i-1] */
        acc1 = acc1 + x[i] * 2;    /* Distance-1: acc1 */
        
        y[i] = z[i-1] * 3;         /* Distance-1: z[i-1] */
        acc2 = acc2 + y[i] * 5;    /* Distance-1: acc2 */
        
        /* Cross-iteration dependency with stride */
        z[i] = x[i-1] + acc1 - acc2;  /* Distance-1: x[i-1] */
        
        asm volatile("" : : "r"(acc1), "r"(acc2) : "memory");
    }
    return acc1 + acc2;
}

/* Function 3: Nested loops with inner loop carried dependency */
int nested_loop_dependency(int n, int m, int* restrict mat, int* restrict vec) {
    volatile int total = 0;
    
    /* Outer loop - force unrolling candidate */
    for (int i = 0; i < n; i += 2) {
        /* Inner loop with carried dependency */
        int inner_acc = 0;
        for (int j = 1; j < m; j++) {
            /* Complex distance-1 pattern */
            mat[i*m + j] = mat[i*m + j-1] * vec[j] + inner_acc;
            inner_acc = inner_acc + mat[i*m + j] * 7;
            vec[j] = vec[j-1] + inner_acc;  /* Distance-1 in inner loop */
            
            asm volatile("" : : "r"(inner_acc) : "memory");
        }
        total += inner_acc;
    }
    return total;
}

/* Function 4: Loop with unknown trip count (parameter) */
int variable_trip_count(int start, int end, int* restrict data, int* restrict coeff) {
    volatile int result = 0;
    
    /* Loop count not known at compile time */
    for (int i = start + 1; i < end; i++) {
        /* Multiple distance-1 dependencies */
        data[i] = data[i-1] * coeff[i] + result;  /* Distance-1: data[i-1] */
        result = result + data[i] * coeff[i-1];    /* Distance-1: coeff[i-1] */
        coeff[i] = coeff[i-1] * 2 - result;       /* Distance-1: coeff[i-1] */
        
        /* Force dependency preservation */
        asm volatile("" : : "r"(result) : "memory");
    }
    return result;
}

/* Function 5: Complex recurrence with multiple uses */
int complex_recurrence(int n, int* restrict p, int* restrict q, int* restrict r) {
    volatile int s1 = 1, s2 = 2, s3 = 3;
    
    for (int i = 1; i < n; i++) {
        /* Chain of distance-1 dependencies */
        p[i] = p[i-1] + q[i-1] + s1;      /* Two distance-1 uses */
        s1 = s1 * p[i] + s2;
        
        q[i] = q[i-1] * r[i-1] - s2;      /* Two distance-1 uses */
        s2 = s2 + q[i] * s3;
        
        r[i] = r[i-1] + p[i-1] * s3;      /* Two distance-1 uses */
        s3 = s3 - r[i] + s1;
        
        /* Critical: This creates distance1_uses = true scenario */
        asm volatile("" : : "r"(s1), "r"(s2), "r"(s3) : "memory");
    }
    return s1 + s2 + s3;
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int n, int* a, int* b, int* c) {
    for (int i = 0; i < n; i++) {
        a[i] = (i * 17) % 100;
        b[i] = (i * 23) % 100;
        c[i] = (i * 31) % 100;
    }
}

int main() {
    const int N = 1024;
    const int M = 512;
    
    /* Allocate and initialize arrays */
    int* a = malloc(N * sizeof(int));
    int* b = malloc(N * sizeof(int));
    int* c = malloc(N * sizeof(int));
    int* x = malloc(N * sizeof(int));
    int* y = malloc(N * sizeof(int));
    int* z = malloc(N * sizeof(int));
    int* mat = malloc(N * M * sizeof(int));
    int* vec = malloc(M * sizeof(int));
    int* p = malloc(N * sizeof(int));
    int* q = malloc(N * sizeof(int));
    int* r = malloc(N * sizeof(int));
    
    if (!a || !b || !c || !x || !y || !z || !mat || !vec || !p || !q || !r) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize all arrays */
    init_arrays(N, a, b, c);
    init_arrays(N, x, y, z);
    init_arrays(N, p, q, r);
    for (int i = 0; i < M; i++) vec[i] = (i * 41) % 100;
    for (int i = 0; i < N * M; i++) mat[i] = (i * 53) % 100;
    
    /* Call all functions to ensure they're compiled */
    int result = 0;
    
    result += loop_carried_accumulator(N, a, b, c);
    result += multiple_accumulators(N, x, y, z);
    result += nested_loop_dependency(8, M, mat, vec);  /* Small outer loop */
    result += variable_trip_count(10, N-10, a, b);
    result += complex_recurrence(N, p, q, r);
    
    /* Print result to prevent dead code elimination */
    printf("Final checksum: %d\n", result);
    
    /* Free allocated memory */
    free(a); free(b); free(c);
    free(x); free(y); free(z);
    free(mat); free(vec);
    free(p); free(q); free(r);
    
    return 0;
}
