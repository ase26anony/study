/* test_condcodes.c - Target x86 condition code coverage */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Prevent constant folding */
extern volatile double external_double __attribute__((weak));
extern volatile float external_float __attribute__((weak));

/* Opaque function to get dynamic values */
double __attribute__((noinline)) get_double_input(int idx) {
    static volatile double values[] = {1.0, -1.0, 0.0, 3.14, -2.71};
    return values[idx % 5];
}

float __attribute__((noinline)) get_float_input(int idx) {
    static volatile float values[] = {1.0f, -1.0f, 0.0f, 2.5f, -3.5f};
    return values[idx % 5];
}

/* Vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Dummy function to prevent optimization */
void __attribute__((noinline)) use_result(int val) {
    volatile static int sink;
    sink = val;
}

int main(void) {
    int checksum = 0;
    
    /* 1. Scalar floating-point comparisons with -ffast-math assumptions */
    volatile double vx = 1.0 / 0.0;  /* Potential INF */
    volatile double vy = 0.0 / 0.0;  /* Potential NaN */
    
    double dx = get_double_input(0);
    double dy = get_double_input(1);
    float fx = get_float_input(0);
    float fy = get_float_input(1);
    
    /* These should generate various condition codes with -ffast-math */
    
    /* UNORDERED: x unord y (either is NaN) */
    if (__builtin_isunordered(dx, dy)) checksum ^= 1;
    if (vx != vx) checksum ^= 2;  /* NaN self-comparison */
    
    /* ORDERED: x ord y (both are numbers) */
    if (!__builtin_isunordered(fx, fy)) checksum ^= 4;
    
    /* UNEQ: unordered or equal */
    if (dx == dy) checksum ^= 8;  /* May generate UNEQ with fast-math */
    
    /* UNGE: unordered or greater-or-equal */
    if (dx >= dy) checksum ^= 16;
    
    /* UNGT: unordered or greater-than */
    if (dx > dy) checksum ^= 32;
    
    /* UNLE: unordered or less-or-equal */
    if (fx <= fy) checksum ^= 64;
    
    /* UNLT: unordered or less-than */
    if (fx < fy) checksum ^= 128;
    
    /* LTGT: less or greater (not equal, not unordered) */
    if (__builtin_islessgreater(dx, dy)) checksum ^= 256;
    
    /* 2. Conditional moves based on FP comparisons */
    double z1 = (dx >= dy) ? dx : dy;  /* May use UNGE */
    double z2 = (fx != fy) ? fx : fy;  /* May use UNEQ or LTGT */
    checksum += (int)(z1 + z2);
    
    /* 3. Loop with FP condition */
    for (int i = 0; i < 10 && (get_double_input(i) != 0.0); ++i) {
        checksum += i;  /* May use UNEQ in loop condition */
    }
    
    /* 4. Switch with FP comparisons (indirect trigger) */
    int selector = checksum & 7;
    double result = 0.0;
    switch (selector) {
        case 0: result = (dx < dy) ? dx : dy; break;  /* UNLT */
        case 1: result = (dx > dy) ? dx : dy; break;  /* UNGT */
        case 2: result = (dx <= dy) ? dx : dy; break; /* UNLE */
        case 3: result = (dx >= dy) ? dx : dy; break; /* UNGE */
        case 4: result = (dx == dy) ? dx : dy; break; /* UNEQ */
        case 5: result = (dx != dy) ? dx : dy; break; /* LTGT or UNEQ */
        default: result = dx + dy;
    }
    checksum += (int)result;
    
    /* 5. Vector (SIMD) comparisons */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {4.0f, 3.0f, 2.0f, 1.0f};
    
    /* Vector comparisons generate predicate masks */
    v4sf cmp_eq = (vec_a == vec_b);  /* May use UNEQ */
    v4sf cmp_lt = (vec_a < vec_b);   /* May use UNLT */
    v4sf cmp_gt = (vec_a > vec_b);   /* May use UNGT */
    v4sf cmp_le = (vec_a <= vec_b);  /* May use UNLE */
    v4sf cmp_ge = (vec_a >= vec_b);  /* May use UNGE */
    v4sf cmp_ne = (vec_a != vec_b);  /* May use LTGT */
    
    /* Extract results to prevent optimization */
    float* fcmp = (float*)&cmp_eq;
    for (int i = 0; i < 4; i++) {
        checksum += (fcmp[i] != 0.0f) ? 1 : 0;
    }
    
    /* 6. Explicit x86 intrinsics for unordered checks */
    __m128 sse_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    
    /* Direct unordered comparison intrinsic */
    __m128 unord_mask = _mm_cmpunord_ps(sse_a, sse_b);  /* UNORDERED */
    __m128 ord_mask = _mm_cmpord_ps(sse_a, sse_b);      /* ORDERED */
    
    /* Store to memory to force generation */
    volatile __m128 store_mask;
    store_mask = unord_mask;
    store_mask = ord_mask;
    
    /* 7. Double precision vector comparisons */
    v2df dvec_a = {dx, dy};
    v2df dvec_b = {dy, dx};
    v2df dvec_cmp = (dvec_a == dvec_b);  /* UNEQ */
    v2df dvec_cmp2 = (dvec_a != dvec_b); /* LTGT */
    
    double* dcmp = (double*)&dvec_cmp;
    checksum += (dcmp[0] != 0.0) ? 1 : 0;
    checksum += (dcmp[1] != 0.0) ? 2 : 0;
    
    /* 8. Mixed integer/float comparisons */
    int ix = (int)dx;
    int iy = (int)dy;
    if (fx < ix) checksum ^= 512;   /* Float < Int */
    if (dy > iy) checksum ^= 1024;  /* Double > Int */
    
    /* 9. Complex conditional expressions */
    checksum = (__builtin_isunordered(fx, fy) && (dx != dy)) 
               ? checksum * 2 
               : checksum / 2;
    
    /* 10. Prevent dead code elimination */
    use_result(checksum);
    
    printf("Result: %d\n", checksum);
    return checksum & 255;
}
