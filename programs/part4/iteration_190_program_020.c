/* Test program for GCC optabs.cc 10/11 operand expansion coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static unsigned int seed = 12345;
static unsigned int prng(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Function to handle 10 operands */
int func_10_operands(int a, int b, int c, int d, int e,
                     int f, int g, int h, int i, int j) {
    int result = 0;
    
    /* Architecture-specific paths for high operand count operations */
    
#ifdef __AVX512F__
    /* AVX-512 mask compress/store operations can involve many operands */
    /* Using inline assembly as a fallback since specific intrinsics vary */
    __asm__ volatile (
        "/* 10 operand dummy operation */\n\t"
        "addl %1, %0\n\t"
        "addl %2, %0\n\t"
        "addl %3, %0\n\t"
        "addl %4, %0\n\t"
        "addl %5, %0\n\t"
        "addl %6, %0\n\t"
        "addl %7, %0\n\t"
        "addl %8, %0\n\t"
        "addl %9, %0\n\t"
        "addl %10, %0"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE scatter/gather operations can have many operands */
    __asm__ volatile (
        "/* SVE 10 operand operation */\n\t"
        "add %0, %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
#elif defined(__POWERPC__) || defined(__PPC__)
    /* PowerPC vector operations */
    __asm__ volatile (
        "/* PowerPC 10 operand operation */\n\t"
        "add %0, %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
#else
    /* Generic fallback with extended inline assembly */
    __asm__ volatile (
        "/* Generic 10 operand operation */\n\t"
        "addl %1, %0\n\t"
        "addl %2, %0\n\t"
        "addl %3, %0\n\t"
        "addl %4, %0\n\t"
        "addl %5, %0\n\t"
        "addl %6, %0\n\t"
        "addl %7, %0\n\t"
        "addl %8, %0\n\t"
        "addl %9, %0\n\t"
        "addl %10, %0"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
#endif
    
    return result;
}

/* Function to handle 11 operands */
int func_11_operands(int a, int b, int c, int d, int e, int f,
                     int g, int h, int i, int j, int k) {
    int result = 0;
    
    /* Similar pattern but with 11 operands */
    
#ifdef __AVX512F__
    __asm__ volatile (
        "/* 11 operand dummy operation */\n\t"
        "addl %1, %0\n\t"
        "addl %2, %0\n\t"
        "addl %3, %0\n\t"
        "addl %4, %0\n\t"
        "addl %5, %0\n\t"
        "addl %6, %0\n\t"
        "addl %7, %0\n\t"
        "addl %8, %0\n\t"
        "addl %9, %0\n\t"
        "addl %10, %0\n\t"
        "addl %11, %0"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j),
          "r" (k)
        : "cc"
    );
#elif defined(__ARM_FEATURE_SVE)
    __asm__ volatile (
        "/* SVE 11 operand operation */\n\t"
        "add %0, %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10\n\t"
        "add %0, %0, %11"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j),
          "r" (k)
        : "cc"
    );
#else
    /* Generic 11 operand inline assembly */
    __asm__ volatile (
        "/* Generic 11 operand operation */\n\t"
        "addl %1, %0\n\t"
        "addl %2, %0\n\t"
        "addl %3, %0\n\t"
        "addl %4, %0\n\t"
        "addl %5, %0\n\t"
        "addl %6, %0\n\t"
        "addl %7, %0\n\t"
        "addl %8, %0\n\t"
        "addl %9, %0\n\t"
        "addl %10, %0\n\t"
        "addl %11, %0"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j),
          "r" (k)
        : "cc"
    );
#endif
    
    return result;
}

int main(int argc, char *argv[]) {
    int vars[12];
    int i, result1, result2, final_result;
    
    /* Initialize variables with non-constant values */
    for (i = 0; i < 12; i++) {
        /* Use argv if available, otherwise PRNG */
        if (argc > i + 1) {
            vars[i] = atoi(argv[i + 1]);
        } else {
            vars[i] = prng() % 100;
        }
    }
    
    /* Call 10-operand function */
    result1 = func_10_operands(vars[0], vars[1], vars[2], vars[3], vars[4],
                               vars[5], vars[6], vars[7], vars[8], vars[9]);
    
    /* Call 11-operand function */
    result2 = func_11_operands(vars[0], vars[1], vars[2], vars[3], vars[4],
                               vars[5], vars[6], vars[7], vars[8], vars[9],
                               vars[10]);
    
    /* Combine results to prevent optimization */
    final_result = result1 + result2;
    
    /* Use the result */
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
