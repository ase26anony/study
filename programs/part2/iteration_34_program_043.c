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
volatile int result_index = 0;

/* Prevent inlining to ensure reload logic isn't optimized away */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Declare many volatile variables to create register pressure */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    volatile long double ld9, ld10, ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    volatile int loop_counter;
    
    /* Initialize with seed to create non-constant values */
    i1 = seed + 1;
    i2 = seed * 2 + 3;
    i3 = seed ^ 0x1234;
    i4 = seed * seed;
    i5 = seed + 1000;
    
    /* Initialize long doubles using integer conversions */
    ld1 = (long double)i1 * 1.5L;
    ld2 = (long double)i2 * 2.5L;
    ld3 = (long double)i3 * 3.5L;
    ld4 = (long double)i4 * 4.5L;
    ld5 = (long double)i5 * 5.5L;
    
    /* More initialization */
    i6 = i1 + i2;
    i7 = i3 - i4;
    i8 = i5 * 2;
    i9 = seed % 100;
    i10 = seed / 3;
    
    ld6 = ld1 + ld2;
    ld7 = ld3 - ld4;
    ld8 = ld5 * 2.0L;
    ld9 = ld1 / 3.0L;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    {
        unsigned long long ts1, ts2;
        ts1 = __builtin_ia32_rdtsc();
        i11 = (int)(ts1 & 0xFFFFFFFF);
        ts2 = __builtin_ia32_rdtsc();
        i12 = (int)(ts2 & 0xFFFFFFFF);
    }
    
    /* Force x87 operations with explicit register constraints */
    
    /* Example 1: x87 addition with 't' constraint (top of x87 stack) */
    asm volatile (
        "faddp %%st(1), %%st"
        : "=t" (ld10)
        : "0" (ld6), "t" (ld7)
        : "st(1)"
    );
    
    /* Example 2: x87 multiplication with 'u' constraint (second x87 register) */
    asm volatile (
        "fmulp %%st(1), %%st"
        : "=t" (ld11)
        : "0" (ld8), "u" (ld9)
        : "st(1)"
    );
    
    /* Example 3: Mixed constraints that may require secondary reloads */
    /* 'rm,t' alternative constraint - compiler may choose 't' requiring secondary reload */
    asm volatile (
        "fildl %2\n\t"
        "faddp %%st(1), %%st"
        : "=t" (ld12)
        : "0" (ld10), "rm,t" (i6)
        : "st(1)"
    );
    
    /* More operations to increase complexity */
    i13 = i11 + i12;
    i14 = i13 * i9;
    i15 = i14 - i10;
    
    /* Another mixed operation with integer input */
    asm volatile (
        "fildl %2\n\t"
        "fmulp %%st(1), %%st"
        : "=t" (ld13)
        : "0" (ld11), "rm" (i7)
        : "st(1)"
    );
    
    /* Chain operations */
    ld14 = ld12 + ld13;
    
    /* Use CRC32 builtin which has fixed register constraints */
    i15 = __builtin_ia32_crc32qi(i15, (unsigned char)seed);
    
    /* Final x87 operation with multiple alternatives */
    asm volatile (
        "faddp %%st(1), %%st"
        : "=t" (ld15)
        : "0" (ld14), "rm,t" (i15)
        : "st(1)"
    );
    
    /* Store results to globals to prevent elimination */
    global_results[result_index++] = ld10;
    global_results[result_index++] = ld11;
    global_results[result_index++] = ld12;
    global_results[result_index++] = ld13;
    global_results[result_index++] = ld14;
    global_results[result_index++] = ld15;
    
    global_ints[0] = i11;
    global_ints[1] = i12;
    global_ints[2] = i13;
    global_ints[3] = i14;
    global_ints[4] = i15;
    
    /* Loop to keep variables live and increase register pressure */
    for (loop_counter = 0; loop_counter < 3; loop_counter++) {
        /* More x87 operations in loop */
        asm volatile (
            "faddp %%st(1), %%st"
            : "=t" (ld1)
            : "0" (ld1), "t" (ld2)
            : "st(1)"
        );
        
        ld2 = ld2 * 1.1L;
        i1 = i1 + loop_counter;
    }
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
    long double checksum = 0.0L;
    for (int i = 0; i < result_index && i < 20; i++) {
        checksum += global_results[i];
    }
    
    printf("Result checksum: %Lf\n", checksum);
    printf("Integer checksum: %d\n", 
           global_ints[0] + global_ints[1] + global_ints[2] + 
           global_ints[3] + global_ints[4]);
    
    return 0;
}
