#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Packed struct to force unaligned accesses and potential secondary reloads */
struct __attribute__((packed)) Packed {
    double d;
    int i;
    float f;
    long l;
    char c;
};

/* Volatile variable to prevent optimization and create conditional paths */
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
    
    /* Multi-dimensional array for complex addressing modes */
    int arr[128][128];
    
    /* Packed struct accessed through volatile pointer */
    struct Packed packed_data;
    volatile struct Packed *volatile_packed = &packed_data;
    
    /* Initialize array with non-constant pattern */
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            arr[i][j] = (i * 7919 + j * 65537) & 0xFF;
        }
    }
    
    /* Initialize packed struct */
    packed_data.d = seed * 3.14159;
    packed_data.i = seed * 2718;
    packed_data.f = seed * 2.71828f;
    packed_data.l = seed * 314159L;
    packed_data.c = seed & 0xFF;
    
    long result = 0;
    
    /* Main computation loop that will force many reloads */
    for (int i = 1; i < N - 1; i++) {
        for (int j = 1; j < N - 1 && j < 127; j++) {
            /* Complex addressing with computed indices - forces address reloads */
            int idx1 = (i * 17 + j * 23) % 127;
            int idx2 = (i * 29 + j * 31) % 127;
            
            /* Use inline assembly with conflicting constraints */
            /* Force input reloads with "r" constraints */
            asm volatile (
                "addl %1, %0\n\t"
                : "+r" (a1)
                : "r" (arr[idx1][idx2])
                : "cc"
            );
            
            /* Another asm with tied operand (output tied to input) */
            asm volatile (
                "imull %1, %0\n\t"
                : "+r" (a2)
                : "r" (a3), "0" (a2)
                : "cc"
            );
            
            /* Float/double operations with constraints */
            asm volatile (
                "addss %1, %0\n\t"
                : "+x" (f1)
                : "x" (f2)
            );
            
            /* Memory constraint forcing spill/reload */
            asm volatile (
                "addl %1, %0\n\t"
                : "+r" (a4)
                : "m" (arr[i][j])
                : "cc"
            );
            
            /* Chain computations to keep variables live */
            a5 = a1 + a2 + a3;
            a6 = a4 * a5 - a2;
            a7 = a6 / (a1 + 1);
            a8 = a7 ^ a5 ^ a3;
            a9 = a8 + a6 + a4;
            a10 = a9 * 2 - a8;
            
            f3 = f1 + f2;
            f4 = f3 * f1 - f2;
            f5 = f4 / (f3 + 1.0f);
            f6 = f5 * 2.0f - f4;
            f7 = f6 + f5 + f3;
            f8 = f7 * 1.5f;
            
            d3 = d1 + d2;
            d4 = d3 * d1 - d2;
            d5 = d4 / (d3 + 1.0);
            d6 = d5 * 2.0 - d4;
            
            l3 = l1 + l2;
            l4 = l3 * l1 - l2;
            l5 = l4 / (l3 + 1);
            l6 = l5 * 2 - l4;
            l7 = l6 + l5 + l3;
            l8 = l7 * 3 - l6;
            
            /* Conditional block for optional reloads */
            if (global_flag & 1) {
                /* Use different subset of variables here */
                asm volatile (
                    "subl %1, %0\n\t"
                    : "+r" (a10)
                    : "r" (a9)
                    : "cc"
                );
                
                f8 = f7 * 0.5f + f6;
                d6 = d5 * 0.5 + d4;
                l8 = l7 / 2 + l6;
                
                /* Access packed struct through volatile pointer */
                a1 += volatile_packed->i;
                f1 += volatile_packed->f;
                d1 += volatile_packed->d;
                l1 += volatile_packed->l;
            } else {
                /* Alternative path with different variable usage */
                a1 -= arr[j][i];
                f1 -= f8;
                d1 -= d6;
                l1 -= l8;
            }
            
            /* Complex array operation with swapped indices */
            arr[i][j] = arr[j][i] + a1;
            arr[idx1][idx2] = arr[idx2][idx1] * 2 - a2;
            
            /* Update packed struct */
            volatile_packed->i = a3;
            volatile_packed->f = f3;
            volatile_packed->d = d3;
            volatile_packed->l = l3;
            
            /* More inline asm with specific register constraints on x86 */
            #ifdef __x86_64__
            asm volatile (
                "movl %1, %%eax\n\t"
                "addl %%eax, %0\n\t"
                : "+r" (result)
                : "r" (a1)
                : "%eax", "cc"
            );
            #else
            /* Generic version for other architectures */
            asm volatile (
                "add %1, %0\n\t"
                : "+r" (result)
                : "r" (a1)
                : "cc"
            );
            #endif
            
            /* Toggle flag to vary execution path */
            global_flag ^= (i * j) & 1;
        }
    }
    
    /* Final computation using all variables to prevent elimination */
    result += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    result += (long)(f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8);
    result += (long)(d1 + d2 + d3 + d4 + d5 + d6);
    result += l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8;
    
    /* Sum array elements */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            result += arr[i][j];
        }
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    int seed = (argc > 2) ? atoi(argv[2]) : time(NULL);
    
    srand(seed);
    
    /* Call reload-intensive function multiple times */
    long total = 0;
    for (int iter = 0; iter < 3; iter++) {
        total += complex_reload_function(N, seed + iter);
    }
    
    printf("Result: %ld\n", total);
    return 0;
}
