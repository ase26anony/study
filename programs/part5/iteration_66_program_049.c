/* Test program to exercise GCC's reload pass */
#include <stdint.h>
#include <stdlib.h>

/* Helper functions to force address-taking */
static int helper1(int *a, int *b) { return *a + *b; }
static long helper2(long *a, long *b, long *c) { return *a + *b + *c; }
static double helper3(double *a, double *b) { return *a * *b; }
static void helper4(int *a, int b) { *a += b; }
static int helper5(int a, int b, int c, int d, int e) { return a + b + c + d + e; }

/* Complex inline assembly with mismatched constraints */
__attribute__((noinline))
int test_reload(void) {
    /* High register pressure: declare 20+ register variables */
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
    register double d0 asm("xmm0") = 1.5;
    register double d1 asm("xmm1") = 2.5;
    register double d2 asm("xmm2") = 3.5;
    register double d3 asm("xmm3") = 4.5;
    register int *p0 asm("r14") = &r0;
    register int *p1 asm("r15") = &r1;
    register int v0 asm("eax") = 0;
    register int v1 asm("ebx") = 0;
    register int v2 asm("ecx") = 0;
    register int v3 asm("edx") = 0;
    
    /* Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 8 + j;
        }
    }
    
    /* Arithmetic operations to create live ranges */
    r0 = r1 + r2 * r3 - r4 / (r5 + 1);
    r1 = r6 ^ r7 | r8 & r9;
    l0 = l1 * l2 + l3 - 1000;
    d0 = d1 * d2 + d3 / 2.0;
    
    /* Complex inline assembly #1: Many operands with mixed constraints */
    asm volatile (
        "/* Complex asm with 8 operands */\n\t"
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out1]\n\t"
        "imul %[in3], %[out2]\n\t"
        "lea (%[in4],%[in5],4), %[out3]\n\t"
        "mov %[out3], %[out4]\n\t"
        "add $1, %[out4]"
        : [out1] "=&r" (v0),      /* Early-clobber output */
          [out2] "=r" (v1),       /* Regular output */
          [out3] "=&r" (v2),      /* Early-clobber */
          [out4] "=m" (arr[1][2]) /* Memory output */
        : [in1] "r" (r0),
          [in2] "r" (r1),
          [in3] "m" (arr[0][1]),  /* Memory input */
          [in4] "r" (r2),
          [in5] "r" (r3),
          "0" (v0)                /* Matching constraint */
        : "memory", "cc", "rax", "rbx", "rcx", "rdx"
    );
    
    /* Use results to prevent dead code elimination */
    r0 = v0 + v1 + v2;
    
    /* Function calls with address-taking in asm operands */
    int temp1, temp2, temp3;
    asm volatile (
        "/* Asm with function calls in operands */\n\t"
        "mov %[call_result], %%eax\n\t"
        "add %%eax, %[out_val]"
        : [out_val] "+r" (r4)
        : [call_result] "i" (helper1(&r0, &r1)),
          [arr_elem] "m" (arr[r2 % 8][r3 % 8]),
          [ptr] "r" (p0)
        : "eax", "memory", "cc"
    );
    
    /* More arithmetic to increase pressure */
    for (int i = 0; i < 4; i++) {
        r0 += arr[i][i];
        l0 += helper2(&l1, &l2, &l3);
        d0 = helper3(&d1, &d2);
    }
    
    /* Complex inline assembly #2: Mismatched modes and array indexing */
    long long_result = 0;
    double double_result = 0.0;
    
    asm volatile (
        "/* Asm with mismatched modes */\n\t"
        "mov %[in_int], %%eax\n\t"
        "cvtsi2sd %%eax, %%xmm0\n\t"
        "movsd %%xmm0, %[out_double]\n\t"
        "mov %[in_long], %%rax\n\t"
        "mov %%rax, %[out_long]"
        : [out_double] "=m" (double_result),  /* Double to memory */
          [out_long] "=r" (long_result)       /* Long to register */
        : [in_int] "r" (arr[r4 % 8][r5 % 8]), /* int from array */
          [in_long] "m" (l0),                 /* long from memory */
          "m" (arr[2][3])                     /* Additional memory input */
        : "rax", "xmm0", "memory", "cc"
    );
    
    /* Complex inline assembly #3: 10 operands with early clobber */
    int results[4];
    asm volatile (
        "/* 10-operand asm */\n\t"
        "mov %[a1], %[o1]\n\t"
        "add %[a2], %[o1]\n\t"
        "mov %[a3], %[o2]\n\t"
        "sub %[a4], %[o2]\n\t"
        "mov %[a5], %[o3]\n\t"
        "imul %[a6], %[o3]\n\t"
        "mov %[a7], %[o4]\n\t"
        "xor %[a8], %[o4]"
        : [o1] "=&r" (results[0]),
          [o2] "=&r" (results[1]),
          [o3] "=&r" (results[2]),
          [o4] "=&r" (results[3])
        : [a1] "r" (r0),
          [a2] "r" (r1),
          [a3] "m" (arr[3][4]),  /* Complex array indexing */
          [a4] "r" (r2),
          [a5] "r" (r3),
          [a6] "m" (arr[r6 % 8][r7 % 8]),  /* Dynamic indexing */
          [a7] "r" (r4),
          [a8] "r" (r5),
          "m" (arr[0][0]),  /* Extra memory operand */
          "m" (arr[7][7])
        : "memory", "cc"
    );
    
    /* Use all results in final computation */
    int final_sum = 0;
    final_sum += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9;
    final_sum += v0 + v1 + v2 + v3;
    final_sum += (int)l0 + (int)l1 + (int)l2 + (int)l3;
    final_sum += (int)d0 + (int)d1 + (int)d2 + (int)d3;
    final_sum += results[0] + results[1] + results[2] + results[3];
    final_sum += (int)long_result;
    final_sum += (int)double_result;
    
    /* Complex array access with function call */
    final_sum += helper5(
        arr[final_sum % 8][0],
        arr[1][final_sum % 8],
        arr[2][3],
        arr[4][5],
        arr[6][7]
    );
    
    return final_sum;
}

/* Main function to call test */
int main(void) {
    int result = test_reload();
    /* Use result to prevent optimization */
    asm volatile ("" : : "r" (result));
    return result % 256;
}
