/* sel-sched-dump-trigger.c
 * 
 * This program is specifically designed to trigger GCC's selective scheduler
 * debug dump logic, particularly the uncovered lines in sel-sched-dump.cc
 * that perform RTL instruction dumping with switch_dump()/restore_dump().
 *
 * Compilation recommendations for coverage:
 *   gcc -O3 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops \
 *       -fdump-rtl-all -c sel-sched-dump-trigger.c -o test.o
 *
 *   gcc -O2 -fschedule-insns -fschedule-insns2 \
 *       -fdump-rtl-sched1 -fdump-rtl-sched2 -c sel-sched-dump-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* ========== Function Attributes for Scheduling Control ========== */

__attribute__((hot, optimize("O3"), noinline))
static float hot_vectorized_loop(float* restrict a, float* restrict b, 
                                 float* restrict c, int n) {
    /* Mixed integer/float operations with RAW/WAR/WAW hazards */
    float sum = 0.0f;
    
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        /* Create RAW hazard: b depends on a */
        float t1 = a[i] * 2.0f;
        
        /* Memory barrier to split scheduling region */
        asm volatile("" ::: "memory");
        
        /* Create WAR hazard: reusing t1 */
        b[i] = t1 + 1.0f;
        t1 = b[i] * 3.0f;
        
        /* Create WAW hazard: multiple writes to c[i] */
        c[i] = t1 + a[i];
        c[i] = c[i] * 0.5f;  /* Second write to same location */
        
        /* Mixed operations challenge scheduler */
        sum += c[i] + (float)(i & 0xFF);
    }
    
    return sum;
}

__attribute__((cold, optimize("sched-pressure"), noinline))
static int cold_pointer_chasing(int* restrict data, int size) {
    /* Pointer chasing with complex dependencies */
    int result = 0;
    int* ptr = data;
    
    for (int i = 0; i < size; i++) {
        /* Load with variable latency pattern */
        int val = *ptr;
        
        /* Inline asm with register clobbering */
        asm volatile("nop" : : : "eax", "ebx", "ecx");
        
        /* Complex dependency chain */
        val = (val * 1103515245 + 12345) & 0x7fffffff;
        
        /* Store with address computation */
        *(ptr + (val % 16)) = val;
        
        /* Update pointer with stride */
        ptr = data + ((i * 17) & (size - 1));
        
        result ^= val;
    }
    
    return result;
}

__attribute__((optimize("O3"), noinline))
static double mixed_control_flow(double* arr, int n) {
    /* Complex control flow with switch and computed gotos */
    double total = 0.0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple early exit points */
        if (i > n - 10) {
            total += arr[i];
            continue;
        }
        
        /* Switch with sparse cases */
        switch (i % 13) {
            case 0:
                arr[i] = sin(arr[i]);
                break;
            case 1:
            case 2:
                arr[i] = cos(arr[i] * 0.5);
                break;
            case 5:
                /* Conditional move */
                arr[i] = (i & 1) ? arr[i] * 2.0 : arr[i] / 2.0;
                break;
            case 8:
                /* Nested ternary */
                arr[i] = (arr[i] > 0) ? 
                         ((i % 3) ? sqrt(arr[i]) : log(arr[i] + 1.0)) :
                         -arr[i];
                break;
            default:
                arr[i] = arr[i] * arr[i];
        }
        
        /* Memory barrier between dependent ops */
        asm volatile("" ::: "memory");
        
        /* Additional computation with hazard */
        double temp = arr[i];
        arr[i] = temp + (double)i * 0.01;
        total += arr[i] - temp;  /* WAR hazard */
    }
    
    return total;
}

