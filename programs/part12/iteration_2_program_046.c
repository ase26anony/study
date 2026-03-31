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

/* Volatile variable to prevent optimization of conditionals */
volatile int volatile_flag = 1;

/* Target function that will induce many reloads */
__attribute__((noinline))
static unsigned long long induce_reloads(int N, int seed) {
    /* Declare many scalar variables of mixed types to exceed register file */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d1, d2, d3, d4, d5, d6;
    long l1, l2, l3, l4, l5;
    
    /* Initialize with non-constant values to prevent constant propagation */
    a1 = seed * 1; a2 = seed * 2; a3 = seed * 3; a4 = seed * 4; a5 = seed * 5;
    a6 = seed * 6; a7 = seed * 7; a8 = seed * 8; a9 = seed * 9; a10 = seed * 10;
    
    f1 = seed * 1.1f; f2 = seed * 2.2f; f3 = seed * 3.3f; f4 = seed * 4.4f;
    f5 = seed * 5.5f; f6 = seed * 6.6f; f7 = seed * 7.7f; f8 = seed * 8.8f;
    
    d1 = seed * 1.11; d2 = seed * 2.22; d3 = seed * 3.33;
    d4 = seed * 4.44; d5 = seed * 5.55; d6 = seed * 6.66;
    
    l1 = seed * 100L; l2 = seed * 200L; l3 = seed * 300L;
    l4 = seed * 400L; l5 = seed * 500L;
    
    /* Multi-dimensional array for complex addressing modes */
    int arr[128][128];
    
    /* Packed struct accessed through volatile pointer */
    struct Packed packed_data;
    volatile struct Packed *volatile_packed = &packed_data;
    
    /* Initialize array with non-constant pattern */
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            arr[i][j] = (i * 7919 + j * 65537 + seed) & 0xFF;
        }
    }
    
    /* Initialize packed struct */
    packed_data.d = seed * 3.14159;
    packed_data.i = seed * 2718;
    packed_data.c = seed & 0xFF;
    packed_data.l = seed * 314159265L;
    packed_data.f = seed * 2.71828f;
    
    unsigned long long checksum = 0;
    
    /* Main computation loop that will force many reloads */
    for (int i = 1; i < N - 1; i++) {
        for (int j = 1; j < N - 1 && j < 127; j++) {
            /* Complex addressing with computed indices - forces address reloads */
            int idx1 = (i * 17 + j * 13) & 127;
            int idx2 = (i * 23 + j * 19) & 127;
            
            /* Chain computations creating data dependencies between many variables */
            a1 = a2 + a3;
            a4 = a5 * a6 - a7;
            
            /* Inline assembly with conflicting constraints to force reloads */
            /* Tied operand constraint: output tied to input 0 */
            asm volatile (
                "addl %1, %0\n\t"
                : "+r"(a1)        /* read-write operand, tied */
                : "r"(a2)         /* input only */
                : "cc"
            );
            
            /* Another asm with different constraints on same variable */
            asm volatile (
                "imull %1, %0\n\t"
                : "+r"(a4)        /* tied operand */
                : "r"(a8)         /* input */
                : "cc"
            );
            
            /* Floating point asm with constraints */
            asm volatile (
                "addss %1, %0\n\t"
                : "+x"(f1)        /* SSE register constraint */
                : "x"(f2)
            );
            
            /* Mixed type computation to force moves between register classes */
            d1 = d2 + (double)f3;
            d3 = d4 * (double)a9;
            
            /* Access packed struct through volatile pointer - may need secondary reload */
            int packed_val = volatile_packed->i;
            volatile_packed->i = packed_val + a1;
            
            /* Complex array access with swapping - forces address calculations */
            int temp = arr[idx1][idx2];
            arr[idx1][idx2] = arr[idx2][idx1] + a1;
            arr[idx2][idx1] = temp - a4;
            
            /* Conditional block for optional reloads */
            if (volatile_flag) {
                /* Use different subset of variables inside conditional */
                l1 = l2 + l3;
                l4 = l5 * a10;
                
                /* Another asm with memory constraint */
                asm volatile (
                    "movl %1, %%eax\n\t"
                    "addl %%eax, %0\n\t"
                    : "+m"(arr[i][j])   /* memory operand */
                    : "r"(l1)           /* register operand */
                    : "%eax", "cc"
                );
                
                d5 = d6 * 1.01;
                f4 = f5 + f6;
            } else {
                /* Alternative path with different variables */
                a3 = a4 + a5;
                f7 = f8 * 2.0f;
                
                /* Asm with specific register constraints */
                asm volatile (
                    "movq %1, %%rax\n\t"
                    "addq %%rax, %0\n\t"
                    : "+r"(l2)
                    : "r"(l3)
                    : "%rax", "cc"
                );
            }
            
            /* More chained computations to keep variables live */
            a2 = a1 + a4;
            a5 = a6 - a7;
            a8 = a9 * a10;
            
            f2 = f1 + f3;
            f5 = f4 * f6;
            f8 = f7 / 2.0f;
            
            d2 = d1 + d3;
            d4 = d5 - d6;
            
            l3 = l1 + l2;
            l5 = l4 * 2;
            
            /* Update checksum with various variables */
            checksum += a1 + a2 + a3 + a4 + a5;
            checksum += (unsigned long long)(f1 + f2 + f3 + f4);
            checksum += (unsigned long long)(d1 + d2 + d3);
            checksum += l1 + l2 + l3 + l4 + l5;
            checksum += arr[i][j] + arr[j][i];
        }
        
        /* Occasionally update volatile flag */
        if (i % 37 == 0) {
            volatile_flag = (volatile_flag + 1) & 1;
        }
    }
    
    /* Final computation using all variables to prevent elimination */
    long final_result = 
        a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
        (long)(f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8) +
        (long)(d1 + d2 + d3 + d4 + d5 + d6) +
        l1 + l2 + l3 + l4 + l5;
    
    checksum += final_result;
    
    /* Sum array elements */
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            checksum += arr[i][j];
        }
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 100;
    int seed = (argc > 2) ? atoi(argv[2]) : time(NULL);
    
    srand(seed);
    
    /* Call reload-inducing function multiple times */
    unsigned long long total_checksum = 0;
    for (int iter = 0; iter < 3; iter++) {
        total_checksum += induce_reloads(N, seed + iter);
    }
    
    printf("Checksum: %llu\n", total_checksum);
    return 0;
}
