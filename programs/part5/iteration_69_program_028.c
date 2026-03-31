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
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.0f, vol_f2 = 2.0f, vol_f3 = 3.0f;

/* Function to create high register pressure with many live variables */
double high_pressure_loop(double *arr, int size) {
    double sum = 0.0;
    double t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    double t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    /* Unrolled loop with many independent computations */
    for (int i = 0; i < size - 7; i += 8) {
        /* Group 1: Independent floating point operations */
        t1 = arr[i] * arr[i] + vol_f1;
        t2 = arr[i+1] * arr[i+1] + vol_f2;
        t3 = arr[i+2] * arr[i+2] + vol_f3;
        t4 = arr[i+3] * arr[i+3] + vol_f1;
        
        /* Group 2: More independent operations */
        t5 = sqrt(t1) * vol_f2;
        t6 = sqrt(t2) * vol_f3;
        t7 = sqrt(t3) * vol_f1;
        t8 = sqrt(t4) * vol_f2;
        
        /* Group 3: Mix of operations to create different priorities */
        t9 = t1 / (vol_f3 + 0.001f);  /* Division has higher latency */
        t10 = t2 / (vol_f1 + 0.001f);
        t11 = t3 / (vol_f2 + 0.001f);
        t12 = t4 / (vol_f3 + 0.001f);
        
        /* Group 4: Integer operations mixed with float */
        t13 = (double)((int)t1 * vol_a);
        t14 = (double)((int)t2 * vol_b);
        t15 = (double)((int)t3 * vol_c);
        t16 = (double)((int)t4 * vol_d);
        
        /* Group 5: Memory operations with potential aliasing */
        t17 = arr[i] * t1 + arr[i+1];
        t18 = arr[i+1] * t2 + arr[i+2];
        t19 = arr[i+2] * t3 + arr[i+3];
        t20 = arr[i+3] * t4 + arr[i];
        
        /* Force all values to be used to prevent dead code elimination */
        sum += t5 + t6 + t7 + t8 + t9 + t10 + t11 + t12 +
               t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20;
    }
    
    return sum;
}

/* Function with artificial dependencies and resource conflicts */
void mixed_dependency(int *int_arr, float *float_arr, int size) {
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
    float ftmp1, ftmp2, ftmp3, ftmp4, ftmp5, ftmp6, ftmp7, ftmp8;
    
    /* Create chains of dependencies */
    tmp1 = vol_a;
    tmp2 = tmp1 * vol_b + int_arr[0];
    tmp3 = tmp2 / (vol_c + 1) + int_arr[1];
    tmp4 = tmp3 * vol_d - int_arr[2];
    
    /* Independent group that can be scheduled in parallel */
    tmp5 = int_arr[3] * int_arr[4];
    tmp6 = int_arr[5] * int_arr[6];
    tmp7 = int_arr[7] * int_arr[8];
    tmp8 = int_arr[9] * int_arr[10];
    
    /* Floating point operations with different latencies */
    ftmp1 = float_arr[0] * 1.234f;
    ftmp2 = float_arr[1] / 3.14159f;  /* Division has higher latency */
    ftmp3 = float_arr[2] + 2.718f;
    ftmp4 = float_arr[3] - 1.414f;
    
    /* More independent groups */
    ftmp5 = ftmp1 * ftmp2;
    ftmp6 = ftmp3 * ftmp4;
    ftmp7 = ftmp1 + ftmp3;
    ftmp8 = ftmp2 - ftmp4;
    
    /* Use inline assembly to clobber registers and create pressure */
    __asm__ volatile (
        "movl $0, %%eax\n\t"
        "movl $1, %%ebx\n\t"
        "movl $2, %%ecx\n\t"
        "movl $3, %%edx\n\t"
        "movl $4, %%esi\n\t"
        "movl $5, %%edi\n\t"
        :
        :
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* Force all values to be used */
    int_arr[0] = tmp1 + tmp2 + tmp3 + tmp4 + tmp5 + tmp6 + tmp7 + tmp8;
    float_arr[0] = ftmp1 + ftmp2 + ftmp3 + ftmp4 + ftmp5 + ftmp6 + ftmp7 + ftmp8;
}

/* Function with control flow to create priority differences */
int control_flow_test(int *arr, int size, int threshold) {
    int result = 0;
    
    for (int i = 0; i < size; i++) {
        /* Critical path with dependencies */
        int x = arr[i];
        int y = x * vol_a;
        int z = y + vol_b;
        
        /* Branch creates different scheduling priorities */
        if (z > threshold) {
            /* Critical path inside branch */
            int a = z * 2;
            int b = a - vol_c;
            result += b;
            
            /* Independent operations in branch */
            int c = arr[(i + 1) % size] * 3;
            int d = arr[(i + 2) % size] * 4;
            result += c + d;
        } else {
            /* Different operations in else path */
            int e = z / 2;
            int f = e + vol_d;
            result += f;
            
            /* More independent operations */
            int g = arr[(i + 3) % size] * 5;
            int h = arr[(i + 4) % size] * 6;
            result += g + h;
        }
        
        /* Loop-carried dependency */
        vol_a = (vol_a + 1) & 0xFF;
    }
    
    return result;
}

int main() {
    double *double_arr = (double*)malloc(ARRAY_SIZE * sizeof(double));
    int *int_arr = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_arr = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double_arr[i] = (double)rand() / RAND_MAX * 100.0;
        int_arr[i] = rand() % 1000;
        float_arr[i] = (float)rand() / RAND_MAX * 50.0f;
    }
    
    double total_sum = 0.0;
    int int_result = 0;
    
    /* Performance-critical loop that will be scheduled */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call functions with different characteristics */
        total_sum += high_pressure_loop(double_arr, ARRAY_SIZE);
        mixed_dependency(int_arr, float_arr, ARRAY_SIZE);
        int_result += control_flow_test(int_arr, ARRAY_SIZE, 500);
        
        /* Modify data slightly to prevent complete optimization */
        double_arr[iter % ARRAY_SIZE] += 0.001;
        int_arr[iter % ARRAY_SIZE] += 1;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Total sum: %f\n", total_sum);
    printf("Int result: %d\n", int_result);
    printf("Checksum: %f\n", total_sum + int_result);
    
    free(double_arr);
    free(int_arr);
    free(float_arr);
    
    return 0;
}
