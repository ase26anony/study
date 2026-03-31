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
volatile int result_index = 0;

/* Prevent inlining to ensure complex reload patterns aren't optimized away */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Declare many volatile variables to create register pressure */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8, ld9, ld10;
    volatile long double ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    volatile int counter;
    
    /* Initialize with seed-dependent values to avoid constant propagation */
    i1 = seed + 1;
    i2 = seed * 2 + 3;
    i3 = seed ^ 0x1234;
    i4 = seed << 3;
    i5 = seed >> 2;
    i6 = seed * seed;
    i7 = seed + 100;
    i8 = seed - 50;
    i9 = seed | 0xFF;
    i10 = seed & 0x7F;
    i11 = seed * 3 + 7;
    i12 = seed / 2;
    i13 = seed % 17;
    i14 = seed + 0xABCD;
    i15 = seed ^ 0xDEADBEEF;
    
    /* Initialize long doubles using integer values */
    ld1 = (long double)i1 * 1.1L;
    ld2 = (long double)i2 * 2.2L;
    ld3 = (long double)i3 * 3.3L;
    ld4 = (long double)i4 * 4.4L;
    ld5 = (long double)i5 * 5.5L;
    ld6 = (long double)i6 * 6.6L;
    ld7 = (long double)i7 * 7.7L;
    ld8 = (long double)i8 * 8.8L;
    ld9 = (long double)i9 * 9.9L;
    ld10 = (long double)i10 * 10.10L;
    ld11 = (long double)i11 * 11.11L;
    ld12 = (long double)i12 * 12.12L;
    ld13 = (long double)i13 * 13.13L;
    ld14 = (long double)i14 * 14.14L;
    ld15 = (long double)i15 * 15.15L;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    {
        unsigned long long ts1, ts2;
        ts1 = __builtin_ia32_rdtsc();
        /* Do some work to prevent optimization */
        i1 = i1 + (ts1 & 0xFF);
        ts2 = __builtin_ia32_rdtsc();
        i2 = i2 + (ts2 - ts1);
    }
    
    /* CRITICAL PART 1: x87 operations with explicit register constraints
     * Using "t" constraint (top of x87 stack) forces secondary reloads
     */
    for (counter = 0; counter < 3; counter++) {
        /* x87 addition with both operands in x87 registers */
        asm volatile (
            "faddp %%st(1), %%st"
            : "=t" (ld1)
            : "0" (ld1), "t" (ld2)
            : "st(1)"
        );
        
        /* x87 multiplication */
        asm volatile (
            "fmulp %%st(1), %%st"
            : "=t" (ld3)
            : "0" (ld3), "t" (ld4)
            : "st(1)"
        );
        
        /* Mixed operation: x87 operation with integer input
         * This may force secondary reload for the integer operand
         */
        int temp_int = i3 + counter;
        asm volatile (
            "fildl %1\n\t"
            "faddp %%st(1), %%st"
            : "+t" (ld5)
            : "m" (temp_int)
            : "st(1)"
        );
    }
    
    /* CRITICAL PART 2: Inline asm with multiple alternative constraints
     * The "rm,t" constraint may select "t" alternative, requiring secondary reload
     */
    {
        volatile int int_val = i4 + seed;
        volatile long double ld_result;
        
        /* This asm has alternative constraints for the third operand:
         * "rm,t" means either memory/register OR x87 top-of-stack
         * GCC may choose the "t" alternative, forcing secondary reload setup
         */
        asm volatile (
            "# Multi-alternative constraint test\n\t"
            "fldt %2\n\t"
            "faddp %%st(1), %%st"
            : "=t" (ld_result)
            : "0" (ld6), "rm,t" (int_val)
            : "st(1)"
        );
        ld7 = ld_result;
    }
    
    /* CRITICAL PART 3: Complex pattern using both "t" and "u" constraints
     * "u" means second x87 register (st(1)), creating more register pressure
     */
    {
        volatile long double tmp1 = ld8, tmp2 = ld9;
        asm volatile (
            "fxch %%st(1)\n\t"
            "fadd %%st(1), %%st\n\t"
            "fxch %%st(1)"
            : "+u" (tmp1), "+t" (tmp2)
            :
            : "st(2)"
        );
        ld8 = tmp1;
        ld9 = tmp2;
    }
    
    /* Use CRC32 builtin which has fixed register constraints */
    {
        unsigned int crc = 0xFFFFFFFF;
        crc = __builtin_ia32_crc32qi(crc, (unsigned char)i5);
        crc = __builtin_ia32_crc32qi(crc, (unsigned char)i6);
        i10 = crc ^ i10;
    }
    
    /* More x87 operations to ensure reloads happen */
    for (counter = 0; counter < 2; counter++) {
        /* Division using x87 */
        asm volatile (
            "fdivp %%st(1), %%st"
            : "=t" (ld10)
            : "0" (ld10), "t" (ld11)
            : "st(1)"
        );
        
        /* Subtraction */
        asm volatile (
            "fsubp %%st(1), %%st"
            : "=t" (ld12)
            : "0" (ld12), "t" (ld13)
            : "st(1)"
        );
    }
    
    /* Store results to globals to prevent elimination */
    global_results[result_index++] = ld1;
    global_results[result_index++] = ld2;
    global_results[result_index++] = ld3;
    global_results[result_index++] = ld4;
    global_results[result_index++] = ld5;
    global_results[result_index++] = ld6;
    global_results[result_index++] = ld7;
    global_results[result_index++] = ld8;
    global_results[result_index++] = ld9;
    global_results[result_index++] = ld10;
    global_results[result_index++] = ld11;
    global_results[result_index++] = ld12;
    global_results[result_index++] = ld13;
    global_results[result_index++] = ld14;
    global_results[result_index++] = ld15;
    
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
    
    /* Compute a simple checksum to use results */
    int checksum = 0;
    for (int i = 0; i < 15; i++) {
        checksum += global_ints[i];
        checksum ^= (int)global_results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
