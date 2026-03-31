/* Test program to trigger push_reload logic in reload.cc */
#include <stdint.h>
#include <stdlib.h>

/* Helper functions to create complex addressing */
static int helper1(int *a, int b) { return *a + b; }
static long helper2(long *a, long b, long c) { return *a * b + c; }
static double helper3(double *a, double b) { return *a / b; }
static void helper4(int *a, int *b, int *c) { *c = *a + *b; }

/* Function with high register pressure and complex inline assembly */
int test_reload(void) {
    /* Declare many register variables to create pressure */
    register int r0 asm("eax") = 1;
    register int r1 asm("ebx") = 2;
    register int r2 asm("ecx") = 3;
    register int r3 asm("edx") = 4;
    register int r4 asm("esi") = 5;
    register int r5 asm("edi") = 6;
    register long l0 asm("r8") = 1000L;
    register long l1 asm("r9") = 2000L;
    register long l2 asm("r10") = 3000L;
    register long l3 asm("r11") = 4000L;
    register double d0 asm("xmm0") = 1.0;
    register double d1 asm("xmm1") = 2.0;
    register double d2 asm("xmm2") = 3.0;
    register double d3 asm("xmm3") = 4.0;
    register int *p0 asm("r12") = &r0;
    register int *p1 asm("r13") = &r1;
    register long *p2 asm("r14") = &l0;
    register double *p3 asm("r15") = &d0;
    
    /* Additional variables without explicit registers */
    register int v0 = 10, v1 = 20, v2 = 30, v3 = 40, v4 = 50;
    register long v5 = 100, v6 = 200, v7 = 300, v8 = 400;
    register double v9 = 5.0, v10 = 6.0, v11 = 7.0, v12 = 8.0;
    
    /* Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Perform arithmetic to create live ranges */
    r0 = r1 + r2 * r3 - r4 / (r5 + 1);
    l0 = l1 * l2 + l3 - v5;
    d0 = d1 * d2 + d3 / v9;
    
    for (int i = 0; i < 5; i++) {
        v0 += v1 * v2 - v3 / (v4 + i);
        v5 += v6 * v7 - v8 / (i + 1);
        v9 = v10 * v11 + v12 / (i + 1.0);
    }
    
    /* First complex inline asm with many operands and mismatched modes */
    int result1, result2;
    long result3;
    double result4;
    
    asm volatile (
        /* 10 operands mixing constraints */
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "imull %[in3], %[out2]\n\t"
        "movq %[in4], %%r8\n\t"
        "addq %[in5], %%r8\n\t"
        "movq %%r8, %[out3]\n\t"
        "movsd %[in6], %%xmm0\n\t"
        "mulsd %[in7], %%xmm0\n\t"
        "movsd %%xmm0, %[out4]\n\t"
        : [out1] "=&r" (result1),      /* early-clobber output */
          [out2] "+r" (r2),            /* read-write operand */
          [out3] "=m" (result3),       /* memory output */
          [out4] "=x" (result4)        /* xmm register output */
        : [in1] "r" (arr[r0][r1]),     /* complex array indexing */
          [in2] "rm" (helper1(&v0, v1)), /* function call in operand */
          [in3] "r" (v2),
          [in4] "r" (helper2(&l0, l1, v5)), /* another function call */
          [in5] "m" (v6),
          [in6] "x" (d0),              /* xmm input */
          [in7] "xm" (helper3(&d1, v9)) /* mixed constraint with function */
        : "eax", "r8", "xmm0", "xmm1", "xmm2", "memory", "cc"
    );
    
    /* Use results to prevent dead code elimination */
    v0 = result1 + r2;
    v5 = result3 + 1000;
    v9 = result4 * 2.0;
    
    /* Second asm with different operand mode mismatches */
    int arr_index1 = r3 % 8;
    int arr_index2 = r4 % 8;
    long wide_result;
    int narrow_result;
    
    asm volatile (
        /* Operands with mode/class mismatches */
        "movl %[arr_elem], %%ebx\n\t"
        "movslq %%ebx, %%rax\n\t"      /* sign extend - mode change */
        "addq %[wide_in], %%rax\n\t"
        "movq %%rax, %[wide_out]\n\t"
        "movl %[in_reg], %%ecx\n\t"
        "leal (%%ecx, %[offset], 4), %%edx\n\t"
        "movl %%edx, %[narrow_out]\n\t"
        : [wide_out] "=r" (wide_result),
          [narrow_out] "=&r" (narrow_result)  /* early-clobber */
        : [arr_elem] "rm" (arr[arr_index1][arr_index2]), /* memory/reg */
          [wide_in] "r" (l2),                 /* DImode in 32-bit */
          [in_reg] "r" (v3),                  /* SImode */
          [offset] "i" (10)                   /* immediate */
        : "rax", "rbx", "rcx", "rdx", "memory", "cc"
    );
    
    /* Third asm with input-output operands and complex addressing */
    int io_var = 100;
    long io_wide = 1000;
    
    asm volatile (
        /* Mixed constraints with '&' and '+' */
        "movl %[io1], %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %[io1]\n\t"
        "movq %[io2], %%r8\n\t"
        "subq $100, %%r8\n\t"
        "movq %%r8, %[io2]\n\t"
        "movl %[in1], %%ebx\n\t"
        "addl %%ebx, %[out1]\n\t"
        : [io1] "+&r" (io_var),        /* early-clobber read-write */
          [io2] "+r" (io_wide),        /* read-write */
          [out1] "=m" (arr[0][0])      /* memory output to array */
        : [in1] "rm" (helper4(&v0, &v1, &v2), v2), /* nested calls */
          [in2] "X" (arr[v0 % 8][v1 % 8]) /* complex array operand */
        : "rax", "rbx", "r8", "r9", "r10", "memory", "cc"
    );
    
    /* Final computations using all results */
    int final_sum = r0 + r1 + r2 + r3 + r4 + r5;
    final_sum += v0 + v1 + v2 + v3 + v4;
    final_sum += (int)l0 + (int)l1 + (int)l2 + (int)l3;
    final_sum += (int)v5 + (int)v6 + (int)v7 + (int)v8;
    final_sum += (int)result1 + narrow_result + io_var;
    final_sum += (int)wide_result + (int)io_wide;
    final_sum += (int)d0 + (int)d1 + (int)d2 + (int)d3;
    final_sum += (int)v9 + (int)v10 + (int)v11 + (int)v12;
    final_sum += arr[0][0] + arr[1][1] + arr[2][2];
    
    return final_sum;
}

/* Main function to call test */
int main(void) {
    int result = test_reload();
    /* Use result to prevent optimization */
    asm volatile ("" : : "r" (result));
    return result % 256;
}
