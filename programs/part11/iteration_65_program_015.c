#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define M 64
#define N 128

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
inline void test_flow_dependence(int *a, int *b, int n, volatile int *checksum) {
    /* Distance 2 flow dependence */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i];
        *checksum += a[i + 2];
    }
    
    /* Distance 1 flow dependence with different data type */
    volatile float *vf = (volatile float *)a;
    for (int i = 1; i < n; i++) {
        vf[i] = vf[i - 1] + 0.5f;
        *checksum += (int)vf[i];
    }
}

/* Test 2: Multiple dependence types in single loop */
__attribute__((always_inline))
inline void test_mixed_dependences(int *a, int *b, int *c, int n, volatile int *checksum) {
    int t;
    /* Contains flow (RAW), anti (WAR), and output (WAW) dependences */
    for (int i = 1; i < n; i++) {
        t = a[i - 1];           /* Flow: read a[i-1] */
        a[i] = t + b[i];        /* Flow: use t, Anti: write a[i] after read in next iter? */
        c[i] = c[i - 1] * 2;    /* Flow: read c[i-1], Output: write c[i] */
        *checksum += a[i] + c[i];
    }
}

/* Test 3: Pointer aliasing with potential self-overlap */
void test_pointer_aliasing(int *p, int *q, int n, volatile int *checksum) {
    /* No restrict - compiler must assume p and q may alias */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];  /* Potential flow dependence */
        *checksum += p[i];
    }
}

/* Test 3b: With restrict for comparison */
void test_restrict_pointers(int *__restrict__ rp, int *__restrict__ rq, int n, volatile int *checksum) {
    /* restrict allows more aggressive optimization */
    for (int i = 0; i < n; i++) {
        rp[i] = rq[i] + rp[(i * 3) % n];  /* Still potential self-dependence */
        *checksum += rp[i];
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
inline void test_nested_loops(int arr[M][N], volatile int *checksum) {
    /* Inner loop has distance 2 flow dependence */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + 1;
            *checksum += arr[i][j];
        }
    }
}

/* Test 5: Volatile memory accesses force dependences */
void test_volatile_access(volatile int *v, int n, volatile int *checksum) {
    /* Strong memory dependence enforced by volatile */
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + i;
        *checksum += v[i];
    }
}

/* Test 6: Complex loop with multiple arrays and indices */
void test_complex_dependences(int *a, int *b, int *c, int *d, int n, volatile int *checksum) {
    /* Multiple interleaved dependences of different distances */
    for (int i = 3; i < n; i++) {
        a[i] = b[i - 1] + c[i - 2];      /* Flow from b (dist 1), c (dist 2) */
        b[i] = a[i - 3] * d[i];          /* Flow from a (dist 3) */
        c[i] = a[i - 1] + b[i - 2];      /* Flow from a (dist 1), b (dist 2) */
        d[i] = c[i - 1] - a[i - 2];      /* Flow from c (dist 1), a (dist 2) */
        *checksum += a[i] + b[i] + c[i] + d[i];
    }
}

/* Test 7: Software pipelining candidate with reduction */
int test_reduction_loop(int *data, int n) {
    int sum = 0;
    /* Loop with carried dependence through reduction variable */
    for (int i = 0; i < n; i++) {
        sum += data[i] * data[i];
    }
    return sum;
}

/* Test 8: While loop with pointer arithmetic */
void test_while_loop(int *p, int *end, volatile int *checksum) {
    int *ptr = p;
    while (ptr < end) {
        *(ptr + 1) = *ptr + *(ptr - 1);  /* Flow dependences through pointers */
        *checksum += *ptr;
        ptr++;
    }
}

int main() {
    /* Initialize with deterministic but non-trivial pattern */
    srand(42);
    
    /* Allocate and initialize arrays */
    int *a = (int *)malloc(SIZE * sizeof(int));
    int *b = (int *)malloc(SIZE * sizeof(int));
    int *c = (int *)malloc(SIZE * sizeof(int));
    int *d = (int *)malloc(SIZE * sizeof(int));
    int *q = (int *)malloc(SIZE * sizeof(int));
    volatile int *v = (volatile int *)malloc(SIZE * sizeof(int));
    int arr[M][N];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        d[i] = rand() % 100;
        q[i] = rand() % 100;
        v[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = rand() % 100;
        }
    }
    
    volatile int checksum = 0;
    
    /* Execute all tests to trigger various DDG edge creations */
    test_flow_dependence(a, b, SIZE, &checksum);
    test_mixed_dependences(a, b, c, SIZE, &checksum);
    test_pointer_aliasing(a, q, SIZE, &checksum);
    test_restrict_pointers(b, q, SIZE, &checksum);
    test_nested_loops(arr, &checksum);
    test_volatile_access(v, SIZE, &checksum);
    test_complex_dependences(a, b, c, d, SIZE, &checksum);
    
    int reduction_result = test_reduction_loop(a, SIZE);
    checksum += reduction_result;
    
    test_while_loop(a, a + SIZE - 10, &checksum);
    
    /* Additional loop to encourage modulo scheduling */
    #pragma GCC unroll 4
    for (int i = 0; i < 1000; i++) {
        checksum += i * checksum;
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(q);
    free((void *)v);
    
    return 0;
}
