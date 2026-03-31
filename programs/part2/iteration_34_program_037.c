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

/* Prevent inlining to ensure reload logic is exercised */
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
    i3 = seed + 3;
    i4 = seed * 4;
    i5 = seed + 5;
    i6 = seed * 6;
    i7 = seed + 7;
    i8 = seed * 8;
    i9 = seed + 9;
    i10 = seed * 10;
    
    /* Initialize long doubles */
    ld1 = (long double)seed / 3.0L;
    ld2 = (long double)seed * 1.5L;
    ld3 = (long double)seed + 2.7L;
    ld4 = (long double)seed / 1.8L;
    ld5 = (long double)seed * 3.14159L;
    ld6 = (long double)seed + 1.618L;
    ld7 = (long double)seed / 2.718L;
    ld8 = (long double)seed * 1.414L;
    ld9 = (long double)seed + 0.577L;
    ld10 = (long double)seed / 1.202L;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    {
        unsigned long long tsc1, tsc2;
        tsc1 = __builtin_ia32_rdtsc();
        i11 = (int)(tsc1 >> 32);
        i12 = (int)(tsc1 & 0xFFFFFFFF);
        
        /* Create some computation to prevent optimization */
        for (volatile int j = 0; j < 3; j++) {
            tsc2 = __builtin_ia32_rdtsc();
        }
        i13 = (int)(tsc2 >> 32);
        i14 = (int)(tsc2 & 0xFFFFFFFF);
    }
    
    /* Force x87 operations with explicit register constraints */
    
    /* Example 1: x87 addition with 't' constraint (top of x87 stack) */
    asm volatile (
        "faddp %%st(1), %%st"
        : "=t" (ld1)
        : "0" (ld1), "t" (ld2)
        : "st(1)"
    );
    
    /* Example 2: x87 multiplication with 'u' constraint (second x87 register) */
    asm volatile (
        "fmulp %%st(1), %%st"
        : "=t" (ld3)
        : "0" (ld3), "u" (ld4)
        : "st(1)"
    );
    
    /* CRITICAL: Multi-alternative constraint that may force secondary reload */
    /* "rm,t" means either memory/register OR x87 top-of-stack */
    /* The compiler may choose the 't' alternative, requiring secondary reload */
    {
        long double result;
        int int_val = i1 + i2;
        
        asm volatile (
            "fildl %2\n\t"
            "faddp %%st(1), %%st"
            : "=t" (result)
            : "0" (ld5), "rm,t" (int_val)
            : "st(1)"
        );
        ld5 = result;
    }
    
    /* Another complex case: mixing x87 with general registers */
    {
        long double temp1 = ld6, temp2 = ld7;
        int int_val1 = i3 * i4;
        int int_val2 = i5 + i6;
        
        /* First load integer into x87 stack */
        asm volatile (
            "fildl %2\n\t"
            "fstpt %0"
            : "=m" (global_ints[0])
            : "m" (int_val1)
            : "st"
        );
        
        /* Then use it in x87 operation */
        asm volatile (
            "fldt %2\n\t"
            "faddp %%st(1), %%st"
            : "=t" (temp1)
            : "0" (temp1), "m" (global_ints[0])
            : "st(1)"
        );
        ld6 = temp1;
        
        /* Alternative approach with output reload */
        asm volatile (
            "fildl %1\n\t"
            "fadds %2\n\t"
            "fstpt %0"
            : "=m" (global_results[0])
            : "rm" (int_val2), "m" (ld8)
        );
    }
    
    /* Use CRC32 builtin which has fixed register constraints */
    {
        unsigned int crc = 0xFFFFFFFF;
        crc = __builtin_ia32_crc32qi(crc, (unsigned char)i7);
        crc = __builtin_ia32_crc32qi(crc, (unsigned char)i8);
        i15 = (int)crc;
    }
    
    /* Complex sequence mixing everything */
    for (volatile int k = 0; k < 2; k++) {
        /* x87 operation */
        asm volatile (
            "fld1\n\t"
            "faddp %%st(1), %%st"
            : "+t" (ld9)
            :
            : "st(1)"
        );
        
        /* Integer operation that might conflict */
        i16 = __builtin_ia32_rdtsc() & 0xFF;
        
        /* Another x87 operation with memory operand */
        asm volatile (
            "fld %1\n\t"
            "fmulp %%st(1), %%st"
            : "=t" (ld10)
            : "m" (ld2), "0" (ld10)
            : "st(1)"
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
    global_ints[15] = i16;
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    printf("Testing secondary reloads with seed = %d\n", seed);
    
    /* Call multiple times to ensure execution */
    for (int i = 0; i < 3; i++) {
        test_secondary_reloads(seed + i);
    }
    
    /* Compute and print a checksum to verify execution */
    int checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum += (int)global_results[i];
        checksum += global_ints[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
