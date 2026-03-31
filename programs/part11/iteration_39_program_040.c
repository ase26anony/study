/* test_scheduler_coverage.c
 * 
 * This program is designed to trigger the uncovered cleanup code in GCC's
 * Haifa scheduler (haifa-sched.cc lines 4681-4691) by creating complex
 * basic blocks that force the scheduler to allocate and use:
 * 1. Target-specific scheduling hooks (targetm.sched.free_sched_context)
 * 2. Frontend state saving (current_sched_info->restore_state)
 * 3. Large instruction queues and ready lists
 * 4. Complex scheduling contexts
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* Force compiler to keep computations */
#define KEEP(expr) do { asm volatile("" : : "g"(expr) : "memory"); } while(0)

/* Vector types for creating wide basic blocks */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Inline functions to increase instruction count */
static inline int compute_hash(int a, int b, int c) {
    return (a * 31 + b) * 31 + c;
}

static inline float fp_complex(float a, float b, float c) {
    float t1 = a * b + c;
    float t2 = a / (b + 1.0f);
    float t3 = sqrtf(fabsf(t1 - t2));
    return t3 * 2.0f - 1.0f;
}

static inline double dp_complex(double a, double b) {
    double x = a * b;
    double y = a + b;
    double z = x / (y + 1.0);
    return sin(x) * cos(y) + tan(z);
}

/* Function 1: Wide basic block with mixed operations and dependencies */
/* This creates a large instruction queue and ready list */
unsigned long long test_wide_block(int n, int *data) {
    /* Create many independent computation chains */
    int a = data[0], b = data[1], c = data[2], d = data[3];
    int e = data[4], f = data[5], g = data[6], h = data[7];
    
    /* Chain 1: Integer arithmetic with dependencies */
    int r1 = a + b;
    int r2 = r1 * c;
    int r3 = r2 - d;
    int r4 = r3 / (e + 1);
    int r5 = r4 ^ f;
    int r6 = r5 | g;
    int r7 = r6 & h;
    int r8 = r7 << 2;
    int r9 = r8 >> 1;
    
    /* Chain 2: Parallel integer operations */
    int s1 = b * c;
    int s2 = d - e;
    int s3 = f + g;
    int s4 = h ^ a;
    int s5 = s1 & s2;
    int s6 = s3 | s4;
    int s7 = s5 * s6;
    int s8 = s7 - s1;
    int s9 = s8 / (s2 + 1);
    
    /* Chain 3: Floating point operations */
    float fa = (float)a, fb = (float)b, fc = (float)c;
    float fd = (float)d, fe = (float)e, ff = (float)f;
    
    float fr1 = fa * fb + fc;
    float fr2 = fd - fe * ff;
    float fr3 = fr1 / (fr2 + 1.0f);
    float fr4 = sqrtf(fabsf(fr3));
    float fr5 = fr4 * 2.0f - 1.0f;
    
    /* Chain 4: More FP with dependencies */
    double da = (double)a, db = (double)b, dc = (double)c;
    double dr1 = sin(da) * cos(db);
    double dr2 = exp(dc) - 1.0;
    double dr3 = dr1 / (dr2 + 1.0);
    double dr4 = log(fabs(dr3) + 1.0);
    
    /* Chain 5: Memory operations with potential aliasing */
    int *ptr1 = &data[0];
    int *ptr2 = &data[4];
    int m1 = *ptr1 + *ptr2;
    *ptr1 = m1;
    int m2 = *(ptr1 + 1) - *(ptr2 + 1);
    *(ptr2 + 1) = m2;
    int m3 = *ptr1 * *(ptr2 + 2);
    
    /* Chain 6: Conditional computations */
    int cond1 = (r1 > r2) ? r1 : r2;
    int cond2 = (s3 < s4) ? s3 * 2 : s4 / 2;
    int cond3 = (m1 != m2) ? cond1 + cond2 : cond1 - cond2;
    
    /* Chain 7: Mixed type conversions */
    float mixed1 = (float)(r3 + s5) / (float)(m3 + 1);
    int mixed2 = (int)(fr5 * 100.0f) + (int)(dr4 * 1000.0);
    
    /* Final combination with many operations */
    unsigned long long result = 
        (unsigned long long)r9 * 1000000007ULL +
        (unsigned long long)s9 * 1000000009ULL +
        (unsigned long long)(fr5 * 1000.0f) * 1000000021ULL +
        (unsigned long long)(dr4 * 10000.0) * 1000000033ULL +
        (unsigned long long)m3 * 1000000087ULL +
        (unsigned long long)cond3 * 1000000093ULL +
        (unsigned long long)mixed2 * 1000000097ULL;
    
    KEEP(result);
    return result;
}

