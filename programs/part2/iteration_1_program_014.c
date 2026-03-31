/* Compile with: gcc -std=c99 -O2 -march=x86-64 -mtune=generic -ffp-contract=off -o fp_conds fp_conds.c */
/* Also try: gcc -std=c99 -O3 -msse4.2 -ftree-vectorize -fno-trapping-math -o fp_conds_vec fp_conds.c */
/* And: gcc -std=c99 -O1 -m32 -mfpmath=387 -fno-inline -o fp_conds_32 fp_conds.c */

#include <stdint.h>
#include <string.h>

/* Force volatile to prevent constant folding */
static volatile double d_nan = __builtin_nan("");
static volatile double d_inf = __builtin_inf();
static volatile double d_ninf = -__builtin_inf();
static volatile double d_zero = 0.0;
static volatile double d_one = 1.0;
static volatile double d_two = 2.0;
static volatile double d_neg = -1.0;

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Function with complex control flow using all FP comparison conditions */
int fp_comparison_stress(double a, double b, double c, double d) {
    volatile double v1 = a;
    volatile double v2 = b;
    volatile double v3 = c;
    volatile double v4 = d;
    
    int result = 0;
    
    /* Matrix of comparisons to trigger different condition codes */
    
    /* UNORDERED cases - comparisons involving NaN */
    if (v1 != v1) { /* NaN != NaN is true */
        result |= 1;
    }
    
    if (v1 == d_nan) { /* UNORDERED/UNEQ path */
        result |= 2;
    }
    
    if (d_nan < v2) { /* UNORDERED/UNLT path */
        result |= 4;
    }
    
    if (v2 > d_nan) { /* UNORDERED/UNGT path */
        result |= 8;
    }
    
    if (d_nan <= v3) { /* UNORDERED/UNLE path */
        result |= 16;
    }
    
    if (v3 >= d_nan) { /* UNORDERED/UNGE path */
        result |= 32;
    }
    
    /* ORDERED cases - normal comparisons */
    if (v1 == v2) { /* EQ */
        result |= 64;
    }
    
    if (v1 != v2) { /* NEQ / LTGT */
        result |= 128;
    }
    
    if (v1 < v2) { /* LT */
        result |= 256;
    }
    
    if (v1 <= v2) { /* LE */
        result |= 512;
    }
    
    if (v1 > v2) { /* GT */
        result |= 1024;
    }
    
    if (v1 >= v2) { /* GE */
        result |= 2048;
    }
    
    /* Complex conditional expressions using ?: operator */
    double cond_val = (v1 < v2) ? v3 : v4;
    result += (int)cond_val;
    
    cond_val = (v1 != v1) ? d_nan : d_one; /* UNORDERED check */
    result += (int)cond_val;
    
    cond_val = (v1 == v1) ? d_one : d_nan; /* ORDERED check */
    result += (int)cond_val;
    
    /* Goto-based control flow to prevent optimization */
    if (v1 < v2) goto label_lt;
    if (v1 > v2) goto label_gt;
    if (v1 == v2) goto label_eq;
    goto label_unordered;
    
label_lt:
    result += 4096;
    goto label_continue;
    
label_gt:
    result += 8192;
    goto label_continue;
    
label_eq:
    result += 16384;
    goto label_continue;
    
label_unordered:
    result += 32768;
    
label_continue:
    
    /* More unordered comparisons with different NaN values */
    volatile double local_nan = __builtin_nan("0xdead");
    if (local_nan == local_nan) { /* Always false */
        result += 65536;
    }
    
    if (local_nan != local_nan) { /* Always true */
        result += 131072;
    }
    
    /* Comparisons with infinity */
    if (v1 == d_inf) {
        result += 262144;
    }
    
    if (v1 < d_inf) { /* All finite numbers < inf */
        result += 524288;
    }
    
    if (d_ninf < v1) { /* -inf < all finite numbers */
        result += 1048576;
    }
    
    return result;
}

