/* Test program to trigger push_reload logic in reload.cc */
#include <stdint.h>
#include <stdlib.h>

/* Helper functions to create complex addressing modes */
static int helper1(int *a, int *b) {
    return *a + *b;
}

static long helper2(long *a, long *b, long *c) {
    return *a * *b + *c;
}

static double helper3(double *a, double *b) {
    return *a / *b;
}

/* Function with high register pressure and complex inline assembly */
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
    register int r6 = 7, r7 = 8, r8 = 9, r9 = 10;
    register int r10 = 11, r11 = 12, r12 = 13, r13 = 14;
    register long l4 = 500, l5 = 600, l6 = 700, l7 = 800;
    register double d4 = 5.0, d5 = 6.0, d6 = 7.0, d7 = 8.0;
    
    /* Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Complex arithmetic to create live ranges */
    r0 = r1 + r2 * r3 - r4 / (r5 + 1);
    l0 = l1 * l2 + l3 - l4;
    d0 = d1 * d2 + d3 / d4;
    
    /* First complex inline asm with many operands and mismatched modes */
    asm volatile (
        /* Mixed constraints with early-clobber */
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "imull %[in3], %%eax\n\t"
        "movl %%eax, %[out2]\n\t"
        /* Memory operand with complex addressing */
        "movl (%[arr_ptr],%[idx1],4), %%ebx\n\t"
        "addl %%ebx, %[out3]\n\t"
        /* Force mismatched mode: DImode value in SImode constraint */
        "movq %[long_in], %%rcx\n\t"
        "shrq $32, %%rcx\n\t"
        "movl %%ecx, %[out4]\n\t"
        /* Function call in operand */
        "pushq %%rdx\n\t"
        "pushq %%rcx\n\t"
        "movl %[call_res], %%edi\n\t"
        "call *%[helper]\n\t"
        "popq %%rcx\n\t"
        "popq %%rdx\n\t"
        : [out1] "=&r" (r0),           /* early-clobber output */
          [out2] "+r" (r1),            /* read-write operand */
          [out3] "=m" (arr[r2][r3]),   /* memory output with array indexing */
          [out4] "=r" (r4)             /* regular output */
        : [in1] "r" (r5),              /* input in register */
          [in2] "m" (r6),              /* input in memory */
          [in3] "i" (123),             /* immediate input */
          [arr_ptr] "r" (arr),         /* array base pointer */
          [idx1] "r" (r7 * 8 + r8),    /* complex index calculation */
          [long_in] "r" (l0),          /* DImode in SImode context */
          [call_res] "r" (helper1(&r9, &r10)),  /* function call in operand */
          [helper] "r" ((void*)helper1) /* function pointer */
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "r8", "r9", "r10",
          "r11", "r12", "r13", "r14", "r15", "xmm0", "xmm1", "xmm2", "xmm3",
          "xmm4", "xmm5", "xmm6", "xmm7", "memory", "cc"
    );
    
    /* More arithmetic to maintain register pressure */
    r1 = r0 + r2;
    l1 = l0 * 2 + l2;
    d1 = d0 / 2.0 + d2;
    
    /* Second inline asm with different constraint patterns */
    int temp1, temp2, temp3;
    long temp4;
    double temp5;
    
    asm volatile (
        /* Multiple output operands with mixed constraints */
        "mov %[in_a], %[out_a]\n\t"
        "lea (%[in_b],%[in_c],2), %[out_b]\n\t"
        /* FP register with mismatched constraint */
        "movsd %[in_d], %%xmm4\n\t"
        "addsd %%xmm4, %%xmm4\n\t"
        "movsd %%xmm4, %[out_c]\n\t"
        /* Complex array indexing in output */
        "movl %[in_e], %%eax\n\t"
        "movl %%eax, (%[arr_base],%[idx_i],4,%[idx_j],4)\n\t"
        /* Input-output operand with early clobber */
        "addl $100, %[inout]\n\t"
        : [out_a] "=r" (temp1),
          [out_b] "=r" (temp2),
          [out_c] "=f" (temp5),        /* FP constraint for double */
          [inout] "+&r" (temp3)        /* early-clobber input-output */
        : [in_a] "r" (r11),
          [in_b] "r" (r12),
          [in_c] "r" (r13),
          [in_d] "x" (d3),             /* XMM register constraint */
          [in_e] "r" (helper2(&l3, &l4, &l5)),  /* nested function call */
          [arr_base] "r" (arr),
          [idx_i] "r" (r14 & 7),       /* complex index computation */
          [idx_j] "r" (r15 % 8)
        : "rax", "xmm4", "xmm5", "xmm6", "xmm7", "memory", "cc"
    );
    
    /* Third asm with vector-type mismatches */
    typedef int v4si __attribute__((vector_size(16)));
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3;
    
    /* This should create mode/class mismatches */
    asm volatile (
        "movdqa %[vec1], %%xmm8\n\t"
        "paddd %[vec2], %%xmm8\n\t"
        "movdqa %%xmm8, %[vec3]\n\t"
        /* Mix with scalar operations */
        "movl %[scalar1], %%eax\n\t"
        "movl %%eax, %[scalar_out]\n\t"
        : [vec3] "=x" (v3),            /* XMM constraint for vector */
          [scalar_out] "=r" (temp1)
        : [vec1] "x" (v1),
          [vec2] "m" (v2),             /* Memory constraint for vector */
          [scalar1] "r" (helper3(&d4, &d5))  /* FP function call */
        : "xmm8", "xmm9", "rax", "memory", "cc"
    );
    
    /* Use all results to prevent dead code elimination */
    int sum = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12 + r13;
    sum += temp1 + temp2 + temp3;
    sum += (int)l0 + (int)l1 + (int)l2 + (int)l3;
    sum += (int)d0 + (int)d1 + (int)d2 + (int)d3;
    sum += arr[0][0] + arr[3][3] + arr[7][7];
    sum += v3[0] + v3[1] + v3[2] + v3[3];
    
    return sum;
}

/* Main function to call the test */
int main(void) {
    int result = test_reload();
    /* Use result to prevent optimization */
    return result % 256;
}
