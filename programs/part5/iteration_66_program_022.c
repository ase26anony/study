/* Test program to trigger push_reload logic in reload.cc */
#include <stdint.h>
#include <stdlib.h>

/* Helper functions that take addresses */
static int helper1(int *a, int *b) { return *a + *b; }
static long helper2(long *a, long *b, long *c) { return *a + *b + *c; }
static double helper3(double *a, double *b) { return *a * *b; }
static void helper4(int *a, int *b, int *c) { *c = *a - *b; }

/* Complex inline assembly test function */
int test_reload(void) {
    /* High register pressure: many register variables */
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
    register double d0 asm("xmm0") = 1.5;
    register double d1 asm("xmm1") = 2.5;
    register double d2 asm("xmm2") = 3.5;
    register double d3 asm("xmm3") = 4.5;
    register int *p0 asm("r12") = &r0;
    register int *p1 asm("r13") = &r1;
    register long *p2 asm("r14") = &l0;
    register double *p3 asm("r15") = &d0;
    register int r6 = 7, r7 = 8, r8 = 9, r9 = 10;
    register int r10 = 11, r11 = 12, r12 = 13, r13 = 14;
    
    /* Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Create live ranges and register pressure */
    r0 = r1 + r2;
    r3 = r4 * r5;
    l0 = l1 - l2;
    l3 = l0 + l1;
    d0 = d1 + d2;
    d3 = d0 * d1;
    
    for (int i = 0; i < 5; i++) {
        r6 += r7;
        r8 -= r9;
        r10 *= r11;
        r12 /= (r13 + 1);
    }
    
    /* First complex inline asm: Many operands with mixed constraints */
    int result1, result2;
    long result3;
    double result4;
    
    asm volatile (
        /* Output operands with early-clobber */
        "=r" (result1),          /* General reg constraint */
        "=&r" (result2),         /* Early-clobber */
        "=m" (arr[1][2]),        /* Memory constraint */
        /* Input-output operands */
        "+r" (r0),               /* Read-write operand */
        "+m" (arr[2][3]),        /* Read-write memory */
        /* Input operands with complex addressing */
        "r" (helper1(&r1, &r2)), /* Function call in operand */
        "m" (arr[r3 % 8][r4 % 8]), /* Complex array indexing */
        "r" (l0),                /* Long in general reg */
        "i" (12345),             /* Immediate */
        "X" (d0)                 /* Any register class */
        : /* No outputs listed separately - mixed with inputs */
        : "memory", "eax", "ebx", "ecx", "edx", "esi", "edi",
          "r8", "r9", "r10", "r11", "xmm0", "xmm1", "xmm2", "xmm3"
    );
    
    /* Use results to prevent dead code elimination */
    r1 = result1 + result2;
    arr[1][2] += r1;
    
    /* Second inline asm: Mismatched modes and register classes */
    /* Using double in integer constraints and vice versa */
    double dtemp;
    int itemp;
    
    asm volatile (
        /* Mismatch: double value with integer register constraint */
        "movq %1, %%rax\n\t"
        "addq $100, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r" (itemp)           /* Integer reg gets double bits */
        : "X" (d1)               /* Double in any reg */
        : "rax", "cc"
    );
    
    /* Third inline asm: Complex array operands with function calls */
    int idx1 = r2 % 8;
    int idx2 = r3 % 8;
    int idx3 = r4 % 8;
    
    asm volatile (
        /* Output to array element with complex index */
        "=m" (arr[helper1(&idx1, &idx2) % 8][idx3]),
        /* Input from array with even more complex index */
        "m" (arr[helper2(&l0, &l1, &l2) % 8][helper1(&r5, &r6) % 8]),
        /* Register operands tied to specific regs */
        "a" (r0),
        "b" (r1),
        "c" (r2),
        "d" (r3),
        "S" (r4),
        "D" (r5),
        /* Input-output with early clobber */
        "+&r" (l0),
        "+&r" (l1),
        /* Memory clobber */
        "memory"
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10"
    );
    
    /* More computations using asm results */
    helper4(&arr[1][2], &arr[2][3], &r0);
    dtemp = helper3(&d0, &d1);
    itemp = helper1(&arr[3][4], &arr[4][5]);
    
    /* Final complex asm with 10 operands */
    int final_result;
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "addl %3, %%eax\n\t"
        "subl %4, %%eax\n\t"
        "imull %5, %%eax\n\t"
        "addl %6, %%eax\n\t"
        "addl %7, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (final_result)
        : "r" (r0), "r" (r1), "r" (r2), "r" (r3),
          "r" (r4), "m" (arr[5][6]), "m" (arr[6][7]),
          "i" (1000), "X" (dtemp)
        : "eax", "cc", "memory"
    );
    
    /* Use all variables to prevent optimization */
    int sum = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
              r10 + r11 + r12 + r13 + itemp + final_result;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            sum += arr[i][j];
        }
    }
    
    sum += (int)l0 + (int)l1 + (int)l2 + (int)l3;
    sum += (int)d0 + (int)d1 + (int)d2 + (int)d3;
    
    return sum;
}

/* Main function to call test */
int main(void) {
    int result = test_reload();
    /* Use result to prevent dead code elimination */
    return result % 256;
}
