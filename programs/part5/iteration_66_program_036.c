/* Test program to trigger push_reload logic in GCC's reload pass */
#include <stdint.h>
#include <stdlib.h>

/* Helper functions that take addresses */
int helper1(int *a, int *b) {
    return (*a + *b) * 2;
}

long helper2(long *a, long *b, long *c) {
    return *a * *b + *c;
}

double helper3(double *a, double *b) {
    return *a / *b;
}

void helper4(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = arr[i] * 3 + 1;
    }
}

/* Complex inline assembly with mismatched constraints */
static inline void asm_block1(int a, int b, long c, double d, 
                             int *out1, long *out2, double *out3) {
    __asm__ volatile (
        "movl %[a], %%eax\n\t"
        "addl %[b], %%eax\n\t"
        "imull %%eax, %%eax\n\t"
        "movl %%eax, %[o1]\n\t"
        "movq %[c], %%r8\n\t"
        "leaq (%%r8, %%r8, 2), %%r9\n\t"
        "movq %%r9, %[o2]\n\t"
        "movsd %[d], %%xmm0\n\t"
        "addsd %%xmm0, %%xmm0\n\t"
        "movsd %%xmm0, %[o3]\n\t"
        : [o1] "=m" (*out1), 
          [o2] "=&r" (*out2),  /* Early clobber */
          [o3] "=m" (*out3)
        : [a] "r" (a), 
          [b] "r" (b), 
          [c] "r" (c), 
          [d] "x" (d)          /* Mismatch: xmm constraint for double */
        : "eax", "r8", "r9", "xmm0", "memory", "cc"
    );
}

/* Inline assembly with array indexing and function calls */
static inline long asm_block2(int (*arr)[8], int i, int j, 
                             long *ptr1, long *ptr2) {
    long result;
    int temp1, temp2;
    
    /* Complex addressing in operands */
    __asm__ volatile (
        "movl %[arr_val], %%ecx\n\t"
        "addl %[i_val], %%ecx\n\t"
        "imull %[j_val], %%ecx\n\t"
        "movl %%ecx, %[temp1]\n\t"
        "movq %[ptr1], %%r10\n\t"
        "movq (%%r10), %%r11\n\t"
        "addq %%r11, %%rcx\n\t"
        "movq %%rcx, %[result]\n\t"
        : [temp1] "=r" (temp1),
          [result] "=&r" (result)  /* Early clobber */
        : [arr_val] "m" ((*arr)[i*8 + j]),  /* Complex array access */
          [i_val] "r" (i),
          [j_val] "r" (j),
          [ptr1] "r" (ptr1)
        : "ecx", "r10", "r11", "rcx", "memory"
    );
    
    /* Function call within asm operand */
    int func_result = helper1(&temp1, &(*arr)[i*8 + j]);
    
    __asm__ volatile (
        "movl %[func_res], %%edx\n\t"
        "addl %%edx, %%edx\n\t"
        "movslq %%edx, %%rax\n\t"
        "addq %[result], %%rax\n\t"
        "movq %%rax, %[result]\n\t"
        : [result] "+r" (result)
        : [func_res] "r" (func_result)
        : "edx", "rax", "cc"
    );
    
    return result;
}

