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

/* Prevent inlining to ensure complex reload patterns aren't optimized away */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Declare many volatile variables to create register pressure */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    volatile long double ld9, ld10, ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    volatile unsigned int cycles_low, cycles_high;
    
    /* Initialize with seed-dependent values to avoid constant propagation */
    i1 = seed + 1;
    i2 = seed * 2 + 3;
    i3 = seed ^ 0x1234;
    i4 = seed * 3 + 7;
    i5 = seed / 2 + 11;
    
    /* Initialize long doubles with non-trivial values */
    ld1 = (long double)seed / 3.1415926535L;
    ld2 = (long double)(seed + 1) * 2.7182818284L;
    ld3 = (long double)(seed * 2) / 1.4142135623L;
    ld4 = (long double)(seed ^ 0xABCD) + 1.6180339887L;
    ld5 = (long double)seed * seed / 100.0L;
    
    /* Force use of RDTSC which uses fixed registers (eax, edx) */
    asm volatile (
        "rdtsc"
        : "=a" (cycles_low), "=d" (cycles_high)
    );
    
    i6 = (int)(cycles_low ^ cycles_high);
    
    /* Complex inline assembly with x87 register constraints that require secondary reloads */
    
    /* 1. Simple x87 operation with 't' constraint (top of x87 stack) */
    asm volatile (
        "fldt %1\n\t"           /* Load first operand onto x87 stack */
        "fldt %2\n\t"           /* Load second operand */
        "faddp %%st(1), %%st\n\t" /* Add and pop */
        "fstpt %0"              /* Store result */
        : "=m" (ld6)
        : "m" (ld1), "m" (ld2)
        : "st", "st(1)"
    );
    
    /* 2. Mixed constraints: 't' (x87 top) and 'rm' (register/memory) */
    /* This may trigger secondary reload for the integer operand */
    asm volatile (
        "fildl %2\n\t"          /* Load integer to x87 stack */
        "fldt %1\n\t"           /* Load long double */
        "fmulp %%st, %%st(1)\n\t" /* Multiply */
        "fstpt %0"              /* Store result */
        : "=m" (ld7)
        : "m" (ld3), "m" (i3)
        : "st", "st(1)"
    );
    
    /* 3. Multi-alternative constraint: "rm,t" - compiler chooses between
       register/memory or x87 top register */
    /* This is key for triggering secondary reload initialization */
    {
        long double result;
        long double input = ld4;
        int int_val = i4;
        
        asm volatile (
            "fldt %1\n\t"       /* Load input (in x87 top if 't' constraint chosen) */
            "fildl %2\n\t"      /* Load integer */
            "faddp %%st, %%st(1)\n\t" /* Add */
            "fstpt %0"          /* Store result */
            : "=m" (result)
            : "t" (input), "rm,t" (int_val)  /* Multi-alternative constraint! */
            : "st", "st(1)"
        );
        
        ld8 = result;
    }
    
    /* 4. More complex pattern with output in x87 register */
    {
        long double out1, out2;
        long double in1 = ld5, in2 = ld6;
        
        /* First operation: result in x87 top */
        asm volatile (
            "fldt %2\n\t"
            "fldt %3\n\t"
            "fsubrp %%st, %%st(1)\n\t"
            "fstpt %0\n\t"
            "fldt %4\n\t"
            "fsqrt\n\t"
            "fstpt %1"
            : "=m" (out1), "=m" (out2)
            : "m" (in1), "m" (in2), "m" (ld7)
            : "st", "st(1)"
        );
        
        ld9 = out1;
        ld10 = out2;
    }
    
    /* 5. Use CRC32 builtin which has fixed register constraints */
    /* This creates additional register pressure with specific requirements */
    i7 = __builtin_ia32_crc32qi(i5, (unsigned char)i6);
    i8 = __builtin_ia32_crc32qi(i7, (unsigned char)i1);
    
    /* 6. Chain operations to keep values live */
    for (volatile int counter = 0; counter < 3; counter++) {
        /* Another multi-alternative constraint example */
        long double temp = ld8 + (long double)counter;
        int int_temp = i7 + counter;
        
        asm volatile (
            "fldt %1\n\t"
            "fildl %2\n\t"
            "fmulp %%st, %%st(1)\n\t"
            "fstpt %0"
            : "=m" (ld11)
            : "t" (temp), "rm,t" (int_temp)
            : "st", "st(1)"
        );
    }
    
    /* 7. Use 'u' constraint (second x87 register) */
    asm volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "fxch %%st(1)\n\t"      /* Exchange st(0) and st(1) */
        "faddp %%st, %%st(1)\n\t"
        "fstpt %0"
        : "=m" (ld12)
        : "u" (ld9), "t" (ld10)  /* 'u' = second x87 register */
        : "st", "st(1)"
    );
    
    /* Store results to globals to prevent elimination */
    global_results[0] = ld6;
    global_results[1] = ld7;
    global_results[2] = ld8;
    global_results[3] = ld9;
    global_results[4] = ld10;
    global_results[5] = ld11;
    global_results[6] = ld12;
    
    global_ints[0] = i6;
    global_ints[1] = i7;
    global_ints[2] = i8;
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    printf("Testing secondary reloads with seed = %d\n", seed);
    
    /* Call multiple times to ensure execution */
    for (int i = 0; i < 5; i++) {
        test_secondary_reloads(seed + i);
    }
    
    /* Compute a simple checksum from results */
    int checksum = 0;
    for (int i = 0; i < 7; i++) {
        checksum += (int)global_results[i];
    }
    for (int i = 0; i < 3; i++) {
        checksum += global_ints[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
