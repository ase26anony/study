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
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    volatile long double ld9, ld10, ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    volatile unsigned int cycles_low, cycles_high;
    volatile long double temp_result;
    
    /* Initialize with seed-dependent values to avoid constant folding */
    i1 = seed + 1;
    i2 = seed * 2 + 3;
    i3 = seed ^ 0x1234;
    i4 = seed * seed;
    i5 = seed + 100;
    
    /* Initialize long doubles with non-constant values */
    ld1 = (long double)(seed + 1) / 3.14159265358979L;
    ld2 = (long double)(seed * 2) / 2.71828182845904L;
    ld3 = (long double)(seed ^ 0xABCD) / 1.41421356237309L;
    ld4 = (long double)(seed + 1000) * 0.57721566490153L;
    ld5 = (long double)(seed * 3) * 1.61803398874989L;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    asm volatile (
        "rdtsc"
        : "=a" (cycles_low), "=d" (cycles_high)
    );
    
    i6 = (int)(cycles_low ^ cycles_high);
    
    /* Force x87 operations with explicit register constraints */
    /* This should trigger secondary reloads for moving values into x87 stack */
    
    /* Operation 1: Simple x87 addition with 't' constraint (top of x87 stack) */
    asm volatile (
        "faddp %%st(1), %%st"
        : "=t" (ld6)
        : "0" (ld1), "t" (ld2)
        : "st(1)"
    );
    
    /* Operation 2: Multiplication with mixed constraints */
    /* The 'u' constraint is second x87 register */
    asm volatile (
        "fmulp %%st(1), %%st"
        : "=t" (ld7)
        : "0" (ld3), "u" (ld4)
        : "st(1)"
    );
    
    /* Operation 3: Critical pattern - mixed integer and x87 with multi-alternative constraint */
    /* The "rm,t" constraint may force secondary reload for integer operand */
    i7 = i1 + i2;
    asm volatile (
        "fildl %2\n\t"
        "faddp %%st(1), %%st"
        : "=t" (ld8)
        : "0" (ld5), "rm,t" (i7)
        : "st(1)"
    );
    
    /* Operation 4: Division with memory operand that might need secondary reload */
    i8 = i3 * i4;
    asm volatile (
        "fidivl %2\n\t"
        : "=t" (ld9)
        : "0" (ld6), "m" (i8)
        : "st(1)"
    );
    
    /* Operation 5: Use CRC32 builtin which has fixed register constraints */
    /* This creates additional register class pressure */
    i9 = __builtin_ia32_crc32qi(i5, (unsigned char)seed);
    
    /* Operation 6: Complex pattern with output in st(1) and input in st(0) */
    /* This requires swapping x87 registers */
    asm volatile (
        "fxch %%st(1)\n\t"
        "fsqrt\n\t"
        "fxch %%st(1)"
        : "=t" (ld10), "=u" (temp_result)
        : "0" (ld7), "1" (ld8)
    );
    
    /* Operation 7: Integer to x87 conversion with multiple alternatives */
    /* The compiler may choose the 't' alternative for the integer */
    i10 = i6 ^ i9;
    asm volatile (
        "fildl %2\n\t"
        "faddp %%st(1), %%st"
        : "=t" (ld11)
        : "0" (ld9), "rm,t" (i10)
        : "st(1)"
    );
    
    /* Create more register pressure with additional operations */
    for (volatile int j = 0; j < 3; j++) {
        /* Mix integer and floating point operations */
        i11 = i1 + j;
        i12 = i2 * (j + 1);
        
        /* Another x87 operation with memory constraint */
        asm volatile (
            "fadds %2\n\t"
            : "=t" (ld12)
            : "0" (ld10), "m" (ld11)
        );
        
        /* Use the result */
        i13 = (int)ld12 + i11;
        
        /* Chain operations to create dependencies */
        asm volatile (
            "fmulp %%st(1), %%st"
            : "=t" (ld13)
            : "0" (ld12), "u" (ld13)
            : "st(1)"
        );
    }
    
    /* Operation 8: Use MMX-style 64-bit operation (another special reg class) */
    /* Note: MMX registers may require secondary reloads when mixing with x87 */
    {
        volatile long long mmx_val = (long long)seed * 1000000LL;
        volatile long double ld_temp = ld13;
        
        /* This asm uses q register constraint (64-bit integer in MMX/SSE) */
        asm volatile (
            "movq %2, %%mm0\n\t"
            "emms\n\t"
            "fildll %2\n\t"
            "faddp %%st(1), %%st"
            : "=t" (ld14)
            : "0" (ld_temp), "m" (mmx_val)
            : "mm0"
        );
    }
    
    /* Operation 9: Final complex operation with multiple constraints */
    i14 = i13 + i7 + i8;
    asm volatile (
        "fildl %2\n\t"      /* Load integer */
        "fmulp %%st(1), %%st\n\t"
        "faddp %%st(1), %%st"
        : "=t" (ld15)
        : "0" (ld14), "rm,t" (i14), "u" (ld11)
        : "st(1)", "st(2)"
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
    printf("Compile with: gcc -O1 -m32 -mfpmath=387 -march=i686 -fno-omit-frame-pointer\n");
    
    /* Call multiple times to ensure code generation */
    for (int i = 0; i < 2; i++) {
        test_secondary_reloads(seed + i);
    }
    
    /* Compute a simple checksum from results */
    int checksum = 0;
    for (int i = 0; i < 15; i++) {
        checksum += global_ints[i];
        checksum += (int)global_results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
