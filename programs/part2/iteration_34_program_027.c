/* test_secondary_reloads.c
 * This program forces GCC to generate secondary reloads by:
 * 1. Using x87 floating-point operations with 't' and 'u' constraints
 * 2. Creating high register pressure with many volatile variables
 * 3. Using inline assembly with multiple alternative constraints
 * 4. Mixing x87 operations with fixed-register builtins
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[20];
volatile int global_ints[20];

/* Prevent inlining to ensure reload happens in this function */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Create high register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8, ld9, ld10;
    volatile long double ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    
    /* Initialize with seed-dependent values to prevent constant folding */
    i1 = seed + 1;
    i2 = seed * 2;
    i3 = seed ^ 0x1234;
    i4 = seed + 0x5678;
    i5 = seed * 3;
    i6 = seed - 100;
    i7 = seed | 0xABCD;
    i8 = seed & 0xF0F0;
    i9 = seed << 3;
    i10 = seed >> 2;
    i11 = seed + 0x1111;
    i12 = seed * 5;
    i13 = seed ^ 0x9999;
    i14 = seed + 0x7777;
    i15 = seed * 7;
    
    /* Initialize long doubles using integer values */
    ld1 = (long double)i1 * 1.1L;
    ld2 = (long double)i2 * 1.2L;
    ld3 = (long double)i3 * 1.3L;
    ld4 = (long double)i4 * 1.4L;
    ld5 = (long double)i5 * 1.5L;
    ld6 = (long double)i6 * 1.6L;
    ld7 = (long double)i7 * 1.7L;
    ld8 = (long double)i8 * 1.8L;
    ld9 = (long double)i9 * 1.9L;
    ld10 = (long double)i10 * 2.0L;
    ld11 = (long double)i11 * 2.1L;
    ld12 = (long double)i12 * 2.2L;
    ld13 = (long double)i13 * 2.3L;
    ld14 = (long double)i14 * 2.4L;
    ld15 = (long double)i15 * 2.5L;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    {
        unsigned long long ts1, ts2;
        ts1 = __builtin_ia32_rdtsc();
        i1 = (int)(ts1 & 0xFFFFFFFF);
        ts2 = __builtin_ia32_rdtsc();
        i2 = (int)(ts2 & 0xFFFFFFFF);
    }
    
    /* Force x87 operations with 't' constraint (top of x87 stack) */
    /* This should trigger secondary reloads for moving values into x87 regs */
    
    /* Example 1: Simple x87 addition with 't' constraint */
    asm volatile (
        "faddp %%st(1), %%st"
        : "=t" (ld1)
        : "0" (ld1), "t" (ld2)
        : "st(1)"
    );
    
    /* Example 2: Multiplication with 't' and 'u' constraints */
    asm volatile (
        "fmulp %%st(1), %%st"
        : "=t" (ld3)
        : "0" (ld3), "u" (ld4)
        : "st(1)"
    );
    
    /* CRITICAL: Inline asm with multiple alternative constraints
     * The "rm,t" constraint may force a secondary reload when 't' is chosen
     * for the integer operand that needs to be in x87 register */
    {
        volatile int temp_int = i3;
        asm volatile (
            "# Multi-alternative constraint test\n\t"
            "fildl %2\n\t"
            "faddp %%st(1), %%st"
            : "=t" (ld5)
            : "0" (ld5), "rm,t" (temp_int)
            : "st(1)"
        );
    }
    
    /* Example 3: Division with mixed constraints */
    asm volatile (
        "fdivp %%st(1), %%st"
        : "=t" (ld6)
        : "0" (ld6), "t" (ld7)
        : "st(1)"
    );
    
    /* Create more complex scenario with chained operations */
    {
        volatile long double tmp1, tmp2;
        volatile int int_val = i4;
        
        /* First load integer into x87 stack */
        asm volatile (
            "fildl %1"
            : "=t" (tmp1)
            : "m" (int_val)
        );
        
        /* Then use it in operation with another x87 value */
        asm volatile (
            "faddp %%st(1), %%st"
            : "=t" (ld8)
            : "0" (ld8), "t" (tmp1)
            : "st(1)"
        );
    }
    
    /* Use CRC32 builtin which has fixed register constraints */
    {
        unsigned int crc = 0xFFFFFFFF;
        crc = __builtin_ia32_crc32qi(crc, (unsigned char)i5);
        i6 = (int)crc;
    }
    
    /* More x87 operations to increase pressure */
    asm volatile (
        "fsubp %%st(1), %%st"
        : "=t" (ld9)
        : "0" (ld9), "t" (ld10)
        : "st(1)"
    );
    
    /* Complex inline asm with output in memory and x87 input */
    {
        volatile long double result;
        asm volatile (
            "# Complex pattern\n\t"
            "fldt %1\n\t"
            "fldt %2\n\t"
            "fmulp %%st(1), %%st\n\t"
            "fstpt %0"
            : "=m" (result)
            : "m" (ld11), "m" (ld12)
            : "st", "st(1)"
        );
        ld13 = result;
    }
    
    /* Loop to create more register pressure and prevent optimization */
    {
        volatile int counter;
        for (counter = 0; counter < 3; counter++) {
            /* Mix integer and x87 operations in loop */
            i7 += counter;
            asm volatile (
                "fadds %1\n\t"
                : "+t" (ld14)
                : "m" (ld15)
            );
        }
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
    global_ints[8] = i8;
    global_ints[9] = i9;
    global_ints[10] = i10;
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call multiple times with different seeds if provided */
    test_secondary_reloads(seed);
    
    if (argc > 2) {
        test_secondary_reloads(seed + 1);
    }
    
    /* Print something to prevent complete optimization */
    printf("Result: %Lf\n", global_results[0]);
    
    return 0;
}
