/* reload_test.c - Test program to trigger push_reload logic in GCC reload pass */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper functions for address-taken arguments */
static int helper1(int *a, int *b) {
    return (*a + *b) * 2;
}

static long helper2(long *a, long *b, long *c) {
    return *a * *b + *c;
}

static double helper3(double *a, double *b) {
    return *a / *b + 1.0;
}

/* Function to create register pressure and complex reload scenarios */
int test_reload(void) {
    /* Phase 1: Declare many register variables to create pressure */
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
    register double d0 asm("xmm0") = 1.1;
    register double d1 asm("xmm1") = 2.2;
    register double d2 asm("xmm2") = 3.3;
    register double d3 asm("xmm3") = 4.4;
    register double d4 asm("xmm4") = 5.5;
    register int *p0 asm("r15") = &r0;
    register int *p1 asm("rbx") = &r1;
    register long *p2 asm("rbp") = &l0;
    register double *p3 asm("rsi") = &d0;
    
    /* Additional non-register variables */
    int arr[10][10];
    volatile int mem_volatile = 0;
    long long huge_array[5][5][5];
    
    /* Phase 2: Initialize multi-dimensional arrays with complex patterns */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            arr[i][j] = i * 100 + j + r0 + r1;
        }
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            for (int k = 0; k < 5; k++) {
                huge_array[i][j][k] = (long long)i * j * k * l0;
            }
        }
    }
    
    /* Phase 3: Arithmetic operations to create live ranges */
    r0 = r1 + r2 * r3 - r4 / (r5 + 1);
    r1 = r6 ^ r7 | r8 & r9;
    l0 = l1 * l2 + l3 - l4;
    l1 = l2 / (l3 + 1) * l4;
    d0 = d1 + d2 * d3 - d4;
    d1 = d2 / d3 + d4 * d0;
    
    /* Complex pointer arithmetic */
    p0 = &arr[r0 % 10][r1 % 10];
    p1 = &arr[r2 % 10][r3 % 10];
    p2 = &huge_array[r4 % 5][r5 % 5][r6 % 5];
    
    /* Phase 4: First complex inline asm with many operands and mismatched modes */
    int result1, result2;
    long result3;
    double result4;
    
    asm volatile (
        /* 10 operands mixing input, output, and input-output */
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out1]\n\t"
        "imul %[in3], %[out1]\n\t"
        "mov %[in4], %[out2]\n\t"
        "lea (%[in5],%[in6],4), %[out2]\n\t"
        "mov %[in7], %[out3]\n\t"
        "add %[in8], %[out3]\n\t"
        "movsd %[in9], %[out4]\n\t"
        "addsd %[in10], %[out4]\n\t"
        "mulsd %[out4], %[out4]"
        : [out1] "=&r" (result1),      /* Early clobber output */
          [out2] "+&r" (result2),      /* Early clobber input-output */
          [out3] "=r" (result3),       /* Regular output */
          [out4] "=x" (result4)        /* XMM register output */
        : [in1] "r" (r0),              /* Register input */
          [in2] "r" (r1),
          [in3] "r" (r2),
          [in4] "m" (arr[2][3]),       /* Memory input */
          [in5] "r" (r3),
          [in6] "r" (r4),
          [in7] "r" (l0),              /* Long in general reg */
          [in8] "m" (huge_array[1][2][3]), /* Memory with complex address */
          [in9] "x" (d0),              /* XMM input */
          [in10] "x" (d1)              /* XMM input */
        : "memory", "cc",              /* Memory and condition codes clobber */
          "rax", "rbx", "rcx", "rdx",  /* Specific register clobbers */
          "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
    );
    
    /* Use results to prevent dead code elimination */
    r0 = result1 + result2;
    l0 = result3;
    d0 = result4;
    
    /* Phase 5: Second asm with nested function calls in operands */
    int func_result1, func_result2;
    double func_result3;
    
    /* Create more register pressure */
    register int t0 asm("eax") = r0 + 100;
    register int t1 asm("ebx") = r1 + 200;
    register long t2 asm("ecx") = l0 + 300;
    register double t3 asm("xmm6") = d0 + 400.0;
    
    asm volatile (
        /* Complex asm with function calls in input expressions */
        "mov %[call1], %%eax\n\t"
        "add %[call2], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        "mov %[arr_elem], %%ebx\n\t"
        "imul %%ebx, %[out1]\n\t"
        "movq %[call3], %%xmm0\n\t"
        "addsd %%xmm0, %%xmm0\n\t"
        "movsd %%xmm0, %[out2]"
        : [out1] "=r" (func_result1),
          [out2] "=x" (func_result3)
        : [call1] "i" (helper1(&r0, &r1)),  /* Function call in input */
          [call2] "i" (helper2(&l0, &l1, &l2)),
          [call3] "x" (helper3(&d0, &d1)),
          [arr_elem] "m" (arr[r2 % 10][r3 % 10])  /* Complex array indexing */
        : "memory", "cc",
          "rax", "rbx", "rcx", "rdx",
          "xmm0", "xmm1", "xmm2", "xmm3"
    );
    
    /* Phase 6: Third asm with mismatched operand modes */
    int8_t small_val = 42;
    int64_t large_val = 0x123456789ABCDEF0LL;
    float float_val = 3.14159f;
    double double_val = 2.71828;
    
    /* Force different register classes */
    register int32_t reg32 asm("edi") = 0xDEADBEEF;
    register int64_t reg64 asm("r8") = 0xCAFEBABECAFEBABELL;
    register __m128i vec128 asm("xmm7");
    
    int final_result;
    
    asm volatile (
        /* Mixing different operand sizes and register classes */
        "mov %[small], %%al\n\t"
        "movsx %%al, %%eax\n\t"
        "add %[reg32], %%eax\n\t"
        "mov %[large], %%rdx\n\t"
        "add %%rdx, %%rax\n\t"
        "cvtsi2ss %[reg32], %%xmm0\n\t"
        "addss %[float_val], %%xmm0\n\t"
        "cvtss2sd %%xmm0, %%xmm1\n\t"
        "addsd %[double_val], %%xmm1\n\t"
        "cvtsd2si %%xmm1, %%ecx\n\t"
        "add %%ecx, %%eax\n\t"
        "mov %%eax, %[result]"
        : [result] "=r" (final_result)
        : [small] "r" ((int)small_val),      /* 8-bit promoted to 32-bit */
          [reg32] "r" (reg32),              /* 32-bit in 64-bit register */
          [large] "r" (large_val),          /* 64-bit value */
          [float_val] "x" (float_val),      /* float in XMM */
          [double_val] "x" (double_val),    /* double in XMM */
          "m" (arr[reg32 % 10][0]),         /* Memory operand */
          "m" (huge_array[0][reg32 % 5][0])
        : "memory", "cc",
          "rax", "rcx", "rdx",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
    );
    
    /* Phase 7: Use all results in final computation */
    int sum = 0;
    sum += result1;
    sum += result2;
    sum += (int)result3;
    sum += (int)result4;
    sum += func_result1;
    sum += (int)func_result3;
    sum += final_result;
    
    /* Use all register variables one more time */
    sum += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9;
    sum += (int)l0 + (int)l1 + (int)l2 + (int)l3 + (int)l4;
    sum += (int)d0 + (int)d1 + (int)d2 + (int)d3 + (int)d4;
    
    /* Complex array access pattern */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            for (int k = 0; k < 5; k++) {
                sum += (int)huge_array[i][j][k];
            }
        }
    }
    
    return sum;
}

/* Main function to run the test */
int main(void) {
    int result = test_reload();
    printf("Result: %d\n", result);
    return 0;
}
