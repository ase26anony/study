/* test_secondary_reloads.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[20];
volatile int global_ints[20];
volatile int result_index = 0;

/* Prevent inlining to ensure complex reload scenarios */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Create many volatile variables to increase register pressure */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    volatile long double ld9, ld10, ld11, ld12;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    
    /* Initialize with seed-dependent values to prevent constant folding */
    i1 = seed + 1;
    i2 = seed * 2 + 3;
    i3 = seed ^ 0x1234;
    i4 = seed - 100;
    i5 = seed * seed;
    i6 = seed / 3;
    i7 = seed % 7;
    i8 = seed | 0xFF00;
    i9 = seed & 0x0F0F;
    i10 = ~seed;
    i11 = seed << 2;
    i12 = seed >> 1;
    i13 = seed + 0xABCD;
    i14 = seed * 3 + 5;
    i15 = seed ^ 0xDEADBEEF;
    
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
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    {
        unsigned long long tsc1, tsc2;
        tsc1 = __builtin_ia32_rdtsc();
        i1 = (int)(tsc1 & 0xFFFFFFFF);
        tsc2 = __builtin_ia32_rdtsc();
        i2 = (int)(tsc2 & 0xFFFFFFFF);
    }
    
    /* Force x87 operations with explicit register constraints */
    /* Using "t" constraint (top of x87 stack) and "u" (second x87 register) */
    
    /* Example 1: Simple x87 operation */
    asm volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "faddp %%st(1), %%st\n\t"
        "fstpt %0"
        : "=m" (ld1)
        : "m" (ld2), "m" (ld3)
        : "st", "st(1)"
    );
    
    /* Example 2: Mixed integer/long double with multi-alternative constraint */
    /* This is key for triggering secondary reloads */
    {
        long double result;
        int int_val = i4;
        long double ld_val = ld4;
        
        /* Constraint "rm,t" - either memory/general register OR x87 top */
        /* The compiler may choose "t" alternative, requiring secondary reload */
        asm volatile (
            "fldt %1\n\t"
            "fildl %2\n\t"
            "faddp %%st(1), %%st\n\t"
            "fstpt %0"
            : "=m" (result)
            : "m" (ld_val), "rm,t" (int_val)
            : "st", "st(1)"
        );
        ld5 = result;
    }
    
    /* Example 3: More complex with multiple x87 operations */
    {
        long double temp1, temp2;
        
        /* First load two values */
        asm volatile (
            "fldt %1\n\t"
            "fldt %2\n\t"
            "fmulp %%st(1), %%st\n\t"
            "fstpt %0"
            : "=m" (temp1)
            : "m" (ld6), "m" (ld7)
            : "st", "st(1)"
        );
        
        /* Then use with integer from general register */
        asm volatile (
            "fldt %1\n\t"
            "fildl %2\n\t"
            "fdivrp %%st(1), %%st\n\t"
            "fstpt %0"
            : "=m" (temp2)
            : "m" (temp1), "r" (i5)
            : "st", "st(1)"
        );
        ld8 = temp2;
    }
    
    /* Example 4: CRC32 builtin which uses fixed register (eax for input/output) */
    {
        unsigned int crc = 0xFFFFFFFF;
        unsigned char data = (unsigned char)seed;
        crc = __builtin_ia32_crc32qi(crc, data);
        i6 = (int)crc;
    }
    
    /* Example 5: Chain x87 operations with intermediate values in memory */
    for (volatile int j = 0; j < 3; j++) {
        long double chain_result;
        
        asm volatile (
            "fldt %1\n\t"
            "fldt %2\n\t"
            "faddp %%st(1), %%st\n\t"
            "fldt %3\n\t"
            "fmulp %%st(1), %%st\n\t"
            "fstpt %0"
            : "=m" (chain_result)
            : "m" (ld9), "m" (ld10), "m" (ld11)
            : "st", "st(1)", "st(2)"
        );
        
        ld12 = chain_result + (long double)j;
    }
    
    /* Store results to global arrays to prevent elimination */
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
    global_ints[6] = i7;
    global_ints[7] = i8;
    global_ints[8] = i9;
    global_ints[9] = i10;
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call multiple times with different seeds */
    for (int i = 0; i < 5; i++) {
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
