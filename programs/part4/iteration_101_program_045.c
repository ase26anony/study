/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Define vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef double v4df __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));
typedef long long v4di __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));

/* Force no inlining to preserve expansion patterns */
__attribute__((noinline, noipa))
v4si test_10_operand_expansion(v4si a, v4si b, v4si c, v4si d, 
                               v4si mask, v4si e, v4si f) {
    /* Complex shuffle operation that may require many operands */
    v4si shuffled1 = __builtin_shuffle(a, b, mask);
    
    /* Vector conditional with arithmetic - may generate VEC_COND_EXPR */
    v4si cmp_result = (shuffled1 > c) ? a * b + d : c - d;
    
    /* Another shuffle with different inputs */
    v4si shuffle_mask = {3, 2, 1, 0};
    v4si shuffled2 = __builtin_shuffle(cmp_result, e, shuffle_mask);
    
    /* Complex arithmetic chain */
    v4si temp1 = shuffled1 * shuffled2;
    v4si temp2 = temp1 + f;
    v4si temp3 = temp2 - a;
    
    /* Volatile store to prevent optimization */
    volatile v4si volatile_store;
    volatile_store = temp3;
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Final blend-like operation using conditional */
    v4si final_result = (mask > (v4si){0, 0, 0, 0}) ? temp3 : shuffled2;
    
    return final_result;
}

__attribute__((noinline, noipa))
v4df test_11_operand_expansion(v4df a, v4df b, v4df c, v4df d,
                               v4df e, v4df mask, v4df f) {
    /* Complex floating-point vector operations */
    v4df mul_result = a * b;
    v4df add_result = mul_result + c;
    
    /* Vector comparison and conditional */
    v4df cmp = (mask > (v4df){0.0, 0.0, 0.0, 0.0});
    v4df cond_result = cmp ? add_result * d : e / f;
    
    /* Shuffle with floating-point vectors */
    long long shuffle_idx[4] = {1, 0, 3, 2};
    v4df shuffled;
    memcpy(&shuffled, &cond_result, sizeof(v4df));
    
    /* Complex arithmetic chain */
    v4df temp1 = shuffled + a;
    v4df temp2 = temp1 * b;
    v4df temp3 = temp2 - c;
    v4df temp4 = temp3 / d;
    
    /* Volatile operations to prevent optimization */
    volatile v4df volatile_fp;
    volatile_fp = temp4;
    
    /* Compiler barrier */
    asm volatile("" ::: "memory");
    
    /* Final conditional blend */
    v4df final = (cmp) ? temp4 : cond_result;
    
    return final;
}

/* Test with AVX-512 style 512-bit vectors if available */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));

__attribute__((noinline, noipa))
v16si test_many_operand_avx512(v16si a, v16si b, v16si c, v16si d,
                               v16si e, v16si f, v16si g, v16si mask) {
    /* Very complex operation chain that might require many operands */
    v16si t1 = a + b;
    v16si t2 = c * d;
    v16si t3 = t1 - t2;
    v16si t4 = e & f;
    v16si t5 = t3 | t4;
    v16si t6 = g ^ t5;
    
    /* Conditional with large vector */
    v16si result = (mask > (v16si){0}) ? t6 : t3;
    
    /* Multiple volatile stores */
    volatile v16si v1, v2;
    v1 = t1;
    v2 = t2;
    asm volatile("" ::: "memory");
    
    return result;
}
#endif

/* Main test driver */
int main() {
    /* Initialize test vectors */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    v4si d = {13, 14, 15, 16};
    v4si mask = {1, 3, 0, 2};
    v4si e = {17, 18, 19, 20};
    v4si f = {21, 22, 23, 24};
    
    /* Test 10-operand expansion path */
    v4si result1 = test_10_operand_expansion(a, b, c, d, mask, e, f);
    
    /* Initialize floating-point vectors */
    v4df fa = {1.0, 2.0, 3.0, 4.0};
    v4df fb = {5.0, 6.0, 7.0, 8.0};
    v4df fc = {9.0, 10.0, 11.0, 12.0};
    v4df fd = {13.0, 14.0, 15.0, 16.0};
    v4df fe = {17.0, 18.0, 19.0, 20.0};
    v4df fmask = {1.0, 0.0, -1.0, 0.0};
    v4df ff = {21.0, 22.0, 23.0, 24.0};
    
    /* Test 11-operand expansion path */
    v4df result2 = test_11_operand_expansion(fa, fb, fc, fd, fe, fmask, ff);
    
    /* Compute checksums to prevent dead code elimination */
    int checksum1 = 0;
    double checksum2 = 0.0;
    
    for (int i = 0; i < 4; i++) {
        checksum1 += result1[i];
        checksum2 += result2[i];
    }
    
    printf("Integer checksum: %d\n", checksum1);
    printf("Float checksum: %f\n", checksum2);
    
    /* Test AVX-512 paths if available */
#ifdef __AVX512F__
    v16si avx512_a, avx512_b, avx512_c, avx512_d;
    v16si avx512_e, avx512_f, avx512_g, avx512_mask;
    
    /* Initialize AVX-512 vectors */
    for (int i = 0; i < 16; i++) {
        avx512_a[i] = i;
        avx512_b[i] = i + 16;
        avx512_c[i] = i + 32;
        avx512_d[i] = i + 48;
        avx512_e[i] = i + 64;
        avx512_f[i] = i + 80;
        avx512_g[i] = i + 96;
        avx512_mask[i] = (i % 2) ? -1 : 0;
    }
    
    v16si avx512_result = test_many_operand_avx512(avx512_a, avx512_b, avx512_c,
                                                   avx512_d, avx512_e, avx512_f,
                                                   avx512_g, avx512_mask);
    
    int avx512_checksum = 0;
    for (int i = 0; i < 16; i++) {
        avx512_checksum += avx512_result[i];
    }
    printf("AVX-512 checksum: %d\n", avx512_checksum);
#endif
    
    /* Return based on checksums to ensure execution */
    return (checksum1 != 0 && checksum2 != 0.0) ? 0 : 1;
}
