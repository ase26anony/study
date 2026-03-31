/* test_secondary_reloads.c
 * Designed to trigger uncovered lines in GCC's reload.cc
 * Specifically targets initialization of secondary_in_reload, secondary_out_reload,
 * secondary_in_icode, and secondary_out_icode fields in struct reload
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[20];
volatile int global_ints[20];

/* Prevent inlining to ensure reload pass runs on this function */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Create high register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8, ld9, ld10;
    volatile long double ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    volatile unsigned int cycles_low, cycles_high;
    volatile unsigned long long timestamp;
    
    /* Initialize with seed-dependent values to prevent constant propagation */
    i1 = seed + 1;
    i2 = seed * 2 + 3;
    i3 = seed ^ 0x1234;
    i4 = seed * seed;
    i5 = seed + 1000;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    __asm__ volatile ("rdtsc" : "=a" (cycles_low), "=d" (cycles_high));
    timestamp = ((unsigned long long)cycles_high << 32) | cycles_low;
    
    i6 = (int)(timestamp & 0xFFFFFFFF);
    i7 = (int)(timestamp >> 32);
    
    /* Initialize long double values using integer conversions */
    ld1 = (long double)i1 + 0.1L;
    ld2 = (long double)i2 + 0.2L;
    ld3 = (long double)i3 + 0.3L;
    ld4 = (long double)i4 + 0.4L;
    ld5 = (long double)i5 + 0.5L;
    
    /* Force x87 operations with explicit register constraints */
    /* This asm uses "t" constraint (top of x87 stack) and "u" (second x87 register) */
    __asm__ volatile (
        "fldt %2\n\t"           /* Load ld2 onto x87 stack */
        "fldt %1\n\t"           /* Load ld1 onto x87 stack */
        "faddp %%st, %%st(1)\n\t" /* Add st(0) to st(1) and pop */
        "fstpt %0"              /* Store result */
        : "=m" (ld6)
        : "m" (ld1), "m" (ld2)
        : "st", "st(1)"
    );
    
    /* More x87 operations mixing with integer values */
    ld7 = ld3 * ld4;
    
    /* Critical asm with multi-alternative constraint to potentially force secondary reload */
    /* The "rm,t" constraint gives alternatives: register/memory OR x87 top register */
    __asm__ volatile (
        "fldt %1\n\t"           /* Load input long double */
        "fildl %2\n\t"          /* Load integer - may need secondary reload */
        "faddp\n\t"             /* Add */
        "fstpt %0"              /* Store result */
        : "=m" (ld8)
        : "m" (ld5), "rm,t" (i6)  /* i6 has alternative constraints */
        : "st"
    );
    
    /* Another complex pattern with output in x87 register */
    __asm__ volatile (
        "fldt %1\n\t"
        "fsqrt\n\t"
        : "=t" (ld9)            /* Output in x87 top register */
        : "m" (ld6)
        : "st"
    );
    
    /* Mix x87 and MMX/SSE constraints (MMX needs secondary reload on x86-64) */
    /* Use division which requires specific registers */
    i8 = i7 / (i1 + 1);
    
    /* CRC32 builtin with fixed register constraints */
    i9 = __builtin_ia32_crc32qi(i8, (unsigned char)i2);
    
    /* More operations to increase register pressure */
    for (volatile int j = 0; j < 3; j++) {
        ld10 = ld7 + ld8 + (long double)j;
        ld11 = ld9 * ld10;
        
        /* Another asm with x87 constraints */
        __asm__ volatile (
            "fldt %1\n\t"
            "fldt %2\n\t"
            "fmulp %%st, %%st(1)\n\t"
            "fchs\n\t"          /* Change sign */
            "fstpt %0"
            : "=m" (ld12)
            : "m" (ld11), "m" (ld5)
            : "st", "st(1)"
        );
        
        i10 = i9 ^ j;
        ld13 = (long double)i10 / 100.0L;
    }
    
    /* Final calculations mixing everything */
    ld14 = ld12 + ld13;
    
    /* One more asm with both input and output in x87 registers */
    long double temp;
    __asm__ volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "fsubrp %%st, %%st(1)\n\t"
        : "=t" (temp)
        : "m" (ld14), "m" (ld8)
        : "st"
    );
    
    ld15 = temp * 2.0L;
    
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
    global_results[11] = ld12;
    global_results[12] = ld13;
    global_results[13] = ld14;
    global_results[14] = ld15;
    
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
    
    printf("Testing secondary reloads with seed = %d\n", seed);
    
    /* Call multiple times to ensure execution */
    for (int i = 0; i < 2; i++) {
        test_secondary_reloads(seed + i);
    }
    
    /* Compute and print a simple checksum */
    int checksum = 0;
    for (int i = 0; i < 15; i++) {
        checksum += global_ints[i];
        checksum += (int)global_results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
