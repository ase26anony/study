/* test_sched_context.c
 * 
 * This program is designed to trigger the cleanup logic in GCC's Haifa scheduler
 * (haifa-sched.cc lines 4681-4691) by creating complex basic blocks that require
 * extensive instruction scheduling with target hooks and state saving.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* Prevent excessive inlining to keep basic blocks intact */
#define NOINLINE __attribute__((noinline))

/* Vector types for SIMD-like operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile int global_seed = 42;
volatile float global_float = 3.14159f;
volatile double global_double = 2.71828;

/* Complex function 1: Mixed integer and floating-point operations
 * Creates dependency chains and uses different execution units
 */
NOINLINE static double complex_math_1(int iterations) {
    double a = global_double;
    float b = global_float;
    int c = global_seed;
    double result = 0.0;
    
    /* Complex basic block with mixed operations */
    for (int i = 0; i < iterations; i++) {
        /* Integer dependency chain */
        c = c * 1103515245 + 12345;
        int d = c >> 16;
        int e = d * 16807;
        int f = e % 2147483647;
        
        /* Floating-point dependency chain */
        b = b * 1.234567f + 0.987654f;
        float g = sinf(b) * cosf(b);
        float h = g * g + 1.0f;
        
        /* Mixed operations */
        a = a * 1.6180339887 + 0.5;
        double j = sin(a) * cos(a);
        double k = exp(-j * j);
        
        /* Memory operations with potential aliasing */
        double* ptr1 = &a;
        double* ptr2 = &result;
        *ptr2 += (*ptr1) * k + (double)g;
        
        /* Conditional that creates scheduling barriers */
        if (f % 1000 < 500) {
            result += 0.1;
        } else {
            result -= 0.1;
        }
    }
    
    return result;
}

/* Complex function 2: SIMD-like operations using vector extensions
 * Creates wide basic blocks with parallel computation paths
 */
NOINLINE static v4sf vector_operations(v4sf a, v4sf b, v4sf c, int count) {
    v4sf result = {0.0f, 0.0f, 0.0f, 0.0f};
    v4sf accum1 = a;
    v4sf accum2 = b;
    v4sf accum3 = c;
    
    /* Unrolled loop creates wide basic block */
    for (int i = 0; i < count; i++) {
        /* Multiple independent vector operations */
        v4sf t1 = accum1 * accum2;
        v4sf t2 = accum2 + accum3;
        v4sf t3 = accum1 - accum3;
        v4sf t4 = t1 * t2;
        v4sf t5 = t2 / (t3 + 1.0f);
        
        /* Cross-lane operations */
        float sum1 = t1[0] + t1[1] + t1[2] + t1[3];
        float sum2 = t2[0] * t2[1] * t2[2] * t2[3];
        
        /* More mixed operations */
        accum1 = t4 + (v4sf){sum1, sum1, sum1, sum1};
        accum2 = t5 * (v4sf){sum2, sum2, sum2, sum2};
        accum3 = accum1 + accum2;
        
        /* Conditional update */
        if (i % 3 == 0) {
            result = result + accum1;
        } else if (i % 3 == 1) {
            result = result + accum2;
        } else {
            result = result + accum3;
        }
    }
    
    return result;
}

/* Complex function 3: Memory-intensive operations with aliasing
 * Forces the scheduler to handle memory dependencies
 */
