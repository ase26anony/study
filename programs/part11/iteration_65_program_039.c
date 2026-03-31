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
        a[i + 2] = a[i] * b[i];
    }
    
    /* Flow dependence with distance 3, different data type */
    volatile float *vf = (volatile float *)a;
    for (int i = 0; i < n - 3; i++) {
        vf[i + 3] = vf[i] + 1.5f;
    }
}

/* Pattern 2: Multiple dependence types in single loop */
__attribute__((always_inline))
inline void pattern2_mixed_dependences(int *a, int *b, int *c, int n) {
    int t;
    /* Contains flow (RAW), anti (WAR), and output (WAW) dependences */
    for (int i = 1; i < n; i++) {
        t = a[i - 1];           /* Flow: read a[i-1] from previous iteration */
        a[i] = t + b[i];        /* Anti: t read before being overwritten next iteration */
        c[i] = c[i - 1] * 2;    /* Flow: c[i-1] -> c[i] */
        c[i - 1] = i;           /* Output: c[i-1] written twice (with i-1 offset) */
    }
}

/* Pattern 3: Indirect addressing with potential aliasing */
void pattern3_indirect_addressing(int *p, int *q, int n) {
    /* No restrict - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];
    }
}

/* Pattern 3b: With restrict to contrast */
void pattern3b_restrict(int *__restrict p, int *__restrict q, int n) {
    /* Restrict allows more optimization */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 3) % n];
    }
}

/* Pattern 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
inline void pattern4_nested_loops(int arr[M][N]) {
    /* Inner loop has distance-2 flow dependence */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + 1;
        }
    }
}

/* Pattern 5: Volatile memory dependencies */
void pattern5_volatile_deps(void) {
    volatile int v[100];
    volatile int result = 0;
    
    /* Simple volatile flow dependence */
    for (int i = 1; i < 100; i++) {
        v[i] = v[i - 1] + i;
    }
    
    /* More complex volatile pattern */
    for (int i = 2; i < 98; i++) {
        v[i + 2] = v[i - 2] * v[i + 1];
    }
}

/* Pattern 6: Complex pointer arithmetic with potential WAR */
void pattern6_pointer_arithmetic(int *base, int n) {
    int *p = base;
    int *q = base + n/2;
    
    for (int i = 0; i < n/2; i++) {
        /* WAR: q[i] read, then p[i] written (aliasing possible) */
        int temp = q[i];
        p[i] = p[i] * 2 + temp;
        /* Flow: p[i] to q[i] with distance 0 in same iteration */
        q[i] = p[i] - 1;
    }
}

/* Pattern 7: Double type for different data_type field */
void pattern7_double_deps(double *d1, double *d2, int n) {
    for (int i = 1; i < n; i++) {
        d1[i] = d1[i - 1] * d2[i];
    }
    
    /* Output dependence */
    for (int i = 0; i < n - 1; i++) {
        d2[i] = d1[i] + d2[i + 1];
        d2[i] = d2[i] * 0.5;  /* WAW on d2[i] */
    }
}

/* Pattern 8: Mixed distances in same loop */
void pattern8_mixed_distances(int *a, int *b, int n) {
    for (int i = 4; i < n; i++) {
        /* Multiple flow dependences with different distances */
        a[i] = a[i - 1] + a[i - 2] + a[i - 4];
        b[i] = b[i - 3] * 2;
    }
}

int main(void) {
    /* Initialize with deterministic but "random-looking" data */
    srand(42);
    
    /* Allocate and initialize arrays */
    int *a = (int *)malloc(SIZE * sizeof(int));
    int *b = (int *)malloc(SIZE * sizeof(int));
    int *c = (int *)malloc(SIZE * sizeof(int));
    double *d1 = (double *)malloc(SIZE * sizeof(double));
    double *d2 = (double *)malloc(SIZE * sizeof(double));
    int arr[M][N];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        d1[i] = (double)(rand() % 100) / 10.0;
        d2[i] = (double)(rand() % 100) / 10.0;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = rand() % 100;
        }
    }
    
    volatile int checksum = 0;
    
    /* Execute patterns to create various DDG edges */
    pattern1_flow_dependence(a, b, SIZE);
    checksum += a[SIZE/2] + b[SIZE/2];
    
    pattern2_mixed_dependences(a, b, c, SIZE);
    checksum += a[SIZE/3] + c[SIZE/3];
    
    pattern3_indirect_addressing(a, b, SIZE);
    checksum += a[SIZE/4];
    
    pattern3b_restrict(c, a, SIZE);
    checksum += c[SIZE/4];
    
    pattern4_nested_loops(arr);
    checksum += arr[M/2][N/2];
    
    pattern5_volatile_deps();
    checksum += 1;  /* Dummy checksum update */
    
    pattern6_pointer_arithmetic(a, SIZE);
    checksum += a[SIZE/2];
    
    pattern7_double_deps(d1, d2, SIZE);
    checksum += (int)(d1[SIZE/2] + d2[SIZE/2]);
    
    pattern8_mixed_distances(b, c, SIZE);
    checksum += b[SIZE/2] + c[SIZE/2];
    
    /* Final computation using all results */
    int final_result = 0;
    for (int i = 0; i < SIZE; i += 16) {
        final_result += a[i] + b[i] + c[i] + (int)d1[i] + (int)d2[i];
    }
    for (int i = 0; i < M; i += 4) {
        for (int j = 0; j < N; j += 8) {
            final_result += arr[i][j];
        }
    }
    
    final_result += checksum;
    
    printf("Result: %d\n", final_result);
    
    free(a);
    free(b);
    free(c);
    free(d1);
    free(d2);
    
    return 0;
}
