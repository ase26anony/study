/* reload_test.c - Complex inline assembly to trigger reload.cc push_reload logic */

#include <stdint.h>
#include <stdlib.h>

/* Helper functions that take addresses */
static int helper1(int *a, int *b) { return *a + *b; }
static long helper2(long *a, long *b, long *c) { return *a + *b + *c; }
static double helper3(double *a, double *b) { return *a * *b; }
static void helper4(int *arr, int idx, int val) { arr[idx] = val; }

/* Function with high register pressure and complex inline assembly */
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
    register long l4 = 500, l5 = 600, l6 = 700;
    register double d4 = 5.5, d5 = 6.6, d6 = 7.7;
    
    /* Phase 2: Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 8 + j;
        }
    }
    
    /* Phase 3: Arithmetic operations to create live ranges */
    r0 = r1 + r2 * r3 - r4 / (r5 + 1);
    l0 = l1 * l2 + l3 - l4;
    d0 = d1 * d2 + d3 / d4 - d5;
    
    for (int i = 0; i < 5; i++) {
        r6 = r6 * r7 + r8 - r9;
        l4 = l5 + l6 * i;
        d4 = d5 * d6 + i;
    }
    
    /* Phase 4: First complex inline asm with many operands and mismatched modes */
    /* This should trigger reloads due to register constraints and clobbers */
    asm volatile (
        /* Output operands with early-clobber */
        "=r" (r0),          /* General reg constraint */
        "=&r" (r1),         /* Early-clobber general reg */
        "=m" (arr[2][3]),   /* Memory output */
        /* Input-output operands */
        "+r" (r2),          /* Read-write operand */
        "+m" (arr[3][4]),   /* Read-write memory */
        /* Input operands with various constraints */
        "r" (r3),           /* General reg */
        "m" (arr[4][5]),    /* Memory input */
        "r" (l0),           /* 64-bit in 32-bit mode mismatch potential */
        "i" (12345),        /* Immediate */
        "X" (d0)            /* Any register class - mode mismatch possible */
        : /* No outputs listed separately - mixed with inputs above */
        : "eax", "ebx", "ecx", "edx", "esi", "edi",  /* Clobber specific regs */
          "r8", "r9", "r10", "r11", "r12", "r13",    /* More clobbers */
          "xmm0", "xmm1", "xmm2", "xmm3",            /* XMM clobbers */
          "memory", "cc"                              /* Memory and flags */
    );
    
    /* Phase 5: Use helper functions with address-taking in asm operands */
    int helper_result;
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "movl %[in2], %%ebx\n\t"
        "pushl %%eax\n\t"
        "pushl %%ebx\n\t"
        "call *%[func]\n\t"
        "addl $8, %%esp\n\t"
        "movl %%eax, %[out]"
        : [out] "=r" (helper_result)
        : [in1] "r" (&r0),          /* Address of register variable */
          [in2] "r" (&arr[r1][r2]), /* Complex array addressing */
          [func] "r" (helper1)      /* Function pointer */
        : "eax", "ebx", "ecx", "edx", "esi", "edi",
          "memory", "cc"
    );
    
    /* Phase 6: Second asm with array indexing in operands and mode mismatches */
    /* Using DImode values with SImode constraints */
    long array_indexed_result;
    int idx1 = r3 % 8;
    int idx2 = r4 % 8;
    
    asm volatile (
        /* Mixed size operands - potential mode/class mismatches */
        "movq %[arr_elem], %%rax\n\t"
        "addq %[long_val], %%rax\n\t"
        "imulq %[idx_calc], %%rax\n\t"
        "movq %%rax, %[result]"
        : [result] "=r" (array_indexed_result)
        : [arr_elem] "m" (arr[idx1][idx2]),  /* Memory operand with indexing */
          [long_val] "r" (l2),               /* 64-bit in potentially 32-bit reg */
          [idx_calc] "i" (helper2(&l0, &l1, &l3))  /* Function call in constraint */
        : "rax", "rbx", "rcx", "rdx",
          "memory", "cc"
    );
    
    /* Phase 7: Third asm with explicit register variables and complex constraints */
    double fp_result;
    register double dr0 asm("xmm4") = d0;
    register double dr1 asm("xmm5") = d1;
    
    asm volatile (
        /* Multiple output constraints including early-clobber */
        "=x" (dr0),         /* XMM register constraint */
        "=&x" (dr1),        /* Early-clobber XMM */
        "=r" (r5),          /* General register */
        /* Input constraints with mismatched types */
        "x" (d2),           /* XMM input */
        "r" ((int)d3),      /* Double truncated to int - mode mismatch */
        "m" (arr[r6][r7]),  /* Complex array addressing */
        "i" (helper3(&d4, &d5))  /* Function returning double used as immediate */
        : /* Outputs are in input list with =/+ modifiers */
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "rax", "rbx", "rcx", "rdx",
          "memory", "cc"
    );
    
    /* Phase 8: Use results to prevent dead code elimination */
    helper4(arr[0], 0, r0 + r1 + r2 + helper_result);
    
    int final_result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
                      (int)l0 + (int)l1 + (int)l2 + (int)l3 +
                      (int)d0 + (int)d1 + (int)d2 + (int)d3 +
                      helper_result + (int)array_indexed_result +
                      arr[0][0] + arr[7][7];
    
    return final_result;
}

/* Main function to call test and prevent optimization */
int main(void) {
    int result = test_reload();
    /* Use result to prevent optimization */
    asm volatile ("" : : "r" (result));
    return result % 256;
}
