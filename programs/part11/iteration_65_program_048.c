#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100
#define M 50

/* Pattern 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
static inline void pattern1_flow_distance(int *a, int *b, int n) {
    /* Flow dependence with distance 2 */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i];
    }
    
    /* Flow dependence with distance 3, different data type */
    volatile float *vf = (volatile float *)a;
    for (int i = 0; i < n - 3; i++) {
        vf[i + 3] = vf[i] + 0.5f;
    }
}

/* Pattern 2: Multiple dependence types in one loop */
__attribute__((always_inline))
static inline int pattern2_mixed_deps(int *a, int *b, int *c, int n) {
    int t = 0;
    int sum = 0;
    
    /* Contains flow (RAW), anti (WAR), and output (WAW) dependences */
    for (int i = 1; i < n; i++) {
        t = a[i - 1];           /* Flow: read a[i-1] from previous iteration */
        a[i] = t + b[i];        /* Anti: t read here, written above */
        c[i] = c[i - 1] * 2;    /* Flow: c[i-1] -> c[i] */
        c[i] = c[i] + 1;        /* Output: c[i] written twice */
        sum += c[i];
    }
    return sum;
}

/* Pattern 3: Pointer aliasing with restrict and without */
static void pattern3_aliasing_no_restrict(int *p, int *q, int n) {
    /* No restrict - compiler must assume p and q may alias */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];  /* Potential flow dependence */
    }
}

static void pattern3_aliasing_restrict(int *__restrict p, int *__restrict q, int n) {
    /* With restrict - compiler knows p and q don't alias */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + 1;  /* No dependence on p */
    }
}

/* Pattern 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
static inline void pattern4_nested(int arr[M][N]) {
    /* Inner loop has carried dependence with distance 2 */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + arr[i][j - 1];
        }
    }
}

/* Pattern 5: Volatile forcing memory dependences */
static void pattern5_volatile_deps(void) {
    volatile int varr[100];
    volatile int vsum = 0;
    
    /* Simple flow dependence through volatile */
    for (int i = 1; i < 100; i++) {
        varr[i] = varr[i - 1] + i;
    }
    
    /* Anti-dependence with volatile */
    for (int i = 0; i < 99; i++) {
        int temp = varr[i];     /* Read */
        varr[i] = varr[i + 1];  /* Write to same location */
        vsum += temp;
    }
    
    /* Use vsum to prevent dead code elimination */
    printf("Volatile sum: %d\n", (int)vsum);
}

/* Pattern 6: Complex addressing with multiple arrays */
static int pattern6_complex_addressing(int *a, int *b, int *c, int n) {
    int sum = 0;
    
    /* Multiple interleaved dependences */
    for (int i = 2; i < n - 2; i++) {
        /* Flow: a[i-2] -> a[i] (distance 2) */
        a[i] = a[i - 2] + b[i];
        
        /* Anti: b[i] read above, written here */
        b[i] = c[i] * 2;
        
        /* Output: c[i] written twice */
        c[i] = a[i] + b[i];
        c[i] = c[i] * 3;
        
        /* Flow: c[i-1] -> sum (distance 1) */
        sum += c[i - 1];
    }
    return sum;
}

/* Pattern 7: While loop with carried dependence */
static void pattern7_while_loop(int *arr, int n) {
    int i = 1;
    while (i < n) {
        arr[i] = arr[i - 1] + i;
        i++;
    }
}

/* Pattern 8: Do-while loop for different control flow */
static void pattern8_do_while(double *darr, int n) {
    int i = 1;
    if (n > 1) {
        do {
            darr[i] = darr[i - 1] * 1.5;
            i++;
        } while (i < n);
    }
}

int main(void) {
    /* Initialize with different patterns to avoid constant propagation */
    int *a = malloc(SIZE * sizeof(int));
    int *b = malloc(SIZE * sizeof(int));
    int *c = malloc(SIZE * sizeof(int));
    int arr2d[M][N];
    
    srand(time(NULL));
    
    /* Initialize arrays with semi-random data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr2d[i][j] = rand() % 100;
        }
    }
    
    volatile int checksum = 0;
    
    /* Execute all patterns to trigger different DDG edge creations */
    pattern1_flow_distance(a, b, SIZE);
    checksum += a[SIZE/2] + b[SIZE/2];
    
    checksum += pattern2_mixed_deps(a, b, c, SIZE);
    
    pattern3_aliasing_no_restrict(a, b, SIZE/2);
    checksum += a[SIZE/4];
    
    pattern3_aliasing_restrict(a + SIZE/2, b + SIZE/2, SIZE/4);
    checksum += a[SIZE/2 + SIZE/8];
    
    pattern4_nested(arr2d);
    checksum += arr2d[M/2][N/2];
    
    pattern5_volatile_deps();
    
    checksum += pattern6_complex_addressing(a, b, c, SIZE);
    
    pattern7_while_loop(a, SIZE);
    checksum += a[SIZE-1];
    
    double *darr = (double *)malloc(SIZE * sizeof(double));
    for (int i = 0; i < SIZE; i++) {
        darr[i] = (double)(rand() % 100) / 10.0;
    }
    pattern8_do_while(darr, SIZE);
    checksum += (int)darr[SIZE-1];
    
    /* Final computation to ensure all results are used */
    int final_result = 0;
    for (int i = 0; i < SIZE; i += 16) {
        final_result += a[i] + b[i] + c[i];
    }
    final_result += checksum;
    
    printf("Final result: %d\n", final_result);
    
    free(a);
    free(b);
    free(c);
    free(darr);
    
    return 0;
}
