/* reload_secondary_test.c
 * Test program to trigger secondary reloads in GCC's reload pass
 * Compile with: gcc -O1 -m32 -mfpmath=387 -march=i686 -fno-omit-frame-pointer reload_secondary_test.c -o reload_test
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[20];
volatile int global_ints[20];

/* Prevent inlining to ensure reload decisions are made */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Declare many volatile variables to create register pressure */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8, ld9, ld10;
    volatile long double ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    
    /* Initialize with seed-dependent values to prevent constant propagation */
    i1 = seed + 1;
    i2 = seed * 2 + 3;
    i3 = seed ^ 0x1234;
    i4 = seed * seed;
    i5 = seed + 100;
    
    /* Use rdtsc builtin which uses fixed registers (eax, edx) */
    {
        unsigned long long tsc;
        tsc = __builtin_ia32_rdtsc();
        i6 = (int)(tsc & 0xFFFFFFFF);
        i7 = (int)(tsc >> 32);
    }
    
    /* Initialize long double variables */
    ld1 = (long double)seed + 0.5L;
    ld2 = (long double)seed * 1.5L;
    ld3 = (long double)seed / 3.0L;
    ld4 = (long double)i1 + 0.25L;
    ld5 = (long double)i2 - 0.75L;
    
    /* Force x87 register usage with inline asm using "t" constraint */
    /* This should trigger secondary reloads for moving values into x87 stack */
    
    /* Example 1: Basic x87 operation with input/output in x87 registers */
    asm volatile (
        "faddp %%st(1), %%st"
        : "=t" (ld6)
        : "0" (ld1), "t" (ld2)
        : "st(1)"
    );
    
    /* Example 2: Mixed constraints - integer to x87 conversion */
    /* The "rm,t" alternative constraint may force secondary reload */
    asm volatile (
        "fildl %2\n\t"
        "faddp %%st(1), %%st"
        : "=t" (ld7)
        : "0" (ld3), "rm,t" (i3)
    );
    
    /* Example 3: More complex pattern with multiple alternatives */
    /* This uses both "t" (top of x87 stack) and "u" (second x87 register) */
    asm volatile (
        "fmulp %%st(2), %%st\n\t"
        "faddp %%st(1), %%st"
        : "=t" (ld8)
        : "0" (ld4), "u" (ld5), "t" (ld6)
        : "st(1)", "st(2)"
    );
    
    /* Example 4: Integer operation that might conflict with x87 usage */
    /* CRC32 instruction uses fixed register for accumulator */
    {
        unsigned int crc = 0xFFFFFFFF;
        crc = __builtin_ia32_crc32qi(crc, (unsigned char)seed);
        i8 = (int)crc;
    }
    
    /* Example 5: Another asm with multiple output constraints */
    /* This creates additional register pressure */
    asm volatile (
        "fldt %2\n\t"
        "fldt %3\n\t"
        "fmulp %%st(1), %%st\n\t"
        "fstpt %0\n\t"
        "fistpl %1"
        : "=m" (ld9), "=m" (i9)
        : "m" (ld7), "m" (ld8)
        : "st"
    );
    
    /* Example 6: Division operation that requires specific registers */
    /* Division uses fixed registers on x86 */
    {
        volatile int divisor = seed + 7;
        if (divisor != 0) {
            asm volatile (
                "divl %2"
                : "=a" (i10), "=d" (i11)
                : "r" (divisor), "0" (i4), "1" (i5)
                : "cc"
            );
        }
    }
    
    /* Example 7: Chain operations to keep values live */
    for (volatile int j = 0; j < 3; j++) {
        /* Mix x87 and general registers */
        asm volatile (
            "fldt %1\n\t"
            "fiaddl %2\n\t"
            "fstpt %0"
            : "=m" (ld10)
            : "m" (ld9), "r" (i6 + j)
            : "st"
        );
        
        /* Another operation with alternative constraints */
        asm volatile (
            "faddp %%st(1), %%st"
            : "=t" (ld11)
            : "0" (ld10), "rm,t" (i7)
        );
    }
    
    /* Example 8: Use MMX registers (another special register class) */
    /* This may trigger secondary reloads when mixing with x87 */
    {
        volatile long long mmx_var = (long long)seed * 1000LL;
        asm volatile (
            "movq %1, %%mm0\n\t"
            "psllq $2, %%mm0\n\t"
            "movq %%mm0, %0\n\t"
            "emms"
            : "=m" (mmx_var)
            : "m" (mmx_var)
            : "mm0"
        );
        i12 = (int)mmx_var;
    }
    
    /* Example 9: Complex expression mixing everything */
    ld12 = ld11 * 2.0L;
    
    asm volatile (
        "fldt %2\n\t"
        "fldt %3\n\t"
        "faddp %%st(1), %%st\n\t"
        "fildl %4\n\t"
        "faddp %%st(1), %%st\n\t"
        "fstpt %0\n\t"
        "fistpl %1"
        : "=m" (ld13), "=m" (i13)
        : "m" (ld12), "m" (ld8), "r" (i8)
        : "st", "st(1)"
    );
    
    /* Example 10: Final computation with high register pressure */
    /* Use all variables to keep them live */
    ld14 = ld1 + ld2 + ld3 + ld4 + ld5 + ld6 + ld7 + ld8 + ld9 + ld10;
    ld15 = ld11 + ld12 + ld13 + ld14;
    
    i14 = i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10;
    i15 = i11 + i12 + i13 + i14 + seed;
    
    /* Store to global arrays to prevent elimination */
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
    
    /* Call multiple times to ensure code generation */
    for (int i = 0; i < 2; i++) {
        test_secondary_reloads(seed + i);
    }
    
    /* Compute and print a checksum to ensure code executed */
    long double sum_ld = 0.0L;
    int sum_int = 0;
    
    for (int i = 0; i < 15; i++) {
        sum_ld += global_results[i];
        sum_int += global_ints[i];
    }
    
    printf("Checksum - long double: %Lf, int: %d\n", sum_ld, sum_int);
    
    return 0;
}
