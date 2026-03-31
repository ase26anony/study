/* reload_test.c - Program to trigger GCC reload pass initialization */
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
volatile int global_flag = 0;
volatile int *volatile volatile_ptr;

/* Target function with high register pressure */
__attribute__((noinline))
static long complex_reload_function(int N, int *checksum) {
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
    
    /* Packed struct access */
    struct PackedStruct ps;
    ps.d = d1;
    ps.i = a1;
    ps.f = f1;
    ps.l = l1;
    
    /* Complex loop with many live variables */
    for (int i = 0; i < N && i < 128; i++) {
        for (int j = 0; j < N && j < 128; j++) {
            /* Force address reloads with complex indexing */
            arr[i][j] = arr[j][i] + i * j;
            
            /* Chain computations to keep variables live */
            a1 = a2 + a3;
            a2 = a3 * a4;
            a3 = a4 - a5;
            a4 = a5 ^ a6;
            a5 = a6 | a7;
            a6 = a7 & a8;
            a7 = a8 + a9;
            a8 = a9 - a10;
            a9 = a10 * a1;
            a10 = a1 + a2;
            
            /* Floating point computations */
            f1 = f2 + f3;
            f2 = f3 * f4;
            f3 = f4 - f5;
            f4 = f5 / f6;
            f5 = f6 + f7;
            f6 = f7 * f8;
            f7 = f8 - f1;
            f8 = f1 / f2;
            
            /* Double computations */
            d1 = d2 + d3;
            d2 = d3 * d4;
            d3 = d4 - d5;
            d4 = d5 / d6;
            d5 = d6 + d1;
            d6 = d1 * d2;
            
            /* Long computations */
            l1 = l2 + l3;
            l2 = l3 * l4;
            l3 = l4 - l5;
            l4 = l5 ^ l1;
            l5 = l1 | l2;
            
            /* Inline assembly with conflicting constraints */
            /* Force input reloads with tied operands */
            asm volatile (
                "add %0, %1, %2\n\t"
                : "=r"(a1)
                : "r"(a2), "0"(a3)
                : "cc"
            );
            
            /* Force output reload with memory constraint */
            asm volatile (
                "mov %0, %1\n\t"
                : "=m"(arr[i][j])
                : "r"(a1)
            );
            
            /* Another asm with float constraints */
            asm volatile (
                "fadds %0, %1, %2\n\t"
                : "=f"(f1)
                : "f"(f2), "f"(f3)
            );
            
            /* Conditional block for optional reloads */
            if (global_flag) {
                /* Use different subset of variables */
                int t1 = a1 + a10;
                float t2 = f1 * f8;
                double t3 = d1 / d6;
                long t4 = l1 & l5;
                
                /* More asm with specific register constraints */
                #ifdef __x86_64__
                asm volatile (
                    "addl %%eax, %%ebx\n\t"
                    : "+b"(t1)
                    : "a"(t4)
                    : "cc"
                );
                #elif defined(__aarch64__)
                asm volatile (
                    "add %w0, %w0, %w1\n\t"
                    : "+r"(t1)
                    : "r"(t4)
                );
                #endif
                
                arr[i][j] += t1;
            }
            
            /* Access packed struct through volatile pointer */
            volatile_ptr = (volatile int*)&ps;
            a1 += *volatile_ptr;
            
            /* Update packed struct member */
            ps.i = arr[i][j];
            ps.d = d1 + d2;
            ps.f = f1 * f2;
            ps.l = l1 + l2;
        }
    }
    
    /* Compute checksum using all variables */
    *checksum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    *checksum += (int)f1 + (int)f2 + (int)f3 + (int)f4 + 
                 (int)f5 + (int)f6 + (int)f7 + (int)f8;
    *checksum += (int)d1 + (int)d2 + (int)d3 + 
                 (int)d4 + (int)d5 + (int)d6;
    *checksum += (int)l1 + (int)l2 + (int)l3 + (int)l4 + (int)l5;
    
    /* Add array checksum */
    for (int i = 0; i < N && i < 128; i++) {
        for (int j = 0; j < N && j < 128; j++) {
            *checksum += arr[i][j];
        }
    }
    
    return l1 + l2 + l3 + l4 + l5;
}

/* Another function to create cross-function register pressure */
__attribute__((noinline))
static void helper_function(int *arr, int size) {
    int temp[10];
    for (int i = 0; i < size && i < 10; i++) {
        /* Force reloads with inline asm */
        asm volatile (
            "mov %0, %1\n\t"
            : "=r"(temp[i])
            : "r"(arr[i])
        );
    }
    
    /* Chain computations */
    for (int i = 1; i < size && i < 10; i++) {
        temp[i] = temp[i-1] + temp[i];
    }
    
    /* Copy back */
    for (int i = 0; i < size && i < 10; i++) {
        arr[i] = temp[i];
    }
}

int main(int argc, char *argv[]) {
    int N = 100;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N < 10) N = 10;
        if (N > 1000) N = 1000;
    }
    
    srand(time(NULL));
    global_flag = rand() & 1;
    
    int checksum1 = 0, checksum2 = 0;
    int arr[100];
    
    /* Initialize array with random values */
    for (int i = 0; i < 100; i++) {
        arr[i] = rand() % 100;
    }
    
    /* Call helper to create additional reload context */
    helper_function(arr, 10);
    
    /* Main reload-intensive function */
    long result = complex_reload_function(N, &checksum1);
    
    /* Use results to prevent optimization */
    checksum2 = checksum1 + (int)result + arr[0];
    
    printf("Checksum1: %d\n", checksum1);
    printf("Checksum2: %d\n", checksum2);
    printf("Result: %ld\n", result);
    
    return checksum1 != 0 ? 0 : 1;
}
