/* test_secondary_reloads.c
 * Designed to cover secondary reload initialization in GCC's reload.cc
 * Compile with: gcc -O1 -m32 -mfpmath=387 -march=i686 -fno-omit-frame-pointer -c test_secondary_reloads.c
 */

#include <stdint.h>
#include <stdlib.h>

/* Global array to prevent dead code elimination */
volatile long double global_results[32];
volatile int global_ints[32];
int global_index = 0;

/* Prevent inlining to ensure reload happens in this function */
__attribute__((noinline, noipa))
void test_secondary_reloads(int seed) {
    /* Create high register pressure with many volatile variables */
    volatile long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    volatile long double ld9, ld10, ld11, ld12, ld13, ld14, ld15;
    volatile int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    volatile int i11, i12, i13, i14, i15, i16, i17, i18, i19, i20;
    volatile unsigned int lo, hi;
    
    /* Initialize with seed-dependent values to avoid constant propagation */
    i1 = seed + 1;
    i2 = seed * 2 + 3;
    i3 = seed ^ 0x1234;
    i4 = seed * 3 - 5;
    i5 = seed + 0x5678;
    i6 = seed * 5 + 7;
    i7 = seed ^ 0x9ABC;
    i8 = seed * 7 - 11;
    i9 = seed + 0xDEF0;
    i10 = seed * 11 + 13;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    __asm__ volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    i11 = lo + hi;
    i12 = lo ^ hi;
    
    /* Initialize long double variables with conversions from integers */
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
    
    /* Force x87 operations with 't' (top of stack) and 'u' (second) constraints */
    
    /* Pattern 1: Simple x87 operation forcing secondary reloads */
    __asm__ volatile (
        "faddp %%st(1), %%st"  /* st(1) = st(1) + st(0), pop stack */
        : "=t" (ld1)
        : "0" (ld1), "u" (ld2)
        : "st(1)"
    );
    
    /* Pattern 2: Multi-alternative constraint with 'rm,t' for integer */
    /* This may force secondary reload for integer operand */
    __asm__ volatile (
        "fild%z1 %1\n\t"       /* load integer to st(0) */
        "faddp %%st(1), %%st"  /* add to st(1) */
        : "=t" (ld3)
        : "0" (ld3), "rm,t" (i3)
        : "st(1)"
    );
    
    /* Pattern 3: Complex pattern mixing x87 and general registers */
    /* Using CRC32 builtin which uses fixed register for accumulator */
    i13 = __builtin_ia32_crc32qi(i11, (unsigned char)i12);
    
    /* Now use that result in x87 operation with multi-alternative constraint */
    __asm__ volatile (
        "fild%z2 %2\n\t"
        "fmulp %%st(1), %%st"
        : "=t" (ld4)
        : "0" (ld4), "rm,t" (i13)
        : "st(1)"
    );
    
    /* Pattern 4: Chain multiple x87 operations to increase register pressure */
    __asm__ volatile (
        "fldt %2\n\t"          /* load ld5 to st(0) */
        "faddp %%st(2), %%st\n\t" /* add to st(2) */
        "fldt %3\n\t"          /* load ld6 to st(0) */
        "fmulp %%st(2), %%st"  /* multiply with st(2) */
        : "=t" (ld7), "=u" (ld8)
        : "0" (ld7), "u" (ld8), "m" (ld5), "m" (ld6)
        : "st(2)"
    );
    
    /* Pattern 5: Division with fixed register constraint 'a' (eax) */
    /* This may require secondary reload for the divisor */
    i14 = i1;
    i15 = i2;
    __asm__ volatile (
        "divb %2"
        : "+a" (i14), "=d" (i15)
        : "rm" ((unsigned char)i3)
        : "cc"
    );
    
    /* Use the division result in x87 operation */
    __asm__ volatile (
        "fildl %1\n\t"
        "faddp %%st(1), %%st"
        : "=t" (ld9)
        : "m" (i14), "0" (ld9)
        : "st(1)"
    );
    
    /* Pattern 6: Shift with fixed 'c' (ecx) register constraint */
    i16 = i4;
    i17 = i5;
    __asm__ volatile (
        "shrdl %%cl, %2, %0"
        : "+r" (i16)
        : "r" (i17), "c" ((unsigned char)i6)
        : "cc"
    );
    
    /* More x87 operations to ensure secondary reload fields are set */
    for (volatile int j = 0; j < 3; j++) {
        __asm__ volatile (
            "fldt %1\n\t"
            "faddp %%st(1), %%st"
            : "+t" (ld10)
            : "m" (ld11)
        );
        
        /* Mix with MMX-style 64-bit operation (may require secondary reload) */
        long long ll1 = (long long)i7 * i8;
        long long ll2 = (long long)i9 * i10;
        
        __asm__ volatile (
            "movq %1, %%mm0\n\t"
            "paddq %2, %%mm0\n\t"
            "movq %%mm0, %0\n\t"
            "emms"
            : "=m" (ll1)
            : "m" (ll1), "m" (ll2)
            : "mm0"
        );
        
        i18 = (int)(ll1 >> 32);
    }
    
    /* Store results to global arrays to prevent elimination */
    global_results[global_index] = ld1;
    global_results[global_index + 1] = ld2;
    global_results[global_index + 2] = ld3;
    global_results[global_index + 3] = ld4;
    global_results[global_index + 4] = ld7;
    global_results[global_index + 5] = ld8;
    global_results[global_index + 6] = ld9;
    global_results[global_index + 7] = ld10;
    
    global_ints[global_index] = i11;
    global_ints[global_index + 1] = i12;
    global_ints[global_index + 2] = i13;
    global_ints[global_index + 3] = i14;
    global_ints[global_index + 4] = i15;
    global_ints[global_index + 5] = i16;
    global_ints[global_index + 6] = i17;
    global_ints[global_index + 7] = i18;
    
    global_index = (global_index + 8) % 32;
}

int main(int argc, char **argv) {
    int seed = 42;  /* Default seed */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call multiple times with different seeds to explore different paths */
    for (int i = 0; i < 10; i++) {
        test_secondary_reloads(seed + i * 100);
    }
    
    /* Compute a simple checksum to use the results */
    int checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum ^= global_ints[i];
        checksum += (int)global_results[i];
    }
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
