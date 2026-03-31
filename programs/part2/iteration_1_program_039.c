/* fp_condition_stress.c - Exhaustive test of FP comparison condition codes */
#include <stdint.h>
#include <string.h>

/* Prevent constant folding and optimization */
static volatile double sink;

/* Function to stress FP comparisons with various conditions */
void stress_fp_comparisons(double a, double b, double nan_val, double inf_val, double neg_inf_val) {
    volatile double x = a;
    volatile double y = b;
    volatile double nan = nan_val;
    volatile double inf = inf_val;
    volatile double neg_inf = neg_inf_val;
    
    int result = 0;
    
    /* Label for goto-based control flow */
    start_comparisons:
    
    /* 1. Exhaustive scalar comparisons with all relational operators */
    /* These should generate various condition codes */
    
    /* UNORDERED cases (comparisons involving NaN) */
    if (x != x) result |= 1;           /* UNORDERED check */
    if (nan == nan) result |= 2;       /* Always false, but tests UNORDERED */
    if (x < nan) result |= 4;          /* UNORDERED */
    if (nan <= x) result |= 8;         /* UNORDERED */
    if (x > nan) result |= 16;         /* UNORDERED */
    if (nan >= x) result |= 32;        /* UNORDERED */
    
    /* ORDERED cases (normal comparisons) */
    if (x == y) result |= 64;          /* EQ/UNEQ */
    if (x != y) result |= 128;         /* NEQ/LTGT */
    if (x < y) result |= 256;          /* LT/UNLT */
    if (x <= y) result |= 512;         /* LE/UNLE */
    if (x > y) result |= 1024;         /* GT/UNGT */
    if (x >= y) result |= 2048;        /* GE/UNGE */
    
    /* Comparisons with infinity */
    if (x == inf) result |= 4096;      /* EQ with infinity */
    if (x < inf) result |= 8192;       /* LT with infinity (always true for finite) */
    if (neg_inf < x) result |= 16384;  /* GT with -inf (always true for finite) */
    
    /* 2. Conditional moves using FP comparison results */
    /* These often generate conditional move instructions with condition codes */
    double cmov_result;
    cmov_result = (x < y) ? x : y;     /* May generate conditional move */
    cmov_result = (x != x) ? nan : x;  /* NaN check -> conditional move */
    cmov_result = (x == inf) ? inf : x;
    sink = cmov_result;
    
    /* 3. Complex conditional expressions */
    /* Force generation of multiple condition code checks */
    if ((x < y) && (x != nan) && (y != nan)) {
        result |= 32768;
    }
    
    if ((x >= y) || (x == inf) || (y == neg_inf)) {
        result |= 65536;
    }
    
    /* 4. Switch based on FP comparison results */
    /* This creates complex control flow */
    int cmp_result = 0;
    if (x < y) cmp_result = 1;
    else if (x > y) cmp_result = 2;
    else if (x == y) cmp_result = 3;
    else cmp_result = 4;  /* UNORDERED */
    
    switch (cmp_result) {
        case 1: result |= 131072; break;  /* LT */
        case 2: result |= 262144; break;  /* GT */
        case 3: result |= 524288; break;  /* EQ */
        case 4: result |= 1048576; break; /* UNORDERED */
    }
    
    /* 5. Inline assembly with explicit condition code usage */
    /* Directly exercises the condition code output logic */
    int cc_result;
    
    /* UNORDERED/ORDERED test */
    asm volatile (
        "ucomisd %[op1], %[op2]\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %[res]"
        : [res] "=r" (cc_result)
        : [op1] "x" (x), [op2] "x" (nan)
        : "al", "cc"
    );
    result |= (cc_result << 20);
    
    /* LT/GT test */
    asm volatile (
        "ucomisd %[op1], %[op2]\n\t"
        "seta %%al\n\t"
        "movzbl %%al, %[res]"
        : [res] "=r" (cc_result)
        : [op1] "x" (x), [op2] "x" (y)
        : "al", "cc"
    );
    result |= (cc_result << 21);
    
    /* EQ/NEQ test */
    asm volatile (
        "ucomisd %[op1], %[op2]\n\t"
        "sete %%al\n\t"
        "movzbl %%al, %[res]"
        : [res] "=r" (cc_result)
        : [op1] "x" (x), [op2] "x" (y)
        : "al", "cc"
    );
    result |= (cc_result << 22);
    
    /* 6. Vectorized FP comparisons using GCC vector extensions */
    /* These generate cmppd/cmpsd instructions with condition codes */
    typedef double v2df __attribute__((vector_size(16)));
    typedef long long v2di __attribute__((vector_size(16)));
    
    v2df vec_a = {x, y};
    v2df vec_b = {y, x};
    v2df vec_nan = {nan, nan};
    
    /* Vector comparisons generate various condition codes */
    v2di mask_eq = (v2di)(vec_a == vec_b);      /* EQ/UNEQ */
    v2di mask_lt = (v2di)(vec_a < vec_b);       /* LT/UNLT */
    v2di mask_le = (v2di)(vec_a <= vec_b);      /* LE/UNLE */
    v2di mask_gt = (v2di)(vec_a > vec_b);       /* GT/UNGT */
    v2di mask_ge = (v2di)(vec_a >= vec_b);      /* GE/UNGE */
    v2di mask_neq = (v2di)(vec_a != vec_b);     /* NEQ/LTGT */
    
    /* Unordered comparisons with NaN */
    v2di mask_unord = (v2di)(vec_a != vec_a);   /* UNORDERED */
    v2di mask_nan_cmp = (v2di)(vec_a < vec_nan); /* UNORDERED */
    
    /* Use the results to prevent optimization */
    long long *mask_ptr = (long long*)&mask_eq;
    result += mask_ptr[0] + mask_ptr[1];
    
    /* 7. Loop with FP comparisons to generate more code */
    volatile double arr[4] = {x, y, nan, inf};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (arr[i] < arr[j]) result++;
            if (arr[i] > arr[j]) result++;
            if (arr[i] == arr[j]) result++;
            if (arr[i] != arr[j]) result++;
            
            /* Force unordered checks */
            if (arr[i] != arr[i]) result++;  /* UNORDERED */
            if (arr[j] != arr[j]) result++;  /* UNORDERED */
        }
    }
    
    /* 8. Goto-based control flow to prevent simplification */
    /* Jump between different comparison blocks */
    static int counter = 0;
    counter++;
    
    if (counter % 3 == 0) {
        goto ordered_checks;
    } else if (counter % 3 == 1) {
        goto unordered_checks;
    } else {
        goto end_comparisons;
    }
    
