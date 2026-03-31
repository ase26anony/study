/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test_sched_coverage
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test_sched_pressure
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define ITERATIONS 100000
#define ARRAY_SIZE 1024

/* Volatile variables to prevent optimizations and create dependencies */
volatile int vol_var1 = 1;
volatile int vol_var2 = 2;
volatile float vol_float1 = 1.5f;
volatile float vol_float2 = 2.5f;

/* Function to create high register pressure with independent instructions */
__attribute__((noinline))
static void high_pressure_loop(float *restrict a, float *restrict b, 
                               float *restrict c, int size) {
    int i;
    /* Many independent variables to create register pressure */
    float t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    float t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    for (i = 0; i < size; i++) {
        /* Group 1: Independent floating point operations */
        t1 = a[i] * b[i];
        t2 = a[i] + b[i];
        t3 = a[i] - b[i];
        t4 = a[i] / (b[i] + 0.001f);
        
        /* Group 2: More independent operations */
        t5 = t1 * t2;
        t6 = t3 + t4;
        t7 = t1 - t4;
        t8 = t2 / (t3 + 0.001f);
        
        /* Group 3: Creating data dependencies */
        t9 = t5 + t6;
        t10 = t7 * t8;
        t11 = t9 - t10;
        t12 = t5 / (t6 + 0.001f);
        
        /* Group 4: Mix with volatile to force delays */
        t13 = t11 * vol_float1;
        t14 = t12 + vol_float2;
        
        /* Group 5: More operations to increase pressure */
        t15 = t13 * t14;
        t16 = t13 + t14;
        t17 = t13 - t14;
        t18 = t13 / (t14 + 0.001f);
        
        /* Group 6: Final computations */
        t19 = t15 + t16 + t17 + t18;
        t20 = t15 * t16 * t17 * t18;
        
        /* Store results with artificial dependencies */
        c[i] = t19 + t20 + (float)vol_var1;
        
        /* Inline assembly to clobber registers and force spills */
        __asm__ volatile (
            "mov $0, %%eax\n"
            "mov $0, %%ebx\n"
            "mov $0, %%ecx\n"
            "mov $0, %%edx\n"
            "mov $0, %%esi\n"
            "mov $0, %%edi\n"
            : : : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
    }
}

/* Function with mixed dependencies and resource conflicts */
__attribute__((noinline))
static float mixed_dependency(int iterations) {
    float result = 0.0f;
    int i;
    
    /* Create alternating chains of dependencies */
    float chain1 = 1.0f;
    float chain2 = 2.0f;
    float chain3 = 3.0f;
    float chain4 = 4.0f;
    
    for (i = 0; i < iterations; i++) {
        /* Long latency operations (divisions) */
        float div1 = chain1 / 3.14159f;
        float div2 = chain2 / 2.71828f;
        
        /* Independent integer operations */
        int int1 = vol_var1 * i;
        int int2 = vol_var2 + i;
        int int3 = int1 - int2;
        int int4 = int1 * int2;
        
        /* Memory operations with aliasing concerns */
        float mem1 = div1 * (float)int3;
        float mem2 = div2 * (float)int4;
        
        /* Resource conflicts: mix FP and integer */
        float fp_op1 = mem1 * mem2;
        int int_op1 = (int)fp_op1 * int3;
        float fp_op2 = (float)int_op1 / mem1;
        int int_op2 = (int)fp_op2 + int4;
        
        /* Volatile accesses create scheduling barriers */
        chain1 = fp_op1 + vol_float1;
        chain2 = fp_op2 - vol_float2;
        chain3 = (float)int_op1 * vol_float1;
        chain4 = (float)int_op2 / vol_float2;
        
        /* Complex expression with many operands */
        result += chain1 * chain2 - chain3 / chain4 + 
                  (float)int1 / (float)int2 - 
                  (float)int3 * (float)int4;
        
        /* Conditional to create control flow variation */
        if (i % 7 == 0) {
            result *= 0.99f;
        } else if (i % 13 == 0) {
            result /= 1.01f;
        }
        
        /* More inline assembly for register pressure */
        __asm__ volatile (
            "fldz\n"
            "fld1\n"
            "faddp %%st, %%st(1)\n"
            : : : "st", "st(1)", "memory"
        );
    }
    
    return result;
}

/* Function with many independent instructions for candidate selection */
__attribute__((noinline))
static int independent_instructions(int seed) {
    /* Many independent integer operations */
    int a = seed * 1103515245 + 12345;
    int b = seed * 1664525 + 1013904223;
    int c = seed * 214013 + 2531011;
    int d = seed * 134775813 + 1;
    int e = a ^ b;
    int f = c & d;
    int g = a | c;
    int h = b ^ d;
    int i = e + f;
    int j = g - h;
    int k = i * j;
    int l = e / (f + 1);
    int m = g % (h + 1);
    int n = k ^ l;
    int o = m & n;
    int p = i | j;
    int q = k + l;
    int r = m - n;
    int s = o * p;
    int t = q / (r + 1);
    int u = s % (t + 1);
    int v = n ^ o;
    int w = p & q;
    int x = r | s;
    int y = t ^ u;
    int z = v + w + x + y;
    
    /* Mix with volatile accesses */
    a += vol_var1;
    b -= vol_var2;
    c *= vol_var1;
    d /= (vol_var2 + 1);
    
    return a + b + c + d + e + f + g + h + i + j + 
           k + l + m + n + o + p + q + r + s + t + 
           u + v + w + x + y + z;
}

int main(void) {
    clock_t start, end;
    double cpu_time_used;
    
    /* Initialize data */
    float *array_a = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float *array_b = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float *array_c = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array_a[i] = (float)rand() / RAND_MAX * 100.0f;
        array_b[i] = (float)rand() / RAND_MAX * 100.0f;
    }
    
    start = clock();
    
    /* Perform computations to trigger scheduling analysis */
    float total = 0.0f;
    for (int iter = 0; iter < ITERATIONS/100; iter++) {
        /* Call functions with different characteristics */
        high_pressure_loop(array_a, array_b, array_c, ARRAY_SIZE);
        
        float md_result = mixed_dependency(100);
        total += md_result;
        
        int ii_result = independent_instructions(iter);
        total += (float)ii_result;
        
        /* Use results to prevent dead code elimination */
        array_c[iter % ARRAY_SIZE] += total * 0.0001f;
        
        /* Update volatile variables to create new dependencies */
        vol_var1 = (vol_var1 * 13 + 17) % 100;
        vol_var2 = (vol_var2 * 7 + 23) % 100;
        vol_float1 = sinf((float)iter) + 1.0f;
        vol_float2 = cosf((float)iter) + 2.0f;
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    /* Compute checksum to verify correctness */
    float checksum = 0.0f;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += array_c[i];
    }
    
    printf("Result: %f\n", total);
    printf("Checksum: %f\n", checksum);
    printf("Time: %f seconds\n", cpu_time_used);
    
    free(array_a);
    free(array_b);
    free(array_c);
    
    return 0;
}
