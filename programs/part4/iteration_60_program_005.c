/* test_i386_float_conds.c
 * 
 * This program is designed to trigger the uncovered condition code
 * mnemonics in GCC's i386 backend for floating-point unordered comparisons.
 * It uses NaN, infinity, and normal values with various comparison
 * patterns to ensure the assembly output includes the specific string
 * representations (unord, ord, ueq, nlt, nle, ule, ult, une).
 *
 * Compile with (for coverage):
 *   gcc -O2 -march=x86-64 -ffast-math -fno-trapping-math -S -o test.s test_i386_float_conds.c
 *   gcc -O3 -march=haswell -mavx2 -ftree-vectorize -fno-omit-frame-pointer -S -o test.s test_i386_float_conds.c
 */

#include <stdio.h>
#include <stdint.h>

#ifdef __x86_64__

/* Helper to prevent optimization */
static volatile int global_counter = 0;

/* Function that uses comparison results to affect control flow */
static void use_result(int cond) {
    global_counter += cond;
}

int main(void) {
    uint64_t checksum = 0;
    
    /* Volatile to prevent constant folding */
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double neg_inf = -__builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    volatile double two = 2.0;
    
    /* 1. Direct unordered comparisons with operators */
    /* These should generate various condition codes */
    if (nan < inf) {           /* UNORDERED likely */
        checksum ^= 1;
    }
    
    if (nan == nan) {          /* UNORDERED/UNEQ */
        checksum ^= 2;
    }
    
    if (inf != nan) {          /* ORDERED/LTGT */
        checksum ^= 4;
    }
    
    if (nan <= inf) {          /* UNORDERED/UNLE */
        checksum ^= 8;
    }
    
    if (nan >= inf) {          /* UNORDERED/UNGE */
        checksum ^= 16;
    }
    
    if (nan > inf) {           /* UNORDERED/UNGT */
        checksum ^= 32;
    }
    
    /* 2. Built-in unordered comparison functions */
    /* These map directly to the condition codes */
    if (__builtin_isunordered(nan, inf)) {   /* UNORDERED */
        checksum ^= 64;
    }
    
    if (__builtin_islessgreater(nan, inf)) { /* LTGT */
        checksum ^= 128;
    }
    
    if (__builtin_isless(nan, inf)) {        /* UNLT */
        checksum ^= 256;
    }
    
    if (__builtin_isgreater(nan, inf)) {     /* UNGT */
        checksum ^= 512;
    }
    
    if (__builtin_islessequal(nan, inf)) {   /* UNLE */
        checksum ^= 1024;
    }
    
    if (__builtin_isgreaterequal(nan, inf)) { /* UNGE */
        checksum ^= 2048;
    }
    
    /* 3. Complex expressions with arithmetic that may produce NaN */
    volatile double inf_minus_inf = inf - inf;
    volatile double zero_div_zero = zero / zero;
    
    /* Mixed-type comparisons */
    volatile float f_nan = __builtin_nanf("");
    volatile float f_inf = __builtin_inff();
    volatile long double ld_nan = __builtin_nanl("");
    
    if (f_nan < (float)inf) {          /* UNORDERED/UNLT */
        checksum ^= 4096;
    }
    
    if ((double)ld_nan == nan) {       /* UNORDERED/UNEQ */
        checksum ^= 8192;
    }
    
    /* FMA with NaN inputs */
    volatile double fma_result = __builtin_fma(nan, one, two);
    if (fma_result > zero) {           /* UNORDERED/UNGT */
        checksum ^= 16384;
    }
    
    /* 4. Vector comparisons using GCC extensions */
    typedef float v4sf __attribute__((vector_size(16)));
    typedef double v2df __attribute__((vector_size(16)));
    
    v4sf vec_a = {nan, inf, 1.0f, 2.0f};
    v4sf vec_b = {inf, nan, 3.0f, 1.0f};
    
    v4sf vec_cmp = vec_a > vec_b;  /* Should generate multiple condition checks */
    
    /* Extract comparison mask to force code generation */
    int mask = __builtin_ia32_movmskps((__v4sf)vec_cmp);
    checksum ^= (mask << 16);
    
    /* Double precision vector */
    v2df vec_da = {nan, inf};
    v2df vec_db = {inf, nan};
    v2df vec_dcmp = vec_da < vec_db;
    
    /* Store to memory and check to prevent elimination */
    volatile int dmask;
    __asm__ volatile ("vmovmskpd %1, %0" : "=r"(dmask) : "x"(vec_dcmp));
    checksum ^= (dmask << 18);
    
    /* 5. Inline assembly with explicit condition codes */
    /* This directly exercises the assembly output logic */
    double a = nan;
    double b = inf;
    int result;
    
    /* ucomisd with setp (parity flag for unordered) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    checksum ^= (result << 20);
    
    /* fucomi with multiple condition checks */
    long double ld_a = ld_nan;
    long double ld_b = (long double)inf;
    int result2;
    
    __asm__ volatile (
        "fucomip %2, %1\n\t"
        "setbe %0"
        : "=r"(result2)
        : "t"(ld_a), "u"(ld_b)
        : "cc"
    );
    checksum ^= (result2 << 21);
    
    /* 6. Control flow driven by unordered results */
    /* Switch statement where cases depend on comparison combinations */
    int cmp_results[8] = {0};
    
    /* Generate various condition code combinations */
    cmp_results[0] = __builtin_isunordered(nan, inf) ? 1 : 0;      /* UNORDERED */
    cmp_results[1] = !__builtin_isunordered(one, two) ? 2 : 0;     /* ORDERED */
    cmp_results[2] = (nan == nan) ? 3 : 0;                         /* UNEQ */
    cmp_results[3] = !(nan < inf) ? 4 : 0;                         /* UNGE (nlt) */
    cmp_results[4] = !(nan <= inf) ? 5 : 0;                        /* UNGT (nle) */
    cmp_results[5] = (nan <= inf) ? 6 : 0;                         /* UNLE */
    cmp_results[6] = (nan < inf) ? 7 : 0;                          /* UNLT */
    cmp_results[7] = (nan != inf) ? 8 : 0;                         /* LTGT (une) */
    
    /* Use in a switch to force conditional jump generation */
    for (int i = 0; i < 8; i++) {
        switch (cmp_results[i]) {
            case 1: checksum += 0x1000; break;  /* UNORDERED path */
            case 2: checksum += 0x2000; break;  /* ORDERED path */
            case 3: checksum += 0x3000; break;  /* UNEQ path */
            case 4: checksum += 0x4000; break;  /* UNGE path */
            case 5: checksum += 0x5000; break;  /* UNGT path */
            case 6: checksum += 0x6000; break;  /* UNLE path */
            case 7: checksum += 0x7000; break;  /* UNLT path */
            case 8: checksum += 0x8000; break;  /* LTGT path */
        }
    }
    
    /* Ternary operators with unordered comparisons */
    volatile double select = __builtin_isunordered(zero_div_zero, inf) ? nan : inf;
    checksum ^= (*(uint64_t*)&select);
    
    /* Loop whose iterations depend on NaN comparisons */
    int iterations = 0;
    volatile double loop_var = nan;
    while (__builtin_islessgreater(loop_var, zero) && iterations < 10) {
        checksum ^= iterations;
        loop_var = loop_var / 2.0;
        iterations++;
    }
    
    /* Final output to prevent dead code elimination */
    printf("Checksum: 0x%016llx\n", (unsigned long long)checksum);
    return (int)(checksum & 0x7FFFFFFF);
}

#else
/* Non-x86 fallback */
int main(void) {
    printf("This test is for x86/x86-64 architecture only.\n");
    return 0;
}
#endif
