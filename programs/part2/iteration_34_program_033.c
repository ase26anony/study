/* test_secondary_reloads.c
 * Designed to trigger secondary reload initialization in GCC's reload pass
 * Compile with: gcc -O1 -m32 -mfpmath=387 -march=i686 -fno-omit-frame-pointer -c test_secondary_reloads.c
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[32];
volatile int global_ints[32];
int global_index = 0;

/* Prevent inlining to ensure complex reload patterns aren't optimized away */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* High register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    volatile long double ld9, ld10, ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15, i16, i17, i18, i19, i20;
    
    /* Initialize with seed to create non-constant values */
    i1 = seed;
    i2 = seed * 2 + 1;
    i3 = seed * 3 + 2;
    i4 = seed * 5 + 3;
    i5 = seed * 7 + 4;
    
    /* Use rdtsc builtin which uses fixed registers (eax, edx) */
    {
        unsigned long long tsc;
        tsc = __builtin_ia32_rdtsc();
        i6 = (int)(tsc & 0xFFFFFFFF);
        i7 = (int)(tsc >> 32);
    }
    
    /* Initialize long double variables */
    ld1 = (long double)i1 / 3.0L;
    ld2 = (long double)i2 / 5.0L;
    ld3 = (long double)i3 / 7.0L;
    ld4 = (long double)i4 / 11.0L;
    ld5 = (long double)i5 / 13.0L;
    
    /* More integer variables to increase register pressure */
    i8 = i1 + i2;
    i9 = i3 * i4;
    i10 = i5 ^ i6;
    i11 = i7 & i8;
    i12 = i9 | i10;
    i13 = i11 - i12;
    i14 = i13 * 2;
    i15 = i14 / 3;
    i16 = i15 + seed;
    i17 = i16 * 17;
    i18 = i17 % 19;
    i19 = i18 << 2;
    i20 = i19 >> 1;
    
    /* More long double variables */
    ld6 = ld1 + ld2;
    ld7 = ld3 * ld4;
    ld8 = ld5 - ld6;
    ld9 = ld7 / ld8;
    ld10 = ld9 * 2.0L;
    ld11 = ld10 + 1.0L;
    ld12 = ld11 - 3.0L;
    ld13 = ld12 * ld1;
    ld14 = ld13 / ld2;
    ld15 = ld14 + ld3;
    
    /* CRITICAL SECTION: Inline assembly with x87 constraints
     * This should trigger secondary reloads */
    
    /* 1. Simple x87 operation with "t" constraint (top of x87 stack) */
    asm volatile ("faddp %%st(1), %%st" 
                  : "=t" (ld1) 
                  : "0" (ld1), "t" (ld2)
                  : "st(1)");
    
    /* 2. Mixed integer and x87 operation with multi-alternative constraint
     * The "rm,t" constraint may force secondary reload for integer */
    {
        long double result;
        int int_val = i1;
        long double ld_val = ld3;
        
        asm volatile ("# Multi-alternative constraint test\n\t"
                      "fildl %2\n\t"
                      "faddp %%st(1), %%st"
                      : "=t" (result)
                      : "0" (ld_val), "rm,t" (int_val)
                      : "st(1)");
        ld4 = result;
    }
    
    /* 3. Another x87 operation using "u" constraint (second x87 register) */
    {
        long double temp1 = ld5;
        long double temp2 = ld6;
        
        asm volatile ("fxch %%st(1)\n\t"
                      "fadds %2\n\t"
                      "fxch %%st(1)"
                      : "=u" (temp1), "=t" (temp2)
                      : "0" (temp1), "1" (temp2), "m" (i2)
                      : "st(2)");
        ld7 = temp1;
        ld8 = temp2;
    }
    
    /* 4. Complex pattern with multiple outputs and x87 constraints */
    {
        long double out1, out2;
        long double in1 = ld9;
        long double in2 = ld10;
        int int_in = i3;
        
        asm volatile ("# Complex x87 pattern\n\t"
                      "fldt %3\n\t"           /* Load int_in to x87 stack */
                      "fxch %%st(2)\n\t"      /* Exchange st(0) and st(2) */
                      "faddp %%st(1), %%st\n\t"
                      "fxch %%st(1)"
                      : "=t" (out1), "=u" (out2)
                      : "0" (in1), "rm,t" (int_in), "1" (in2)
                      : "st(2)", "st(3)");
        ld11 = out1;
        ld12 = out2;
    }
    
    /* 5. Use CRC32 builtin which has fixed register constraints */
    {
        unsigned int crc = 0xFFFFFFFF;
        unsigned char data = (unsigned char)i4;
        crc = __builtin_ia32_crc32qi(crc, data);
        i5 = (int)crc;
    }
    
    /* 6. More x87 operations in a loop to increase reload opportunities */
    {
        volatile int counter;
        long double accum = 1.0L;
        
        for (counter = 0; counter < 3; counter++) {
            long double operand = ld13 + (long double)counter;
            
            asm volatile ("fld1\n\t"
                          "faddp %%st(1), %%st"
                          : "+t" (accum)
                          : "0" (accum)
                          : "st(1)");
                          
            asm volatile ("fmulp %%st(1), %%st"
                          : "+t" (accum)
                          : "0" (accum), "t" (operand)
                          : "st(1)");
        }
        ld14 = accum;
    }
    
    /* 7. Final complex asm with memory and register constraints */
    {
        int mem_var = i6;
        long double x87_var = ld15;
        long double result;
        
        /* This asm has both memory and x87 constraints, potentially
         * requiring secondary reloads for spilling */
        asm volatile ("# Mixed memory/x87 constraints\n\t"
                      "fildl %1\n\t"
                      "faddp %%st(1), %%st\n\t"
                      "fstpt %0"
                      : "=m" (result)
                      : "m" (mem_var), "t" (x87_var)
                      : "st(1)");
        ld15 = result;
    }
    
    /* Store results to global arrays to prevent elimination */
    global_results[global_index++] = ld1;
    global_results[global_index++] = ld2;
    global_results[global_index++] = ld3;
    global_results[global_index++] = ld4;
    global_results[global_index++] = ld5;
    global_results[global_index++] = ld6;
    global_results[global_index++] = ld7;
    global_results[global_index++] = ld8;
    global_results[global_index++] = ld9;
    global_results[global_index++] = ld10;
    global_results[global_index++] = ld11;
    global_results[global_index++] = ld12;
    global_results[global_index++] = ld13;
    global_results[global_index++] = ld14;
    global_results[global_index++] = ld15;
    
    global_ints[0] = i1; global_ints[1] = i2; global_ints[2] = i3;
    global_ints[3] = i4; global_ints[4] = i5; global_ints[5] = i6;
    global_ints[6] = i7; global_ints[7] = i8; global_ints[8] = i9;
    global_ints[9] = i10; global_ints[10] = i11; global_ints[11] = i12;
    global_ints[12] = i13; global_ints[13] = i14; global_ints[14] = i15;
    global_ints[15] = i16; global_ints[16] = i17; global_ints[17] = i18;
    global_ints[18] = i19; global_ints[19] = i20;
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
    for (int i = 0; i < 20; i++) {
        checksum ^= global_ints[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}
