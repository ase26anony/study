/* test_secondary_reloads.c
 * This program forces GCC to generate secondary reloads by:
 * 1. Using x87 long double operations with "t" and "u" constraints
 * 2. Creating high register pressure with many volatile variables
 * 3. Using inline assembly with multiple alternative constraints
 * 4. Employing builtins with fixed register requirements
 */

#include <stdint.h>
#include <stdlib.h>

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
    i2 = seed * 2;
    i3 = seed + 3;
    i4 = seed * 4;
    i5 = seed + 5;
    i6 = seed * 6;
    i7 = seed + 7;
    i8 = seed * 8;
    i9 = seed + 9;
    i10 = seed * 10;
    i11 = seed + 11;
    i12 = seed * 12;
    i13 = seed + 13;
    i14 = seed * 14;
    i15 = seed + 15;
    
    /* Convert some ints to long double for x87 operations */
    ld1 = (long double)i1;
    ld2 = (long double)i2;
    ld3 = (long double)i3;
    ld4 = (long double)i4;
    ld5 = (long double)i5;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    {
        unsigned long long ts1, ts2;
        ts1 = __builtin_ia32_rdtsc();
        i6 = (int)(ts1 & 0xFFFFFFFF);
        ts2 = __builtin_ia32_rdtsc();
        i7 = (int)(ts2 & 0xFFFFFFFF);
    }
    
    /* Force x87 operations with explicit register constraints */
    /* This asm uses "t" (top of x87 stack) and "u" (second x87 register) */
    asm volatile (
        "fldt %2\n\t"           /* load ld2 to st(0) */
        "fldt %1\n\t"           /* load ld1 to st(0), ld2 moves to st(1) */
        "faddp %%st, %%st(1)\n\t" /* st(1) = st(0) + st(1), pop */
        "fstpt %0"
        : "=m" (ld6)
        : "m" (ld1), "m" (ld2)
        : "st", "st(1)"
    );
    
    /* More x87 operations to increase pressure */
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fmulp %%st, %%st(1)\n\t"
        "fstpt %0"
        : "=m" (ld7)
        : "m" (ld3), "m" (ld4)
        : "st", "st(1)"
    );
    
    /* CRITICAL: Inline asm with multiple alternative constraints
     * This can trigger secondary reloads when the compiler chooses
     * the "t" alternative for an integer operand */
    {
        volatile int temp_int = i8;
        volatile long double temp_ld = ld5;
        
        /* "rm,t" constraint: either memory/register OR x87 top register
         * This may force secondary reload for the integer */
        asm volatile (
            "fildl %2\n\t"      /* load integer to x87 stack */
            "fldt %1\n\t"       /* load long double */
            "faddp %%st, %%st(1)\n\t"
            "fstpt %0"
            : "=m" (ld8)
            : "m" (temp_ld), "rm,t" (temp_int)
            : "st", "st(1)"
        );
    }
    
    /* Another complex pattern: mixing x87 and general registers */
    {
        volatile long double a = ld6;
        volatile long double b = ld7;
        volatile int c = i9;
        
        /* Multiple output constraints with x87 registers */
        asm volatile (
            "fldt %2\n\t"       /* b -> st(0) */
            "fldt %1\n\t"       /* a -> st(0), b -> st(1) */
            "faddp %%st, %%st(1)\n\t"  /* st(1) = a + b, pop */
            "fistpl %0\n\t"     /* convert to int and store */
            "fstpt %3"          /* store long double result */
            : "=m" (i10), "=t" (ld9)
            : "m" (a), "m" (b), "0" (c)
            : "st", "st(1)"
        );
    }
    
    /* Use CRC32 builtin which has fixed register constraints */
    {
        unsigned int crc = 0xFFFFFFFF;
        crc = __builtin_ia32_crc32qi(crc, (unsigned char)i11);
        i12 = (int)crc;
    }
    
    /* More operations to ensure code isn't optimized away */
    ld10 = ld8 + ld9;
    ld11 = ld6 * ld7;
    ld12 = ld10 - ld11;
    
    /* Convert between int and long double multiple times */
    ld13 = (long double)i13;
    i14 = (int)ld13;
    ld14 = (long double)i14;
    
    /* Final x87 operation with explicit "t" constraint on output */
    asm volatile (
        "fldt %1\n\t"
        "fsqrt\n\t"
        "fstpt %0"
        : "=t" (ld15)
        : "m" (ld12)
        : "st"
    );
    
    /* Store results to global arrays to prevent elimination */
    global_results[0] = ld6;
    global_results[1] = ld7;
    global_results[2] = ld8;
    global_results[3] = ld9;
    global_results[4] = ld10;
    global_results[5] = ld11;
    global_results[6] = ld12;
    global_results[7] = ld13;
    global_results[8] = ld14;
    global_results[9] = ld15;
    
    global_ints[0] = i6;
    global_ints[1] = i7;
    global_ints[2] = i8;
    global_ints[3] = i9;
    global_ints[4] = i10;
    global_ints[5] = i11;
    global_ints[6] = i12;
    global_ints[7] = i13;
    global_ints[8] = i14;
    global_ints[9] = i15;
}

int main(int argc, char **argv) {
    int seed = 42;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call multiple times with different seeds */
    test_secondary_reloads(seed);
    test_secondary_reloads(seed + 1);
    test_secondary_reloads(seed + 2);
    
    /* Use results to prevent dead code elimination */
    volatile int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += global_ints[i];
        sum += (int)global_results[i];
    }
    
    return sum & 0xFF;  /* Return non-zero to indicate execution */
}
