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

/* Volatile variable to prevent optimization and create conditional paths */
volatile int global_flag = 1;

/* Function with high register pressure and complex operations */
void trigger_reloads(int N, int *checksum) {
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
    int arr[128][128];
    
    /* Packed struct accessed through volatile pointer */
    struct Packed p;
    volatile struct Packed *volatile_p = &p;
    
    /* Initialize array with non-constant pattern */
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            arr[i][j] = (i * 7919 + j * 65537) % 1000;
        }
    }
    
    /* Complex nested loops with many live variables */
    for (int i = 1; i < N && i < 127; i++) {
        for (int j = 1; j < N && j < 127; j++) {
            /* Create complex addressing with computed indices */
            int idx1 = (i * 13 + j * 17) % 128;
            int idx2 = (i * 19 + j * 23) % 128;
            
            /* Force address reloads with complex array access */
            int temp = arr[idx1][idx2] + arr[idx2][idx1];
            
            /* Chain computations to keep variables live */
            a1 = a2 + a3;
            a2 = a3 + a4;
            a3 = a4 + a5;
            a4 = a5 + a6;
            a5 = a6 + a7;
            
            /* Floating point computations */
            f1 = f2 + f3;
            f2 = f3 + f4;
            f3 = f4 * f5;
            
            /* Double computations */
            d1 = d2 * d3;
            d2 = d3 / (d4 + 1.0);
            
            /* Long computations */
            l1 = l2 + l3;
            l2 = l3 - l4;
            
            /* Inline assembly with conflicting constraints */
            /* Force input/output reloads with tied operands */
            asm volatile (
                "add %0, %1, %2\n\t"
                : "=r"(a6) 
                : "r"(a7), "0"(a8)
                : "cc"
            );
            
            /* Another asm with memory constraint */
            asm volatile (
                "mov %0, %1\n\t"
                : "=r"(a7)
                : "m"(arr[i][j])
                : "cc"
            );
            
            /* Assembly with specific register constraints (x86) */
            #ifdef __x86_64__
            asm volatile (
                "addl %%eax, %%ebx\n\t"
                : "=b"(a8)
                : "a"(a9), "b"(a10)
                : "cc"
            );
            #endif
            
            /* Access packed struct through volatile pointer */
            /* This may require secondary reloads due to alignment */
            volatile_p->i = temp;
            volatile_p->d = d1 + d2;
            
            /* Conditional block for optional reloads */
            if (global_flag & 1) {
                /* Use different subset of variables */
                a9 = a10 * 2;
                f4 = f5 * 3.0f;
                d3 = d4 - d5;
                l3 = l4 | l5;
                
                /* More assembly in conditional path */
                asm volatile (
                    "sub %0, %1, %2\n\t"
                    : "=r"(a10)
                    : "r"(a1), "r"(a2)
                    : "cc"
                );
            } else {
                /* Alternative path with different variables */
                f5 = f6 / 2.0f;
                d4 = d5 * 2.0;
                l4 = l5 << 2;
            }
            
            /* Update array with complex addressing */
            arr[i][j] = (arr[i-1][j] + arr[i][j-1] + temp) / 3;
            
            /* Chain more computations */
            f6 = f7 + f8;
            f7 = f8 * 2.0f;
            d5 = d6 + 1.0;
            d6 = d1 * 0.5;
            l5 = l1 ^ l2;
            
            /* Force spill by using all variables in expression */
            *checksum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10
                       + (int)f1 + (int)f2 + (int)f3 + (int)f4 
                       + (int)f5 + (int)f6 + (int)f7 + (int)f8
                       + (int)d1 + (int)d2 + (int)d3 
                       + (int)d4 + (int)d5 + (int)d6
                       + (int)l1 + (int)l2 + (int)l3 
                       + (int)l4 + (int)l5
                       + arr[i][j];
        }
        
        /* Alternate between flag values */
        global_flag ^= 1;
    }
    
    /* Final computations using all variables */
    int final = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    *checksum += final;
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    int checksum = 0;
    
    srand(time(NULL));
    
    /* Call function multiple times to increase pressure */
    for (int iter = 0; iter < 3; iter++) {
        trigger_reloads(N + iter, &checksum);
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
