/* test_condcodes.c - Target x86 condition code mnemonics for i386.cc coverage */

#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent optimization of inputs */
extern volatile double get_double_input(void) __attribute__((noinline));
extern volatile float get_float_input(void) __attribute__((noinline));
extern void use_result(int) __attribute__((noinline));

/* Vector types for SIMD comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Opaque function to prevent constant folding */
volatile double get_double_input(void) {
    static volatile double counter = 0.0;
    return counter++ * 1.5;
}

volatile float get_float_input(void) {
    static volatile float counter = 0.0f;
    return counter++ * 1.25f;
}

void use_result(int val) {
    /* Create side effect to prevent optimization */
    static volatile int sink;
    sink = val;
}

int main(void) {
    int checksum = 0;
    
    /* 1. Initialize floating-point variables with mixed sources */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    double d1 = get_double_input();
    double d2 = get_double_input() + 3.14159;
    float f1 = get_float_input();
    float f2 = get_float_input() * 2.0f;
    int i1 = 42;
    
    /* 2. Perform scalar floating-point comparisons with relational operators */
    /* These should generate various condition codes with -ffast-math */
    
    /* UNORDERED case - may generate "unord" */
    if (d1 != d1) { /* NaN check */
        checksum += 1;
    }
    
    /* UNEQ case - may generate "ueq" */
    if (f1 == f2) {
        checksum += 2;
    }
    
    /* UNGE case - may generate "nlt" */
    double z1 = (d1 >= d2) ? d1 : d2;
    checksum += (int)(z1 > 0);
    
    /* UNGT case - may generate "nle" */
    if (d1 > d2) {
        checksum += 4;
    }
    
    /* UNLE case - may generate "ule" */
    float z2 = (f1 <= f2) ? f1 : f2;
    checksum += (int)(z2 * 10);
    
    /* UNLT case - may generate "ult" */
    if (vd1 < vd2) {
        checksum += 8;
    }
    
    /* LTGT case - may generate "une" */
    if (d1 != d2) {
        checksum += 16;
    }
    
    /* Mixed float/double/int comparisons */
    if ((double)f1 > d2) {
        checksum += 32;
    }
    
    if (f1 != (float)i1) {
        checksum += 64;
    }
    
    /* 3. Use builtins for explicit unordered checks */
    /* Directly maps to specific condition codes */
    
    /* __builtin_isunordered -> UNORDERED */
    if (__builtin_isunordered(d1, d2)) {
        checksum += 128;
    }
    
    /* __builtin_islessgreater -> LTGT */
    if (__builtin_islessgreater(f1, f2)) {
        checksum += 256;
    }
    
    /* Other unordered builtins */
    checksum += __builtin_islessequal(d1, d2) ? 512 : 0;  /* UNLE? */
    checksum += __builtin_isgreaterequal(f1, f2) ? 1024 : 0; /* UNGE? */
    
    /* 4. Vector (SIMD) comparisons */
    /* Initialize vector variables */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df vec_da = {d1, d2};
    v2df vec_db = {d2, d1};
    
    /* Vector comparisons - may generate various condition codes */
    v4sf cmp_eq = (vec_a == vec_b);      /* May use UNEQ */
    v4sf cmp_neq = (vec_a != vec_b);     /* May use LTGT */
    v4sf cmp_lt = (vec_a < vec_b);       /* May use UNLT */
    v4sf cmp_gt = (vec_a > vec_b);       /* May use UNGT */
    v4sf cmp_le = (vec_a <= vec_b);      /* May use UNLE */
    v4sf cmp_ge = (vec_a >= vec_b);      /* May use UNGE */
    
    /* Extract results to prevent optimization */
    float sum = 0.0f;
    for (int i = 0; i < 4; i++) {
        sum += cmp_eq[i] + cmp_neq[i] + cmp_lt[i] + cmp_gt[i] + cmp_le[i] + cmp_ge[i];
    }
    checksum += (int)sum;
    
    /* Double vector comparisons */
    v2df cmp_d_eq = (vec_da == vec_db);
    v2df cmp_d_neq = (vec_da != vec_db);
    checksum += (int)(cmp_d_eq[0] + cmp_d_eq[1] + cmp_d_neq[0] + cmp_d_neq[1]);
    
    /* 5. Use x86 intrinsics for explicit unordered comparison */
    /* Direct SSE intrinsic for unordered compare */
    __m128 a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 cmp_unord = _mm_cmpunord_ps(a, b);  /* Explicit UNORDERED */
    
    /* Extract mask from comparison result */
    int mask = _mm_movemask_ps(cmp_unord);
    checksum += mask;
    
    /* 6. Loop with floating-point condition */
    /* Creates multiple emission sites for condition codes */
    volatile float arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = get_float_input();
    }
    
    int count = 0;
    for (int i = 0; i < 10 && (arr[i] != 0.0f); i++) {
        count++;
    }
    checksum += count;
    
    /* Switch based on comparison results */
    int cmp_result = 0;
    cmp_result += (d1 < d2) ? 1 : 0;
    cmp_result += (f1 > f2) ? 2 : 0;
    cmp_result += (d1 == d2) ? 4 : 0;
    cmp_result += (f1 != f2) ? 8 : 0;
    
    switch (cmp_result & 3) {
        case 0:
            checksum += 1000;
            break;
        case 1:
            checksum += 2000;  /* UNLT path */
            break;
        case 2:
            checksum += 3000;  /* UNGT path */
            break;
        case 3:
            checksum += 4000;
            break;
    }
    
    /* 7. Final output to prevent dead code elimination */
    use_result(checksum);
    printf("Checksum: %d\n", checksum);
    
    return checksum > 0 ? 0 : 1;
}
