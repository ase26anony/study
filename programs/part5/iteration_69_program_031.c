/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test_prog
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test_prog_pressure
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define ITERATIONS 100000
#define ARRAY_SIZE 1024

/* Volatile variables to prevent optimization and create dependencies */
volatile int vol_a = 1, vol_b = 2, vol_c = 3;
volatile float vol_f1 = 1.0f, vol_f2 = 2.0f, vol_f3 = 3.0f;

/* Function to create high register pressure with independent instructions */
__attribute__((noinline))
void high_pressure_loop(float* restrict arr1, float* restrict arr2, 
                        float* restrict arr3, float* restrict out) {
    /* Many independent variables to create register pressure */
    float t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    float t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    
    /* Force scheduler to consider multiple candidates by creating 
     * sequences of independent operations */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Group 1: Independent floating point operations */
        t1 = arr1[i] * 1.1f;
        t2 = arr2[i] + 2.2f;
        t3 = arr3[i] - 3.3f;
        t4 = t1 / 4.4f;  /* Division has longer latency */
        t5 = t2 * t3;
        
        /* Group 2: More independent operations */
        t6 = arr1[i] * arr2[i];
        t7 = arr2[i] * arr3[i];
        t8 = arr3[i] * arr1[i];
        t9 = t6 + t7;
        t10 = t8 - t9;
        
        /* Group 3: Integer operations mixed with float */
        i1 = (int)t1;
        i2 = (int)t2;
        i3 = i1 * i2;
        i4 = i1 + i2;
        i5 = i3 - i4;
        
        /* Group 4: More operations creating register pressure */
        t11 = t4 * t5;
        t12 = t6 / t7;  /* Another division with longer latency */
        t13 = t8 + t9;
        t14 = t10 - t11;
        t15 = t12 * t13;
        
        /* Group 5: Use volatile to create memory dependencies */
        t16 = t14 * vol_f1;
        t17 = t15 + vol_f2;
        t18 = vol_f3 / t16;  /* Division depends on volatile */
        
        /* Group 6: Final computations with mixed operations */
        t19 = t17 * t18;
        t20 = t19 - t14;
        
        /* Store result, preventing dead code elimination */
        out[i] = t20 + (float)i5;
        
        /* Artificial dependency chain to create delays */
        vol_f1 = t1 * 0.5f;
        vol_f2 = t2 * 0.5f;
        vol_f3 = t3 * 0.5f;
    }
}

/* Function with mixed dependencies and resource conflicts */
__attribute__((noinline))
void mixed_dependency(int* restrict data_int, float* restrict data_float,
                      double* restrict data_double) {
    /* Create artificial resource conflicts */
    double d1, d2, d3, d4, d5;
    float f1, f2, f3, f4, f5;
    int i1, i2, i3, i4, i5;
    
    /* Long latency operations mixed with short ones */
    for (int i = 0; i < ARRAY_SIZE / 2; i++) {
        /* Floating point divide (long latency) */
        d1 = data_double[i] / 3.1415926535;
        
        /* Independent integer operations */
        i1 = data_int[i] * 2;
        i2 = data_int[i + 1] + 3;
        i3 = i1 - i2;
        
        /* More floating point */
        f1 = data_float[i] * 2.0f;
        f2 = data_float[i + 1] / 1.5f;  /* Another division */
        
        /* Dependency chain */
        d2 = d1 * 2.0;
        d3 = d2 + (double)f1;
        d4 = d3 / (double)f2;  /* Division depends on previous results */
        
        /* Integer operations that can be scheduled independently */
        i4 = i3 * vol_a;  /* Volatile dependency */
        i5 = i4 + vol_b;
        
        /* Store results with volatile to prevent reordering */
        vol_c = i5;
        data_int[i] = i5;
        
        /* More mixed operations */
        f3 = f1 + f2;
        f4 = f3 * vol_f1;
        f5 = f4 - (float)d4;
        
        data_float[i] = f5;
        data_double[i] = d4;
        
        /* Inline assembly to clobber registers and force spills */
        __asm__ volatile (
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

/* Function with control flow to create priority differences */
__attribute__((noinline))
int control_flow_priority(int* data, int threshold) {
    int sum = 0;
    int prod = 1;
    int diff = 0;
    
    /* Complex control flow creates different instruction priorities */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Independent operations */
        int temp1 = data[i] * 2;
        int temp2 = data[i] + 3;
        int temp3 = temp1 - temp2;
        
        /* Control flow creates critical path differences */
        if (data[i] > threshold) {
            /* Critical path operations */
            sum += temp3;
            prod *= (temp1 + 1);
            
            /* More operations on critical path */
            diff -= temp2;
            vol_a = sum;  /* Volatile store creates memory dependency */
        } else {
            /* Less critical operations */
            sum -= temp3;
            prod /= (temp2 + 1);  /* Integer division */
            
            /* Independent operations */
            diff += temp1;
            vol_b = diff;
        }
        
        /* Loop-carried dependency */
        threshold += (sum % 10);
    }
    
    return sum + prod + diff;
}

/* Main function that creates the hot loops */
int main() {
    /* Allocate and initialize data */
    float* arr1 = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float* arr2 = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float* arr3 = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float* out = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    
    int* int_data = (int*)aligned_alloc(32, ARRAY_SIZE * sizeof(int));
    float* float_data = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    double* double_data = (double*)aligned_alloc(32, ARRAY_SIZE * sizeof(double));
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = (float)rand() / RAND_MAX;
        arr2[i] = (float)rand() / RAND_MAX;
        arr3[i] = (float)rand() / RAND_MAX;
        int_data[i] = rand() % 100;
        float_data[i] = (float)rand() / RAND_MAX;
        double_data[i] = (double)rand() / RAND_MAX;
    }
    
    int total = 0;
    
    /* Performance-critical loop that will be scheduled */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call functions with different scheduling characteristics */
        high_pressure_loop(arr1, arr2, arr3, out);
        
        mixed_dependency(int_data, float_data, double_data);
        
        int result = control_flow_priority(int_data, iter % 50);
        total += result;
        
        /* Modify data slightly to prevent complete optimization */
        arr1[iter % ARRAY_SIZE] += 0.001f;
        int_data[iter % ARRAY_SIZE] += 1;
    }
    
    /* Use results to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += out[i] + (float)int_data[i] + (float)float_data[i];
    }
    
    printf("Result: total=%d, checksum=%f\n", total, checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(out);
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}
