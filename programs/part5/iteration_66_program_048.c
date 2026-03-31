/* Test program to trigger push_reload logic in GCC reload.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper functions to create complex addressing modes */
int helper1(int *a, int *b) {
    return *a + *b;
}

long helper2(long *a, long *b, long *c) {
    return *a * *b + *c;
}

double helper3(double *a, double *b) {
    return *a / *b;
}

void helper4(int *arr, int idx) {
    arr[idx] = idx * 2;
}

/* Complex inline assembly test function */
int test_reload(void) {
    /* Create high register pressure with many register variables */
    register int r0 asm("r0") = 1;
    register int r1 asm("r1") = 2;
    register int r2 asm("r2") = 3;
    register int r3 asm("r3") = 4;
    register int r4 asm("r4") = 5;
    register int r5 asm("r5") = 6;
    register int r6 asm("r6") = 7;
    register int r7 asm("r7") = 8;
    register int r8 asm("r8") = 9;
    register int r9 asm("r9") = 10;
    register long l0 asm("r10") = 100;
    register long l1 asm("r11") = 200;
    register long l2 asm("r12") = 300;
    register long l3 asm("r13") = 400;
    register long l4 asm("r14") = 500;
    register double d0 asm("xmm0") = 1.0;
    register double d1 asm("xmm1") = 2.0;
    register double d2 asm("xmm2") = 3.0;
    register double d3 asm("xmm3") = 4.0;
    register double d4 asm("xmm4") = 5.0;
    register int *p0 asm("r15") = &r0;
    register int *p1 asm("ebx") = &r1;
    register int *p2 asm("esi") = &r2;
    register int *p3 asm("edi") = &r3;
    
    /* Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 8 + j;
        }
    }
    
    /* Create live ranges and register pressure through computations */
    r0 = r1 + r2 * r3 - r4 / (r5 + 1);
    r1 = r2 ^ r3 | r4 & r5;
    r2 = r3 << r4 >> r5;
    l0 = l1 * l2 + l3 - l4;
    l1 = l2 / (l3 + 1) * l4;
    d0 = d1 * d2 + d3 / d4;
    d1 = d2 - d3 * d4;
    
    /* Complex inline assembly block 1: Many operands with mixed constraints */
    int result1, result2, result3;
    long result4;
    double result5;
    
    asm volatile (
        /* 8 operands with mixed constraints */
        "mov %[in1], %%eax\n\t"
        "add %[in2], %%eax\n\t"
        "imul %[in3], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        "mov %[in4], %%ebx\n\t"
        "sub %[in5], %%ebx\n\t"
        "mov %%ebx, %[out2]\n\t"
        "mov %[in6], %%rcx\n\t"
        "xor %[in7], %%rcx\n\t"
        "mov %%rcx, %[out3]\n\t"
        "movsd %[in8], %%xmm0\n\t"
        "addsd %[in9], %%xmm0\n\t"
        "movsd %%xmm0, %[out4]\n\t"
        : [out1] "=&r" (result1),      /* Early clobber */
          [out2] "=r" (result2),
          [out3] "=r" (result3),
          [out4] "=x" (result5)
        : [in1] "r" (r0),
          [in2] "r" (r1),
          [in3] "r" (r2),
          [in4] "m" (arr[r0 % 8][r1 % 8]),  /* Memory operand with complex addressing */
          [in5] "r" (r3),
          [in6] "r" (l0),
          [in7] "r" (l1),
          [in8] "x" (d0),
          [in9] "x" (d1)
        : "eax", "ebx", "rcx", "xmm0", "xmm1", "xmm2", "memory"
    );
    
    /* Use results to prevent dead code elimination */
    r4 = result1 + result2;
    l2 = result3 * 2;
    d2 = result5 * 3.14;
    
    /* Complex inline assembly block 2: Mismatched modes and function calls */
    int idx1 = r0 % 7;
    int idx2 = r1 % 7;
    int idx3 = r2 % 7;
    
    /* Function calls in asm operands create complex addressing */
    int complex_val1 = helper1(&arr[idx1][idx2], &arr[idx2][idx3]);
    long complex_val2 = helper2(&l0, &l1, &l2);
    double complex_val3 = helper3(&d0, &d1);
    
    int out_val1, out_val2;
    long out_val3;
    
    asm volatile (
        /* Operands with mismatched modes/classes */
        "mov %[in1], %%eax\n\t"
        "add %[in2], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        "mov %[in3], %%rbx\n\t"
        "sub %[in4], %%rbx\n\t"
        "mov %%rbx, %[out2]\n\t"
        "cvtsi2sd %[in5], %%xmm0\n\t"
        "addsd %[in6], %%xmm0\n\t"
        "cvtsd2si %%xmm0, %%ecx\n\t"
        "mov %%ecx, %[out3]\n\t"
        : [out1] "=&r" (out_val1),     /* Early clobber */
          [out2] "=r" (out_val2),
          [out3] "=r" (out_val3)
        : [in1] "r" (complex_val1),    /* Integer from function call */
          [in2] "r" (arr[idx1][idx2]), /* Array element */
          [in3] "r" (complex_val2),    /* Long from function call */
          [in4] "m" (arr[idx3][idx1]), /* Memory constraint */
          [in5] "r" (r5),              /* Integer */
          [in6] "x" (complex_val3)     /* Double - mismatched with 'r' constraint */
        : "eax", "ebx", "ecx", "xmm0", "xmm1", "xmm2", "xmm3", "memory"
    );
    
    /* Complex inline assembly block 3: IO operands and array indexing */
    int io_var1 = r6;
    long io_var2 = l3;
    double io_var3 = d3;
    
    /* Multi-dimensional array indexing in operands */
    asm volatile (
        "mov %[io1], %%eax\n\t"
        "add $100, %%eax\n\t"
        "mov %%eax, %[io1]\n\t"
        "mov %[io2], %%rbx\n\t"
        "sub $50, %%rbx\n\t"
        "mov %%rbx, %[io2]\n\t"
        "movsd %[io3], %%xmm0\n\t"
        "mulsd %[in1], %%xmm0\n\t"
        "movsd %%xmm0, %[io3]\n\t"
        "lea (%[base], %[idx1], 4), %%rcx\n\t"
        "mov (%%rcx, %[idx2], 4), %%edx\n\t"
        "mov %%edx, %[out1]\n\t"
        : [io1] "+&r" (io_var1),       /* Input-output with early clobber */
          [io2] "+r" (io_var2),
          [io3] "+x" (io_var3),
          [out1] "=r" (result1)
        : [in1] "x" (d4),
          [base] "r" (&arr[0][0]),     /* Base pointer */
          [idx1] "r" (r7 % 4),         /* Index 1 */
          [idx2] "r" (r8 % 4)          /* Index 2 */
        : "eax", "ebx", "rcx", "edx", "xmm0", "xmm1", "memory"
    );
    
    /* Use all results in final computation */
    int final_sum = 0;
    final_sum += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9;
    final_sum += l0 + l1 + l2 + l3 + l4;
    final_sum += (int)d0 + (int)d1 + (int)d2 + (int)d3 + (int)d4;
    final_sum += result1 + result2 + result3 + out_val1 + out_val2;
    final_sum += (int)io_var1 + (int)io_var2 + (int)io_var3;
    final_sum += arr[0][0] + arr[7][7];
    
    /* More computations to increase register pressure */
    for (int i = 0; i < 8; i++) {
        helper4(&arr[i][0], i);
        for (int j = 0; j < 8; j++) {
            final_sum += arr[i][j];
        }
    }
    
    return final_sum;
}

/* Main function to run the test */
int main(void) {
    int result = test_reload();
    printf("Result: %d\n", result);
    return 0;
}
