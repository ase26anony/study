/* test1.c - Floating-point intensive computations with nested loops */
#include <math.h>
#include <stdlib.h>

#define SIZE 256
#define ITERS 1000

/* Function with tight floating-point loop - creates scheduling pressure */
void compute_kernel_fp(float *restrict a, float *restrict b, float *restrict c, int n) {
    int i, j, k;
    
    /* Triple nested loop with FP operations */
    for (i = 0; i < n; i++) {
        float acc = 0.0f;
        for (j = 0; j < n; j++) {
            float temp = a[i * n + j];
            for (k = 0; k < n; k++) {
                /* Mixed FP operations that create dependencies */
                temp = temp * b[j * n + k] + c[k * n + i];
                temp = fabsf(temp) + 0.5f;
            }
            acc += temp;
        }
        a[i * n + i] = acc / n;
    }
}

/* Another function with different pattern */
void vector_operations(float *v1, float *v2, float *v3, int len) {
    int i;
    volatile int counter = 0; /* Prevent optimization */
    
    /* Loop with conditional FP operations */
    for (i = 0; i < len; i++) {
        if (i % 3 == 0) {
            v1[i] = v2[i] * v3[i] + sinf(v1[i]);
        } else if (i % 3 == 1) {
            v1[i] = sqrtf(fabsf(v2[i] - v3[i]));
        } else {
            v1[i] = (v2[i] + v3[i]) * (v2[i] - v3[i]);
        }
        
        /* Create cross-iteration dependency */
        if (i > 0) {
            v1[i] += v1[i-1] * 0.1f;
        }
        
        counter++;
    }
}

/* Main computation function */
void test1_main(void) {
    static float array1[SIZE * SIZE];
    static float array2[SIZE * SIZE];
    static float array3[SIZE * SIZE];
    float vec1[SIZE], vec2[SIZE], vec3[SIZE];
    int i;
    
    /* Initialize with non-zero values */
    for (i = 0; i < SIZE * SIZE; i++) {
        array1[i] = (i % 7) * 0.1f;
        array2[i] = (i % 11) * 0.2f;
        array3[i] = (i % 13) * 0.3f;
    }
    
    for (i = 0; i < SIZE; i++) {
        vec1[i] = i * 0.01f;
        vec2[i] = i * 0.02f;
        vec3[i] = i * 0.03f;
    }
    
    /* Call compute-intensive functions multiple times */
    for (i = 0; i < 5; i++) {
        compute_kernel_fp(array1, array2, array3, 16); /* Smaller size for speed */
        vector_operations(vec1, vec2, vec3, SIZE);
    }
}
