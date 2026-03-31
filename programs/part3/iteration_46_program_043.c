/* test_condcodes.c - Target x86 condition code coverage */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double;
extern volatile float external_float;

/* Opaque function to get dynamic values */
double get_input_double(void) __attribute__((noinline));
float get_input_float(void) __attribute__((noinline));

double get_input_double(void) {
    static double counter = 0.1;
    counter += 0.3;
    return counter;
}

float get_input_float(void) {
    static float counter = 0.2f;
    counter += 0.4f;
    return counter;
}

/* Dummy function to create side effects */
void use_result(int val) __attribute__((noinline));
void use_result(int val) {
    /* Prevent optimization */
    volatile static int sink;
    sink = val;
}

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

int main(void) {
    int checksum = 0;
    
    /* Initialize mixed floating-point variables */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    volatile float vf1 = 3.0f;
    volatile float vf2 = 4.0f;
    
    double d1 = get_input_double();
    double d2 = get_input_double();
    float f1 = get_input_float();
    float f2 = get_input_float();
    
    /* ===== SCALAR COMPARISONS WITH RELATIONAL OPERATORS ===== */
    
    /* UNORDERED/ORDERED patterns - using fast-math assumptions */
    if (d1 != d2) {  /* May generate UNEQ or LTGT */
        checksum += 1;
    }
    
    if (vd1 <= vd2) {  /* May generate UNLE */
        checksum += 2;
    }
    
    if (f1 >= f2) {  /* May generate UNGE */
        checksum += 3;
    }
    
    if (vf1 < vf2) {  /* May generate UNLT */
        checksum += 4;
    }
    
    if (d1 > d2) {  /* May generate UNGT */
        checksum += 5;
    }
    
    /* Mixed float/double comparisons */
    if ((double)f1 == d1) {  /* May generate UNEQ */
        checksum += 6;
    }
    
    /* ===== EXPLICIT BUILTIN FUNCTIONS ===== */
    
    /* Direct unordered check - should generate UNORDERED */
    if (__builtin_isunordered(d1, d2)) {
        checksum += 7;
    }
    
    /* Ordered check - should generate ORDERED */
    if (__builtin_isordered(f1, f2)) {
        checksum += 8;
    }
    
    /* Less-greater check - should generate LTGT */
    if (__builtin_islessgreater(vd1, vd2)) {
        checksum += 9;
    }
    
    /* ===== CONDITIONAL MOVES BASED ON FP COMPARISONS ===== */
    
    /* Conditional move with UNGE/UNLE patterns */
    double cmov_result = (d1 >= d2) ? d1 : d2;  /* May use UNGE */
    checksum += (int)(cmov_result * 10);
    
    float f_cmov = (f1 <= f2) ? f1 : f2;  /* May use UNLE */
    checksum += (int)(f_cmov * 20);
    
    /* ===== VECTOR (SIMD) COMPARISONS ===== */
    
    /* GCC vector extensions */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    
    /* Vector comparisons generate various condition codes */
    v4sf cmp_eq = (vec_a == vec_b);  /* May use UNEQ */
    v4sf cmp_neq = (vec_a != vec_b); /* May use LTGT */
    v4sf cmp_lt = (vec_a < vec_b);   /* May use UNLT */
    v4sf cmp_ge = (vec_a >= vec_b);  /* May use UNGE */
    
    /* Extract results to scalar checksum */
    float* eq_ptr = (float*)&cmp_eq;
    for (int i = 0; i < 4; i++) {
        if (eq_ptr[i] != 0.0f) checksum += 30 + i;
    }
    
    /* x86 intrinsics for explicit unordered comparison */
    __m128 sse_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    
    /* _CMP_UNORD_Q generates UNORDERED condition */
    __m128 unord_cmp = _mm_cmpunord_ps(sse_a, sse_b);
    
    /* Check unordered results */
    float* unord_ptr = (float*)&unord_cmp;
    for (int i = 0; i < 4; i++) {
        if (unord_ptr[i] != 0.0f) checksum += 40 + i;
    }
    
    /* Double precision vector */
    v2df vec_da = {d1, d2};
    v2df vec_db = {d2, d1};
    v2df cmp_dneq = (vec_da != vec_db);  /* May use LTGT */
    
    /* ===== LOOP WITH FP CONDITION ===== */
    
    /* Create array with potential NaN/Inf values */
    double arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = get_input_double() * ((i % 3) - 1);  /* Can produce negative/zero */
    }
    
    /* Loop condition with != comparison - may generate UNEQ/LTGT */
    int count = 0;
    for (int i = 0; i < 10 && (arr[i] != 0.0); i++) {
        count++;
    }
    checksum += count * 100;
    
    /* ===== SWITCH BASED ON FP COMPARISON RESULTS ===== */
    
    /* Create multiple comparison sites */
    int cmp_results = 0;
    cmp_results |= (d1 < d2) ? 0x1 : 0;    /* UNLT */
    cmp_results |= (d1 > d2) ? 0x2 : 0;    /* UNGT */
    cmp_results |= (f1 == f2) ? 0x4 : 0;   /* UNEQ */
    cmp_results |= (f1 != f2) ? 0x8 : 0;   /* LTGT */
    
    /* Use in switch to create multiple branches */
    switch (cmp_results & 0xF) {
        case 0x1: checksum += 1000; break;  /* UNLT path */
        case 0x2: checksum += 2000; break;  /* UNGT path */
        case 0x4: checksum += 3000; break;  /* UNEQ path */
        case 0x8: checksum += 4000; break;  /* LTGT path */
        default:  checksum += 5000; break;  /* Other combinations */
    }
    
    /* ===== FINAL OUTPUT ===== */
    
    /* Create observable output to prevent elimination */
    printf("Condition code test checksum: %d\n", checksum);
    use_result(checksum);
    
    return checksum & 0xFF;  /* Return non-constant */
}
