/* reload_trigger.c - Program to force GCC reload pass initialization */
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

/* Volatile variables to prevent optimization */
volatile int volatile_flag = 1;
volatile int volatile_index = 0;

/* Target function with high register pressure */
__attribute__((noinline))
static long heavy_computation(int N, int *checksum) {
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
        packed_data[i].f = i * 2.345f;
        packed_data[i].l = i * 1000L;
        packed_data[i].c = i;
    }
    
    /* Complex nested loops with many live variables */
    for (int i = 1; i < N && i < 127; i++) {
        for (int j = 1; j < N && j < 127; j++) {
            /* Force address reloads with complex array indexing */
            int idx1 = (i * j) % 128;
            int idx2 = (i + j) % 128;
            
            /* Use many variables in computation to keep them live */
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
                "addl $1, %0\n\t"
                : "=r" (a3)
                : "0" (a4)
                : "cc"
            );
            
            /* Force memory constraint vs register constraint */
            asm volatile (
                "movl %1, %%eax\n\t"
                "addl %%eax, %0\n\t"
                : "+m" (arr[i][j])
                : "r" (a5)
                : "%eax", "cc"
            );
            
            /* Float/double assembly to force FP reloads */
            asm volatile (
                "addss %1, %0\n\t"
                : "+x" (f1)
                : "x" (f2)
            );
            
            /* Conditional block for optional reloads */
            if (volatile_flag) {
                /* Use different subset of variables */
                a6 = a7 + a8;
                f3 = f4 * f5;
                d2 = d3 + d4;
                l2 = l3 + l4;
                
                /* More assembly in conditional path */
                asm volatile (
                    "subl %1, %0\n\t"
                    : "+r" (a6)
                    : "r" (a7)
                    : "cc"
                );
            } else {
                /* Alternative path with different variables */
                a9 = a10 * a1;
                f6 = f7 / f8;
                d5 = d6 - d1;
                l5 = l1 * l2;
            }
            
            /* Access packed struct through volatile pointer */
            volatile struct Packed *volatile_ptr = &packed_data[(i + j) % 16];
            
            /* Force reloads for unaligned packed struct access */
            int packed_val = volatile_ptr->i;
            double packed_dbl = volatile_ptr->d;
            
            /* Use packed values in computation */
            a1 += packed_val;
            d1 += packed_dbl;
            
            /* Complex array operation forcing address reloads */
            arr[idx1][idx2] = arr[idx2][idx1] + a1;
            
            /* Chain computations to maintain dependencies */
            a2 = a3 + arr[i][j];
            a3 = a4 + arr[j][i];
            a4 = a5 + a2;
            a5 = a6 + a3;
            
            f2 = f3 + f1;
            f3 = f4 + f2;
            f4 = f5 + f3;
            
            d3 = d4 + d1;
            d4 = d5 + d2;
            d5 = d6 + d3;
            
            l3 = l4 + l1;
            l4 = l5 + l2;
            l5 = l1 + l3;
        }
        
        /* Update volatile index to prevent loop optimizations */
        volatile_index = i;
    }
    
    /* Compute checksum using all variables */
    long total = 0;
    total += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    total += (long)(f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8);
    total += (long)(d1 + d2 + d3 + d4 + d5 + d6);
    total += l1 + l2 + l3 + l4 + l5;
    
    /* Add array checksum */
    for (int i = 0; i < 16 && i < N; i++) {
        for (int j = 0; j < 16 && j < N; j++) {
            total += arr[i][j];
        }
    }
    
    *checksum = (int)total;
    return total;
}

/* Another function to create cross-function register pressure */
__attribute__((noinline))
static void additional_pressure(int N, int *arr) {
    int x1 = N, x2 = N*2, x3 = N*3, x4 = N*4, x5 = N*5;
    float y1 = N*1.5f, y2 = N*2.5f, y3 = N*3.5f;
    double z1 = N*1.25, z2 = N*2.25;
    
    for (int i = 0; i < N && i < 100; i++) {
        /* Force spilling with many live variables */
        x1 = x2 + x3;
        x2 = x3 + x4;
        x3 = x4 + x5;
        x4 = x5 + x1;
        x5 = x1 + x2;
        
        y1 = y2 + y3;
        y2 = y3 + y1;
        y3 = y1 + y2;
        
        z1 = z2 + z1;
        z2 = z1 + z2;
        
        /* Memory access with index computation */
        arr[i % 100] = x1 + (int)y1 + (int)z1;
        
        /* Assembly with specific register constraints for x86 */
        #ifdef __x86_64__
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %%eax, %0\n\t"
            "movl %0, %%ebx\n\t"
            : "+r" (x1)
            : "r" (x2)
            : "%eax", "%ebx", "cc"
        );
        #endif
    }
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    if (N < 10) N = 10;
    if (N > 1000) N = 1000;
    
    srand(time(NULL));
    
    int checksum1 = 0;
    int arr[100];
    
    /* Initialize array with random values */
    for (int i = 0; i < 100; i++) {
        arr[i] = rand() % 1000;
    }
    
    /* Call functions to create register pressure */
    long result1 = heavy_computation(N, &checksum1);
    additional_pressure(N, arr);
    
    /* Compute final checksum */
    long final_checksum = result1;
    for (int i = 0; i < 100; i++) {
        final_checksum += arr[i];
    }
    
    printf("Checksum: %ld\n", final_checksum);
    printf("Volatile index: %d\n", volatile_index);
    
    return (final_checksum > 0) ? 0 : 1;
}
