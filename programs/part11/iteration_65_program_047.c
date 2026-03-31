#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100
#define M 50

/* Pattern 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
inline void pattern1_flow_dependence(int *a, int *b, int n) {
    /* Flow dependence with distance 2 */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i] + 1;
    }
    
    /* Anti-dependence with distance 1 */
    for (int i = 1; i < n; i++) {
        int temp = a[i - 1];
        a[i - 1] = b[i];
        b[i] = temp + i;
    }
}

/* Pattern 2: Multiple dependence types in single loop */
__attribute__((always_inline))
inline void pattern2_mixed_dependences(int *a, int *b, int *c, int n) {
    int t;
    /* Contains flow, anti, and output dependences */
    for (int i = 1; i < n; i++) {
        t = a[i - 1];           /* Flow: read a[i-1] */
        a[i] = t + b[i];        /* Flow: use t, Anti: write a[i] after read */
        c[i] = c[i - 1] * 2;    /* Flow: c[i-1] -> c[i] */
        b[i - 1] = c[i] + 1;    /* Output: write b[i-1] */
    }
}

/* Pattern 3: Pointer aliasing with potential self-overlap */
void pattern3_pointer_aliasing(int *p, int *q, int n) {
    /* No restrict - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n] * 2;
    }
}

/* Pattern 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
inline void pattern4_nested_loops(int arr[M][N]) {
    /* Inner loop has distance 2 flow dependence */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 4
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + arr[i][j - 1] + i + j;
        }
    }
}

/* Pattern 5: Volatile memory accesses to force dependences */
void pattern5_volatile_access(volatile int *v, int n) {
    /* Strong memory dependence enforced by volatile */
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + v[i] * 3;
    }
}

/* Pattern 6: Different data types for varied data_type field */
__attribute__((always_inline))
inline void pattern6_mixed_data_types(float *fa, double *da, int *ia, int n) {
    /* Float array with distance 1 flow */
    for (int i = 1; i < n; i++) {
        fa[i] = fa[i - 1] * 1.5f + ia[i];
    }
    
    /* Double array with distance 3 flow */
    for (int i = 3; i < n; i++) {
        da[i] = da[i - 3] * 2.0 + fa[i];
    }
}

/* Pattern 7: Complex index calculations */
void pattern7_complex_indices(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        int idx1 = (i * 3 + 7) % n;
        int idx2 = (i * 5 + 11) % n;
        int idx3 = (i * 13 + 17) % n;
        
        arr[idx1] = arr[idx2] + arr[idx3] * arr[i];
    }
}

/* Pattern 8: Loop with if-condition creating varying dependences */
void pattern8_conditional_dependence(int *a, int *b, int n) {
    for (int i = 2; i < n; i++) {
        if (i % 3 == 0) {
            a[i] = a[i - 2] + b[i];  /* Distance 2 flow */
        } else if (i % 3 == 1) {
            b[i] = a[i - 1] * 2;     /* Distance 1 flow */
            a[i] = b[i] + 1;         /* Anti-dependence */
        } else {
            a[i] = a[i - 1] + a[i];  /* Distance 1 flow + output */
        }
    }
}

int main() {
    /* Initialize with different patterns to avoid constant propagation */
    int *a = malloc(SIZE * sizeof(int));
    int *b = malloc(SIZE * sizeof(int));
    int *c = malloc(SIZE * sizeof(int));
    float *fa = malloc(SIZE * sizeof(float));
    double *da = malloc(SIZE * sizeof(double));
    volatile int *v = malloc(SIZE * sizeof(int));
    int arr[M][N];
    
    srand(time(NULL));
    
    /* Initialize arrays with semi-random data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        fa[i] = (float)(rand() % 100) / 10.0f;
        da[i] = (double)(rand() % 100) / 5.0;
        v[i] = rand() % 50;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = rand() % 100;
        }
    }
    
    volatile int checksum = 0;
    
    /* Execute all patterns to create various DDG edges */
    pattern1_flow_dependence(a, b, SIZE);
    pattern2_mixed_dependences(a, b, c, SIZE);
    pattern3_pointer_aliasing(a, b, SIZE);
    pattern4_nested_loops(arr);
    pattern5_volatile_access(v, SIZE);
    pattern6_mixed_data_types(fa, da, a, SIZE);
    pattern7_complex_indices(c, SIZE);
    pattern8_conditional_dependence(a, c, SIZE);
    
    /* Compute checksum to ensure all computations are live */
    for (int i = 0; i < SIZE; i++) {
        checksum += a[i] + b[i] + c[i] + (int)fa[i] + (int)da[i] + v[i];
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            checksum += arr[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(a);
    free(b);
    free(c);
    free(fa);
    free(da);
    free((void*)v);
    
    return 0;
}
