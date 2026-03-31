/* test_secondary_reloads.c
 * Designed to trigger uncovered lines in GCC's reload.cc (lines 1381-1399)
 * Specifically targets initialization of secondary_in_reload, secondary_out_reload,
 * secondary_in_icode, and secondary_out_icode fields in struct reload.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[32];
volatile int global_ints[32];
volatile int result_index = 0;

/* Prevent inlining to ensure reload happens in this function */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Create high register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    volatile long double ld9, ld10, ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15, i16, i17, i18, i19, i20;
    
    /* Initialize with seed to create non-constant values */
    i1 = seed;
    i2 = seed * 2 + 1;
    i3 = seed * 3 + 2;
    i4 = seed * 5 + 3;
    i5 = seed * 7 + 4;
    
    /* Initialize long doubles using integer values */
    ld1 = (long double)i1 / 100.0L;
    ld2 = (long double)i2 / 100.0L;
    ld3 = (long double)i3 / 100.0L;
    ld4 = (long double)i4 / 100.0L;
    ld5 = (long double)i5 / 100.0L;
    
    /* More initialization to increase register pressure */
    i6 = i1 + i2;
    i7 = i3 + i4;
    i8 = i5 * 2;
    i9 = i6 - i7;
    i10 = i8 + seed;
    
    ld6 = ld1 + ld2;
    ld7 = ld3 - ld4;
    ld8 = ld5 * 2.0L;
    ld9 = ld6 / 1.5L;
    ld10 = ld7 * ld8;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    {
        uint64_t tsc1, tsc2;
        tsc1 = __builtin_ia32_rdtsc();
        i11 = (int)(tsc1 & 0xFFFFFFFF);
        i12 = (int)(tsc1 >> 32);
        
        /* Create some computation between RDTSC calls */
        ld11 = (long double)i11 / 1000000.0L;
        ld12 = (long double)i12 / 1000000.0L;
        
        tsc2 = __builtin_ia32_rdtsc();
        i13 = (int)((tsc2 - tsc1) & 0xFFFFFFFF);
    }
    
    /* CRITICAL PART 1: Inline asm with x87 constraints that may need secondary reloads */
    /* Using "t" constraint (top of x87 stack) and "u" constraint (second x87 register) */
    {
        long double temp1, temp2, temp3;
        
        /* Load values into x87 stack */
        asm volatile ("fldt %1\n\t"
                      "fldt %2\n\t"
                      "faddp %%st(1), %%st\n\t"
                      "fstpt %0"
                      : "=m" (temp1)
                      : "m" (ld1), "m" (ld2)
                      : "st", "st(1)");
        
        /* Complex pattern: multiple alternatives with mixed constraints */
        /* This may trigger secondary reload for the integer operand */
        asm volatile ("fldt %2\n\t"          /* Load ld3 into st(0) */
                      "movl %3, %%eax\n\t"   /* Move i3 into eax */
                      "pushl %%eax\n\t"      /* Push to stack */
                      "fildl (%%esp)\n\t"    /* Load integer into x87 */
                      "addl $4, %%esp\n\t"   /* Clean up stack */
                      "fmulp %%st(1), %%st\n\t"
                      "fstpt %0\n\t"
                      "fldt %1\n\t"          /* Another x87 operation */
                      "fsin\n\t"
                      "fstpt %4"
                      : "=m" (temp2), "=m" (ld13)
                      : "m" (ld3), "r" (i3), "m" (ld4)
                      : "eax", "st", "st(1)", "memory");
        
        /* Store intermediate results */
        ld14 = temp1;
        ld15 = temp2;
    }
    
    /* CRITICAL PART 2: Multi-alternative constraint that may force secondary reload */
    /* The "rm,t" constraint gives two alternatives: general reg/mem OR x87 top */
    {
        long double result;
        int int_val = i4 + i5;
        
        /* This asm has alternatives: if compiler chooses "t" for int_val,
           it will need a secondary reload to get an integer into x87 stack */
        asm volatile ("fldt %1\n\t"
                      "movl %2, %%eax\n\t"
                      "pushl %%eax\n\t"
                      "fildl (%%esp)\n\t"
                      "addl $4, %%esp\n\t"
                      "faddp %%st(1), %%st\n\t"
                      "fstpt %0"
                      : "=m" (result)
                      : "m" (ld5), "rm,t" (int_val)
                      : "eax", "st", "st(1)", "memory");
        
        ld8 = result;
    }
    
    /* More operations to increase complexity */
    for (volatile int loop = 0; loop < 3; loop++) {
        /* CRC32 builtin with fixed register constraints */
        i14 = __builtin_ia32_crc32qi(i6, (unsigned char)i7);
        i15 = __builtin_ia32_crc32qi(i14, (unsigned char)i8);
        
        /* Mix x87 and general registers */
        asm volatile ("fldt %1\n\t"
                      "fldt %2\n\t"
                      "fmulp %%st(1), %%st\n\t"
                      "fistpl %0"
                      : "=m" (i16)
                      : "m" (ld6), "m" (ld7)
                      : "st", "st(1)", "memory");
        
        /* Another multi-alternative constraint */
        {
            long double src = ld9;
            long double dst;
            int opt_int = i9;
            
            asm volatile ("fldt %1\n\t"
                          "movl %2, %%ecx\n\t"
                          "pushl %%ecx\n\t"
                          "fildl (%%esp)\n\t"
                          "addl $4, %%esp\n\t"
                          "fdivrp %%st(1), %%st\n\t"
                          "fstpt %0"
                          : "=m" (dst)
                          : "m" (src), "rm,t" (opt_int)
                          : "ecx", "st", "st(1)", "memory");
            
            ld10 = dst;
        }
    }
    
    /* Store results to globals to prevent elimination */
    global_results[result_index++] = ld1;
    global_results[result_index++] = ld2;
    global_results[result_index++] = ld3;
    global_results[result_index++] = ld8;
    global_results[result_index++] = ld10;
    global_results[result_index++] = ld13;
    global_results[result_index++] = ld14;
    global_results[result_index++] = ld15;
    
    global_ints[0] = i11;
    global_ints[1] = i12;
    global_ints[2] = i13;
    global_ints[3] = i14;
    global_ints[4] = i15;
    global_ints[5] = i16;
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
    for (int i = 0; i < result_index && i < 32; i++) {
        checksum += (int)global_results[i];
    }
    for (int i = 0; i < 6; i++) {
        checksum += global_ints[i];
    }
    
    printf("Result checksum: %d\n", checksum);
    return 0;
}
