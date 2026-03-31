/* test_secondary_reloads.c
 * This program forces GCC to generate secondary reloads by:
 * 1. Using x87 floating-point constraints in inline assembly
 * 2. Creating high register pressure with many volatile variables
 * 3. Mixing x87 operations with fixed-register builtins
 * 4. Using multi-alternative constraints that require secondary reloads
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
    
    /* Initialize with seed-dependent values to prevent constant folding */
    ld1 = seed * 1.123L;
    ld2 = seed * 2.234L;
    ld3 = seed * 3.345L;
    ld4 = seed * 4.456L;
    ld5 = seed * 5.567L;
    ld6 = seed * 6.678L;
    ld7 = seed * 7.789L;
    ld8 = seed * 8.890L;
    ld9 = seed * 9.901L;
    ld10 = seed * 10.012L;
    ld11 = seed * 11.123L;
    ld12 = seed * 12.234L;
    ld13 = seed * 13.345L;
    ld14 = seed * 14.456L;
    ld15 = seed * 15.567L;
    
    i1 = seed * 1;
    i2 = seed * 2;
    i3 = seed * 3;
    i4 = seed * 4;
    i5 = seed * 5;
    i6 = seed * 6;
    i7 = seed * 7;
    i8 = seed * 8;
    i9 = seed * 9;
    i10 = seed * 10;
    i11 = seed * 11;
    i12 = seed * 12;
    i13 = seed * 13;
    i14 = seed * 14;
    i15 = seed * 15;
    
    /* Force use of fixed-register builtins (rdtsc uses eax/edx) */
    tsc1 = __builtin_ia32_rdtsc();
    
    /* Loop to increase register pressure and prevent optimization */
    for (loop_counter = 0; loop_counter < 3; loop_counter++) {
        /* CRITICAL: Inline asm with x87 constraints that require secondary reloads */
        
        /* 1. Simple x87 operation with "t" constraint (top of x87 stack) */
        asm volatile (
            "faddp %%st(1), %%st"
            : "=t" (ld1)
            : "0" (ld1), "t" (ld2)
            : "st(1)"
        );
        
        /* 2. Multi-alternative constraint: "rm,t" - may choose x87 alternative */
        /* This is key for triggering secondary reload initialization */
        asm volatile (
            "fildl %2\n\t"
            "faddp %%st(1), %%st"
            : "=t" (ld3)
            : "0" (ld3), "rm,t" (i3)
            : "st(1)"
        );
        
        /* 3. Another x87 operation using "u" constraint (second x87 register) */
        asm volatile (
            "fmulp %%st(1), %%st"
            : "=t" (ld4)
            : "0" (ld4), "u" (ld5)
            : "st(1)"
        );
        
        /* 4. Complex pattern: integer to x87 with memory constraint */
        /* May require secondary reload to get integer into memory */
        asm volatile (
            "fildl %2\n\t"
            "fstpt %0"
            : "=m" (global_results[0])
            : "0" (global_results[0]), "rm" (i4)
        );
        
        /* 5. Mix x87 and general registers in same asm */
        long double temp_ld;
        asm volatile (
            "fldt %1\n\t"
            "fistpl %2\n\t"
            "fld1\n\t"
            "faddp %%st(1), %%st"
            : "=t" (temp_ld), "=m" (global_ints[0])
            : "0" (ld6), "m" (global_ints[0])
            : "st(1)"
        );
        ld6 = temp_ld;
        
        /* 6. CRC32 builtin with fixed register (accumulator in eax) */
        /* Mixed with x87 operations to create complex reload requirements */
        i5 = __builtin_ia32_crc32qi(i5, i6);
        
        /* 7. Another multi-alternative with different register classes */
        asm volatile (
            "fadds %2\n\t"
            : "=t" (ld7)
            : "0" (ld7), "m,t" (ld8)
        );
        
        /* Perform some integer arithmetic to use the general registers */
        i7 = i7 * i8 + i9;
        i10 = i10 ^ i11 | i12;
        
        /* More x87 operations */
        asm volatile (
            "fsubrp %%st(1), %%st"
            : "=t" (ld9)
            : "0" (ld9), "t" (ld10)
            : "st(1)"
        );
        
        /* Convert between x87 and integer with complex constraints */
        asm volatile (
            "fistpl %1\n\t"
            "fld1\n\t"
            "faddp %%st(1), %%st"
            : "=t" (ld11), "=m" (i13)
            : "0" (ld11), "1" (i13)
            : "st(1)"
        );
    }
    
    /* Final rdtsc to use fixed registers again */
    tsc2 = __builtin_ia32_rdtsc();
    
    /* Store results to globals to prevent elimination */
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
    
    /* Compute checksum */
    checksum = (uint64_t)(ld1 + ld2 + ld3 + ld4 + ld5 + ld6 + ld7 + ld8 + ld9 + ld10 + ld11);
    checksum ^= tsc2 - tsc1;
    checksum ^= i1 ^ i2 ^ i3 ^ i4 ^ i5 ^ i6 ^ i7 ^ i8 ^ i9 ^ i10 ^ i11;
    
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
    if (global_results[0] > 100.0L) {
        printf("Some results: %Lf, %Lf\n", global_results[0], global_results[1]);
    }
    
    return (int)(result & 0x7FFFFFFF);
}
