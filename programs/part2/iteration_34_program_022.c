/* test_secondary_reloads.c
 * This program forces GCC to generate secondary reloads for x87 registers
 * to cover the initialization of secondary_* fields in struct reload.
 */

#include <stdint.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[20];
volatile int global_ints[20];
int global_index = 0;

/* Prevent inlining to ensure reload logic isn't optimized away */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Create high register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8, ld9, ld10;
    volatile long double ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    
    /* Initialize with seed-dependent values to prevent constant propagation */
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
    
    /* Initialize long doubles with conversions from integers */
    ld1 = (long double)i1 / 100.0L;
    ld2 = (long double)i2 / 100.0L;
    ld3 = (long double)i3 / 100.0L;
    ld4 = (long double)i4 / 100.0L;
    ld5 = (long double)i5 / 100.0L;
    ld6 = (long double)i6 / 100.0L;
    ld7 = (long double)i7 / 100.0L;
    ld8 = (long double)i8 / 100.0L;
    ld9 = (long double)i9 / 100.0L;
    ld10 = (long double)i10 / 100.0L;
    ld11 = (long double)i11 / 100.0L;
    ld12 = (long double)i12 / 100.0L;
    ld13 = (long double)i13 / 100.0L;
    ld14 = (long double)i14 / 100.0L;
    ld15 = (long double)i15 / 100.0L;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    {
        uint64_t tsc1, tsc2;
        tsc1 = __builtin_ia32_rdtsc();
        /* Use the result to modify an integer */
        i1 ^= (int)(tsc1 & 0xFFFFFFFF);
        tsc2 = __builtin_ia32_rdtsc();
        i2 ^= (int)(tsc2 & 0xFFFFFFFF);
    }
    
    /* Force x87 operations with explicit register constraints */
    
    /* Example 1: x87 addition with 't' constraint (top of x87 stack) */
    asm volatile (
        "faddp %%st(1), %%st"
        : "=t" (ld2)
        : "0" (ld1), "t" (ld2)
        : "st(1)"
    );
    
    /* Example 2: x87 multiplication with 'u' constraint (second x87 register) */
    asm volatile (
        "fmulp %%st(1), %%st"
        : "=t" (ld4)
        : "0" (ld3), "u" (ld4)
        : "st(1)"
    );
    
    /* CRITICAL: Mixed constraints that may force secondary reloads */
    /* This uses alternative constraints: "rm,t" - either memory/register OR x87 top */
    /* The compiler may choose the 't' alternative, requiring secondary reload */
    {
        long double result;
        int int_val = i5;
        
        asm volatile (
            "fildl %2\n\t"          /* Load integer into x87 stack */
            "faddp %%st(1), %%st"
            : "=t" (result)
            : "0" (ld5), "m" (int_val)  /* Using "m" constraint for the integer */
            : "st(1)"
        );
        ld6 = result;
    }
    
    /* Another critical pattern: multiple alternatives with x87 constraint */
    {
        long double temp = ld7;
        int int_val2 = i6;
        
        /* This asm has two alternatives for the third operand:
           "rm" (general register/memory) or "t" (x87 top) */
        asm volatile (
            "# multi-alternative test\n\t"
            "faddp %%st(1), %%st"
            : "=t" (ld8)
            : "0" (temp), "rm,t" (int_val2)
            : "st(1)"
        );
    }
    
    /* Complex chain of operations mixing x87 and general registers */
    {
        /* Use CRC32 builtin which has fixed register constraints */
        unsigned int crc = 0xFFFFFFFF;
        crc = __builtin_ia32_crc32qi(crc, (unsigned char)i7);
        i8 = (int)crc;
        
        /* Then use that result in x87 operation */
        long double crc_ld = (long double)crc;
        asm volatile (
            "faddp %%st(1), %%st"
            : "=t" (ld9)
            : "0" (ld8), "t" (crc_ld)
            : "st(1)"
        );
    }
    
    /* More x87 operations to increase pressure */
    asm volatile (
        "fsubrp %%st(1), %%st"
        : "=t" (ld11)
        : "0" (ld10), "t" (ld11)
        : "st(1)"
    );
    
    asm volatile (
        "fdivrp %%st(1), %%st"
        : "=t" (ld13)
        : "0" (ld12), "t" (ld13)
        : "st(1)"
    );
    
    /* Store results to globals to prevent elimination */
    global_results[global_index++] = ld1;
    global_results[global_index++] = ld2;
    global_results[global_index++] = ld3;
    global_results[global_index++] = ld4;
    global_results[global_index++] = ld5;
    global_results[global_index++] = ld6;
    global_results[global_index++] = ld7;
    global_results[global_index++] = ld8;
    global_results[global_index++] = ld9;
    global_results[global_index++] = ld10;
    global_results[global_index++] = ld11;
    global_results[global_index++] = ld12;
    global_results[global_index++] = ld13;
    global_results[global_index++] = ld14;
    global_results[global_index++] = ld15;
    
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
    int seed = 42;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call multiple times with different seeds */
    for (int i = 0; i < 3; i++) {
        test_secondary_reloads(seed + i);
    }
    
    /* Compute a simple checksum to use results */
    int checksum = 0;
    for (int i = 0; i < 15; i++) {
        checksum += (int)global_results[i];
    }
    for (int i = 0; i < 10; i++) {
        checksum += global_ints[i];
    }
    
    return checksum & 0xFF;
}
