/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent optimization */
#define NOINLINE __attribute__((noinline, noclone))
#define BARRIER() asm volatile("" ::: "memory")

/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Larger vector types for AVX */
#ifdef __AVX__
typedef int v8si __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
#endif

#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));
#endif

/* Complex shuffle with many operands */
NOINLINE v4si test_10_operands(v4si a, v4si b, v4si c, v4si d, v4si mask) {
    volatile v4si temp1, temp2, temp3;
    
    /* Force multiple operations that may require many operands */
    /* 1. Vector arithmetic with multiple temporaries */
    v4si t1 = a + b;
    v4si t2 = c * d;
    BARRIER();
    
    /* 2. Shuffle with dynamic mask - may expand to many operands */
    /* __builtin_shuffle typically needs: 2 input vectors, mask, output, temps */
    v4si shuffled = __builtin_shuffle(t1, t2, mask);
    temp1 = shuffled;  /* volatile store to prevent elimination */
    BARRIER();
    
    /* 3. Conditional select with comparison - VEC_COND_EXPR expansion */
    v4si cmp = (a > b);
    v4si t3 = (cmp) ? t1 : t2;
    temp2 = t3;
    BARRIER();
    
    /* 4. Another shuffle combining multiple vectors */
    v4si final_shuffle = __builtin_shufflevector(t1, t3, 3, 2, 1, 0);
    temp3 = final_shuffle;
    BARRIER();
    
    /* 5. Blend operation simulated with conditional */
    v4si blend_mask = (mask != (v4si){0, 0, 0, 0});
    v4si result = (blend_mask) ? shuffled : final_shuffle;
    
    return result + t3;
}

#ifdef __AVX__
/* Test with AVX vectors - more elements, more potential operands */
NOINLINE v8si test_avx_10_operands(v8si a, v8si b, v8si c, v8si d, v8si mask) {
    volatile v8si temp1, temp2;
    
    /* Complex expression that may require many temporaries */
    v8si t1 = a + b;
    v8si t2 = c - d;
    v8si t3 = a * c;
    v8si t4 = b * d;
    BARRIER();
    
    /* Shuffle with larger vectors - more operands needed */
    v8si shuffled = __builtin_shufflevector(t1, t2, 7, 6, 5, 4, 3, 2, 1, 0);
    temp1 = shuffled;
    BARRIER();
    
    /* Vector comparison and conditional */
    v8si cmp = (a > (v8si){1, 2, 3, 4, 5, 6, 7, 8});
    v8si blended = (cmp) ? shuffled : t3;
    temp2 = blended;
    BARRIER();
    
    /* Another operation chain */
    v8si t5 = t1 * t2 + t3 - t4;
    v8si result = (mask > 0) ? blended : t5;
    
    return result;
}
#endif

#ifdef __AVX512F__
/* AVX-512 with masking - likely to hit many-operand patterns */
NOINLINE v16si test_avx512_11_operands(v16si a, v16si b, v16si c, v16si d, 
                                       v16si mask1, v16si mask2) {
    volatile v16si temp1, temp2, temp3;
    
    /* Multiple arithmetic operations */
    v16si t1 = a + b;
    v16si t2 = c * d;
    v16si t3 = a - c;
    v16si t4 = b - d;
    BARRIER();
    
    /* Complex conditional with two masks */
    v16si cmp1 = (mask1 > 0);
    v16si cmp2 = (mask2 > 0);
    
    /* Nested conditional - may expand to many operands */
    v16si sel1 = (cmp1) ? t1 : t2;
    v16si sel2 = (cmp2) ? t3 : t4;
    temp1 = sel1;
    temp2 = sel2;
    BARRIER();
    
    /* Final blend of two selections */
    v16si final_cmp = (sel1 > sel2);
    v16si result = (final_cmp) ? sel1 : sel2;
    temp3 = result;
    
    return result;
}
#endif

/* Main test function that combines everything */
NOINLINE v4si test_function(v4si a, v4si b, v4si c, v4si d, v4si mask) {
    v4si result1 = test_10_operands(a, b, c, d, mask);
    
    /* Additional operations to ensure coverage */
    v4si t = result1 * a;
    v4si u = b + c;
    
    /* Another shuffle with different pattern */
    v4si shuffled2 = __builtin_shufflevector(t, u, 1, 0, 3, 2);
    
    /* Vector conditional with arithmetic */
    v4si cmp = (shuffled2 > result1);
    v4si final = (cmp) ? shuffled2 * 2 : result1 + 1;
    
    return final;
}

int main() {
    /* Initialize test vectors with pattern */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    v4si d = {13, 14, 15, 16};
    v4si mask = {3, 2, 1, 0};  /* Reverse shuffle mask */
    
    /* Call test function multiple times with different inputs */
    v4si result = {0};
    for (int i = 0; i < 100; i++) {
        /* Modify inputs slightly each iteration */
        a[0] += i & 1;
        b[1] += i & 2;
        mask[2] = (i % 4);
        
        v4si r = test_function(a, b, c, d, mask);
        result = result + r;
        
        BARRIER();
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 4; i++) {
        checksum += result[i];
    }
    
    /* Print result to ensure execution */
    printf("Checksum: %d\n", checksum);
    
    /* Return based on checksum to affect exit code */
    return (checksum > 0) ? 0 : 1;
}
