/* Compile with: gcc -std=c99 -O2 -march=x86-64 -ffp-contract=off -fno-trapping-math -o fp_cond_test fp_cond_test.c */
/* Also try: gcc -std=c99 -O3 -msse4.2 -ftree-vectorize -fno-trapping-math -o fp_cond_test_vec fp_cond_test.c */
/* And: gcc -std=c99 -O1 -m32 -mfpmath=387 -fno-inline -o fp_cond_test_32 fp_cond_test.c */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Function to stress FP comparison condition code generation */
static int stress_fp_comparisons(double a, double b, double c, double d) {
    volatile double v1 = a;
    volatile double v2 = b;
    volatile double v3 = c;
    volatile double v4 = d;
    
    int result = 0;
    int temp;
    
    /* Create NaN and infinity constants */
    const double nan = __builtin_nan("");
    const double pinf = __builtin_inf();
    const double ninf = -__builtin_inf();
    
    /* Exhaustive comparison matrix with volatile operands */
    volatile double vnan = nan;
    volatile double vpinf = pinf;
    volatile double vninf = ninf;
    
    /* Block 1: Direct comparisons with conditional branches */
    /* This should generate various condition codes */
    
    /* UNORDERED cases (comparisons involving NaN) */
    if (vnan == v1) {
        result ^= 1;
    }
    
    if (v1 < vnan) {
        result ^= 2;
    }
    
    if (vnan <= v2) {
        result ^= 4;
    }
    
    if (v3 > vnan) {
        result ^= 8;
    }
    
    if (vnan >= v4) {
        result ^= 16;
    }
    
    if (vnan != vnan) {  /* Always true: NaN != NaN */
        result ^= 32;
    }
    
    /* ORDERED cases (normal comparisons) */
    if (v1 == v2) {
        result ^= 64;
    }
    
    if (v1 < v2) {
        result ^= 128;
    }
    
    if (v1 <= v2) {
        result ^= 256;
    }
    
    if (v1 > v2) {
        result ^= 512;
    }
    
    if (v1 >= v2) {
        result ^= 1024;
    }
    
    if (v1 != v2) {
        result ^= 2048;
    }
    
    /* Comparisons with infinity */
    if (v1 == vpinf) {
        result ^= 4096;
    }
    
    if (vninf < v2) {
        result ^= 8192;
    }
    
    /* Block 2: Conditional expressions (ternary operator) */
    /* These may generate conditional move instructions */
    double cond_val = (v1 < vnan) ? 1.0 : 2.0;
    result ^= (int)cond_val;
    
    cond_val = (vnan == v2) ? 3.0 : 4.0;
    result ^= (int)cond_val;
    
    cond_val = (v1 <= vpinf) ? 5.0 : 6.0;
    result ^= (int)cond_val;
    
    cond_val = (vninf >= v2) ? 7.0 : 8.0;
    result ^= (int)cond_val;
    
    /* Block 3: Complex control flow with goto */
    /* Prevents optimization and simplification */
    if (v1 != v1) {  /* Check if v1 is NaN */
        goto nan_case;
    } else if (v1 > vpinf) {
        goto inf_case;
    } else {
        goto normal_case;
    }
    
nan_case:
    result ^= 16384;
    if (v2 == v2) {  /* v2 is not NaN */
        goto mixed_case;
    }
    goto end_block;
    
inf_case:
    result ^= 32768;
    goto end_block;
    
mixed_case:
    result ^= 65536;
    /* UNEQ: unordered or equal */
    if (!(vnan < v2) && !(vnan > v2)) {
        result ^= 131072;
    }
    goto end_block;
    
normal_case:
    result ^= 262144;
    /* LTGT: less than or greater than (ordered and not equal) */
    if ((v1 < v2) || (v1 > v2)) {
        result ^= 524288;
    }
    goto end_block;
    
end_block:
    /* UNGE: unordered or greater than or equal */
    if (!(v1 < v2)) {
        result ^= 1048576;
    }
    
    /* UNGT: unordered or greater than */
    if (!(v1 <= v2)) {
        result ^= 2097152;
    }
    
    /* UNLE: unordered or less than or equal */
    if (!(v1 > v2)) {
        result ^= 4194304;
    }
    
    /* UNLT: unordered or less than */
    if (!(v1 >= v2)) {
        result ^= 8388608;
    }
    
    return result;
}

