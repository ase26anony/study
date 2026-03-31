/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Define large vector types */
typedef int v8si __attribute__((vector_size(32)));
typedef long long v4di __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));

/* SSE/AVX types for intrinsics */
#ifdef __SSE2__
#include <emmintrin.h>
#include <immintrin.h>
#endif

/* Prevent optimization */
#define KEEP(expr) do { \
    volatile __typeof__(expr) _tmp = (expr); \
    asm volatile("" : : "r"(&_tmp) : "memory"); \
} while(0)

/* Compiler barrier */
#define BARRIER() asm volatile("" : : : "memory")

/* Noinline test function */
__attribute__((noinline,noipa))
v8si test_10_11_operands(v8si a, v8si b, v8si c, v8si d, 
                         v4df fa, v4df fb, v4df fc, v4df fd) {
    volatile v8si v1, v2, v3, v4;
    volatile v4df fv1, fv2, fv3, fv4;
    
    /* Complex shuffle operation - may require many operands */
    v8si shuffle_mask = {7, 6, 5, 4, 3, 2, 1, 0};
    v8si shuffled = __builtin_shuffle(a, b, shuffle_mask);
    v1 = shuffled;
    BARRIER();
    
    /* Vector conditional with comparison - generates VEC_COND_EXPR */
    v8si cmp_result = (a > b) ? (c * d) : (c + d);
    v2 = cmp_result;
    BARRIER();
    
    /* Chain of operations that might expand to many operands */
    v8si temp1 = a * b + c;
    v8si temp2 = d - a * 2;
    v8si blend_result = __builtin_shuffle(temp1, temp2, shuffle_mask);
    v3 = blend_result;
    BARRIER();
    
    /* Floating point vector conditional with mixed operations */
    v4df fcmp = (fa > fb) ? (fc * fd) : (fc / fd);
    fv1 = fcmp;
    BARRIER();
    
    /* Another complex shuffle with arithmetic */
    v4df fshuffle_mask = {3.0, 2.0, 1.0, 0.0};
    v4df fshuffled = __builtin_shufflevector(fa, fb, 3, 2, 1, 0);
    v4df fresult = fshuffled * fc + fd;
    fv2 = fresult;
    BARRIER();
    
    /* Combine all results */
    v8si final_int = shuffled + cmp_result + blend_result;
    v4 = final_int;
    
    /* Use floating point results in integer context */
    v4di double_as_int = *(v4di*)&fresult;
    v8si final = final_int + (v8si){double_as_int[0], double_as_int[1], 
                                   double_as_int[2], double_as_int[3],
                                   double_as_int[0], double_as_int[1],
                                   double_as_int[2], double_as_int[3]};
    
    return final;
}

/* Another test focusing on 11 operands */
__attribute__((noinline,noipa))
v4df test_11_operand_pattern(v4df a, v4df b, v4df c, v4df d,
                             v4df e, v4df f, v4df g, v4df h) {
    volatile v4df v1, v2, v3;
    
    /* Complex conditional blend - may need many operands */
    v4df mask = a > b;
    v4df blend = __builtin_shufflevector(
        __builtin_shufflevector(a, b, 3, 2, 1, 0),
        __builtin_shufflevector(c, d, 3, 2, 1, 0),
        3, 2, 1, 0
    );
    
    v1 = blend;
    BARRIER();
    
    /* Chain of operations */
    v4df temp1 = a * b + c;
    v4df temp2 = d * e - f;
    v4df temp3 = g / h + a;
    
    /* Complex expression that might need many temporaries */
    v4df result = (mask > 0.5) ? (temp1 * temp2) : (temp3 / blend);
    v2 = result;
    BARRIER();
    
    /* Another shuffle with conversion */
    v4df shuffled = __builtin_shufflevector(result, blend, 3, 2, 1, 0);
    v4df final = shuffled * 2.0 - 1.0;
    v3 = final;
    
    return final;
}

int main() {
    /* Initialize test vectors */
    v8si a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b = {8, 7, 6, 5, 4, 3, 2, 1};
    v8si c = {2, 4, 6, 8, 10, 12, 14, 16};
    v8si d = {1, 3, 5, 7, 9, 11, 13, 15};
    
    v4df fa = {1.0, 2.0, 3.0, 4.0};
    v4df fb = {4.0, 3.0, 2.0, 1.0};
    v4df fc = {0.5, 1.5, 2.5, 3.5};
    v4df fd = {2.0, 4.0, 6.0, 8.0};
    
    /* More vectors for 11 operand test */
    v4df fe = {1.1, 2.2, 3.3, 4.4};
    v4df ff = {4.4, 3.3, 2.2, 1.1};
    v4df fg = {0.1, 0.2, 0.3, 0.4};
    v4df fh = {1.0, 2.0, 3.0, 4.0};
    
    /* Call test functions */
    v8si result1 = test_10_11_operands(a, b, c, d, fa, fb, fc, fd);
    v4df result2 = test_11_operand_pattern(fa, fb, fc, fd, fe, ff, fg, fh);
    
    /* Compute checksums to prevent dead code elimination */
    int checksum1 = 0;
    for (int i = 0; i < 8; i++) {
        checksum1 += result1[i];
    }
    
    double checksum2 = 0.0;
    for (int i = 0; i < 4; i++) {
        checksum2 += result2[i];
    }
    
    /* Use results to affect program output */
    printf("Checksum1: %d\n", checksum1);
    printf("Checksum2: %f\n", checksum2);
    
    /* Return based on checksums to ensure execution */
    return (checksum1 != 0 && checksum2 != 0.0) ? 0 : 1;
}
