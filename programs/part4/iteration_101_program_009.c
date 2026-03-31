/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <string.h>
#include <x86intrin.h>

/* Define vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

typedef int v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));

/* AVX-512 types for more operands */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
#endif

/* Force no inlining to prevent optimization */
__attribute__((noinline, noipa))
v4si test_10_operands(v4si a, v4si b, v4si c, v4si d, v4si mask) {
    volatile v4si temp1, temp2, temp3;
    
    /* Complex shuffle with many operands */
    v4si shuffled = __builtin_shuffle(a, b, mask);
    
    /* Store to volatile to force operations */
    temp1 = shuffled;
    
    /* Vector conditional with arithmetic - may expand to many operands */
    v4si cmp_result = (a > b) ? (c * d + shuffled) : (c - d * shuffled);
    
    temp2 = cmp_result;
    
    /* Another shuffle with different sources */
    v4si shuffle_mask = {3, 1, 2, 0};
    v4si final_shuffle = __builtin_shuffle(cmp_result, a + b, shuffle_mask);
    
    temp3 = final_shuffle;
    
    /* Compiler barrier */
    asm volatile("" : : "r"(temp1), "r"(temp2), "r"(temp3) : "memory");
    
    return final_shuffle;
}

__attribute__((noinline, noipa))
v4df test_11_operands(v4df a, v4df b, v4df c, v4df d, v4df e, v4si intmask) {
    volatile v4df temp1, temp2, temp3, temp4;
    
    /* Complex floating point operations that may require many operands */
    v4df blend_temp = __builtin_ia32_blendvpd256(a, b, c);
    temp1 = blend_temp;
    
    /* Conditional with multiple operations */
    v4df cmp = (a > b);
    v4df cond_result = cmp ? (c * d + e) : (c / d - e);
    temp2 = cond_result;
    
    /* Convert mask for shuffle */
    v2di mask64 = __builtin_convertvector(intmask, v2di);
    v4df shuffled = __builtin_shuffle(cond_result, blend_temp, mask64);
    temp3 = shuffled;
    
    /* Another operation chain */
    v4df final = shuffled * a + b / c - d;
    temp4 = final;
    
    /* Compiler barrier with many operands */
    asm volatile("" : : "r"(temp1), "r"(temp2), "r"(temp3), "r"(temp4) : "memory");
    
    return final;
}

/* Test with AVX2/AVX-512 for more operand possibilities */
#ifdef __AVX2__
__attribute__((noinline, noipa))
v8si test_many_operands_avx2(v8si a, v8si b, v8si c, v8si d, v8si e, v8si mask) {
    volatile v8si temp[4];
    
    /* Multiple operations that could expand to many operands */
    v8si op1 = a + b;
    temp[0] = op1;
    
    v8si op2 = c * d;
    temp[1] = op2;
    
    v8si op3 = __builtin_shuffle(op1, op2, mask);
    temp[2] = op3;
    
    /* Complex conditional */
    v8si cmp = (a > b);
    v8si result = cmp ? (op3 + e) : (op3 - e);
    temp[3] = result;
    
    /* Barrier */
    asm volatile("" : : "r"(temp[0]), "r"(temp[1]), "r"(temp[2]), "r"(temp[3]) : "memory");
    
    return result;
}
#endif

#ifdef __AVX512F__
__attribute__((noinline, noipa))
v16si test_avx512_many_ops(v16si a, v16si b, v16si c, v16si d, 
                           v16si e, v16si f, v16si mask) {
    volatile v16si temp[6];
    
    /* Chain of operations that may require many operands */
    v16si t1 = a + b;
    temp[0] = t1;
    
    v16si t2 = c * d;
    temp[1] = t2;
    
    v16si t3 = __builtin_shuffle(t1, t2, mask);
    temp[2] = t3;
    
    v16si t4 = (t3 > e) ? f : t3;
    temp[3] = t4;
    
    v16si t5 = t4 + a - b;
    temp[4] = t5;
    
    v16si result = __builtin_shuffle(t5, t3, mask);
    temp[5] = result;
    
    /* Large barrier with many operands */
    asm volatile("" : : "r"(temp[0]), "r"(temp[1]), "r"(temp[2]), 
                  "r"(temp[3]), "r"(temp[4]), "r"(temp[5]) : "memory");
    
    return result;
}
#endif

int main() {
    /* Initialize test vectors */
    v4si a4 = {1, 2, 3, 4};
    v4si b4 = {5, 6, 7, 8};
    v4si c4 = {9, 10, 11, 12};
    v4si d4 = {13, 14, 15, 16};
    v4si mask4 = {3, 2, 1, 0};
    
    v4df ad = {1.0, 2.0, 3.0, 4.0};
    v4df bd = {5.0, 6.0, 7.0, 8.0};
    v4df cd = {9.0, 10.0, 11.0, 12.0};
    v4df dd = {13.0, 14.0, 15.0, 16.0};
    v4df ed = {17.0, 18.0, 19.0, 20.0};
    
    printf("Testing 10/11 operand expansion paths...\n");
    
    /* Test 10 operand path */
    v4si result1 = test_10_operands(a4, b4, c4, d4, mask4);
    
    /* Test 11 operand path */
    v4df result2 = test_11_operands(ad, bd, cd, dd, ed, mask4);
    
    /* Compute checksums to prevent dead code elimination */
    int checksum1 = 0;
    double checksum2 = 0.0;
    
    for (int i = 0; i < 4; i++) {
        checksum1 += result1[i];
        checksum2 += result2[i];
    }
    
    printf("Checksum1: %d\n", checksum1);
    printf("Checksum2: %f\n", checksum2);
    
#ifdef __AVX2__
    v8si a8 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si b8 = {9, 10, 11, 12, 13, 14, 15, 16};
    v8si c8 = {17, 18, 19, 20, 21, 22, 23, 24};
    v8si d8 = {25, 26, 27, 28, 29, 30, 31, 32};
    v8si e8 = {33, 34, 35, 36, 37, 38, 39, 40};
    v8si mask8 = {7, 6, 5, 4, 3, 2, 1, 0};
    
    v8si result3 = test_many_operands_avx2(a8, b8, c8, d8, e8, mask8);
    
    int checksum3 = 0;
    for (int i = 0; i < 8; i++) {
        checksum3 += result3[i];
    }
    printf("Checksum3 (AVX2): %d\n", checksum3);
#endif
    
#ifdef __AVX512F__
    v16si a16 = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    v16si b16 = {17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32};
    v16si c16 = {33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48};
    v16si d16 = {49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64};
    v16si e16 = {65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80};
    v16si f16 = {81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96};
    v16si mask16 = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
    
    v16si result4 = test_avx512_many_ops(a16, b16, c16, d16, e16, f16, mask16);
    
    int checksum4 = 0;
    for (int i = 0; i < 16; i++) {
        checksum4 += result4[i];
    }
    printf("Checksum4 (AVX512): %d\n", checksum4);
#endif
    
    /* Return based on checksums to ensure execution */
    return (checksum1 != 0 && checksum2 != 0.0) ? 0 : 1;
}
