/* test_sched_coverage.c
 * 
 * Compile with scheduling debug flags to trigger haifa-sched.cc uncovered lines:
 * 1. Basic: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test
 * 2. Register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test
 * 3. With delays: gcc -O2 -fsched-verbose=5 -fno-schedule-insns -fno-schedule-insns2 -march=native test_sched_coverage.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100000

/* Volatile variables to prevent optimizations and create dependencies */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.0f, vol_f2 = 2.0f, vol_f3 = 3.0f;

/* Function to create high register pressure with many live variables */
double high_pressure_loop(double *arr1, double *arr2, double *arr3, int n) {
    /* Many register-tied variables to create pressure */
    double r0 = arr1[0], r1 = arr1[1], r2 = arr1[2], r3 = arr1[3];
    double r4 = arr2[0], r5 = arr2[1], r6 = arr2[2], r7 = arr2[3];
    double r8 = arr3[0], r9 = arr3[1], r10 = arr3[2], r11 = arr3[3];
    double r12 = vol_f1, r13 = vol_f2, r14 = vol_f3;
    
    /* Independent instruction groups - scheduler will have multiple candidates */
    for (int i = 0; i < n; i++) {
        /* Group 1: Independent FP operations */
        r0 = r0 * r4 + r8;
        r1 = r1 * r5 + r9;
        r2 = r2 * r6 + r10;
        r3 = r3 * r7 + r11;
        
        /* Group 2: More independent operations */
        r4 = r4 / (r0 + 1.0);
        r5 = r5 / (r1 + 1.0);
        r6 = r6 / (r2 + 1.0);
        r7 = r7 / (r3 + 1.0);
        
        /* Group 3: Cross dependencies to create priority differences */
        r8 = r12 * r0 - r4;
        r9 = r13 * r1 - r5;
        r10 = r14 * r2 - r6;
        r11 = (r3 - r7) * vol_f1;
        
        /* Use volatile to force memory dependencies */
        r12 += vol_f1;
        r13 += vol_f2;
        r14 += vol_f3;
        
        /* Mix integer operations to compete for ALUs */
        vol_a = (int)(r0 * 100);
        vol_b = (int)(r1 * 100);
        vol_c = (int)(r2 * 100);
        vol_d = (int)(r3 * 100);
    }
    
    /* Force all results to be used */
    return r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + 
           r8 + r9 + r10 + r11 + r12 + r13 + r14;
}

/* Function with artificial delays and resource conflicts */
float mixed_dependency_chain(int iterations) {
    float f1 = 1.0f, f2 = 2.0f, f3 = 3.0f, f4 = 4.0f;
    float f5 = 5.0f, f6 = 6.0f, f7 = 7.0f, f8 = 8.0f;
    int i1 = 1, i2 = 2, i3 = 3, i4 = 4;
    
    /* Long latency operations (FP divide) */
    for (int i = 0; i < iterations; i++) {
        /* These divides create long latency - potential for delay= output */
        f1 = f1 / f2;
        f3 = f3 / f4;
        f5 = f5 / f6;
        f7 = f7 / f8;
        
        /* Independent integer chains - scheduler has choices */
        i1 = i1 * i2 + vol_a;
        i2 = i2 * i3 + vol_b;
        i3 = i3 * i4 + vol_c;
        i4 = i4 * i1 + vol_d;
        
        /* Memory operations with aliasing potential */
        f2 = f2 * vol_f1;
        f4 = f4 * vol_f2;
        f6 = f6 * vol_f3;
        f8 = f8 * vol_f1;
        
        /* Cross-type dependencies */
        f1 += (float)i1 * 0.1f;
        f3 += (float)i2 * 0.1f;
        f5 += (float)i3 * 0.1f;
        f7 += (float)i4 * 0.1f;
        
        /* Inline asm to clobber registers and force spills */
        __asm__ volatile (
            "movl $0, %%eax\n"
            "movl $0, %%ebx\n"
            "movl $0, %%ecx\n"
            "movl $0, %%edx\n"
            : /* no outputs */
            : /* no inputs */
            : "eax", "ebx", "ecx", "edx", "memory"
        );
    }
    
    return f1 + f3 + f5 + f7 + f2 + f4 + f6 + f8 + i1 + i2 + i3 + i4;
}

/* Function with completely independent instructions for candidate selection */
void independent_instruction_groups(int n) {
    double a1 = 1.0, a2 = 2.0, a3 = 3.0, a4 = 4.0;
    double b1 = 5.0, b2 = 6.0, b3 = 7.0, b4 = 8.0;
    double c1 = 9.0, c2 = 10.0, c3 = 11.0, c4 = 12.0;
    
    /* These groups are independent - scheduler will see many candidates */
    for (int i = 0; i < n; i++) {
        /* Group A - independent from B and C */
        a1 = a1 * a2 + i;
        a2 = a2 * a3 + i;
        a3 = a3 * a4 + i;
        a4 = a4 * a1 + i;
        
        /* Group B - independent from A and C */
        b1 = b1 / (b2 + 1.0);
        b2 = b2 / (b3 + 1.0);
        b3 = b3 / (b4 + 1.0);
        b4 = b4 / (b1 + 1.0);
        
        /* Group C - independent from A and B */
        c1 = c1 - c2 * 0.5;
        c2 = c2 - c3 * 0.5;
        c3 = c3 - c4 * 0.5;
        c4 = c4 - c1 * 0.5;
        
        /* Minimal dependency to prevent dead code elimination */
        vol_a = (int)(a1 + b1 + c1);
    }
}

/* Main driver that creates the scheduling scenarios */
int main() {
    double *arr1 = malloc(ARRAY_SIZE * sizeof(double));
    double *arr2 = malloc(ARRAY_SIZE * sizeof(double));
    double *arr3 = malloc(ARRAY_SIZE * sizeof(double));
    
    /* Initialize with some data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = (double)(i % 100) * 0.1;
        arr2[i] = (double)((i + 1) % 100) * 0.2;
        arr3[i] = (double)((i + 2) % 100) * 0.3;
    }
    
    double total = 0.0;
    
    /* Run multiple iterations to ensure hot code scheduling */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call high pressure function - triggers register pressure modeling */
        total += high_pressure_loop(arr1, arr2, arr3, 10);
        
        /* Call mixed dependency function - triggers delay calculations */
        total += mixed_dependency_chain(5);
        
        /* Call independent groups function - gives scheduler many candidates */
        independent_instruction_groups(5);
        
        /* Prevent loop unrolling from simplifying too much */
        if (iter % 100 == 0) {
            vol_f1 += 0.001f;
            vol_f2 += 0.002f;
            vol_f3 += 0.003f;
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %f\n", total);
    printf("Volatiles: %d %d %d %d\n", vol_a, vol_b, vol_c, vol_d);
    
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
