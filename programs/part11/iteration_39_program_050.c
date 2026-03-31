/* test_sched_context.c - Complex scheduling test for GCC Haifa scheduler cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* Force inlining to create larger basic blocks */
#define ALWAYS_INLINE __attribute__((always_inline))

/* Vector types for parallel operations */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Non-pure function to create scheduling barriers */
static int ALWAYS_INLINE side_effect_func(int x) {
    static int counter = 0;
    counter += x;
    return counter;
}

/* Complex integer computation with dependencies */
static int ALWAYS_INLINE test_int_dep_chain(int a, int b, int c, int d, int e) {
    int t1 = a + b;
    int t2 = t1 * c;
    int t3 = t2 - d;
    int t4 = t3 / (e + 1);
    int t5 = t4 << 2;
    int t6 = t5 ^ t1;
    int t7 = t6 | t2;
    int t8 = t7 & t3;
    int t9 = t8 * t4;
    int t10 = t9 - t5;
    
    /* Create side effect */
    t10 += side_effect_func(t10);
    
    return t10;
}

/* Mixed FP and integer operations */
static float ALWAYS_INLINE test_mixed_operations(float a, float b, int c, float d) {
    float f1 = a * b;
    float f2 = sinf(f1) + cosf(b);
    int i1 = (int)f2 * c;
    float f3 = f2 / (d + 1.0f);
    float f4 = f3 * expf(f1);
    int i2 = i1 ^ (int)f4;
    float f5 = f4 + (float)i2;
    
    /* Memory operation with potential aliasing */
    static float mem[4] = {1.1f, 2.2f, 3.3f, 4.4f};
    f5 += mem[i2 & 3];
    
    return f5;
}

/* Vector operations to create parallel instruction chains */
static v4sf ALWAYS_INLINE test_vector_ops(v4sf a, v4sf b, v4sf c) {
    v4sf t1 = a + b;
    v4sf t2 = t1 * c;
    v4sf t3 = t2 - a;
    v4sf t4 = t3 / (b + 1.0f);
    v4sf t5 = __builtin_shuffle(t4, t1, (v4si){0, 1, 2, 3});
    v4sf t6 = t5 * t2;
    
    return t6;
}

/* Complex loop with software pipelining potential */
static double test_loop_pipelining(int iterations) {
    double sum = 0.0;
    double a = 1.0, b = 2.0, c = 3.0;
    
    /* Small loop that might be software pipelined */
    for (int i = 0; i < iterations; i++) {
        double t1 = a * b;
        double t2 = sin(t1) * c;
        double t3 = cos(b) + t2;
        double t4 = t3 / (a + 1.0);
        
        sum += t4;
        
        /* Update with dependencies */
        a = t2 * 0.9;
        b = t3 * 1.1;
        c = t4 * 0.8;
        
        /* Conditional that might cause speculative scheduling */
        if (sum > 1000.0) {
            sum *= 0.99;
        }
    }
    
    return sum;
}

/* Wide basic block with unrolled operations */
static int test_wide_block(int *arr, int n) {
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    
    /* Manual unrolling to create wide block */
    for (int i = 0; i < n; i += 4) {
        /* Independent chains that can fill ready list */
        int t1 = arr[i] * 3;
        int t2 = arr[i+1] + 7;
        int t3 = arr[i+2] - 5;
        int t4 = arr[i+3] ^ 0xFF;
        
        int u1 = t1 << 2;
        int u2 = t2 >> 1;
        int u3 = t3 * t4;
        int u4 = t1 + t2;
        
        sum1 += u1 * u2;
        sum2 += u3 | u4;
        sum3 += (u1 ^ u3) * u2;
        sum4 += side_effect_func(u4);
        
        /* Memory operations with potential aliasing */
        arr[i] = sum1;
        arr[i+1] = sum2;
        arr[i+2] = sum3;
        arr[i+3] = sum4;
    }
    
    return sum1 + sum2 + sum3 + sum4;
}

