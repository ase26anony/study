/* test_scheduler_coverage.c
 * 
 * This program is designed to trigger the uncovered cleanup code in GCC's
 * Haifa scheduler by creating complex basic blocks that require:
 * 1. Target-specific scheduling hooks
 * 2. Frontend state saving
 * 3. Large instruction queues and ready lists
 * 4. Complex instruction mixes with dependencies
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* Force inlining to create larger basic blocks */
#define ALWAYS_INLINE __attribute__((always_inline))

/* Vector types for SIMD-like operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function with side effects to create scheduling barriers */
static int ALWAYS_INLINE side_effect_func(int x) {
    /* Inline assembly creates a scheduling barrier */
    asm volatile ("" : "+r" (x) : : "memory");
    return x;
}

/* Complex integer computation chain with dependencies */
static int ALWAYS_INLINE test_integer_chain(int a, int b, int c, int d, int e) {
    /* Long dependency chain */
    int t1 = a + b;
    int t2 = t1 * c;
    int t3 = t2 - d;
    int t4 = t3 / (e + 1);
    int t5 = t4 << 2;
    int t6 = t5 ^ 0x55AA55AA;
    int t7 = t6 & 0x0F0F0F0F;
    int t8 = t7 | 0x01010101;
    int t9 = t8 >> 1;
    int t10 = t9 * 3;
    
    /* Introduce side effect to prevent reordering */
    t10 = side_effect_func(t10);
    
    /* More computations */
    int t11 = t10 + a;
    int t12 = t11 * b;
    int t13 = t12 - c;
    int t14 = t13 / (d + 1);
    int t15 = t14 << 3;
    
    return t15;
}

/* Mixed integer and floating-point operations */
static float ALWAYS_INLINE test_mixed_ops(int a, float b, double c, int d) {
    /* Integer to FP conversion creates scheduling complexity */
    float f1 = (float)a + b;
    double d1 = (double)f1 * c;
    float f2 = (float)d1 / (d + 1.0f);
    
    /* Trigonometric operations */
    float f3 = sinf(f2);
    float f4 = cosf(f2);
    float f5 = f3 * f3 + f4 * f4;
    
    /* More mixed operations */
    int i1 = (int)(f5 * 1000.0f);
    float f6 = (float)i1 / 1000.0f;
    
    /* Memory barrier */
    asm volatile ("" : : : "memory");
    
    return f6;
}

/* Function with vector operations */
static v4sf ALWAYS_INLINE test_vector_ops(v4sf a, v4sf b, v4sf c) {
    /* Multiple vector operations */
    v4sf r1 = a + b;
    v4sf r2 = r1 * c;
    v4sf r3 = r2 - a;
    v4sf r4 = r3 + b;
    v4sf r5 = r4 * r1;
    v4sf r6 = r5 - r2;
    v4sf r7 = r6 + r3;
    v4sf r8 = r7 * r4;
    
    /* Horizontal operations */
    float sum = r8[0] + r8[1] + r8[2] + r8[3];
    v4sf r9 = {sum, sum, sum, sum};
    
    return r9;
}

/* Complex loop with software pipelining potential */
static void test_loop_pipelining(int *arr, int n, int *result) {
    /* This loop should trigger frontend state saving */
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    
    /* Small iteration count encourages software pipelining */
    for (int i = 0; i < 8; i++) {
        /* Multiple dependent operations per iteration */
        int val = arr[i];
        int t1 = val * 3;
        int t2 = t1 + i;
        int t3 = t2 << 2;
        int t4 = t3 ^ 0xFF;
        
        /* Conditional update creates control flow complexity */
        if (t4 > 100) {
            sum1 += t4;
            sum2 += t1;
        } else {
            sum3 += t2;
            sum4 += t3;
        }
        
        /* Function call with side effects */
        val = side_effect_func(val);
        
        /* More computations */
        t1 = val * 7;
        t2 = t1 - i;
        t3 = t2 >> 1;
        
        if (t3 < 50) {
            sum1 -= t3;
            sum2 += t4;
        }
    }
    
    *result = sum1 + sum2 + sum3 + sum4;
}

/* Wide basic block with many independent computations */
static int test_wide_block(int a, int b, int c, int d, int e, 
                          int f, int g, int h, int i, int j) {
    /* Many independent computation chains to fill ready list */
    int r1 = a * b + c;
    int r2 = d - e * f;
    int r3 = g / (h + 1);
    int r4 = i ^ j;
    int r5 = a << b;
    int r6 = c & d;
    int r7 = e | f;
    int r8 = g ^ h;
    int r9 = i * j;
    int r10 = a + b + c + d;
    
    /* More chains */
    int r11 = r1 * r2;
    int r12 = r3 + r4;
    int r13 = r5 - r6;
    int r14 = r7 & r8;
    int r15 = r9 ^ r10;
    
    /* Even more */
    int r16 = r11 + r12;
    int r17 = r13 * r14;
    int r18 = r15 - r16;
    int r19 = r17 ^ r18;
    int r20 = r19 & 0xFFFF;
    
    /* Mix with floating point */
    float fr1 = (float)r1 * 1.5f;
    float fr2 = (float)r2 / 2.0f;
    float fr3 = fr1 + fr2;
    int r21 = (int)fr3;
    
    /* Vector operations */
    v4si v1 = {a, b, c, d};
    v4si v2 = {e, f, g, h};
    v4si v3 = v1 + v2;
    v4si v4 = v1 * v2;
    v4si v5 = v3 - v4;
    int r22 = v5[0] + v5[1] + v5[2] + v5[3];
    
    /* Final combination with side effect */
    int result = r20 + r21 + r22;
    result = side_effect_func(result);
    
    return result;
}

