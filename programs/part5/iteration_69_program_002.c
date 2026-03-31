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

/* Volatile variables to prevent optimization and create dependencies */
volatile int vol_var1 = 1;
volatile int vol_var2 = 2;
volatile float vol_float1 = 1.5f;
volatile float vol_float2 = 2.5f;

/* Function to create high register pressure with independent instructions */
__attribute__((noinline))
static void high_pressure_loop(float* restrict a, float* restrict b, 
                               float* restrict c, int size) {
    /* Many independent variables to create register pressure */
    float t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    float t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    for (int i = 0; i < size; i++) {
        /* Group 1: Independent floating-point operations */
        t1 = a[i] * 1.1f;
        t2 = b[i] * 2.2f;
        t3 = c[i] * 3.3f;
        t4 = t1 + t2;
        t5 = t2 - t3;
        
        /* Group 2: More independent operations */
        t6 = a[i] / 4.4f;  /* Division for longer latency */
        t7 = b[i] / 5.5f;
        t8 = c[i] / 6.6f;
        t9 = t6 * t7;
        t10 = t7 + t8;
        
        /* Group 3: Mix of operations */
        t11 = sqrtf(fabsf(t1));  /* Complex operation */
        t12 = sinf(t2);
        t13 = cosf(t3);
        t14 = t11 * t12;
        t15 = t12 + t13;
        
        /* Group 4: More temporaries */
        t16 = t4 * t5;
        t17 = t9 / t10;
        t18 = t14 - t15;
        t19 = t16 + t17;
        t20 = t18 * t19;
        
        /* Store results creating dependencies */
        a[i] = t4 + t20;
        b[i] = t5 * t19;
        c[i] = t10 / t18;
        
        /* Artificial dependency through volatile */
        if (vol_var1 > 0) {
            a[i] += vol_float1;
        }
    }
}

/* Function with mixed dependencies and resource conflicts */
__attribute__((noinline))
static void mixed_dependency(int* restrict arr1, int* restrict arr2, 
                             int* restrict arr3, int size) {
    int dep1 = vol_var1;  /* Start dependency chain */
    int dep2 = vol_var2;
    
    for (int i = 0; i < size; i++) {
        /* Long dependency chain */
        int val1 = arr1[i] + dep1;
        int val2 = arr2[i] * dep2;
        int val3 = arr3[i] - val1;
        
        /* Independent computations that can be scheduled in parallel */
        int tmp1 = arr1[i] * 3;
        int tmp2 = arr2[i] << 2;
        int tmp3 = arr3[i] >> 1;
        int tmp4 = tmp1 ^ tmp2;
        int tmp5 = tmp2 | tmp3;
        int tmp6 = tmp3 & tmp1;
        
        /* Resource conflict: multiple divisions in sequence */
        if (val2 != 0) {
            tmp1 = val1 / (val2 + 1);  /* Integer division */
            tmp2 = val3 / (val1 + 1);
        }
        
        /* Memory operations with potential aliasing */
        arr1[i] = val1 + tmp4;
        arr2[i] = val2 ^ tmp5;
        arr3[i] = val3 & tmp6;
        
        /* Update dependencies for next iteration */
        dep1 = arr1[i] % 256;
        dep2 = arr2[i] % 128;
        
        /* Inline assembly to clobber registers and force spills */
        __asm__ volatile (
            "movl $0, %%eax\n\t"
            "movl $1, %%ebx\n\t"
            "movl $2, %%ecx\n\t"
            "movl $3, %%edx\n\t"
            "movl $4, %%esi\n\t"
            "movl $5, %%edi\n\t"
            : /* no outputs */
            : /* no inputs */
            : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
    }
}

/* Function with control flow creating priority differences */
__attribute__((noinline))
static int control_flow_test(int* data, int size) {
    int sum = 0;
    int product = 1;
    int xor_result = 0;
    
    for (int i = 0; i < size; i++) {
        /* Critical path operations */
        int val = data[i];
        
        /* Branch creates different priority paths */
        if (val > 1000) {
            /* High priority path - complex computation */
            sum += val * 2;
            product *= (val % 100) + 1;
            xor_result ^= val;
            
            /* More operations on critical path */
            sum = (sum << 3) | (sum >> 29);  /* Rotate */
            product = product + (product >> 4);
        } else if (val > 500) {
            /* Medium priority path */
            sum += val / 2;
            product *= (val % 50) + 1;
            xor_result |= val;
        } else {
            /* Low priority path */
            sum += val;
            product = (product * 3) % 65536;
            xor_result &= val;
        }
        
        /* Independent operations that can be scheduled around branches */
        int tmp1 = val * val;
        int tmp2 = tmp1 + i;
        int tmp3 = tmp2 >> 4;
        data[i] = tmp3;
        
        /* Volatile access creates scheduling barrier */
        if (vol_var2 > 0) {
            sum += vol_var1;
        }
    }
    
    return sum + product + xor_result;
}

/* Main performance-critical computation */
int main() {
    /* Initialize with random data */
    srand(time(NULL));
    
    float* farr1 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* farr2 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* farr3 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    
    int* iarr1 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* iarr2 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* iarr3 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        farr1[i] = (float)rand() / RAND_MAX * 100.0f;
        farr2[i] = (float)rand() / RAND_MAX * 100.0f;
        farr3[i] = (float)rand() / RAND_MAX * 100.0f;
        iarr1[i] = rand() % 2000;
        iarr2[i] = rand() % 2000;
        iarr3[i] = rand() % 2000;
    }
    
    int total_result = 0;
    
    /* Hot loop that will be scheduled */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Modify volatile to affect scheduling */
        vol_var1 = (iter % 256);
        vol_var2 = (iter % 128) + 1;
        vol_float1 = (float)(iter % 100) * 0.1f;
        vol_float2 = (float)(iter % 200) * 0.05f;
        
        /* Call functions with different scheduling characteristics */
        high_pressure_loop(farr1, farr2, farr3, ARRAY_SIZE);
        mixed_dependency(iarr1, iarr2, iarr3, ARRAY_SIZE);
        total_result += control_flow_test(iarr1, ARRAY_SIZE);
        
        /* Prevent loop invariant code motion */
        if (iter % 1000 == 0) {
            for (int i = 0; i < ARRAY_SIZE; i++) {
                farr1[i] += 0.001f;
                iarr1[i] ^= iter;
            }
        }
    }
    
    /* Final computation to prevent dead code elimination */
    float final_sum = 0.0f;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum += farr1[i] + farr2[i] + farr3[i];
    }
    
    total_result += (int)final_sum;
    
    printf("Result: %d (checksum to prevent optimization)\n", total_result);
    
    free(farr1);
    free(farr2);
    free(farr3);
    free(iarr1);
    free(iarr2);
    free(iarr3);
    
    return 0;
}
