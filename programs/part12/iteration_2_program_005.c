/* reload_coverage.c - Program to trigger GCC reload pass initialization */
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
volatile int volatile_flag = 1;
volatile int volatile_index = 0;

/* Target function with high register pressure */
__attribute__((noinline))
unsigned long long induce_reloads(int N, int seed) {
    /* Declare many scalar variables to exceed available registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d1, d2, d3, d4, d5, d6;
    long l1, l2, l3, l4, l5, l6, l7, l8;
    
    /* Multi-dimensional array for address reloads */
    int arr[128][128];
    
    /* Packed struct for secondary reloads */
    struct PackedStruct ps[64];
    
    /* Initialize with pseudo-random values to prevent constant propagation */
    srand(seed);
    a1 = rand(); a2 = rand(); a3 = rand(); a4 = rand(); a5 = rand();
    a6 = rand(); a7 = rand(); a8 = rand(); a9 = rand(); a10 = rand();
    
    f1 = rand() / 1000.0f; f2 = rand() / 1000.0f; f3 = rand() / 1000.0f;
    f4 = rand() / 1000.0f; f5 = rand() / 1000.0f; f6 = rand() / 1000.0f;
    f7 = rand() / 1000.0f; f8 = rand() / 1000.0f;
    
    d1 = rand() / 10000.0; d2 = rand() / 10000.0; d3 = rand() / 10000.0;
    d4 = rand() / 10000.0; d5 = rand() / 10000.0; d6 = rand() / 10000.0;
    
    l1 = rand() * 1000L; l2 = rand() * 1000L; l3 = rand() * 1000L;
    l4 = rand() * 1000L; l5 = rand() * 1000L; l6 = rand() * 1000L;
    l7 = rand() * 1000L; l8 = rand() * 1000L;
    
    /* Initialize array with non-constant pattern */
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            arr[i][j] = (i * 7919 + j * 65537) & 0xFFFF;
        }
    }
    
    /* Initialize packed structs */
    for (int i = 0; i < 64; i++) {
        ps[i].d = i * 3.14159;
        ps[i].i = i * 17;
        ps[i].f = i * 2.71828f;
        ps[i].l = i * 1000000L;
        ps[i].c = i & 0xFF;
        ps[i].s = i * 3;
    }
    
    unsigned long long checksum = 0;
    
    /* Main computation loop with high register pressure */
    for (int i = 1; i < N; i++) {
        for (int j = 1; j < N; j++) {
            /* Complex array access pattern forcing address reloads */
            int idx1 = (i * 13 + j * 17) % 127;
            int idx2 = (i * 19 + j * 23) % 127;
            
            /* Chain of arithmetic operations keeping many variables live */
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
            
            f1 = f2 + f3;
            f2 = f3 * f4;
            f3 = f4 - f5;
            f4 = f5 * f6;
            f5 = f6 + f7;
            f6 = f7 - f8;
            f7 = f8 * f1;
            f8 = f1 + f2;
            
            d1 = d2 + d3;
            d2 = d3 * d4;
            d3 = d4 - d5;
            d4 = d5 * d6;
            d5 = d6 + d1;
            d6 = d1 - d2;
            
            l1 = l2 + l3;
            l2 = l3 * l4;
            l3 = l4 - l5;
            l4 = l5 ^ l6;
            l5 = l6 | l7;
            l6 = l7 & l8;
            l7 = l8 + l1;
            l8 = l1 - l2;
            
            /* Inline assembly with conflicting constraints to force reloads */
            /* Tied operand constraint (output = input 0) */
            asm volatile (
                "add %0, %1, %2\n\t"
                : "=r"(a1)
                : "r"(a2), "0"(a3)
                : "cc"
            );
            
            /* Memory constraint forcing spill/reload */
            asm volatile (
                "mov %0, %1\n\t"
                : "=r"(a4)
                : "m"(a5)
            );
            
            /* Multiple constraints on the same variable */
            asm volatile (
                "imul %0, %1\n\t"
                : "+r"(a6), "+r"(a7)
                :
                : "cc"
            );
            
            /* Float/double operations with register constraints */
            asm volatile (
                "addsd %0, %1, %2\n\t"
                : "=x"(d1)
                : "x"(d2), "x"(d3)
            );
            
            /* Access packed struct through volatile pointer - forces secondary reloads */
            volatile struct PackedStruct *vps = &ps[(i + j) % 64];
            double temp_d = vps->d;
            int temp_i = vps->i;
            
            /* Use packed struct members in computation */
            d1 += temp_d;
            a1 += temp_i;
            
            /* Array manipulation with complex addressing */
            int temp = arr[idx1][idx2];
            arr[idx2][idx1] = arr[i % 127][j % 127] + temp;
            arr[i % 127][j % 127] = temp + a1;
            
            /* Conditional block for optional reloads */
            if (volatile_flag) {
                /* Different computation path using subset of variables */
                f1 = f3 * f5 + f7;
                d2 = d4 * d6 + temp_d;
                l3 = l5 ^ l7 | l1;
                
                /* Another inline asm in conditional path */
                asm volatile (
                    "sub %0, %1, %2\n\t"
                    : "=r"(a8)
                    : "r"(a9), "r"(a10)
                    : "cc"
                );
            } else {
                /* Alternative path */
                f2 = f4 * f6 + f8;
                d3 = d5 * d1 + temp_d;
                l4 = l6 ^ l8 | l2;
            }
            
            /* Update volatile index */
            volatile_index = (volatile_index + 1) % 100;
            
            /* More arithmetic to maintain dependencies */
            a1 = a1 * 3 + a2;
            a2 = a2 / 2 + a3;
            a3 = a3 + a4 * 5;
            a4 = a4 - a5 / 3;
            a5 = a5 ^ a6;
            a6 = a6 | a7;
            a7 = a7 & a8;
            a8 = a8 + a9;
            a9 = a9 - a10;
            a10 = a10 * a1;
            
            /* Accumulate checksum to prevent optimization */
            checksum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
            checksum += (int)f1 + (int)f2 + (int)f3 + (int)f4;
            checksum += (long)d1 + (long)d2;
            checksum += l1 + l2 + l3 + l4;
        }
    }
    
    /* Final aggregation with array */
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            checksum += arr[i][j];
        }
    }
    
    /* Use packed structs in final computation */
    for (int i = 0; i < 64; i++) {
        checksum += (int)ps[i].d + ps[i].i + (int)ps[i].f + ps[i].l;
    }
    
    return checksum;
}

