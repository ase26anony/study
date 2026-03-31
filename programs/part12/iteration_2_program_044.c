/* reload_trigger.c - Program to trigger GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Packed struct to force unaligned accesses and secondary reloads */
struct __attribute__((packed)) PackedStruct {
    double d;
    int i;
    float f;
    long l;
    char c;
    short s;
};

/* Volatile variables to prevent optimization */
volatile int volatile_flag = 0;
volatile int* volatile_ptr;

/* Function with high register pressure and complex operations */
void trigger_reloads(int N, int* checksum) {
    /* Declare many variables to exceed available registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d1, d2, d3, d4, d5, d6;
    long l1, l2, l3, l4, l5;
    
    /* Initialize with non-constant values */
    a1 = N * 1; a2 = N * 2; a3 = N * 3; a4 = N * 4; a5 = N * 5;
    a6 = N * 6; a7 = N * 7; a8 = N * 8; a9 = N * 9; a10 = N * 10;
    
    f1 = N * 1.1f; f2 = N * 2.2f; f3 = N * 3.3f; f4 = N * 4.4f;
    f5 = N * 5.5f; f6 = N * 6.6f; f7 = N * 7.7f; f8 = N * 8.8f;
    
    d1 = N * 1.11; d2 = N * 2.22; d3 = N * 3.33;
    d4 = N * 4.44; d5 = N * 5.55; d6 = N * 6.66;
    
    l1 = N * 100L; l2 = N * 200L; l3 = N * 300L;
    l4 = N * 400L; l5 = N * 500L;
    
    /* Multi-dimensional array for address reloads */
    int arr[100][100];
    
    /* Packed struct for unaligned accesses */
    struct PackedStruct ps;
    ps.d = 3.14159;
    ps.i = 42;
    ps.f = 2.71828f;
    ps.l = 123456789L;
    ps.c = 'X';
    ps.s = 999;
    
    /* Volatile pointer to packed struct */
    volatile struct PackedStruct* vps = &ps;
    
    /* Complex nested loops with array accesses */
    for (int i = 0; i < N && i < 100; i++) {
        for (int j = 0; j < N && j < 100; j++) {
            /* Force address reloads with complex array indexing */
            arr[i][j] = arr[j][i] + i * j;
            
            /* Chain computations to keep variables live */
            a1 = a2 + a3;
            a4 = a5 * a6;
            a7 = a8 - a9;
            a10 = a1 + a4;
            
            f1 = f2 + f3;
            f4 = f5 * f6;
            f7 = f8 - f1;
            
            d1 = d2 + d3;
            d4 = d5 * d6;
            
            l1 = l2 + l3;
            l4 = l5 - l1;
            
            /* Inline assembly with conflicting constraints */
            /* Force input reloads with "r" constraints */
            asm volatile (
                "addl %1, %0\n\t"
                : "+r" (a1)
                : "r" (a2)
                : "cc"
            );
            
            /* Force output reload with tied operand */
            asm volatile (
                "mov %1, %0\n\t"
                "addl %2, %0\n\t"
                : "=&r" (a3)
                : "r" (a4), "0" (a5)
                : "cc"
            );
            
            /* Memory constraint to force spill/reload */
            asm volatile (
                "movl %1, %%eax\n\t"
                "addl %%eax, %0\n\t"
                : "+m" (arr[i][j])
                : "r" (a6)
                : "%eax", "cc"
            );
            
            /* Float/double operations with constraints */
            asm volatile (
                "addss %1, %0\n\t"
                : "+x" (f1)
                : "x" (f2)
            );
            
            /* Conditional block for optional reloads */
            if (volatile_flag) {
                /* Use different variables inside conditional */
                a9 = a10 * 2;
                f7 = f8 / 2.0f;
                d5 = d6 * 3.14;
                
                /* More inline asm in conditional path */
                asm volatile (
                    "subl %1, %0\n\t"
                    : "+r" (a9)
                    : "r" (a8)
                    : "cc"
                );
            } else {
                /* Alternative path with different variables */
                a8 = a7 + 1;
                f6 = f5 - 1.0f;
                d4 = d3 * 2.0;
            }
            
            /* Access packed struct through volatile pointer */
            /* This may require secondary reloads due to alignment */
            int tmp = vps->i;
            vps->i = tmp + 1;
            
            /* Force reload of struct member */
            asm volatile (
                "movl %1, %%eax\n\t"
                "addl $1, %%eax\n\t"
                "movl %%eax, %0\n\t"
                : "=m" (vps->i)
                : "m" (vps->i)
                : "%eax", "cc"
            );
            
            /* More chained computations */
            a2 = a3 + a4 + a5;
            f3 = f4 + f5 + f6;
            d2 = d3 + d4 + d5;
            l2 = l3 + l4 + l5;
            
            /* Cross-type computations to force moves between reg classes */
            a6 = (int)f1 + (int)d1;
            f2 = (float)a2 + (float)l2;
            
            /* Update checksum */
            *checksum += arr[i][j] + a1 + a2 + a3 + a4 + a5 + 
                        (int)f1 + (int)f2 + (int)d1 + (int)l1;
        }
        
        /* Additional computations between outer loop iterations */
        /* Create more register pressure */
        a1 = a1 * 2 - a2;
        a2 = a2 * 3 - a3;
        a3 = a3 * 4 - a4;
        a4 = a4 * 5 - a5;
        a5 = a5 * 6 - a6;
        
        f1 = f1 * 1.5f - f2;
        f2 = f2 * 2.5f - f3;
        f3 = f3 * 3.5f - f4;
        
        d1 = d1 * 1.1 - d2;
        d2 = d2 * 2.2 - d3;
        
        l1 = l1 * 2 - l2;
        l2 = l2 * 3 - l3;
        
        /* Force spilling by using all variables in a complex expression */
        *checksum += (a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10) *
                    ((int)f1 + (int)f2 + (int)f3 + (int)f4 + 
                     (int)f5 + (int)f6 + (int)f7 + (int)f8) +
                    ((int)d1 + (int)d2 + (int)d3 + (int)d4 + 
                     (int)d5 + (int)d6) +
                    (int)(l1 + l2 + l3 + l4 + l5);
    }
    
    /* Final computations to ensure all variables are used */
    int final_result = 
        a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
        (int)f1 + (int)f2 + (int)f3 + (int)f4 + 
        (int)f5 + (int)f6 + (int)f7 + (int)f8 +
        (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 + (int)d6 +
        (int)l1 + (int)l2 + (int)l3 + (int)l4 + (int)l5;
    
    *checksum += final_result;
}

int main(int argc, char** argv) {
    int N = 50;  /* Default loop bound */
    
    /* Use command line argument for variable loop count */
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 50;
        if (N > 100) N = 100;  /* Prevent excessive runtime */
    }
    
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Set volatile flag randomly */
    volatile_flag = rand() % 2;
    
    int checksum = 0;
    
    /* Call the function that triggers reloads */
    trigger_reloads(N, &checksum);
    
    /* Additional calls with different parameters */
    for (int i = 0; i < 3; i++) {
        trigger_reloads(N / (i + 2), &checksum);
    }
    
    /* Print result to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
