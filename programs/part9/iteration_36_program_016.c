#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent constant folding and optimization */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;

/* Vector type for SSE/AVX comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function that performs all possible floating-point comparisons */
static int compare_all_conditions(double a, double b) {
    int result = 0;
    
    /* Standard C comparison operators - may generate unordered conditions */
    if (a < b) result |= 1;
    if (a > b) result |= 2;
    if (a <= b) result |= 4;
    if (a >= b) result |= 8;
    if (a == b) result |= 16;
    if (a != b) result |= 32;
    
    /* <math.h> macros that map to x86 unordered comparison predicates */
    if (isunordered(a, b)) result |= 64;
    if (isless(a, b)) result |= 128;
    if (isgreater(a, b)) result |= 256;
    if (islessequal(a, b)) result |= 512;
    if (isgreaterequal(a, b)) result |= 1024;
    if (islessgreater(a, b)) result |= 2048;
    
    return result;
}

/* Function with switch based on comparison results */
static const char* classify_comparison(double a, double b) {
    /* Check for unordered first */
    if (isunordered(a, b)) {
        /* Further classification within unordered cases */
        if (isless(a, b) || isgreater(a, b)) {
            return "LTGT (unordered and not equal)";
        }
        return "UNORDERED";
    }
    
    /* Ordered comparisons */
    if (a < b) return "LESS";
    if (a > b) return "GREATER";
    if (a == b) return "EQUAL";
    
    /* Special cases for -0.0 and 0.0 */
    if (a == 0.0 && b == 0.0) {
        uint64_t a_bits, b_bits;
        memcpy(&a_bits, &a, sizeof(double));
        memcpy(&b_bits, &b, sizeof(double));
        if ((a_bits ^ b_bits) == (1ULL << 63)) {
            return "EQUAL (with different signs)";
        }
    }
    
    return "UNKNOWN";
}

/* Inline assembly to force generation of condition code mnemonics */
static int inline_asm_fp_compare(double a, double b) {
    int result;
    
    /* Using fucomip which sets flags including parity for unordered */
    asm volatile (
        "fucomip %%st(1), %%st\n\t"
        "setp %%al\n\t"
        "setb %%ah\n\t"
        "sete %%dl\n\t"
        "movzbl %%al, %%eax\n\t"
        "movzbl %%ah, %%edx\n\t"
        "shl $8, %%edx\n\t"
        "orl %%edx, %%eax"
        : "=a"(result)
        : "t"(a), "u"(b)
        : "cc", "st", "dl"
    );
    
    return result;
}

/* Another inline assembly variant with different condition codes */
static int inline_asm_fcomip(double a, double b) {
    int flags;
    
    asm volatile (
        "fcomip %%st(1), %%st\n\t"
        "pushf\n\t"
        "pop %0"
        : "=r"(flags)
        : "t"(a), "u"(b)
        : "cc", "st"
    );
    
    return flags;
}

/* Vector comparison function */
static void vector_comparisons(void) {
    v2df vec_a = {1.0, 2.0};
    v2df vec_b = {NAN, 3.0};
    v2df vec_c = {global_nan, global_inf};
    
    /* These vector comparisons may generate multiple condition codes */
    v2df cmp_lt = vec_a < vec_b;
    v2df cmp_gt = vec_a > vec_b;
    v2df cmp_eq = vec_a == vec_b;
    v2df cmp_ne = vec_a != vec_b;
    
    /* Use volatile to prevent optimization */
    volatile v2df volatile_cmp = cmp_lt;
    (void)volatile_cmp;
    volatile_cmp = cmp_gt;
    volatile_cmp = cmp_eq;
    volatile_cmp = cmp_ne;
    
    /* Float vector comparisons */
    v4sf fvec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec_b = {NAN, 1.0f, INFINITY, -INFINITY};
    
    v4sf fcmp = fvec_a < fvec_b;
    volatile v4sf volatile_fcmp = fcmp;
    (void)volatile_fcmp;
}

/* Parse double from string, handling special cases */
static double parse_double(const char* str) {
    if (strcmp(str, "nan") == 0 || strcmp(str, "NAN") == 0) {
        return NAN;
    }
    if (strcmp(str, "inf") == 0 || strcmp(str, "INF") == 0) {
        return INFINITY;
    }
    if (strcmp(str, "-inf") == 0 || strcmp(str, "-INF") == 0) {
        return -INFINITY;
    }
    return atof(str);
}