NOINLINE static void memory_intensive(int size, double* output) {
    double* array1 = (double*)malloc(size * sizeof(double));
    double* array2 = (double*)malloc(size * sizeof(double));
    double* array3 = (double*)malloc(size * sizeof(double));
    
    if (!array1 || !array2 || !array3) return;
    
    /* Initialize arrays */
    for (int i = 0; i < size; i++) {
        array1[i] = i * 0.1;
        array2[i] = i * 0.2;
        array3[i] = i * 0.3;
    }
    
    /* Complex memory access pattern with potential aliasing */
    double sum = 0.0;
    for (int i = 1; i < size - 1; i++) {
        /* Read from multiple arrays with overlapping indices */
        double a = array1[i];
        double b = array2[i-1];
        double c = array3[i+1];
        
        /* Compute with dependencies */
        double d = a * b + c;
        double e = sin(d) * cos(d);
        double f = exp(-e * e);
        
        /* Write back with pointer arithmetic (potential aliasing) */
        double* ptr1 = &array1[i];
        double* ptr2 = &array2[i];
        *ptr1 = d * f;
        *ptr2 = e + f;
        
        /* Update sum with conditional */
        sum += (*ptr1) + (*ptr2);
        
        /* Function call with side effects (scheduling barrier) */
        if (i % 100 == 0) {
            sum += global_double * 0.01;
        }
    }
    
    *output = sum;
    
    free(array1);
    free(array2);
    free(array3);
}

/* Complex function 4: Nested loops with small iteration counts
 * May trigger software pipelining attempts
 */
NOINLINE static double nested_loops(int outer, int inner) {
    double matrix[8][8];
    double result = 0.0;
    
    /* Initialize small matrix */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            matrix[i][j] = (i * 8 + j) * 0.01;
        }
    }
    
    /* Nested loops with small iteration counts */
    for (int o = 0; o < outer; o++) {
        /* Small inner loop that might be software pipelined */
        for (int i = 0; i < inner; i++) {
            /* Complex computation with matrix operations */
            double temp[8];
            for (int j = 0; j < 8; j++) {
                temp[j] = 0.0;
                for (int k = 0; k < 8; k++) {
                    temp[j] += matrix[j][k] * matrix[k][j];
                }
            }
            
            /* Update matrix */
            for (int j = 0; j < 8; j++) {
                matrix[j][j] = temp[j] * 0.99 + 0.01;
            }
            
            /* Accumulate result */
            result += matrix[o % 8][i % 8];
        }
        
        /* Conditional that may cause speculative scheduling */
        if (o % 2 == 0) {
            result *= 1.01;
        } else {
            result *= 0.99;
        }
    }
    
    return result;
}

/* Complex function 5: Switch statement with computed values
 * Creates complex control flow requiring state tracking
 */
NOINLINE static double switch_computation(int mode, int count) {
    double x = 1.0;
    double y = 2.0;
    double z = 3.0;
    
    for (int i = 0; i < count; i++) {
        /* Complex switch with computed cases */
        switch ((mode + i) % 7) {
            case 0:
                x = sin(x) * cos(y) + tan(z);
                y = x * y - z;
                break;
            case 1:
                y = exp(x) * log(y + 1.0);
                z = y * z + x;
                break;
            case 2:
                z = sqrt(fabs(x)) + pow(fabs(y), 0.5);
                x = z * x - y;
                break;
            case 3:
                x = y * z / (x + 1.0);
                y = sin(x + y) * cos(z);
                break;
            case 4:
                z = tan(x * y) + atan(z);
                x = z * 0.5 + x * 0.5;
                break;
            case 5:
                y = asin(fmin(0.99, fabs(x))) + acos(fmin(0.99, fabs(y)));
                z = y * z * 0.618;
                break;
            default:
                x = x * 1.618 + y * 0.618;
                y = y * 1.414 + z * 0.414;
                z = z * 1.732 + x * 0.268;
                break;
        }
        
        /* Additional computation after switch */
        double t = x + y + z;
        x = sin(t) * 0.5;
        y = cos(t) * 0.5;
        z = tan(t) * 0.1;
    }
    
    return x + y + z;
}

/* Complex function 6: Inline assembly mixed with C code
 * Creates scheduling barriers and requires precise scheduling
 */
