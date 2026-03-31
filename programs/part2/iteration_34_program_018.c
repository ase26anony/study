/* test_secondary_reloads.c
 * This program forces GCC to generate secondary reloads for x87 registers
 * to cover the initialization of secondary_* fields in struct reload.
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
    /* Create high register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    volatile long double ld9, ld10, ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    volatile int counter;
    
    /* Initialize with seed-dependent values to prevent constant folding */
    i1 = seed * 1;
    i2 = seed * 2 + 1;
    i3 = seed * 3 + 2;
    i4 = seed * 4 + 3;
    i5 = seed * 5 + 4;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    {
        uint64_t tsc1, tsc2;
        tsc1 = __builtin_ia32_rdtsc();
        i6 = (int)(tsc1 >> 32) + i1;
        i7 = (int)(tsc1 & 0xFFFFFFFF) + i2;
        
        /* Create some computation to prevent optimization */
        for (counter = 0; counter < 3; counter++) {
            tsc2 = __builtin_ia32_rdtsc();
            i8 = (int)(tsc2 - tsc1) + i3 + counter;
        }
    }
    
    /* Initialize long double values */
    ld1 = (long double)i1 / 3.14159265358979323846L;
    ld2 = (long double)i2 / 2.71828182845904523536L;
    ld3 = (long double)i3 / 1.41421356237309504880L;
    ld4 = (long double)i4 / 1.61803398874989484820L;
    ld5 = (long double)i5 / 0.57721566490153286060L;
    
    /* Force x87 operations with explicit register constraints */
    
    /* Example 1: x87 addition with "t" (top of stack) constraint */
    asm volatile (
        "faddp %%st(1), %%st"
        : "=t" (ld6)
        : "0" (ld1), "t" (ld2)
        : "st(1)"
    );
    
    /* Example 2: x87 multiplication with "u" (second x87 register) constraint */
    asm volatile (
        "fmulp %%st(1), %%st"
        : "=t" (ld7)
        : "0" (ld3), "u" (ld4)
        : "st(1)"
    );
    
    /* CRITICAL: Multi-alternative constraint that may force secondary reload */
    /* "rm,t" means either memory/general register OR x87 top register */
    /* The compiler may choose "t" alternative for the integer, requiring secondary reload */
    {
        int temp_int = i6 + i7;
        long double temp_ld = ld5;
        
        asm volatile (
            "# Multi-alternative constraint test\n\t"
            "fildl %2\n\t"           /* Load integer into x87 stack */
            "faddp %%st(1), %%st"    /* Add to existing value */
            : "=t" (ld8)
            : "0" (temp_ld), "rm,t" (temp_int)
            : "st(1)"
        );
    }
    
    /* Example 3: Complex pattern mixing x87 and general registers */
    /* This often triggers secondary reloads for the output */
    {
        long double result1, result2;
        int int_val = i8 * 2;
        
        /* First operation: load integer into x87 */
        asm volatile (
            "fildl %1\n\t"
            "fstpt %0"
            : "=m" (result1)
            : "rm" (int_val)
            : "st"
        );
        
        /* Second operation: use that result in another x87 operation */
        asm volatile (
            "fldt %1\n\t"
            "fldt %2\n\t"
            "faddp\n\t"
            "fstpt %0"
            : "=m" (result2)
            : "m" (result1), "m" (ld6)
            : "st", "st(1)"
        );
        
        ld9 = result2;
    }
    
    /* Example 4: Division with fixed register constraints */
    /* Division on x86 often requires specific registers */
    {
        int dividend = i9 = seed * 6 + 5;
        int divisor = i10 = seed + 7;
        int quotient, remainder;
        
        /* Use inline asm that requires rax/eax and rdx/edx */
        asm volatile (
            "movl %2, %%eax\n\t"
            "cltd\n\t"
            "idivl %3\n\t"
            "movl %%eax, %0\n\t"
            "movl %%edx, %1"
            : "=r" (quotient), "=r" (remainder)
            : "r" (dividend), "r" (divisor)
            : "eax", "edx"
        );
        
        i11 = quotient;
        i12 = remainder;
    }
    
    /* Mix everything together to increase register pressure */
    ld10 = ld6 + ld7 + ld8;
    ld11 = ld9 * ld10;
    
    /* Use CRC32 builtin which has fixed register constraints */
    {
        unsigned int crc = 0xFFFFFFFF;
        unsigned char data = (unsigned char)(seed & 0xFF);
        crc = __builtin_ia32_crc32qi(crc, data);
        i13 = (int)crc + i11;
    }
    
    /* More x87 operations to ensure reloads are needed */
    for (counter = 0; counter < 2; counter++) {
        long double temp;
        
        asm volatile (
            "fldt %1\n\t"
            "fsin\n\t"
            "fstpt %0"
            : "=m" (temp)
            : "m" (ld11)
            : "st"
        );
        
        ld12 = temp + (long double)counter;
        
        /* Another multi-alternative constraint test */
        {
            int scale = i13 + counter;
            asm volatile (
                "# Another multi-alternative\n\t"
                "fildl %2\n\t"
                "fscale\n\t"
                "fstp %%st(1)"
                : "=t" (ld13)
                : "0" (ld12), "rm,t" (scale)
                : "st(1)"
            );
        }
    }
    
    /* Store results to globals to prevent elimination */
    global_results[result_index++] = ld6;
    global_results[result_index++] = ld7;
    global_results[result_index++] = ld8;
    global_results[result_index++] = ld9;
    global_results[result_index++] = ld10;
    global_results[result_index++] = ld11;
    global_results[result_index++] = ld12;
    global_results[result_index++] = ld13;
    
    global_ints[0] = i11;
    global_ints[1] = i12;
    global_ints[2] = i13;
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call multiple times to increase coverage chances */
    for (int i = 0; i < 3; i++) {
        test_secondary_reloads(seed + i);
    }
    
    /* Compute a simple checksum from results */
    int checksum = 0;
    for (int i = 0; i < result_index && i < 20; i++) {
        checksum += (int)global_results[i];
    }
    for (int i = 0; i < 3; i++) {
        checksum += global_ints[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
