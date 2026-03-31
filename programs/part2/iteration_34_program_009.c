/* test_secondary_reloads.c */
#include <stdint.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[20];
volatile int global_ints[20];
volatile int result_index = 0;

/* Prevent inlining to ensure complex reload scenarios */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Create high register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    volatile long double ld9, ld10, ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    
    /* Initialize with seed-dependent values to prevent constant folding */
    i1 = seed + 1;
    i2 = seed * 2 + 3;
    i3 = seed ^ 0x1234;
    i4 = seed * seed;
    i5 = seed - 100;
    i6 = seed + 200;
    i7 = seed | 0xFF00;
    i8 = seed & 0x00FF;
    i9 = seed << 3;
    i10 = seed >> 2;
    i11 = ~seed;
    i12 = seed * 3;
    i13 = seed + 500;
    i14 = seed * 7;
    i15 = seed - 300;
    
    /* Initialize long doubles with conversions from integers */
    ld1 = (long double)i1 + 0.1L;
    ld2 = (long double)i2 + 0.2L;
    ld3 = (long double)i3 + 0.3L;
    ld4 = (long double)i4 + 0.4L;
    ld5 = (long double)i5 + 0.5L;
    ld6 = (long double)i6 + 0.6L;
    ld7 = (long double)i7 + 0.7L;
    ld8 = (long double)i8 + 0.8L;
    ld9 = (long double)i9 + 0.9L;
    ld10 = (long double)i10 + 1.0L;
    ld11 = (long double)i11 + 1.1L;
    ld12 = (long double)i12 + 1.2L;
    ld13 = (long double)i13 + 1.3L;
    ld14 = (long double)i14 + 1.4L;
    ld15 = (long double)i15 + 1.5L;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    {
        uint64_t tsc1, tsc2;
        tsc1 = __builtin_ia32_rdtsc();
        /* Use the result to influence other variables */
        i1 = (int)(tsc1 & 0xFFFFFFFF);
        tsc2 = __builtin_ia32_rdtsc();
        i2 = (int)(tsc2 & 0xFFFFFFFF);
    }
    
    /* Force x87 register usage with inline assembly */
    /* Using "t" constraint (top of x87 stack) and "u" constraint (second x87 register) */
    
    /* Example 1: Simple x87 operation forcing secondary reloads */
    asm volatile (
        "fldt %1\n\t"           /* load ld1 onto x87 stack */
        "fldt %2\n\t"           /* load ld2 onto x87 stack */
        "faddp %%st, %%st(1)\n\t" /* st(1) = st(1) + st, pop st */
        "fstpt %0"
        : "=m" (ld3)
        : "m" (ld1), "m" (ld2)
        : "st", "st(1)"
    );
    
    /* Example 2: Mixed integer and x87 operation with multiple alternatives */
    /* This may trigger secondary reload for integer operand */
    {
        long double result;
        int int_val = i3;
        long double ld_val = ld4;
        
        asm volatile (
            "fldt %2\n\t"
            "fildl %1\n\t"      /* load integer into x87 stack */
            "faddp %%st, %%st(1)\n\t"
            "fstpt %0"
            : "=m" (result)
            : "rm" (int_val),   /* "rm" allows register or memory, may need secondary reload */
              "m" (ld_val)
            : "st", "st(1)"
        );
        ld5 = result;
    }
    
    /* Example 3: Complex inline assembly with multiple output constraints */
    /* Using both "t" and "=t" constraints */
    {
        long double temp1, temp2;
        
        asm volatile (
            "fldt %2\n\t"
            "fldt %3\n\t"
            "fmulp %%st, %%st(1)\n\t"
            "fstpt %0\n\t"
            "fldt %4\n\t"
            "fsqrt\n\t"
            "fstpt %1"
            : "=m" (temp1), "=m" (temp2)
            : "m" (ld6), "m" (ld7), "m" (ld8)
            : "st", "st(1)"
        );
        ld9 = temp1;
        ld10 = temp2;
    }
    
    /* Example 4: CRC32 builtin which has fixed register constraints */
    /* This creates additional register pressure */
    {
        unsigned int crc = 0xFFFFFFFF;
        crc = __builtin_ia32_crc32qi(crc, (unsigned char)i4);
        crc = __builtin_ia32_crc32hi(crc, (unsigned short)i5);
        crc = __builtin_ia32_crc32si(crc, (unsigned int)i6);
        i7 = (int)crc;
    }
    
    /* Example 5: Multi-alternative constraint that may force secondary reload */
    /* "rm,t" constraint - compiler may choose "t" alternative requiring x87 */
    {
        long double out_val;
        long double in_val = ld11;
        int alt_val = i8;
        
        asm volatile (
            "fldt %1\n\t"
            /* The tricky part: integer operand with dual constraint */
            "fildl %k2\n\t"     /* Use %k2 for 32-bit register name */
            "faddp %%st, %%st(1)\n\t"
            "fstpt %0"
            : "=m" (out_val)
            : "m" (in_val),
              "rm,t" (alt_val)   /* Dual constraint: register/memory OR x87 top */
            : "st", "st(1)"
        );
        ld12 = out_val;
    }
    
    /* More x87 operations to increase pressure */
    asm volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "fdivrp %%st, %%st(1)\n\t"
        "fstpt %0"
        : "=m" (ld13)
        : "m" (ld13), "m" (ld14)
        : "st", "st(1)"
    );
    
    /* Store results to global array to prevent elimination */
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
    
    /* Call multiple times to ensure execution */
    for (volatile int i = 0; i < 3; i++) {
        test_secondary_reloads(seed + i);
    }
    
    /* Compute a simple checksum to use results */
    int checksum = 0;
    for (int i = 0; i < 15; i++) {
        checksum += global_ints[i];
        checksum ^= (int)global_results[i];
    }
    
    return checksum & 0xFF;  /* Return non-zero, non-constant value */
}
