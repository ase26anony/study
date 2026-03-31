/* test_secondary_reloads.c
 * This program forces GCC to generate secondary reloads by:
 * 1. Using x87 long double operations with specific constraints
 * 2. Creating high register pressure
 * 3. Using inline assembly with multiple alternative constraints
 * 4. Employing builtins with fixed register requirements
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[20];
volatile int global_ints[20];

/* Prevent inlining to ensure reload happens in this function */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Create high register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8, ld9, ld10;
    volatile long double ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    
    /* Initialize with seed-dependent values to prevent constant folding */
    i1 = seed + 1;
    i2 = seed * 2;
    i3 = seed + 3;
    i4 = seed * 4;
    i5 = seed + 5;
    i6 = seed * 6;
    i7 = seed + 7;
    i8 = seed * 8;
    i9 = seed + 9;
    i10 = seed * 10;
    i11 = seed + 11;
    i12 = seed * 12;
    i13 = seed + 13;
    i14 = seed * 14;
    i15 = seed + 15;
    
    /* Initialize long doubles */
    ld1 = (long double)i1 / 3.0L;
    ld2 = (long double)i2 / 5.0L;
    ld3 = (long double)i3 / 7.0L;
    ld4 = (long double)i4 / 11.0L;
    ld5 = (long double)i5 / 13.0L;
    ld6 = (long double)i6 / 17.0L;
    ld7 = (long double)i7 / 19.0L;
    ld8 = (long double)i8 / 23.0L;
    ld9 = (long double)i9 / 29.0L;
    ld10 = (long double)i10 / 31.0L;
    ld11 = (long double)i11 / 37.0L;
    ld12 = (long double)i12 / 41.0L;
    ld13 = (long double)i13 / 43.0L;
    ld14 = (long double)i14 / 47.0L;
    ld15 = (long double)i15 / 53.0L;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    {
        unsigned long long ts1, ts2;
        ts1 = __builtin_ia32_rdtsc();
        /* Use the result to affect our variables */
        i1 ^= (int)(ts1 & 0xFFFFFFFF);
        ts2 = __builtin_ia32_rdtsc();
        i2 ^= (int)(ts2 & 0xFFFFFFFF);
    }
    
    /* Force x87 operations with specific constraints */
    /* Operation 1: Simple x87 addition with "t" constraint */
    asm volatile (
        "faddp %%st(1), %%st"
        : "=t" (ld1)
        : "0" (ld1), "t" (ld2)
        : "st(1)"
    );
    
    /* Operation 2: x87 multiplication with "u" constraint (second x87 reg) */
    asm volatile (
        "fmulp %%st(1), %%st"
        : "=t" (ld3)
        : "0" (ld3), "u" (ld4)
        : "st(1)"
    );
    
    /* Operation 3: Mixed operation with multiple alternative constraints
     * This is key for triggering secondary reloads */
    {
        volatile int temp_int = i3;
        asm volatile (
            "# Mixed operation with alternative constraints\n\t"
            "fildl %2\n\t"
            "faddp %%st(1), %%st"
            : "=t" (ld5)
            : "0" (ld5), "rm,t" (temp_int)
            : "st(1)"
        );
    }
    
    /* Operation 4: Another mixed operation with memory constraint */
    {
        volatile long double temp_ld = ld6;
        volatile int mem_int = i4;
        /* Force memory operand for integer */
        asm volatile (
            "# Force memory operand\n\t"
            "fildl %2\n\t"
            "fmulp %%st(1), %%st"
            : "=t" (ld7)
            : "0" (temp_ld), "m" (mem_int)
            : "st(1)"
        );
    }
    
    /* Operation 5: Complex chain of operations to increase pressure */
    {
        volatile long double a = ld8, b = ld9;
        volatile int c = i5;
        
        /* First load integer onto x87 stack */
        asm volatile (
            "fildl %2"
            : "=t" (a)
            : "0" (a), "rm" (c)
        );
        
        /* Then do x87 operation */
        asm volatile (
            "faddp %%st(1), %%st"
            : "=t" (b)
            : "0" (b), "t" (a)
            : "st(1)"
        );
        
        ld10 = b;
    }
    
    /* Use CRC32 builtin which has specific register constraints */
    {
        unsigned int crc = 0xFFFFFFFF;
        crc = __builtin_ia32_crc32qi(crc, (unsigned char)i6);
        i7 ^= crc;
    }
    
    /* More operations to keep variables live */
    for (volatile int loop = 0; loop < 3; loop++) {
        /* Chain of x87 operations */
        asm volatile (
            "fldt %1\n\t"
            "fldt %2\n\t"
            "faddp %%st(1), %%st\n\t"
            "fstpt %0"
            : "=m" (ld11)
            : "m" (ld12), "m" (ld13)
        );
        
        /* Integer operation that might interfere */
        i8 = i8 * 2 + loop;
    }
    
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
    for (int i = 0; i < 2; i++) {
        test_secondary_reloads(seed + i);
    }
    
    /* Print something to verify execution */
    printf("Results: %Lf, %d\n", global_results[0], global_ints[0]);
    
    return 0;
}