ordered_checks:
    /* More ordered comparisons */
    if (x == 0.0) result++;
    if (y != 0.0) result++;
    if (x < 1.0) result++;
    if (y > -1.0) result++;
    goto end_comparisons;
    
unordered_checks:
    /* More unordered comparisons */
    if (nan == x) result++;
    if (x != nan) result++;
    if (nan < x) result++;
    if (x > nan) result++;
    /* Fall through */
    
end_comparisons:
    sink = result;
    
    /* 9. Final complex expression with all condition types */
    /* This should generate code for all condition codes */
    int final_check = 
        (x == y) ? 1 : 0 +           /* EQ/UNEQ */
        (x != y) ? 2 : 0 +           /* NEQ/LTGT */
        (x < y) ? 4 : 0 +            /* LT/UNLT */
        (x <= y) ? 8 : 0 +           /* LE/UNLE */
        (x > y) ? 16 : 0 +           /* GT/UNGT */
        (x >= y) ? 32 : 0 +          /* GE/UNGE */
        (x != x) ? 64 : 0 +          /* UNORDERED */
        (y == y) ? 128 : 0 +         /* ORDERED */
        (x == inf) ? 256 : 0 +       /* EQ with infinity */
        (x < inf) ? 512 : 0;         /* LT with infinity */
    
    sink = final_check;
}

/* Main function that sets up various FP values and calls stress function */
int main() {
    /* Initialize various floating-point values */
    double normal1 = 3.141592653589793;
    double normal2 = 2.718281828459045;
    double zero = 0.0;
    double neg_zero = -0.0;
    double infinity = __builtin_inf();
    double neg_infinity = -__builtin_inf();
    double nan_value = __builtin_nan("");
    
    /* Call stress function multiple times with different arguments */
    /* to ensure different code paths are taken */
    stress_fp_comparisons(normal1, normal2, nan_value, infinity, neg_infinity);
    stress_fp_comparisons(zero, neg_zero, nan_value, infinity, neg_infinity);
    stress_fp_comparisons(infinity, neg_infinity, nan_value, infinity, neg_infinity);
    stress_fp_comparisons(normal1, nan_value, nan_value, infinity, neg_infinity);
    stress_fp_comparisons(nan_value, nan_value, nan_value, infinity, neg_infinity);
    
    /* Additional tests with different values */
    double values[] = {1.0, -1.0, 100.0, -100.0, 1e-10, -1e-10};
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            stress_fp_comparisons(values[i], values[j], nan_value, infinity, neg_infinity);
        }
    }
    
    /* Return something based on sink to prevent dead code elimination */
    return (int)sink & 0xFF;
}
