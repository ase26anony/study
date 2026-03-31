/* test_secondary_reloads.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[20];
volatile int global_ints[20];

/* Prevent inlining to ensure reload logic isn't optimized away */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Create high register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    volatile long double ld9, ld10, ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    
    /* Initialize with seed-dependent values to prevent constant folding */
    i1 = seed + 1;
    i2 = seed * 2 + 3;
    i3 = seed ^ 0x1234;
    i4 = seed * 3 - 5;
    i5 = seed + 0xABCD;
    i6 = seed * 7 + 11;
    i7 = seed / 3;
    i8 = seed % 100;
    i9 = seed * seed;
    i10 = seed + 999;
    i11 = seed | 0xFF00;
    i12 = seed & 0x0F0F;
    i13 = seed << 3;
    i14 = seed >> 2;
    i15 = ~seed;
    
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
    
    /* Force use of x87 registers with inline assembly */
    /* This asm uses "t" constraint (top of x87 stack) which often requires secondary reloads */
    asm volatile (
        "fldt %2\n\t"           /* Load ld2 onto x87 stack */
        "fldt %1\n\t"           /* Load ld1 onto x87 stack */
        "faddp %%st, %%st(1)\n\t" /* st(1) = st(1) + st, pop st */
        "fstpt %0\n\t"          /* Store result */
        : "=m" (ld1)
        : "m" (ld1), "m" (ld2)
        : "st", "st(1)"
    );
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    {
        unsigned long long ts1, ts2;
        ts1 = __builtin_ia32_rdtsc();
        /* Use the timestamp to modify an integer */
        i1 = (int)(ts1 & 0xFFFFFFFF);
        ts2 = __builtin_ia32_rdtsc();
        i2 = (int)(ts2 & 0xFFFFFFFF);
    }
    
    /* Complex asm with multiple alternative constraints */
    /* The "rm,t" constraint may force secondary reload for integer operand */
    {
        long double result;
        int int_val = i3;
        long double ld_val = ld3;
        
        asm volatile (
            "fldt %2\n\t"       /* Load ld_val */
            "fildl %1\n\t"      /* Load and convert int_val to long double */
            "faddp %%st, %%st(1)\n\t"
            "fstpt %0\n\t"
            : "=m" (result)
            : "rm,t" (int_val), "m" (ld_val)
            : "st", "st(1)"
        );
        ld3 = result;
    }
    
    /* Another asm using x87 stack manipulation with "u" constraint (second x87 reg) */
    {
        long double temp1 = ld4, temp2 = ld5;
        asm volatile (
            "fldt %2\n\t"       /* temp2 -> st(0) */
            "fldt %1\n\t"       /* temp1 -> st(0), temp2 -> st(1) */
            "fmulp %%st, %%st(1)\n\t"  /* st(1) = st(1) * st, pop st */
            "fstpt %0\n\t"
            : "=m" (ld4)
            : "u" (temp1), "t" (temp2)
            : "st", "st(1)"
        );
    }
    
    /* Mix integer and floating point with conversions */
    {
        /* Force integer to be moved to x87 stack */
        int divisor = i6;
        long double dividend = ld6;
        long double quotient;
        
        /* This may require secondary reload to get integer into x87 */
        asm volatile (
            "fldt %1\n\t"       /* Load dividend */
            "fildl %2\n\t"      /* Load and convert divisor */
            "fdivrp %%st, %%st(1)\n\t" /* st(1) = st(1) / st, pop st */
            "fstpt %0\n\t"
            : "=m" (quotient)
            : "m" (dividend), "rm" (divisor)
            : "st", "st(1)"
        );
        ld6 = quotient;
    }
    
    /* Use CRC32 builtin which has fixed register constraints */
    {
        unsigned int crc = 0xFFFFFFFF;
        unsigned char data = (unsigned char)seed;
        crc = __builtin_ia32_crc32qi(crc, data);
        i7 = (int)crc;
    }
    
    /* More x87 operations to increase pressure */
    asm volatile (
        "fldt %1\n\t"
        "fsqrt\n\t"
        "fstpt %0\n\t"
        : "=m" (ld7)
        : "m" (ld7)
        : "st"
    );
    
    /* Chain operations to create complex live ranges */
    for (volatile int k = 0; k < 3; k++) {
        long double accum = ld8;
        int modifier = i8 + k;
        
        asm volatile (
            "fldt %1\n\t"
            "fildl %2\n\t"
            "faddp %%st, %%st(1)\n\t"
            "fstpt %0\n\t"
            : "=m" (accum)
            : "m" (accum), "rm" (modifier)
            : "st", "st(1)"
        );
        
        ld8 = accum;
        
        /* Use the result in another asm */
        asm volatile (
            "fldt %1\n\t"
            "fchs\n\t"          /* Change sign */
            "fstpt %0\n\t"
            : "=m" (ld9)
            : "m" (ld8)
            : "st"
        );
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
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call multiple times with different seeds */
    for (int i = 0; i < 5; i++) {
        test_secondary_reloads(seed + i);
    }
    
    /* Compute a simple checksum to use results */
    int checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum += global_ints[i];
        checksum += (int)global_results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
