/* Test program to trigger reload.cc lines 1381-1399 */
#include <stdio.h>
#include <stdint.h>

/* Helper functions to create complex addressing */
int helper1(int *a, int b) { return *a + b; }
long helper2(long *a, long b, long c) { return *a * b + c; }
double helper3(double *a, double b) { return *a / b; }
void helper4(int *a, int *b, int *c) { *c = *a + *b; }
int* helper5(int *a, int idx) { return &a[idx]; }

/* Complex inline assembly with register pressure */
void test_reload(void) {
    /* High register pressure: 20+ register variables */
    register int r0 asm("eax") = 1;
    register int r1 asm("ebx") = 2;
    register int r2 asm("ecx") = 3;
    register int r3 asm("edx") = 4;
    register int r4 asm("esi") = 5;
    register int r5 asm("edi") = 6;
    register long r6 asm("r8") = 7L;
    register long r7 asm("r9") = 8L;
    register long r8 asm("r10") = 9L;
    register long r9 asm("r11") = 10L;
    register double d0 asm("xmm0") = 1.0;
    register double d1 asm("xmm1") = 2.0;
    register double d2 asm("xmm2") = 3.0;
    register double d3 asm("xmm3") = 4.0;
    register double d4 asm("xmm4") = 5.0;
    register double d5 asm("xmm5") = 6.0;
    register int *p0 asm("r12") = &r0;
    register int *p1 asm("r13") = &r1;
    register long *p2 asm("r14") = &r6;
    register double *p3 asm("r15") = &d0;
    register int r10 asm("r8d") = 11;  /* Conflict with r6 */
    register int r11 asm("r9d") = 12;  /* Conflict with r7 */
    
    /* Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Pre-asm computations to create live ranges */
    r0 = r1 * r2 + r3;
    r1 = r4 ^ r5 | r0;
    r6 = r7 * r8 / (r9 + 1);
    r7 = r6 << 3;
    d0 = d1 * d2 + d3;
    d1 = d4 / d5 - d0;
    p0 = &arr[0][0];
    p1 = &arr[1][1];
    
    /* ASM BLOCK 1: Complex inline assembly with 8 operands */
    /* Mixed constraints with early-clobber and mismatched modes */
    asm volatile (
        "/* Complex asm with many operands */\n\t"
        "mov %[in1], %%eax\n\t"
        "add %[in2], %%eax\n\t"
        "imul %[in3], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        "lea (%[in4], %[in5], 4), %%ebx\n\t"
        "mov %%ebx, %[out2]\n\t"
        "movd %[in6], %%xmm0\n\t"
        "paddd %%xmm0, %%xmm0\n\t"
        "movd %%xmm0, %[out3]"
        : [out1] "=&r" (r0),        /* early-clobber output */
          [out2] "=r" (r1),         /* regular output */
          [out3] "=m" (arr[2][2])   /* memory output */
        : [in1] "r" (r2),           /* register input */
          [in2] "m" (arr[1][1]),    /* memory input */
          [in3] "i" (3),            /* immediate */
          [in4] "r" (r3),           /* register */
          [in5] "r" (r4),           /* register */
          [in6] "x" (d0)            /* SSE register - mismatched with output */
        : "eax", "ebx", "xmm0", "memory", "cc"
    );
    
    /* Intermediate computations */
    r2 = helper1(&r0, r1);
    r3 = helper1(&arr[2][2], r2);
    r8 = helper2(&r6, r7, r9);
    d2 = helper3(&d0, d1);
    
    /* ASM BLOCK 2: More complex with function calls in operands */
    /* Mismatched operand modes and register classes */
    int idx1 = r0 % 8;
    int idx2 = r1 % 8;
    
    asm volatile (
        "/* Asm with function calls and array indexing */\n\t"
        "mov %[arr_elem], %%eax\n\t"
        "add %[func_res], %%eax\n\t"
        "mov %%eax, %[out_val]\n\t"
        "mov %[ptr_val], %%ebx\n\t"
        "add $4, %%ebx\n\t"
        "mov %%ebx, %[out_ptr]"
        : [out_val] "=r" (r4),      /* SImode output */
          [out_ptr] "=r" (p0)       /* DImode pointer output - potential mismatch */
        : [arr_elem] "m" (arr[idx1][idx2]),  /* Complex addressing */
          [func_res] "r" (helper1(&r2, r3)), /* Function call in operand */
          [ptr_val] "r" (helper5(arr[idx2], idx1))  /* Another function call */
        : "eax", "ebx", "memory", "cc"
    );
    
    /* More register pressure */
    register int r12 asm("r8d") = r0 + r1;
    register int r13 asm("r9d") = r2 * r3;
    register double d6 asm("xmm6") = d0 + d1;
    register double d7 asm("xmm7") = d2 * d3;
    
    /* ASM BLOCK 3: Input-output operands with clobbers */
    /* Mixed constraints including "+r" and memory */
    int temp1 = 100;
    long temp2 = 200;
    double temp3 = 3.14;
    
    asm volatile (
        "/* Input-output operands with many clobbers */\n\t"
        "add %[inc], %[io1]\n\t"
        "imul %[io1], %[io2]\n\t"
        "movq %[fp_in], %%xmm0\n\t"
        "addsd %%xmm0, %%xmm0\n\t"
        "movq %%xmm0, %[fp_out]"
        : [io1] "+r" (temp1),       /* input-output */
          [io2] "+r" (temp2),       /* input-output */
          [fp_out] "=m" (temp3)     /* memory output */
        : [inc] "i" (10),           /* immediate */
          [fp_in] "x" (d6)          /* SSE register input */
        : "xmm0", "xmm1", "xmm2", "memory", "cc"
    );
    
    /* Use results to prevent dead code elimination */
    helper4(&r4, &temp1, &arr[3][3]);
    arr[4][4] = temp2;
    d3 = temp3;
    
    /* Final computation using all variables */
    int sum = r0 + r1 + r2 + r3 + r4 + r5 + r10 + r11 + r12 + r13;
    sum += arr[0][0] + arr[1][1] + arr[2][2] + arr[3][3] + arr[4][4];
    sum += (int)d0 + (int)d1 + (int)d2 + (int)d3 + (int)d6 + (int)d7;
    
    printf("Result: %d\n", sum);
}

/* Main function to call test */
int main(void) {
    test_reload();
    return 0;
}
