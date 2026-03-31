/* test_secondary_reloads.c
 * This program forces GCC to generate secondary reloads by:
 * 1. Using x87 long double operations with "t" and "u" constraints
 * 2. Creating high register pressure with many volatile variables
 * 3. Using inline assembly with multiple alternative constraints
 * 4. Mixing x87, general purpose, and fixed-register operations
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[20];
volatile int global_ints[20];

/* Prevent inlining to ensure reload happens in this function */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Create high register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8, ld9, ld10;
    volatile long double ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    
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
    i11 = seed + 11;
    i12 = seed * 12;
    i13 = seed + 13;
    i14 = seed * 14;
    i15 = seed + 15;
    
    /* Initialize long doubles */
    ld1 = (long double)(i1) / 3.0L;
    ld2 = (long double)(i2) / 5.0L;
    ld3 = (long double)(i3) / 7.0L;
    ld4 = (long double)(i4) / 11.0L;
    ld5 = (long double)(i5) / 13.0L;
    ld6 = (long double)(i6) / 17.0L;
    ld7 = (long double)(i7) / 19.0L;
    ld8 = (long double)(i8) / 23.0L;
    ld9 = (long double)(i9) / 29.0L;
    ld10 = (long double)(i10) / 31.0L;
    ld11 = (long double)(i11) / 37.0L;
    ld12 = (long double)(i12) / 41.0L;
    ld13 = (long double)(i13) / 43.0L;
    ld14 = (long double)(i14) / 47.0L;
    ld15 = (long double)(i15) / 53.0L;
    
    /* Use RDTSC which implicitly uses eax and edx */
    {
        uint32_t lo, hi;
        asm volatile ("rdtsc" : "=a"(lo), "=d"(hi));
        i1 = lo + hi;
    }
    
    /* Force x87 operations with explicit register constraints */
    /* This should trigger secondary reloads for moving values into x87 stack */
    
    /* Example 1: Simple x87 operation with "t" constraint */
    asm volatile (
        "faddp %%st(1), %%st"
        : "=t"(ld2)
        : "0"(ld1), "t"(ld2)
        : "st(1)"
    );
    
    /* Example 2: Using "u" constraint (second x87 register) */
    asm volatile (
        "fmulp %%st(1), %%st"
        : "=t"(ld4)
        : "0"(ld3), "u"(ld4)
        : "st(1)"
    );
    
    /* Example 3: Mixed integer and x87 - this is key for secondary reloads */
    /* The "rm,t" alternative constraint may force secondary reload for integer */
    {
        int temp_int = i2;
        long double temp_ld = ld5;
        
        asm volatile (
            "# Mixed operation with alternative constraints\n\t"
            "fildl %2\n\t"
            "faddp %%st(1), %%st"
            : "=t"(ld5)
            : "0"(temp_ld), "rm,t"(temp_int)
            : "st(1)"
        );
    }
    
    /* Example 4: Complex pattern with multiple alternatives */
    /* This may force the compiler to choose x87 alternative */
    {
        long double result;
        int int_val = i3;
        long double ld_val = ld6;
        
        asm volatile (
            "# Complex alternative constraints\n\t"
            "1: fildl %2\n\t"
            "faddp %%st(1), %%st\n\t"
            "2:\n"
            : "=t"(result), "=t"(ld6)
            : "0"(ld_val), "rm,t"(int_val), "t"(ld6)
            : "st(1)", "st(2)"
        );
        ld6 = result;
    }
    
    /* Example 5: Use CRC32 builtin which has fixed register constraints */
    /* Then mix with x87 operations */
    {
        unsigned int crc = 0xFFFFFFFF;
        crc = __builtin_ia32_crc32qi(crc, (unsigned char)i4);
        i5 = (int)crc;
        
        /* Now use this in x87 operation */
        long double temp = ld7;
        int crc_int = (int)crc;
        
        asm volatile (
            "fildl %2\n\t"
            "fmulp %%st(1), %%st"
            : "=t"(ld7)
            : "0"(temp), "rm,t"(crc_int)
            : "st(1)"
        );
    }
    
    /* Create more register pressure with additional operations */
    for (volatile int j = 0; j < 3; j++) {
        /* Chain x87 operations */
        asm volatile (
            "fldt %1\n\t"
            "fldt %2\n\t"
            "faddp %%st(1), %%st\n\t"
            "fstpt %0"
            : "=m"(ld8)
            : "m"(ld9), "m"(ld10)
            : "st", "st(1)"
        );
        
        /* Mix with integer operations */
        i6 = i7 * i8 + j;
        ld9 = ld8 * 1.1L;
        
        /* Another mixed operation */
        {
            long double src = ld11;
            int ival = i9 + j;
            
            asm volatile (
                "# Another mixed op\n\t"
                "fildl %2\n\t"
                "faddp %%st(1), %%st"
                : "=t"(ld11)
                : "0"(src), "rm,t"(ival)
                : "st(1)"
            );
        }
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
}

int main(int argc, char *argv[]) {
    int seed = 42;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call multiple times to ensure execution */
    for (int i = 0; i < 2; i++) {
        test_secondary_reloads(seed + i);
    }
    
    /* Compute and print a checksum to ensure code runs */
    int checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum += global_ints[i];
        checksum += (int)global_results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