/* Function with switch statement for complex control flow */
static int test_switch_block(int x, int y, int z) {
    int result = 0;
    
    /* Complex computation before switch */
    int a = x * y + z;
    int b = y * z - x;
    int c = z * x + y;
    
    /* Switch with multiple cases */
    switch (a % 7) {
        case 0:
            result = b + c;
            /* More computations in case */
            result *= 3;
            result = side_effect_func(result);
            break;
        case 1:
            result = b - c;
            result <<= 2;
            break;
        case 2:
            result = b * c;
            result /= 2;
            break;
        case 3:
            result = b ^ c;
            result &= 0xFF;
            break;
        case 4:
            result = (b << 3) | (c & 0x07);
            break;
        case 5:
            result = (b >> 2) + (c * 3);
            break;
        default:
            result = b + c * 2;
            result = -result;
            break;
    }
    
    /* More computations after switch */
    result += a;
    result = result * 2 - 1;
    
    return result;
}

/* Matrix multiplication to create many memory operations */
static void test_matrix_ops(int size, float *A, float *B, float *C) {
    /* Unrolled loops create wide basic blocks */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            float sum = 0.0f;
            
            /* Partially unrolled loop */
            for (int k = 0; k < size; k += 4) {
                sum += A[i * size + k] * B[k * size + j];
                sum += A[i * size + k + 1] * B[(k + 1) * size + j];
                sum += A[i * size + k + 2] * B[(k + 2) * size + j];
                sum += A[i * size + k + 3] * B[(k + 3) * size + j];
            }
            
            C[i * size + j] = sum;
        }
    }
}

/* Main test driver */
int main() {
    clock_t start = clock();
    int checksum = 0;
    
    /* Test 1: Integer dependency chains */
    printf("Test 1: Integer dependency chains\n");
    for (int i = 0; i < 100; i++) {
        int result = test_integer_chain(i, i+1, i+2, i+3, i+4);
        checksum ^= result;
    }
    
    /* Test 2: Mixed operations */
    printf("Test 2: Mixed integer/float operations\n");
    for (int i = 0; i < 50; i++) {
        float result = test_mixed_ops(i, i * 1.5f, i * 2.5, i + 10);
        checksum ^= (int)(result * 1000.0f);
    }
    
    /* Test 3: Loop pipelining */
    printf("Test 3: Loop pipelining\n");
    int arr[8] = {10, 20, 30, 40, 50, 60, 70, 80};
    int loop_result;
    for (int i = 0; i < 100; i++) {
        test_loop_pipelining(arr, 8, &loop_result);
        checksum ^= loop_result;
        arr[i % 8] += i;  /* Modify array to prevent optimization */
    }
    
    /* Test 4: Wide basic block */
    printf("Test 4: Wide basic block\n");
    for (int i = 0; i < 50; i++) {
        int result = test_wide_block(i, i+1, i+2, i+3, i+4, 
                                    i+5, i+6, i+7, i+8, i+9);
        checksum ^= result;
    }
    
    /* Test 5: Switch statement */
    printf("Test 5: Switch statement\n");
    for (int i = 0; i < 100; i++) {
        int result = test_switch_block(i, i*2, i*3);
        checksum ^= result;
    }
    
    /* Test 6: Matrix operations */
    printf("Test 6: Matrix operations\n");
    const int MAT_SIZE = 16;
    float *A = malloc(MAT_SIZE * MAT_SIZE * sizeof(float));
    float *B = malloc(MAT_SIZE * MAT_SIZE * sizeof(float));
    float *C = malloc(MAT_SIZE * MAT_SIZE * sizeof(float));
    
    if (A && B && C) {
        /* Initialize matrices */
        for (int i = 0; i < MAT_SIZE * MAT_SIZE; i++) {
            A[i] = (float)(i % 100);
            B[i] = (float)((i + 50) % 100);
        }
        
        test_matrix_ops(MAT_SIZE, A, B, C);
        
        /* Compute checksum from result */
        float matrix_sum = 0.0f;
        for (int i = 0; i < MAT_SIZE * MAT_SIZE; i++) {
            matrix_sum += C[i];
        }
        checksum ^= (int)(matrix_sum * 100.0f);
    }
    
    free(A);
    free(B);
    free(C);
    
    /* Test 7: Vector operations */
    printf("Test 7: Vector operations\n");
    v4sf va = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vb = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vc = {9.0f, 10.0f, 11.0f, 12.0f};
    
    for (int i = 0; i < 100; i++) {
        v4sf result = test_vector_ops(va, vb, vc);
        checksum ^= (int)(result[0] + result[1] + result[2] + result[3]);
        va[0] += 0.1f;  /* Prevent optimization */
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("\nFinal checksum: %d\n", checksum);
    printf("Total time: %.3f seconds\n", elapsed);
    printf("All tests completed.\n");
    
    return checksum != 0 ? 0 : 1;
}
