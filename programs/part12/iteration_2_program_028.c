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

/* Volatile flag for conditional execution */
volatile int reload_flag = 1;

/* Target function with high register pressure */
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
    
    /* Packed struct with volatile pointer */
    struct Packed p;
    volatile struct Packed *vp = &p;
    
    /* Initialize array */
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            arr[i][j] = i * 1000 + j;
        }
    }
    
    /* Complex loop with many live variables */
    for (int i = 1; i < N; i++) {
        for (int j = 1; j < N; j++) {
            /* Force address reloads with complex array access */
            int temp = arr[i][j] + arr[j][i];
            
            /* Inline assembly with conflicting constraints */
            /* Tied operand forcing reload */
            asm volatile (
                "add %0, %1, %2"
                : "=r"(a1)
                : "r"(a2), "0"(a3)
                : "cc"
            );
            
            /* Another asm with memory constraint */
            asm volatile (
                "mov %0, %1"
                : "=r"(a4)
                : "m"(arr[i][j])
            );
            
            /* Float/double operations to use FP registers */
            f1 = f2 + f3;
            f4 = f5 * f6;
            d1 = d2 - d3;
            d4 = d5 / d6;
            
            /* Chain computations to keep variables live */
            a5 = a1 + a2 + a3 + a4;
            a6 = a5 * a7 - a8;
            a9 = a6 / (a10 + 1);
            
            /* Conditional block for optional reloads */
            if (reload_flag) {
                /* Use different subset of variables */
                l1 = l2 + l3;
                l4 = l5 * l1;
                
                /* More inline asm with specific constraints */
                asm volatile (
                    "sub %0, %1, %2"
                    : "=r"(l2)
                    : "r"(l3), "r"(l4)
                );
                
                /* Update array with complex addressing */
                arr[i-1][j-1] = arr[j-1][i-1] + temp;
            } else {
                /* Alternative path with different variables */
                f7 = f8 * 2.0f;
                d5 = d6 * 3.0;
            }
            
            /* Access packed struct through volatile pointer */
            vp->i = a1 + a2;
            vp->d = d1 + d2;
            vp->f = f1 + f2;
            vp->l = l1 + l2;
            
            /* More arithmetic to create data dependencies */
            a2 = a3 * a4 - a5;
            a3 = a4 + a6 / a7;
            a7 = a8 ^ a9 | a10;
            
            f2 = f3 * 1.5f - f4;
            f3 = f4 + f5 / f6;
            
            d2 = d3 * 1.7 - d4;
            d3 = d4 + d5 * d6;
            
            l2 = l3 << 2;
            l3 = l4 >> 1;
            
            /* Swap array elements forcing address reloads */
            int tmp = arr[i][j];
            arr[i][j] = arr[j][i];
            arr[j][i] = tmp;
        }
        
        /* Update checksum with all variables */
        *checksum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
        *checksum += (int)f1 + (int)f2 + (int)f3 + (int)f4;
        *checksum += (int)f5 + (int)f6 + (int)f7 + (int)f8;
        *checksum += (int)d1 + (int)d2 + (int)d3;
        *checksum += (int)d4 + (int)d5 + (int)d6;
        *checksum += (int)l1 + (int)l2 + (int)l3 + (int)l4 + (int)l5;
    }
    
    /* Final array checksum */
    for (int i = 0; i < 128 && i < N; i++) {
        for (int j = 0; j < 128 && j < N; j++) {
            *checksum += arr[i][j];
        }
    }
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    int checksum = 0;
    
    /* Seed RNG for variable initialization */
    srand(time(NULL));
    
    /* Call reload-intensive function */
    trigger_reloads(N, &checksum);
    
    /* Print checksum to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    /* Additional reload pressure in main */
    {
        /* More variables in different scope */
        volatile int v1 = rand(), v2 = rand(), v3 = rand();
        volatile float vf1 = rand() * 1.0f, vf2 = rand() * 2.0f;
        volatile double vd1 = rand() * 3.0, vd2 = rand() * 4.0;
        
        /* Inline asm with multiple constraints */
        asm volatile (
            "imul %0, %1, %2"
            : "=r"(v1)
            : "r"(v2), "r"(v3)
        );
        
        /* Mixed-type computation */
        checksum += v1 + (int)vf1 + (int)vf2 + (int)vd1 + (int)vd2;
    }
    
    printf("Final checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
