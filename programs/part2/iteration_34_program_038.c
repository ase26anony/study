/* test_secondary_reloads.c
 * Designed to trigger GCC's secondary reload initialization in reload.cc
 * Compile with: gcc -O1 -m32 -mfpmath=387 -march=i686 -fno-omit-frame-pointer test_secondary_reloads.c -o test_reloads
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[32];
volatile int global_ints[32];
int global_index = 0;

/* Prevent inlining to ensure reload logic isn't optimized away */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Create high register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    volatile long double ld9, ld10, ld11, ld12, ld13, ld14, ld15, ld16;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8;
    volatile int i9, i10, i11, i12, i13, i14, i15, i16;
    
    /* Initialize with seed-dependent values to avoid constant propagation */
    i1 = seed + 1;
    i2 = seed * 2;
    i3 = seed ^ 0x1234;
    i4 = seed - 100;
    i5 = seed * seed;
    i6 = seed + 0x5678;
    i7 = seed | 0xABCD;
    i8 = seed & 0xF0F0;
    i9 = seed << 3;
    i10 = seed >> 2;
    i11 = ~seed;
    i12 = seed + i1;
    i13 = seed * i2;
    i14 = seed ^ i3;
    i15 = seed - i4;
    i16 = seed + i5;
    
    /* Initialize long doubles with conversions from integers */
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
    ld16 = (long double)i16 * 16.16L;
    
    /* Force use of rdtsc which uses fixed registers (eax, edx) */
    {
        uint32_t lo, hi;
        __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
        i1 = lo;
        i2 = hi;
    }
    
    volatile int loop_counter;
    for (loop_counter = 0; loop_counter < 3; loop_counter++) {
        /* Pattern 1: x87 operations with explicit st(0), st(1) constraints */
        __asm__ volatile (
            "fldt %[in1]\n\t"
            "fldt %[in2]\n\t"
            "faddp %%st(1), %%st\n\t"
            "fstpt %[out]"
            : [out] "=m" (ld1)
            : [in1] "m" (ld2), [in2] "m" (ld3)
            : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
        );
        
        /* Pattern 2: Mixed constraints with "t" (x87 top) and "rm" alternatives */
        /* This is key for triggering secondary reload initialization */
        long double temp_ld = ld4;
        int temp_int = i4;
        
        __asm__ volatile (
            "fldt %[ld_in]\n\t"
            "fildl %[int_in]\n\t"
            "fmulp\n\t"
            "fstpt %[ld_out]"
            : [ld_out] "=m" (ld4)
            : [ld_in] "m" (temp_ld),
              [int_in] "rm,t" (temp_int)  /* Multi-alternative constraint */
            : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
        );
        
        /* Pattern 3: Output constraint "=t" (must be in x87 top register) */
        long double result;
        __asm__ volatile (
            "fldt %1\n\t"
            "fldt %2\n\t"
            "fmulp %%st, %%st(1)\n\t"
            : "=t" (result)
            : "m" (ld5), "m" (ld6)
        );
        ld5 = result;
        
        /* Pattern 4: Input constraint "u" (x87 second register) */
        __asm__ volatile (
            "fldt %[op1]\n\t"
            "fldt %[op2]\n\t"
            "faddp\n\t"
            "fstpt %[result]"
            : [result] "=m" (ld7)
            : [op1] "m" (ld8), [op2] "u" (ld9)
            : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
        );
        
        /* Pattern 5: Complex pattern with multiple x87 operations */
        __asm__ volatile (
            "fldt %1\n\t"
            "fldt %2\n\t"
            "faddp %%st, %%st(1)\n\t"
            "fldt %3\n\t"
            "fmulp %%st, %%st(1)\n\t"
            "fstpt %0"
            : "=m" (ld10)
            : "m" (ld11), "m" (ld12), "m" (ld13)
            : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
        );
        
        /* Use CRC32 builtin which has fixed register constraints */
        i3 = __builtin_ia32_crc32qi(i3, i4 & 0xFF);
        
        /* More arithmetic to increase register pressure */
        ld14 = ld14 * ld15 + ld16;
        ld15 = ld15 / ld14 - ld13;
        ld16 = ld16 + ld14 * ld15;
        
        i5 = i5 + i6 * i7;
        i6 = i6 - i8 / (i9 + 1);
        i7 = i7 ^ i10 & i11;
        
        /* Store intermediate results to prevent optimization */
        global_results[global_index % 32] = ld1 + ld4 + ld5 + ld7 + ld10;
        global_ints[global_index % 32] = i3 + i5 + i6 + i7;
        global_index++;
    }
    
    /* Final mixing to produce a checksum */
    volatile int checksum = 0;
    checksum += (int)ld1 + (int)ld4 + (int)ld5;
    checksum += i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8;
    
    global_results[31] = checksum;
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call multiple times with different seeds */
    test_secondary_reloads(seed);
    test_secondary_reloads(seed + 1);
    test_secondary_reloads(seed + 2);
    
    /* Print something to prevent complete optimization */
    printf("Result: %Lf\n", global_results[31]);
    
    return 0;
}
