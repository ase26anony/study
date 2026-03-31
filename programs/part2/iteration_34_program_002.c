/* test_secondary_reloads.c */
/* Compile with: gcc -O1 -m32 -mfpmath=387 -march=i686 -fno-omit-frame-pointer test_secondary_reloads.c -o test_secondary_reloads */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[20];
volatile int global_ints[20];

/* Prevent inlining to ensure reload logic is exercised */
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
    i4 = seed * 3 + 5;
    i5 = seed / 2 + 7;
    i6 = seed + 11;
    i7 = seed * 5 - 3;
    i8 = seed ^ 0x5678;
    i9 = seed + 13;
    i10 = seed * 7 + 17;
    i11 = seed - 5;
    i12 = seed ^ 0x9ABC;
    i13 = seed * 11 + 19;
    i14 = seed + 23;
    i15 = seed / 3 + 29;
    
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
    ld10 = (long double)i10 * 10.1L;
    ld11 = (long double)i11 * 11.1L;
    ld12 = (long double)i12 * 12.1L;
    ld13 = (long double)i13 * 13.1L;
    ld14 = (long double)i14 * 14.1L;
    ld15 = (long double)i15 * 15.1L;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    {
        uint64_t tsc1, tsc2;
        tsc1 = __builtin_ia32_rdtsc();
        i1 = (int)(tsc1 & 0xFFFFFFFF);
        tsc2 = __builtin_ia32_rdtsc();
        i2 = (int)(tsc2 & 0xFFFFFFFF);
    }
    
    volatile int loop_counter;
    for (loop_counter = 0; loop_counter < 3; loop_counter++) {
        /* CRITICAL: Inline asm with x87 register constraints that may require secondary reloads */
        
        /* 1. Basic x87 operation with "t" constraint (top of x87 stack) */
        asm volatile (
            "fldt %2\n\t"           /* Load ld2 onto x87 stack */
            "fldt %1\n\t"           /* Load ld1 onto x87 stack */
            "faddp %%st, %%st(1)\n\t" /* st(1) = st(1) + st, pop st */
            "fstpt %0\n\t"          /* Store result */
            : "=m" (ld1)            /* Output in memory */
            : "m" (ld1), "m" (ld2)  /* Inputs in memory */
            : "st", "st(1)"
        );
        
        /* 2. Mixed integer/x87 operation with multi-alternative constraint */
        /* This is key: "rm,t" constraint may force secondary reload for integer */
        int temp_int = i3 + loop_counter;
        asm volatile (
            "fildl %2\n\t"          /* Load integer onto x87 stack */
            "fldt %1\n\t"           /* Load ld3 onto x87 stack */
            "fmulp %%st, %%st(1)\n\t" /* Multiply */
            "fstpt %0\n\t"          /* Store result */
            : "=m" (ld3)
            : "m" (ld3), 
              "rm,t" (temp_int)     /* CRITICAL: Alternative constraints */
            : "st", "st(1)"
        );
        
        /* 3. Complex pattern with output in "t" and input in "u" (second x87 reg) */
        asm volatile (
            "fldt %2\n\t"           /* Load ld5 */
            "fldt %1\n\t"           /* Load ld4 */
            "fsubrp %%st, %%st(1)\n\t" /* st(1) = st - st(1), pop st */
            : "=t" (ld4)            /* Output in top of x87 stack */
            : "u" (ld4),            /* Input in second x87 register */
              "t" (ld5)             /* Input in top of x87 stack */
        );
        
        /* 4. Use CRC32 builtin which has fixed register constraints */
        i4 = __builtin_ia32_crc32qi(i4, (unsigned char)i5);
        
        /* 5. Another mixed operation with memory and register constraints */
        long double temp_ld = ld6;
        asm volatile (
            "fldt %1\n\t"
            "fsqrt\n\t"             /* Square root */
            "fstpt %0\n\t"
            : "=m" (ld6)
            : "m" (temp_ld)
            : "st"
        );
        
        /* 6. Chain operations to increase register pressure */
        ld7 = ld7 * ld8 + ld9;
        ld10 = ld10 / ld11 - ld12;
        
        /* 7. Integer operations to use general registers */
        i6 = i6 * i7 + i8;
        i9 = i9 ^ i10 | i11;
        
        /* 8. Conversion between types to force moves between register classes */
        i12 = (int)ld13;
        ld14 = (long double)i13;
        
        /* 9. Another asm with complex constraints */
        int int_for_x87 = i14 + seed;
        asm volatile (
            "fildl %1\n\t"
            "fldt %2\n\t"
            "fdivrp %%st, %%st(1)\n\t"
            "fstpt %0\n\t"
            : "=m" (ld15)
            : "rm,t" (int_for_x87),  /* Integer with alternative constraints */
              "m" (ld15)
            : "st", "st(1)"
        );
    }
    
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
    
    /* Call multiple times with different seeds */
    test_secondary_reloads(seed);
    test_secondary_reloads(seed + 1);
    test_secondary_reloads(seed + 2);
    
    /* Compute and print a checksum to ensure code runs */
    int checksum = 0;
    for (int i = 0; i < 15; i++) {
        checksum += global_ints[i];
        checksum += (int)global_results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
