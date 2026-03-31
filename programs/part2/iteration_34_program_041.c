/* test_secondary_reloads.c */
#include <stdint.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[20];
volatile int global_ints[20];
int global_index = 0;

/* Prevent inlining to ensure complex reload patterns aren't optimized away */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Create high register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8, ld9, ld10;
    volatile long double ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15;
    
    /* Initialize with seed-dependent values to prevent constant propagation */
    i1 = seed + 1;
    i2 = seed * 2 + 3;
    i3 = seed ^ 0x1234;
    i4 = seed << 3;
    i5 = seed >> 2;
    i6 = seed * seed;
    i7 = seed + 0xABCD;
    i8 = seed | 0xFF00;
    i9 = seed & 0x00FF;
    i10 = ~seed;
    i11 = seed * 3 + 7;
    i12 = seed / 2;
    i13 = seed % 100;
    i14 = seed + 0xDEAD;
    i15 = seed * seed + seed;
    
    /* Convert some integers to long double for x87 operations */
    ld1 = (long double)i1;
    ld2 = (long double)i2;
    ld3 = (long double)i3;
    ld4 = (long double)i4;
    ld5 = (long double)i5;
    ld6 = (long double)i6;
    ld7 = (long double)i7;
    ld8 = (long double)i8;
    ld9 = (long double)i9;
    ld10 = (long double)i10;
    ld11 = (long double)i11;
    ld12 = (long double)i12;
    ld13 = (long double)i13;
    ld14 = (long double)i14;
    ld15 = (long double)i15;
    
    /* Force use of RDTSC which uses fixed registers (eax, edx) */
    {
        uint64_t tsc1, tsc2;
        tsc1 = __builtin_ia32_rdtsc();
        /* Use the result to prevent optimization */
        i1 = (int)(tsc1 & 0xFFFFFFFF);
        i2 = (int)(tsc1 >> 32);
        
        /* Create register pressure with the result */
        ld1 = ld1 * (long double)i1;
        ld2 = ld2 / (long double)i2;
        
        tsc2 = __builtin_ia32_rdtsc();
        i3 = (int)(tsc2 & 0xFFFFFFFF);
        i4 = (int)(tsc2 >> 32);
    }
    
    /* CRITICAL: Inline assembly with x87 register constraints
       This is the primary mechanism to trigger secondary reloads */
    
    /* 1. Simple x87 operation with "t" constraint (top of x87 stack) */
    asm volatile (
        "faddp %%st(1), %%st"
        : "=t" (ld3)
        : "0" (ld1), "t" (ld2)
        : "st(1)"
    );
    
    /* 2. Multi-alternative constraint: "rm,t" - may choose x87 register */
    /* This is likely to trigger secondary reload setup */
    asm volatile (
        "faddl %2\n\t"
        "fstp %%st(1)"
        : "=t" (ld4)
        : "0" (ld3), "rm,t" (i5)
    );
    
    /* 3. Another x87 operation mixing registers */
    asm volatile (
        "fmulp %%st(1), %%st"
        : "=t" (ld5)
        : "0" (ld4), "u" (ld6)  /* "u" = second x87 register */
        : "st(1)"
    );
    
    /* 4. Complex pattern with multiple outputs/inputs */
    long double tmp1, tmp2;
    asm volatile (
        "fldt %2\n\t"
        "fldt %3\n\t"
        "faddp %%st(1), %%st\n\t"
        "fstpt %0\n\t"
        "fldt %4\n\t"
        "fstpt %1"
        : "=m" (tmp1), "=m" (tmp2)
        : "m" (ld7), "m" (ld8), "m" (ld9)
        : "st", "st(1)", "st(2)"
    );
    ld10 = tmp1 + tmp2;
    
    /* 5. Use CRC32 builtin which has fixed register constraints */
    {
        unsigned int crc = 0xFFFFFFFF;
        crc = __builtin_ia32_crc32qi(crc, (unsigned char)i6);
        crc = __builtin_ia32_crc32hi(crc, (unsigned short)i7);
        crc = __builtin_ia32_crc32si(crc, (unsigned int)i8);
        i9 = (int)crc;
    }
    
    /* 6. More x87 operations to increase pressure */
    asm volatile (
        "fdivrp %%st(1), %%st"
        : "=t" (ld11)
        : "0" (ld10), "t" (ld5)
        : "st(1)"
    );
    
    /* 7. Integer operation that might need specific register */
    /* Division uses fixed registers on x86 */
    {
        volatile int dividend = i10;
        volatile int divisor = i11 ? i11 : 1; /* Avoid division by zero */
        volatile int quotient, remainder;
        
        asm volatile (
            "xorl %%edx, %%edx\n\t"
            "divl %2"
            : "=a" (quotient), "=d" (remainder)
            : "r" (divisor), "a" (dividend), "d" (0)
            : "cc"
        );
        i12 = quotient;
        i13 = remainder;
    }
    
    /* 8. Final mixed operation with complex constraints */
    /* This pattern often requires secondary reloads */
    asm volatile (
        "fildl %2\n\t"
        "faddp %%st(1), %%st"
        : "=t" (ld12)
        : "0" (ld11), "m" (i14)
        : "st(1)"
    );
    
    /* Store results to globals to prevent elimination */
    global_results[global_index] = ld1; global_index = (global_index + 1) % 20;
    global_results[global_index] = ld2; global_index = (global_index + 1) % 20;
    global_results[global_index] = ld3; global_index = (global_index + 1) % 20;
    global_results[global_index] = ld4; global_index = (global_index + 1) % 20;
    global_results[global_index] = ld5; global_index = (global_index + 1) % 20;
    global_results[global_index] = ld6; global_index = (global_index + 1) % 20;
    global_results[global_index] = ld7; global_index = (global_index + 1) % 20;
    global_results[global_index] = ld8; global_index = (global_index + 1) % 20;
    global_results[global_index] = ld9; global_index = (global_index + 1) % 20;
    global_results[global_index] = ld10; global_index = (global_index + 1) % 20;
    global_results[global_index] = ld11; global_index = (global_index + 1) % 20;
    global_results[global_index] = ld12; global_index = (global_index + 1) % 20;
    
    global_ints[global_index] = i1; global_index = (global_index + 1) % 20;
    global_ints[global_index] = i2; global_index = (global_index + 1) % 20;
    global_ints[global_index] = i3; global_index = (global_index + 1) % 20;
    global_ints[global_index] = i4; global_index = (global_index + 1) % 20;
    global_ints[global_index] = i5; global_index = (global_index + 1) % 20;
    global_ints[global_index] = i6; global_index = (global_index + 1) % 20;
    global_ints[global_index] = i7; global_index = (global_index + 1) % 20;
    global_ints[global_index] = i8; global_index = (global_index + 1) % 20;
    global_ints[global_index] = i9; global_index = (global_index + 1) % 20;
    global_ints[global_index] = i10; global_index = (global_index + 1) % 20;
    global_ints[global_index] = i11; global_index = (global_index + 1) % 20;
    global_ints[global_index] = i12; global_index = (global_index + 1) % 20;
    global_ints[global_index] = i13; global_index = (global_index + 1) % 20;
    global_ints[global_index] = i14; global_index = (global_index + 1) % 20;
    global_ints[global_index] = i15; global_index = (global_index + 1) % 20;
}

/* Loop to increase chances of triggering reloads */
__attribute__((noinline))
void stress_test(int iterations, int seed) {
    volatile int counter = 0;
    for (int i = 0; i < iterations; i++) {
        test_secondary_reloads(seed + i);
        counter++;
    }
    /* Use counter to prevent loop elimination */
    global_results[0] += (long double)counter;
}

int main(int argc, char *argv[]) {
    int seed = 12345;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Run multiple iterations to increase coverage probability */
    stress_test(10, seed);
    
    /* Compute a simple checksum from results */
    long double checksum = 0.0;
    for (int i = 0; i < 20; i++) {
        checksum += global_results[i];
        checksum += (long double)global_ints[i];
    }
    
    /* Return something based on checksum to prevent optimization */
    return (int)(checksum / 1000.0) & 0xFF;
}
