/* Test program to trigger push_reload logic in reload.cc */
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
static void complex_asm1(int a, long b, double c, int *d, long *e) {
    /* Force register pressure with many register variables */
    register int r0 asm("r8") = a;
    register int r1 asm("r9") = a * 2;
    register int r2 asm("r10") = a + 5;
    register int r3 asm("r11") = a - 3;
    register long r4 asm("r12") = b;
    register long r5 asm("r13") = b * 3;
    register long r6 asm("r14") = b + 7;
    register long r7 asm("r15") = b - 2;
    register double f0 asm("xmm0") = c;
    register double f1 asm("xmm1") = c * 2.0;
    register double f2 asm("xmm2") = c + 1.0;
    register double f3 asm("xmm3") = c - 1.0;
    register int *p0 asm("rbx") = d;
    register long *p1 asm("rbp") = e;
    
    /* Additional register variables for more pressure */
    register int r8 = r0 + r1;
    register int r9 = r2 * r3;
    register long r10 = r4 + r5;
    register long r11 = r6 - r7;
    register double f4 = f0 + f1;
    register double f5 = f2 * f3;
    register int r12 = r8 ^ r9;
    register int r13 = r12 << 2;
    register long r14 = r10 | r11;
    register long r15 = r14 >> 3;
    
    /* Complex inline assembly with 8 operands and mismatched modes */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "imull %[in3], %%eax\n\t"
        "movq %[in4], %%rcx\n\t"
        "addq %[in5], %%rcx\n\t"
        "movl %%eax, %[out1]\n\t"
        "movq %%rcx, %[out2]\n\t"
        "movsd %[in6], %%xmm4\n\t"
        "addsd %[in7], %%xmm4\n\t"
        "movsd %%xmm4, %[out3]"
        : [out1] "=r" (r0),           /* Output in general reg */
          [out2] "=&r" (r4),          /* Early-clobber output */
          [out3] "=x" (f0),           /* Output in XMM register */
          "+m" (*d),                  /* Input-output memory */
          "+r" (r1)                   /* Input-output register */
        : [in1] "r" (r2),             /* Input register */
          [in2] "m" (r3),             /* Input memory */
          [in3] "i" (5),              /* Immediate */
          [in4] "r" (r5),             /* Input register */
          [in5] "m" (r6),             /* Input memory */
          [in6] "x" (f1),             /* Input XMM register */
          [in7] "f" (f2),             /* Input floating reg (mismatch!) */
          "r" (r8), "r" (r9), "r" (r10)  /* Extra input registers */
        : "eax", "rcx", "xmm4", "xmm5", "xmm6", "xmm7",
          "r8", "r9", "r10", "r11", "memory", "cc"
    );
    
    /* Use results to prevent optimization */
    *d = r0 + r1;
    *e = r4 + r15;
}

/* Second complex assembly with array indexing */
static int complex_asm2(int arr[][8], int idx1, int idx2) {
    /* More register variables */
    register int i0 asm("eax") = idx1;
    register int i1 asm("ebx") = idx2;
    register int i2 = i0 * 3;
    register int i3 = i1 * 5;
    register int i4 = i2 + i3;
    register int i5 = i4 << 1;
    register int i6 = i5 >> 2;
    register int i7 = i6 ^ 0xFF;
    register int i8 = i7 + 1;
    register int i9 = i8 * 2;
    register int i10 = i9 - 3;
    register int i11 = i10 / 2;
    register int i12 = i11 | 0xAA;
    register int i13 = i12 & 0x55;
    register int i14 = i13 + i0;
    register int i15 = i14 - i1;
    
    /* Complex array indexing in asm operands */
    int result;
    asm volatile (
        "movl (%[arr], %[idx1], 8), %%eax\n\t"      /* arr[idx1][0] */
        "addl 4(%[arr], %[idx2], 8), %%eax\n\t"     /* arr[idx2][1] */
        "imull %[const1], %%eax\n\t"
        "movl %%eax, %[result]\n\t"
        /* Call helper function from asm operand */
        "pushq %%rax\n\t"
        "pushq %[addr1]\n\t"
        "pushq %[addr2]\n\t"
        "call *%[helper]\n\t"
        "addq $24, %%rsp\n\t"
        "addl %%eax, %[result]"
        : [result] "=r" (result),
          "+m" (arr[i0][i1]),        /* Input-output array element */
          "=m" (arr[i2][i3])         /* Output array element */
        : [arr] "r" (arr),
          [idx1] "r" (i0),
          [idx2] "r" (i1),
          [const1] "i" (7),
          [addr1] "r" (&i4),
          [addr2] "r" (&i5),
          [helper] "r" (helper1),
          "r" (i6), "r" (i7), "r" (i8), "r" (i9), "r" (i10)
        : "eax", "ecx", "edx", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "memory", "cc"
    );
    
    return result + i15;
}

