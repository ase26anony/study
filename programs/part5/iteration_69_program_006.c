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
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.0f, vol_f2 = 2.0f, vol_f3 = 3.0f;

/* Function to create high register pressure with many live variables */
double high_pressure_loop(double *arr, int size) {
    /* Many register-tied variables to create pressure */
    double r0 = arr[0], r1 = arr[1], r2 = arr[2], r3 = arr[3];
    double r4 = arr[4], r5 = arr[5], r6 = arr[6], r7 = arr[7];
    double r8 = arr[8], r9 = arr[9], r10 = arr[10], r11 = arr[11];
    double r12 = arr[12], r13 = arr[13], r14 = arr[14], r15 = arr[15];
    
    double sum = 0.0;
    
    /* Unrolled loop with independent operations to create scheduling candidates */
    for (int i = 0; i < size; i += 16) {
        /* Group 1: Independent FP operations */
        r0 = r0 * r1 + r2;
        r1 = r1 * r3 - r4;
        r2 = r2 / r5 + r6;
        r3 = r3 * r7 - r8;
        
        /* Group 2: More independent operations */
        r4 = r4 + r9 * r10;
        r5 = r5 - r11 / r12;
        r6 = r6 * r13 + r14;
        r7 = r7 / r15 - r0;
        
        /* Group 3: Mix with volatile to force ordering */
        r8 = r8 * vol_f1 + r1;
        r9 = r9 - vol_f2 * r2;
        r10 = r10 / vol_f3 + r3;
        r11 = r11 * r4 - r5;
        
        /* Group 4: Long latency operations (divisions) */
        r12 = r12 / (r6 + 1.0);
        r13 = r13 / (r7 + 1.0);
        r14 = r14 / (r8 + 1.0);
        r15 = r15 / (r9 + 1.0);
        
        /* Use results to prevent dead code elimination */
        sum += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 +
               r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
        
        /* Rotate registers to keep them all live */
        double tmp = r0;
        r0 = r1; r1 = r2; r2 = r3; r3 = r4;
        r4 = r5; r5 = r6; r6 = r7; r7 = r8;
        r8 = r9; r9 = r10; r10 = r11; r11 = r12;
        r12 = r13; r13 = r14; r14 = r15; r15 = tmp;
    }
    
    return sum;
}

/* Function with artificial dependencies and resource conflicts */
float mixed_dependency(int iterations) {
    /* Start with volatile to create initial dependency */
    float f1 = vol_f1, f2 = vol_f2, f3 = vol_f3;
    int i1 = vol_a, i2 = vol_b, i3 = vol_c, i4 = vol_d;
    
    float result = 0.0f;
    
    for (int i = 0; i < iterations; i++) {
        /* Create data dependencies chains */
        f1 = f1 * 1.1f;          /* FP op 1 */
        f2 = f2 / 1.1f;          /* FP div (long latency) */
        f3 = f3 + f1;            /* Dependent on f1 */
        
        /* Integer operations that compete for ALUs */
        i1 = i1 * 3 + i2;
        i2 = i2 / 2 + i3;
        i3 = i3 * 5 - i4;
        i4 = i4 + i1;
        
        /* Memory operations with potential aliasing */
        vol_f1 = f1;             /* Volatile write - creates barrier */
        f2 = f2 + vol_f2;        /* Volatile read - creates dependency */
        
        /* More FP operations with dependencies */
        float f4 = f1 * f2;
        float f5 = f3 / f4;
        float f6 = f4 + f5;
        
        /* Use inline assembly to clobber registers and force spills */
        asm volatile (
            "movl %0, %%eax\n\t"
            "movl %1, %%ebx\n\t"
            "addl %%ebx, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "+r" (i1), "+r" (i2)
            :
            : "%eax", "%ebx", "cc"
        );
        
        /* Mix results */
        result += f6 + i1 + i2 + i3 + i4;
        
        /* Control flow variation to affect priorities */
        if (i % 17 == 0) {
            f1 = f2;
            i3 = i4;
        } else if (i % 23 == 0) {
            f2 = f3;
            i4 = i1;
        }
    }
    
    return result;
}

/* Function with completely independent instructions for candidate selection */
void independent_instructions(int *out1, int *out2, int *out3, int *out4) {
    int a = vol_a * 2;
    int b = vol_b + 3;
    int c = vol_c / 2;
    int d = vol_d - 1;
    
    /* These 8 instructions have no dependencies between groups */
    /* Group A - independent from others */
    int t1 = a * b;
    int t2 = c + d;
    
    /* Group B - independent from others */
    int t3 = a + 5;
    int t4 = b * 3;
    
    /* Group C - independent from others */
    int t5 = c - 2;
    int t6 = d / 2;
    
    /* Group D - independent from others */
    int t7 = t1 + 1;
    int t8 = t2 * 2;
    
    /* Final combinations */
    *out1 = t1 + t3 + t5 + t7;
    *out2 = t2 + t4 + t6 + t8;
    *out3 = t1 * t3 - t5;
    *out4 = t2 / (t4 + 1) + t6;
}

/* Main driver that creates hot loops for scheduler analysis */
int main() {
    srand(time(NULL));
    
    /* Initialize data arrays */
    double *arr = (double*)malloc(ARRAY_SIZE * sizeof(double));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    double total = 0.0;
    
    /* Performance-critical section that will be scheduled */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call functions that create different scheduling scenarios */
        total += high_pressure_loop(arr, ARRAY_SIZE);
        
        float fresult = mixed_dependency(100);
        total += fresult;
        
        int r1, r2, r3, r4;
        independent_instructions(&r1, &r2, &r3, &r4);
        total += r1 + r2 + r3 + r4;
        
        /* Modify volatile to break optimization patterns */
        vol_a = (vol_a * 3 + 7) % 97;
        vol_b = (vol_b * 5 + 11) % 101;
        vol_f1 = sin(total * 0.01);
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %f\n", total);
    
    free(arr);
    return 0;
}
