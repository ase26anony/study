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
    /* Many independent variables to create register pressure */
    float t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    float t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    
    for (int i = 0; i < size; i++) {
        /* Group 1: Independent floating point operations */
        t1 = a[i] * b[i];
        t2 = a[i] + b[i];
        t3 = a[i] - b[i];
        t4 = b[i] * vol_float1;
        t5 = t1 / (vol_float2 + 0.001f);  /* Division for longer latency */
        
        /* Group 2: More independent operations */
        t6 = t2 * t3;
        t7 = t4 + t5;
        t8 = t6 - t7;
        t9 = t8 * 1.1f;
        t10 = t9 / 2.0f;
        
        /* Group 3: Integer operations mixed in */
        i1 = (int)t1;
        i2 = (int)t2;
        i3 = i1 * vol_var1;
        i4 = i2 + vol_var2;
        i5 = i3 - i4;
        i6 = i5 * 2;
        
        /* Group 4: More floating point with dependencies */
        t11 = t10 + (float)i6;
        t12 = t11 * 0.5f;
        t13 = t12 / 3.14159f;  /* Another division for latency */
        t14 = sinf(t13);       /* Function call for scheduling complexity */
        
        /* Group 5: Memory operations with potential aliasing */
        t15 = c[i] * t14;
        t16 = c[size - i - 1] + t15;
        t17 = t16 * a[i];
        t18 = t17 - b[i];
        
        /* Group 6: Final computations */
        t19 = t18 * t13;
        t20 = t19 / t14;
        
        /* Store results creating write pressure */
        a[i] = t10 + t20;
        b[i] = t13 + t15;
        c[i] = t18 + t19;
        
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

/* Function with artificial dependencies and resource conflicts */
__attribute__((noinline))
static void mixed_dependency(int *restrict arr1, int *restrict arr2, int size) {
    int dep1 = vol_var1;
    int dep2 = vol_var2;
    
    /* Create a chain of dependencies */
    for (int i = 0; i < size; i++) {
        /* Long dependency chain */
        dep1 = dep1 * arr1[i] + dep2;
        dep2 = dep2 * arr2[i] - dep1;
        
        /* Independent computations that can be scheduled in parallel */
        int tmp1 = arr1[i] * 3;
        int tmp2 = arr2[i] * 5;
        int tmp3 = arr1[size - i - 1] * 7;
        int tmp4 = arr2[size - i - 1] * 11;
        
        /* Mix with volatile accesses for scheduling delays */
        tmp1 += vol_var1;
        tmp2 -= vol_var2;
        
        /* Floating point operations to compete for different functional units */
        float ftmp1 = (float)tmp1 * 1.5f;
        float ftmp2 = (float)tmp2 / 2.5f;  /* Division for latency */
        float ftmp3 = ftmp1 + ftmp2;
        float ftmp4 = ftmp1 - ftmp2;
        
        /* More independent integer ops */
        int tmp5 = tmp3 * tmp4;
        int tmp6 = tmp5 + dep1;
        int tmp7 = tmp6 - dep2;
        
        /* Store results with write-after-write dependencies */
        arr1[i] = tmp1 + tmp7;
        arr2[i] = tmp2 + (int)ftmp3;
        
        /* Conditional to create control flow and priority differences */
        if (arr1[i] > 1000) {
            arr1[i] = arr1[i] / 2;  /* Division for delay */
            vol_var1 = arr1[i];     /* Volatile write creates barrier */
        } else {
            arr2[i] = arr2[i] * 2;
            vol_var2 = arr2[i];
        }
    }
}

/* Function with many independent instructions for candidate selection */
__attribute__((noinline))
static void independent_instructions(float *restrict out, 
                                     const float *restrict in1,
                                     const float *restrict in2,
                                     int size) {
    /* This function is designed to create many independent instructions
     * that the scheduler can reorder, giving it multiple candidates */
    for (int i = 0; i < size; i += 4) {
        /* Unrolled loop with independent operations */
        float r1 = in1[i] * in2[i];
        float r2 = in1[i+1] + in2[i+1];
        float r3 = in1[i+2] - in2[i+2];
        float r4 = in1[i+3] / (in2[i+3] + 0.001f);
        
        float r5 = r1 * 1.1f;
        float r6 = r2 * 2.2f;
        float r7 = r3 * 3.3f;
        float r8 = r4 * 4.4f;
        
        float r9 = r5 + r6;
        float r10 = r7 - r8;
        float r11 = r9 * r10;
        float r12 = r10 / (r9 + 0.001f);
        
        float r13 = sqrtf(fabsf(r11));  /* Function call for complexity */
        float r14 = sqrtf(fabsf(r12));
        
        float r15 = r13 * r14;
        float r16 = r13 + r14;
        
        out[i] = r15;
        out[i+1] = r16;
        out[i+2] = r15 * r16;
        out[i+3] = r15 / (r16 + 0.001f);
    }
}

int main() {
    /* Initialize with random data */
    srand(time(NULL));
    
    float *array1 = malloc(ARRAY_SIZE * sizeof(float));
    float *array2 = malloc(ARRAY_SIZE * sizeof(float));
    float *array3 = malloc(ARRAY_SIZE * sizeof(float));
    float *array4 = malloc(ARRAY_SIZE * sizeof(float));
    
    int *int_array1 = malloc(ARRAY_SIZE * sizeof(int));
    int *int_array2 = malloc(ARRAY_SIZE * sizeof(int));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (float)rand() / RAND_MAX * 100.0f;
        array2[i] = (float)rand() / RAND_MAX * 100.0f;
        array3[i] = (float)rand() / RAND_MAX * 100.0f;
        array4[i] = 0.0f;
        int_array1[i] = rand() % 1000;
        int_array2[i] = rand() % 1000;
    }
    
    printf("Starting scheduling test...\n");
    
    /* Perform many iterations to ensure hot code scheduling */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call functions with different scheduling characteristics */
        high_pressure_loop(array1, array2, array3, ARRAY_SIZE);
        mixed_dependency(int_array1, int_array2, ARRAY_SIZE);
        independent_instructions(array4, array1, array2, ARRAY_SIZE);
        
        /* Prevent dead code elimination */
        vol_var1 = (int)array1[0];
        vol_var2 = (int)array2[0];
    }
    
    /* Compute checksum to ensure all computations are used */
    float checksum = 0.0f;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += array1[i] + array2[i] + array3[i] + array4[i] 
                  + int_array1[i] + int_array2[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Test completed.\n");
    
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    free(int_array1);
    free(int_array2);
    
    return 0;
}
