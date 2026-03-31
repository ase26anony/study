/* Test program to trigger reload.cc lines 1381-1399 */
#include <stdint.h>
#include <stdlib.h>

/* Helper functions for address-taken arguments */
static int helper1(int *a, int *b) { return *a + *b; }
static long helper2(long *a, long *b, long *c) { return *a + *b + *c; }
static double helper3(double *a, double *b) { return *a * *b; }
static void helper4(int *a, int *b, int *c) { *c = *a - *b; }

/* Function with high register pressure and complex inline assembly */
int test_reload(void) {
    /* Declare many register variables to create register pressure */
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
    register double d0 asm("xmm0") = 1.5;
    register double d1 asm("xmm1") = 2.5;
    register double d2 asm("xmm2") = 3.5;
    register double d3 asm("xmm3") = 4.5;
    register int *p0 asm("r12") = &r0;
    register int *p1 asm("r13") = &r1;
    register long *p2 asm("r14") = &l0;
    register double *p3 asm("r15") = &d0;
    register int r6 = 7, r7 = 8, r8 = 9, r9 = 10;
    register long l4 = 500, l5 = 600, l6 = 700, l7 = 800;
    register double d4 = 5.5, d5 = 6.5, d6 = 7.5, d7 = 8.5;
    
    /* Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Complex arithmetic to create live ranges */
    r0 = r1 + r2 * r3 - r4 / (r5 + 1);
    l0 = l1 * l2 + l3 - l4 / (l5 + 1);
    d0 = d1 * d2 + d3 - d4 / (d5 + 1.0);
    
    /* First complex inline asm with many operands and mismatched modes */
    int result1, result2;
    long result3;
    double result4;
    
    asm volatile (
        /* Mixed constraints with early-clobber */
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        /* Mismatched mode: DImode operation on SImode constraint */
        "movq %[in3], %%r8\n\t"
        "addq %[in4], %%r8\n\t"
        "movq %%r8, %[out2]\n\t"
        /* Memory operand with complex addressing */
        "movl (%[arr_ptr],%[idx1],4), %%ecx\n\t"
        "addl (%[arr_ptr],%[idx2],4), %%ecx\n\t"
        "movl %%ecx, %[out3]\n\t"
        /* FP operation with mismatched constraint */
        "movsd %[in5], %%xmm4\n\t"
        "addsd %[in6], %%xmm4\n\t"
        "movsd %%xmm4, %[out4]\n\t"
        /* Function call within operand */
        "pushq %%rax\n\t"
        "pushq %%rcx\n\t"
        "movq %[addr1], %%rdi\n\t"
        "movq %[addr2], %%rsi\n\t"
        "call *%[func_ptr]\n\t"
        "popq %%rcx\n\t"
        "popq %%rax\n\t"
        : [out1] "=&r" (result1),      /* Early-clobber output */
          [out2] "=r" (result3),       /* Regular output */
          [out3] "=m" (arr[2][3]),     /* Memory output */
          [out4] "=x" (result4)        /* XMM register output */
        : [in1] "r" (r0),              /* Input in register */
          [in2] "m" (r1),              /* Input from memory */
          [in3] "r" (l0),              /* DImode input */
          [in4] "r" (l1),              /* Another DImode input */
          [in5] "x" (d0),              /* XMM input */
          [in6] "x" (d1),              /* Another XMM input */
          [arr_ptr] "r" (arr),         /* Array base pointer */
          [idx1] "r" (r2),             /* Index 1 */
          [idx2] "r" (r3),             /* Index 2 */
          [addr1] "r" (&r4),           /* Address-taken variable */
          [addr2] "r" (&r5),           /* Another address-taken */
          [func_ptr] "r" (helper1)     /* Function pointer */
        : "rax", "rcx", "rdx", "r8", "r9", "r10", "r11",
          "xmm4", "xmm5", "xmm6", "xmm7",
          "cc", "memory"
    );
    
    /* More arithmetic to maintain register pressure */
    r6 = r7 * r8 - r9;
    l4 = l5 + l6 * l7;
    d4 = d5 * d6 - d7;
    
    /* Second inline asm with input-output operands and array indexing */
    int temp1 = 100, temp2 = 200;
    long temp3 = 300;
    
    asm volatile (
        /* Input-output operand with '+' constraint */
        "addl %[inc], %[io1]\n\t"
        /* Complex array indexing in operand */
        "movl (%[arr],%[i],4), %%eax\n\t"
        "imull (%[arr],%[j],4), %%eax\n\t"
        "movl %%eax, %[out5]\n\t"
        /* Mixed size operations */
        "movswl %[in7], %%eax\n\t"
        "addl %[in8], %%eax\n\t"
        "movl %%eax, %[out6]\n\t"
        /* Function call with multiple address arguments */
        "pushq %%rax\n\t"
        "pushq %%rbx\n\t"
        "pushq %%rcx\n\t"
        "movq %[a1], %%rdi\n\t"
        "movq %[a2], %%rsi\n\t"
        "movq %[a3], %%rdx\n\t"
        "call *%[func2]\n\t"
        "popq %%rcx\n\t"
        "popq %%rbx\n\t"
        "popq %%rax\n\t"
        : [io1] "+r" (temp1),          /* Input-output operand */
          [out5] "=r" (result2),       /* Output */
          [out6] "=m" (arr[3][4])      /* Memory output */
        : [inc] "ir" (50),             /* Immediate input */
          [arr] "r" (arr[0]),          /* Array base */
          [i] "r" (r4),                /* Index i */
          [j] "r" (r5),                /* Index j */
          [in7] "r" ((short)temp2),    /* Different mode */
          [in8] "r" (r6),              /* Another input */
          [a1] "r" (&l0),              /* Address argument 1 */
          [a2] "r" (&l1),              /* Address argument 2 */
          [a3] "r" (&l2),              /* Address argument 3 */
          [func2] "r" (helper2)        /* Function pointer */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11",
          "cc", "memory"
    );
    
    /* Third asm with vector-like operations and mismatched classes */
    struct Vec4 { int x, y, z, w; } vec = {1, 2, 3, 4};
    int vec_result[4];
    
    asm volatile (
        /* Attempt to use FP register for integer operation */
        "movd %[vec_x], %%xmm8\n\t"
        "movd %[vec_y], %%xmm9\n\t"
        "paddd %%xmm9, %%xmm8\n\t"
        "movd %%xmm8, %[out7]\n\t"
        /* Complex addressing with multiple indices */
        "movl (%[base],%[idx_a],4), %%eax\n\t"
        "addl (%[base],%[idx_b],4), %%eax\n\t"
        "addl (%[base],%[idx_c],4), %%eax\n\t"
        "movl %%eax, %[out8]\n\t"
        /* Function call in FP context */
        "subq $32, %%rsp\n\t"
        "movsd %[dbl1], (%%rsp)\n\t"
        "movsd %[dbl2], 8(%%rsp)\n\t"
        "movq %%rsp, %%rdi\n\t"
        "leaq 8(%%rsp), %%rsi\n\t"
        "call *%[func3]\n\t"
        "movsd (%%rsp), %%xmm10\n\t"
        "movsd %%xmm10, %[out9]\n\t"
        "addq $32, %%rsp\n\t"
        : [out7] "=r" (vec_result[0]),
          [out8] "=m" (arr[4][5]),
          [out9] "=x" (d0)
        : [vec_x] "r" (vec.x),
          [vec_y] "r" (vec.y),
          [base] "r" (arr[0]),
          [idx_a] "r" (r6),
          [idx_b] "r" (r7),
          [idx_c] "r" (r8),
          [dbl1] "x" (d2),
          [dbl2] "x" (d3),
          [func3] "r" (helper3)
        : "rax", "rcx", "xmm8", "xmm9", "xmm10",
          "cc", "memory"
    );
    
    /* Use results to prevent dead code elimination */
    int final_sum = result1 + result2 + arr[2][3] + arr[3][4] + arr[4][5];
    final_sum += (int)result4 + (int)d0 + vec_result[0] + temp1;
    
    /* More operations with address-taken variables */
    helper4(&r0, &r1, &r2);
    helper4(&r3, &r4, &r5);
    
    /* Final complex asm with all features combined */
    asm volatile (
        "movl %[a], %%eax\n\t"
        "addl %[b], %%eax\n\t"
        "movl %%eax, %[sum]\n\t"
        : [sum] "=rm" (final_sum)
        : [a] "irm" (final_sum),
          [b] "irm" (1000)
        : "eax", "cc"
    );
    
    return final_sum;
}

/* Main function to call test and prevent optimization */
int main(void) {
    int result = test_reload();
    /* Use result to prevent dead code elimination */
    asm volatile ("" : : "r" (result));
    return result % 256;
}
