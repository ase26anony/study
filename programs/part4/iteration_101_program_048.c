/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Prevent optimization */
#define NOINLINE __attribute__((noinline))
#define NOCLONE __attribute__((noclone))

/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef double v4df __attribute__((vector_size(32)));
typedef long long v2di __attribute__((vector_size(16)));
typedef int v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));

/* Compiler barrier */
static inline void barrier(void) {
    asm volatile("" ::: "memory");
}

/* Test function with many vector operations */
NOINLINE NOCLONE static v8si test_many_operands(v4si a, v4si b, v4si c, v4si d,
                                                v2df f1, v2df f2, v4df f3, v4df f4,
                                                v2di m1, v2di m2) {
    volatile v4si vtmp1, vtmp2, vtmp3;
    volatile v2df ftmp1, ftmp2;
    volatile v4df ftmp3, ftmp4;
    volatile v8si result1, result2;
    
    /* Force memory operations */
    vtmp1 = a;
    vtmp2 = b;
    barrier();
    
    /* Shuffle operation - may require many operands */
    v4si shuffle1 = __builtin_shuffle(vtmp1, vtmp2, (v4si){3, 2, 1, 0});
    barrier();
    
    /* Another shuffle with different pattern */
    v4si shuffle2 = __builtin_shuffle(c, d, (v4si){1, 0, 3, 2});
    barrier();
    
    /* Vector conditional expression */
    v4si cmp_mask = shuffle1 > shuffle2;
    v4si select1 = cmp_mask ? shuffle1 : shuffle2;
    barrier();
    
    /* Convert to different vector size */
    v8si wide1 = __builtin_convertvector(select1, v8si);
    v8si wide2 = __builtin_convertvector(shuffle1, v8si);
    barrier();
    
    /* Complex vector arithmetic with many operands */
    v8si arith1 = wide1 + wide2;
    v8si arith2 = wide1 - wide2;
    v8si arith3 = wide1 * wide2;
    barrier();
    
    /* Another conditional with vector comparison */
    v8si cmp_wide = arith1 > arith2;
    v8si select_wide = cmp_wide ? arith1 : arith2;
    barrier();
    
    /* Blend the results */
    v8si final_result = select_wide + arith3;
    
    /* Store to volatile to force operation */
    result1 = final_result;
    
    /* Floating point vector operations */
    ftmp1 = f1;
    ftmp2 = f2;
    barrier();
    
    /* Vector conditional with floats */
    v2df fcmp = ftmp1 > ftmp2;
    v2df fselect = fcmp ? ftmp1 * 2.0 : ftmp2 / 2.0;
    barrier();
    
    /* Larger vector operations */
    ftmp3 = f3;
    ftmp4 = f4;
    barrier();
    
    /* Complex expression with many operands */
    v4df fcomplex = ftmp3 + ftmp4 * 1.5 - ftmp3 / 2.0;
    barrier();
    
    /* Convert between vector types */
    v8si from_float = __builtin_convertvector(fcomplex, v8si);
    barrier();
    
    /* Final combination */
    result2 = final_result + from_float;
    
    return result2;
}

/* Another test focusing on exactly 10/11 operand patterns */
NOINLINE NOCLONE static v4si test_exact_operands(v4si a, v4si b, v4si c, v4si d,
                                                 v4si e, v4si f, v4si g, v4si h,
                                                 v4si i, v4si j) {
    /* Chain of operations that might require many operands */
    v4si t1 = a + b;
    v4si t2 = c - d;
    v4si t3 = e * f;
    v4si t4 = g / (h + (v4si){1,1,1,1});
    barrier();
    
    /* Shuffle with all inputs */
    v4si shuffle1 = __builtin_shuffle(t1, t2, (v4si){0,1,2,3});
    v4si shuffle2 = __builtin_shuffle(t3, t4, (v4si){3,2,1,0});
    barrier();
    
    /* Multiple comparisons */
    v4si cmp1 = shuffle1 > shuffle2;
    v4si cmp2 = t1 < t3;
    v4si cmp3 = a == b;
    barrier();
    
    /* Nested conditional */
    v4si sel1 = cmp1 ? shuffle1 : shuffle2;
    v4si sel2 = cmp2 ? t1 : t3;
    v4si sel3 = cmp3 ? a : b;
    barrier();
    
    /* Final combination using all inputs */
    v4si result = sel1 + sel2 * sel3 - i + j;
    
    /* Volatile store */
    volatile v4si vresult = result;
    barrier();
    
    return vresult;
}

int main(void) {
    /* Initialize vectors with pattern */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    v4si d = {13, 14, 15, 16};
    v4si e = {17, 18, 19, 20};
    v4si f = {21, 22, 23, 24};
    v4si g = {25, 26, 27, 28};
    v4si h = {29, 30, 31, 32};
    v4si i = {33, 34, 35, 36};
    v4si j = {37, 38, 39, 40};
    
    v2df f1 = {1.0, 2.0};
    v2df f2 = {3.0, 4.0};
    v4df f3 = {5.0, 6.0, 7.0, 8.0};
    v4df f4 = {9.0, 10.0, 11.0, 12.0};
    
    v2di m1 = {0x1, 0x2};
    v2di m2 = {0x3, 0x4};
    
    /* Call test functions */
    v8si result1 = test_many_operands(a, b, c, d, f1, f2, f3, f4, m1, m2);
    v4si result2 = test_exact_operands(a, b, c, d, e, f, g, h, i, j);
    
    /* Compute checksums to prevent dead code elimination */
    int checksum1 = 0;
    int checksum2 = 0;
    
    /* Extract elements from result1 */
    int *r1 = (int*)&result1;
    for (int k = 0; k < 8; k++) {
        checksum1 += r1[k];
    }
    
    /* Extract elements from result2 */
    int *r2 = (int*)&result2;
    for (int k = 0; k < 4; k++) {
        checksum2 += r2[k];
    }
    
    /* Use results */
    printf("Checksum1: %d\n", checksum1);
    printf("Checksum2: %d\n", checksum2);
    
    /* Return based on checksums to ensure execution */
    return (checksum1 + checksum2) != 0 ? 0 : 1;
}
