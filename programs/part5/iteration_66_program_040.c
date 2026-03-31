/* Test program to trigger push_reload logic in GCC reload.cc */
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

/* Complex inline assembly with mismatched constraints */
static inline void asm_block1(int *arr, long idx1, long idx2, 
                             int *out1, long *out2, double *out3) {
    __asm__ volatile (
        "movl %%eax, %%ecx\n\t"
        "addl %[in1], %%ecx\n\t"
        "imull %[in2], %%ecx\n\t"
        "movl %%ecx, %[out1]\n\t"
        "leaq (%[arr],%[idx1],4), %%r8\n\t"
        "movq (%%r8), %[out2]\n\t"
        "cvtsi2sd %[in3], %%xmm0\n\t"
        "movsd %%xmm0, %[out3]"
        : [out1] "=m" (*out1), 
          [out2] "=&r" (*out2),  /* Early clobber */
          [out3] "=m" (*out3)
        : [in1] "r" (idx1),      /* SImode but used as index */
          [in2] "r" (idx2),
          [in3] "r" (*arr),      /* Memory operand forced to reg */
          [arr] "r" (arr),
          [idx1] "r" (idx1)
        : "eax", "ecx", "r8", "xmm0", "memory", "cc"
    );
}

/* Inline assembly with FP/vector mode mismatches */
static inline void asm_block2(double *dbl_arr, int *int_arr, 
                             float *flt_arr, int dim1, int dim2) {
    __asm__ volatile (
        "movdqu (%[dbl]), %%xmm1\n\t"    /* DImode -> V2DF mismatch */
        "movdqu (%[int]), %%xmm2\n\t"    /* SImode -> V4SI */
        "addps %%xmm2, %%xmm1\n\t"       /* Mixed precision */
        "movdqu %%xmm1, (%[flt])\n\t"
        "cvttsd2si %%xmm0, %%eax\n\t"
        "addl %[dim1], %%eax\n\t"
        "imull %[dim2], %%eax"
        : 
        : [dbl] "r" (dbl_arr),
          [int] "r" (int_arr),
          [flt] "r" (flt_arr),
          [dim1] "r" (dim1),     /* Integer in FP context */
          [dim2] "r" (dim2)
        : "eax", "xmm0", "xmm1", "xmm2", "memory"
    );
}

