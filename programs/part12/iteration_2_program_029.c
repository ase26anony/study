/* reload_trigger.c - Program to trigger GCC reload pass initialization */
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
volatile int global_flag = 0;

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
    int arr[100][100];
    
    /* Packed struct for unaligned accesses */
    struct Packed packed_var;
    volatile struct Packed *volatile_packed = &packed_var;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Complex nested loop with many live variables */
    for (int i = 1; i < N && i < 99; i++) {
        for (int j = 1; j < N && j < 99; j++) {
            /* Create complex addressing modes */
            int idx1 = (i * j) % 98 + 1;
            int idx2 = (i + j) % 98 + 1;
            
            /* Force address reloads with computed indices */
            int temp = arr[idx1][idx2] + arr[idx2][idx1];
            
            /* Inline assembly with conflicting constraints */
            /* Force input reloads with "r" constraints */
            asm volatile (
                "addl %1, %0\n\t"
                "addl %2, %0"
                : "+r" (a1)
                : "r" (temp), "r" (a2)
                : "cc"
            );
            
            /* Another asm with tied operand (forces reload) */
            asm volatile (
                "imull %1, %0"
                : "+0" (a3)
                : "r" (a4)
                : "cc"
            );
            
            /* Floating point asm with constraints */
            asm volatile (
                "addss %1, %0\n\t"
                "mulss %2, %0"
                : "+x" (f1)
                : "x" (f2), "x" (f3)
            );
            
            /* Mixed type operations to force different register classes */
            d1 = d2 + d3 * (double)f4;
            d4 = d5 - d6 / (double)a5;
            
            /* Chain computations to keep variables live */
            a6 = a7 + a8 * a9;
            a10 = a1 + a6 - a3;
            
            f5 = f6 * f7 + f8;
            f2 = f1 - f5 * f3;
            
            l1 = l2 + l3 * l4;
            l5 = l1 - l2 + a10;
            
            /* Conditional block for optional reloads */
            if (global_flag) {
                /* Different computation path */
                a2 = a3 * a4 + a5;
                f3 = f4 * f5 - f6;
                d3 = d4 + d5 * d6;
                
                /* Access packed struct through volatile pointer */
                volatile_packed->i = a2;
                volatile_packed->d = d3;
                volatile_packed->f = f3;
            } else {
                /* Another path */
                a7 = a8 + a9 * a10;
                f7 = f8 * f1 - f2;
                d5 = d6 + d1 * d2;
                
                /* Different packed struct access */
                volatile_packed->l = l5;
                volatile_packed->c = (char)a7;
            }
            
            /* Array operation with complex addressing */
            arr[i][j] = arr[j][i] + temp + a1;
            
            /* More asm with memory constraints */
            asm volatile (
                "movl %1, %%eax\n\t"
                "addl %%eax, %0"
                : "+m" (arr[i][j])
                : "r" (a2)
                : "eax", "cc"
            );
            
            /* Update checksum */
            *checksum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
            *checksum += (int)f1 + (int)f2 + (int)f3 + (int)f4;
            *checksum += (int)d1 + (int)d2 + (int)d3;
            *checksum += (int)l1 + (int)l2 + arr[i][j];
        }
        
        /* Update global_flag occasionally */
        if (i % 17 == 0) {
            global_flag = !global_flag;
        }
    }
    
    /* Final complex computation using all variables */
    double final_d = d1 + d2 - d3 * d4 / d5 + d6;
    float final_f = f1 * f2 + f3 - f4 * f5 / f6 + f7 * f8;
    long final_l = l1 + l2 - l3 * l4 / l5;
    int final_i = a1 + a2 - a3 * a4 / a5 + a6 - a7 * a8 / a9 + a10;
    
    *checksum += (int)final_d + (int)final_f + (int)final_l + final_i;
}

/* Another function to increase compilation unit complexity */
void extra_pressure(int N, int *sum) {
    /* More variables */
    int x1 = N, x2 = N*2, x3 = N*3, x4 = N*4, x5 = N*5;
    double y1 = N*1.5, y2 = N*2.5, y3 = N*3.5;
    
    /* Loop with address computations */
    for (int i = 0; i < N && i < 50; i++) {
        /* Force spills with many live values */
        x1 = x2 + x3;
        x2 = x3 + x4;
        x3 = x4 + x5;
        x4 = x5 + x1;
        x5 = x1 + x2;
        
        y1 = y2 * y3;
        y2 = y3 / (i + 1.0);
        y3 = y1 + y2;
        
        /* Inline asm with specific register constraints */
        asm volatile (
            "mov %1, %%rax\n\t"
            "add %2, %%rax\n\t"
            "mov %%rax, %0"
            : "=r" (x1)
            : "r" (x2), "r" (x3)
            : "rax", "cc"
        );
        
        *sum += x1 + x2 + x3 + x4 + x5 + (int)y1 + (int)y2 + (int)y3;
    }
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 100;
    int checksum = 0;
    
    /* Seed RNG for variable initialization */
    srand(time(NULL));
    
    /* Call functions that trigger reloads */
    trigger_reloads(N, &checksum);
    extra_pressure(N, &checksum);
    
    /* Additional pressure in main */
    {
        /* Large local array */
        double big_array[200];
        for (int i = 0; i < 200; i++) {
            big_array[i] = i * 1.234;
        }
        
        /* Complex computation with array */
        double array_sum = 0.0;
        for (int i = 0; i < 199; i++) {
            /* Force address reloads with non-linear access */
            int idx = (i * 37) % 199;
            array_sum += big_array[idx] * big_array[idx + 1];
            
            /* Inline asm with memory operand */
            asm volatile (
                "movsd %1, %%xmm0\n\t"
                "addsd %2, %%xmm0\n\t"
                "movsd %%xmm0, %0"
                : "=m" (big_array[idx])
                : "m" (big_array[idx + 1]), "m" (array_sum)
                : "xmm0"
            );
        }
        
        checksum += (int)array_sum;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
