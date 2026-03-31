/* ddg_test.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Complex function with multiple dependency patterns */
int compute(int *a, int *b, int *c, int n) {
    int i, j;
    int sum = 0;
    int temp_reg = 0;
    int scalar_x = 1, scalar_y = 2;
    
    /* Outer loop with loop-carried dependencies */
    for (i = 1; i < n; i++) {
        /* FLOW (RAW) dependency: a[i] depends on a[i-1] */
        a[i] = a[i-1] + b[i];
        
        /* Control flow creates basic block boundaries */
        if (i % 2 == 0) {
            /* ANTI (WAR) dependency: c[i] read before write */
            temp_reg = c[i] * scalar_x;
            
            /* OUTPUT (WAW) dependency: c[i] written twice */
            c[i] = temp_reg + scalar_y;
            c[i] = c[i] * 2;  // Second write to c[i]
            
            /* Register dependencies */
            scalar_x = scalar_x + 1;
        } else {
            /* Different dependency pattern in else branch */
            /* FLOW dependency through memory */
            b[i] = a[i] + c[i-1];
            
            /* ANTI dependency with scalar */
            scalar_y = scalar_x;
            scalar_x = i * 3;
        }
        
        /* Mixed memory and register operations */
        /* FLOW: temp_reg depends on previous assignment */
        temp_reg = temp_reg + a[i];
        
        /* OUTPUT dependency in scalar */
        scalar_x = scalar_y + 1;
        scalar_x = scalar_x * 2;  // Second write to scalar_x
        
        /* Reduction for side effect */
        sum += a[i] + b[i] + c[i];
    }
    
    /* Nested loop with different distance */
    for (i = 0; i < M; i++) {
        for (j = 1; j < 8; j++) {
            /* Loop-carried with distance > 1 */
            if (j > 1) {
                /* FLOW with distance 1 in inner loop */
                c[i*8 + j] = c[i*8 + j-1] + b[i];
                
                /* ANTI across iterations */
                temp_reg = a[i*8 + j];
                a[i*8 + j] = temp_reg * j;
            }
            
            /* OUTPUT dependency in inner loop */
            b[i] = i + j;
            b[i] = b[i] * 3;  // Second write
            
            sum += c[i*8 + j];
        }
    }
    
    return sum;
}

/* Another function with pointer aliasing potential */
void transform(int *src, int *dst, int n) {
    int i;
    
    /* Loop with potential pointer aliasing */
    for (i = 1; i < n; i++) {
        /* Complex dependencies when src and dst may alias */
        dst[i] = src[i-1] + dst[i-1];
        src[i] = dst[i] * 2;
        
        /* Self-dependency */
        dst[i] = dst[i] + 1;
    }
}

int main() {
    int *array_a, *array_b, *array_c;
    int result1, result2;
    
    /* Allocate and initialize arrays */
    array_a = (int*)malloc(N * sizeof(int));
    array_b = (int*)malloc(N * sizeof(int));
    array_c = (int*)malloc(N * sizeof(int));
    
    if (!array_a || !array_b || !array_c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        array_a[i] = i;
        array_b[i] = i * 2;
        array_c[i] = i * 3;
    }
    
    /* Call functions that create complex dependencies */
    result1 = compute(array_a, array_b, array_c, N);
    
    /* Transform with potential aliasing */
    transform(array_a, array_b, N/2);
    
    /* Second computation */
    result2 = compute(array_b, array_c, array_a, N/2);
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    
    return 0;
}
