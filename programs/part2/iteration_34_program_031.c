/* test_secondary_reloads.c
 * Designed to trigger secondary reload initialization in GCC's reload pass
 * Compile with: gcc -O1 -m32 -mfpmath=387 -march=i686 -fno-omit-frame-pointer test_secondary_reloads.c -o test
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[32];
volatile int global_ints[32];
int global_index = 0;

/* Prevent inlining to ensure reload pass runs on this function */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Declare many volatile variables to create register pressure */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    volatile long double ld9, ld10, ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    volatile unsigned int lo, hi;
    
    /* Initialize with seed to create non-constant values */
    i1 = seed + 1;
    i2 = seed * 2 + 3;
    i3 = seed ^ 0x1234;
    i4 = seed * seed;
    i5 = seed + 1000;
    
    /* Use rdtsc builtin which uses fixed registers (eax, edx) */
    __asm__ volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    i6 = lo ^ hi;
    i7 = (lo * hi) & 0xFFFF;
    
    /* Initialize long double values using integer conversions */
    ld1 = (long double)i1 * 1.1L;
    ld2 = (long double)i2 * 2.2L;
    ld3 = (long double)i3 * 3.3L;
    ld4 = (long double)i4 * 4.4L;
    ld5 = (long double)i5 * 5.5L;
    
    /* Force x87 register usage with explicit asm and "t" constraint */
    /* This should trigger secondary reload setup */
    
    /* Example 1: Simple x87 operation with "t" constraint */
    __asm__ volatile (
        "faddp %%st(1), %%st"
        : "=t" (ld6)
        : "0" (ld1), "t" (ld2)
        : "st(1)"
    );
    
    /* More x87 operations to increase pressure */
    __asm__ volatile (
        "fmulp %%st(1), %%st"
        : "=t" (ld7)
        : "0" (ld3), "t" (ld4)
        : "st(1)"
    );
    
    /* Use "u" constraint (second x87 register) */
    __asm__ volatile (
        "fsubrp %%st(1), %%st"
        : "=t" (ld8)
        : "0" (ld5), "u" (ld6)
        : "st(1)"
    );
    
    /* CRITICAL: Multi-alternative constraint that may need secondary reload */
    /* "rm,t" means either memory/register OR x87 top-of-stack */
    /* The compiler may choose "t" alternative, requiring secondary reload */
    i8 = i6 + i7;
    __asm__ volatile (
        "# multi-alternative constraint"
        : "=t" (ld9)
        : "0" (ld7), "rm,t" (i8)
    );
    
    /* Another complex case mixing register classes */
    i9 = i1 * i2;
    __asm__ volatile (
        "# mixed constraints"
        : "=t" (ld10), "=a" (i10)
        : "0" (ld8), "t" (ld9), "a" (i9), "c" (i3)
        : "edx"
    );
    
    /* Use CRC32 builtin which has fixed register constraints */
    i11 = __builtin_ia32_crc32qi(i4, (unsigned char)i5);
    
    /* More operations to keep values live */
    ld11 = ld9 + ld10;
    ld12 = ld11 * ld7;
    
    /* Force integer to x87 conversion with potential secondary reload */
    i12 = i11 ^ i10;
    __asm__ volatile (
        "fildl %1\n\t"
        "fstpt %0"
        : "=m" (ld13)
        : "m" (i12)
        : "st"
    );
    
    /* Complex sequence with multiple outputs */
    i13 = i6 - i7;
    i14 = i8 * i9;
    __asm__ volatile (
        "# complex multi-output"
        : "=t" (ld14), "=r" (i15)
        : "0" (ld12), "rm" (i13), "r" (i14)
        : "st(1)"
    );
    
    /* Final mixing */
    ld15 = ld13 + ld14;
    
    /* Store results to globals to prevent elimination */
    global_results[global_index & 31] = ld15;
    global_ints[global_index & 31] = i15;
    global_index++;
    
    /* Use volatile counter to prevent loop optimization */
    volatile int counter;
    for (counter = 0; counter < 3; counter++) {
        /* More x87 operations in loop */
        __asm__ volatile (
            "faddp %%st(1), %%st"
            : "=t" (ld15)
            : "0" (ld15), "t" (ld14)
            : "st(1)"
        );
        global_results[(global_index + counter) & 31] = ld15;
    }
}

/* Secondary test function with different pattern */
__attribute__((noinline, noipa))
void test_mmx_secondary(int seed) {
    /* Try to trigger MMX secondary reloads */
    volatile long long mm1, mm2, mm3;
    volatile int i1, i2;
    
    i1 = seed * 3;
    i2 = seed * 7;
    
    /* MMX-style operations (though GCC may use SSE instead) */
    mm1 = ((long long)i1 << 32) | i2;
    mm2 = ((long long)i2 << 32) | i1;
    
    /* Inline asm that might use MMX registers */
    __asm__ volatile (
        "# potential MMX operation\n\t"
        "movq %1, %%mm0\n\t"
        "paddd %2, %%mm0\n\t"
        "movq %%mm0, %0\n\t"
        "emms"
        : "=m" (mm3)
        : "m" (mm1), "m" (mm2)
        : "mm0"
    );
    
    global_ints[global_index & 31] = (int)(mm3 >> 32);
    global_index++;
}

int main(int argc, char *argv[]) {
    int seed = 12345;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call test functions multiple times with different seeds */
    test_secondary_reloads(seed);
    test_secondary_reloads(seed + 1);
    test_secondary_reloads(seed + 2);
    test_mmx_secondary(seed + 3);
    test_mmx_secondary(seed + 4);
    
    /* Compute checksum of results */
    int checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += global_ints[i];
        checksum ^= (int)global_results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}
