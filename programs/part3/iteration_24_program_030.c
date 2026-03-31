/* ddg_test.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Complex loop with various dependency patterns */
void process_data(int *a, int *b, int *c, int n) {
    int i, j;
    int temp_reg = 0;
    int accum = 0;
    
    /* Outer loop with loop-carried dependency */
    for (i = 1; i < n; i++) {
        /* FLOW (RAW) dependency: a[i] depends on a[i-1] */
        a[i] = a[i-1] + b[i];
        
        /* Conditional to create basic block boundaries */
        if (i % 3 == 0) {
            /* ANTI (WAR) dependency: temp_reg read before write */
            c[i] = temp_reg + i;
            temp_reg = a[i] * 2;  /* Write to temp_reg */
            
            /* OUTPUT (WAW) dependency on a[i] */
            a[i] = c[i] + 1;      /* Second write to a[i] */
        } else if (i % 3 == 1) {
            /* Another ANTI dependency pattern */
            accum = b[i] + accum; /* accum has loop-carried flow dep */
            a[i] = accum;
            
            /* MEM->REG and REG->MEM dependencies */
            temp_reg = c[i] * 3;  /* Read c[i] */
            c[i] = temp_reg - 1;  /* Write c[i] */
        } else {
            /* Mixed dependencies */
            int local_var = a[i] + c[i];  /* Flow from a[i], c[i] */
            b[i] = local_var * 2;         /* Flow to b[i] */
            a[i] = b[i-1] + local_var;    /* Anti with b[i], output on a[i] */
        }
        
        /* Additional scalar operation creating register dependencies */
        accum = (accum * 2) % 100;
    }
    
    /* Nested loop with different dependency distance */
    for (i = 0; i < n/2; i++) {
        for (j = 1; j < M; j++) {
            /* 2D loop-carried dependency with distance > 1 */
            if (j > 1) {
                /* Flow dependency with distance 2 in j dimension */
                c[i*M + j] = c[i*M + j-2] + a[i];
            }
            
            /* Anti dependency across loop iterations */
            temp_reg = b[i*M + j];
            b[i*M + j] = temp_reg + j;
            
            /* Control-dependent operation */
            if (temp_reg > 50) {
                a[i] = a[i] + 1;  /* Output dependency on a[i] */
            }
        }
    }
}

/* Helper with pointer aliasing to create memory ambiguity */
void process_with_aliasing(int * restrict x, int * restrict y, int *z, int n) {
    int i;
    
    /* z may alias with x or y, creating potential memory dependencies */
    for (i = 1; i < n; i++) {
        /* Flow dependencies */
        x[i] = x[i-1] + y[i];
        z[i] = x[i] * 2;
        
        /* Anti and output dependencies */
        int t = z[i];      /* Read z[i] */
        z[i] = t + i;      /* Write z[i] - output dependency */
        y[i] = z[i-1] + t; /* Flow from z[i-1], anti from t */
    }
}

int main() {
    /* Allocate and initialize arrays */
    int *array1 = (int*)malloc(N * M * sizeof(int));
    int *array2 = (int*)malloc(N * M * sizeof(int));
    int *array3 = (int*)malloc(N * M * sizeof(int));
    
    if (!array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-zero values */
    for (int i = 0; i < N * M; i++) {
        array1[i] = i % 100;
        array2[i] = (i * 3) % 100;
        array3[i] = (i * 7) % 100;
    }
    
    /* Process data to create dependencies */
    process_data(array1, array2, array3, N);
    
    /* Additional processing with potential aliasing */
    int *alias_test = array2 + 10; /* Create potential overlap */
    process_with_aliasing(array1, alias_test, array3, N-10);
    
    /* Final reduction to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < N * M; i++) {
        sum = (sum + array1[i] + array2[i] + array3[i]) % 1000000;
    }
    
    printf("Result: %d\n", sum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
