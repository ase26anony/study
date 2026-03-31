/* Test program to trigger reload.cc uncovered lines 1381-1399 */
#include <stdio.h>
#include <stdint.h>

/* Helper functions to create complex addressing */
int helper1(int *a, int *b) { return *a + *b; }
long helper2(long *a, long *b, long *c) { return *a + *b + *c; }
double helper3(double *a, double *b) { return *a * *b; }
void helper4(int *a, int *b, int *c) { *c = *a + *b; }
int* helper5(int *a, int *b) { return (*a > *b) ? a : b; }

/* Complex inline assembly with multiple operands and mismatched modes */
void test_reload() {
    /* High register pressure: 20+ register variables */
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
    register double d0 asm("xmm0") = 1.0;
    register double d1 asm("xmm1") = 2.0;
    register double d2 asm("xmm2") = 3.0;
    register double d3 asm("xmm3") = 4.0;
    register int *p0 asm("r12") = &r0;
    register int *p1 asm("r13") = &r1;
    register long *p2 asm("r14") = &l0;
    register double *p3 asm("r15") = &d0;
    register int r6 = 7, r7 = 8, r8 = 9, r9 = 10;
    register int r10 = 11, r11 = 12, r12 = 13, r13 = 14;
    
    /* Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Arithmetic operations to create live ranges */
    r0 = r1 + r2 * r3 - r4 / (r5 + 1);
    l0 = l1 * l2 + l3 - (r0 * 2);
    d0 = d1 * d2 + d3 / 2.0;
    p0 = &r6 + (r7 >> 2);
    
    /* Complex inline assembly block 1: Many operands with mixed constraints */
    asm volatile (
        "mov %[in1], %%eax\n\t"
        "add %[in2], %%eax\n\t"
        "imul %[in3], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        "lea (%[in4],%[in5],4), %%ebx\n\t"
        "mov %%ebx, %[out2]\n\t"
        "movq %[din1], %%xmm4\n\t"
        "addsd %[din2], %%xmm4\n\t"
        "movq %%xmm4, %[dout]\n\t"
        : [out1] "=&r" (r0),      /* Early clobber output */
          [out2] "=r" (r1),       /* Regular output */
          [dout] "=m" (arr[2][3]) /* Memory output */
        : [in1] "r" (r2),         /* Input in register */
          [in2] "m" (arr[r3][r4]),/* Memory input with complex addressing */
          [in3] "i" (5),          /* Immediate */
          [in4] "r" (r5),
          [in5] "r" (r6),
          [din1] "x" (d0),        /* SSE register */
          [din2] "xm" (d1)        /* SSE register or memory */
        : "eax", "ebx", "xmm4", "memory", "cc"
    );
    
    /* Use results to prevent dead code elimination */
    r10 = r0 + r1 + arr[2][3];
    
    /* Complex inline assembly block 2: Mismatched modes and function calls */
    int temp1, temp2, temp3;
    long temp4;
    double temp5;
    
    /* Function calls in operands create complex addressing */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "addl %%eax, %[out1]\n\t"
        "movq %[in2], %%xmm0\n\t"
        "subsd %[in3], %%xmm0\n\t"
        "movq %%xmm0, %[out2]\n\t"
        "leal (%[in4],%[in5],2), %%ecx\n\t"
        "movl %%ecx, %[out3]\n\t"
        : [out1] "+&r" (temp1),           /* Read-write with early clobber */
          [out2] "=m" (temp5),            /* Memory output for double */
          [out3] "=r" (temp2)             /* Register output */
        : [in1] "r" (helper1(&r0, &r1)),  /* Function call in input */
          [in2] "x" (helper3(&d0, &d1)),  /* Another function call */
          [in3] "xm" (d2),                /* Mixed constraint */
          [in4] "r" (helper1(&r2, &r3)),  /* Function call */
          [in5] "r" (arr[helper1(&r4, &r5)][helper1(&r6, &r7)]) /* Complex array indexing */
        : "eax", "ecx", "xmm0", "xmm1", "xmm2", "memory", "cc"
    );
    
    /* More arithmetic to increase pressure */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
        }
    }
    
    /* Complex inline assembly block 3: Multi-dimensional array operands */
    asm volatile (
        "mov %[arr_in], %%rax\n\t"
        "mov (%%rax), %%ebx\n\t"
        "add %[idx1], %%ebx\n\t"
        "mov %%ebx, %[out1]\n\t"
        "mov %[ptr_in], %%rcx\n\t"
        "mov (%%rcx), %%edx\n\t"
        "imul %[idx2], %%edx\n\t"
        "mov %%edx, %[out2]\n\t"
        "movd %[vec_in], %%xmm5\n\t"
        "paddd %[vec_const], %%xmm5\n\t"
        "movd %%xmm5, %[out3]\n\t"
        : [out1] "=&r" (temp3),
          [out2] "=r" (temp4),      /* long output from int operation */
          [out3] "=r" (r11)
        : [arr_in] "r" (&arr[r8 % 8][r9 % 8]),  /* Complex array address */
          [idx1] "r" (r10),
          [ptr_in] "r" (helper5(&r0, &r1)),     /* Function returning pointer */
          [idx2] "r" (r12),
          [vec_in] "x" (arr[3][4]),             /* Integer treated as vector */
          [vec_const] "x" (0x01010101)          /* Vector constant */
        : "rax", "rbx", "rcx", "rdx", "xmm5", "memory", "cc"
    );
    
    /* Final computation using all results */
    int final_result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12 + r13;
    final_result += temp1 + temp2 + temp3 + (int)temp4 + (int)temp5;
    final_result += arr[0][0] + arr[7][7];
    
    printf("Result: %d\n", final_result);
}

/* Main function to call test */
int main() {
    test_reload();
    return 0;
}
