/* test_secondary_reloads.c
 * This program forces GCC to generate secondary reloads for x87 registers
 * to cover the initialization of secondary_* fields in struct reload.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[20];
volatile int global_ints[20];

/* Prevent inlining to ensure reload logic isn't optimized away */
__attribute__((noinline, noipa))
uint64_t test_secondary_reloads(int seed) {
    /* High register pressure: many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8, ld9, ld10;
    volatile long double ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    volatile uint64_t tsc1, tsc2;
    volatile int loop_counter;
    uint64_t checksum = 0;
    
    /* Initialize with seed-dependent values to avoid constant propagation */
    i1 = seed + 1;
    i2 = seed * 2 + 2;
    i3 = seed * 3 + 3;
    i4 = seed * 4 + 4;
    i5 = seed * 5 + 5;
    i6 = seed * 6 + 6;
    i7 = seed * 7 + 7;
    i8 = seed * 8 + 8;
    i9 = seed * 9 + 9;
    i10 = seed * 10 + 10;
    i11 = seed * 11 + 11;
    i12 = seed * 12 + 12;
    i13 = seed * 13 + 13;
    i14 = seed * 14 + 14;
    i15 = seed * 15 + 15;
    
    /* Initialize long doubles with non-constant values */
    ld1 = (long double)(i1) / 3.1415926535L;
    ld2 = (long double)(i2) / 2.7182818284L;
    ld3 = (long double)(i3) * 1.4142135623L;
    ld4 = (long double)(i4) * 1.7320508075L;
    ld5 = (long double)(i5) + 12345.6789L;
    ld6 = (long double)(i6) - 9876.54321L;
    ld7 = (long double)(i7) / 1.23456789L;
    ld8 = (long double)(i8) * 3.1415926535L;
    ld9 = (long double)(i9) / 2.7182818284L;
    ld10 = (long double)(i10) * 1.6180339887L;
    ld11 = (long double)(i11) - 31415.926535L;
    ld12 = (long double)(i12) + 27182.818284L;
    ld13 = (long double)(i13) * 2.3025850929L;
    ld14 = (long double)(i14) / 1.4426950408L;
    ld15 = (long double)(i15) * 0.6931471805L;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    tsc1 = __builtin_ia32_rdtsc();
    
    /* Force integer values into registers before x87 operations */
    i1 = i1 * i2 + i3;
    i2 = i4 ^ i5 | i6;
    i3 = i7 + i8 - i9;
    
    /* Loop to increase register pressure and prevent optimization */
    for (loop_counter = 0; loop_counter < 3; loop_counter++) {
        /* CRITICAL: Inline asm with x87 constraints that may need secondary reloads */
        
        /* Pattern 1: x87 operation with "t" (top of stack) constraint */
        asm volatile (
            "fldt %2\n\t"           /* Load ld2 onto x87 stack */
            "fldt %1\n\t"           /* Load ld1 onto x87 stack */
            "faddp %%st, %%st(1)\n\t" /* st(1) = st(1) + st, pop st */
            "fstpt %0"              /* Store result */
            : "=m" (ld1)            /* Output in memory (forces store/reload) */
            : "m" (ld1), "m" (ld2)  /* Inputs from memory */
            : "st", "st(1)"
        );
        
        /* Pattern 2: Mixed constraints - "t" for x87 and "rm" for general reg/mem */
        /* This may trigger secondary reload for the integer operand */
        long double temp_ld;
        asm volatile (
            "fldt %1\n\t"           /* Load input long double */
            "fildl %2\n\t"          /* Load/converted integer to x87 stack */
            "fmulp %%st, %%st(1)\n\t" /* Multiply st(1) = st(1) * st, pop st */
            "fstpt %0"              /* Store result */
            : "=m" (temp_ld)
            : "m" (ld3), 
              "rm" (i1)             /* CRITICAL: "rm,t" alternative would be better
                                       but GCC doesn't allow "t" for integer types.
                                       Using "rm" still forces consideration of
                                       register vs memory alternatives. */
            : "st", "st(1)"
        );
        ld3 = temp_ld;
        
        /* Pattern 3: Multi-alternative constraint for x87 registers */
        /* Using "t" (top of stack) and "u" (second x87 register) constraints */
        asm volatile (
            "faddp %%st(1), %%st"
            : "=t" (ld4)            /* Output in st(0) */
            : "%0" (ld4),           /* Input in st(0) */
              "u" (ld5)             /* Input in st(1) - may need secondary reload */
            : "st(1)"
        );
        
        /* Pattern 4: Complex pattern with multiple x87 operations */
        /* This creates pressure on x87 stack positioning */
        asm volatile (
            "fldt %2\n\t"           /* ld7 -> st(0) */
            "fldt %1\n\t"           /* ld6 -> st(0), ld7 -> st(1) */
            "fmul %%st(1), %%st\n\t" /* st(0) = st(0) * st(1) */
            "fldt %3\n\t"           /* ld8 -> st(0), others shift down */
            "faddp %%st(2), %%st\n\t" /* st(2) = st(2) + st(0), pop st(0) */
            "fxch %%st(1)\n\t"      /* Exchange st(0) and st(1) */
            "fstpt %0"              /* Store st(0) to ld6 */
            : "=m" (ld6)
            : "m" (ld6), "m" (ld7), "m" (ld8)
            : "st", "st(1)", "st(2)"
        );
        
        /* Use CRC32 builtin which has fixed register constraints */
        i4 = __builtin_ia32_crc32qi(i4, i5 & 0xFF);
        
        /* More x87 operations to keep values live */
        asm volatile (
            "fldt %1\n\t"
            "fsqrt\n\t"
            "fstpt %0"
            : "=m" (ld9)
            : "m" (ld9)
            : "st"
        );
        
        /* Mix integer results back into long doubles */
        tsc2 = __builtin_ia32_rdtsc();
        ld10 = ld10 * (long double)(tsc2 - tsc1);
        
        /* Rotate values to create dependencies */
        ld11 = ld11 + ld12;
        ld12 = ld12 - ld13;
        ld13 = ld13 * ld14;
        ld14 = ld14 / ld15;
        
        i5 = i6 + i7;
        i6 = i8 ^ i9;
        i7 = i10 * i11;
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
    
    /* Compute checksum */
    checksum = (uint64_t)(ld1 * 1000) + (uint64_t)(ld2 * 1000) +
               (uint64_t)(ld3 * 1000) + (uint64_t)(ld4 * 1000) +
               (uint64_t)(ld5 * 1000) + i1 + i2 + i3 + i4;
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int seed = 42;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    uint64_t result = test_secondary_reloads(seed);
    
    printf("Result: %llu\n", (unsigned long long)result);
    
    /* Use results to prevent optimization */
    for (int i = 0; i < 15; i++) {
        if (global_results[i] > 1e100) {
            printf("Large value at %d\n", i);
        }
    }
    
    return (int)(result % 256);
}
