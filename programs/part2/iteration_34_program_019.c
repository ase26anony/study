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

/* Prevent inlining to ensure reload logic isn't optimized away */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Create high register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    volatile long double ld9, ld10, ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    
    /* Initialize with seed-dependent values to avoid constant propagation */
    i1 = seed + 1;
    i2 = seed * 2 + 3;
    i3 = seed ^ 0x1234;
    i4 = seed << 3;
    i5 = seed | 0xABCD;
    
    /* Initialize long doubles using integer conversions */
    ld1 = (long double)i1 * 1.5L;
    ld2 = (long double)i2 * 2.5L;
    ld3 = (long double)i3 * 3.5L;
    ld4 = (long double)i4 * 4.5L;
    ld5 = (long double)i5 * 5.5L;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    {
        unsigned long long ts1, ts2;
        ts1 = __builtin_ia32_rdtsc();
        i6 = (int)(ts1 & 0xFFFFFFFF);
        ts2 = __builtin_ia32_rdtsc();
        i7 = (int)(ts2 & 0xFFFFFFFF);
    }
    
    /* Force x87 register usage with inline assembly */
    /* This asm uses "t" constraint (top of x87 stack) */
    asm volatile (
        "fldt %1\n\t"           /* Load ld1 onto x87 stack */
        "fldt %2\n\t"           /* Load ld2 onto x87 stack */
        "faddp %%st, %%st(1)\n\t" /* Add st to st(1) and pop */
        "fstpt %0\n\t"          /* Store result */
        : "=m" (ld6)
        : "m" (ld1), "m" (ld2)
        : "st", "st(1)"
    );
    
    /* More x87 operations to increase pressure */
    asm volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "fmulp %%st, %%st(1)\n\t"
        "fstpt %0\n\t"
        : "=m" (ld7)
        : "m" (ld3), "m" (ld4)
        : "st", "st(1)"
    );
    
    /* Critical part: inline asm with multiple alternative constraints
     * The "rm,t" constraint may force secondary reloads */
    {
        volatile long double ld_tmp = ld5;
        volatile int int_val = i6;
        
        asm volatile (
            "fldt %1\n\t"           /* Load input long double */
            "fildl %2\n\t"          /* Load integer - may need secondary reload */
            "faddp %%st, %%st(1)\n\t"
            "fstpt %0\n\t"
            : "=m" (ld8)
            : "m" (ld_tmp), "rm,t" (int_val)  /* Multi-alternative constraint */
            : "st", "st(1)"
        );
    }
    
    /* Another complex case: mixing x87 with MMX-like operations */
    {
        volatile long double ld_src1 = ld6;
        volatile long double ld_src2 = ld7;
        volatile int int_src = i7;
        
        /* This asm uses both "t" (x87 top) and "u" (x87 second) constraints */
        asm volatile (
            "fldt %2\n\t"           /* Load second operand to st(0) */
            "fldt %1\n\t"           /* Load first operand to st(0), previous moves to st(1) */
            "faddp %%st, %%st(1)\n\t" /* st(1) = st(1) + st, pop st */
            "fildl %3\n\t"          /* Load integer - potential secondary reload */
            "faddp %%st, %%st(1)\n\t" /* Add integer to sum */
            "fstpt %0\n\t"
            : "=m" (ld9)
            : "m" (ld_src1), "m" (ld_src2), "rm,t" (int_src)
            : "st", "st(1)", "st(2)"
        );
    }
    
    /* Use CRC32 builtin which has fixed register constraints */
    i8 = __builtin_ia32_crc32qi(i1, i2 & 0xFF);
    i9 = __builtin_ia32_crc32qi(i8, i3 & 0xFF);
    
    /* More operations to create register pressure */
    for (volatile int j = 0; j < 3; j++) {
        /* Complex asm with output in x87 register */
        long double result;
        asm volatile (
            "fldt %1\n\t"
            "fldt %2\n\t"
            "fdivrp %%st, %%st(1)\n\t"
            : "=t" (result)        /* Output in x87 top register */
            : "m" (ld8), "m" (ld9)
        );
        ld10 = result + j;
    }
    
    /* Store results to globals to prevent elimination */
    global_results[0] = ld6;
    global_results[1] = ld7;
    global_results[2] = ld8;
    global_results[3] = ld9;
    global_results[4] = ld10;
    
    global_ints[0] = i6;
    global_ints[1] = i7;
    global_ints[2] = i8;
    global_ints[3] = i9;
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call multiple times to increase coverage chances */
    for (int i = 0; i < 5; i++) {
        test_secondary_reloads(seed + i);
    }
    
    /* Compute and print a checksum to ensure code executes */
    int checksum = 0;
    for (int i = 0; i < 5; i++) {
        checksum += (int)global_results[i] + global_ints[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