/* Vectorized FP comparisons */
void vector_fp_comparisons(double *arr1, double *arr2, int *mask, int n) {
    /* Use vector extensions for SIMD comparisons */
    for (int i = 0; i < n; i += 2) {
        v2df v1, v2;
        memcpy(&v1, &arr1[i], sizeof(v2df));
        memcpy(&v2, &arr2[i], sizeof(v2df));
        
        /* Generate various comparison masks */
        v2di cmp_eq = (v2di)(v1 == v2);      /* EQ */
        v2di cmp_ne = (v2di)(v1 != v2);      /* NE / LTGT */
        v2di cmp_lt = (v2di)(v1 < v2);       /* LT */
        v2di cmp_le = (v2di)(v1 <= v2);      /* LE */
        v2di cmp_gt = (v2di)(v1 > v2);       /* GT */
        v2di cmp_ge = (v2di)(v1 >= v2);      /* GE */
        
        /* Store results */
        long long *eq_ptr = (long long*)&cmp_eq;
        mask[i] |= (int)eq_ptr[0];
        if (i+1 < n) mask[i+1] |= (int)eq_ptr[1];
    }
}

/* Function with inline assembly using condition codes */
int asm_fp_conditions(double a, double b) {
    int result = 0;
    unsigned char unordered, equal, less, greater;
    
    /* Inline assembly that reads FP condition codes */
    asm volatile (
        "ucomisd %[b], %[a]\n\t"
        "setp %[unordered]\n\t"      /* UNORDERED */
        "sete %[equal]\n\t"          /* EQ */
        "setb %[less]\n\t"           /* LT / UNLT */
        "seta %[greater]"            /* GT / UNGT */
        : [unordered]"=r"(unordered),
          [equal]"=r"(equal),
          [less]"=r"(less),
          [greater]"=r"(greater)
        : [a]"x"(a), [b]"x"(b)
        : "cc"
    );
    
    result = unordered | (equal << 1) | (less << 2) | (greater << 3);
    
    /* Conditional move based on FP comparison */
    double cmov_result;
    asm volatile (
        "ucomisd %[b], %[a]\n\t"
        "fcmovbe %[alt], %[result]"
        : [result]"=t"(cmov_result)
        : [a]"t"(a), [b]"t"(b), [alt]"t"(d_one)
        : "cc"
    );
    
    result += (int)cmov_result;
    
    return result;
}

/* Main function that exercises all paths */
int main() {
    double test_values[] = {
        d_zero, d_one, d_two, d_neg, d_inf, d_ninf, d_nan
    };
    int n_values = sizeof(test_values) / sizeof(test_values[0]);
    
    int total_result = 0;
    
    /* Exhaustive matrix of comparisons */
    for (int i = 0; i < n_values; i++) {
        for (int j = 0; j < n_values; j++) {
            total_result += fp_comparison_stress(
                test_values[i],
                test_values[j],
                test_values[(i+1) % n_values],
                test_values[(j+1) % n_values]
            );
        }
    }
    
    /* Vectorized comparisons */
    double arr1[16], arr2[16];
    int masks[16] = {0};
    
    for (int i = 0; i < 16; i++) {
        arr1[i] = (i % 2 == 0) ? (double)i : -((double)i);
        arr2[i] = (i % 3 == 0) ? d_nan : (double)(i * 2);
    }
    
    vector_fp_comparisons(arr1, arr2, masks, 16);
    
    for (int i = 0; i < 16; i++) {
        total_result += masks[i];
    }
    
    /* Inline assembly tests */
    for (int i = 0; i < n_values; i++) {
        for (int j = 0; j < n_values; j++) {
            total_result += asm_fp_conditions(test_values[i], test_values[j]);
        }
    }
    
    /* Additional unordered comparison patterns */
    volatile double special_nan = __builtin_nan("0x1234");
    volatile double special_inf = __builtin_inf();
    
    /* UNEQ: unordered or equal */
    if (!(special_nan < test_values[0]) && !(special_nan > test_values[0])) {
        total_result += 1;
    }
    
    /* UNGE: unordered or greater-or-equal */
    if (!(test_values[1] < special_nan)) {
        total_result += 2;
    }
    
    /* UNGT: unordered or greater */
    if (!(test_values[2] <= special_nan)) {
        total_result += 4;
    }
    
    /* UNLE: unordered or less-or-equal */
    if (!(special_nan > test_values[3])) {
        total_result += 8;
    }
    
    /* UNLT: unordered or less */
    if (!(special_nan >= test_values[4])) {
        total_result += 16;
    }
    
    /* LTGT: less or greater (ordered and not equal) */
    if (test_values[5] < test_values[6] || test_values[5] > test_values[6]) {
        total_result += 32;
    }
    
    /* Prevent dead code elimination */
    volatile int *sink = &total_result;
    asm volatile ("" : "+r"(*sink));
    
    return total_result != 0 ? 0 : 1;
}
