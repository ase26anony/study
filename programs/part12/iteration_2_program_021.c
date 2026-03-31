#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Packed struct to force unaligned accesses and potential secondary reloads */
struct __attribute__((packed)) Packed {
    double d;
    int i;
    char c;
    long l;
    float f;
};

/* Volatile variable to prevent optimization of conditional paths */
volatile int global_flag = 1;

/* Target function that will induce many reloads */
__attribute__((noinline))
static long complex_reload_function(int N, int seed) {
    /* Declare many scalar variables to exceed available registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d1, d2, d3, d4, d5, d6;
    long l1, l2, l3, l4, l5, l6, l7, l8;
    
    /* Initialize with non-constant values to prevent constant propagation */
    a1 = seed * 1; a2 = seed * 2; a3 = seed * 3; a4 = seed * 4; a5 = seed * 5;
    a6 = seed * 6; a7 = seed * 7; a8 = seed * 8; a9 = seed * 9; a10 = seed * 10;
    
    f1 = seed * 1.1f; f2 = seed * 2.2f; f3 = seed * 3.3f; f4 = seed * 4.4f;
    f5 = seed * 5.5f; f6 = seed * 6.6f; f7 = seed * 7.7f; f8 = seed * 8.8f;
    
    d1 = seed * 1.11; d2 = seed * 2.22; d3 = seed * 3.33; d4 = seed * 4.44;
    d5 = seed * 5.55; d6 = seed * 6.66;
    
    l1 = seed * 100L; l2 = seed * 200L; l3 = seed * 300L; l4 = seed * 400L;
    l5 = seed * 500L; l6 = seed * 600L; l7 = seed * 700L; l8 = seed * 800L;
    
    /* Multi-dimensional array to force address reloads */
    int arr[128][128];
    
    /* Packed struct accessed through volatile pointer */
    struct Packed packed_data;
    volatile struct Packed *volatile_packed = &packed_data;
    
    /* Initialize array with non-constant pattern */
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            arr[i][j] = (i * 7919 + j * 104729) ^ seed;
        }
    }
    
    /* Initialize packed struct */
    packed_data.d = seed * 3.14159;
    packed_data.i = seed * 271828;
    packed_data.c = seed & 0xFF;
    packed_data.l = seed * 314159265L;
    packed_data.f = seed * 2.71828f;
    
    long result = 0;
    
    /* Main computation loop - designed to maximize register pressure */
    for (int i = 1; i < N - 1; i++) {
        for (int j = 1; j < N - 1 && j < 127; j++) {
            /* Complex array access pattern requiring address reloads */
            int temp1 = arr[i][j];
            int temp2 = arr[j][i];
            int temp3 = arr[i-1][j+1];
            int temp4 = arr[i+1][j-1];
            
            /* Chain computations to keep many variables live */
            a1 = a2 + a3;
            a2 = a3 * a4;
            a3 = a4 ^ a5;
            a4 = a5 - a6;
            a5 = a6 | a7;
            
            /* Floating point computations */
            f1 = f2 + f3;
            f2 = f3 * f4;
            f3 = f4 - f5;
            f4 = f5 / (f6 + 1.0f);
            
            /* Double computations */
            d1 = d2 * d3;
            d2 = d3 + d4;
            d3 = d4 - d5;
            d4 = d5 / (d6 + 1.0);
            
            /* Long computations */
            l1 = l2 + l3;
            l2 = l3 * l4;
            l3 = l4 ^ l5;
            l4 = l5 - l6;
            l5 = l6 | l7;
            
            /* Inline assembly with conflicting constraints to force reloads */
            /* Force input/output conflicts */
            asm volatile (
                "addl %1, %0\n\t"
                : "+r" (a1)        /* tied output/input */
                : "r" (a2)         /* input in register */
                : "cc"
            );
            
            /* Another asm with memory constraint */
            asm volatile (
                "mov %1, %%eax\n\t"
                "add %%eax, %0\n\t"
                : "+m" (arr[i][j])  /* memory output */
                : "r" (temp1)       /* register input */
                : "%eax", "cc"
            );
            
            /* Asm with specific register constraints (x86) */
            asm volatile (
                "imull %%edx, %%ecx\n\t"
                "movl %%ecx, %0\n\t"
                : "=r" (a7)
                : "d" (a8), "c" (a9)  /* edx and ecx constraints */
                : "cc"
            );
            
            /* Access packed struct through volatile pointer - may need secondary reload */
            int packed_int = volatile_packed->i;
            double packed_double = volatile_packed->d;
            
            /* Use packed data in computation */
            a10 = a10 + packed_int;
            d6 = d6 + packed_double;
            
            /* Conditional block to create optional reload contexts */
            if (global_flag & 1) {
                /* Use different subset of variables here */
                f5 = f6 * f7;
                f6 = f7 + f8;
                l6 = l7 ^ l8;
                
                /* Another asm with tied operand */
                asm volatile (
                    "add %1, %0\n\t"
                    : "+r" (l6)
                    : "r" (l7)
                    : "cc"
                );
            } else {
                /* Alternative path using other variables */
                f7 = f8 * f1;
                f8 = f1 + f2;
                l7 = l8 ^ l1;
            }
            
            /* More complex chaining */
            a6 = a7 + a8 + a9 + a10;
            a7 = a8 * a9 / (a10 + 1);
            
            f7 = f8 * 1.414f + f1;
            f8 = f1 / (f2 + 0.001f) - f3;
            
            d5 = d6 * 1.41421356 + d1;
            d6 = d1 / (d2 + 0.000001) - d3;
            
            l6 = l7 + l8 + l1 + l2;
            l7 = l8 * l1 / (l2 + 1);
            
            /* Update array with computed values - complex addressing */
            arr[i][j] = (a1 + a2 + a3 + a4 + a5) ^ 
                       (int)(f1 + f2 + f3 + f4) ^
                       (int)(d1 + d2 + d3 + d4) ^
                       (int)(l1 + l2 + l3 + l4);
            
            /* Accumulate result to prevent optimization */
            result += arr[i][j] + a1 + a2 + a3 + (int)f1 + (int)d1 + (int)l1;
        }
        
        /* Occasionally update global_flag to affect conditional paths */
        if (i % 37 == 0) {
            global_flag = (global_flag << 1) | (result & 1);
        }
    }
    
    /* Final computation using all variables */
    result += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    result += (long)(f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8);
    result += (long)(d1 + d2 + d3 + d4 + d5 + d6);
    result += l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8;
    
    /* Add array checksum */
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            result += arr[i][j];
        }
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    int seed = (argc > 2) ? atoi(argv[2]) : time(NULL);
    
    srand(seed);
    
    /* Call the reload-intensive function multiple times */
    long total = 0;
    for (int iter = 0; iter < 3; iter++) {
        total += complex_reload_function(N + iter, seed + iter);
    }
    
    printf("Result: %ld\n", total);
    return 0;
}
