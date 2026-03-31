/* test_secondary_reloads.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[20];
volatile int global_ints[20];
volatile int result_index = 0;

/* Prevent inlining to ensure reload logic is exercised */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Create high register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    volatile long double ld9, ld10, ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    volatile uint64_t tsc1, tsc2;
    
    /* Initialize with seed-dependent values to avoid constant propagation */
    i1 = seed * 1;
    i2 = seed * 2 + 1;
    i3 = seed * 3 + 2;
    i4 = seed * 4 + 3;
    i5 = seed * 5 + 4;
    
    /* Initialize long doubles using integer conversions */
    ld1 = (long double)i1 / 100.0L;
    ld2 = (long double)i2 / 100.0L;
    ld3 = (long double)i3 / 100.0L;
    ld4 = (long double)i4 / 100.0L;
    ld5 = (long double)i5 / 100.0L;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    tsc1 = __builtin_ia32_rdtsc();
    i6 = (int)(tsc1 & 0xFFFFFFFF);
    
    /* Force x87 operations with explicit register constraints */
    /* This asm uses "t" constraint (top of x87 stack) */
    asm volatile (
        "fldt %1\n\t"           /* load ld2 onto x87 stack */
        "fldt %2\n\t"           /* load ld3 onto x87 stack */
        "faddp %%st(1), %%st\n\t" /* st(1) = st(1) + st, pop */
        "fstpt %0"
        : "=m" (ld6)
        : "m" (ld2), "m" (ld3)
        : "st", "st(1)"
    );
    
    /* More x87 operations mixing with integer values */
    ld7 = ld1 + ld6;
    
    /* Complex asm with multiple alternative constraints */
    /* The "rm,t" constraint may force secondary reload for integer operand */
    asm volatile (
        "fildl %2\n\t"          /* load integer to x87 stack */
        "fldt %1\n\t"           /* load long double */
        "faddp %%st(1), %%st\n\t"
        "fstpt %0"
        : "=m" (ld8)
        : "0" (ld7), "rm,t" (i6)  /* i6 has alternative constraints */
        : "st", "st(1)"
    );
    
    /* Another asm using "u" constraint (second x87 register) */
    asm volatile (
        "fldt %2\n\t"           /* ld4 -> st(0) */
        "fldt %1\n\t"           /* ld5 -> st(0), ld4 -> st(1) */
        "fmulp %%st(1), %%st\n\t" /* st(1) = st(1) * st, pop */
        "fstpt %0"
        : "=m" (ld9)
        : "u" (ld4), "t" (ld5)    /* u=2nd x87 reg, t=top x87 reg */
        : "st", "st(1)"
    );
    
    /* Use CRC32 builtin which has fixed register constraints */
    i7 = __builtin_ia32_crc32qi(i1, (unsigned char)i2);
    i8 = __builtin_ia32_crc32qi(i7, (unsigned char)i3);
    
    /* More register pressure with conversions */
    ld10 = (long double)i7 / 1000.0L;
    ld11 = (long double)i8 / 1000.0L;
    
    /* Another complex asm with output in x87 register */
    long double temp_result;
    asm volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "fdivrp %%st(1), %%st\n\t"
        : "=t" (temp_result)
        : "t" (ld9), "u" (ld10)
    );
    ld12 = temp_result;
    
    /* Mix everything together */
    tsc2 = __builtin_ia32_rdtsc();
    i9 = (int)(tsc2 - tsc1);
    
    /* Final calculation using all variables */
    ld13 = ld8 + ld12;
    ld14 = ld13 * (long double)i9;
    
    /* Store results to globals to prevent elimination */
    global_results[result_index++] = ld1;
    global_results[result_index++] = ld6;
    global_results[result_index++] = ld8;
    global_results[result_index++] = ld9;
    global_results[result_index++] = ld12;
    global_results[result_index++] = ld14;
    
    global_ints[0] = i6;
    global_ints[1] = i7;
    global_ints[2] = i8;
    global_ints[3] = i9;
}

/* Wrapper to create loop with volatile counter */
__attribute__((noinline))
void stress_test(int iterations, int base_seed) {
    volatile int counter = 0;
    for (int i = 0; i < iterations; i++) {
        test_secondary_reloads(base_seed + i);
        counter++;  /* volatile to prevent loop optimization */
    }
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Run multiple iterations to increase coverage chances */
    stress_test(5, seed);
    
    /* Compute and print a checksum to ensure code runs */
    long double sum = 0.0L;
    for (int i = 0; i < result_index && i < 20; i++) {
        sum += global_results[i];
    }
    
    printf("Result checksum: %Lf\n", sum);
    printf("Integer checksum: %d\n", 
           global_ints[0] + global_ints[1] + global_ints[2] + global_ints[3]);
    
    return 0;
}