/* Main test function with high register pressure */
int test_reload(void) {
    /* Declare many register variables to create pressure */
    register int r0 asm("r12") = 1;
    register int r1 asm("r13") = 2;
    register int r2 asm("r14") = 3;
    register int r3 asm("r15") = 4;
    register int r4 asm("ebx") = 5;
    register int r5 = 6;
    register int r6 = 7;
    register int r7 = 8;
    register int r8 = 9;
    register int r9 = 10;
    register long l0 asm("r8") = 100;
    register long l1 asm("r9") = 200;
    register long l2 asm("r10") = 300;
    register long l3 = 400;
    register long l4 = 500;
    register double d0 asm("xmm0") = 1.5;
    register double d1 asm("xmm1") = 2.5;
    register double d2 asm("xmm2") = 3.5;
    register double d3 = 4.5;
    register double d4 = 5.5;
    register int *p0 asm("r11") = &r0;
    register int *p1 = &r1;
    register long *p2 = &l0;
    register double *p3 = &d0;
    
    /* Multi-dimensional array */
    int arr[16][8];
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 8 + j;
        }
    }
    
    /* Complex computations to create live ranges */
    for (int i = 0; i < 100; i++) {
        r0 = r1 + r2 * r3 - r4;
        r1 = r0 ^ r2 | r3 & r4;
        r2 = r1 * r3 / (r4 + 1);
        r3 = r2 << r0 >> r1;
        r4 = r3 % (r0 + 1);
        
        l0 = l1 + l2 * l3 - l4;
        l1 = l0 ^ l2 | l3 & l4;
        l2 = l1 * l3 / (l4 + 1);
        l3 = l2 << 3 >> 2;
        l4 = l3 % (l0 + 1);
        
        d0 = d1 + d2 * d3 - d4;
        d1 = d0 * d2 / d3;
        d2 = d1 - d3 + d4;
        d3 = d2 * 2.0;
        d4 = d3 / 1.5;
    }
    
    /* First complex asm block with mismatched modes */
    int out1;
    long out2;
    double out3;
    
    /* Force different modes: SImode for int, DImode for long, DFmode for double */
    asm_block1(r0, r1, l0, d0, &out1, &out2, &out3);
    
    /* Use results to prevent dead code elimination */
    r5 = out1 + r2;
    l2 = out2 + l1;
    d2 = out3 + d1;
    
    /* Second asm block with array indexing and function calls */
    int idx1 = r0 % 16;
    int idx2 = r1 % 8;
    long asm2_result = asm_block2(arr, idx1, idx2, &l0, &l1);
    
    /* Third asm block with many operands and clobbers */
    long final_result;
    __asm__ volatile (
        /* Multiple input/output operands with various constraints */
        "movl %[a], %%eax\n\t"
        "addl %[b], %%eax\n\t"
        "movl %%eax, %[r1]\n\t"
        "movq %[c], %%r8\n\t"
        "addq %[d], %%r8\n\t"
        "movq %%r8, %[r2]\n\t"
        "movsd %[e], %%xmm0\n\t"
        "mulsd %[f], %%xmm0\n\t"
        "movsd %%xmm0, %[r3]\n\t"
        "leaq (%[g], %[h], 4), %%r9\n\t"
        "movq %%r9, %[r4]\n\t"
        "imull %[i], %[j]\n\t"
        "movl %[j], %[r5]\n\t"
        : [r1] "=m" (r5),      /* Memory output */
          [r2] "=&r" (l3),     /* Early clobber register output */
          [r3] "=m" (d3),      /* Memory output */
          [r4] "=r" (p1),      /* Register output */
          [r5] "+r" (r6)       /* Read-write operand */
        : [a] "r" (r0),        /* Register input */
          [b] "r" (r1),        /* Register input */
          [c] "r" (l0),        /* Register input (DImode) */
          [d] "r" (l1),        /* Register input (DImode) */
          [e] "x" (d0),        /* XMM register input */
          [f] "x" (d1),        /* XMM register input */
          [g] "r" ((long)p0),  /* Pointer in register */
          [h] "r" ((long)p1),  /* Pointer in register */
          [i] "r" (r2),        /* Register input */
          [j] "0" (r6)         /* Matching constraint */
        : "eax", "r8", "r9", "xmm0", "xmm1", "xmm2", 
          "xmm3", "xmm4", "xmm5", "memory", "cc"
    );
    
    /* Function calls with address-taken arguments within asm operands */
    int helper_arg1 = r5 + r6;
    int helper_arg2 = r7 + r8;
    int helper_result;
    
    __asm__ volatile (
        "pushq %%rbx\n\t"
        "movl %[arg1], %%edi\n\t"
        "movl %[arg2], %%esi\n\t"
        "call helper1\n\t"
        "movl %%eax, %[result]\n\t"
        "popq %%rbx\n\t"
        : [result] "=r" (helper_result)
        : [arg1] "r" (helper_arg1),
          [arg2] "r" (helper_arg2)
        : "edi", "esi", "rax", "rbx", "memory"
    );
    
    /* More operations using all variables */
    for (int i = 0; i < 50; i++) {
        r0 = r1 + helper_result;
        r1 = r2 * arr[i%16][i%8];
        r2 = r3 ^ r4;
        r3 = r4 | r5;
        r4 = r5 & r6;
        
        l0 = l1 + asm2_result;
        l1 = l2 * (i + 1);
        l2 = l3 ^ l4;
        
        d0 = d1 * 1.1;
        d1 = d2 + 0.5;
        d2 = d3 / 2.0;
    }
    
    /* Call helper with array address */
    helper4(&arr[0][0], 128);
    
    /* Final computation using all results */
    int final_sum = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9;
    final_sum += (int)l0 + (int)l1 + (int)l2 + (int)l3 + (int)l4;
    final_sum += (int)d0 + (int)d1 + (int)d2 + (int)d3 + (int)d4;
    final_sum += out1 + (int)out2 + (int)out3;
    final_sum += (int)asm2_result + helper_result;
    final_sum += arr[0][0] + arr[15][7];
    
    return final_sum;
}

/* Entry point */
int main(void) {
    int result = test_reload();
    return result % 256;  /* Return non-zero to prevent optimization */
}
