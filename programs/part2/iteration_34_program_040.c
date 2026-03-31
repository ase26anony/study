/* test_secondary_reloads.c
 * Designed to trigger secondary reload initialization in GCC's reload pass
 * Compile with: gcc -O1 -m32 -mfpmath=387 -march=i686 -fno-omit-frame-pointer -c test_secondary_reloads.c
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
    
    /* Initialize with seed-dependent values to avoid constant propagation */
    i1 = seed * 1;
    i2 = seed * 2 + 1;
    i3 = seed * 3 + 2;
    i4 = seed * 4 + 3;
    i5 = seed * 5 + 4;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    {
        unsigned long long ts1, ts2;
        ts1 = __builtin_ia32_rdtsc();
        i6 = (int)(ts1 >> 32) + i1;
        i7 = (int)(ts1 & 0xFFFFFFFF) + i2;
        ts2 = __builtin_ia32_rdtsc();
        i8 = (int)(ts2 >> 32) + i3;
        i9 = (int)(ts2 & 0xFFFFFFFF) + i4;
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
        : "=t" (ld1)
        : "0" (ld1), "t" (ld2)
        : "st(1)"
    );
    
    /* 2. Mixed operation with memory operand */
    ld3 = ld3 * ld4;
    asm volatile (
        "fldt %1\n\t"
        "faddp %%st(1), %%st"
        : "=t" (ld3)
        : "m" (ld5), "0" (ld3)
        : "st(1)"
    );
    
    /* 3. CRITICAL: Multi-alternative constraint that may force secondary reload */
    /* "rm,t" constraint: compiler can choose register/memory OR x87 top */
    /* This is where secondary reload fields get initialized */
    i10 = i5 + i6;
    asm volatile (
        "fildl %2\n\t"          /* Load integer into x87 stack */
        "faddp %%st(1), %%st"
        : "=t" (ld4)
        : "0" (ld4), "rm,t" (i10)
        : "st(1)"
    );
    
    /* 4. More complex pattern with "u" constraint (second x87 register) */
    ld6 = ld6 + ld7;
    asm volatile (
        "fxch %%st(1)\n\t"
        "fadd %%st(1), %%st\n\t"
        "fxch %%st(1)"
        : "=u" (ld6), "=t" (ld7)
        : "0" (ld6), "1" (ld7)
    );
    
    /* 5. Use CRC32 builtin which has fixed register constraints */
    {
        unsigned int crc = 0xFFFFFFFF;
        crc = __builtin_ia32_crc32qi(crc, (unsigned char)i7);
        i11 = (int)crc + i8;
    }
    
    /* 6. Another multi-alternative with different register classes */
    /* Mix x87 with general register constraints */
    i12 = i9 * 2;
    asm volatile (
        "pushl %2\n\t"
        "fildl (%%esp)\n\t"
        "faddp %%st(1), %%st\n\t"
        "addl $4, %%esp"
        : "=t" (ld8)
        : "0" (ld8), "ri" (i12)  /* "ri" = register or immediate */
        : "memory"
    );
    
    /* 7. Chain operations to increase register pressure */
    ld9 = ld1 + ld3;
    ld10 = ld4 + ld6;
    
    /* Force another x87 operation with memory constraint */
    asm volatile (
        "fldt %2\n\t"
        "fmulp %%st(1), %%st"
        : "=t" (ld9)
        : "0" (ld9), "m" (ld10)
        : "st(1)"
    );
    
    /* 8. Division operation that might use fixed registers */
    i13 = i10 / (i11 + 1);
    
    /* 9. More x87 operations in a loop to create multiple reload opportunities */
    for (volatile int j = 0; j < 3; j++) {
        long double temp = ld8 * (j + 1);
        asm volatile (
            "faddp %%st(1), %%st"
            : "=t" (ld11)
            : "0" (ld11), "t" (temp)
            : "st(1)"
        );
    }
    
    /* 10. Final mixed operation with multiple constraints */
    i14 = i13 * i12;
    asm volatile (
        "fildl %2\n\t"
        "fldt %3\n\t"
        "faddp %%st(1), %%st\n\t"
        "fmulp %%st(1), %%st"
        : "=t" (ld12)
        : "0" (ld9), "rm" (i14), "m" (ld11)
        : "st(1)"
    );
    
    /* Store results to globals to prevent elimination */
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
    
    /* Call multiple times with different seeds */
    test_secondary_reloads(seed);
    test_secondary_reloads(seed + 1);
    test_secondary_reloads(seed + 2);
    
    /* Compute checksum to use results */
    int checksum = 0;
    for (int i = 0; i < 15; i++) {
        checksum += global_ints[i];
        checksum += (int)global_results[i % 12];
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