/* Main test function */
int test_reload(void) {
    /* Multi-dimensional array */
    int arr[16][8];
    
    /* Initialize array */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 8 + j;
        }
    }
    
    /* Many register variables to create pressure */
    register int v0 asm("r8") = 1;
    register int v1 asm("r9") = 2;
    register int v2 asm("r10") = 3;
    register int v3 asm("r11") = 4;
    register int v4 asm("r12") = 5;
    register int v5 asm("r13") = 6;
    register int v6 asm("r14") = 7;
    register int v7 asm("r15") = 8;
    register long l0 = 1000;
    register long l1 = 2000;
    register long l2 = 3000;
    register long l3 = 4000;
    register double d0 = 1.1;
    register double d1 = 2.2;
    register double d2 = 3.3;
    register double d3 = 4.4;
    register int *p0 = &v0;
    register int *p1 = &v1;
    register long *p2 = &l0;
    register long *p3 = &l1;
    
    /* Arithmetic operations to create live ranges */
    v0 = v1 + v2 * v3 - v4 / (v5 + 1);
    v1 = v0 ^ v2 | v3 & v4;
    v2 = (v1 << 3) + (v0 >> 2);
    v3 = helper1(&v0, &v1);
    v4 = v2 * v3 - v1;
    v5 = helper1(&v2, &v3);
    v6 = v4 + v5 - v0;
    v7 = v6 * 2 + v1;
    
    l0 = l1 + l2 * l3;
    l1 = helper2(&l0, &l1, &l2);
    l2 = l0 - l1 + l3;
    l3 = l2 * 3 / 2;
    
    d0 = d1 * d2 + d3;
    d1 = helper3(&d0, &d1);
    d2 = d0 / d1 - d3;
    d3 = d2 * 2.0 + 1.0;
    
    /* First complex assembly */
    complex_asm1(v0, l0, d0, p0, p2);
    
    /* More computations between asm blocks */
    v0 = arr[v1][v2] + arr[v3][v4];
    v1 = helper1(&arr[v0][v1], &arr[v2][v3]);
    v2 = arr[v4][v5] * arr[v6][v7];
    
    /* Second complex assembly with array indexing */
    int r1 = complex_asm2(arr, v0 & 0xF, v1 & 0x7);
    
    /* Third inline assembly with vector-type mismatch */
    register __int128 vec1 = ((__int128)v0 << 64) | v1;
    register __int128 vec2 = ((__int128)v2 << 64) | v3;
    __int128 vec_result;
    
    /* Assembly with mismatched vector mode */
    asm volatile (
        "movq %[in1], %%rax\n\t"
        "movq %[in1]+8, %%rdx\n\t"
        "addq %[in2], %%rax\n\t"
        "adcq %[in2]+8, %%rdx\n\t"
        "movq %%rax, %[out]\n\t"
        "movq %%rdx, %[out]+8"
        : [out] "=o" (vec_result),      /* Memory output with offset */
          "+r" (v4), "+r" (v5)          /* Input-output registers */
        : [in1] "r" (vec1),             /* Input register (128-bit in 64-bit reg!) */
          [in2] "m" (vec2),             /* Input memory */
          "r" (l0), "r" (l1), "r" (l2), "r" (l3)
        : "rax", "rdx", "rcx", "rbx", "rsi", "rdi",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "memory", "cc"
    );
    
    /* Use all results in final computation */
    int final = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
    final += (int)l0 + (int)l1 + (int)l2 + (int)l3;
    final += (int)d0 + (int)d1 + (int)d2 + (int)d3;
    final += r1;
    final += (int)(vec_result & 0xFFFFFFFF);
    final += arr[final & 0xF][0];
    
    return final;
}

/* Entry point */
int main(void) {
    return test_reload() & 0xFF;
}
