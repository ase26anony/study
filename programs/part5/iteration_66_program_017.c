/* Test program to trigger push_reload logic in reload.cc */
#include <stdint.h>
#include <stdlib.h>

/* Helper functions that take addresses */
static int helper1(int *a, int *b) {
    return (*a + *b) * 2;
}

static long helper2(long *a, long *b, long *c) {
    return *a * *b + *c;
}

static double helper3(double *a, double *b) {
    return *a / *b + 1.0;
}

/* Complex inline assembly with mismatched modes */
static inline void asm_block1(int *arr, long idx1, long idx2, 
                              int *out1, long *out2, double *out3) {
    asm volatile (
        "movl %[arr_val], %%eax\n\t"
        "imull %%eax, %%eax\n\t"
        "movl %%eax, %[o1]\n\t"
        "leaq (%%rax, %[idx1], 4), %[o2]\n\t"
        "cvtsi2sd %%eax, %%xmm0\n\t"
        "movsd %%xmm0, %[o3]\n\t"
        : [o1] "=m" (*out1), 
          [o2] "=&r" (*out2),  /* Early clobber */
          [o3] "=m" (*out3)
        : [arr_val] "m" (arr[idx1]),  /* Memory operand */
          [idx1] "r" (idx2),          /* Mismatched: using idx2 for idx1 */
          "m" (arr[idx2])             /* Additional memory input */
        : "eax", "xmm0", "memory", "cc"
    );
}

/* Another complex asm with many operands */
static inline void asm_block2(int a, long b, double c, 
                              int *d, long *e, double *f,
                              int g, long h, double i) {
    register int r1 asm("r10") = a;
    register long r2 asm("r11") = b;
    register double r3 asm("xmm1") = c;
    
    asm volatile (
        "addl %[g_val], %[r1]\n\t"
        "imulq %[h_val], %[r2]\n\t"
        "addsd %[i_val], %[r3]\n\t"
        "movl %[r1], (%[d_ptr])\n\t"
        "movq %[r2], (%[e_ptr])\n\t"
        "movsd %[r3], (%[f_ptr])\n\t"
        : [r1] "+r" (r1),
          [r2] "+r" (r2),
          [r3] "+r" (r3),
          "=m" (*d),
          "=m" (*e),
          "=m" (*f)
        : [g_val] "ri" (g),      /* Register or immediate */
          [h_val] "r" (h),       /* Register constraint */
          [i_val] "x" (i),       /* SSE register constraint */
          [d_ptr] "r" (d),
          [e_ptr] "r" (e),
          [f_ptr] "r" (f)
        : "memory", "cc"
    );
}

