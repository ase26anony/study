/* test_secondary_reloads.c */
/* Compile with: -O1 -m32 -mfpmath=387 -march=i686 -fno-omit-frame-pointer */
/* Or: -O2 -m64 -mfpmath=387 -march=x86-64 -fno-schedule-insns */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[20];
volatile int global_ints[20];

/* Prevent inlining to ensure reload logic isn't optimized away */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* High register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    volatile long double ld9, ld10, ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    
    /* Initialize with seed-dependent values to avoid constant propagation */
    i1 = seed * 1;
    i2 = seed * 2 + 1;
    i3 = seed * 3 + 2;
    i4 = seed * 4 + 3;
    i5 = seed * 5 + 4;
    
    ld1 = (long double)(seed + 1) / 3.14159265358979323846L;
    ld2 = (long double)(seed + 2) / 2.71828182845904523536L;
    ld3 = (long double)(seed + 3) / 1.41421356237309504880L;
    ld4 = (long double)(seed + 4) / 1.61803398874989484820L;
    ld5 = (long double)(seed + 5) / 0.57721566490153286060L;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    {
        unsigned long long ts1, ts2;
        ts1 = __builtin_ia32_rdtsc();
        i6 = (int)(ts1 >> 32) + i1;
        ts2 = __builtin_ia32_rdtsc();
        i7 = (int)(ts2 & 0xFFFFFFFF) + i2;
    }
    
    /* Force x87 register usage with inline asm */
    /* This should trigger secondary reloads for moving between x87 and general regs */
    
    /* Example 1: x87 arithmetic with 't' and 'u' constraints */
    asm volatile (
        "fldt %2\n\t"           /* load ld2 onto x87 stack */
        "fldt %1\n\t"           /* load ld1 onto x87 stack (now st(0)=ld1, st(1)=ld2) */
        "faddp %%st, %%st(1)\n\t" /* st(1) = st(1) + st(0), pop stack */
        "fstpt %0"
        : "=m" (ld6)
        : "m" (ld1), "m" (ld2)
        : "st", "st(1)"
    );
    
    /* Example 2: Mixed integer and x87 with multi-alternative constraint */
    /* The "rm,t" constraint may force secondary reload for integer operand */
    {
        long double result;
        int int_val = i3 + 12345;
        
        asm volatile (
            "fildl %2\n\t"      /* load integer to x87 stack */
            "fldt %1\n\t"       /* load long double */
            "fmulp %%st, %%st(1)\n\t"
            "fstpt %0"
            : "=m" (result)
            : "m" (ld3), "r" (int_val)  /* "r" constraint - may need secondary reload */
            : "st", "st(1)"
        );
        ld7 = result;
    }
    
    /* Example 3: More complex with output in x87 register */
    {
        long double temp1 = ld4, temp2 = ld5;
        long double out1, out2;
        
        /* First asm: compute using x87, output in st(0) */
        asm volatile (
            "fldt %2\n\t"
            "fldt %1\n\t"
            "fdivrp %%st, %%st(1)\n\t"
            : "=t" (out1)
            : "0" (temp1), "t" (temp2)  /* Both inputs in x87 regs */
        );
        
        /* Second asm: take x87 output and integer, force potential secondary reload */
        int ival = i4 * 2;
        asm volatile (
            "fildl %2\n\t"      /* integer to x87 */
            "fxch %%st(1)\n\t"  /* exchange st(0) and st(1) */
            "faddp %%st, %%st(1)\n\t"
            "fstpt %0"
            : "=m" (out2)
            : "t" (out1), "r" (ival)    /* "r" for integer - may need secondary */
            : "st"
        );
        ld8 = out2;
    }
    
    /* Example 4: CRC32 builtin which has fixed register constraints */
    {
        unsigned int crc = 0xFFFFFFFF;
        unsigned char data = (unsigned char)(seed & 0xFF);
        crc = __builtin_ia32_crc32qi(crc, data);
        i8 = (int)crc + i5;
    }
    
    /* Create register pressure with more operations */
    for (volatile int k = 0; k < 3; k++) {
        /* Mix x87 and general purpose operations */
        ld9 = ld6 + ld7 + (long double)k;
        i9 = i6 + i7 + k * 7;
        
        /* Another asm with x87 constraints */
        long double a = ld8, b = ld9;
        long double c;
        
        asm volatile (
            "fldt %2\n\t"
            "fldt %1\n\t"
            "fsubrp %%st, %%st(1)\n\t"
            "fstpt %0"
            : "=m" (c)
            : "m" (a), "m" (b)
            : "st", "st(1)"
        );
        ld10 = c;
        
        i10 = i8 ^ i9;
    }
    
    /* More variables to increase pressure */
    ld11 = ld1 * ld2 - ld3;
    ld12 = ld4 / ld5 + ld6;
    ld13 = ld7 * 2.0L - ld8;
    ld14 = ld9 / 3.0L + ld10;
    
    i11 = i1 * i2 - i3;
    i12 = i4 + i5 * i6;
    i13 = i7 ^ i8 | i9;
    i14 = i10 * 3 + 17;
    
    /* Final complex asm with multiple constraints */
    {
        long double final_result;
        int mix_int = i11 + i12;
        
        /* This asm has alternative constraints that may trigger secondary reload */
        asm volatile (
            "fildl %2\n\t"          /* integer to x87 */
            "fldt %1\n\t"           /* long double to x87 */
            "fmulp %%st, %%st(1)\n\t"
            "fstpt %0"
            : "=m" (final_result)
            : "m" (ld11), 
              "r,m" (mix_int)       /* Alternative: register OR memory */
            : "st", "st(1)"
        );
        ld15 = final_result;
    }
    
    i15 = i13 * i14 + seed;
    
    /* Store to globals to prevent optimization */
    global_results[0] = ld1; global_results[1] = ld2;
    global_results[2] = ld3; global_results[3] = ld4;
    global_results[4] = ld5; global_results[5] = ld6;
    global_results[6] = ld7; global_results[7] = ld8;
    global_results[8] = ld9; global_results[9] = ld10;
    global_results[10] = ld11; global_results[11] = ld12;
    global_results[12] = ld13; global_results[13] = ld14;
    global_results[14] = ld15;
    
    global_ints[0] = i1; global_ints[1] = i2;
    global_ints[2] = i3; global_ints[3] = i4;
    global_ints[4] = i5; global_ints[5] = i6;
    global_ints[6] = i7; global_ints[7] = i8;
    global_ints[8] = i9; global_ints[9] = i10;
    global_ints[10] = i11; global_ints[11] = i12;
    global_ints[12] = i13; global_ints[13] = i14;
    global_ints[14] = i15;
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
    
    /* Compute a simple checksum */
    int checksum = 0;
    for (int i = 0; i < 15; i++) {
        checksum += global_ints[i];
        checksum ^= (int)(global_results[i] * 1000.0L);
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}