/* Main test function with high register pressure */
int test_reload(void) {
    /* Multi-dimensional array */
    int md_arr[8][8][8];
    double dbl_arr[8][8];
    float flt_arr[8][8];
    
    /* Many register variables to create pressure */
    register int r0 asm("r12") = 1;
    register int r1 asm("r13") = 2;
    register int r2 asm("r14") = 3;
    register int r3 asm("r15") = 4;
    register long r4 asm("rbx") = 5;
    register long r5 asm("rbp") = 6;
    register double r6 asm("xmm8") = 7.0;
    register double r7 asm("xmm9") = 8.0;
    register double r8 asm("xmm10") = 9.0;
    register double r9 asm("xmm11") = 10.0;
    register int r10 asm("r8") = 11;
    register int r11 asm("r9") = 12;
    register int r12 asm("r10") = 13;
    register int r13 asm("r11") = 14;
    register long r14 asm("rax") = 15;
    register long r15 asm("rcx") = 16;
    register long r16 asm("rdx") = 17;
    register long r17 asm("rsi") = 18;
    register long r18 asm("rdi") = 19;
    register double r19 asm("xmm12") = 20.0;
    register double r20 asm("xmm13") = 21.0;
    register double r21 asm("xmm14") = 22.0;
    register double r22 asm("xmm15") = 23.0;
    register int *r23 asm("r12") = &r0;  /* Reuse constrained reg */
    register long *r24 asm("r13") = &r4;
    register double *r25 asm("r14") = &r6;
    
    /* Initialize arrays with complex indexing */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                md_arr[i][j][k] = i * 64 + j * 8 + k;
            }
            dbl_arr[i][j] = i * 8.0 + j * 1.0;
            flt_arr[i][j] = i * 4.0f + j * 0.5f;
        }
    }
    
    /* Complex computations to create live ranges */
    r0 = r1 * r2 + r3;
    r4 = r5 * r6 + r7;  /* Mixed int/double */
    r8 = r9 / r10 + r11;
    r12 = r13 | r14 & r15;
    r16 = r17 ^ r18 + r19;
    r20 = r21 * r22 - r0;
    
    /* Chain computations to prevent dead code elimination */
    for (int i = 0; i < 100; i++) {
        r0 = r0 + r1;
        r1 = r1 * r2;
        r2 = r2 - r3;
        r3 = r3 / (r4 ? r4 : 1);
        r4 = r4 ^ r5;
        r5 = r5 | r6;
    }
    
    /* First complex asm with many operands and function calls */
    int out1;
    long out2;
    double out3;
    
    /* Use array elements with complex addressing in asm operands */
    __asm__ volatile (
        "movl %[idx1], %%eax\n\t"
        "addl %[idx2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "leaq (%[base],%[idx1],8), %%rcx\n\t"
        "leaq (%%rcx,%[idx2],4), %%rdx\n\t"
        "movq (%%rdx), %[out2]\n\t"
        "cvtsi2sd %[val], %%xmm0\n\t"
        "addsd %[dbl], %%xmm0\n\t"
        "movsd %%xmm0, %[out3]"
        : [out1] "=m" (out1),
          [out2] "=&r" (out2),      /* Early clobber */
          [out3] "=m" (out3)
        : [idx1] "r" (helper1(&r0, &r1)),  /* Function call in operand */
          [idx2] "r" (helper2(&r4, &r5, &r16)),
          [base] "r" (md_arr[0][0]),
          [val] "r" (md_arr[r0 % 8][r1 % 8][r2 % 8]),  /* Complex array index */
          [dbl] "r" (dbl_arr[r3 % 8][r4 % 8])          /* Another array element */
        : "eax", "ecx", "rdx", "xmm0", "memory", "cc"
    );
    
    /* Use results */
    r0 += out1;
    r4 += out2;
    r6 += out3;
    
    /* Second asm with mismatched modes */
    int idx_i = r0 % 8;
    int idx_j = r1 % 8;
    int idx_k = r2 % 8;
    
    __asm__ volatile (
        "movd %[int_val], %%xmm0\n\t"      /* SImode -> V4SI */
        "movd %[int_val2], %%xmm1\n\t"
        "paddd %%xmm1, %%xmm0\n\t"
        "movd %%xmm0, %[out]\n\t"
        "cvtsi2sd %[arr_elem], %%xmm2\n\t" /* Memory -> XMM */
        "mulsd %[dbl_elem], %%xmm2\n\t"
        "movsd %%xmm2, %[dbl_out]"
        : [out] "=r" (r10),
          [dbl_out] "=m" (r19)
        : [int_val] "r" (md_arr[idx_i][idx_j][idx_k]),  /* Array element */
          [int_val2] "r" (md_arr[idx_j][idx_k][idx_i]), /* Different permutation */
          [arr_elem] "m" (md_arr[idx_k][idx_i][idx_j]), /* Memory constraint */
          [dbl_elem] "r" (dbl_arr[idx_i][idx_j])        /* FP value */
        : "xmm0", "xmm1", "xmm2", "memory"
    );
    
    /* Third asm with input-output operands */
    __asm__ volatile (
        "addl %[inc], %[io1]\n\t"
        "imull %[io1], %[io2]\n\t"
        "addq %[io2], %[io3]\n\t"
        "cvtsi2sd %[io3], %%xmm3\n\t"
        "addsd %[io4], %%xmm3\n\t"
        "movsd %%xmm3, %[io4]"
        : [io1] "+r" (r0),      /* Input-output */
          [io2] "+&r" (r1),     /* Early clobber input-output */
          [io3] "+r" (r4),
          [io4] "+r" (r6)
        : [inc] "r" (helper3(&r7, &r8))  /* Function call */
        : "xmm3", "memory", "cc"
    );
    
    /* Use asm_block1 with complex addressing */
    int arr_out1;
    long arr_out2;
    double arr_out3;
    
    asm_block1(&md_arr[r0 % 8][r1 % 8][0], 
               helper2(&r4, &r5, &r16),
               helper1(&r0, &r1),
               &arr_out1, &arr_out2, &arr_out3);
    
    /* Use asm_block2 with mode mismatches */
    asm_block2(&dbl_arr[r0 % 8][0],
               &md_arr[r1 % 8][r2 % 8][0],
               &flt_arr[r3 % 8][0],
               helper1(&r10, &r11),
               helper2(&r12, &r13, &r14));
    
    /* Final computation using all results */
    int final_sum = r0 + r1 + r2 + r3 + r10 + r11 + r12 + r13;
    final_sum += (int)r4 + (int)r5 + (int)r6 + (int)r7;
    final_sum += (int)r19 + (int)r20 + (int)arr_out1 + (int)arr_out2;
    final_sum += md_arr[final_sum % 8][0][0];
    
    return final_sum;
}

/* Entry point */
int main(void) {
    int result = test_reload();
    /* Use result to prevent optimization */
    return result % 256;
}
