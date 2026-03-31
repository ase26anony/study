/* test_secondary_reloads.c
 * This program forces GCC to generate secondary reloads by:
 * 1. Using x87 floating-point operations with 't' and 'u' constraints
 * 2. Creating high register pressure with many volatile variables
 * 3. Using inline assembly with multiple alternative constraints
 * 4. Mixing x87 registers with fixed-register builtins
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

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
    i2 = seed * 2 + 3;
    i3 = seed ^ 0x1234;
    i4 = seed << 3;
    i5 = seed >> 2;
    i6 = seed * seed;
    i7 = seed + 0xABCD;
    i8 = seed | 0xFF00;
    i9 = seed & 0x00FF;
    i10 = ~seed;
    i11 = seed * 3 + 7;
    i12 = seed / 2;
    i13 = seed % 100;
    i14 = seed + 0xDEAD;
    i15 = seed * 0xBEEF;
    
    /* Convert integers to long double for x87 operations */
    ld1 = (long double)i1;
    ld2 = (long double)i2;
    ld3 = (long double)i3;
    ld4 = (long double)i4;
    ld5 = (long double)i5;
    ld6 = (long double)i6;
    ld7 = (long double)i7;
    ld8 = (long double)i8;
    ld9 = (long double)i9;
    ld10 = (long double)i10;
    ld11 = (long double)i11;
    ld12 = (long double)i12;
    ld13 = (long double)i13;
    ld14 = (long double)i14;
    ld15 = (long double)i15;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    {
        uint64_t tsc1, tsc2;
        tsc1 = __builtin_ia32_rdtsc();
        /* Do some work to prevent optimization */
        i1 = i1 + (tsc1 & 0xFF);
        tsc2 = __builtin_ia32_rdtsc();
        i2 = i2 + ((tsc2 - tsc1) & 0xFF);
    }
    
    /* CRITICAL: Inline assembly with x87 constraints that require secondary reloads */
    
    /* 1. Simple x87 operation with 't' constraint (top of x87 stack) */
    asm volatile (
        "faddp %%st(1), %%st"
        : "=t" (ld1)
        : "0" (ld1), "t" (ld2)
        : "st(1)"
    );
    
    /* 2. More complex: fmul with 'u' constraint (second x87 register) */
    asm volatile (
        "fmulp %%st(1), %%st"
        : "=t" (ld3)
        : "0" (ld3), "u" (ld4)
        : "st(1)"
    );
    
    /* 3. CRITICAL FOR COVERAGE: Multi-alternative constraint that may force secondary reload */
    /* The "rm,t" constraint gives alternatives: memory/general register OR x87 top */
    /* When the integer needs to go to x87, it requires a secondary reload */
    {
        volatile int temp_int = i5;
        asm volatile (
            "# Multi-alternative constraint test\n\t"
            "fildl %2\n\t"
            "faddp %%st(1), %%st"
            : "=t" (ld5)
            : "0" (ld5), "rm,t" (temp_int)
            : "st(1)"
        );
    }
    
    /* 4. Another multi-alternative with mixing register classes */
    {
        volatile long double temp_ld = ld6;
        volatile int temp_int2 = i6;
        asm volatile (
            "# Mixed constraints\n\t"
            "faddp %%st(1), %%st"
            : "=t" (ld6)
            : "0" (temp_ld), "rm,t" (temp_int2)
            : "st(1)"
        );
    }
    
    /* 5. Chain x87 operations to increase register pressure */
    asm volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "faddp %%st(1), %%st\n\t"
        "fstpt %0"
        : "=m" (ld7)
        : "m" (ld8), "m" (ld9)
        : "st", "st(1)"
    );
    
    /* 6. Use CRC32 builtin which has fixed register constraints */
    {
        unsigned int crc = 0xFFFFFFFF;
        crc = __builtin_ia32_crc32qi(crc, (unsigned char)i7);
        i8 = crc ^ i8;
    }
    
    /* 7. More x87 operations in a loop to create multiple reload opportunities */
    {
        volatile int loop_counter;
        for (loop_counter = 0; loop_counter < 3; loop_counter++) {
            asm volatile (
                "fldt %1\n\t"
                "fldt %2\n\t"
                "fmulp %%st(1), %%st\n\t"
                "fstpt %0"
                : "=m" (ld10)
                : "m" (ld11), "m" (ld12)
                : "st", "st(1)"
            );
            
            /* Mix with integer operations */
            i9 = i9 + loop_counter;
        }
    }
    
    /* 8. Complex pattern: x87 operation with memory output and x87 input */
    {
        volatile long double result;
        asm volatile (
            "fldt %1\n\t"
            "fsin\n\t"
            "fstpt %0"
            : "=m" (result)
            : "m" (ld13)
            : "st"
        );
        ld13 = result;
    }
    
    /* 9. Division operation that might use fixed registers */
    {
        volatile int divisor = i10;
        if (divisor != 0) {
            i11 = i12 / divisor;
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
    global_ints[7] = i8;
    global_ints[8] = i9;
    global_ints[9] = i10;
    global_ints[10] = i11;
    global_ints[11] = i12;
    global_ints[12] = i13;
    global_ints[13] = i14;
    global_ints[14] = i15;
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
    
    /* Compute a simple checksum to use results */
    int checksum = 0;
    for (int i = 0; i < 15; i++) {
        checksum += global_ints[i];
        checksum ^= (int)global_results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
