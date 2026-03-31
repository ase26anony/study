/* test_secondary_reloads.c */
#include <stdint.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[20];
volatile int global_ints[20];
int global_index = 0;

/* Prevent inlining to ensure complex reload decisions remain */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Create high register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8, ld9, ld10;
    volatile long double ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    
    /* Initialize with seed-dependent values to avoid constant propagation */
    i1 = seed + 1;
    i2 = seed * 2 + 3;
    i3 = seed ^ 0x1234;
    i4 = seed * seed;
    i5 = seed + 1000;
    i6 = seed - 500;
    i7 = seed | 0xFF00;
    i8 = seed & 0x0F0F;
    i9 = seed << 3;
    i10 = seed >> 2;
    i11 = ~seed;
    i12 = seed + i1;
    i13 = seed * i2;
    i14 = seed ^ i3;
    i15 = seed + i4;
    
    /* Initialize long doubles using integer values */
    ld1 = (long double)i1 + 0.5L;
    ld2 = (long double)i2 + 1.5L;
    ld3 = (long double)i3 + 2.5L;
    ld4 = (long double)i4 + 3.5L;
    ld5 = (long double)i5 + 4.5L;
    ld6 = (long double)i6 + 5.5L;
    ld7 = (long double)i7 + 6.5L;
    ld8 = (long double)i8 + 7.5L;
    ld9 = (long double)i9 + 8.5L;
    ld10 = (long double)i10 + 9.5L;
    ld11 = (long double)i11 + 10.5L;
    ld12 = (long double)i12 + 11.5L;
    ld13 = (long double)i13 + 12.5L;
    ld14 = (long double)i14 + 13.5L;
    ld15 = (long double)i15 + 14.5L;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    {
        uint64_t tsc1, tsc2;
        tsc1 = __builtin_ia32_rdtsc();
        i1 = (int)(tsc1 & 0xFFFFFFFF);
        tsc2 = __builtin_ia32_rdtsc();
        i2 = (int)(tsc2 & 0xFFFFFFFF);
    }
    
    /* Force x87 operations with explicit register constraints */
    
    /* 1. Simple x87 operation with "t" constraint (top of x87 stack) */
    asm volatile ("faddp %%st(1), %%st" 
                  : "=t" (ld1) 
                  : "0" (ld1), "t" (ld2));
    
    /* 2. Mixed operation: x87 with general register input */
    /* This may trigger secondary reload for the integer operand */
    asm volatile ("fildl %1\n\t"
                  "faddp %%st(1), %%st"
                  : "=t" (ld3)
                  : "t" (ld3), "m" (i3));
    
    /* 3. Multi-alternative constraint: "rm,t" - may choose x87 alternative */
    /* This is key for triggering secondary reload initialization */
    asm volatile ("faddp %%st(1), %%st"
                  : "=t" (ld4)
                  : "0" (ld4), "rm,t" (i4));
    
    /* 4. Complex chain of x87 operations */
    asm volatile ("fldt %1\n\t"
                  "fldt %2\n\t"
                  "fmulp %%st(1), %%st\n\t"
                  "faddp %%st(1), %%st"
                  : "=t" (ld5)
                  : "t" (ld5), "u" (ld6));
    
    /* 5. Use CRC32 builtin which has fixed register constraints */
    {
        unsigned int crc = 0xFFFFFFFF;
        crc = __builtin_ia32_crc32qi(crc, (unsigned char)seed);
        i5 = (int)crc;
    }
    
    /* 6. Another multi-alternative with memory/x87 choice */
    asm volatile ("fldt %1\n\t"
                  "faddp %%st(1), %%st"
                  : "=t" (ld7)
                  : "t" (ld7), "rm,t" (ld8));
    
    /* 7. Integer to x87 conversion with secondary reload potential */
    asm volatile ("fildl %1\n\t"
                  "fxch %%st(1)\n\t"
                  "faddp %%st(1), %%st"
                  : "=t" (ld9)
                  : "t" (ld9), "m" (i9));
    
    /* 8. Use division which requires specific registers on x86 */
    {
        volatile int divisor = seed + 1;
        if (divisor != 0) {
            asm volatile ("divl %2"
                          : "=a" (i10), "=d" (i11)
                          : "r" (divisor), "0" (i10), "1" (i11));
        }
    }
    
    /* 9. More x87 operations to increase pressure */
    asm volatile ("fmulp %%st(1), %%st"
                  : "=t" (ld10)
                  : "0" (ld10), "t" (ld11));
    
    /* 10. Mixed integer/x87 operation with output in x87 */
    asm volatile ("fildl %1\n\t"
                  "fmulp %%st(1), %%st"
                  : "=t" (ld12)
                  : "t" (ld12), "m" (i12));
    
    /* Store results to prevent elimination */
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
    
    /* Call multiple times to ensure execution */
    for (volatile int i = 0; i < 3; i++) {
        test_secondary_reloads(seed + i);
    }
    
    /* Compute a simple checksum to use results */
    int checksum = 0;
    for (int i = 0; i < 15; i++) {
        checksum += global_ints[i];
        checksum += (int)global_results[i];
    }
    
    return checksum & 0xFF;
}
