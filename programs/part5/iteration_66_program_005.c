/* reload_test.c - Complex inline assembly to trigger reload.cc uncovered lines */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Helper functions to create complex addressing modes */
static int helper1(int *a, int *b) { return *a + *b; }
static long helper2(long *a, long *b, long *c) { return *a + *b + *c; }
static double helper3(double *a, double *b) { return *a * *b; }
static void helper4(int *a, int *b, int *c) { *c = *a - *b; }

/* Complex test function with high register pressure */
int test_reload(void) {
    /* Phase 1: Declare many register variables to create pressure */
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
    register double d0 asm("xmm0") = 1.1;
    register double d1 asm("xmm1") = 2.2;
    register double d2 asm("xmm2") = 3.3;
    register double d3 asm("xmm3") = 4.4;
    register int *p0 asm("r12") = &r0;
    register int *p1 asm("r13") = &r1;
    register long *p2 asm("r14") = &l0;
    register double *p3 asm("r15") = &d0;
    register int r6 = 7, r7 = 8, r8 = 9, r9 = 10;
    register int r10 = 11, r11 = 12, r12 = 13, r13 = 14;
    
    /* Phase 2: Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Phase 3: Arithmetic operations to create live ranges */
    r0 = r1 + r2 * r3 - r4 / (r5 + 1);
    l0 = l1 * l2 + l3 - (r0 * 2);
    d0 = d1 + d2 * d3 - (l0 / 100.0);
    
    /* Complex index calculations using register variables */
    int idx1 = (r0 + r1) % 8;
    int idx2 = (r2 * r3) % 8;
    int idx3 = (r4 ^ r5) % 8;
    int idx4 = (r6 | r7) % 8;
    
    /* ASM BLOCK 1: Many operands with mixed constraints and early-clobber */
    asm volatile (
        /* Output operands with different constraints */
        "=r" (r0),           /* Simple output */
        "=&r" (r1),          /* Early-clobber output */
        "+r" (r2),           /* Input-output */
        "=m" (arr[idx1][idx2]), /* Memory output */
        "=r" (l0),           /* Long output */
        
        /* Input operands with complex addressing */
        : "r" (r3),          /* Register input */
        "m" (arr[idx3][idx4]), /* Memory input */
        "r" (helper1(&r4, &r5)), /* Function call in input */
        "i" (12345),         /* Immediate */
        "X" (d0)             /* Any register class - may mismatch */
        
        /* Extensive clobber list */
        : "memory", "eax", "ebx", "ecx", "edx", 
          "r8", "r9", "r10", "r11", "r12",
          "xmm0", "xmm1", "xmm2", "xmm3",
          "cc"
        
        /* Assembly template with many instructions */
        : "movl %3, %%eax\n\t"
          "addl %4, %%eax\n\t"
          "movl %%eax, %0\n\t"
          "imull %5, %1\n\t"
          "addl %6, %2\n\t"
          "movq %7, %%r8\n\t"
          "addq %%r8, %4\n\t"
          "movsd %8, %%xmm0\n\t"
          "addsd %%xmm0, %%xmm0"
    );
    
    /* Intermediate computations to maintain register pressure */
    r4 = r0 * r1 + r2 - r3;
    l1 = l0 * 2 + helper2(&l2, &l3, &l0);
    d1 = helper3(&d0, &d2) + d3;
    
    /* ASM BLOCK 2: Mismatched modes and array indexing */
    asm volatile (
        /* DImode output but SImode variable - potential mismatch */
        "=r" (*(long*)&r5),  /* Type punning to force mode issues */
        
        /* Array element with complex index calculation */
        "+m" (arr[(r0 + idx1) % 8][(r1 + idx2) % 8]),
        
        /* Input with memory constraint but register variable */
        : "m" (r6),          /* Mismatch: register var with 'm' constraint */
        
        /* Function call with address-taking in operand */
        "r" (helper4(&r7, &r8, &r9)),
        
        /* Immediate with wrong mode expectation */
        "i" (0xFFFFFFFF),
        
        /* Double input in integer context */
        "r" (*(int*)&d0)     /* Bit-cast double to int */
        
        /* Clobbers */
        : "memory", "rax", "rbx", "rcx", "rdx",
          "rsi", "rdi", "xmm4", "xmm5", "xmm6", "xmm7",
          "cc"
        
        /* Complex template with multiple operations */
        : "movl %2, %%eax\n\t"
          "addl %%eax, %1\n\t"
          "movq %3, %%rbx\n\t"
          "orq %%rbx, %0\n\t"
          "movl %4, %%ecx\n\t"
          "xorl %%ecx, %%eax\n\t"
          "movl %%eax, %1\n\t"
          "movd %5, %%xmm4\n\t"
          "cvtsi2sd %%eax, %%xmm5"
    );
    
    /* ASM BLOCK 3: Many operands with nested function calls */
    int temp1, temp2, temp3;
    asm volatile (
        /* 10 operands mixing all types */
        "=r" (r10),
        "=&r" (r11),
        "+r" (r12),
        "=m" (arr[idx2][idx3]),
        "=r" (l2),
        "+m" (arr[idx4][idx1]),
        "=r" (temp1),
        "=&r" (temp2),
        "+r" (temp3),
        "=r" (*(int*)&d2)  /* Double as integer */
        
        : "r" (helper1(&r0, &r1)),
        "m" (arr[helper1(&idx1, &idx2) % 8][helper1(&idx3, &idx4) % 8]),
        "r" (helper2(&l0, &l1, &l3)),
        "i" (255),
        "r" (r13),
        "X" (d3),
        "m" (r9),
        "r" (helper3(&d0, &d1)),
        "i" (4096),
        "r" (p0)
        
        : "memory", "rax", "rbx", "rcx", "rdx",
          "rsi", "rdi", "r8", "r9", "r10", "r11",
          "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3",
          "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15",
          "cc"
        
        : "movl %10, %%eax\n\t"
          "addl %11, %%eax\n\t"
          "movl %%eax, %0\n\t"
          "movq %12, %%r8\n\t"
          "addq %%r8, %4\n\t"
          "movl %13, %%ebx\n\t"
          "andl %%ebx, %1\n\t"
          "movl %14, %%ecx\n\t"
          "orl %%ecx, %2\n\t"
          "movsd %15, %%xmm0\n\t"
          "mulsd %%xmm0, %%xmm0\n\t"
          "movl %16, %%edx\n\t"
          "subl %%edx, %5\n\t"
          "movsd %17, %%xmm1\n\t"
          "addsd %%xmm1, %%xmm1\n\t"
          "movl %18, %%esi\n\t"
          "shll $2, %%esi\n\t"
          "movq %19, %%rdi\n\t"
          "addq %%rdi, %%r8"
    );
    
    /* Phase 4: Use results in calculations to prevent dead code elimination */
    int sum = 0;
    sum += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9;
    sum += r10 + r11 + r12 + r13;
    sum += (int)l0 + (int)l1 + (int)l2 + (int)l3;
    sum += (int)d0 + (int)d1 + (int)d2 + (int)d3;
    
    /* Sum array elements accessed in asm */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            sum += arr[i][j];
        }
    }
    
    sum += temp1 + temp2 + temp3;
    
    return sum;
}

/* Main function to call test and prevent optimization */
int main(void) {
    int result = test_reload();
    printf("Result: %d\n", result);
    return 0;
}
