/* test_secondary_reloads.c
 * Designed to trigger secondary reload initialization in GCC's reload pass
 * Compile with: gcc -O1 -m32 -mfpmath=387 -march=i686 -fno-omit-frame-pointer -c test_secondary_reloads.c
 */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define VOLATILE volatile

/* Global array to prevent dead code elimination */
static VOLATILE long double global_results[32];
static VOLATILE int global_ints[32];
static int result_idx = 0;

/* Function to mix values using seed */
NOINLINE static unsigned int mix(unsigned int x) {
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x;
}

/* Main stress function that should trigger secondary reloads */
NOINLINE static long double test_secondary_reloads(unsigned int seed) {
    /* Create many volatile variables to increase register pressure */
    VOLATILE long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    VOLATILE long double ld9, ld10, ld11, ld12, ld13, ld14, ld15, ld16;
    VOLATILE int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    VOLATILE unsigned int u1, u2, u3, u4, u5;
    VOLATILE long double result = 0.0L;
    
    /* Initialize with seed-dependent values to prevent constant folding */
    u1 = seed;
    u2 = mix(seed);
    u3 = mix(u2);
    u4 = mix(u3);
    u5 = mix(u4);
    
    /* Convert to long double with some arithmetic */
    ld1 = (long double)u1 * 0.1L;
    ld2 = (long double)u2 * 0.2L;
    ld3 = (long double)u3 * 0.3L;
    ld4 = (long double)u4 * 0.4L;
    ld5 = (long double)u5 * 0.5L;
    
    /* More initialization */
    i1 = (int)u1;
    i2 = (int)u2;
    i3 = (int)u3;
    i4 = (int)u4;
    i5 = (int)u5;
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    {
        unsigned long long tsc;
        tsc = __builtin_ia32_rdtsc();
        i6 = (int)(tsc >> 32);
        i7 = (int)(tsc & 0xFFFFFFFFU);
    }
    
    /* Force x87 register usage with inline asm */
    /* This asm uses "t" constraint (top of x87 stack) */
    asm volatile (
        "fldt %1\n\t"           /* load ld1 onto x87 stack */
        "fldt %2\n\t"           /* load ld2 onto x87 stack */
        "faddp %%st, %%st(1)\n\t" /* st(1) = st(1) + st, pop */
        "fstpt %0"
        : "=m" (ld6)
        : "m" (ld1), "m" (ld2)
        : "st", "st(1)"
    );
    
    /* More x87 operations to increase pressure */
    asm volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "fmulp %%st, %%st(1)\n\t"
        "fstpt %0"
        : "=m" (ld7)
        : "m" (ld3), "m" (ld4)
        : "st", "st(1)"
    );
    
    /* CRITICAL: Multi-alternative constraint that may trigger secondary reload */
    /* "rm,t" means either memory/general register OR x87 top register */
    /* The compiler may choose the "t" alternative for the integer */
    {
        VOLATILE int temp_int = i1 + i2;
        VOLATILE long double temp_ld = ld5;
        
        asm volatile (
            "fldt %1\n\t"           /* load temp_ld */
            "fildl %2\n\t"          /* load integer - may need secondary reload */
            "faddp %%st, %%st(1)\n\t"
            "fstpt %0"
            : "=m" (ld8)
            : "m" (temp_ld), "rm,t" (temp_int)
            : "st", "st(1)"
        );
    }
    
    /* Another complex case: mixing x87 with MMX-like operations */
    /* Use division which requires specific registers on x86 */
    {
        VOLATILE int divisor = i3 | 1;  /* ensure non-zero */
        VOLATILE int dividend = i4;
        VOLATILE int quotient;
        
        /* Division uses fixed registers: eax for dividend, edx for remainder */
        asm volatile (
            "movl %1, %%eax\n\t"
            "cltd\n\t"
            "idivl %2\n\t"
            "movl %%eax, %0"
            : "=r" (quotient)
            : "r" (dividend), "r" (divisor)
            : "eax", "edx"
        );
        i8 = quotient;
    }
    
    /* More register pressure with x87 */
    ld9 = ld6 + ld7;
    ld10 = ld8 * ld9;
    
    /* Use "u" constraint (second x87 register) */
    asm volatile (
        "fldt %2\n\t"   /* load ld10 to st(0) */
        "fldt %1\n\t"   /* load ld9 to st(0), ld10 moves to st(1) */
        "fxch %%st(1)\n\t"  /* swap: now st(0)=ld10, st(1)=ld9 */
        "fsubrp %%st, %%st(1)\n\t"  /* st(1) = st(1) - st(0), pop */
        "fstpt %0"
        : "=m" (ld11)
        : "m" (ld9), "m" (ld10)
        : "st", "st(1)"
    );
    
    /* Chain operations to keep values live */
    for (VOLATILE int counter = 0; counter < 3; counter++) {
        /* Mix integer and floating point */
        i9 = i8 + counter;
        ld12 = ld11 * (long double)i9;
        
        /* Another asm with complex constraints */
        {
            VOLATILE long double a = ld12;
            VOLATILE long double b = ld10;
            
            asm volatile (
                "fldt %2\n\t"
                "fldt %1\n\t"
                : "=t" (ld13)
                : "0" (a), "u" (b)
                : "st(1)"
            );
        }
        
        result += ld13;
    }
    
    /* Store results to globals to prevent elimination */
    global_results[result_idx % 32] = result;
    global_ints[result_idx % 32] = i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9;
    result_idx++;
    
    return result;
}

/* Wrapper with multiple calls to increase reload opportunities */
NOINLINE static long double stress_function(unsigned int seed) {
    VOLATILE long double total = 0.0L;
    VOLATILE unsigned int s = seed;
    
    for (int i = 0; i < 5; i++) {
        s = mix(s + i);
        total += test_secondary_reloads(s);
        
        /* Add some integer operations between calls */
        VOLATILE int x = (int)s;
        VOLATILE int y = (int)total;
        
        /* Use CRC32 builtin which has fixed register constraints */
        if (i & 1) {
            unsigned int crc = 0;
            crc = __builtin_ia32_crc32qi(crc, (unsigned char)x);
            crc = __builtin_ia32_crc32hi(crc, (unsigned short)y);
            global_ints[(result_idx + i) % 32] ^= (int)crc;
        }
    }
    
    return total;
}

int main(int argc, char *argv[]) {
    unsigned int seed = 12345;
    
    if (argc > 1) {
        seed = (unsigned int)atoi(argv[1]);
    }
    
    VOLATILE long double final_result = stress_function(seed);
    
    /* Use the result to prevent dead code elimination */
    return (int)(final_result * 0.0001L) & 0xFF;
}
