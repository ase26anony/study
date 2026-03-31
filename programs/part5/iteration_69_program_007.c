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
    
    /* Unrolled loop with many independent calculations */
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
        
        /* Group 3: Mixed operations creating register pressure */
        t9 = t1 / t5 + t2 / t6;  /* Floating point divide - long latency */
        t10 = t3 / t7 + t4 / t8; /* Another divide */
        
        /* Group 4: More temporaries */
        t11 = t9 * t10;
        t12 = t9 + t10;
        t13 = t9 - t10;
        t14 = t11 * t12;
        
        /* Group 5: Even more temporaries to stress register allocator */
        t15 = t13 * vol_f1;
        t16 = t14 * vol_f2;
        t17 = t15 + t16;
        t18 = t15 - t16;
        t19 = t17 * t18;
        t20 = t17 / (t18 + 1.0); /* Potential divide by zero protection */
        
        sum += t19 + t20;
        
        /* Inline assembly to clobber registers and force spills */
        __asm__ volatile (
            "movq $0, %%rax\n"
            "movq $0, %%rbx\n"
            "movq $0, %%rcx\n"
            "movq $0, %%rdx\n"
            "movq $0, %%rsi\n"
            "movq $0, %%rdi\n"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "cc"
        );
    }
    
    return sum;
}

/* Function with artificial dependencies and resource conflicts */
int mixed_dependency(int *data, int size) {
    int result = 0;
    volatile int dep1 = vol_a, dep2 = vol_b;
    
    for (int i = 0; i < size; i++) {
        /* Create dependency chain */
        int x = data[i] + dep1;
        dep1 = x * vol_c;
        
        /* Independent parallel chain */
        int y = data[size - i - 1] + dep2;
        dep2 = y / (vol_d + 1);  /* Integer divide - variable latency */
        
        /* More independent operations that can be scheduled in parallel */
        int z1 = x * y;
        int z2 = x + y;
        int z3 = x - y;
        int z4 = y - x;
        
        /* Control flow to create priority differences */
        if (z1 > 1000) {
            result += z1 * 2;  /* Higher priority path */
        } else if (z1 < 100) {
            result += z1 / 2;  /* Lower priority path */
        } else {
            result += z1;      /* Medium priority */
        }
        
        /* More operations with different latencies */
        float f1 = (float)z2 * vol_f1;
        float f2 = (float)z3 * vol_f2;
        float f3 = f1 / (f2 + 0.001f);  /* Float divide - long latency */
        
        result += (int)(f3 * 100.0f);
        
        /* Memory operations with potential aliasing */
        data[i] = result;
        if (i > 0) {
            data[i-1] = data[i] + data[i-1];  /* Creates store-load dependency */
        }
    }
    
    return result;
}

/* Function with many independent instructions for candidate selection */
void independent_instructions(int *out, const int *in, int n) {
    /* Large basic block with no dependencies between groups */
    for (int i = 0; i < n; i++) {
        /* Group A - completely independent from Group B */
        int a1 = in[i] * 2;
        int a2 = in[i] + 5;
        int a3 = in[i] - 3;
        int a4 = in[i] / 2;
        int a5 = in[i] % 7;
        
        /* Group B - independent from Group A */
        int b1 = in[n-i-1] * 3;
        int b2 = in[n-i-1] + 7;
        int b3 = in[n-i-1] - 2;
        int b4 = in[n-i-1] / 3;
        int b5 = in[n-i-1] % 5;
        
        /* Group C - mixes results from A and B */
        int c1 = a1 + b1;
        int c2 = a2 - b2;
        int c3 = a3 * b3;
        int c4 = a4 + b4;
        int c5 = a5 - b5;
        
        /* Group D - more mixing */
        int d1 = c1 * c2;
        int d2 = c3 + c4;
        int d3 = c5 * 2;
        int d4 = d1 - d2;
        int d5 = d3 + d4;
        
        out[i] = d5;
        
        /* Inline asm to create resource conflicts */
        __asm__ volatile (
            "movl $0, %%eax\n\t"
            "movl $0, %%ebx\n\t"
            "movl $0, %%ecx\n\t"
            "movl $0, %%edx\n\t"
            "addl $1, %%eax\n\t"
            "addl $2, %%ebx\n\t"
            "addl $3, %%ecx\n\t"
            "addl $4, %%edx"
            : /* no outputs */
            : /* no inputs */
            : "eax", "ebx", "ecx", "edx", "cc"
        );
    }
}

/* Main function that creates the scheduling scenario */
int main() {
    double *double_arr = (double*)malloc(ARRAY_SIZE * sizeof(double));
    int *int_arr = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *out_arr = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!double_arr || !int_arr || !out_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double_arr[i] = (double)rand() / RAND_MAX * 100.0;
        int_arr[i] = rand() % 1000;
    }
    
    double total_sum = 0.0;
    int int_result = 0;
    
    /* Perform many iterations to make the scheduler analyze hot code */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call functions that create different scheduling scenarios */
        total_sum += high_pressure_loop(double_arr, ARRAY_SIZE);
        int_result += mixed_dependency(int_arr, ARRAY_SIZE / 4);
        independent_instructions(out_arr, int_arr, ARRAY_SIZE / 8);
        
        /* Prevent optimization of loops */
        if (iter % 1000 == 0) {
            vol_a = (vol_a * 3) % 17;
            vol_b = (vol_b * 5) % 19;
            vol_c = (vol_c * 7) % 23;
            vol_d = (vol_d * 11) % 29;
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: sum=%f, int_result=%d, out[0]=%d\n", 
           total_sum, int_result, out_arr[0]);
    
    free(double_arr);
    free(int_arr);
    free(out_arr);
    
    return 0;
}