__attribute__((optimize("O3")))
static void simd_unrolled_operations(int* restrict out, 
                                     const int* restrict in1,
                                     const int* restrict in2,
                                     int size) {
    /* SIMD-friendly loop with manual unrolling hints */
    int i = 0;
    
    #pragma GCC unroll 8
    for (; i + 3 < size; i += 4) {
        /* Vectorizable operations with mixed patterns */
        out[i]     = in1[i] * in2[i] + (i & 0xF);
        out[i+1]   = in1[i+1] - in2[i+1] | 0x1;
        out[i+2]   = (in1[i+2] << 2) ^ in2[i+2];
        out[i+3]   = in1[i+3] + (in2[i+3] >> 1);
        
        /* Insert scheduling barrier every 4 iterations */
        if ((i & 0xF) == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    /* Remainder loop */
    for (; i < size; i++) {
        out[i] = in1[i] + in2[i] * 3;
    }
}

__attribute__((noinline, optimize("O3")))
static int nested_loop_hazards(int size) {
    /* Nested loops with complex dependencies */
    int matrix[32][32];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            matrix[i][j] = i * 31 + j;
        }
    }
    
    /* Complex access pattern with hazards */
    for (int iter = 0; iter < size; iter++) {
        for (int i = 1; i < 31; i++) {
            for (int j = 1; j < 31; j++) {
                /* RAW: matrix[i][j] depends on neighbors */
                int val = matrix[i-1][j] + matrix[i][j-1] +
                          matrix[i+1][j] + matrix[i][j+1];
                
                /* WAW: multiple writes in dependency chain */
                matrix[i][j] = val / 4;
                matrix[i][j] = matrix[i][j] * 2 - 1;  /* Second write */
                
                /* Mixed float/int */
                sum += matrix[i][j] + (int)(sin(val * 0.01) * 100);
            }
        }
        
        /* Scheduling barrier between outer loop iterations */
        asm volatile("" ::: "memory");
    }
    
    return sum;
}

/* ========== Main Execution Flow ========== */

int main(void) {
    /* Allocate aligned memory for vectorization */
    float* fa = aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* fb = aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* fc = aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    
    double* darr = aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    
    int* iarr1 = aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* iarr2 = aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* iarr3 = aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    
    /* Initialize with pattern */
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fa[i] = (float)rand() / RAND_MAX * 100.0f;
        fb[i] = (float)rand() / RAND_MAX * 100.0f;
        darr[i] = (double)rand() / RAND_MAX * 200.0;
        iarr1[i] = rand() % 1000;
        iarr2[i] = rand() % 1000;
    }
    
    /* Accumulator to prevent dead code elimination */
    volatile float fsum = 0.0f;
    volatile double dsum = 0.0;
    volatile int isum = 0;
    
    /* Execute all test functions in sequence */
    printf("Starting selective scheduling stress tests...\n");
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* 1. Hot vectorized loop with mixed hazards */
        fsum += hot_vectorized_loop(fa, fb, fc, ARRAY_SIZE);
        
        /* 2. Cold pointer chasing */
        isum += cold_pointer_chasing(iarr1, ARRAY_SIZE);
        
        /* 3. Mixed control flow */
        dsum += mixed_control_flow(darr, ARRAY_SIZE);
        
        /* 4. SIMD unrolled operations */
        simd_unrolled_operations(iarr3, iarr1, iarr2, ARRAY_SIZE);
        isum += iarr3[ARRAY_SIZE / 2];
        
        /* 5. Nested loop hazards */
        isum += nested_loop_hazards(10);
        
        /* Modify inputs slightly each iteration */
        fa[iter % ARRAY_SIZE] += 0.1f;
        darr[iter % ARRAY_SIZE] *= 1.01;
        iarr1[iter % ARRAY_SIZE] ^= 0x55AA;
    }
    
    /* Print results to ensure execution */
    printf("Results: fsum=%.2f, dsum=%.2f, isum=%d\n", 
           (double)fsum, dsum, isum);
    
    /* Cleanup */
    free(fa);
    free(fb);
    free(fc);
    free(darr);
    free(iarr1);
    free(iarr2);
    free(iarr3);
    
    return 0;
}
