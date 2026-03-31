/* test_secondary_reloads.c
 * Designed to trigger uncovered lines in GCC's reload.cc
 * Specifically targets initialization of secondary_in_reload, secondary_out_reload,
 * secondary_in_icode, and secondary_out_icode fields in struct reload
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[32];
volatile int global_ints[32];
volatile int result_index = 0;

/* Prevent inlining to ensure reload logic isn't optimized away */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Create high register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    volatile long double ld9, ld10, ld11, ld12, ld13, ld14, ld15, ld16;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15, i16, i17, i18, i19, i20;
    
    /* Initialize with seed to create non-constant values */
    i1 = seed + 1;
    i2 = seed * 2;
    i3 = seed ^ 0x1234;
    i4 = seed + 0x5678;
    i5 = seed * 3;
    i6 = seed / 2;
    i7 = seed << 3;
    i8 = seed >> 2;
    i9 = seed | 0xABCD;
    i10 = seed & 0xEF01;
    i11 = seed + 0x2345;
    i12 = seed * 5;
    i13 = seed ^ 0x6789;
    i14 = seed + 0x89AB;
    i15 = seed * 7;
    i16 = seed / 3;
    i17 = seed << 4;
    i18 = seed >> 3;
    i19 = seed | 0xCDEF;
    i20 = seed & 0x0123;
    
    /* Initialize long doubles using integer values */
    ld1 = (long double)i1 + 0.1;
    ld2 = (long double)i2 + 0.2;
    ld3 = (long double)i3 + 0.3;
    ld4 = (long double)i4 + 0.4;
    ld5 = (long double)i5 + 0.5;
    ld6 = (long double)i6 + 0.6;
    ld7 = (long double)i7 + 0.7;
    ld8 = (long double)i8 + 0.8;
    ld9 = (long double)i9 + 0.9;
    ld10 = (long double)i10 + 1.0;
    ld11 = (long double)i11 + 1.1;
    ld12 = (long double)i12 + 1.2;
    ld13 = (long double)i13 + 1.3;
    ld14 = (long double)i14 + 1.4;
    ld15 = (long double)i15 + 1.5;
    ld16 = (long double)i16 + 1.6;
    
    /* Use __builtin_ia32_rdtsc() which uses fixed registers (eax, edx) */
    {
        uint64_t tsc1, tsc2;
        tsc1 = __builtin_ia32_rdtsc();
        i1 = (int)(tsc1 & 0xFFFFFFFF);
        tsc2 = __builtin_ia32_rdtsc();
        i2 = (int)(tsc2 & 0xFFFFFFFF);
    }
    
    /* Force x87 operations with "t" (top of x87 stack) constraint */
    /* This should trigger secondary reloads for moving values into x87 */
    
    /* Example 1: Simple x87 addition with both operands in x87 registers */
    asm volatile (
        "faddp %%st(1), %%st"
        : "=t" (ld1)
        : "0" (ld1), "t" (ld2)
        : "st(1)"
    );
    
    /* Example 2: Multiplication with x87 constraints */
    asm volatile (
        "fmulp %%st(1), %%st"
        : "=t" (ld3)
        : "0" (ld3), "t" (ld4)
        : "st(1)"
    );
    
    /* Example 3: Mixed integer and x87 operation using "u" constraint (second x87 reg) */
    {
        long double temp = ld5;
        int int_val = i3;
        asm volatile (
            "fildl %2\n\t"
            "faddp %%st(1), %%st"
            : "=t" (ld5)
            : "0" (temp), "m" (int_val)
            : "st(1)"
        );
    }
    
    /* CRITICAL: Multi-alternative constraint that can trigger secondary reload */
    /* "rm,t" means either memory/register OR x87 top-of-stack */
    /* The compiler may choose the "t" alternative, requiring secondary reload */
    {
        long double result;
        long double input = ld6;
        int alt_input = i4;
        
        asm volatile (
            "# multi-alternative constraint test\n\t"
            "fld1\n\t"           /* Load constant 1.0 */
            "faddp %%st(1), %%st"
            : "=t" (result)
            : "0" (input), "rm,t" (alt_input)
            : "st(1)"
        );
        
        ld6 = result;
    }
    
    /* Example using CRC32 builtin which has fixed register constraints */
    {
        unsigned int crc = 0xFFFFFFFF;
        unsigned char data = (unsigned char)seed;
        crc = __builtin_ia32_crc32qi(crc, data);
        i5 = (int)crc;
    }
    
    /* Complex sequence mixing x87 and general registers */
    {
        volatile long double a = ld7;
        volatile long double b = ld8;
        volatile int c = i6;
        volatile long double d;
        
        /* First do x87 operation */
        asm volatile (
            "fsubrp %%st(1), %%st"
            : "=t" (a)
            : "0" (a), "t" (b)
            : "st(1)"
        );
        
        /* Then convert integer to long double on x87 stack */
        asm volatile (
            "fildl %1\n\t"
            : "=t" (d)
            : "m" (c)
        );
        
        /* Final x87 operation mixing values */
        asm volatile (
            "fdivp %%st(1), %%st"
            : "=t" (ld7)
            : "0" (a), "t" (d)
            : "st(1)"
        );
    }
    
    /* Loop to increase register pressure and prevent optimization */
    {
        volatile int counter;
        for (counter = 0; counter < 3; counter++) {
            long double tmp1 = ld9 + (long double)counter;
            long double tmp2 = ld10 * (long double)(counter + 1);
            
            asm volatile (
                "faddp %%st(1), %%st"
                : "=t" (ld9)
                : "0" (tmp1), "t" (tmp2)
                : "st(1)"
            );
            
            /* Use the integer in a way that might need secondary reload */
            int int_tmp = i7 + counter;
            asm volatile (
                "# using integer value\n\t"
                : 
                : "rm,t" (int_tmp)
                : "cc"
            );
        }
    }
    
    /* Store results to global arrays to prevent elimination */
    global_results[result_index++] = ld1;
    global_results[result_index++] = ld2;
    global_results[result_index++] = ld3;
    global_results[result_index++] = ld4;
    global_results[result_index++] = ld5;
    global_results[result_index++] = ld6;
    global_results[result_index++] = ld7;
    global_results[result_index++] = ld8;
    global_results[result_index++] = ld9;
    
    global_ints[0] = i1;
    global_ints[1] = i2;
    global_ints[2] = i3;
    global_ints[3] = i4;
    global_ints[4] = i5;
    global_ints[5] = i6;
    global_ints[6] = i7;
    global_ints[7] = i8;
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
    
    /* Compute a simple checksum to use results */
    int checksum = 0;
    for (int i = 0; i < result_index && i < 32; i++) {
        checksum += (int)global_results[i];
    }
    for (int i = 0; i < 8; i++) {
        checksum += global_ints[i];
    }
    
    printf("Result checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