int main(int argc, char** argv) {
    /* Test cases including NaN, infinity, and normal numbers */
    double test_cases[][2] = {
        {NAN, 1.0},
        {1.0, NAN},
        {NAN, NAN},
        {INFINITY, 1.0},
        {1.0, -INFINITY},
        {INFINITY, -INFINITY},
        {0.0, -0.0},
        {1.0, 2.0},
        {2.0, 1.0},
        {1.0, 1.0},
        {global_nan, global_inf},
        {global_neg_inf, global_nan}
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    /* Use command line arguments if provided */
    if (argc >= 3) {
        double a = parse_double(argv[1]);
        double b = parse_double(argv[2]);
        
        printf("Command line test: a=%g, b=%g\n", a, b);
        
        /* Force all comparison types */
        int cmp_result = compare_all_conditions(a, b);
        const char* classification = classify_comparison(a, b);
        
        printf("Comparison result mask: 0x%x\n", cmp_result);
        printf("Classification: %s\n", classification);
        
        /* Use inline assembly */
        int asm_result = inline_asm_fp_compare(a, b);
        int flags = inline_asm_fcomip(a, b);
        
        printf("Inline ASM result: 0x%x\n", asm_result);
        printf("Flags from fcomip: 0x%x\n", flags);
    }
    
    /* Run through all test cases */
    printf("\nRunning comprehensive test suite:\n");
    printf("===============================\n");
    
    for (int i = 0; i < num_cases; i++) {
        double a = test_cases[i][0];
        double b = test_cases[i][1];
        
        /* Volatile to prevent constant folding */
        volatile double volatile_a = a;
        volatile double volatile_b = b;
        
        printf("\nTest case %d: a=%g, b=%g\n", i, volatile_a, volatile_b);
        
        /* Perform comparisons - results may be unused but operations must happen */
        int result = compare_all_conditions(volatile_a, volatile_b);
        const char* cls = classify_comparison(volatile_a, volatile_b);
        
        /* Use inline assembly */
        int asm_res = inline_asm_fp_compare(volatile_a, volatile_b);
        
        /* Prevent dead code elimination */
        asm volatile("" : : "r"(result), "r"(asm_res) : "memory");
        
        printf("  Classification: %s\n", cls);
        printf("  Result mask: 0x%04x\n", result);
    }
    
    /* Vector comparisons */
    printf("\nPerforming vector comparisons:\n");
    printf("=============================\n");
    vector_comparisons();
    
    /* Complex control flow with mixed comparisons */
    printf("\nComplex control flow test:\n");
    printf("=========================\n");
    
    double values[] = {NAN, INFINITY, -INFINITY, 0.0, 1.0, -1.0, 2.0};
    int num_values = sizeof(values) / sizeof(values[0]);
    
    int total_classifications = 0;
    for (int i = 0; i < num_values; i++) {
        for (int j = 0; j < num_values; j++) {
            volatile double vi = values[i];
            volatile double vj = values[j];
            
            /* Complex if-else chain that may generate various condition codes */
            if (isunordered(vi, vj)) {
                if (vi < vj || vi > vj) {
                    total_classifications |= 1; /* LTGT */
                } else if (vi == vj) {
                    total_classifications |= 2; /* UNEQ */
                } else if (!(vi < vj)) {
                    total_classifications |= 4; /* UNGE or NLT */
                } else if (!(vi > vj)) {
                    total_classifications |= 8; /* UNLE */
                } else if (!(vi <= vj)) {
                    total_classifications |= 16; /* UNGT or NLE */
                } else if (!(vi >= vj)) {
                    total_classifications |= 32; /* UNLT */
                }
            } else {
                if (vi < vj) total_classifications |= 64;
                if (vi > vj) total_classifications |= 128;
                if (vi == vj) total_classifications |= 256;
                if (vi != vj) total_classifications |= 512;
            }
        }
    }
    
    printf("Total classifications mask: 0x%08x\n", total_classifications);
    
    /* Final summary to prevent optimization */
    volatile int final_result = total_classifications;
    printf("\nFinal result (preventing optimization): %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
