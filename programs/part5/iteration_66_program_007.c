/* reload_test.c - Complex inline assembly to trigger push_reload logic */
#include <stdint.h>
#include <stdlib.h>

/* Helper functions to create complex addressing */
int helper1(int *a, int b) { return *a + b; }
long helper2(long *a, long b, long c) { return *a * b + c; }
double helper3(double *a, double b) { return *a / b; }
void helper4(int *a, int *b, int *c) { *c = *a + *b; }

/* Complex function with register pressure and inline assembly */
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
    register double d5 asm("xmm6") = 6.6;
    register int *p0 asm("rsi") = &r0;
    register int *p1 asm("rdi") = &r1;
    register volatile int v0 asm("ebp") = 42;
    register volatile long v1 asm("esp") = 99;
    
    /* Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Complex computations to create live ranges */
    r0 = r1 * r2 + r3 - r4 / (r5 + 1);
    l0 = l1 * l2 + l3 - (l0 >> 2);
    d0 = d1 * d2 + d3 / d4 - d5;
    
    /* Force spills with many intermediate calculations */
    for (int i = 0; i < 5; i++) {
        r0 = r0 + r1 * r2 - r3;
        l0 = l0 + l1 * l2 - l3;
        d0 = d0 + d1 * d2 - d3;
        r1 = r1 + r2 * r3 - r4;
        l1 = l1 + l2 * l3 - l0;
        d1 = d1 + d2 * d3 - d4;
    }
    
    /* FIRST COMPLEX ASM: Many operands with mixed constraints */
    int result1, result2;
    long result3;
    double result4;
    
    asm volatile (
        /* Output operands with early-clobber */
        "=r" (result1),          /* General reg output */
        "=&r" (result2),         /* Early-clobber general reg */
        "=m" (arr[1][2]),        /* Memory output */
        "=r" (result3),          /* Long output */
        "=f" (result4)           /* FP reg output */
        :
        /* Input operands with complex addressing */
        "r" (r0),                /* Simple register */
        "m" (arr[r0 % 8][r1 % 8]), /* Complex memory addressing */
        "r" (helper1(&r2, r3)),  /* Function call in operand */
        "i" (256),               /* Immediate */
        "r" (l0),                /* Long in general reg */
        "f" (d0),                /* Double in FP reg */
        "m" (arr[2][3]),         /* Another memory operand */
        "r" (r4)                 /* Another register */
        :
        /* Extensive clobber list */
        "memory", "eax", "ebx", "ecx", "edx", "esi", "edi",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
        "cc"
    );
    
    /* Use results to prevent dead code elimination */
    r0 = result1 + result2;
    l0 = result3 + r0;
    d0 = result4 + r0;
    
    /* SECOND ASM: Mismatched modes and array indexing */
    int idx1 = r0 % 8;
    int idx2 = r1 % 8;
    int idx3 = r2 % 8;
    
    asm volatile (
        /* Mixed constraints with potential mode mismatches */
        "=r" (arr[idx1][idx2]),      /* int output */
        "=m" (arr[idx3][idx1]),      /* memory output */
        "+r" (r5),                   /* Input-output */
        "=&r" (r6),                  /* Early-clobber */
        "=f" (d2)                    /* FP output */
        :
        /* Operands with complex expressions */
        "r" (arr[helper1(&idx1, idx2) % 8][idx3]), /* Complex array access */
        "m" (arr[idx2][idx3]),                     /* Memory input */
        "r" ((long)helper2(&l0, l1, l2)),          /* Long from function */
        "f" (helper3(&d0, d1)),                    /* Double from function */
        "i" (128),                                 /* Immediate */
        "r" (r7)                                   /* Register */
        :
        /* Clobbers */
        "memory", "rax", "rbx", "rcx", "rdx",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4",
        "cc"
    );
    
    /* THIRD ASM: Nested function calls in operands */
    int temp1, temp2, temp3;
    
    asm volatile (
        /* Multiple output operands */
        "=r" (temp1),
        "=m" (arr[4][5]),
        "=&r" (temp2),
        "=r" (temp3)
        :
        /* Complex inputs with address-taking */
        "r" (helper4(&r0, &r1, &temp1)),  /* Function with address args */
        "m" (arr[helper1(&r2, r3) % 8][helper1(&r4, r5) % 8]), /* Very complex */
        "r" (&arr[r0 % 8][r1 % 8]),       /* Address of array element */
        "i" (64),                         /* Immediate */
        "r" (r2),                         /* Register */
        "r" (r3),                         /* Another register */
        "r" (r4)                          /* Another register */
        :
        /* Comprehensive clobber list */
        "memory",
        "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
        "cc"
    );
    
    /* Final computations using all results */
    int final_sum = 0;
    final_sum += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
    final_sum += l0 + l1 + l2 + l3;
    final_sum += (int)d0 + (int)d1 + (int)d2 + (int)d3;
    final_sum += result1 + result2 + temp1 + temp2 + temp3;
    
    /* Use array elements */
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
    return result % 256;  /* Prevent overflow issues */
}
