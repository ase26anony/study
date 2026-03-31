/* test_i386_cc.c - Program to trigger x86 floating-point condition code output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <stdint.h>

/* Prevent constant folding */
volatile double global_nan = NAN;
volatile double global_inf = INFINITY;
volatile double global_neg_inf = -INFINITY;
volatile double global_zero = 0.0;
volatile double global_neg_zero = -0.0;

/* Vector type for SSE/AVX comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function to parse NaN from command line */
static double parse_double(const char *str) {
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

/* Function that performs all possible floating-point comparisons */
static int compare_all(double a, double b) {
    int result = 0;
    
    /* Standard C comparisons - these may generate unordered condition codes */
    if (a < b) result |= 1;
    if (a > b) result |= 2;
    if (a <= b) result |= 4;
    if (a >= b) result |= 8;
    if (a == b) result |= 16;
    if (a != b) result |= 32;
    
    /* <math.h> macros that map directly to x86 condition codes */
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
    /* Use fpclassify to get detailed information */
    int a_class = fpclassify(a);
    int b_class = fpclassify(b);
    
    if (isunordered(a, b)) {
        return "UNORDERED";
    }
    
    /* Perform multiple comparisons to generate different condition codes */
    if (a < b) {
        if (a == -0.0 && b == 0.0) return "UNEQ (negative zero equals zero)";
        return "LT";
    }
    if (a > b) {
        return "GT";
    }
    if (a == b) {
        if (signbit(a) != signbit(b)) return "UNEQ (signed zeros)";
        return "EQ";
    }
    
    /* This should only be reached for unordered cases, but handle anyway */
    if (a != b) {
        if (islessgreater(a, b)) return "LTGT";
        return "NEQ";
    }
    
    return "UNKNOWN";
}

/* Inline assembly to force condition code output */
static int inline_asm_fp_compare(double a, double b) {
    int result = 0;
    
    /* Using x87 floating-point compare */
    asm volatile (
        "fldl %2\n\t"           /* Load b onto FPU stack */
        "fldl %1\n\t"           /* Load a onto FPU stack */
        "fucomip %%st(1), %%st\n\t"  /* Compare and pop */
        "fstp %%st(0)\n\t"      /* Pop remaining value */
        "setp %%al\n\t"         /* Set if unordered (parity flag) */
        "setb %%cl\n\t"         /* Set if below (CF=1) */
        "sete %%dl\n\t"         /* Set if equal (ZF=1) */
        "movzbl %%al, %%eax\n\t"
        "movzbl %%cl, %%ecx\n\t"
        "movzbl %%dl, %%edx\n\t"
        "shll $1, %%ecx\n\t"
        "shll $2, %%edx\n\t"
        "orl %%ecx, %%eax\n\t"
        "orl %%edx, %%eax\n\t"
        : "=a" (result)
        : "m" (a), "m" (b)
        : "cc", "st", "ecx", "edx"
    );
    
    return result;
}

/* Vector comparison function */
static void vector_comparisons(void) {
    volatile v2df vec_a, vec_b, vec_cmp;
    volatile v4sf vec_fa, vec_fb, vec_fcmp;
    
    /* Initialize vectors with mixed values */
    double a_arr[2] = {NAN, 1.0};
    double b_arr[2] = {2.0, NAN};
    float fa_arr[4] = {NAN, INFINITY, -INFINITY, 0.0f};
    float fb_arr[4] = {0.0f, -INFINITY, INFINITY, NAN};
    
    /* Load into vectors */
    vec_a = *(v2df*)a_arr;
    vec_b = *(v2df*)b_arr;
    vec_fa = *(v4sf*)fa_arr;
    vec_fb = *(v4sf*)fb_arr;
    
    /* Perform vector comparisons - these may generate condition codes */
    vec_cmp = vec_a < vec_b;
    vec_cmp = vec_a > vec_b;
    vec_cmp = vec_a <= vec_b;
    vec_cmp = vec_a >= vec_b;
    
    vec_fcmp = vec_fa < vec_fb;
    vec_fcmp = vec_fa > vec_fb;
    vec_fcmp = vec_fa <= vec_fb;
    vec_fcmp = vec_fa >= vec_fb;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "m" (vec_cmp), "m" (vec_fcmp));
}

/* Main test function */
int main(int argc, char *argv[]) {
    double test_values[10];
    int i, j;
    
    /* Parse command line arguments or use defaults */
    if (argc > 1) {
        for (i = 0; i < argc - 1 && i < 10; i++) {
            test_values[i] = parse_double(argv[i + 1]);
        }
    } else {
        /* Default test cases */
        test_values[0] = NAN;
        test_values[1] = INFINITY;
        test_values[2] = -INFINITY;
        test_values[3] = 0.0;
        test_values[4] = -0.0;
        test_values[5] = 1.0;
        test_values[6] = -1.0;
        test_values[7] = DBL_MIN;
        test_values[8] = DBL_MAX;
        test_values[9] = 3.141592653589793;
    }
    
    printf("Testing floating-point comparisons to trigger x86 condition code output...\n");
    
    /* Test all pairs */
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            double a = test_values[i];
            double b = test_values[j];
            
            /* Force compiler to generate actual comparisons */
            volatile int cmp_result = compare_all(a, b);
            volatile const char *cls = classify_comparison(a, b);
            
            /* Use inline assembly for direct condition code generation */
            volatile int asm_result = inline_asm_fp_compare(a, b);
            
            /* Prevent dead code elimination */
            if (cmp_result != 0 || cls != NULL || asm_result != 0) {
                /* Do nothing, just reference the variables */
                asm volatile("" : : "m" (cmp_result), "m" (cls), "m" (asm_result));
            }
        }
    }
    
    /* Test vector comparisons */
    vector_comparisons();
    
    /* Additional tests with volatile to prevent optimization */
    volatile double v1 = global_nan;
    volatile double v2 = 1.0;
    volatile double v3 = global_inf;
    volatile double v4 = global_neg_inf;
    
    /* Complex conditional with multiple branches */
    if (v1 < v2 || v2 > v3 || v3 <= v4 || v4 >= v1 || v1 == v2 || v2 != v3) {
        /* This should generate various condition codes */
        asm volatile("" : : "m" (v1), "m" (v2), "m" (v3), "m" (v4));
    }
    
    /* Switch-like behavior using computed goto (indirect branch) */
    {
        volatile int condition = 0;
        if (isunordered(v1, v2)) condition = 1;
        else if (v1 < v2) condition = 2;
        else if (v1 > v2) condition = 3;
        else if (v1 == v2) condition = 4;
        
        /* Force compiler to consider all branches */
        switch (condition) {
            case 1: /* UNORDERED */ break;
            case 2: /* LT */ break;
            case 3: /* GT */ break;
            case 4: /* EQ */ break;
            default: /* UNKNOWN */ break;
        }
    }
    
    printf("Test completed. Compile with: gcc -O2 -mfpmath=387 -march=i686 -S test_i386_cc.c\n");
    printf("Or with: gcc -O3 -march=native -msse2 -mno-sse4 -c test_i386_cc.c\n");
    
    return 0;
}
