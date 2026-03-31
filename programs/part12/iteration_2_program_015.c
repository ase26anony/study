/* reload_test.c - Program to trigger GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Packed struct to force unaligned accesses and secondary reloads */
struct __attribute__((packed)) Packed {
    double d;
    int i;
    char c;
    long l;
    float f;
};

/* Volatile flag for conditional execution */
volatile int reload_flag = 1;

/* Target function with high register pressure */
void __attribute__((noinline)) 
force_reloads(int N, int arr[200][200], struct Packed *p) 
{
    /* Declare many live variables of mixed types */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5;
    double d1, d2, d3, d4, d5;
    long l1, l2, l3, l4;
    
    /* Initialize with non-constant values */
    a1 = N * 1; a2 = N * 2; a3 = N * 3; a4 = N * 4; a5 = N * 5;
    a6 = N * 6; a7 = N * 7; a8 = N * 8; a9 = N * 9; a10 = N * 10;
    
    f1 = N * 1.1f; f2 = N * 2.2f; f3 = N * 3.3f; 
    f4 = N * 4.4f; f5 = N * 5.5f;
    
    d1 = N * 1.11; d2 = N * 2.22; d3 = N * 3.33;
    d4 = N * 4.44; d5 = N * 5.55;
    
    l1 = N * 100L; l2 = N * 200L; l3 = N * 300L; l4 = N * 400L;
    
    /* Complex nested loops with array accesses */
    for (int i = 0; i < N && i < 200; i++) {
        for (int j = 0; j < N && j < 200; j++) {
            /* Force address reloads with complex indexing */
            int temp = arr[i][j];
            arr[j][i] = temp + a1;
            
            /* Chain computations to keep variables live */
            a1 = a2 + a3;
            a2 = a3 * a4;
            a3 = a4 - a5;
            a4 = a5 ^ a6;
            a5 = a6 | a7;
            
            /* Floating point computations */
            f1 = f2 + f3;
            f2 = f3 * f4;
            f3 = f4 - f5;
            
            d1 = d2 + d3;
            d2 = d3 * d4;
            
            l1 = l2 + l3;
            l2 = l3 - l4;
            
            /* Inline assembly with conflicting constraints */
            /* Force input reloads with tied operands */
            asm volatile (
                "addl %1, %0\n\t"
                : "=r"(a6) 
                : "r"(a7), "0"(a6)
                : "cc"
            );
            
            /* Another asm with memory constraint */
            asm volatile (
                "movl %1, %%eax\n\t"
                "addl %%eax, %0\n\t"
                : "=r"(a8)
                : "m"(a9)
                : "eax", "cc"
            );
            
            /* Conditional block for optional reloads */
            if (reload_flag) {
                /* Use different subset of variables */
                a10 = a1 + a2 + a3;
                f4 = f1 * f2 * f3;
                d3 = d1 + d2 + d4;
                
                /* Force spill/reload with volatile asm */
                asm volatile (
                    "movq %1, %%rax\n\t"
                    "addq %%rax, %0\n\t"
                    : "=r"(l3)
                    : "r"(l4)
                    : "rax", "cc"
                );
            } else {
                /* Alternative path with different variables */
                a9 = a4 * a5 * a6;
                f5 = f3 / f4;
                d4 = d2 - d5;
            }
            
            /* Access packed struct through volatile pointer */
            volatile struct Packed *vp = p;
            vp->i = a1;
            vp->d = d1;
            
            /* Force reload of struct member */
            int packed_val = vp->i;
            a7 = packed_val + a8;
            
            /* More arithmetic to maintain live ranges */
            a1 = a1 + 1;
            a2 = a2 * 2;
            a3 = a3 - 3;
            a4 = a4 ^ 4;
            a5 = a5 | 5;
            
            f1 = f1 + 1.0f;
            f2 = f2 * 2.0f;
            
            d1 = d1 + 1.0;
            d2 = d2 * 2.0;
            
            l1 = l1 + 1;
            l2 = l2 * 2;
        }
    }
    
    /* Final computation using all variables to prevent elimination */
    int sum_int = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    float sum_float = f1 + f2 + f3 + f4 + f5;
    double sum_double = d1 + d2 + d3 + d4 + d5;
    long sum_long = l1 + l2 + l3 + l4;
    
    /* Store results in array to ensure they're used */
    arr[0][0] = sum_int;
    arr[0][1] = (int)sum_float;
    arr[0][2] = (int)sum_double;
    arr[0][3] = (int)sum_long;
}

int main(int argc, char *argv[]) 
{
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    
    /* Large multi-dimensional array */
    int (*arr)[200] = malloc(200 * 200 * sizeof(int));
    
    /* Packed struct */
    struct Packed p = {3.14159, 42, 'X', 123456789L, 2.71828f};
    
    /* Initialize array with non-constant pattern */
    srand(time(NULL));
    for (int i = 0; i < 200; i++) {
        for (int j = 0; j < 200; j++) {
            arr[i][j] = rand() % 1000;
        }
    }
    
    /* Call the reload-intensive function */
    force_reloads(N, arr, &p);
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < 100 && i < N; i++) {
        for (int j = 0; j < 100 && j < N; j++) {
            checksum += arr[i][j];
        }
    }
    
    /* Access packed struct members */
    checksum += p.i + (int)p.d + p.c + (int)p.l + (int)p.f;
    
    printf("Checksum: %ld\n", checksum);
    
    free(arr);
    return 0;
}