/* Function 2: Loop with software pipelining potential */
/* Triggers frontend state saving (restore_state) */
double test_loop_pipelining(int iterations, double *array) {
    double sum1 = 0.0, sum2 = 0.0, sum3 = 0.0, sum4 = 0.0;
    double prod1 = 1.0, prod2 = 1.0;
    
    /* Small loop that might be software pipelined */
    for (int i = 0; i < iterations && i < 8; i++) {
        /* Dependent operations within loop */
        double x = array[i];
        double y = sin(x * 0.1);
        double z = cos(x * 0.2);
        
        sum1 += x * y;
        sum2 += y * z;
        sum3 += z * x;
        sum4 += x + y + z;
        
        prod1 *= (x + 1.0);
        prod2 *= (y + 1.0);
        
        /* Conditional that creates scheduling barriers */
        if (x > 0.5) {
            sum1 -= z * 0.5;
            prod1 /= (z + 1.0);
        } else {
            sum2 += x * 0.3;
            prod2 *= (x + 0.5);
        }
        
        /* More complex dependency chain */
        double t1 = sum1 * 0.1 + sum2 * 0.2;
        double t2 = sum3 * 0.3 + sum4 * 0.4;
        double t3 = t1 * t2 / (prod1 + prod2 + 1.0);
        
        array[i] = t3;  /* Store back creates memory dependency */
    }
    
    /* Final computation with many operations */
    double result = (sum1 * sum2 + sum3 * sum4) / (prod1 + prod2);
    KEEP(result);
    return result;
}

/* Function 3: Vector operations using GCC vector extensions */
/* Creates many parallel operations for the scheduler */
v4si test_vector_ops(v4si a, v4si b, v4si c, v4si d) {
    /* Multiple vector operations creating wide basic block */
    v4si r1 = a + b;
    v4si r2 = c - d;
    v4si r3 = a * b;
    v4si r4 = c & d;
    v4si r5 = a | b;
    v4si r6 = c ^ d;
    
    v4si t1 = r1 * r2;
    v4si t2 = r3 + r4;
    v4si t3 = r5 - r6;
    v4si t4 = r1 & r3;
    v4si t5 = r2 | r4;
    
    v4si u1 = t1 * 2;
    v4si u2 = t2 + 1;
    v4si u3 = t3 - t4;
    v4si u4 = t5 ^ t1;
    
    v4si v1 = u1 * u2;
    v4si v2 = u3 + u4;
    v4si v3 = u1 - u2;
    v4si v4 = u3 & u4;
    
    /* Final combination */
    v4si result = v1 + v2 + v3 + v4;
    KEEP(result);
    return result;
}

/* Function 4: Complex control flow with switch statement */
/* Forces state saving for speculative scheduling */
int test_control_flow(int x, int y, int z) {
    int result = 0;
    
    /* Complex computation before control flow */
    int a = x * y + z;
    int b = y * z - x;
    int c = z * x + y;
    int d = compute_hash(x, y, z);
    
    /* Switch with multiple cases - creates scheduling barriers */
    switch (d % 7) {
        case 0:
            result = a + b * 2;
            /* Inline more computations */
            result += fp_complex((float)a, (float)b, (float)c) * 100.0f;
            break;
        case 1:
            result = b - c / 2;
            result *= (int)dp_complex((double)a, (double)b);
            break;
        case 2:
            result = c * a + 1;
            for (int i = 0; i < 4; i++) {
                result += (a >> i) & 1;
            }
            break;
        case 3:
            result = d ^ (a & b);
            result |= (c << 3);
            break;
        case 4:
            result = (a + b + c) * d;
            result -= fp_complex((float)result, (float)a, (float)b);
            break;
        case 5:
            result = a * b * c;
            result /= (d + 1);
            break;
        default:
            result = a | b | c | d;
            result = ~result;
            break;
    }
    
    /* More computations after switch */
    int final = result;
    final += compute_hash(result, a, b);
    final -= (int)fp_complex((float)result, (float)c, (float)d);
    final *= (int)dp_complex((double)final, (double)result);
    
    KEEP(final);
    return final;
}

