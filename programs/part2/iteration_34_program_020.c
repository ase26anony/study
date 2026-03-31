/* test_secondary_reloads.c */
#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization */
#define VOL volatile
#define NOINLINE __attribute__((noinline))

/* Global array to prevent dead code elimination */
static long double global_results[32];
static int global_index = 0;

/* Function to mix bits */
static inline uint64_t mix_bits(uint64_t x) {
    x ^= (x >> 33);
    x *= 0xff51afd7ed558ccdULL;
    x ^= (x >> 33);
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= (x >> 33);
    return x;
}

NOINLINE static void test_secondary_reloads(int seed) {
    /* Create high register pressure with many volatile variables */
    VOL long double ld1, ld2, ld3, ld4, ld5, ld6, ld7, ld8;
    VOL long double ld9, ld10, ld11, ld12, ld13, ld14, ld15;
    VOL int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    VOL int i11, i12, i13, i14, i15;
    VOL uint64_t tsc1, tsc2;
    VOL int counter;
    
    /* Initialize with seed-dependent values to avoid constants */
    uint64_t state = seed;
    
    /* Initialize long doubles using integer arithmetic converted to long double */
    for (counter = 0; counter < 15; counter++) {
        state = mix_bits(state);
        /* Cast to long double to force x87 usage */
        ((long double*)&ld1)[counter] = (long double)(state & 0xFFFF) / 1000.0L;
    }
    
    /* Initialize integers */
    for (counter = 0; counter < 15; counter++) {
        state = mix_bits(state);
        ((int*)&i1)[counter] = (int)(state & 0xFFF);
    }
    
    /* Use RDTSC builtin which uses fixed registers (eax, edx) */
    tsc1 = __builtin_ia32_rdtsc();
    
    /* Force x87 operations with explicit register constraints */
    
    /* 1. Basic x87 operation with "t" constraint (top of x87 stack) */
    asm volatile (
        "fldt %1\n\t"           /* Load first operand to st(0) */
        "fldt %2\n\t"           /* Load second operand to st(0), previous moves to st(1) */
        "faddp %%st, %%st(1)\n\t" /* st(1) = st(1) + st(0), pop stack */
        "fstpt %0"
        : "=m" (ld1)
        : "m" (ld2), "m" (ld3)
        : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
    );
    
    /* 2. Mixed operation: x87 with general register input */
    /* This may trigger secondary reload for integer->x87 */
    asm volatile (
        "fildl %2\n\t"          /* Load integer to x87 stack */
        "fldt %1\n\t"           /* Load long double */
        "fmulp %%st, %%st(1)\n\t"
        "fstpt %0"
        : "=m" (ld4)
        : "m" (ld5), "m" (i1)
        : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
    );
    
    /* 3. Multi-alternative constraint: "rm,t" - may choose x87 register */
    /* This is key for triggering secondary reload initialization */
    {
        VOL long double ld_tmp = ld6;
        VOL int int_tmp = i2;
        
        asm volatile (
            "fldt %1\n\t"
            "fildl %2\n\t"
            "faddp %%st, %%st(1)\n\t"
            "fstpt %0"
            : "=m" (ld7)
            : "m" (ld_tmp), "rm,t" (int_tmp)
            : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
        );
    }
    
    /* 4. Complex pattern: x87 with memory output and multiple inputs */
    /* Use "u" constraint (second x87 register) */
    asm volatile (
        "fldt %2\n\t"           /* ld8 -> st(0) */
        "fldt %3\n\t"           /* ld9 -> st(0), ld8 -> st(1) */
        "fadd %%st(1), %%st\n\t" /* st(0) = st(0) + st(1) */
        "fstpt %0\n\t"
        "fstpt %1"
        : "=m" (ld10), "=m" (ld11)
        : "u" (ld8), "t" (ld9)
        : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
    );
    
    /* 5. CRC32 builtin which has fixed register constraints */
    /* Mix with x87 operations to increase complexity */
    i3 = __builtin_ia32_crc32qi(i3, (uint8_t)state);
    
    /* Convert CRC result to long double and add to x87 chain */
    {
        VOL int crc_temp = i3;
        asm volatile (
            "fildl %1\n\t"
            "fldt %2\n\t"
            "faddp %%st, %%st(1)\n\t"
            "fstpt %0"
            : "=m" (ld12)
            : "m" (crc_temp), "m" (ld13)
            : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
        );
    }
    
    /* 6. Another RDTSC to create timing dependency */
    tsc2 = __builtin_ia32_rdtsc();
    
    /* Use the difference in a x87 operation */
    {
        VOL uint64_t tsc_diff = tsc2 - tsc1;
        asm volatile (
            "fildq %1\n\t"      /* Load 64-bit integer to x87 */
            "fldt %2\n\t"
            "fdivp %%st, %%st(1)\n\t"
            "fstpt %0"
            : "=m" (ld14)
            : "m" (tsc_diff), "m" (ld15)
            : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
        );
    }
    
    /* Store results to global array to prevent elimination */
    global_results[global_index++] = ld1;
    global_results[global_index++] = ld4;
    global_results[global_index++] = ld7;
    global_results[global_index++] = ld10;
    global_results[global_index++] = ld11;
    global_results[global_index++] = ld12;
    global_results[global_index++] = ld14;
    
    /* Mix all integer results */
    i4 = i1 + i2 + i3 + i5 + i6 + i7 + i8 + i9 + i10;
    i11 = i12 + i13 + i14 + i15 + (int)tsc1 + (int)tsc2;
}

int main(int argc, char *argv[]) {
    int seed = 42;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call multiple times with different seeds */
    for (int i = 0; i < 10; i++) {
        test_secondary_reloads(seed + i);
    }
    
    /* Compute a simple checksum from results */
    long double checksum = 0.0L;
    for (int i = 0; i < global_index; i++) {
        checksum += global_results[i];
    }
    
    /* Use checksum to prevent elimination */
    asm volatile ("" : : "r" (checksum));
    
    return (int)checksum;
}
