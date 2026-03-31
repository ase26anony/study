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
volatile int global_flag = 1;

/* Function to create register pressure and various reload scenarios */
void force_reloads(int N, int *checksum) {
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
    struct Packed packed_data;
    volatile struct Packed *volatile_packed = &packed_data;
    
    /* Initialize array with non-constant pattern */
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            arr[i][j] = (i * 7919 + j * 65537) % 1000;
        }
    }
    
    /* Initialize packed struct */
    packed_data.d = N * 3.14159;
    packed_data.i = N * 42;
    packed_data.f = N * 2.71828f;
    packed_data.l = N * 123456789L;
    packed_data.c = N % 256;
    
    /* Main computation loop - creates register pressure and reloads */
    for (int i = 1; i < N - 1; i++) {
        for (int j = 1; j < N - 1 && j < 127; j++) {
            /* Complex array access pattern forcing address reloads */
            int temp1 = arr[i][j];
            int temp2 = arr[j][i];
            int temp3 = arr[i-1][j+1];
            int temp4 = arr[i+1][j-1];
            
            /* Chain computations to keep variables live */
            a1 = a2 + temp1;
            a2 = a3 * temp2;
            a3 = a4 - temp3;
            a4 = a5 ^ temp4;
            
            f1 = f2 + (float)temp1;
            f2 = f3 * (float)temp2;
            f3 = f4 - (float)temp3;
            
            d1 = d2 + (double)temp1;
            d2 = d3 * (double)temp2;
            
            l1 = l2 + (long)temp1 * temp2;
            l2 = l3 - (long)temp3 * temp4;
            
            /* Inline assembly with conflicting constraints to force reloads */
            /* Tied operand constraint (output tied to input) */
            asm volatile (
                "add %0, %1, %2\n\t"
                : "=r"(a5)
                : "r"(a6), "0"(a7)
                : "cc"
            );
            
            /* Different register class constraints */
            asm volatile (
                "imul %0, %1\n\t"
                : "+r"(a8)
                : "r"(a9)
                : "cc"
            );
            
            /* Memory constraint forcing spill/reload */
            asm volatile (
                "mov %0, %1\n\t"
                : "=r"(a10)
                : "m"(arr[i][j])
                : "cc"
            );
            
            /* Conditional block for optional reloads */
            if (global_flag & 1) {
                /* Use different subset of variables */
                f4 = f5 + f6;
                f5 = f7 * f8;
                d3 = d4 / d5;
                l3 = l4 | l5;
                
                /* Another inline asm with specific register constraints */
                #ifdef __x86_64__
                asm volatile (
                    "addl %1, %0\n\t"
                    : "+a"(a1)
                    : "r"(a2)
                    : "cc"
                );
                #endif
            } else {
                /* Alternative path with different variable usage */
                f6 = f7 - f8;
                f7 = f1 * f2;
                d4 = d5 + d6;
                l4 = l5 ^ l1;
            }
            
            /* Access packed struct through volatile pointer - may need secondary reload */
            volatile_packed->i = a1 + a2;
            volatile_packed->f = f1 + f2;
            
            /* Update array with complex addressing */
            arr[i][j] = (a1 * a2 + a3 * a4) ^ (arr[i-1][j] + arr[i+1][j]);
            
            /* More chained computations */
            a5 = a6 + a7;
            a6 = a8 * a9;
            a7 = a10 ^ a1;
            
            f8 = f1 * 1.01f;
            d5 = d1 * 1.01;
            l5 = l1 + 1;
            
            /* Force spill by using all variables in a complex expression */
            *checksum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10
                       + (int)f1 + (int)f2 + (int)f3 + (int)f4 
                       + (int)f5 + (int)f6 + (int)f7 + (int)f8
                       + (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 + (int)d6
                       + (int)l1 + (int)l2 + (int)l3 + (int)l4 + (int)l5
                       + temp1 + temp2 + temp3 + temp4;
        }
        
        /* Toggle flag to create different execution paths */
        global_flag ^= 1;
    }
    
    /* Final computations using all variables to prevent dead code elimination */
    int final_result = 
        a1 * a2 - a3 * a4 + a5 * a6 - a7 * a8 + a9 * a10 +
        (int)(f1 * f2 - f3 * f4 + f5 * f6 - f7 * f8) +
        (int)(d1 * d2 - d3 * d4 + d5 * d6) +
        (int)(l1 + l2 - l3 + l4 - l5);
    
    *checksum += final_result;
    
    /* Use array elements to prevent elimination */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            *checksum += arr[i][j];
        }
    }
    
    /* Use packed struct data */
    *checksum += volatile_packed->i + (int)volatile_packed->f;
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 100;
    int checksum = 0;
    
    /* Seed RNG for variable initialization */
    srand(time(NULL));
    
    /* Call function multiple times to increase reload opportunities */
    for (int iter = 0; iter < 3; iter++) {
        force_reloads(N + iter, &checksum);
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
