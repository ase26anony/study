/* Test program to trigger push_reload logic in GCC reload pass */
#include <stdio.h>
#include <stdint.h>

/* Helper functions to create complex addressing */
int helper1(int *a, int b) { return *a + b; }
long helper2(long *a, long b, long c) { return *a * b + c; }
double helper3(double *a, double b) { return *a / b; }
void helper4(int *a, int *b, int *c) { *c = *a + *b; }

/* Complex inline assembly with register pressure */
int test_reload(void) {
    /* Create high register pressure with many register variables */
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
    register int v0 = 10, v1 = 20, v2 = 30, v3 = 40, v4 = 50;
    register long v5 = 60, v6 = 70, v7 = 80, v8 = 90, v9 = 100;
    register double v10 = 1.1, v11 = 2.2, v12 = 3.3, v13 = 4.4;
    
    /* Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Create live ranges and register pressure through computations */
    r0 = r1 + r2 * r3 - r4 / (r5 + 1);
    l0 = l1 * l2 + l3 - v5 * v6;
    d0 = d1 * d2 + d3 / v10 - v11;
    
    for (int i = 0; i < 5; i++) {
        v0 += v1 * v2 - v3 / (v4 + i);
        v5 += v6 * v7 - v8 / (v9 + i);
        v10 += v11 * v12 - v13 / (1.0 + i);
    }
    
    /* First complex inline asm: Many operands with mixed constraints */
    int result1, result2;
    long result3;
    double result4;
    
    asm volatile (
        /* 10 operands with mixed constraints */
        "mov %[in1], %%eax\n\t"
        "add %[in2], %%eax\n\t"
        "imul %[in3], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        "lea (%[in4],%[in5],4), %%ebx\n\t"
        "mov %%ebx, %[out2]\n\t"
        "mov %[in6], %%rcx\n\t"
        "add %[in7], %%rcx\n\t"
        "mov %%rcx, %[out3]\n\t"
        "movsd %[in8], %%xmm4\n\t"
        "addsd %[in9], %%xmm4\n\t"
        "movsd %%xmm4, %[out4]\n\t"
        : [out1] "=r" (result1),           /* Output constraint */
          [out2] "=&r" (result2),          /* Early-clobber output */
          [out3] "=r" (result3),           /* Long output */
          [out4] "=x" (result4)            /* XMM register output */
        : [in1] "r" (r0),                  /* Input in register */
          [in2] "m" (arr[r0%8][r1%8]),     /* Memory operand with complex addressing */
          [in3] "r" (helper1(&v0, v1)),    /* Function call in operand */
          [in4] "r" (r2),
          [in5] "r" (r3),
          [in6] "r" (l0),
          [in7] "r" (helper2(&l1, l2, l3)), /* Another function call */
          [in8] "x" (d0),                  /* XMM input */
          [in9] "m" (arr[r2%8][r3%8])      /* Memory operand with FP value cast */
        : "eax", "ebx", "ecx", "edx", "esi", "edi",
          "r8", "r9", "r10", "r11", "xmm4", "xmm5", "memory"
    );
    
    /* Use results to prevent dead code elimination */
    v0 += result1;
    v5 += result3;
    v10 += result4;
    
    /* Second inline asm: Mismatched modes and array indexing */
    int idx1 = r4 % 8;
    int idx2 = r5 % 8;
    long long_result;
    double double_result;
    
    asm volatile (
        /* Operands with potentially mismatched modes */
        "mov %[arr_elem], %%eax\n\t"
        "cvtsi2sd %%eax, %%xmm6\n\t"
        "movsd %[dbl_in], %%xmm7\n\t"
        "mulsd %%xmm6, %%xmm7\n\t"
        "cvttsd2si %%xmm7, %%ebx\n\t"
        "mov %%ebx, %[int_out]\n\t"
        "mov %[long_in], %%rcx\n\t"
        "add %[offset], %%rcx\n\t"
        "mov %%rcx, %[long_out]\n\t"
        "movsd %%xmm7, %[dbl_out]\n\t"
        : [int_out] "=r" (result1),
          [long_out] "=r" (long_result),
          [dbl_out] "=x" (double_result)
        : [arr_elem] "m" (arr[idx1][idx2]),      /* int from array */
          [dbl_in] "x" (d1),                     /* double input */
          [long_in] "r" (l2),                    /* long input */
          [offset] "i" (100LL)                   /* immediate */
        : "eax", "ebx", "ecx", "xmm6", "xmm7", "memory"
    );
    
    /* Third inline asm: Input-output operands with complex addressing */
    int io1 = v0;
    long io2 = v5;
    
    asm volatile (
        /* Input-output operands with '+' constraint */
        "add $100, %[io1]\n\t"
        "imul %[factor], %[io1]\n\t"
        "add %[addend], %[io2]\n\t"
        "shl $2, %[io2]\n\t"
        : [io1] "+r" (io1),
          [io2] "+r" (io2)
        : [factor] "r" (helper1(&v2, v3)),      /* Function call */
          [addend] "m" (arr[io1%8][io2%8])      /* Complex array indexing */
        : "cc", "memory"
    );
    
    /* More computations using all variables */
    for (int i = 0; i < 3; i++) {
        r0 = r1 + arr[i][i] - io1;
        l0 = l1 * io2 + arr[i][i+1];
        d0 = d1 + helper3(&v10, v11) - arr[i][i+2];
        
        /* Call helper with address-taken arguments */
        helper4(&r0, &r1, &arr[i][i+3]);
    }
    
    /* Final computation using all results */
    int final_sum = r0 + r1 + r2 + r3 + r4 + r5 +
                    (int)l0 + (int)l1 + (int)l2 + (int)l3 +
                    (int)d0 + (int)d1 + (int)d2 + (int)d3 +
                    v0 + v1 + v2 + v3 + v4 +
                    (int)v5 + (int)v6 + (int)v7 + (int)v8 + (int)v9 +
                    (int)v10 + (int)v11 + (int)v12 + (int)v13 +
                    result1 + result2 + (int)result3 + (int)result4 +
                    (int)long_result + (int)double_result + io1 + (int)io2;
    
    /* Use array elements to prevent optimization */
    for (int i = 0; i < 8; i++) {
        final_sum += arr[i][i];
    }
    
    return final_sum;
}

/* Main function to call test */
int main() {
    int result = test_reload();
    printf("Result: %d\n", result);
    return 0;
}
