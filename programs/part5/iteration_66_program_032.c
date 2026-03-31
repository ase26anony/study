/* Test program for GCC reload pass coverage */
#include <stdint.h>
#include <stdlib.h>

/* Helper functions to create complex addressing modes */
static int helper1(int *a, int b) { return *a + b; }
static long helper2(long *a, long b, long c) { return *a * b + c; }
static double helper3(double *a, double b) { return *a / b; }
static void helper4(int *a, int b) { *a = b * 2; }
static int helper5(int a, int b, int c, int d, int e) { 
    return a + b - c * d / e; 
}

/* Main test function with high register pressure */
int test_reload(void) {
    /* Phase 1: Declare many register variables to create pressure */
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
    register int *p0 asm("rsi") = &r0;
    register int *p1 asm("rdi") = &r1;
    register volatile int v0 asm("ebp") = 50;
    register volatile long v1 asm("esp") = 60;
    
    /* Additional variables without explicit registers */
    register int a1 = 10, a2 = 20, a3 = 30, a4 = 40;
    register long b1 = 500, b2 = 600, b3 = 700, b4 = 800;
    register double c1 = 5.5, c2 = 6.6, c3 = 7.7, c4 = 8.8;
    
    /* Phase 2: Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Phase 3: Arithmetic operations to create live ranges */
    r0 = r1 + r2 * r3 - r4 / r5;
    l0 = l1 * l2 + l3 - b1;
    d0 = d1 * d2 - d3 / c1;
    
    for (int i = 0; i < 4; i++) {
        a1 += arr[i][i] * r0;
        b1 += arr[i][i+1] * l0;
        c1 += arr[i][i+2] * d0;
    }
    
    /* Phase 4: First complex inline asm with many operands and mismatches */
    int result1, result2;
    long result3;
    double result4;
    
    asm volatile (
        /* Mixed constraints with early-clobber and mismatched modes */
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out1]\n\t"
        "imul %[in3], %[out1]\n\t"
        "mov %[in4], %[out2]\n\t"
        "lea (%[in5],%[in6],4), %[out2]\n\t"
        "movq %[in7], %[out3]\n\t"
        "addsd %[in8], %[out3]\n\t"
        "mulsd %[in9], %[out3]\n\t"
        "mov %[in10], %[out4]\n\t"
        : [out1] "=&r" (result1),      /* Early-clobber general reg */
          [out2] "+&r" (result2),      /* Early-clobber input-output */
          [out3] "=r" (result3),       /* General reg, but used for double */
          [out4] "=m" (result4)        /* Memory output */
        : [in1] "r" (r0),              /* Input in general reg */
          [in2] "m" (arr[1][2]),       /* Memory input */
          [in3] "r" (helper1(&r1, r2)), /* Function call in operand */
          [in4] "r" (a1),
          [in5] "r" (l0),
          [in6] "r" (l1),
          [in7] "x" (d0),              /* SSE register for double */
          [in8] "xm" (d1),             /* SSE reg or memory */
          [in9] "x" (helper3(&c1, 2.0)), /* Function call returning double */
          [in10] "x" (d2)              /* SSE reg constraint */
        : "memory", "eax", "ebx", "ecx", "edx", "r8", "r9", "r10", 
          "r11", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "cc"
    );
    
    /* Use results to prevent dead code elimination */
    r0 = result1 + result2;
    d0 = result4 + c1;
    
    /* Phase 5: Second asm with array indexing and complex addressing */
    int idx1 = r0 % 8;
    int idx2 = r1 % 8;
    int idx3 = r2 % 8;
    
    asm volatile (
        /* Complex array addressing with mismatched operand sizes */
        "movl %[arr1], %%eax\n\t"
        "addl %[arr2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movq %[arr3], %%xmm0\n\t"
        "cvtsi2sd %[arr4], %%xmm1\n\t"
        "addsd %%xmm0, %%xmm1\n\t"
        "movsd %%xmm1, %[out2]\n\t"
        "lea (%[base],%[idx1],8), %%rcx\n\t"
        "add (%[base],%[idx2],8), %[out3]\n\t"
        : [out1] "=r" (a2),
          [out2] "=m" (c2),
          [out3] "+r" (a3)
        : [arr1] "m" (arr[idx1][idx2]),      /* 2D array element */
          [arr2] "m" (arr[idx2][idx3]),      /* Another 2D element */
          [arr3] "m" (arr[idx3][idx1]),      /* Different indices */
          [arr4] "r" (arr[helper1(&idx1, 1) % 8][helper1(&idx2, 2) % 8]), /* Function in index */
          [base] "r" (arr[0]),               /* Base pointer */
          [idx1] "r" (idx1 * sizeof(int)),   /* Scaled index */
          [idx2] "r" (idx2 * sizeof(int))    /* Another scaled index */
        : "memory", "rax", "rcx", "xmm0", "xmm1", "xmm2", "cc"
    );
    
    /* Phase 6: Third asm with input-output operands and clobbers */
    int io1 = r3;
    long io2 = l2;
    double io3 = d2;
    
    asm volatile (
        /* Multiple input-output operands with different constraints */
        "add %[inc1], %[io1]\n\t"
        "imul %[inc2], %[io2]\n\t"
        "addsd %[inc3], %[io3]\n\t"
        "mov %[addr1], %%r8\n\t"
        "add (%%r8), %[io1]\n\t"
        "mov %[addr2], %%r9\n\t"
        "mov (%%r9), %%r10\n\t"
        "add %%r10, %[io2]\n\t"
        : [io1] "+r" (io1),
          [io2] "+&r" (io2),      /* Early-clobber for long */
          [io3] "+x" (io3)        /* SSE register for double */
        : [inc1] "rm" (r4),       /* Register or memory */
          [inc2] "rm" (helper2(&l3, b2, b3)), /* Complex function call */
          [inc3] "xm" (helper3(&d3, c3)),     /* Another function call */
          [addr1] "r" (&arr[idx1][idx2]),     /* Address of array element */
          [addr2] "r" (&arr[idx2][idx3])      /* Another address */
        : "memory", "r8", "r9", "r10", "r11", "xmm4", "xmm5", "xmm6", "cc"
    );
    
    /* Phase 7: More computations using asm results */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            arr[i][j] += io1 + (int)io2 + (int)io3;
        }
    }
    
    /* Final computation and return */
    int final_sum = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            final_sum += arr[i][j];
        }
    }
    
    final_sum += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
    final_sum += (int)l0 + (int)l1 + (int)l2 + (int)l3;
    final_sum += (int)d0 + (int)d1 + (int)d2 + (int)d3;
    final_sum += a1 + a2 + a3 + a4;
    final_sum += (int)b1 + (int)b2 + (int)b3 + (int)b4;
    final_sum += (int)c1 + (int)c2 + (int)c3 + (int)c4;
    final_sum += result1 + result2 + (int)result3 + (int)result4;
    final_sum += io1 + (int)io2 + (int)io3;
    
    return final_sum;
}

/* Wrapper to prevent optimization */
int main(void) {
    return test_reload() % 1000;
}
