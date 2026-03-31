/* Compile with: gcc -std=c99 -O2 -march=x86-64 -mtune=generic -ffp-contract=off -o fp_conds fp_conds.c */
/* Also try: gcc -std=c99 -O3 -msse4.2 -ftree-vectorize -fno-trapping-math -o fp_conds_vec fp_conds.c */
/* And: gcc -std=c99 -O1 -m32 -mfpmath=387 -fno-inline -o fp_conds_32 fp_conds.c */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Function to stress FP comparison condition code generation */
static int stress_fp_comparisons(double a, double b, double nan_val, double inf_val, double neg_inf_val) {
    volatile double v1 = a;
    volatile double v2 = b;
    volatile double v_nan = nan_val;
    volatile double v_inf = inf_val;
    volatile double v_neg_inf = neg_inf_val;
    
    int result = 0;
    
    /* Matrix of comparisons to trigger different condition codes */
    
    /* Normal number comparisons */
    if (v1 == v2) result ^= 1;
    if (v1 != v2) result ^= 2;
    if (v1 < v2)  result ^= 4;
    if (v1 <= v2) result ^= 8;
    if (v1 > v2)  result ^= 16;
    if (v1 >= v2) result ^= 32;
    
    /* Comparisons with NaN - will trigger unordered conditions */
    if (v1 == v_nan) result ^= 64;      /* Always false, but generates code */
    if (v1 != v_nan) result ^= 128;     /* Always true for non-NaN v1 */
    if (v1 < v_nan)  result ^= 256;     /* Always false */
    if (v_nan <= v2) result ^= 512;     /* Always false */
    if (v_nan > v_inf) result ^= 1024;  /* Always false */
    if (v_neg_inf >= v_nan) result ^= 2048; /* Always false */
    
    /* NaN vs NaN comparisons */
    if (v_nan == v_nan) result ^= 4096;  /* Always false */
    if (v_nan != v_nan) result ^= 8192;  /* Always true - classic NaN test */
    
    /* Infinity comparisons */
    if (v_inf == v_inf) result ^= 16384;
    if (v_neg_inf < v_inf) result ^= 32768;
    if (v1 <= v_inf) result ^= 65536;
    if (v_inf >= v2) result ^= 131072;
    
    /* Complex conditional expressions using ?: operator */
    double cond_val = (v1 < v2) ? 1.0 : (v1 > v2) ? -1.0 : 0.0;
    result ^= (int)(cond_val * 1000);
    
    cond_val = (v1 != v_nan) ? 2.0 : 0.0;
    result ^= (int)(cond_val * 100);
    
    /* Goto-based control flow to prevent optimization */
    if (v1 < v2) goto less;
    if (v1 > v2) goto greater;
    goto equal;
    
less:
    result |= 0x1000000;
    goto done;
greater:
    result |= 0x2000000;
    goto done;
equal:
    result |= 0x4000000;
done:
    
    return result;
}

/* Function with vectorized comparisons */
static int vector_fp_comparisons(void) {
    v2df vec1 = {1.0, 2.0};
    v2df vec2 = {2.0, 1.0};
    v2df vec_nan = {__builtin_nan(""), 3.0};
    v2df vec_inf = {__builtin_inf(), -__builtin_inf()};
    
    /* Various vector comparisons generating different condition codes */
    v2di cmp_eq = (v2di)(vec1 == vec2);
    v2di cmp_ne = (v2di)(vec1 != vec2);
    v2di cmp_lt = (v2di)(vec1 < vec2);
    v2di cmp_le = (v2di)(vec1 <= vec2);
    v2di cmp_gt = (v2di)(vec1 > vec2);
    v2di cmp_ge = (v2di)(vec1 >= vec2);
    
    /* Comparisons with NaN */
    v2di cmp_nan_eq = (v2di)(vec1 == vec_nan);
    v2di cmp_nan_ne = (v2di)(vec1 != vec_nan);
    v2di cmp_nan_lt = (v2di)(vec1 < vec_nan);
    
    /* Comparisons with infinity */
    v2di cmp_inf_gt = (v2di)(vec1 > vec_inf);
    v2di cmp_inf_le = (v2di)(vec1 <= vec_inf);
    
    /* Extract results to prevent optimization */
    long long *eq_ptr = (long long*)&cmp_eq;
    long long *ne_ptr = (long long*)&cmp_ne;
    long long *nan_ne_ptr = (long long*)&cmp_nan_ne;
    
    return (int)(eq_ptr[0] ^ ne_ptr[0] ^ nan_ne_ptr[0]);
}