/* Main test function */
int test_reload(void) {
    /* Create high register pressure with many register variables */
    register int v1 asm("ebx") = 1;
    register int v2 asm("r12d") = 2;
    register int v3 asm("r13d") = 3;
    register int v4 asm("r14d") = 4;
    register int v5 asm("r15d") = 5;
    register long l1 asm("rbp") = 100;
    register long l2 asm("r8") = 200;
    register long l3 asm("r9") = 300;
    register long l4 asm("r10") = 400;
    register long l5 asm("r11") = 500;
    register double d1 asm("xmm2") = 1.1;
    register double d2 asm("xmm3") = 2.2;
    register double d3 asm("xmm4") = 3.3;
    register double d4 asm("xmm5") = 4.4;
    register double d5 asm("xmm6") = 5.5;
    register int *p1 asm("rsi") = &v1;
    register int *p2 asm("rdi") = &v2;
    register long *p3 asm("rbx") = &l1;
    register double *p4 asm("r12") = &d1;
    
    /* Additional variables to increase pressure */
    register int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    register long l6 = 600, l7 = 700, l8 = 800, l9 = 900, l10 = 1000;
    register double d6 = 6.6, d7 = 7.7, d8 = 8.8, d9 = 9.9, d10 = 10.10;
    
    /* Multi-dimensional array with complex indexing */
    int arr[10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Perform arithmetic to create live ranges */
    v1 = v2 + v3 * v4 - v5;
    l1 = l2 * l3 / (l4 + 1);
    d1 = d2 * d3 - d4 / d5;
    
    v6 = helper1(&v1, &v2);
    l6 = helper2(&l1, &l2, &l3);
    d6 = helper3(&d1, &d2);
    
    /* Complex index calculations */
    int idx1 = (v1 + v2 * v3 - v4) % 10;
    int idx2 = (v5 + v6 * v7 - v8) % 10;
    int idx3 = (v9 + v10 * v1 - v2) % 10;
    
    /* First asm block with array indexing */
    int out1;
    long out2;
    double out3;
    
    asm_block1(&arr[0][0], idx1, idx2, &out1, &out2, &out3);
    
    /* Use results in calculations */
    v1 += out1;
    l1 += out2;
    d1 += out3;
    
    /* Second asm block with many operands and mismatched constraints */
    int out4;
    long out5;
    double out6;
    
    /* Force mismatched modes: using int where long expected, etc. */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "movq %[in2], %%rbx\n\t"      /* in2 is int but constraint expects long */
        "addq %%rax, %%rbx\n\t"
        "cvtsi2sd %[in3], %%xmm0\n\t" /* in3 is long but constraint expects double */
        "movl %%ebx, %[out4]\n\t"
        "movq %%rbx, %[out5]\n\t"
        "movsd %%xmm0, %[out6]\n\t"
        : [out4] "=m" (out4),
          [out5] "=m" (out5),
          [out6] "=m" (out6)
        : [in1] "r" (v1),            /* int in general reg */
          [in2] "r" ((long)v2),      /* int cast to long - potential mismatch */
          [in3] "r" (l1),            /* long in reg, used as double source */
          "m" (arr[idx1][idx2]),     /* Additional memory operand */
          "m" (arr[idx3][idx1])
        : "rax", "rbx", "xmm0", "memory", "cc"
    );
    
    /* Third asm block with 10 operands */
    int out7, out8, out9, out10;
    long out11, out12;
    double out13, out14;
    
    asm volatile (
        "movl %[a1], %%eax\n\t"
        "addl %[a2], %%eax\n\t"
        "movl %%eax, %[o7]\n\t"
        "movl %[a3], %%ebx\n\t"
        "subl %[a4], %%ebx\n\t"
        "movl %%ebx, %[o8]\n\t"
        "movq %[b1], %%rcx\n\t"
        "imulq %[b2], %%rcx\n\t"
        "movq %%rcx, %[o11]\n\t"
        "movsd %[c1], %%xmm0\n\t"
        "addsd %[c2], %%xmm0\n\t"
        "movsd %%xmm0, %[o13]\n\t"
        "movl %%eax, %[o9]\n\t"
        "movl %%ebx, %[o10]\n\t"
        "movq %%rcx, %[o12]\n\t"
        "movsd %%xmm0, %[o14]\n\t"
        : [o7] "=&r" (out7),   /* Early clobber */
          [o8] "=&r" (out8),   /* Early clobber */
          [o9] "=m" (out9),
          [o10] "=m" (out10),
          [o11] "=&r" (out11), /* Early clobber */
          [o12] "=m" (out12),
          [o13] "=&x" (out13), /* Early clobber SSE */
          [o14] "=m" (out14)
        : [a1] "r" (v3),
          [a2] "r" (v4),
          [a3] "r" (v5),
          [a4] "r" (v6),
          [b1] "r" (l3),
          [b2] "r" (l4),
          [c1] "x" (d3),       /* SSE register constraint */
          [c2] "x" (d4),       /* SSE register constraint */
          "m" (arr[idx2][idx3])
        : "rax", "rbx", "rcx", "xmm0", "memory", "cc"
    );
    
    /* Use all results to prevent optimization */
    int final1 = out1 + out4 + out7 + out8 + out9 + out10;
    long final2 = out2 + out5 + out11 + out12;
    double final3 = out3 + out6 + out13 + out14;
    
    /* More arithmetic with all variables */
    for (int i = 0; i < 5; i++) {
        v1 = v2 + v3;
        v2 = v3 + v4;
        v3 = v4 + v5;
        v4 = v5 + v6;
        v5 = v6 + v7;
        
        l1 = l2 + l3;
        l2 = l3 + l4;
        l3 = l4 + l5;
        l4 = l5 + l6;
        l5 = l6 + l7;
        
        d1 = d2 + d3;
        d2 = d3 + d4;
        d3 = d4 + d5;
        d4 = d5 + d6;
        d5 = d6 + d7;
        
        /* Array access with complex indices */
        arr[(v1 + i) % 10][(v2 + i) % 10] += final1;
        arr[(v3 + i) % 10][(v4 + i) % 10] += final2;
    }
    
    /* Final computation using all variables */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9 + l10;
    result += (int)(d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10);
    result += out1 + out4 + out7 + out8 + out9 + out10;
    result += arr[0][0] + arr[5][5] + arr[9][9];
    
    return result;
}

/* Entry point */
int main(void) {
    int result = test_reload();
    return result % 256; /* Return non-zero to indicate execution */
}
