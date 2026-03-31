/* Test program to trigger push_reload logic in GCC reload.cc */
#include <stdint.h>
#include <stdlib.h>

/* Helper functions to create complex addressing */
static int helper1(int *a, int b) { return *a + b; }
static long helper2(long *a, long *b) { return *a + *b; }
static double helper3(double *a, double b) { return *a * b; }
static void helper4(int *a, int b, int c) { *a = b + c; }
static int helper5(int a, int b, int *c) { *c = a - b; return a * b; }

/* Complex inline assembly with register pressure */
int test_reload(void) {
    /* Phase 1: Create register pressure with many register variables */
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
    register volatile int v0 asm("ebp") = 99;
    register volatile long v1 asm("esp") = 999;
    
    /* Phase 2: Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Phase 3: Arithmetic to create live ranges */
    r0 = r1 + r2 * r3 - r4 / (r5 + 1);
    l0 = l1 * l2 + l3 - (r0 * 2);
    d0 = d1 * d2 + d3 / d4 - d5;
    
    for (int i = 0; i < 5; i++) {
        r0 += arr[i][i] * i;
        l0 += arr[i][i+1] * l1;
        d0 += arr[i][i+2] * d1;
    }
    
    /* ASM BLOCK 1: Many operands with mixed constraints and early-clobber */
    int out1, out2, out3;
    long out4;
    double out5;
    
    asm volatile (
        /* Output operands with various constraints */
        "=r" (out1),          /* General reg constraint */
        "=&r" (out2),         /* Early-clobber general reg */
        "=m" (out3),          /* Memory constraint */
        "=r" (out4),          /* Long in general reg (mismatch potential) */
        "=x" (out5)           /* XMM register */
        :
        /* Input operands with complex expressions */
        "r" (r0 + r1 * 2),    /* Complex expression input */
        "m" (arr[r2][r3]),    /* Multi-dim array with register indices */
        "r" (helper1(&r4, r5)), /* Function call in input */
        "x" (d0),             /* XMM input */
        "r" (l0),             /* Long in general reg */
        "i" (12345)           /* Immediate */
        :
        /* Extensive clobber list */
        "memory", "eax", "ebx", "ecx", "edx", "esi", "edi",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
    );
    
    /* Use results to prevent dead code elimination */
    r0 = out1 + out2;
    l0 = out4 + (out3 * 2);
    d0 = out5 * 2.0;
    
    /* ASM BLOCK 2: Mismatched modes and array indexing with address-taking */
    int idx1 = r0 % 8;
    int idx2 = r1 % 8;
    int idx3 = r2 % 8;
    
    /* Complex addressing in operands */
    asm volatile (
        /* Mixed input/output with '+' constraint */
        "+r" (r0),
        "+r" (r1),
        "=r" (r2),
        "=m" (arr[idx1][idx2]),  /* Array element as output */
        "=x" (d1)
        :
        /* Inputs with address-taking function calls */
        "r" (helper2(&l0, &l1)),  /* Takes addresses */
        "m" (arr[helper1(&idx3, 1)][idx2]), /* Function in array index */
        "r" ((long)helper5(r3, r4, &r5)),   /* Function with side effect */
        "x" (d2),
        "r" (helper3(&d0, d3))    /* Double helper */
        :
        /* Clobbers */
        "memory", "cc", "rax", "rbx", "rcx", "rdx",
        "xmm8", "xmm9", "xmm10", "xmm11"
    );
    
    /* ASM BLOCK 3: Input-output operands with complex constraints */
    int io1 = r0;
    long io2 = l0;
    double io3 = d0;
    
    asm volatile (
        /* Input-output operands */
        "+&r" (io1),           /* Early-clobber input-output */
        "+r" (io2),            /* Regular input-output (long in int reg) */
        "+x" (io3),            /* XMM input-output */
        "=m" (arr[io1 % 8][io2 % 8])  /* Array element based on IO values */
        :
        /* Additional inputs with complex expressions */
        "r" (arr[io1 % 7][io2 % 7] + arr[io1 % 5][io2 % 5]),
        "m" (arr[2][3]),
        "r" (helper4(&r6, r7, io1), r6), /* Function with side effect, comma operator */
        "x" (d3 * d4 + d5)
        :
        /* Full clobber */
        "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
        "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
    );
    
    /* Phase 4: Use all results in final computation */
    int sum = 0;
    sum += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
    sum += (int)(l0 + l1 + l2 + l3);
    sum += (int)(d0 + d1 + d2 + d3 + d4 + d5);
    sum += out1 + out2 + out3 + (int)out4 + (int)out5;
    sum += io1 + (int)io2 + (int)io3;
    
    /* Use array elements */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            sum += arr[i][j];
        }
    }
    
    /* Final arithmetic with all variables */
    sum = sum * 2 - (r0 * r1) / (r2 + 1) + (int)(d0 * 100.0);
    
    return sum;
}

/* Main function to call test */
int main() {
    int result = test_reload();
    /* Use result to prevent optimization */
    asm volatile ("" : : "r"(result));
    return result % 256;
}