/* Wrapper function to increase register pressure further */
__attribute__((noinline))
unsigned long long wrapper_function(int N, int iterations) {
    unsigned long long total = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Call with different seeds to vary computation */
        total += induce_reloads(N, iter * 12345);
        
        /* Additional local variables in wrapper */
        int extra1 = iter * 3, extra2 = iter * 5, extra3 = iter * 7;
        float extra4 = iter * 1.1f, extra5 = iter * 2.2f;
        double extra6 = iter * 3.3, extra7 = iter * 4.4;
        
        /* More inline asm with constraints */
        asm volatile (
            "lea (%1, %2, 2), %0\n\t"
            : "=r"(extra1)
            : "r"(extra2), "r"(extra3)
        );
        
        asm volatile (
            "addsd %0, %1, %2\n\t"
            : "=x"(extra6)
            : "x"(extra6), "x"(extra7)
        );
        
        total += extra1 + (int)extra4 + (int)extra5 + (long)extra6;
    }
    
    return total;
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    int iterations = (argc > 2) ? atoi(argv[2]) : 5;
    
    if (N > 100) N = 100; /* Safety limit */
    if (iterations > 10) iterations = 10;
    
    printf("Starting reload coverage test with N=%d, iterations=%d\n", N, iterations);
    
    clock_t start = clock();
    unsigned long long result = wrapper_function(N, iterations);
    clock_t end = clock();
    
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Result: %llu\n", result);
    printf("Time elapsed: %.3f seconds\n", elapsed);
    printf("Volatile index: %d\n", volatile_index);
    
    return 0;
}
