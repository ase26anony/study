#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Packed struct to force unaligned accesses */
struct __attribute__((packed)) Packed {
    double d;
    int i;
    char c;
    long l;
    float f;
};

/* Volatile variable to prevent optimization */
volatile int global_flag = 1;

/* Function with high register pressure */
void reload_inducing_function(int N, int *checksum) {
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
    
    /* Multi-dimensional array for address reloads */
    int arr[128][128];
    
    /* Packed struct for unaligned accesses */
    struct Packed packed_data[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            arr[i][j] = i * 1000 + j;
        }
    }
    
    for (int i = 0; i < 16; i++) {
        packed_data[i].d = i * 1.234;
        packed_data[i].i = i * 100;
        packed_data[i].c = i;
        packed_data[i].l = i * 1000L;
        packed_data[i].f = i * 2.5f;
    }
    
    /* Complex nested loops with many live variables */
    for (int i = 1; i < N && i < 127; i++) {
        for (int j = 1; j < N && j < 127; j++) {
            /* Create complex addressing with multiple indices */
            int idx1 = (i * j) % 128;
            int idx2 = (i + j) % 128;
            int idx3 = (i ^ j) % 128;
            
            /* Force address reloads with complex array access */
            int temp1 = arr[idx1][idx2];
            int temp2 = arr[idx2][idx3];
            int temp3 = arr[idx3][idx1];
            
            /* Inline assembly with conflicting constraints */
            /* Force input reload with "r" constraint */
            asm volatile (
                "addl %1, %0\n\t"
                : "+r" (a1)
                : "r" (temp1)
                : "cc"
            );
            
            /* Another asm with tied operand (output tied to input) */
            asm volatile (
                "imull %1, %0\n\t"
                : "+0" (a2)
                : "r" (temp2)
                : "cc"
            );
            
            /* Asm with memory constraint to force spills */
            asm volatile (
                "addl %1, %0\n\t"
                : "+r" (a3)
                : "m" (temp3)
                : "cc"
            );
            
            /* Chain computations to keep variables live */
            f1 = f2 + f3;
            f4 = f5 * f6;
            d1 = d2 - d3;
            d4 = d5 / (d6 + 1.0);
            
            l1 = l2 + l3;
            l4 = l5 - l1;
            
            /* Conditional block for optional reloads */
            if (global_flag) {
                /* Use different variables inside conditional */
                a4 = a5 + a6;
                a7 = a8 * a9;
                f7 = f8 * 2.0f;
                
                /* Access packed struct through volatile pointer */
                volatile struct Packed *p = &packed_data[(i + j) % 16];
                double packed_d = p->d;
                int packed_i = p->i;
                
                /* Force reload of packed data */
                d2 += packed_d;
                a10 += packed_i;
            } else {
                /* Alternative path with different variables */
                a5 = a6 - a7;
                a8 = a9 / 2;
                f8 = f1 * 3.0f;
            }
            
            /* More arithmetic to create data dependencies */
            a1 = a2 + a3;
            a2 = a3 * a4;
            a3 = a4 - a5;
            a4 = a5 / (a6 + 1);
            a5 = a6 ^ a7;
            a6 = a7 | a8;
            a7 = a8 & a9;
            a8 = a9 << 2;
            a9 = a10 >> 1;
            a10 = a1 + a2;
            
            f2 = f3 * f4;
            f3 = f4 + f5;
            f4 = f5 - f6;
            f5 = f6 * f7;
            f6 = f7 / f8;
            
            d3 = d4 + d5;
            d4 = d5 * d6;
            d5 = d6 - d1;
            d6 = d1 / (d2 + 1.0);
            
            l2 = l3 + l4;
            l3 = l4 * l5;
            l4 = l5 - l1;
            l5 = l1 << 2;
            
            /* Complex array update with swapped indices */
            arr[j][i] = arr[i][j] + a1;
            arr[idx1][idx2] = arr[idx2][idx1] + a2;
            
            /* Update checksum */
            *checksum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
            *checksum += (int)f1 + (int)f2 + (int)f3 + (int)f4 + 
                        (int)f5 + (int)f6 + (int)f7 + (int)f8;
            *checksum += (int)d1 + (int)d2 + (int)d3 + 
                        (int)d4 + (int)d5 + (int)d6;
            *checksum += (int)l1 + (int)l2 + (int)l3 + (int)l4 + (int)l5;
        }
    }
    
    /* Final complex computation using all variables */
    int final_result = 
        a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
        (int)f1 + (int)f2 + (int)f3 + (int)f4 + 
        (int)f5 + (int)f6 + (int)f7 + (int)f8 +
        (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 + (int)d6 +
        (int)l1 + (int)l2 + (int)l3 + (int)l4 + (int)l5;
    
    *checksum += final_result;
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    int checksum = 0;
    
    srand(time(NULL));
    
    /* Call reload-inducing function multiple times */
    for (int iter = 0; iter < 3; iter++) {
        reload_inducing_function(N + iter, &checksum);
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
