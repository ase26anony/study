/* test_secondary_reloads.c
 * This program forces GCC to generate secondary reloads for x87 floating-point
 * operations, covering the initialization of secondary_* fields in struct reload.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[20];
volatile int global_ints[20];
volatile int result_index = 0;

/* Prevent optimization of the key function */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Create high register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8, ld9, ld10;
    volatile long double ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    
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
    i14 = ~seed;
    i15 = seed + 0xABCD;
    
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
    
    /* Force use of RDTSC which uses fixed registers (eax, edx) */
    {
        uint64_t tsc1, tsc2;
        tsc1 = __builtin_ia32_rdtsc();
        /* Use the result to prevent elimination */
        i1 = (int)(tsc1 & 0xFFFFFFFF);
        tsc2 = __builtin_ia32_rdtsc();
        i2 = (int)(tsc2 & 0xFFFFFFFF);
    }
    
    /* CRITICAL PART 1: x87 operations with 't' and 'u' constraints
     * 't' = top of x87 stack (st(0))
     * 'u' = second x87 register (st(1))
     * These constraints force secondary reloads for non-x87 operands
     */
    
    /* x87 addition with both operands in x87 registers */
    asm volatile (
        "faddp %%st, %%st(1)\n\t"
        : "=t"(ld1)
        : "0"(ld1), "u"(ld2)
        : "st(1)"
    );
    
    /* x87 multiplication */
    asm volatile (
        "fmulp %%st, %%st(1)\n\t"
        : "=t"(ld3)
        : "0"(ld3), "u"(ld4)
        : "st(1)"
    );
    
    /* CRITICAL PART 2: Mixed constraints with alternatives
     * "rm,t" means either memory/register OR x87 top-of-stack
     * This may trigger secondary reload setup when choosing 't' alternative
     */
    {
        volatile long double ld_tmp = ld5;
        volatile int int_val = i3;
        
        /* This asm has multiple alternatives for the third operand.
         * The 't' constraint (x87 top) will require secondary reloads
         * for the integer value if it needs to be in st(0).
         */
        asm volatile (
            "fildl %2\n\t"
            "faddp %%st, %%st(1)\n\t"
            : "=t"(ld_tmp)
            : "0"(ld_tmp), "rm,t"((int)int_val)
        );
        ld5 = ld_tmp;
    }
    
    /* More complex: x87 operation with memory input that might need
     * secondary reload to get into x87 register */
    {
        volatile long double src = ld6;
        volatile long double result;
        
        /* Using 't' constraint for output and input, but the
         * memory operand might need special handling */
        asm volatile (
            "fldt %1\n\t"      /* Load long double from memory */
            "fsqrt\n\t"        /* Square root */
            "fstpt %0\n\t"     /* Store back */
            : "=m"(result)
            : "m"(src)
            : "st"
        );
        ld6 = result;
    }
    
    /* CRITICAL PART 3: Chain operations to increase register pressure
     * and force spill/reload decisions */
    {
        volatile long double a = ld7, b = ld8, c = ld9;
        
        /* Multiple x87 operations in sequence */
        asm volatile (
            "fldt %1\n\t"      /* Load a */
            "fldt %2\n\t"      /* Load b */
            "faddp %%st, %%st(1)\n\t"
            "fldt %3\n\t"      /* Load c */
            "fmulp %%st, %%st(1)\n\t"
            "fstpt %0\n\t"
            : "=m"(a)
            : "m"(a), "m"(b), "m"(c)
            : "st", "st(1)"
        );
        ld7 = a;
    }
    
    /* Use CRC32 builtin which has fixed register constraints */
    {
        unsigned int crc = 0xFFFFFFFF;
        unsigned char data = (unsigned char)seed;
        crc = __builtin_ia32_crc32qi(crc, data);
        i4 = (int)crc;
    }
    
    /* More register pressure: convert between int and long double */
    for (volatile int j = 0; j < 3; j++) {
        /* This conversion may need x87 instructions */
        ld10 = ld10 + (long double)i5;
        i5 = i5 + (int)ld11;
        
        /* Another asm with mixed constraints */
        {
            volatile long double tmp = ld12;
            asm volatile (
                "fildl %1\n\t"
                "faddp %%st, %%st(1)\n\t"
                : "+t"(tmp)
                : "rm"(i6)
            );
            ld12 = tmp;
        }
    }
    
    /* Store results to prevent elimination */
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
    
    global_ints[0] = i1;
    global_ints[1] = i2;
    global_ints[2] = i3;
    global_ints[3] = i4;
    global_ints[4] = i5;
    global_ints[5] = i6;
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call multiple times to ensure execution */
    for (int i = 0; i < 2; i++) {
        test_secondary_reloads(seed + i);
    }
    
    /* Compute a simple checksum to use results */
    int checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum += (int)global_results[i];
    }
    for (int i = 0; i < 6; i++) {
        checksum += global_ints[i];
    }
    
    printf("Result checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
