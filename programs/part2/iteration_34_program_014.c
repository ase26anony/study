/* test_secondary_reloads.c
 * This program forces GCC to generate secondary reloads by:
 * 1. Using x87 floating-point constraints ("t", "u") in inline assembly
 * 2. Creating high register pressure with many volatile variables
 * 3. Using builtins with fixed register requirements
 * 4. Employing multi-alternative constraints that require secondary reloads
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[20];
volatile int global_ints[20];

/* Prevent inlining to ensure reload logic isn't optimized away */
__attribute__((noinline, noipa))
int test_secondary_reloads(int seed) {
    /* Create high register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8, ld9, ld10;
    volatile long double ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    
    /* Initialize with seed-dependent values to avoid constant propagation */
    i1 = seed + 1;
    i2 = seed + 2;
    i3 = seed + 3;
    i4 = seed + 4;
    i5 = seed + 5;
    i6 = seed * 2;
    i7 = seed * 3;
    i8 = seed * 4;
    i9 = seed * 5;
    i10 = seed * 6;
    i11 = seed ^ 0x1234;
    i12 = seed ^ 0x5678;
    i13 = seed ^ 0x9ABC;
    i14 = seed ^ 0xDEF0;
    i15 = seed ^ 0x1357;
    
    /* Initialize long double values */
    ld1 = (long double)seed + 0.1L;
    ld2 = (long double)seed + 0.2L;
    ld3 = (long double)seed + 0.3L;
    ld4 = (long double)seed + 0.4L;
    ld5 = (long double)seed + 0.5L;
    ld6 = (long double)seed * 1.1L;
    ld7 = (long double)seed * 1.2L;
    ld8 = (long double)seed * 1.3L;
    ld9 = (long double)seed * 1.4L;
    ld10 = (long double)seed * 1.5L;
    ld11 = (long double)(seed ^ 0x2468) / 100.0L;
    ld12 = (long double)(seed ^ 0x4680) / 100.0L;
    ld13 = (long double)(seed ^ 0x6802) / 100.0L;
    ld14 = (long double)(seed ^ 0x8024) / 100.0L;
    ld15 = (long double)(seed ^ 0x0246) / 100.0L;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    {
        unsigned long long tsc1, tsc2;
        tsc1 = __builtin_ia32_rdtsc();
        /* Do some work to prevent optimization */
        i1 = i1 ^ (tsc1 & 0xFFFFFFFF);
        i2 = i2 ^ (tsc1 >> 32);
        tsc2 = __builtin_ia32_rdtsc();
        i3 = i3 ^ (tsc2 & 0xFFFFFFFF);
        i4 = i4 ^ (tsc2 >> 32);
    }
    
    volatile int loop_counter;
    for (loop_counter = 0; loop_counter < 3; loop_counter++) {
        /* CRITICAL: Inline assembly with x87 constraints forcing secondary reloads */
        
        /* Pattern 1: Simple x87 operation with "t" constraint (top of x87 stack) */
        /* This forces values into x87 registers, potentially requiring secondary reloads */
        asm volatile (
            "faddp %%st(1), %%st"
            : "=t" (ld1)
            : "0" (ld1), "t" (ld2)
            : "st(1)"
        );
        
        /* Pattern 2: Multi-alternative constraint with "rm,t" 
         * The compiler may choose "t" alternative, requiring secondary reload
         * for the integer operand */
        {
            long double result;
            int int_val = i1 + loop_counter;
            asm volatile (
                "fildl %2\n\t"
                "faddp %%st(1), %%st"
                : "=t" (result)
                : "0" (ld3), "rm,t" (int_val)
                : "st(1)"
            );
            ld3 = result;
        }
        
        /* Pattern 3: Using "u" constraint (second x87 register) */
        asm volatile (
            "fmulp %%st(1), %%st"
            : "=t" (ld4)
            : "0" (ld4), "u" (ld5)
            : "st(1)"
        );
        
        /* Pattern 4: Complex pattern mixing x87 and general registers */
        {
            long double temp1, temp2;
            int int_val1 = i2 + loop_counter;
            int int_val2 = i3 + loop_counter;
            
            /* First load integer into x87 stack */
            asm volatile (
                "fildl %1\n\t"
                : "=t" (temp1)
                : "m" (int_val1)
            );
            
            /* Second load with different constraint */
            asm volatile (
                "fildl %1\n\t"
                : "=t" (temp2)
                : "m" (int_val2)
            );
            
            /* Operation on both x87 values */
            asm volatile (
                "faddp %%st(1), %%st"
                : "=t" (temp1)
                : "0" (temp1), "t" (temp2)
                : "st(1)"
            );
            
            /* Store back to memory through x87 */
            asm volatile (
                "fistpl %0"
                : "=m" (i5)
                : "t" (temp1)
            );
        }
        
        /* Pattern 5: Division with fixed register constraint ("a" for eax) */
        {
            int dividend = i6;
            int divisor = i7 + 1; /* Avoid division by zero */
            int quotient, remainder;
            
            asm volatile (
                "divl %4"
                : "=a" (quotient), "=d" (remainder)
                : "a" (dividend), "d" (0), "r" (divisor)
            );
            
            i8 = quotient;
            i9 = remainder;
        }
        
        /* Mix operations to keep all variables live */
        ld6 = ld6 * ld7 + (long double)i4;
        ld7 = ld7 / ld8 - (long double)i5;
        ld8 = ld8 + ld9 * (long double)(i6 ^ i7);
        
        /* Use CRC32 builtin which has fixed register constraints */
        {
            unsigned int crc = 0xFFFFFFFF;
            crc = __builtin_ia32_crc32qi(crc, (unsigned char)i10);
            crc = __builtin_ia32_crc32hi(crc, (unsigned short)i11);
            crc = __builtin_ia32_crc32si(crc, (unsigned int)i12);
            i13 = crc ^ i13;
        }
        
        /* More x87 operations to increase pressure */
        asm volatile (
            "fsubrp %%st(1), %%st"
            : "=t" (ld9)
            : "0" (ld9), "t" (ld10)
            : "st(1)"
        );
        
        asm volatile (
            "fmulp %%st(1), %%st"
            : "=t" (ld11)
            : "0" (ld11), "u" (ld12)
            : "st(1)"
        );
    }
    
    /* Store results to global arrays to prevent elimination */
    global_results[0] = ld1;
    global_results[1] = ld2;
    global_results[2] = ld3;
    global_results[3] = ld4;
    global_results[4] = ld5;
    global_results[5] = ld6;
    global_results[6] = ld7;
    global_results[7] = ld8;
    global_results[8] = ld9;
    global_results[9] = ld10;
    global_results[10] = ld11;
    global_results[11] = ld12;
    global_results[12] = ld13;
    global_results[13] = ld14;
    global_results[14] = ld15;
    
    global_ints[0] = i1;
    global_ints[1] = i2;
    global_ints[2] = i3;
    global_ints[3] = i4;
    global_ints[4] = i5;
    global_ints[5] = i6;
    global_ints[6] = i7;
    global_ints[7] = i8;
    global_ints[8] = i9;
    global_ints[9] = i10;
    global_ints[10] = i11;
    global_ints[11] = i12;
    global_ints[12] = i13;
    global_ints[13] = i14;
    global_ints[14] = i15;
    
    /* Return a checksum */
    return (int)ld1 + (int)ld2 + i1 + i2 + i3;
}

int main(int argc, char *argv[]) {
    int seed = 42; /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    int result = test_secondary_reloads(seed);
    
    printf("Result: %d\n", result);
    
    /* Use results to prevent optimization */
    for (int i = 0; i < 15; i++) {
        printf("Result[%d]: %.3Lf, Int[%d]: %d\n", 
               i, global_results[i], i, global_ints[i]);
    }
    
    return 0;
}
