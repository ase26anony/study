/* Test program to trigger push_reload logic in GCC reload.cc */
#include <stdint.h>
#include <stdlib.h>

/* Helper functions to force address-taking */
static int helper1(int *a, int *b) { return *a + *b; }
static long helper2(long *a, long *b, long *c) { return *a * *b + *c; }
static double helper3(double *a, double *b) { return *a / *b; }
static void helper4(int *a, int b) { *a += b; }
static void helper5(long *a, long b, long c) { *a = b - c; }

/* Complex inline assembly with mismatched constraints */
void test_reload(void) {
    /* High register pressure: declare 20+ register variables */
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
    register long l4 asm("r12") = 500;
    register long l5 asm("r13") = 600;
    register double d0 asm("xmm0") = 1.0;
    register double d1 asm("xmm1") = 2.0;
    register double d2 asm("xmm2") = 3.0;
    register double d3 asm("xmm3") = 4.0;
    register double d4 asm("xmm4") = 5.0;
    register double d5 asm("xmm5") = 6.0;
    register int *p0 asm("r14") = &r0;
    register int *p1 asm("r15") = &r1;
    register long *p2 = &l0;
    register double *p3 = &d0;
    
    /* Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Create live ranges and register pressure through computations */
    r0 = r1 + r2 * r3 - r4 / (r5 + 1);
    l0 = l1 * l2 + l3 - l4 / (l5 | 1);
    d0 = d1 * d2 + d3 - d4 / (d5 + 1.0);
    
    for (int i = 0; i < 5; i++) {
        r0 += arr[i][i];
        l0 += arr[i][i+1];
        d0 += arr[i][i+2];
    }
    
    /* First complex inline asm: 8 operands with mixed constraints */
    asm volatile (
        "/* Complex asm 1: mixed constraints */\n\t"
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out1]\n\t"
        "imul %[in3], %[out1]\n\t"
        "mov %[out1], %[out2]\n\t"
        "lea (%[in4],%[in5],4), %[out3]\n\t"
        "mov %[out3], %[out4]\n\t"
        : [out1] "=&r" (r0),        /* early-clobber output */
          [out2] "=m" (arr[2][3]),  /* memory output */
          [out3] "=&r" (r1),        /* early-clobber */
          [out4] "=r" (r2)          /* regular output */
        : [in1] "r" (r3),           /* register input */
          [in2] "m" (arr[1][2]),    /* memory input */
          [in3] "r" (r4),           /* register input */
          [in4] "r" (r5),           /* register input */
          [in5] "r" (arr[3][4])     /* memory->register reload needed */
        : "memory", "cc", "eax", "ebx", "ecx", "edx", "esi", "edi"
    );
    
    /* Use results to prevent dead code elimination */
    r3 = helper1(&r0, &arr[2][3]);
    helper4(&r1, r2);
    
    /* Second inline asm: mismatched modes and classes */
    /* DImode value in SImode constraint, FP value in integer constraint */
    asm volatile (
        "/* Complex asm 2: mismatched modes */\n\t"
        "mov %[in_long], %%rax\n\t"
        "add %[in_int], %%eax\n\t"
        "mov %%rax, %[out_long]\n\t"
        "movq %[in_double], %%xmm6\n\t"
        "cvtsd2si %%xmm6, %[out_int]\n\t"
        : [out_long] "=r" (l0),     /* DImode output */
          [out_int] "=r" (r4)       /* SImode output */
        : [in_long] "r" (l1),       /* DImode input */
          [in_int] "r" (arr[4][5]), /* SImode input (but array element) */
          [in_double] "x" (d0)      /* XMM register constraint */
        : "memory", "cc", "rax", "xmm6"
    );
    
    /* Complex addressing in asm operands with function calls */
    int idx1 = r0 % 8;
    int idx2 = l0 % 8;
    int idx3 = helper1(&r1, &r2) % 8;
    
    /* Third inline asm: 10 operands with nested function calls */
    asm volatile (
        "/* Complex asm 3: many operands with complex addressing */\n\t"
        "mov %[a1], %[t1]\n\t"
        "add %[a2], %[t1]\n\t"
        "mov %[t1], %[o1]\n\t"
        "mov %[a3], %[t2]\n\t"
        "sub %[a4], %[t2]\n\t"
        "mov %[t2], %[o2]\n\t"
        "imul %[a5], %[o3]\n\t"
        "add %[a6], %[o3]\n\t"
        : [o1] "=&r" (r5),
          [o2] "=r" (arr[idx1][idx2]),
          [o3] "=r" (arr[idx2][idx3]),
          [t1] "=&r" (l2),
          [t2] "=&r" (l3)
        : [a1] "r" (helper2(&l0, &l1, &l2)),  /* function call in operand */
          [a2] "m" (arr[helper1(&idx1, &idx2)][idx3]), /* complex addressing */
          [a3] "r" (helper1(p0, p1)),         /* pointer arguments */
          [a4] "r" (arr[5][6]),
          [a5] "r" (r3),
          [a6] "i" (100)                      /* immediate */
        : "memory", "cc", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "xmm0", "xmm1", "xmm2", "xmm3"
    );
    
    /* More computations to use all variables */
    d1 = helper3(&d0, &d2);
    helper5(&l4, l5, l0);
    
    /* Final computation using array elements with complex indexing */
    long final_sum = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            final_sum += arr[i][j];
            final_sum += arr[j][i];  /* transpose access pattern */
        }
    }
    
    final_sum += r0 + r1 + r2 + r3 + r4 + r5;
    final_sum += l0 + l1 + l2 + l3 + l4 + l5;
    final_sum += (long)d0 + (long)d1 + (long)d2 + (long)d3 + (long)d4 + (long)d5;
    
    /* Use final_sum to prevent optimization */
    asm volatile ("" : : "r" (final_sum));
}

/* Additional test with vector modes for more reload scenarios */
void test_vector_reload(void) {
    /* Vector types to trigger different register classes */
    typedef int v4si __attribute__((vector_size(16)));
    typedef double v2df __attribute__((vector_size(16)));
    
    register v4si v0 asm("xmm0");
    register v4si v1 asm("xmm1");
    register v2df v2 asm("xmm2");
    register v2df v3 asm("xmm3");
    
    int arr2d[4][4] = {{1,2,3,4},{5,6,7,8},{9,10,11,12},{13,14,15,16}};
    
    /* Load vector from mismatched memory operand */
    asm volatile (
        "movdqu %[mem], %[vec]\n\t"
        "paddd %[reg], %[vec]\n\t"
        : [vec] "=x" (v0)
        : [mem] "m" (arr2d[1][0]),  /* Memory operand for vector */
          [reg] "x" (v1)            /* XMM register */
        : "memory"
    );
    
    /* Mixed scalar/vector operands */
    double scalar = 2.0;
    asm volatile (
        "movq %[scalar], %[vec]\n\t"
        "addpd %[v2], %[vec]\n\t"
        : [vec] "=x" (v3)
        : [scalar] "r" (scalar),    /* Integer register for double */
          [v2] "x" (v2)             /* XMM register */
        : "memory"
    );
}

int main(void) {
    test_reload();
    test_vector_reload();
    return 0;
}
