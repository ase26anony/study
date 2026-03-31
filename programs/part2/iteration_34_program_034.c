/* reload_secondary_test.c
 * Test program to cover secondary reload initialization in GCC's reload pass.
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
    volatile int counter;
    
    /* Initialize with seed-dependent values to avoid constant propagation */
    i1 = seed * 1;
    i2 = seed * 2;
    i3 = seed * 3;
    i4 = seed * 4;
    i5 = seed * 5;
    
    /* Use rdtsc builtin which uses fixed registers (eax, edx) */
    {
        unsigned long long tsc;
        tsc = __builtin_ia32_rdtsc();
        i6 = (int)(tsc & 0xFFFFFFFF);
        i7 = (int)(tsc >> 32);
    }
    
    /* Initialize long doubles with conversions from integers */
    ld1 = (long double)i1 + 0.5L;
    ld2 = (long double)i2 + 1.5L;
    ld3 = (long double)i3 + 2.5L;
    ld4 = (long double)i4 + 3.5L;
    ld5 = (long double)i5 + 4.5L;
    
    /* Force x87 operations with explicit asm using "t" (top of x87 stack) constraint */
    
    /* Example 1: Simple x87 operation forcing secondary reloads */
    asm volatile (
        "fldt %1\n\t"           /* Load first operand to st(0) */
        "fldt %2\n\t"           /* Load second operand to st(0), previous moves to st(1) */
        "faddp %%st, %%st(1)\n\t" /* st(1) = st(1) + st(0), pop st(0) */
        "fstpt %0"              /* Store result */
        : "=m" (ld6)
        : "m" (ld1), "m" (ld2)
        : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
    );
    
    /* Example 2: Mixed constraints - "rm,t" alternative for secondary reload */
    i8 = i1 + i2;
    asm volatile (
        "fldt %1\n\t"           /* Load ld3 to st(0) */
        "fildl %2\n\t"          /* Load integer i8 to st(0), previous moves to st(1) */
        "faddp %%st, %%st(1)\n\t" /* Add */
        "fstpt %0"
        : "=m" (ld7)
        : "m" (ld3), "m" (i8)
        : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
    );
    
    /* Example 3: Multiple alternative constraints with x87 register */
    /* This is key for triggering secondary reload initialization */
    i9 = i3 * 2;
    {
        long double result;
        asm volatile (
            "fldt %1\n\t"       /* Load input to st(0) */
            /* The integer operand has two alternatives: general reg/mem OR x87 register */
            /* GCC may need secondary reload to move integer to x87 register */
            "fildl %2\n\t"      /* Load integer - will need secondary reload if in register */
            "fmulp %%st, %%st(1)\n\t"
            "fstpt %0"
            : "=m" (result)
            : "m" (ld4), "r,m" (i9)  /* "r,m" alternative - may force secondary reload */
            : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
        );
        ld8 = result;
    }
    
    /* Example 4: Output in x87 register with input alternatives */
    {
        long double input = ld5;
        int multiplier = i4;
        long double output;
        
        asm volatile (
            /* Input constraint with "t" (x87 top) alternative */
            "fldt %1\n\t"       /* Load long double to st(0) */
            /* Integer input with "r" (general reg) and "t" (x87) alternatives */
            /* When "t" is chosen for the integer, GCC needs secondary reload */
            "fildl %2\n\t"      /* Convert integer to long double in st(0) */
            "fmulp %%st, %%st(1)\n\t"
            : "=t" (output)     /* Output in x87 top register */
            : "t" (input), "r,t" (multiplier)  /* Critical: "r,t" for secondary reload */
            : "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
        );
        ld9 = output;
    }
    
    /* Example 5: Complex pattern with multiple x87 operations */
    /* Create register pressure by using many x87 registers */
    asm volatile (
        "fldt %1\n\t"           /* ld2 -> st(0) */
        "fldt %2\n\t"           /* ld3 -> st(0), ld2 -> st(1) */
        "fldt %3\n\t"           /* ld4 -> st(0), others shift down */
        "fxch %%st(2)\n\t"      /* exchange st(0) and st(2) */
        "faddp %%st, %%st(1)\n\t" /* st(1) = st(1) + st(0), pop st(0) */
        "fxch %%st(1)\n\t"
        "fmulp %%st, %%st(1)\n\t"
        "fstpt %0"
        : "=m" (ld10)
        : "m" (ld2), "m" (ld3), "m" (ld4)
        : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
    );
    
    /* Use CRC32 builtin which has fixed register constraints */
    i10 = __builtin_ia32_crc32qi(i1, (char)i2);
    
    /* More operations to increase register pressure */
    for (counter = 0; counter < 3; counter++) {
        /* Mix integer and float operations */
        i11 = i10 + counter;
        ld11 = ld10 * (long double)(i11 + 1);
        
        /* Another asm with x87 constraints */
        asm volatile (
            "fldt %1\n\t"
            "fldt %2\n\t"
            "faddp %%st, %%st(1)\n\t"
            "fstpt %0"
            : "=m" (ld12)
            : "m" (ld11), "m" (ld1)
            : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
        );
        
        /* Use the "u" constraint (second x87 register) */
        {
            long double tmp1 = ld12, tmp2 = ld2;
            asm volatile (
                "fldt %2\n\t"   /* tmp2 -> st(0) */
                "fldt %1\n\t"   /* tmp1 -> st(0), tmp2 -> st(1) */
                "fsubrp %%st, %%st(1)\n\t" /* st(1) = st(0) - st(1), pop st(0) */
                "fstpt %0"
                : "=m" (ld13)
                : "u" (tmp1), "m" (tmp2)  /* "u" is st(1) */
                : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
            );
        }
    }
    
    /* Store results to globals to prevent elimination */
    global_results[0] = ld6;
    global_results[1] = ld7;
    global_results[2] = ld8;
    global_results[3] = ld9;
    global_results[4] = ld10;
    global_results[5] = ld13;
    
    global_ints[0] = i6;
    global_ints[1] = i7;
    global_ints[2] = i8;
    global_ints[3] = i9;
    global_ints[4] = i10;
    global_ints[5] = i11;
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
    for (int i = 0; i < 6; i++) {
        checksum += global_ints[i];
        checksum += (int)global_results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
