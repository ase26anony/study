#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 256
#define M 128

/* Pattern 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
static inline void pattern1_flow_dependence(int *a, int *b, int n) {
    /* Flow dependence with distance 2 */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i];  /* RAW: a[i] read, a[i+2] written, distance=2 */
    }
}

/* Pattern 2: Mixed dependence types in single loop */
__attribute__((always_inline))
static inline void pattern2_mixed_dependences(int *a, int *b, int *c, int n) {
    int t;
    for (int i = 1; i < n; i++) {
        t = a[i - 1];           /* Flow: read a[i-1] */
        a[i] = t + b[i];        /* Flow: write a[i] after reading t */
        c[i] = c[i - 1] * 2;    /* Flow: read c[i-1], write c[i] */
        b[i] = b[i - 1] + 1;    /* Anti: read b[i-1], write b[i] (WAR) */
    }
}

/* Pattern 3: Output dependence (WAW) */
__attribute__((always_inline))
static inline void pattern3_output_dependence(double *arr, int n) {
    for (int i = 2; i < n; i++) {
        arr[i] = arr[i - 1] * 1.5;  /* Flow */
        arr[i] = arr[i] * 2.0;      /* WAW: same location written twice */
    }
}

/* Pattern 4: Pointer aliasing with potential self-overlap */
static void pattern4_pointer_aliasing(int *p, int *q, int n) {
    /* No restrict - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];  /* Potential flow dependence if p overlaps */
    }
}

/* Pattern 5: Nested loop with inner loop carried dependence */
__attribute__((always_inline))
static inline void pattern5_nested_loop(int arr[M][N]) {
    #pragma GCC unroll 2
    for (int i = 0; i < M; i++) {
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + 1;  /* Flow, distance=2 in inner loop */
        }
    }
}

/* Pattern 6: Volatile memory dependence */
static void pattern6_volatile_dependence(volatile int *v, int n) {
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + i;  /* Flow through volatile, must create DDG edge */
    }
}

/* Pattern 7: Anti-dependence with register pressure */
__attribute__((always_inline))
static inline void pattern7_anti_dependence(float *fa, float *fb, int n) {
    float temp;
    for (int i = 1; i < n; i++) {
        temp = fa[i];      /* Read fa[i] */
        fa[i - 1] = temp * fb[i];  /* Write fa[i-1] - anti to next iteration's read */
        fb[i] = temp + 1.0f;       /* Write fb[i] - output if unrolled */
    }
}

/* Pattern 8: Complex index with multiple distances */
static void pattern8_complex_indices(int *data, int n) {
    for (int i = 3; i < n; i++) {
        data[i] = data[i - 1] +      /* distance 1 */
                  data[i - 2] * 2 +  /* distance 2 */
                  data[i - 3] / 3;   /* distance 3 */
    }
}

/* Pattern 9: Mixed data types to test data_type field */
static void pattern9_mixed_types(int *idata, float *fdata, double *ddata, int n) {
    for (int i = 1; i < n; i++) {
        idata[i] = (int)(fdata[i - 1] * 2.0f);  /* float to int flow */
        fdata[i] = (float)(ddata[i - 1] * 0.5); /* double to float flow */
        ddata[i] = idata[i - 1] * 3.14159;      /* int to double flow */
    }
}

/* Pattern 10: Loop with if condition creating varying dependences */
static void pattern10_conditional_dep(int *a, int *b, int n) {
    for (int i = 2; i < n; i++) {
        if (i % 3 == 0) {
            a[i] = a[i - 2] + b[i];  /* distance 2 */
        } else {
            a[i] = a[i - 1] * b[i];  /* distance 1 */
        }
        b[i] = a[i - 1] + i;         /* anti-dependence */
    }
}

int main(void) {
    /* Initialize with deterministic but varied data */
    srand(42);
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    int *c = (int*)malloc(SIZE * sizeof(int));
    double *darr = (double*)malloc(SIZE * sizeof(double));
    float *fa = (float*)malloc(SIZE * sizeof(float));
    float *fb = (float*)malloc(SIZE * sizeof(float));
    double *ddata = (double*)malloc(SIZE * sizeof(double));
    volatile int *varr = (volatile int*)malloc(SIZE * sizeof(int));
    int arr2d[M][N];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        darr[i] = (double)(rand() % 100) / 10.0;
        fa[i] = (float)(rand() % 100) / 5.0f;
        fb[i] = (float)(rand() % 100) / 5.0f;
        ddata[i] = (double)(rand() % 100) / 3.0;
        varr[i] = rand() % 50;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr2d[i][j] = rand() % 100;
        }
    }
    
    /* Volatile checksum to ensure computations aren't optimized away */
    volatile int checksum = 0;
    
    /* Execute all patterns to create various DDG edges */
    pattern1_flow_dependence(a, b, SIZE);
    pattern2_mixed_dependences(a, b, c, SIZE);
    pattern3_output_dependence(darr, SIZE);
    pattern4_pointer_aliasing(a, b, SIZE);
    pattern5_nested_loop(arr2d);
    pattern6_volatile_dependence(varr, SIZE);
    pattern7_anti_dependence(fa, fb, SIZE);
    pattern8_complex_indices(c, SIZE);
    pattern9_mixed_types(a, fa, ddata, SIZE);
    pattern10_conditional_dep(a, c, SIZE);
    
    /* Accumulate results into checksum */
    for (int i = 0; i < SIZE; i++) {
        checksum += a[i] + b[i] + c[i] + (int)darr[i] + (int)fa[i] + 
                   (int)fb[i] + (int)ddata[i] + varr[i];
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            checksum += arr2d[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(darr);
    free(fa); free(fb); free(ddata);
    free((void*)varr);
    
    return 0;
}
