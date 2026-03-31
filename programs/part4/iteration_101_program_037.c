/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Prevent optimization */
#define NOINLINE __attribute__((noinline, noclone))
#define KEEP(expr) do { asm volatile("" : : "r"(expr) : "memory"); } while(0)

/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef double v4df __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));
typedef long long v4di __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));

/* Complex shuffle with many operands */
NOINLINE v4si test_10_operands(v4si a, v4si b, v4si c, v4si d, 
                               v4si mask1, v4si mask2, v4si mask3) {
    volatile v4si temp1, temp2, temp3;
    
    /* Chain operations that may require many operands */
    v4si shuffle1 = __builtin_shuffle(a, b, mask1);
    KEEP(shuffle1);
    
    /* Vector conditional with arithmetic - may generate VEC_COND_EXPR */
    v4si cmp = a > b;
    v4si sel1 = cmp ? (a * b + c) : (d - a);
    KEEP(sel1);
    
    /* Another shuffle with different inputs */
    v4si shuffle2 = __builtin_shuffle(sel1, shuffle1, mask2);
    temp1 = shuffle2;  /* volatile store forces RTL generation */
    
    /* Complex expression with multiple operations */
    v4si expr1 = (a & b) | (c ^ d);
    v4si expr2 = (a << 2) + (b >> 1);
    v4si blend = cmp ? expr1 : expr2;
    
    /* Final shuffle combining everything */
    v4si shuffle3 = __builtin_shuffle(blend, temp1, mask3);
    temp2 = shuffle3;
    
    /* Mix in some arithmetic */
    v4si result = shuffle3 * a + b - c;
    temp3 = result;
    
    return temp3;
}

/* Test with floating-point vectors for different patterns */
NOINLINE v4df test_11_operands(v4df a, v4df b, v4df c, v4df d,
                               v4df e, v4si mask) {
    volatile v4df temp1, temp2, temp3;
    
    /* Complex FP expression - may expand to many operands */
    v4df cmp = a > b;
    KEEP(cmp);
    
    /* Conditional with multiple operations */
    v4df sel = cmp ? (a * b + c) : (d / e);
    temp1 = sel;
    
    /* Convert mask for shuffle */
    v2df low = __builtin_convertvector((v4si){0,1,2,3}, v2df);
    v2df high = __builtin_convertvector((v4si){4,5,6,7}, v2df);
    KEEP(low);
    KEEP(high);
    
    /* Chain of operations preventing optimization */
    v4df t1 = a + b;
    v4df t2 = c - d;
    v4df t3 = t1 * t2;
    v4df t4 = e / a;
    v4df t5 = t3 + t4;
    
    /* Another conditional */
    v4df cmp2 = t5 > sel;
    v4df final = cmp2 ? t5 : sel;
    temp2 = final;
    
    /* Mix with integer mask */
    v4df masked = final * __builtin_convertvector(mask, v4df);
    temp3 = masked;
    
    return temp3;
}

/* AVX-512 style 512-bit vectors if supported */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));

NOINLINE v16si test_many_operands_avx512(v16si a, v16si b, v16si c, 
                                         v16si d, v16si mask) {
    /* Very complex expression that might need many temporaries */
    v16si t1 = a + b;
    v16si t2 = c * d;
    v16si t3 = t1 & t2;
    v16si t4 = t1 | t2;
    v16si cmp = a > b;
    v16si result = cmp ? t3 : t4;
    
    /* Shuffle with large mask */
    v16si shuffled = __builtin_shuffle(result, mask);
    
    /* Multiple volatile stores */
    volatile v16si temp = shuffled;
    return temp;
}
#endif

/* Main test function */
NOINLINE int run_tests(void) {
    /* Initialize test vectors */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    v4si d = {13, 14, 15, 16};
    v4si mask1 = {3, 2, 1, 0};
    v4si mask2 = {1, 0, 3, 2};
    v4si mask3 = {2, 3, 0, 1};
    
    v4df fa = {1.0, 2.0, 3.0, 4.0};
    v4df fb = {5.0, 6.0, 7.0, 8.0};
    v4df fc = {9.0, 10.0, 11.0, 12.0};
    v4df fd = {13.0, 14.0, 15.0, 16.0};
    v4df fe = {17.0, 18.0, 19.0, 20.0};
    
    /* Run tests */
    v4si res1 = test_10_operands(a, b, c, d, mask1, mask2, mask3);
    KEEP(res1);
    
    v4df res2 = test_11_operands(fa, fb, fc, fd, fe, mask1);
    KEEP(res2);
    
    /* Extract checksum */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += res1[i];
        sum += (int)res2[i];
    }
    
    return sum;
}

int main(void) {
    int result = run_tests();
    
    /* Use result to prevent dead code elimination */
    printf("Result checksum: %d\n", result);
    
    /* Return non-zero if result is 0 (unlikely) */
    return result == 0 ? 1 : 0;
}
