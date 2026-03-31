/* reload_coverage.c - Program to force GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Packed struct to force unaligned accesses and secondary reloads */
struct __attribute__((packed)) Packed {
    double d;
    int i;
    float f;
    long l;
    char c;
};

/* Volatile variable to prevent optimization */
volatile int g_flag = 0;

/* Target function with high register pressure */
__attribute__((noinline))
static long process_data(int N, int arr[200][200]) {
    /* Declare many scalar variables to exceed available registers */
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
    
    /* Packed struct to force unaligned accesses */
    struct Packed p1, p2;
    p1.d = d1; p1.i = a1; p1.f = f1; p1.l = l1; p1.c = 'A';
    p2.d = d2; p2.i = a2; p2.f = f2; p2.l = l2; p2.c = 'B';
    
    /* Volatile pointer to packed struct */
    volatile struct Packed *volatile pp = &p1;
    
    /* Complex nested loops with array accesses */
    for (int i = 0; i < N && i < 200; i++) {
        for (int j = 0; j < N && j < 200; j++) {
            /* Force address reloads with complex array indexing */
            arr[i][j] = arr[j][i] + a1 + a2;
            
            /* Chain computations to keep variables live */
            a1 = a2 + a3;
            a2 = a3 + a4;
            a3 = a4 + a5;
            a4 = a5 + a6;
            a5 = a6 + a7;
            
            /* Floating point computations */
            f1 = f2 + f3;
            f2 = f3 + f4;
            f3 = f4 + f5;
            
            d1 = d2 + d3;
            d2 = d3 + d4;
            
            l1 = l2 + l3;
            l2 = l3 + l4;
            
            /* Inline assembly with conflicting constraints */
            /* Force input reloads with "r" constraints */
            asm volatile (
                "add %0, %1, %2\n\t"
                : "=r" (a6)
                : "r" (a7), "0" (a8)
                : /* no clobbers */
            );
            
            /* Another asm with different constraints */
            asm volatile (
                "imul %0, %1, %2\n\t"
                : "=r" (a7)
                : "r" (a8), "r" (a9)
                : /* no clobbers */
            );
            
            /* Force memory operand reload with "m" constraint */
            asm volatile (
                "mov %0, %1\n\t"
                : "=r" (a8)
                : "m" (arr[i][j])
                : /* no clobbers */
            );
            
            /* Conditional block for optional reloads */
            if (g_flag) {
                /* Use different subset of variables */
                a9 = a10 * 2;
                a10 = arr[i][j] * 3;
                f4 = f5 * 1.5f;
                d3 = d4 * 2.5;
                
                /* Access packed struct through volatile pointer */
                pp->i = a9;
                pp->f = f4;
                pp->d = d3;
            } else {
                /* Alternative path with different variables */
                f5 = f6 * 2.0f;
                f6 = f7 * 3.0f;
                d4 = d5 * 1.5;
                d5 = d6 * 2.0;
                
                /* More inline assembly with tied operands */
                asm volatile (
                    "sub %0, %1, %2\n\t"
                    : "=r" (l3)
                    : "r" (l4), "0" (l5)
                    : /* no clobbers */
                );
            }
            
            /* Force secondary reloads via packed struct access */
            int tmp = pp->i;
            float ftmp = pp->f;
            double dtmp = pp->d;
            
            /* Use the values to prevent elimination */
            a10 += tmp;
            f7 += ftmp;
            d6 += dtmp;
            
            /* More chained computations */
            l4 = l5 + a1;
            l5 = a2 + a3;
            
            /* Swap packed structs */
            struct Packed tmp_p = p1;
            p1 = p2;
            p2 = tmp_p;
            pp = (pp == &p1) ? &p2 : &p1;
        }
        
        /* Additional computations between outer loop iterations */
        a1 = a1 ^ a2;
        a2 = a2 ^ a3;
        a3 = a3 ^ a4;
        
        f1 = f1 * 0.99f;
        f2 = f2 * 1.01f;
        
        d1 = d1 * 0.999;
        d2 = d2 * 1.001;
        
        /* Force spill/reload by using all variables */
        l1 = l1 + (long)a1 + (long)a2 + (long)a3 + (long)a4 + (long)a5;
        l2 = l2 + (long)a6 + (long)a7 + (long)a8 + (long)a9 + (long)a10;
        
        /* Mix float and double */
        d3 = d3 + (double)f1 + (double)f2 + (double)f3 + (double)f4;
        d4 = d4 + (double)f5 + (double)f6 + (double)f7 + (double)f8;
    }
    
    /* Compute checksum using all variables */
    long checksum = 0;
    checksum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    checksum += (long)f1 + (long)f2 + (long)f3 + (long)f4;
    checksum += (long)f5 + (long)f6 + (long)f7 + (long)f8;
    checksum += (long)d1 + (long)d2 + (long)d3 + (long)d4 + (long)d5 + (long)d6;
    checksum += l1 + l2 + l3 + l4 + l5;
    checksum += pp->i + (long)pp->f + (long)pp->d + pp->l + pp->c;
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 100;
    
    /* Seed RNG for non-constant initialization */
    srand(time(NULL));
    
    /* Large multi-dimensional array */
    int arr[200][200];
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < 200; i++) {
        for (int j = 0; j < 200; j++) {
            arr[i][j] = rand() % 1000;
        }
    }
    
    /* Toggle volatile flag */
    g_flag = rand() % 2;
    
    /* Call the function that creates register pressure */
    long result = process_data(N, arr);
    
    /* Additional array processing to force more reloads */
    for (int i = 0; i < N && i < 200; i++) {
        for (int j = 0; j < N && j < 200; j++) {
            /* Force more address reloads */
            int tmp = arr[i][j];
            arr[j][i] = arr[i][j] * 2 - tmp;
            
            /* Inline asm with memory constraint */
            asm volatile (
                "addl $1, %0\n\t"
                : "+m" (arr[i][j])
                : 
                : "cc"
            );
        }
    }
    
    /* Final checksum computation */
    long final_sum = result;
    for (int i = 0; i < 100 && i < 200; i++) {
        for (int j = 0; j < 100 && j < 200; j++) {
            final_sum += arr[i][j];
        }
    }
    
    printf("Result: %ld\n", final_sum);
    return 0;
}
