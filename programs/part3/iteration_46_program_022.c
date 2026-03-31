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
    static double counter = 0.0;
    return counter++ * 1.5;
}

float get_input_float(void) {
    static float counter = 0.0f;
    return counter++ * 1.25f;
}

/* Dummy function to create side effects */
void use_result(int val) __attribute__((noinline));
void use_result(int val) {
    /* Prevent optimization */
    volatile static int sink = 0;
    sink += val;
}

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

int main(void) {
    int checksum = 0;
    
    /* 1. Initialize mixed floating-point variables */
    volatile double vd1 = 1.0;
    volatile double vd2 = 2.0;
    volatile float vf1 = 1.0f;
    volatile float vf2 = 2.0f;
    
    double d1 = get_input_double();
    double d2 = get_input_double();
    float f1 = get_input_float();
    float f2 = get_input_float();
    
    /* Mix with integer operands */
    int int_val = 3;
    
    /* 2. Perform relational operator comparisons */
    /* UNORDERED/ORDERED cases - likely with -ffast-math */
    if (d1 != d2) {  /* May generate UNEQ or LTGT */
        checksum += 1;
    }
    
    if (f1 <= f2) {  /* May generate UNLE */
        checksum += 2;
    }
    
    if (vd1 >= vd2) {  /* May generate UNGE */
        checksum += 4;
    }
    
    if (f1 > f2) {  /* May generate UNGT */
        checksum += 8;
    }
    
    if (d1 < d2) {  /* May generate UNLT */
        checksum += 16;
    }
    
    if (vf1 == vf2) {  /* May generate UNEQ */
        checksum += 32;
    }
    
    /* Mixed type comparisons */
    if (d1 != int_val) {
        checksum += 64;
    }
    
    if (f1 <= int_val) {
        checksum += 128;
    }
    
    /* 3. Use builtins for explicit unordered checks */
    checksum += __builtin_isunordered(d1, d2) ? 256 : 0;      /* UNORDERED */
    checksum += __builtin_islessgreater(f1, f2) ? 512 : 0;    /* LTGT */
    checksum += __builtin_islessequal(vd1, vd2) ? 1024 : 0;   /* UNLE */
    checksum += __builtin_isgreaterequal(f1, f2) ? 2048 : 0;  /* UNGE */
    
    /* 4. Conditional moves based on FP results */
    double cond_move1 = (d1 >= d2) ? d1 : d2;  /* May use UNGE */
    float cond_move2 = (f1 != f2) ? f1 : f2;   /* May use UNEQ or LTGT */
    
    checksum += (int)cond_move1;
    checksum += (int)cond_move2;
    
    /* Loop with FP condition */
    for (int i = 0; i < 10 && (d1 + i != d2); ++i) {
        checksum += i;
    }
    
    /* 5. Vector (SIMD) comparisons */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    
    /* Vector comparisons - may generate various condition codes */
    v4sf cmp_eq = (vec_a == vec_b);    /* May use UNEQ */
    v4sf cmp_neq = (vec_a != vec_b);   /* May use LTGT */
    v4sf cmp_lt = (vec_a < vec_b);     /* May use UNLT */
    v4sf cmp_le = (vec_a <= vec_b);    /* May use UNLE */
    v4sf cmp_gt = (vec_a > vec_b);     /* May use UNGT */
    v4sf cmp_ge = (vec_a >= vec_b);    /* May use UNGE */
    
    /* Extract results from vectors */
    for (int i = 0; i < 4; ++i) {
        checksum += cmp_eq[i] ? 1 : 0;
        checksum += cmp_neq[i] ? 2 : 0;
        checksum += cmp_lt[i] ? 4 : 0;
        checksum += cmp_le[i] ? 8 : 0;
        checksum += cmp_gt[i] ? 16 : 0;
        checksum += cmp_ge[i] ? 32 : 0;
    }
    
    /* Use x86 intrinsics for explicit unordered comparison */
    __m128 mm_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 mm_b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 mm_unord = _mm_cmpunord_ps(mm_a, mm_b);  /* UNORDERED */
    
    /* Store to prevent optimization */
    float unord_result[4];
    _mm_store_ps(unord_result, mm_unord);
    checksum += (int)unord_result[0];
    
    /* Double precision vectors */
    v2df vec_da = {d1, d2};
    v2df vec_db = {d2, d1};
    v2df cmp_d_eq = (vec_da == vec_db);
    checksum += (int)cmp_d_eq[0] + (int)cmp_d_eq[1];
    
    /* 6. Switch statement with FP comparisons */
    int switch_val = (int)(d1 * 100) % 5;
    switch (switch_val) {
        case 0:
            if (f1 < f2) checksum += 1000;
            break;
        case 1:
            if (d1 > d2) checksum += 2000;
            break;
        case 2:
            if (vf1 == vf2) checksum += 3000;
            break;
        case 3:
            if (vd1 != vd2) checksum += 4000;
            break;
        case 4:
            if (f1 >= f2) checksum += 5000;
            break;
    }
    
    /* Create observable output */
    use_result(checksum);
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