/* Function with switch statement for complex control flow */
static int test_switch_block(int x, int y) {
    int result = 0;
    
    /* Multiple dependent operations before switch */
    int a = x * y;
    int b = a + (x ^ y);
    int c = b * 3 - y;
    float f = sinf((float)c) * 100.0f;
    
    /* Switch creates control flow complexity */
    switch (x % 8) {
        case 0:
            result = a + (int)f;
            break;
        case 1:
            result = b * (int)f;
            break;
        case 2:
            result = c ^ (int)f;
            break;
        case 3:
            result = (a * b) / (c + 1);
            break;
        case 4:
            result = side_effect_func(a) + b;
            break;
        case 5:
            result = test_int_dep_chain(a, b, c, x, y);
            break;
        case 6:
            result = (int)test_mixed_operations((float)a, (float)b, c, f);
            break;
        default:
            result = a + b + c;
            break;
    }
    
    /* More operations after switch */
    result *= 2;
    result -= side_effect_func(result);
    
    return result;
}

/* Matrix operations for complex scheduling */
static void test_matrix_ops(float *A, float *B, float *C, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            float sum = 0.0f;
            /* Unrolled inner loop */
            for (int k = 0; k < n; k += 4) {
                sum += A[i*n + k] * B[k*n + j];
                sum += A[i*n + k+1] * B[(k+1)*n + j];
                sum += A[i*n + k+2] * B[(k+2)*n + j];
                sum += A[i*n + k+3] * B[(k+3)*n + j];
            }
            C[i*n + j] = sum;
            
            /* Conditional with side effect */
            if (C[i*n + j] > 1000.0f) {
                C[i*n + j] = logf(C[i*n + j]);
                side_effect_func((int)C[i*n + j]);
            }
        }
    }
}

/* Main test driver */
int main() {
    const int N = 256;
    const int ITERS = 100;
    
    /* Allocate test arrays */
    int *int_arr = malloc(N * sizeof(int));
    float *matA = malloc(N * N * sizeof(float));
    float *matB = malloc(N * N * sizeof(float));
    float *matC = malloc(N * N * sizeof(float));
    
    /* Initialize with random data */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        int_arr[i] = rand() % 1000;
    }
    for (int i = 0; i < N * N; i++) {
        matA[i] = (float)rand() / RAND_MAX * 10.0f;
        matB[i] = (float)rand() / RAND_MAX * 10.0f;
    }
    
    int total_result = 0;
    double total_fp = 0.0;
    
    /* Execute all test functions multiple times */
    for (int iter = 0; iter < ITERS; iter++) {
        /* Test 1: Integer dependency chains */
        total_result += test_int_dep_chain(
            int_arr[iter % N],
            int_arr[(iter + 1) % N],
            int_arr[(iter + 2) % N],
            int_arr[(iter + 3) % N],
            int_arr[(iter + 4) % N]
        );
        
        /* Test 2: Mixed operations */
        total_fp += test_mixed_operations(
            (float)int_arr[iter % N],
            (float)int_arr[(iter + 5) % N],
            int_arr[(iter + 6) % N],
            (float)int_arr[(iter + 7) % N]
        );
        
        /* Test 3: Wide block with unrolling */
        total_result += test_wide_block(int_arr, 64);
        
        /* Test 4: Loop pipelining */
        total_fp += test_loop_pipelining(8);
        
        /* Test 5: Switch block */
        total_result += test_switch_block(iter, int_arr[iter % N]);
        
        /* Test 6: Vector operations */
        v4sf va = {1.0f, 2.0f, 3.0f, 4.0f};
        v4sf vb = {5.0f, 6.0f, 7.0f, 8.0f};
        v4sf vc = {9.0f, 10.0f, 11.0f, 12.0f};
        v4sf vresult = test_vector_ops(va, vb, vc);
        
        /* Test 7: Matrix operations (every 10 iterations) */
        if (iter % 10 == 0) {
            test_matrix_ops(matA, matB, matC, 16);
        }
    }
    
    /* Compute checksum to prevent optimization */
    float checksum = (float)total_result + (float)total_fp;
    for (int i = 0; i < N; i++) {
        checksum += (float)int_arr[i];
    }
    for (int i = 0; i < N * N; i += 257) {
        checksum += matC[i % (N * N)];
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Cleanup */
    free(int_arr);
    free(matA);
    free(matB);
    free(matC);
    
    return (checksum > 0) ? 0 : 1;
}