/* Function with vectorized FP comparisons */
static int vector_fp_comparisons(double *arr1, double *arr2, int n) {
    v2df vzero = {0.0, 0.0};
    v2di mask_acc = {0, 0};
    
    for (int i = 0; i < n - 1; i += 2) {
        /* Load two doubles into vector */
        v2df v1 = {arr1[i], arr1[i + 1]};
        v2df v2 = {arr2[i], arr2[i + 1]};
        
        /* Perform various vector comparisons */
        /* Each generates different condition codes */
        
        /* Equal */
        v2di mask_eq = (v2di)(v1 == v2);
        mask_acc |= mask_eq;
        
        /* Not equal */
        v2di mask_ne = (v2di)(v1 != v2);
        mask_acc ^= mask_ne;
        
        /* Less than */
        v2di mask_lt = (v2di)(v1 < v2);
        mask_acc += mask_lt;
        
        /* Less than or equal */
        v2di mask_le = (v2di)(v1 <= v2);
        mask_acc -= mask_le;
        
        /* Greater than */
        v2di mask_gt = (v2di)(v1 > v2);
        mask_acc |= mask_gt;
        
        /* Greater than or equal */
        v2di mask_ge = (v2di)(v1 >= v2);
        mask_acc ^= mask_ge;
    }
    
    /* Extract results from vector */
    long long *mask_ptr = (long long *)&mask_acc;
    return (int)(mask_ptr[0] ^ mask_ptr[1]);
}

/* Function with inline assembly using condition codes */
static int asm_fp_conditions(double a, double b) {
    int result = 0;
    unsigned char cc_result;
    
    /* Inline assembly that reads FP condition codes */
    /* SETP: parity flag (unordered) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 0);
    
    /* SETA: above (greater than, ordered) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "seta %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 1);
    
    /* SETB: below (less than, ordered) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 2);
    
    /* SETE: equal (ordered) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "sete %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 3);
    
    /* SETAE: above or equal (ordered, not less than) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setae %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 4);
    
    /* SETBE: below or equal (ordered, not greater than) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setbe %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 5);
    
    /* SETNE: not equal (ordered or unordered) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setne %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 6);
    
    return result;
}

int main() {
    /* Initialize test values */
    double normal1 = 3.14159;
    double normal2 = 2.71828;
    double normal3 = -1.41421;
    double normal4 = 0.0;
    
    const double nan = __builtin_nan("");
    const double pinf = __builtin_inf();
    const double ninf = -__builtin_inf();
    
    int checksum = 0;
    
    /* Test 1: Stress FP comparisons with various value combinations */
    checksum ^= stress_fp_comparisons(normal1, normal2, normal3, normal4);
    checksum ^= stress_fp_comparisons(nan, normal1, pinf, normal2);
    checksum ^= stress_fp_comparisons(normal3, nan, normal4, pinf);
    checksum ^= stress_fp_comparisons(pinf, ninf, nan, normal1);
    checksum ^= stress_fp_comparisons(nan, nan, pinf, pinf);
    
    /* Test 2: Vectorized comparisons */
    double arr1[8] = {1.0, 2.0, 3.0, 4.0, nan, 6.0, pinf, 8.0};
    double arr2[8] = {1.0, 3.0, 3.0, 2.0, 5.0, nan, pinf, ninf};
    
    checksum ^= vector_fp_comparisons(arr1, arr2, 8);
    
    /* Test 3: Inline assembly with condition codes */
    checksum ^= asm_fp_conditions(normal1, normal2);
    checksum ^= asm_fp_conditions(normal1, nan);
    checksum ^= asm_fp_conditions(nan, normal2);
    checksum ^= asm_fp_conditions(pinf, normal1);
    checksum ^= asm_fp_conditions(normal1, pinf);
    checksum ^= asm_fp_conditions(ninf, pinf);
    checksum ^= asm_fp_conditions(nan, nan);
    
    /* Additional unordered comparison patterns */
    volatile double vnan = nan;
    volatile double vnum = 42.0;
    
    /* Generate UNORDERED condition */
    if (vnan < vnum) {
        checksum ^= 0x11111111;
    }
    
    /* Generate ORDERED condition */
    if (vnum == vnum) {
        checksum ^= 0x22222222;
    }
    
    /* Generate UNEQ condition (unordered or equal) */
    if (!(vnan < vnum) && !(vnan > vnum)) {
        checksum ^= 0x33333333;
    }
    
    /* Generate UNGE condition (unordered or >=) */
    if (!(vnum < vnan)) {
        checksum ^= 0x44444444;
    }
    
    /* Generate UNGT condition (unordered or >) */
    if (!(vnum <= vnan)) {
        checksum ^= 0x55555555;
    }
    
    /* Generate UNLE condition (unordered or <=) */
    if (!(vnum > vnan)) {
        checksum ^= 0x66666666;
    }
    
    /* Generate UNLT condition (unordered or <) */
    if (!(vnum >= vnan)) {
        checksum ^= 0x77777777;
    }
    
    /* Generate LTGT condition (ordered and not equal) */
    if ((normal1 < normal2) || (normal1 > normal2)) {
        checksum ^= 0x88888888;
    }
    
    /* Final output to prevent optimization */
    printf("FP condition test checksum: %d (0x%x)\n", checksum, checksum);
    
    return checksum != 0 ? 0 : 1;
}
