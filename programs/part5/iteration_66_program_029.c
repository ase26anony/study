/* Test program to exercise push_reload logic in reload.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper functions to create complex addressing modes */
int helper1(int *a, int *b, int *c) {
    return *a + *b + *c;
}

long helper2(long *a, long *b, int idx) {
    return a[idx] + b[idx];
}

double helper3(double *arr, int i, int j) {
    return arr[i] * arr[j];
}

void helper4(volatile int *dest, volatile int *src) {
    *dest = *src;
}

/* Complex inline assembly with mismatched modes */
#define COMPLEX_ASM1(num1, num2, num3, arr, idx1, idx2, result) \
    __asm__ volatile ( \
        "movl %[in1], %%eax\n\t" \
        "addl %[in2], %%eax\n\t" \
        "imull %[in3], %%eax\n\t" \
        "movl %%eax, %[out1]\n\t" \
        "leal (%[arr], %[idx1], 4), %%ebx\n\t" \
        "movl (%%ebx), %%ecx\n\t" \
        "addl %%ecx, %[out1]\n\t" \
        : [out1] "=r" (result), "=&r" (num1) \
        : [in1] "0" (num1), [in2] "r" (num2), \
          [in3] "rm" (num3), [arr] "r" (arr), \
          [idx1] "r" (idx1), [idx2] "m" (idx2) \
        : "eax", "ebx", "ecx", "memory", "cc" \
    )

/* Another complex asm with different constraints */
#define COMPLEX_ASM2(dbl1, dbl2, ptr, arr2d, i, j, k) \
    __asm__ volatile ( \
        "movsd %[d1], %%xmm0\n\t" \
        "addsd %[d2], %%xmm0\n\t" \
        "movsd %%xmm0, %[tmp]\n\t" \
        "movq %[ptr], %%r8\n\t" \
        "movl (%[arr], %[i], 4), %%r9d\n\t" \
        "addl %%r9d, (%%r8)\n\t" \
        : [tmp] "=m" (dbl1), "+&r" (ptr) \
        : [d1] "x" (dbl1), [d2] "xm" (dbl2), \
          [arr] "r" (arr2d), [i] "r" (i), \
          [j] "m" (j), [k] "r" (k) \
        : "xmm0", "r8", "r9", "memory", "cc" \
    )

/* Test function with high register pressure */
int test_reload(void) {
    /* Declare many register variables to create pressure */
    register int v1 asm("ebx") = 1;
    register int v2 asm("ecx") = 2;
    register int v3 asm("edx") = 3;
    register int v4 asm("esi") = 4;
    register int v5 asm("edi") = 5;
    register long l1 asm("r8") = 100;
    register long l2 asm("r9") = 200;
    register long l3 asm("r10") = 300;
    register double d1 asm("xmm0") = 1.5;
    register double d2 asm("xmm1") = 2.5;
    register double d3 asm("xmm2") = 3.5;
    register int *p1 asm("r11") = &v1;
    register int *p2 asm("r12") = &v2;
    register int *p3 asm("r13") = &v3;
    register int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    register int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    register int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    register int v21 = 21, v22 = 22, v23 = 23, v24 = 24, v25 = 25;
    
    /* Multi-dimensional array */
    int arr2d[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr2d[i][j] = i * 10 + j;
        }
    }
    
    /* Complex computations to create live ranges */
    v1 = v2 + v3 * v4 - v5 / 2;
    v6 = helper1(&v1, &v2, &v3) + v4;
    l1 = helper2(&l1, &l2, v6 % 8);
    d1 = helper3(&d1, v7, v8 % 4);
    
    /* First complex inline asm with many operands */
    int idx1 = v9 % 8;
    int idx2 = v10 % 8;
    int result1;
    
    COMPLEX_ASM1(v1, v2, v3, arr2d[0], idx1, idx2, result1);
    
    /* Use result in further computation */
    v11 = result1 + v4 + v5;
    
    /* Second inline asm with mismatched modes */
    int i = v12 % 8;
    int j = v13 % 8;
    int k = v14 % 8;
    
    COMPLEX_ASM2(d1, d2, p1, arr2d[i][j], i, j, k);
    
    /* Third asm with array element as operand */
    int arr_element;
    __asm__ volatile (
        "movl %[arr_elem], %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %[out]\n\t"
        "imull %[v15], %%eax\n\t"
        "addl %%eax, %[v16]\n\t"
        : [out] "=&r" (arr_element), "+r" (v16)
        : [arr_elem] "m" (arr2d[v17 % 8][v18 % 8]),
          [v15] "r" (v15), [v17] "r" (v17), [v18] "r" (v18)
        : "eax", "memory", "cc"
    );
    
    /* More computations using asm results */
    v19 = arr_element + v11;
    v20 = helper1(&v19, &arr_element, &v11);
    
    /* Another asm with function call in operand */
    int complex_result;
    __asm__ volatile (
        "push %[v21]\n\t"
        "push %[v22]\n\t"
        "push %[v23]\n\t"
        "call *%[helper]\n\t"
        "add $12, %%esp\n\t"
        "movl %%eax, %[result]\n\t"
        : [result] "=r" (complex_result)
        : [v21] "rm" (v21), [v22] "rm" (v22), 
          [v23] "rm" (v23), [helper] "r" (helper1)
        : "eax", "ecx", "edx", "memory", "cc"
    );
    
    /* Final computation using all variables */
    int final_sum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                   v21 + v22 + v23 + v24 + v25 + result1 + arr_element + complex_result +
                   (int)l1 + (int)l2 + (int)l3 + (int)d1 + (int)d2 + (int)d3;
    
    return final_sum;
}

/* Main function to run the test */
int main(void) {
    int result = test_reload();
    printf("Result: %d\n", result);
    
    /* Run multiple times to increase coverage chance */
    for (int i = 0; i < 10; i++) {
        result += test_reload();
    }
    
    printf("Final result: %d\n", result);
    return 0;
}
