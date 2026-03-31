/* test_secondary_reloads.c */
/* Compile with: gcc -O1 -m32 -mfpmath=387 -march=i686 -fno-omit-frame-pointer -o test test_secondary_reloads.c */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[20];
volatile int global_ints[20];

/* Prevent inlining to ensure reload logic isn't optimized away */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* High register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    volatile long double ld9, ld10, ld11, ld12;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    
    /* Initialize with seed-dependent values to avoid constant folding */
    i1 = seed * 1;
    i2 = seed * 2 + 1;
    i3 = seed * 3 + 2;
    i4 = seed * 4 + 3;
    i5 = seed * 5 + 4;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    {
        uint64_t tsc1, tsc2;
        tsc1 = __builtin_ia32_rdtsc();
        i6 = (int)(tsc1 >> 32) + i1;
        i7 = (int)(tsc1 & 0xFFFFFFFF) + i2;
        tsc2 = __builtin_ia32_rdtsc();
        i8 = (int)(tsc2 - tsc1) + i3;
    }
    
    /* Initialize long double values with conversions from integers */
    ld1 = (long double)i1 * 1.1L;
    ld2 = (long double)i2 * 1.2L;
    ld3 = (long double)i3 * 1.3L;
    ld4 = (long double)i4 * 1.4L;
    ld5 = (long double)i5 * 1.5L;
    ld6 = (long double)i6 * 1.6L;
    ld7 = (long double)i7 * 1.7L;
    ld8 = (long double)i8 * 1.8L;
    
    /* Force x87 operations with explicit register constraints */
    
    /* 1. Simple x87 operation with "t" constraint (top of x87 stack) */
    asm volatile (
        "faddp %%st(1), %%st"
        : "=t" (ld9)
        : "0" (ld1), "t" (ld2)
        : "st(1)"
    );
    
    /* 2. Mixed operation: x87 with integer input that might need secondary reload */
    /* "rm,t" constraint: either memory/register OR x87 top */
    /* This can trigger secondary reload setup for the integer operand */
    i9 = i4 * 3;
    asm volatile (
        "fildl %2\n\t"
        "faddp %%st(1), %%st"
        : "=t" (ld10)
        : "0" (ld3), "rm,t" (i9)
        : "st(1)"
    );
    
    /* 3. More complex: multiple x87 operations chained */
    asm volatile (
        "fmulp %%st(1), %%st\n\t"
        "fsubrp %%st(1), %%st"
        : "=t" (ld11)
        : "0" (ld4), "t" (ld5), "u" (ld6)
        : "st(1)", "st(2)"
    );
    
    /* 4. Use CRC32 builtin which has fixed register constraints */
    /* This creates additional register class mixing */
    {
        unsigned int crc = 0xFFFFFFFF;
        crc = __builtin_ia32_crc32qi(crc, (unsigned char)i5);
        i10 = (int)crc + i6;
    }
    
    /* 5. Another mixed operation with alternative constraints */
    /* The compiler may choose the "t" alternative for i10, requiring secondary reload */
    i11 = i7 ^ i8;
    asm volatile (
        "fildl %2\n\t"
        "fmulp %%st(1), %%st"
        : "=t" (ld12)
        : "0" (ld7), "rm,t" (i11)
        : "st(1)"
    );
    
    /* 6. Operation requiring value in second x87 register ("u" constraint) */
    /* This can trigger secondary reloads for moving between x87 registers */
    {
        long double temp1 = ld8 * 2.0L;
        long double temp2 = ld9 * 3.0L;
        asm volatile (
            "faddp %%st(1), %%st"
            : "=u" (temp1)
            : "0" (temp1), "t" (temp2)
            : "st(1)"
        );
        ld8 = temp1;
    }
    
    /* 7. Complex inline asm with multiple outputs and inputs */
    /* Mixing x87 and general registers increases reload complexity */
    i12 = i9 + i10;
    i13 = i11 * 2;
    asm volatile (
        "fildl %3\n\t"          /* load i12 into x87 stack */
        "fildl %4\n\t"          /* load i13 into x87 stack */
        "faddp %%st(1), %%st\n\t"
        "fistpl %0\n\t"         /* store result to i14 */
        "fwait"
        : "=m" (i14), "=t" (ld9)
        : "0" (i14), "rm" (i12), "rm" (i13), "1" (ld10)
        : "st(1)", "st(2)"
    );
    
    /* 8. Final computation mixing everything */
    i15 = i14 + seed;
    {
        long double final_ld;
        /* Multi-alternative constraint that may force secondary reload */
        asm volatile (
            "fildl %2\n\t"
            "faddp %%st(1), %%st\n\t"
            "fsqrt"
            : "=t" (final_ld)
            : "0" (ld11), "rm,t" (i15)
            : "st(1)"
        );
        ld12 = final_ld;
    }
    
    /* Store results to global arrays to prevent optimization */
    global_results[0] = ld1; global_results[1] = ld2;
    global_results[2] = ld3; global_results[3] = ld4;
    global_results[4] = ld5; global_results[5] = ld6;
    global_results[6] = ld7; global_results[7] = ld8;
    global_results[8] = ld9; global_results[9] = ld10;
    global_results[10] = ld11; global_results[11] = ld12;
    
    global_ints[0] = i1; global_ints[1] = i2;
    global_ints[2] = i3; global_ints[3] = i4;
    global_ints[4] = i5; global_ints[5] = i6;
    global_ints[6] = i7; global_ints[7] = i8;
    global_ints[8] = i9; global_ints[9] = i10;
    global_ints[10] = i11; global_ints[11] = i12;
    global_ints[12] = i13; global_ints[13] = i14;
    global_ints[14] = i15;
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call multiple times with different seeds to explore different paths */
    for (int i = 0; i < 3; i++) {
        test_secondary_reloads(seed + i);
    }
    
    /* Compute a simple checksum from results */
    int checksum = 0;
    for (int i = 0; i < 15; i++) {
        checksum += global_ints[i];
        checksum ^= (int)global_results[i % 12];
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
