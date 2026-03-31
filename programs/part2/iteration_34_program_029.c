/* test_secondary_reloads.c */
/* Compile with: gcc -O1 -m32 -mfpmath=387 -march=i686 -fno-omit-frame-pointer test_secondary_reloads.c -o test_reloads */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[32];
volatile int global_ints[32];
int global_index = 0;

/* Prevent inlining to ensure complex reload patterns aren't optimized away */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Create high register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    volatile long double ld9, ld10, ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15, i16, i17, i18, i19, i20;
    
    /* Initialize with seed-dependent values to prevent constant folding */
    i1 = seed + 1;
    i2 = seed * 2;
    i3 = seed ^ 0x1234;
    i4 = seed - 100;
    i5 = seed * seed;
    i6 = seed + 0x5678;
    i7 = seed | 0x9ABC;
    i8 = seed & 0xDEF0;
    i9 = seed << 3;
    i10 = seed >> 2;
    
    /* Initialize long double values */
    ld1 = (long double)seed + 0.1L;
    ld2 = (long double)seed * 2.2L;
    ld3 = (long double)seed / 3.3L;
    ld4 = (long double)(seed ^ 0x1111) + 4.4L;
    ld5 = (long double)(seed * 3) - 5.5L;
    ld6 = (long double)(seed + 100) * 0.6L;
    ld7 = (long double)(seed - 50) / 7.7L;
    ld8 = (long double)(seed | 0x2222) + 8.8L;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    {
        unsigned long long tsc;
        tsc = __builtin_ia32_rdtsc();
        i11 = (int)(tsc & 0xFFFFFFFF);
        i12 = (int)(tsc >> 32);
    }
    
    /* Force x87 register usage with inline assembly */
    /* This should trigger secondary reloads for moving values into x87 stack */
    
    /* Example 1: Basic x87 operation with 't' constraint (top of x87 stack) */
    asm volatile (
        "fldt %1\n\t"          /* load ld1 into st(0) */
        "fldt %2\n\t"          /* load ld2 into st(0), ld1 moves to st(1) */
        "faddp %%st, %%st(1)\n\t"  /* st(1) = st(1) + st(0), pop stack */
        "fstpt %0"
        : "=m" (ld9)
        : "m" (ld1), "m" (ld2)
        : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
    );
    
    /* Example 2: Mixed constraints - 't' for x87 and 'r' for general register */
    /* This is likely to trigger secondary reload setup */
    {
        long double result;
        int temp = i1 + i2;
        
        /* Multi-alternative constraint: "rm,t" - either memory/general reg OR x87 top */
        asm volatile (
            "fldt %1\n\t"          /* load input long double */
            "fildl %2\n\t"         /* convert integer to long double and push */
            "fmulp %%st, %%st(1)\n\t"  /* multiply and pop */
            "fstpt %0"
            : "=m" (result)
            : "m" (ld3), "rm,t" (temp)
            : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
        );
        ld10 = result;
    }
    
    /* Example 3: More complex pattern with output in 't' constraint */
    {
        long double out;
        long double in1 = ld4;
        long double in2 = ld5;
        
        /* Output in 't' constraint, inputs also in x87 constraints */
        asm volatile (
            "fldt %2\n\t"      /* load in2 to st(0) */
            "fldt %1\n\t"      /* load in1 to st(0), in2 moves to st(1) */
            "fsubrp %%st, %%st(1)\n\t"  /* st(1) = st(1) - st(0), pop */
            : "=t" (out)
            : "t" (in1), "t" (in2)
            : "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
        );
        ld11 = out;
    }
    
    /* Example 4: CRC32 builtin which has fixed register constraints */
    /* This creates additional register pressure and special constraints */
    {
        unsigned int crc = 0xFFFFFFFF;
        unsigned char data = (unsigned char)seed;
        
        /* CRC32 instruction uses eax for accumulator */
        crc = __builtin_ia32_crc32qi(crc, data);
        i13 = (int)crc;
        
        /* Now use this in an x87 operation */
        long double crc_ld = (long double)crc;
        asm volatile (
            "fldt %1\n\t"
            "fldt %2\n\t"
            "faddp %%st, %%st(1)\n\t"
            "fstpt %0"
            : "=m" (ld12)
            : "m" (ld6), "m" (crc_ld)
            : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
        );
    }
    
    /* Example 5: Complex inline asm with multiple outputs and inputs */
    /* Using 'u' constraint (second x87 register) */
    {
        long double out1, out2;
        long double in_a = ld7;
        long double in_b = ld8;
        int scale = i3;
        
        asm volatile (
            "fldt %3\n\t"          /* in_b -> st(0) */
            "fldt %2\n\t"          /* in_a -> st(0), in_b -> st(1) */
            "fildl %4\n\t"         /* scale -> st(0), others shift up */
            "fxch %%st(2)\n\t"     /* exchange st(0) and st(2) */
            "fmulp %%st, %%st(2)\n\t"  /* st(2) = st(2) * st(0), pop */
            "faddp %%st, %%st(1)\n\t"  /* st(1) = st(1) + st(0), pop */
            "fstpt %0\n\t"         /* store result1 */
            "fstpt %1"             /* store result2 */
            : "=m" (out1), "=m" (out2)
            : "m" (in_a), "m" (in_b), "rm,t" (scale)
            : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
        );
        ld13 = out1;
        ld14 = out2;
    }
    
    /* Loop to increase register pressure and create more reload opportunities */
    {
        volatile int counter;
        long double accum = 1.0L;
        
        for (counter = 0; counter < 3; counter++) {
            long double temp = ld1 + (long double)counter;
            
            /* This asm uses 't' constraint with a changing value */
            asm volatile (
                "fldt %1\n\t"
                "fld1\n\t"
                "faddp %%st, %%st(1)\n\t"
                "fstpt %0"
                : "=m" (accum)
                : "m" (temp)
                : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
            );
            
            /* Mix with integer operations */
            i14 += i4 * counter;
        }
        ld15 = accum;
    }
    
    /* Store results to globals to prevent elimination */
    global_results[global_index++] = ld9;
    global_results[global_index++] = ld10;
    global_results[global_index++] = ld11;
    global_results[global_index++] = ld12;
    global_results[global_index++] = ld13;
    global_results[global_index++] = ld14;
    global_results[global_index++] = ld15;
    
    global_ints[0] = i11;
    global_ints[1] = i12;
    global_ints[2] = i13;
    global_ints[3] = i14;
}

int main(int argc, char *argv[]) {
    int seed = 12345;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call multiple times with different seeds */
    test_secondary_reloads(seed);
    test_secondary_reloads(seed + 1);
    test_secondary_reloads(seed + 2);
    
    /* Compute a simple checksum to use results */
    int checksum = 0;
    for (int i = 0; i < global_index && i < 32; i++) {
        checksum += (int)global_results[i];
    }
    for (int i = 0; i < 4; i++) {
        checksum += global_ints[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
