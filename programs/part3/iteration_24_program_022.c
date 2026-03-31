/* ddg_coverage.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Complex function with various dependency patterns */
int compute(int *a, int *b, int *c, int n) {
    int i, j;
    int sum = 0;
    int temp_reg = 0;
    int accum = 0;
    
    /* Outer loop with loop-carried dependencies */
    for (i = 1; i < n; i++) {
        /* FLOW (RAW) dependency: a[i] depends on a[i-1] */
        a[i] = a[i-1] + b[i];
        
        /* Control flow creates basic block boundaries */
        if (i % 2 == 0) {
            /* ANTI (WAR) dependency: temp_reg read before write */
            accum = temp_reg + c[i];
            
            /* OUTPUT (WAW) dependency: temp_reg written twice */
            temp_reg = accum * 2;
            temp_reg = temp_reg + 1;  /* Second write to same variable */
            
            /* Memory anti-dependency */
            int old_val = b[i];
            b[i] = accum + old_val;
        } else {
            /* Different path with register dependencies */
            int local_var = a[i] * 3;
            
            /* Flow dependency through memory */
            c[i] = local_var + b[i-1];
            
            /* Register output dependency */
            temp_reg = local_var;
        }
        
        /* Cross-iteration dependency with distance > 0 */
        if (i > 3) {
            /* Flow dependency with distance 2 */
            a[i] += c[i-2];
        }
        
        /* Reduction to prevent elimination */
        sum += a[i] + temp_reg;
    }
    
    /* Nested loop for additional complexity */
    for (i = 0; i < n/2; i++) {
        for (j = 1; j < M; j++) {
            /* Complex memory access pattern */
            int idx = (i * M + j) % n;
            
            /* Multiple dependencies in inner loop */
            b[idx] = a[idx] + b[idx-1];  /* Flow + anti */
            a[idx] = b[idx] * 2;         /* Output */
            
            /* Register pressure */
            int r1 = b[idx];
            int r2 = a[idx];
            int r3 = r1 + r2;
            int r4 = r3 * r2;
            accum += r4;
        }
    }
    
    return sum + accum;
}

/* Another function with pointer aliasing potential */
void transform(int *x, int *y, int *z, int n) {
    int i;
    
    /* Unrolled loop with dependencies */
    for (i = 0; i < n-4; i += 4) {
        /* Independent chains with dependencies */
        x[i] = y[i] + z[i];
        x[i+1] = x[i] * 2;           /* Flow dep */
        y[i+1] = x[i+1] - y[i];      /* Anti dep through y[i] */
        z[i+1] = y[i+1] + z[i];      /* Flow + anti */
        
        /* Output dependency in same iteration */
        x[i] = x[i] + 1;             /* WAW on x[i] */
    }
}

int main() {
    /* Allocate and initialize arrays */
    int *array1 = (int*)malloc(N * sizeof(int));
    int *array2 = (int*)malloc(N * sizeof(int));
    int *array3 = (int*)malloc(N * sizeof(int));
    
    if (!array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        array1[i] = i;
        array2[i] = i * 2;
        array3[i] = i * 3;
    }
    
    /* Call functions to create dependency graphs */
    int result1 = compute(array1, array2, array3, N);
    transform(array1, array2, array3, N);
    
    /* Final computation mixing all arrays */
    int final_result = 0;
    for (int i = 1; i < N; i++) {
        /* Complex web of dependencies */
        int t1 = array1[i-1];
        int t2 = array2[i];
        array3[i] = t1 + t2;          /* Flow from array1[i-1] */
        array1[i] = array3[i] * 2;    /* Flow from array3[i] */
        array2[i] = array1[i] - t2;   /* Anti on t2 (array2[i]) */
        
        final_result += array1[i] + array2[i] + array3[i];
    }
    
    printf("Result: %d\n", final_result + result1);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