/* Function with inline assembly using FP condition codes */
static int asm_fp_conditions(double a, double b) {
    int result = 0;
    char unordered, equal, less, greater;
    
    /* Inline assembly that reads FP condition codes after comparison */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0\n\t"          /* Set if unordered (parity) */
        : "=r"(unordered)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %0\n\t"          /* Set if equal */
        : "=r"(equal)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0\n\t"          /* Set if below (less than) */
        : "=r"(less)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %0\n\t"          /* Set if above (greater than) */
        : "=r"(greater)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    result = (unordered << 0) | (equal << 1) | (less << 2) | (greater << 3);
    
    /* More complex inline assembly with conditional moves */
    double cmov_result;
    asm volatile (
        "ucomisd %2, %1\n\t"
        "jp 1f\n\t"            /* Jump if unordered */
        "ja 2f\n\t"            /* Jump if above (greater than) */
        "jb 3f\n\t"            /* Jump if below (less than) */
        "mov $0x3FF0000000000000, %0\n\t"  /* Equal: 1.0 */
        "jmp 4f\n"
        "1:\n\t"
        "mov $0x7FF8000000000000, %0\n\t"  /* Unordered: NaN */
        "jmp 4f\n"
        "2:\n\t"
        "mov $0x4000000000000000, %0\n\t"  /* Greater: 2.0 */
        "jmp 4f\n"
        "3:\n\t"
        "mov $0x3FE0000000000000, %0\n\t"  /* Less: 0.5 */
        "4:\n\t"
        : "=r"(cmov_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* Use the result to prevent optimization */
    result ^= (int)(*(long long*)&cmov_result);
    
    return result;
}

/* Main function that exercises all comparison patterns */
int main(void) {
    double normal1 = 1.5;
    double normal2 = 2.5;
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf_val = -__builtin_inf();
    double zero = 0.0;
    double neg_zero = -0.0;
    
    int checksum = 0;
    
    /* Test various combinations of values */
    checksum ^= stress_fp_comparisons(normal1, normal2, nan_val, inf_val, neg_inf_val);
    checksum ^= stress_fp_comparisons(normal2, normal1, nan_val, inf_val, neg_inf_val);
    checksum ^= stress_fp_comparisons(normal1, normal1, nan_val, inf_val, neg_inf_val);
    checksum ^= stress_fp_comparisons(nan_val, normal1, nan_val, inf_val, neg_inf_val);
    checksum ^= stress_fp_comparisons(inf_val, normal1, nan_val, inf_val, neg_inf_val);
    checksum ^= stress_fp_comparisons(neg_inf_val, inf_val, nan_val, inf_val, neg_inf_val);
    checksum ^= stress_fp_comparisons(zero, neg_zero, nan_val, inf_val, neg_inf_val);
    
    /* Vector comparisons */
    checksum ^= vector_fp_comparisons();
    
    /* Inline assembly comparisons */
    checksum ^= asm_fp_conditions(normal1, normal2);
    checksum ^= asm_fp_conditions(normal1, nan_val);
    checksum ^= asm_fp_conditions(inf_val, normal1);
    checksum ^= asm_fp_conditions(zero, zero);
    
    /* Additional unordered comparison tests */
    volatile double v_nan = nan_val;
    volatile double v_inf = inf_val;
    
    /* These should trigger UNORDERED, ORDERED, UNEQ, etc. */
    if (!(v_nan == v_nan)) checksum |= 0x80000000;  /* UNORDERED path */
    if (v_nan != v_nan) checksum |= 0x40000000;     /* Always true */
    
    /* Complex expression mixing ordered and unordered comparisons */
    double x = normal1;
    double y = normal2;
    int complex_result = 0;
    
    if (x < y || x != x || y != y) complex_result |= 1;  /* UNLT or UNORDERED */
    if (x > y && x == x && y == y) complex_result |= 2;  /* ORDERED and GT */
    if (!(x < y) && x == x && y == y) complex_result |= 4; /* UNGE (not less than) */
    if (!(x > y) || x != x) complex_result |= 8;         /* UNLE or UNORDERED */
    if (x != y && x == x && y == y) complex_result |= 16; /* LTGT */
    
    checksum ^= complex_result;
    
    /* Loop with FP comparisons to generate more code */
    volatile double arr[4] = {1.0, 2.0, nan_val, inf_val};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (arr[i] < arr[j]) checksum += i * j;
            if (arr[i] > arr[j]) checksum -= i * j;
            if (arr[i] == arr[j]) checksum ^= (i << 4) | j;
            if (arr[i] != arr[j]) checksum ^= ~((i << 4) | j);
        }
    }
    
    printf("Final checksum: %d (0x%08x)\n", checksum, checksum);
    
    /* Return non-zero if NaN test worked */
    return (nan_val != nan_val) ? 0 : 1;
}