/* Function 5: Memory-intensive with aliasing concerns */
float test_memory_aliasing(float *arr1, float *arr2, int size) {
    float sum = 0.0f;
    float prod = 1.0f;
    
    /* Unrolled loop creates wide basic block */
    for (int i = 0; i < size && i < 16; i += 4) {
        /* Independent loads and stores with potential aliasing */
        float a1 = arr1[i];
        float a2 = arr1[i + 1];
        float a3 = arr2[i];
        float a4 = arr2[i + 1];
        
        /* Complex computations */
        float b1 = a1 * a2 + a3;
        float b2 = a2 - a3 * a4;
        float b3 = sqrtf(fabsf(a1 + a4));
        float b4 = fp_complex(a1, a2, a3);
        
        /* Stores create memory dependencies */
        arr1[i] = b1;
        arr2[i] = b2;
        arr1[i + 1] = b3;
        arr2[i + 1] = b4;
        
        /* More computations using stored values */
        float c1 = arr1[i] * 0.5f;
        float c2 = arr2[i] * 1.5f;
        float c3 = arr1[i + 1] + arr2[i + 1];
        float c4 = c1 * c2 - c3;
        
        sum += c1 + c2 + c3 + c4;
        prod *= (c4 + 1.0f);
        
        /* Conditional store */
        if (c4 > 0.0f) {
            arr1[i] += c4;
            arr2[i] -= c4;
        }
    }
    
    float result = sum / (prod + 1.0f);
    KEEP(result);
    return result;
}

/* Function 6: Mixed integer/float with function calls */
/* Creates scheduling barriers */
double test_mixed_with_calls(int n, double *data) {
    double acc = 0.0;
    
    /* Multiple function calls create scheduling barriers */
    for (int i = 0; i < n && i < 12; i++) {
        double x = data[i];
        
        /* Call math functions - act as scheduling barriers */
        double s = sin(x * 0.01);
        double c = cos(x * 0.02);
        double t = tan(x * 0.005);
        
        /* Complex dependency chain */
        double y = s * c + t;
        double z = exp(s - c);
        double w = log(fabs(t) + 1.0);
        
        /* Integer operations mixed in */
        int ix = (int)(x * 1000.0);
        int iy = (int)(y * 1000.0);
        int iz = compute_hash(ix, iy, i + 1);
        
        /* More FP */
        double u = dp_complex(y, z);
        double v = fp_complex((float)s, (float)c, (float)t);
        
        acc += u * v + (double)iz * 0.001;
        
        /* Store with potential aliasing */
        data[i] = acc;
    }
    
    KEEP(acc);
    return acc;
}

/* Main driver that calls all test functions */
int main() {
    /* Initialize test data */
    const int DATA_SIZE = 64;
    int int_data[DATA_SIZE];
    float float_arr1[DATA_SIZE], float_arr2[DATA_SIZE];
    double double_data[DATA_SIZE];
    
    srand(time(NULL));
    for (int i = 0; i < DATA_SIZE; i++) {
        int_data[i] = rand() % 1000;
        float_arr1[i] = (float)(rand() % 1000) * 0.01f;
        float_arr2[i] = (float)(rand() % 1000) * 0.02f;
        double_data[i] = (double)(rand() % 1000) * 0.001;
    }
    
    unsigned long long checksum = 0;
    
    /* Call test functions multiple times to ensure scheduling occurs */
    for (int iter = 0; iter < 3; iter++) {
        /* Test 1: Wide basic block */
        checksum += test_wide_block(DATA_SIZE, int_data);
        
        /* Test 2: Loop pipelining */
        double loop_result = test_loop_pipelining(8, double_data);
        checksum += (unsigned long long)(loop_result * 1000000.0);
        
        /* Test 3: Vector operations */
        v4si va = {1, 2, 3, 4};
        v4si vb = {5, 6, 7, 8};
        v4si vc = {9, 10, 11, 12};
        v4si vd = {13, 14, 15, 16};
        v4si vresult = test_vector_ops(va, vb, vc, vd);
        for (int i = 0; i < 4; i++) {
            checksum += vresult[i];
        }
        
        /* Test 4: Control flow */
        int cf_result = test_control_flow(
            int_data[0], int_data[1], int_data[2]);
        checksum += cf_result;
        
        /* Test 5: Memory aliasing */
        float mem_result = test_memory_aliasing(
            float_arr1, float_arr2, DATA_SIZE);
        checksum += (unsigned long long)(mem_result * 1000.0f);
        
        /* Test 6: Mixed with calls */
        double call_result = test_mixed_with_calls(12, double_data);
        checksum += (unsigned long long)(call_result * 1000000.0);
    }
    
    printf("Final checksum: %llu\n", checksum);
    return 0;
}
