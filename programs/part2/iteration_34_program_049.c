/* reload_secondary_test.c
 * Test program to cover secondary reload initialization in GCC's reload pass
 * Compile with: gcc -O1 -m32 -mfpmath=387 -march=i686 -fno-omit-frame-pointer reload_secondary_test.c -o reload_test
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[32];
volatile int global_ints[32];
volatile int result_index = 0;

/* Prevent inlining to ensure reload happens at call boundaries */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* High register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    volatile long double ld9, ld10, ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15, i16, i17, i18, i19, i20;
    
    /* Initialize with seed-dependent values to prevent constant folding */
    i1 = seed + 1;
    i2 = seed * 2 + 3;
    i3 = seed ^ 0x55AA;
    i4 = seed << 3;
    i5 = seed >> 2;
    i6 = seed * seed;
    i7 = seed + 0x1234;
    i8 = seed - 0x5678;
    i9 = ~seed;
    i10 = seed | 0xF0F0;
    
    /* More variables for pressure */
    i11 = i1 + i2;
    i12 = i3 * i4;
    i13 = i5 ^ i6;
    i14 = i7 & i8;
    i15 = i9 | i10;
    i16 = i11 << 2;
    i17 = i12 >> 1;
    i18 = i13 + i14;
    i19 = i15 * i16;
    i20 = i17 ^ i18;
    
    /* Initialize long double variables */
    ld1 = (long double)seed + 0.1L;
    ld2 = (long double)seed * 2.5L;
    ld3 = (long double)(seed ^ 0xAA) / 3.0L;
    ld4 = (long double)i1 + 0.7L;
    ld5 = (long double)i2 - 1.3L;
    ld6 = (long double)i3 * 0.8L;
    ld7 = (long double)i4 / 2.0L;
    ld8 = (long double)i5 + 3.14159L;
    
    ld9 = ld1 + ld2;
    ld10 = ld3 * ld4;
    ld11 = ld5 - ld6;
    ld12 = ld7 / ld8;
    ld13 = ld9 * 1.1L;
    ld14 = ld10 + 2.2L;
    ld15 = ld11 - 3.3L;
    
    /* Force use of x87 registers with inline asm */
    /* This asm uses "t" constraint (top of x87 stack) */
    asm volatile (
        "fldt %1\n\t"          /* load ld1 into st(0) */
        "fldt %2\n\t"          /* load ld2 into st(0), ld1 moves to st(1) */
        "faddp %%st, %%st(1)\n\t" /* st(1) = st(1) + st(0), pop stack */
        "fstpt %0"
        : "=m" (ld1)
        : "m" (ld2), "m" (ld3)
        : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
    );
    
    /* Use __builtin_ia32_rdtsc() which uses fixed registers (eax, edx) */
    {
        unsigned long long tsc1, tsc2;
        tsc1 = __builtin_ia32_rdtsc();
        /* Do some work to prevent optimization */
        i1 = (int)(tsc1 >> 32) + (int)tsc1;
        tsc2 = __builtin_ia32_rdtsc();
        i2 = (int)(tsc2 >> 32) - (int)tsc2;
    }
    
    /* Complex asm with multiple alternative constraints */
    /* "rm,t" constraint: either general register/memory OR x87 top register */
    {
        long double result;
        int int_val = i3;
        long double ld_val = ld4;
        
        asm volatile (
            "fldt %2\n\t"
            /* Alternative 1: int_val in register/memory, convert to long double */
            "fildl %1\n\t"
            /* Alternative 2: int_val already in x87 stack (if "t" chosen) */
            "1:\n\t"
            "faddp %%st, %%st(1)\n\t"
            "fstpt %0"
            : "=m" (result)
            : "rm,t" (int_val), "m" (ld_val)
            : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
        );
        
        ld5 = result;
    }
    
    /* Another complex pattern: mixing x87 with MMX-like operations */
    /* Use division which requires specific registers on x86 */
    {
        volatile int divisor = i4 + 1;
        volatile int dividend = i5;
        int quotient, remainder;
        
        /* Division uses fixed registers: eax for dividend, edx for remainder */
        asm volatile (
            "movl %2, %%eax\n\t"
            "cltd\n\t"
            "idivl %3\n\t"
            "movl %%eax, %0\n\t"
            "movl %%edx, %1"
            : "=r" (quotient), "=r" (remainder)
            : "r" (dividend), "r" (divisor)
            : "eax", "edx", "cc"
        );
        
        i6 = quotient;
        i7 = remainder;
    }
    
    /* Chain x87 operations with register constraints */
    {
        long double temp1, temp2, temp3;
        
        /* First operation: load two values, add them */
        asm volatile (
            "fldt %1\n\t"
            "fldt %2\n\t"
            "faddp %%st, %%st(1)\n\t"
            "fstpt %0"
            : "=m" (temp1)
            : "m" (ld6), "m" (ld7)
            : "st", "st(1)"
        );
        
        /* Second operation: use "u" constraint (second x87 register) */
        asm volatile (
            "fldt %1\n\t"      /* temp1 -> st(0) */
            "fldt %2\n\t"      /* ld8 -> st(0), temp1 -> st(1) */
            "fmulp %%st, %%st(1)\n\t" /* st(1) = st(1) * st(0), pop */
            "fstpt %0"
            : "=m" (temp2)
            : "m" (temp1), "m" (ld8)
            : "st", "st(1)"
        );
        
        /* Third operation with output in "t" constraint */
        asm volatile (
            "fldt %1\n\t"
            "fsqrt\n\t"
            : "=t" (temp3)
            : "m" (temp2)
        );
        
        ld9 = temp3;
    }
    
    /* Use CRC32 builtin which has fixed register constraints */
    {
        unsigned int crc = 0xFFFFFFFF;
        unsigned char data = (unsigned char)seed;
        
        crc = __builtin_ia32_crc32qi(crc, data);
        i8 = (int)crc;
    }
    
    /* Store results to globals to prevent elimination */
    global_results[result_index++] = ld1;
    global_results[result_index++] = ld5;
    global_results[result_index++] = ld9;
    global_ints[0] = i1;
    global_ints[1] = i2;
    global_ints[2] = i6;
    global_ints[3] = i7;
    global_ints[4] = i8;
    
    /* Loop to increase reload opportunities */
    volatile int loop_counter;
    for (loop_counter = 0; loop_counter < 3; loop_counter++) {
        long double loop_ld = ld10 + (long double)loop_counter;
        int loop_int = i9 + loop_counter;
        
        asm volatile (
            "fldt %1\n\t"
            "fildl %2\n\t"
            "faddp %%st, %%st(1)\n\t"
            "fstpt %0"
            : "=m" (loop_ld)
            : "m" (loop_ld), "r" (loop_int)
            : "st", "st(1)"
        );
        
        ld10 = loop_ld;
    }
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
    
    /* Compute checksum to use results */
    int checksum = 0;
    for (int i = 0; i < result_index && i < 32; i++) {
        checksum += (int)global_results[i];
    }
    for (int i = 0; i < 5; i++) {
        checksum += global_ints[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}
