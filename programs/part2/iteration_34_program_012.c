/* test_secondary_reloads.c
 * Designed to trigger secondary reload initialization in GCC's reload pass
 * Compile with: gcc -O1 -m32 -mfpmath=387 -march=i686 -fno-omit-frame-pointer
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[32];
volatile int global_ints[32];
int global_index = 0;

/* Prevent inlining to ensure reload happens at call boundaries */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* High register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    volatile long double ld9, ld10, ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    
    /* Initialize with seed-dependent values to avoid constant folding */
    i1 = seed * 1;
    i2 = seed * 2 + 1;
    i3 = seed * 3 + 2;
    i4 = seed * 4 + 3;
    i5 = seed * 5 + 4;
    
    /* Use rdtsc builtin which uses fixed registers (eax, edx) */
    {
        unsigned long long tsc;
        tsc = __builtin_ia32_rdtsc();
        i6 = (int)(tsc & 0xFFFFFFFF);
        i7 = (int)(tsc >> 32);
    }
    
    /* Initialize long double variables */
    ld1 = (long double)i1 / 3.14159265358979323846L;
    ld2 = (long double)i2 / 2.71828182845904523536L;
    ld3 = (long double)i3 / 1.41421356237309504880L;
    ld4 = (long double)i4;
    ld5 = (long double)i5;
    
    /* Force x87 operations with explicit st(0), st(1) constraints */
    
    /* Pattern 1: Simple x87 operation with "t" constraint */
    asm volatile (
        "faddp %%st(1), %%st"
        : "=t" (ld6)
        : "0" (ld1), "t" (ld2)
        : "st(1)"
    );
    
    /* Pattern 2: Mixed x87 and integer with multi-alternative constraint
     * This is key for triggering secondary reloads */
    asm volatile (
        "fildl %2\n\t"
        "faddp %%st(1), %%st"
        : "=t" (ld7)
        : "0" (ld3), "rm,t" (i6)  /* "rm,t" alternative may force secondary reload */
        : "st(1)"
    );
    
    /* Pattern 3: More complex x87 operation chain */
    ld8 = ld4;
    asm volatile (
        "fmulp %%st(1), %%st\n\t"
        "fsin"
        : "=t" (ld9)
        : "0" (ld8), "u" (ld5)  /* "u" = st(1) constraint */
        : "st(1)"
    );
    
    /* Pattern 4: CRC32 builtin (uses fixed register eax) mixed with x87 */
    i8 = __builtin_ia32_crc32qi(i7, (unsigned char)seed);
    
    /* Convert integer to long double using x87 */
    asm volatile (
        "fildl %1\n\t"
        : "=t" (ld10)
        : "rm" (i8)
    );
    
    /* Pattern 5: Another multi-alternative with memory operand */
    ld11 = ld6 + ld7;
    asm volatile (
        "fadds %2\n\t"
        : "=t" (ld12)
        : "0" (ld11), "m" (ld9)
    );
    
    /* Create register pressure with more operations */
    i9 = i1 + i2 + i3;
    i10 = i4 * i5 - i6;
    
    /* Pattern 6: Division using x87 - division often needs specific handling */
    asm volatile (
        "fdivrp %%st(1), %%st"
        : "=t" (ld13)
        : "0" (ld10), "u" (ld12)
        : "st(1)"
    );
    
    /* Use the results to prevent optimization */
    global_results[global_index++] = ld6;
    global_results[global_index++] = ld7;
    global_results[global_index++] = ld9;
    global_results[global_index++] = ld10;
    global_results[global_index++] = ld12;
    global_results[global_index++] = ld13;
    
    global_ints[0] = i8;
    global_ints[1] = i9;
    global_ints[2] = i10;
    
    /* Loop to increase live range and register pressure */
    volatile int loop_counter;
    for (loop_counter = 0; loop_counter < 3; loop_counter++) {
        /* Mix integer and float operations in loop */
        i11 = i9 + loop_counter;
        asm volatile (
            "fildl %1\n\t"
            "faddp %%st(1), %%st"
            : "+t" (ld14)
            : "rm" (i11)
            : "st(1)"
        );
    }
    
    /* Final store */
    global_results[global_index++] = ld14;
}

/* Secondary test with different patterns */
__attribute__((noinline, noipa))
void test_mmx_secondary(int seed) {
    /* Test MMX secondary reloads (64-bit integer vectors) */
    volatile long long mm1, mm2, mm3;
    volatile int i1 = seed;
    
    /* Initialize */
    mm1 = (long long)i1 * 0x0102030405060708LL;
    mm2 = (long long)(i1 + 1) * 0x0807060504030201LL;
    
    /* MMX builtin - may trigger MMX register constraints */
    #ifdef __MMX__
    asm volatile (
        "paddq %1, %0"
        : "+x" (mm3)
        : "xm" (mm1)
    );
    #endif
    
    global_ints[10] = (int)mm3;
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call multiple times with different seeds */
    test_secondary_reloads(seed);
    test_secondary_reloads(seed + 1);
    test_mmx_secondary(seed + 2);
    
    /* Compute checksum to use results */
    int checksum = 0;
    for (int i = 0; i < global_index && i < 32; i++) {
        checksum += (int)global_results[i];
    }
    for (int i = 0; i < 32; i++) {
        checksum += global_ints[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
