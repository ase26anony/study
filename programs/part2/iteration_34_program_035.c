/* reload_coverage.c - Test program to cover secondary reload initialization in GCC's reload pass */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[20];
volatile int global_ints[20];
volatile int result_index = 0;

/* Prevent inlining to ensure reload logic is exercised */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* High register pressure: many volatile variables to force spills/reloads */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    volatile long double ld9, ld10, ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15, i16, i17, i18, i19, i20;
    volatile unsigned int cycles_low, cycles_high;
    volatile unsigned long long timestamp;
    
    /* Initialize with seed-dependent values to avoid constant propagation */
    i1 = seed + 1;
    i2 = seed * 2 + 3;
    i3 = seed ^ 0x1234;
    i4 = seed * seed;
    i5 = seed % 100;
    i6 = seed + 1000;
    i7 = seed | 0xFF00;
    i8 = seed << 3;
    i9 = seed >> 2;
    i10 = ~seed;
    
    /* Initialize long double values using integer conversions */
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
    
    /* More variables for additional pressure */
    ld11 = ld1 + ld2;
    ld12 = ld3 * ld4;
    ld13 = ld5 / ld6;
    ld14 = ld7 - ld8;
    ld15 = ld9 * ld10;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    __asm__ volatile ("rdtsc" : "=a" (cycles_low), "=d" (cycles_high));
    timestamp = ((unsigned long long)cycles_high << 32) | cycles_low;
    i11 = (int)(timestamp % 1000000);
    
    /* CRITICAL: Force secondary reloads by using x87 constraints */
    /* This asm uses "t" constraint (top of x87 stack) which requires
       secondary reloads to move values into/out of x87 registers */
    
    /* Operation 1: x87 addition with both operands in x87 registers */
    __asm__ volatile (
        "faddp %%st(1), %%st"  /* st(1) = st(1) + st(0), pop stack */
        : "=t" (ld1)           /* output in top of x87 stack */
        : "0" (ld1),           /* input in same register as output */
          "u" (ld2)            /* input in second x87 register (st(1)) */
        : "st(1)"              /* clobber second x87 register */
    );
    
    /* Operation 2: Mixed constraints - "rm,t" alternative for integer */
    /* This may force secondary reload for integer operand */
    i12 = i1 + i2;
    __asm__ volatile (
        "fildl %2\n\t"         /* load integer into x87 stack */
        "faddp %%st(1), %%st"
        : "=t" (ld3)
        : "0" (ld3),
          "rm,t" (i12)         /* Alternative: memory/register OR x87 top */
        : "st(1)"
    );
    
    /* Operation 3: x87 multiplication with memory operand */
    __asm__ volatile (
        "fmull %2\n\t"         /* multiply st(0) with memory operand */
        : "=t" (ld4)
        : "0" (ld4),
          "m" (ld5)            /* Memory constraint - may need reload */
    );
    
    /* Operation 4: Complex pattern with multiple alternatives */
    /* One alternative uses x87, another uses general registers */
    i13 = i3 * i4;
    __asm__ volatile (
        "faddp %%st(1), %%st"
        : "=&t" (ld6)          /* earlyclobber x87 top */
        : "%0" (ld6),          /* matching constraint */
          "gt" (ld7)           /* general or x87 top constraint */
        : "st(1)"
    );
    
    /* Operation 5: Use CRC32 builtin which has fixed register constraints */
    /* This creates additional register pressure and special requirements */
    i14 = __builtin_ia32_crc32qi(i5, (unsigned char)i6);
    
    /* More operations to increase pressure and force reload decisions */
    for (volatile int j = 0; j < 3; j++) {
        /* x87 operations in loop to prevent optimization */
        __asm__ volatile (
            "faddp %%st(1), %%st"
            : "=t" (ld8)
            : "0" (ld8),
              "u" (ld9)
            : "st(1)"
        );
        
        /* Integer operations mixing with x87 results */
        i15 = (int)ld8 + i7;
        
        /* Convert x87 to integer using fistp - requires x87 stack */
        __asm__ volatile (
            "fistpl %0\n\t"
            : "=m" (i16)
            : "t" (ld10)
            :
        );
        
        /* Reload long double from integer */
        __asm__ volatile (
            "fildl %1\n\t"
            : "=t" (ld10)
            : "m" (i16)
        );
    }
    
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
    
    global_ints[result_index % 20] = i11 + i12 + i13 + i14 + i15 + i16;
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call multiple times to increase coverage chances */
    for (int i = 0; i < 5; i++) {
        test_secondary_reloads(seed + i);
    }
    
    /* Compute and print a checksum to ensure code executed */
    long double sum = 0.0L;
    for (int i = 0; i < result_index && i < 20; i++) {
        sum += global_results[i];
    }
    
    printf("Result checksum: %Lf\n", sum);
    printf("Test completed with seed %d\n", seed);
    
    return 0;
}
