/* Test program to trigger push_reload logic in GCC reload pass */
#include <stdio.h>
#include <stdint.h>

/* Helper functions to force address-taking and complex expressions */
int helper1(int *a, int *b) { return *a + *b; }
long helper2(long *a, long *b, long *c) { return *a * *b + *c; }
double helper3(double *a, double *b) { return *a / *b; }
void helper4(int *a, int *b, int *c) { *c = *a - *b; }
int* helper5(int *arr, int idx) { return &arr[idx]; }

/* Complex inline assembly with mismatched constraints */
void test_reload(void) {
    /* High register pressure: declare many register variables */
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
    register double d0 asm("xmm0") = 1.0;
    register double d1 asm("xmm1") = 2.0;
    register double d2 asm("xmm2") = 3.0;
    register double d3 asm("xmm3") = 4.0;
    register int *p0 asm("r12") = &r0;
    register int *p1 asm("r13") = &r1;
    register int *p2 asm("r14") = &r2;
    register int *p3 asm("r15") = &r3;
    register int r6 = 7, r7 = 8, r8 = 9, r9 = 10;
    register int r10 = 11, r11 = 12, r12 = 13, r13 = 14;
    register int r14 = 15, r15 = 16, r16 = 17, r17 = 18;
    register int r18 = 19, r19 = 20;
    
    /* Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Create live ranges and register pressure */
    for (int i = 0; i < 100; i++) {
        r0 = r1 + r2;
        r1 = r3 * r4;
        r2 = r5 / (r0 + 1);
        l0 = l1 + l2;
        l1 = l3 * l0;
        d0 = d1 + d2;
        d1 = d3 * d0;
        p0 = &r0;
        p1 = &r1;
    }
    
    /* First complex inline asm: Many operands with mixed constraints */
    int result1, result2;
    long result3;
    double result4;
    
    asm volatile (
        /* Output operands with early-clobber */
        "=r" (result1),          /* General reg constraint */
        "=&r" (result2),         /* Early-clobber general reg */
        "=m" (arr[1][2]),        /* Memory constraint */
        "=r" (result3),          /* Long in general reg (mismatch) */
        "=f" (result4)           /* Float reg constraint */
        :
        /* Input operands with complex addressing */
        : "r" (r0),              /* Simple register */
          "m" (arr[r1%8][r2%8]), /* Complex memory addressing */
          "r" (helper1(&r3, &r4)), /* Function call in operand */
          "i" (256),             /* Immediate */
          "r" (l0),              /* Long in general reg */
          "f" (d0),              /* Double in float reg */
          "m" (arr[helper1(&r5, &r6)%4][r7%8]), /* Nested function in address */
          "r" (p0),              /* Pointer */
          "r" (p1)               /* Another pointer */
        : "memory", "eax", "ebx", "ecx", "edx", "esi", "edi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "cc"
    );
    
    /* Use results to prevent dead code elimination */
    r0 = result1 + result2;
    arr[0][0] = result3;
    d0 = result4 * 2.0;
    
    /* Second inline asm: Mismatched modes and input-output operands */
    int io1 = 100, io2 = 200;
    long io3 = 300;
    double io4 = 400.0;
    
    asm volatile (
        /* Input-output operands with mismatched modes */
        "+r" (io1),              /* int in general reg */
        "+&r" (io2),             /* Early-clobber int */
        "+m" (arr[2][3]),        /* Memory operand */
        "+r" (io3),              /* long in general reg - mode mismatch */
        "+f" (io4)               /* double in float reg */
        :
        : "r" (helper2(&l1, &l2, &l3)),  /* Function returning long */
          "m" (arr[io1%8][io2%8]),       /* Complex array indexing */
          "r" ((int)helper3(&d1, &d2)),  /* Double->int conversion */
          "i" (512),                     /* Immediate */
          "r" (p2),                      /* Pointer */
          "r" (p3)                       /* Pointer */
        : "memory", "eax", "ebx", "ecx", "edx",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4",
          "cc"
    );
    
    /* Third inline asm: Complex array operands with nested function calls */
    int idx1 = r0 % 8;
    int idx2 = r1 % 8;
    int idx3 = r2 % 8;
    int idx4 = r3 % 8;
    
    asm volatile (
        "=r" (r0),
        "=m" (arr[idx1][idx2]),
        "=r" (r1),
        "=m" (arr[idx3][idx4]),
        "=r" (r2)
        : 
        : "r" (arr[helper1(&idx1, &idx2)][helper1(&idx3, &idx4)]), /* Double nested */
          "m" (arr[helper5(arr[idx1], idx2) - arr[0]][idx3]),     /* Pointer arithmetic */
          "r" (helper4(&r4, &r5, &r6)),                           /* void function */
          "i" (1024),
          "r" (l0),
          "r" (l1),
          "f" (d0),
          "f" (d1),
          "m" (arr[0][0]),
          "m" (arr[7][7])
        : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
          "cc"
    );
    
    /* Final computation using all results */
    int final_sum = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            final_sum += arr[i][j];
        }
    }
    final_sum += r0 + r1 + r2 + r3 + r4 + r5;
    final_sum += io1 + io2 + (int)io3 + (int)io4;
    final_sum += result1 + result2 + (int)result3 + (int)result4;
    
    /* Use final_sum to prevent optimization */
    volatile int use_result = final_sum;
    (void)use_result;
}

/* Main function to call test */
int main(void) {
    test_reload();
    return 0;
}
