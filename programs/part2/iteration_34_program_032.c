/* test_secondary_reloads.c
 * Designed to trigger secondary reloads in GCC's reload pass
 * Compile with: gcc -O1 -m32 -mfpmath=387 -march=i686 -fno-omit-frame-pointer -o test test_secondary_reloads.c
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[20];
volatile int global_ints[20];

/* Prevent inlining to ensure complex reload scenarios aren't optimized away */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Declare many volatile variables to create register pressure */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8, ld9, ld10;
    volatile long double ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    volatile unsigned int cycles_low, cycles_high;
    
    /* Initialize with seed-dependent values to prevent constant folding */
    i1 = seed + 1;
    i2 = seed * 2 + 3;
    i3 = seed ^ 0x1234;
    i4 = seed * seed;
    i5 = seed + 100;
    i6 = seed - 50;
    i7 = seed * 3;
    i8 = seed / 2;
    i9 = seed << 3;
    i10 = seed >> 2;
    i11 = seed | 0xFF00;
    i12 = seed & 0x0F0F;
    i13 = ~seed;
    i14 = seed + 0xABCD;
    i15 = seed * 0x123;
    
    /* Initialize long double variables with conversions from integers */
    ld1 = (long double)i1 * 1.1L;
    ld2 = (long double)i2 * 2.2L;
    ld3 = (long double)i3 * 3.3L;
    ld4 = (long double)i4 * 4.4L;
    ld5 = (long double)i5 * 5.5L;
    ld6 = (long double)i6 * 6.6L;
    ld7 = (long double)i7 * 7.7L;
    ld8 = (long double)i8 * 8.8L;
    ld9 = (long double)i9 * 9.9L;
    ld10 = (long double)i10 * 10.10L;
    ld11 = (long double)i11 * 11.11L;
    ld12 = (long double)i12 * 12.12L;
    ld13 = (long double)i13 * 13.13L;
    ld14 = (long double)i14 * 14.14L;
    ld15 = (long double)i15 * 15.15L;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    __asm__ volatile ("rdtsc" : "=a" (cycles_low), "=d" (cycles_high));
    
    /* Mix the RDTSC result into our calculations */
    i1 += (int)cycles_low;
    i2 += (int)cycles_high;
    
    /* Force x87 operations with explicit register constraints */
    /* This asm uses "t" constraint (top of x87 stack) which may require secondary reloads */
    __asm__ volatile (
        "fldt %2\n\t"           /* Load ld2 onto x87 stack */
        "fldt %1\n\t"           /* Load ld1 onto x87 stack */
        "faddp %%st, %%st(1)\n\t" /* Add st(0) to st(1), pop stack */
        "fstpt %0"              /* Store result */
        : "=m" (ld1)
        : "m" (ld1), "m" (ld2)
        : "st", "st(1)"
    );
    
    /* Another x87 operation with different constraints */
    __asm__ volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "fmulp %%st, %%st(1)\n\t"
        "fstpt %0"
        : "=m" (ld3)
        : "m" (ld3), "m" (ld4)
        : "st", "st(1)"
    );
    
    /* CRITICAL: Inline asm with multi-alternative constraint that includes x87 constraint
     * This is designed to trigger secondary reload setup */
    {
        volatile long double ld_result;
        volatile int int_input = i5;
        
        /* "rm,t" constraint: either memory/general register OR x87 top-of-stack
         * The compiler may choose the "t" alternative, requiring secondary reload
         * for the integer value */
        __asm__ volatile (
            "fildl %2\n\t"      /* Convert integer to long double on x87 stack */
            "fldt %1\n\t"       /* Load ld5 onto stack */
            "faddp %%st, %%st(1)\n\t"
            "fstpt %0"
            : "=m" (ld_result)
            : "m" (ld5), "rm,t" (int_input)
            : "st", "st(1)"
        );
        
        ld5 = ld_result;
    }
    
    /* More complex mixing: x87 operation with result used in subsequent asm */
    {
        volatile long double temp;
        __asm__ volatile (
            "fldt %1\n\t"
            "fsqrt\n\t"
            "fstpt %0"
            : "=m" (temp)
            : "m" (ld6)
            : "st"
        );
        ld6 = temp;
    }
    
    /* Use CRC32 builtin which has fixed register constraints */
    {
        unsigned int crc = 0xFFFFFFFF;
        crc = __builtin_ia32_crc32qi(crc, (unsigned char)i7);
        i8 += (int)crc;
    }
    
    /* Another multi-alternative constraint example with x87 and general register */
    {
        volatile long double ld_tmp = ld7;
        volatile int int_val = i9;
        
        __asm__ volatile (
            "fildl %2\n\t"      /* Integer to x87 */
            "fldt %1\n\t"       /* Load long double */
            "fsubrp %%st, %%st(1)\n\t"
            "fstpt %0"
            : "=m" (ld_tmp)
            : "m" (ld7), "rm,t" (int_val)
            : "st", "st(1)"
        );
        
        ld7 = ld_tmp;
    }
    
    /* Chain operations to create complex dependency graph */
    for (volatile int loop = 0; loop < 3; loop++) {
        /* Mix integer and floating point */
        i10 += loop;
        ld8 += (long double)loop * 0.5L;
        
        /* Another x87 operation with "u" constraint (second x87 register) */
        __asm__ volatile (
            "fldt %1\n\t"
            "fldt %2\n\t"
            "fdivrp %%st, %%st(1)\n\t"
            "fstpt %0"
            : "=m" (ld9)
            : "m" (ld9), "m" (ld10)
            : "st", "st(1)"
        );
        
        /* Use the result in another operation */
        ld10 = ld9 * 2.0L;
    }
    
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
    
    printf("Testing secondary reloads with seed = %d\n", seed);
    
    /* Call multiple times with different seeds to explore different paths */
    for (int i = 0; i < 5; i++) {
        test_secondary_reloads(seed + i);
    }
    
    /* Compute a simple checksum from results */
    int checksum = 0;
    for (int i = 0; i < 15; i++) {
        checksum += global_ints[i];
        checksum += (int)global_results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
