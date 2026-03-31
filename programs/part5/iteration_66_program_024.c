/* Test program to trigger reload.cc uncovered lines 1381-1399 */
#include <stdint.h>
#include <stdlib.h>

/* Helper functions to create complex addressing modes */
static int helper1(int *a, int *b) { return *a + *b; }
static long helper2(long *a, long *b, long *c) { return *a + *b + *c; }
static double helper3(double *a, double *b) { return *a * *b; }
static void helper4(int *a, int *b, int *c) { *c = *a + *b; }

/* Function with high register pressure and complex inline assembly */
int test_reload(void) {
    /* Phase 1: Declare many register variables to create pressure */
    register int r0 asm("eax") = 1;
    register int r1 asm("ebx") = 2;
    register int r2 asm("ecx") = 3;
    register int r3 asm("edx") = 4;
    register int r4 asm("esi") = 5;
    register int r5 asm("edi") = 6;
    register long l0 asm("r8") = 100;
    register long l1 asm("r9") = 200;
    register long l2 asm("r10") = 300;
    register long l3 asm("r11") = 400;
    register double d0 asm("xmm0") = 1.1;
    register double d1 asm("xmm1") = 2.2;
    register double d2 asm("xmm2") = 3.3;
    register double d3 asm("xmm3") = 4.4;
    register int *p0 asm("r12") = &r0;
    register int *p1 asm("r13") = &r1;
    register long *p2 asm("r14") = &l0;
    register double *p3 asm("r15") = &d0;
    register int r6 = 7, r7 = 8, r8 = 9, r9 = 10;
    register int r10 = 11, r11 = 12, r12 = 13, r13 = 14;
    register long l4 = 500, l5 = 600, l6 = 700, l7 = 800;
    register double d4 = 5.5, d5 = 6.6, d6 = 7.7, d7 = 8.8;
    
    /* Phase 2: Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Phase 3: Arithmetic operations to create live ranges */
    r0 = r1 + r2 * r3 - r4 / (r5 + 1);
    l0 = l1 * l2 + l3 - l4;
    d0 = d1 * d2 + d3 / d4;
    
    for (int i = 0; i < 4; i++) {
        r6 += arr[i][i] * r7;
        l4 += arr[i][3] * l5;
        d4 += helper3(&d5, &d6) * d7;
    }
    
    /* Phase 4: First complex inline asm with many operands and mismatched modes */
    int result1, result2;
    long result3;
    double result4;
    
    asm volatile (
        /* Mixed constraints with early-clobber and memory */
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "imull %[in3], %[out2]\n\t"
        /* Mismatched mode: using DImode operation on SImode constraint */
        "movq %[in4], %%rax\n\t"
        "addq $100, %%rax\n\t"
        "movq %%rax, %[out3]\n\t"
        /* FP operation with mismatched constraint */
        "movsd %[in5], %%xmm4\n\t"
        "addsd %[in6], %%xmm4\n\t"
        "movsd %%xmm4, %[out4]\n\t"
        /* Complex addressing with function call result */
        "leal (%[idx1], %[idx2], 4), %%ecx\n\t"
        "movl arr(,%%ecx,4), %%edx\n\t"
        "addl %%edx, %[out1]\n\t"
        /* Input-output operand with early-clobber */
        "addl $1, %[inout1]\n\t"
        : [out1] "=r" (result1),           /* output */
          [out2] "=rm" (result2),          /* register or memory */
          [out3] "=&r" (result3),          /* early-clobber output */
          [out4] "=x" (result4),           /* xmm register */
          [inout1] "+&r" (r8)              /* early-clobber input-output */
        : [in1] "r" (r0),                  /* input */
          [in2] "rm" (r1),                 /* register or memory */
          [in3] "i" (5),                   /* immediate */
          [in4] "r" (l0),                  /* DImode input to SImode constraint */
          [in5] "x" (d0),                  /* xmm input */
          [in6] "xm" (d1),                 /* xmm or memory */
          [idx1] "r" (r2),                 /* index 1 */
          [idx2] "r" (r3),                  /* index 2 */
          "m" (arr)                        /* memory input */
        : "eax", "ecx", "edx", "rax", "xmm4", "xmm5", "xmm6", "memory", "cc"
    );
    
    /* Use results to prevent dead code elimination */
    r9 = result1 + result2;
    l5 = result3 + l1;
    d5 = result4 + d2;
    
    /* Phase 5: Second asm with array element operands and function calls */
    int arr_idx1 = r4 % 8;
    int arr_idx2 = r5 % 8;
    
    asm volatile (
        /* Complex array indexing in operands */
        "movl %[arr_elem], %%eax\n\t"
        "addl %%eax, %[sum]\n\t"
        /* Function call in operand expression */
        "pushq %%rbx\n\t"
        "movl %[addr1], %%edi\n\t"
        "movl %[addr2], %%esi\n\t"
        "call helper1\n\t"
        "popq %%rbx\n\t"
        "addl %%eax, %[sum]\n\t"
        /* Multiple output constraints */
        "movl %[in_a], %%eax\n\t"
        "movl %%eax, %[out_a]\n\t"
        "movl %[in_b], %%ebx\n\t"
        "movl %%ebx, %[out_b]\n\t"
        : [sum] "+r" (r10),                /* input-output */
          [out_a] "=r" (r11),              /* output */
          [out_b] "=r" (r12)               /* output */
        : [arr_elem] "rm" (arr[arr_idx1][arr_idx2]),  /* array element */
          [addr1] "r" (&r6),               /* address of register variable */
          [addr2] "r" (&r7),               /* address of register variable */
          [in_a] "r" (helper1(&r8, &r9)),  /* function call in input */
          [in_b] "r" (helper2(&l2, &l3, &l4)), /* another function call */
          "m" (arr)                        /* memory constraint */
        : "eax", "ebx", "edi", "esi", "memory", "cc"
    );
    
    /* Phase 6: Third asm with mixed types and clobbers */
    double array_elem_d = arr[3][3] * 0.1;
    
    asm volatile (
        /* Mixed integer/float operations */
        "cvtsi2sd %[int_in], %%xmm7\n\t"
        "addsd %[float_in], %%xmm7\n\t"
        "movsd %%xmm7, %[float_out]\n\t"
        /* Memory operand with complex addressing */
        "movl (%[base], %[index], 4), %%eax\n\t"
        "addl %%eax, %[int_out]\n\t"
        /* Multiple clobbered registers */
        "movl $0, %%ebx\n\t"
        "movl $0, %%ecx\n\t"
        : [float_out] "=x" (d6),           /* xmm output */
          [int_out] "=r" (r13)             /* integer output */
        : [int_in] "r" (r10),              /* integer input */
          [float_in] "xm" (array_elem_d),  /* float input */
          [base] "r" (arr),                /* array base */
          [index] "r" (r11)                /* index register */
        : "eax", "ebx", "ecx", "xmm7", "xmm8", "xmm9", "memory", "cc"
    );
    
    /* Phase 7: Final computation using all results */
    int final_sum = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12 + r13;
    final_sum += (int)l0 + (int)l1 + (int)l2 + (int)l3 + (int)l4 + (int)l5;
    final_sum += (int)d0 + (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 + (int)d6;
    
    /* Use array to prevent optimization */
    for (int i = 0; i < 8; i++) {
        final_sum += arr[i][i % 4];
    }
    
    return final_sum;
}

/* Main function to call test and prevent dead code elimination */
int main(void) {
    int result = test_reload();
    /* Use volatile to ensure computation isn't optimized away */
    volatile int dummy = result;
    return dummy % 256;  /* Return non-zero to indicate execution */
}