NOINLINE static double asm_mixed_computation(double a, double b, int count) {
    double result = 0.0;
    
    for (int i = 0; i < count; i++) {
        double temp1, temp2, temp3;
        
        /* Inline assembly creates scheduling barriers */
        #if defined(__x86_64__) || defined(__i386__)
        asm volatile (
            "movsd %1, %%xmm0\n\t"
            "movsd %2, %%xmm1\n\t"
            "mulsd %%xmm1, %%xmm0\n\t"
            "addsd %%xmm0, %%xmm0\n\t"
            "movsd %%xmm0, %0\n\t"
            : "=m"(temp1)
            : "m"(a), "m"(b)
            : "xmm0", "xmm1"
        );
        #elif defined(__arm__) || defined(__aarch64__)
        asm volatile (
            "fmul %d0, %d1, %d2\n\t"
            "fadd %d0, %d0, %d0\n\t"
            : "=w"(temp1)
            : "w"(a), "w"(b)
        );
        #else
        temp1 = a * b * 2.0;
        #endif
        
        /* C computations around assembly */
        temp2 = sin(temp1) * cos(temp1);
        temp3 = exp(-temp2 * temp2);
        
        /* More assembly */
        #if defined(__x86_64__) || defined(__i386__)
        asm volatile (
            "movsd %1, %%xmm0\n\t"
            "movsd %2, %%xmm1\n\t"
            "addsd %%xmm1, %%xmm0\n\t"
            "sqrtsd %%xmm0, %%xmm0\n\t"
            "movsd %%xmm0, %0\n\t"
            : "=m"(temp2)
            : "m"(temp1), "m"(temp3)
            : "xmm0", "xmm1"
        );
        #elif defined(__arm__) || defined(__aarch64__)
        asm volatile (
            "fadd %d0, %d1, %d2\n\t"
            "fsqrt %d0, %d0\n\t"
            : "=w"(temp2)
            : "w"(temp1), "w"(temp3)
        );
        #else
        temp2 = sqrt(temp1 + temp3);
        #endif
        
        result += temp2;
        
        /* Update parameters */
        a = a * 0.99 + 0.01;
        b = b * 1.01 - 0.01;
    }
    
    return result;
}

/* Main driver function that calls all test functions */
int main(int argc, char** argv) {
    double total_result = 0.0;
    clock_t start, end;
    
    printf("Starting scheduler context test...\n");
    start = clock();
    
    /* Test 1: Mixed integer/float operations */
    printf("Running complex_math_1...\n");
    double r1 = complex_math_1(1000);
    total_result += r1;
    printf("  Result: %f\n", r1);
    
    /* Test 2: Vector operations */
    printf("Running vector_operations...\n");
    v4sf v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf v3 = {0.1f, 0.2f, 0.3f, 0.4f};
    v4sf vr = vector_operations(v1, v2, v3, 500);
    total_result += vr[0] + vr[1] + vr[2] + vr[3];
    printf("  Result: %f, %f, %f, %f\n", vr[0], vr[1], vr[2], vr[3]);
    
    /* Test 3: Memory intensive operations */
    printf("Running memory_intensive...\n");
    double r3;
    memory_intensive(10000, &r3);
    total_result += r3;
    printf("  Result: %f\n", r3);
    
    /* Test 4: Nested loops */
    printf("Running nested_loops...\n");
    double r4 = nested_loops(50, 8);
    total_result += r4;
    printf("  Result: %f\n", r4);
    
    /* Test 5: Switch computation */
    printf("Running switch_computation...\n");
    double r5 = switch_computation(argc > 1 ? atoi(argv[1]) : 3, 1000);
    total_result += r5;
    printf("  Result: %f\n", r5);
    
    /* Test 6: Assembly mixed computation */
    printf("Running asm_mixed_computation...\n");
    double r6 = asm_mixed_computation(1.0, 2.0, 1000);
    total_result += r6;
    printf("  Result: %f\n", r6);
    
    end = clock();
    double cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("\nTotal result: %.15f\n", total_result);
    printf("Total CPU time: %.3f seconds\n", cpu_time);
    
    /* Use result to prevent dead code elimination */
    if (total_result > 1000000.0) {
        printf("Result is very large!\n");
    }
    
    return (int)(total_result * 0.000001) % 256;
}
