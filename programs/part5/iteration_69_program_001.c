/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test_prog
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test_prog_pressure
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1024
#define ITERATIONS 100000

/* Volatile variables to prevent optimizations and create dependencies */
volatile int vol_var1 = 1;
volatile int vol_var2 = 2;
volatile float vol_float1 = 1.5f;
volatile float vol_float2 = 2.5f;

/* Function to create high register pressure with independent instructions */
void high_pressure_loop(float *a, float *b, float *c, int n) {
    /* Many independent floating point operations to create scheduling candidates */
    for (int i = 0; i < n; i++) {
        /* Group 1: Independent FP operations */
        float t1 = a[i] * b[i];
        float t2 = a[i] / (b[i] + 0.001f);  /* Division creates longer latency */
        float t3 = b[i] * c[i];
        float t4 = c[i] / (a[i] + 0.001f);
        
        /* Group 2: More independent operations */
        float t5 = t1 + t2;
        float t6 = t3 - t4;
        float t7 = t1 * t3;
        float t8 = t2 / t4;
        
        /* Group 3: Mix with volatile to force ordering */
        t5 += vol_float1;
        t6 -= vol_float2;
        
        /* Group 4: More temporaries for register pressure */
        float t9 = t5 * t6;
        float t10 = t7 / t8;
        float t11 = t9 + t10;
        float t12 = t9 - t10;
        
        /* Store results creating memory dependencies */
        a[i] = t11 + t12;
        b[i] = t11 - t12;
        c[i] = t9 * t10;
        
        /* Inline assembly to clobber registers and force spills */
        asm volatile (
            "mov $0, %%eax\n\t"
            "mov $0, %%ebx\n\t"
            "mov $0, %%ecx\n\t"
            "mov $0, %%edx\n\t"
            "mov $0, %%esi\n\t"
            "mov $0, %%edi\n\t"
            :
            :
            : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
    }
}

/* Function with mixed dependencies and resource conflicts */
void mixed_dependency(int *arr1, int *arr2, int *arr3, int n) {
    int dep1 = vol_var1;  /* Volatile dependency */
    int dep2 = vol_var2;
    
    for (int i = 0; i < n; i++) {
        /* Create artificial dependency chain */
        int val1 = arr1[i] + dep1;
        int val2 = arr2[i] * dep2;
        int val3 = val1 ^ val2;  /* XOR creates different functional unit usage */
        
        /* Independent operations that can be scheduled in parallel */
        int ind1 = arr3[i] << 2;
        int ind2 = arr1[i] >> 1;
        int ind3 = arr2[i] & 0xFF;
        int ind4 = arr3[i] | 0xAA;
        
        /* Mix with floating point to create resource conflicts */
        float f1 = (float)val1 * 1.1f;
        float f2 = (float)val2 / 2.2f;  /* FP division has longer latency */
        
        /* More operations with varying latencies */
        int val4 = val3 + ind1;
        int val5 = ind2 - ind3;
        float f3 = f1 + f2;
        
        /* Volatile access creates memory barrier effect */
        dep1 = vol_var1 + i;
        dep2 = vol_var2 - i;
        
        /* Store results with address calculations */
        arr1[i] = val4 + (int)f3;
        arr2[i] = val5 * dep1;
        arr3[i] = (int)(f1 * f2) ^ dep2;
        
        /* Another inline assembly to clobber FP registers */
        asm volatile (
            "fldz\n\t"
            "fld1\n\t"
            "faddp %%st, %%st(1)\n\t"
            : 
            : 
            : "st", "st(1)", "memory"
        );
    }
}

/* Function with control flow to create priority differences */
void control_flow_priority(float *data, int n) {
    float acc1 = 0.0f, acc2 = 0.0f, acc3 = 0.0f, acc4 = 0.0f;
    float acc5 = 0.0f, acc6 = 0.0f, acc7 = 0.0f, acc8 = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Many accumulators to increase register pressure */
        float val = data[i];
        
        /* Conditional operations create different control paths */
        if (val > 0.5f) {
            acc1 += val * 1.1f;
            acc2 -= val * 0.9f;
            /* Critical path operations */
            acc3 = acc1 * acc2;
            acc4 = acc1 / (acc2 + 0.001f);  /* Division in critical path */
        } else {
            acc5 += val * 2.1f;
            acc6 -= val * 1.9f;
            /* Less critical operations */
            acc7 = acc5 * acc6;
            acc8 = acc5 / (acc6 + 0.001f);
        }
        
        /* Loop-carried dependency */
        if (i % 3 == 0) {
            acc1 = acc3 + acc4;
            vol_var1++;  /* Volatile access creates delay */
        } else if (i % 3 == 1) {
            acc5 = acc7 + acc8;
            vol_var2++;  /* Another volatile access */
        }
        
        /* Independent computations that can be scheduled together */
        float tmp1 = acc1 * acc5;
        float tmp2 = acc2 * acc6;
        float tmp3 = acc3 * acc7;
        float tmp4 = acc4 * acc8;
        
        /* Use results to prevent elimination */
        data[i] = tmp1 + tmp2 + tmp3 + tmp4;
    }
}

/* Main computational kernel */
void compute_kernel() {
    static float array1[SIZE], array2[SIZE], array3[SIZE];
    static int iarray1[SIZE], iarray2[SIZE], iarray3[SIZE];
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (float)(i % 100) * 0.1f;
        array2[i] = (float)((i + 1) % 100) * 0.2f;
        array3[i] = (float)((i + 2) % 100) * 0.3f;
        iarray1[i] = i;
        iarray2[i] = i * 2;
        iarray3[i] = i * 3;
    }
    
    /* Perform multiple passes to ensure scheduling analysis */
    for (int iter = 0; iter < ITERATIONS / 1000; iter++) {
        high_pressure_loop(array1, array2, array3, SIZE);
        mixed_dependency(iarray1, iarray2, iarray3, SIZE);
        control_flow_priority(array1, SIZE);
        
        /* Mix data between passes */
        for (int i = 0; i < SIZE; i++) {
            array2[i] = array1[i] * 0.9f;
            array3[i] = array2[i] * 1.1f;
        }
    }
}

int main() {
    clock_t start, end;
    double cpu_time_used;
    
    start = clock();
    
    /* Call the kernel multiple times */
    for (int run = 0; run < 10; run++) {
        compute_kernel();
    }
    
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    /* Use results to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < SIZE; i++) {
        checksum += (float)iarray1[i] + (float)iarray2[i] + (float)iarray3[i];
    }
    
    printf("Scheduling test completed in %.2f seconds\n", cpu_time_used);
    printf("Checksum: %.2f\n", checksum);
    
    return 0;
}
