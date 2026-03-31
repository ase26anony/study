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
__attribute__((noinline))
int test_reload(void) {
    /* Create high register pressure with many register variables */
    register int r0 asm("r8") = 1;
    register int r1 asm("r9") = 2;
    register int r2 asm("r10") = 3;
    register int r3 asm("r11") = 4;
    register int r4 asm("r12") = 5;
    register int r5 asm("r13") = 6;
    register int r6 asm("r14") = 7;
    register int r7 asm("r15") = 8;
    register long l0 asm("rax") = 100;
    register long l1 asm("rbx") = 200;
    register long l2 asm("rcx") = 300;
    register long l3 asm("rdx") = 400;
    register double d0 asm("xmm0") = 1.1;
    register double d1 asm("xmm1") = 2.2;
    register double d2 asm("xmm2") = 3.3;
    register double d3 asm("xmm3") = 4.4;
    register double d4 asm("xmm4") = 5.5;
    register double d5 asm("xmm5") = 6.6;
    register int *p0 asm("rsi") = &r0;
    register int *p1 asm("rdi") = &r1;
    register volatile int v0 asm("ebp") = 50;
    register volatile long v1 asm("rsp") = 500;
    
    /* Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Complex computations to create live ranges */
    for (int i = 0; i < 100; i++) {
        r0 = r0 * r1 + r2 - r3;
        r1 = r1 ^ r4 | r5 & r6;
        r2 = r2 + r7 * (i % 10);
        l0 = l0 + l1 * l2 - l3;
        l1 = l1 ^ l2 | l3;
        d0 = d0 * d1 + d2 / d3;
        d1 = d1 - d4 * d5;
        
        /* Use array elements in computations */
        r3 = arr[r0 % 8][r1 % 8] + r2;
        r4 = arr[r2 % 8][r3 % 8] * r1;
        l2 = arr[r3 % 8][r4 % 8] + l0;
    }
    
    /* First inline asm: Many operands with mixed constraints */
    int out1, out2, out3;
    long out4;
    double out5;
    
    asm volatile (
        /* Complex constraints forcing reloads */
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out1]\n\t"
        "imul %[in3], %[out1]\n\t"
        "mov %[in4], %[out2]\n\t"
        "xor %%eax, %[out2]\n\t"
        "mov %[in5], %[out3]\n\t"
        "lea (%[in6],%[in7],2), %[out4]\n\t"
        "movsd %[in8], %[out5]\n\t"
        "addsd %[in9], %[out5]\n\t"
        "mulsd %[in10], %[out5]"
        : [out1] "=&r" (out1),      /* Early clobber */
          [out2] "=r" (out2),
          [out3] "=m" (out3),       /* Memory output */
          [out4] "=&r" (out4),      /* Early clobber */
          [out5] "=x" (out5)        /* XMM register */
        : [in1] "r" (r0),
          [in2] "m" (arr[2][3]),    /* Memory input */
          [in3] "r" (r1),
          [in4] "r" (r2),
          [in5] "m" (arr[3][4]),    /* Memory input */
          [in6] "r" (l0),
          [in7] "r" (l1),
          [in8] "x" (d0),           /* XMM input */
          [in9] "x" (d1),           /* XMM input */
          [in10] "m" (arr[4][5])    /* Memory input with mismatched mode */
        : "memory", "rax", "rbx", "rcx", "rdx", "r8", "r9", "r10", 
          "r11", "r12", "r13", "r14", "r15", "xmm0", "xmm1", "xmm2",
          "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "cc"
    );
    
    /* Use results to prevent dead code elimination */
    r0 = out1 + out2 + out3;
    l0 = out4 + r0;
    d0 = out5 + d0;
    
    /* Second inline asm: Mismatched modes and function calls in operands */
    int result1, result2;
    double result3;
    
    /* Complex array indexing with function calls */
    int idx1 = helper1(&r0, &r1) % 8;
    int idx2 = helper1(&r2, &r3) % 8;
    
    asm volatile (
        /* Operands with mismatched constraints */
        "mov %[arr_elem], %%eax\n\t"
        "add %[func_res], %%eax\n\t"
        "mov %%eax, %[res1]\n\t"
        "mov %[long_val], %[res2]\n\t"
        "cvtsi2sd %[res2], %[res3]\n\t"
        "addsd %[dbl_val], %[res3]"
        : [res1] "=r" (result1),
          [res2] "=r" (result2),    /* Should be r constraint but gets long */
          [res3] "=x" (result3)     /* XMM output */
        : [arr_elem] "m" (arr[idx1][idx2]),  /* Complex addressing */
          [func_res] "r" (helper2(&l0, &l1, &l2)),  /* Function call in operand */
          [long_val] "r" (helper2(&l1, &l2, &l3)),  /* Another function call */
          [dbl_val] "x" (helper3(&d0, &d1))  /* Double function call */
        : "memory", "rax", "rbx", "rcx", "rdx", "xmm0", "xmm1", "xmm2",
          "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "cc"
    );
    
    /* Third inline asm: Input-output operands with complex constraints */
    int io1 = r4;
    long io2 = l2;
    double io3 = d2;
    
    asm volatile (
        /* Mixed input-output constraints */
        "add $1, %[io1]\n\t"
        "imul %[in1], %[io1]\n\t"
        "sub %[in2], %[io2]\n\t"
        "xor %%rax, %[io2]\n\t"
        "addsd %[in3], %[io3]\n\t"
        "mulsd %[in4], %[io3]"
        : [io1] "+&r" (io1),        /* Early clobber input-output */
          [io2] "+r" (io2),         /* Input-output */
          [io3] "+x" (io3)          /* XMM input-output */
        : [in1] "r" (r5),
          [in2] "m" (arr[helper1(&r6, &r7) % 8][r5 % 8]), /* Complex address */
          [in3] "x" (d3),
          [in4] "m" (arr[r6 % 8][r7 % 8])  /* Memory with potential mode mismatch */
        : "memory", "rax", "xmm0", "xmm1", "xmm2", "xmm3", "cc"
    );
    
    /* Final computation using all results */
    int final_sum = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
    final_sum += out1 + out2 + out3;
    final_sum += result1 + result2;
    final_sum += io1;
    final_sum += (int)l0 + (int)l1 + (int)l2 + (int)l3;
    final_sum += (int)d0 + (int)d1 + (int)d2 + (int)d3;
    final_sum += (int)io3;
    
    /* Use array to prevent optimization */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            final_sum += arr[i][j];
        }
    }
    
    return final_sum;
}

/* Main function to call test */
int main(void) {
    int result = test_reload();
    
    /* Additional calls with different parameters */
    for (int i = 0; i < 10; i++) {
        result += test_reload() % 1000;
    }
    
    return result % 256;
}
