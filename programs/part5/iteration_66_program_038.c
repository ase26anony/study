/* Test program to trigger push_reload logic in GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper functions to create complex addressing */
static int helper1(int *a, int b) { return *a + b; }
static long helper2(long *a, long b, long c) { return *a * b + c; }
static double helper3(double *a, double b) { return *a / b; }
static void helper4(int *a, int *b, int *c) { *c = *a + *b; }

/* Function with high register pressure and complex inline asm */
int test_reload(void) {
    /* Declare many register variables to create pressure */
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
    register long *p2 asm("r14") = &l0;
    register double *p3 asm("r15") = &d0;
    
    /* Additional variables without explicit registers */
    register int v0 = 10, v1 = 20, v2 = 30, v3 = 40, v4 = 50;
    register long v5 = 60, v6 = 70, v7 = 80, v8 = 90, v9 = 100;
    register double v10 = 1.1, v11 = 2.2, v12 = 3.3, v13 = 4.4;
    
    /* Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Create live ranges and register pressure through computations */
    for (int i = 0; i < 100; i++) {
        r0 = r0 * r1 + r2 - r3;
        r1 = r1 ^ r4 | r5;
        l0 = l0 + l1 * l2 - l3;
        l1 = l1 & l2 | l3;
        d0 = d0 * d1 + d2 - d3;
        d1 = d1 / d2 * d3;
        v0 = v0 + v1 * v2 - v3;
        v5 = v5 + v6 * v7 - v8;
        v10 = v10 * v11 + v12 - v13;
    }
    
    /* First complex inline asm: Many operands with mixed constraints */
    int result1, result2;
    long result3;
    double result4;
    
    asm volatile (
        /* Output operands with early-clobber */
        "=r" (result1),        /* General reg constraint */
        "=&r" (result2),       /* Early-clobber general reg */
        "=m" (arr[2][3]),      /* Memory output */
        "=r" (result3),        /* Long in general reg (mismatch) */
        "=x" (result4)         /* XMM register (FP) */
        :
        /* Input operands with complex addressing */
        "r" (helper1(&arr[r0 & 7][r1 & 7], v0)),  /* Function call in operand */
        "m" (arr[v2 & 7][v3 & 7]),                /* Memory input */
        "r" (l0),                                 /* Long in general reg */
        "x" (d0),                                 /* Double in XMM */
        "i" (123),                                /* Immediate */
        "r" (v4),                                 /* Simple register */
        "m" (arr[helper1(&v0, 1) & 7][v1 & 7])    /* Complex memory address */
        :
        /* Extensive clobber list */
        "eax", "ebx", "ecx", "edx", "esi", "edi",
        "r8", "r9", "r10", "r11", "r12", "r13",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
        "memory", "cc"
    );
    
    /* Use results to prevent dead code elimination */
    v0 = result1 + result2;
    v5 = result3;
    v10 = result4;
    
    /* Second inline asm: Mismatched modes and array indexing */
    int idx1 = helper1(&r0, r1) & 7;
    int idx2 = helper1(&r2, r3) & 7;
    int idx3 = helper1(&r4, r5) & 7;
    
    asm volatile (
        /* Mixed input/output operands */
        "+r" (v0),              /* Read-write operand */
        "=r" (v1),
        "+m" (arr[idx1][idx2]), /* Read-write memory */
        "=x" (v10),
        "=&r" (v2),             /* Early-clobber */
        "=r" (v3)
        :
        /* Inputs with mismatched modes/classes */
        "r" ((long)v0),         /* Cast creates mode mismatch */
        "m" (arr[idx3][idx1]),  /* Memory with complex index */
        "x" ((float)d0),        /* Float in XMM (mode mismatch) */
        "i" (256),              /* Immediate */
        "r" (helper2(&l0, l1, l2)),  /* Function call */
        "m" (arr[helper1(&idx1, idx2) & 7][helper1(&idx2, idx3) & 7])
        :
        /* Clobbers */
        "rax", "rbx", "rcx", "rdx",
        "xmm0", "xmm1", "xmm2", "xmm3",
        "xmm4", "xmm5", "xmm6", "xmm7",
        "memory"
    );
    
    /* Third inline asm: Input-output with complex constraints */
    int io1 = 1000;
    long io2 = 2000;
    double io3 = 3.14159;
    
    asm volatile (
        /* Complex operand list */
        "+r" (io1),             /* Input-output */
        "=&r" (io2),            /* Early-clobber output */
        "+x" (io3),             /* FP input-output */
        "=m" (arr[4][5]),
        "=r" (v4),
        "=&r" (v5),
        "=x" (v11),
        "=r" (v6)
        :
        /* Mixed constraints */
        "r" (helper3(&d0, d1)),  /* Function returning double */
        "m" (arr[io1 & 7][io2 & 7]),  /* Memory with register indices */
        "r" (helper4(&io1, &v0, &v1)),  /* Void function in operand */
        "i" (4096),
        "x" (d2),
        "r" (v7),
        "m" (arr[helper1(&io1, 2) & 7][helper1(&io2, 3) & 7])
        :
        /* Full clobber */
        "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
        "xmm6", "xmm7", "xmm8", "xmm9", "xmm10",
        "memory", "cc"
    );
    
    /* Use all results in final computation */
    int final_sum = 0;
    final_sum += r0 + r1 + r2 + r3 + r4 + r5;
    final_sum += v0 + v1 + v2 + v3 + v4 + v5 + v6;
    final_sum += (int)l0 + (int)l1 + (int)l2 + (int)l3;
    final_sum += (int)d0 + (int)d1 + (int)d2 + (int)d3;
    final_sum += (int)v10 + (int)v11 + (int)v12 + (int)v13;
    final_sum += io1 + (int)io2 + (int)io3;
    
    /* Use array elements */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            final_sum += arr[i][j];
        }
    }
    
    return final_sum;
}

/* Main function to run the test */
int main(void) {
    int result = test_reload();
    printf("Result: %d\n", result);
    return 0;
}
