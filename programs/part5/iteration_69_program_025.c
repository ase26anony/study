/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test_sched_coverage
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test_sched_pressure
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1024
#define ITERS 100000

/* Volatile variables to prevent optimization and create artificial dependencies */
volatile int vol_var1, vol_var2, vol_var3;
volatile double vol_double1, vol_double2;

/* Function to create high register pressure with many live variables */
double high_pressure_loop(double *arr1, double *arr2, double *arr3, int n) {
    double sum = 0.0;
    double t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    double a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    double b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    
    /* Unrolled loop with many independent operations to create scheduling candidates */
    for (int i = 0; i < n; i += 10) {
        /* Group 1: Independent floating point operations */
        t1 = arr1[i] * arr2[i];
        t2 = arr1[i+1] / (arr2[i+1] + 1.0);  /* Division for longer latency */
        t3 = arr1[i+2] + arr2[i+2];
        t4 = arr1[i+3] - arr2[i+3];
        t5 = arr1[i+4] * arr2[i+4] * 1.5;
        
        /* Group 2: More independent operations */
        t6 = arr2[i] * arr3[i];
        t7 = arr2[i+1] / (arr3[i+1] + 1.0);
        t8 = arr2[i+2] + arr3[i+2];
        t9 = arr2[i+3] - arr3[i+3];
        t10 = arr2[i+4] * arr3[i+4] * 2.5;
        
        /* Cross dependencies to create priority differences */
        a1 = t1 + t6;
        a2 = t2 + t7;
        a3 = t3 + t8;
        a4 = t4 + t9;
        a5 = t5 + t10;
        
        /* More operations with mixed criticality */
        b1 = a1 * a2;
        b2 = a3 * a4;
        b3 = a5 * t1;
        b4 = t2 * t3;
        b5 = t4 * t5;
        
        /* Use volatile to force ordering and create delays */
        vol_double1 = b1;
        b6 = b2 + vol_double1;
        
        vol_double2 = b3;
        b7 = b4 + vol_double2;
        
        b8 = b5 * b6;
        b9 = b7 / (b8 + 1.0);  /* Another division for latency */
        b10 = b9 * 0.5;
        
        /* Accumulate results */
        sum += b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10;
        
        /* Inline assembly to clobber registers and increase pressure */
        __asm__ volatile (
            "movq $0, %%rax\n\t"
            "movq $0, %%rbx\n\t"
            "movq $0, %%rcx\n\t"
            "movq $0, %%rdx\n\t"
            "movq $0, %%rsi\n\t"
            "movq $0, %%rdi\n\t"
            "movq $0, %%r8\n\t"
            "movq $0, %%r9\n\t"
            "movq $0, %%r10\n\t"
            "movq $0, %%r11\n\t"
            "movq $0, %%r12\n\t"
            "movq $0, %%r13\n\t"
            "movq $0, %%r14\n\t"
            "movq $0, %%r15\n\t"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
        );
    }
    
    return sum;
}

/* Function with mixed dependencies and resource conflicts */
int mixed_dependency(int *arr_int, double *arr_double, int n) {
    int result = 0;
    int r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
    double d1, d2, d3, d4, d5;
    
    /* Create independent instruction groups for scheduler candidates */
    for (int i = 0; i < n; i++) {
        /* Group A: Integer operations with data dependencies */
        r1 = arr_int[i] * 3;
        r2 = r1 + i;
        r3 = r2 / 7;  /* Integer division for latency */
        
        /* Group B: Independent integer operations */
        r4 = arr_int[(i + 1) % n] << 2;
        r5 = arr_int[(i + 2) % n] >> 1;
        r6 = r4 ^ r5;
        
        /* Group C: Floating point operations competing for FP units */
        d1 = arr_double[i] * 1.234;
        d2 = arr_double[(i + 1) % n] / 5.678;  /* Division for FP latency */
        d3 = d1 + d2;
        
        /* Volatile accesses to create memory barriers and delays */
        vol_var1 = r3;
        r7 = vol_var1 + r6;
        
        vol_var2 = (int)d3;
        r8 = vol_var2 * 11;
        
        /* Conditional to create control flow and priority differences */
        if (r7 > r8) {
            r9 = r7 - r8;
            /* More operations in taken path */
            d4 = sin(d3);  /* Math function call for latency */
            d5 = cos(d4);
            r10 = (int)(d5 * 1000);
        } else {
            r9 = r8 - r7;
            /* Different operations in not-taken path */
            d4 = sqrt(fabs(d3));  /* Another math function */
            d5 = d4 * 2.0;
            r10 = (int)(d5 * 500);
        }
        
        /* Final accumulation with complex expression */
        result += (r9 * r10) / ((i % 10) + 1);
        
        /* Artificial resource conflict via inline assembly */
        __asm__ volatile (
            "mov $1000, %%ecx\n"
            "1:\n\t"
            "add $1, %%eax\n\t"
            "loop 1b\n\t"
            : "=a"(vol_var3)
            : "a"(result)
            : "ecx", "cc"
        );
        
        result = vol_var3;
    }
    
    return result;
}

/* Main function that creates the hot loops for scheduler analysis */
int main() {
    double *arr1 = malloc(SIZE * sizeof(double));
    double *arr2 = malloc(SIZE * sizeof(double));
    double *arr3 = malloc(SIZE * sizeof(double));
    int *arr_int = malloc(SIZE * sizeof(int));
    double *arr_double = malloc(SIZE * sizeof(double));
    
    /* Initialize with random data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (double)rand() / RAND_MAX * 100.0;
        arr2[i] = (double)rand() / RAND_MAX * 100.0;
        arr3[i] = (double)rand() / RAND_MAX * 100.0;
        arr_int[i] = rand() % 1000;
        arr_double[i] = (double)rand() / RAND_MAX * 200.0 - 100.0;
    }
    
    double total_sum = 0.0;
    int int_result = 0;
    
    /* Hot loop that will be heavily scheduled */
    for (int iter = 0; iter < ITERS; iter++) {
        /* Call high pressure function */
        total_sum += high_pressure_loop(arr1, arr2, arr3, SIZE);
        
        /* Call mixed dependency function */
        int_result += mixed_dependency(arr_int, arr_double, SIZE);
        
        /* Modify inputs slightly to prevent complete optimization */
        arr1[iter % SIZE] += 0.001;
        arr_int[iter % SIZE] += 1;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Total sum: %f\n", total_sum);
    printf("Integer result: %d\n", int_result);
    printf("Checksum: %f\n", total_sum + int_result);
    
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr_int);
    free(arr_double);
    
    return 0;
}
