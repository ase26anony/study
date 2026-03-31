/* reload_test.c - Test program to trigger push_reload logic in GCC reload pass */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Helper functions to create complex addressing modes */
static int helper1(int *a, int *b, int *c) {
    return *a + *b + *c;
}

static long helper2(long *a, long *b, int idx) {
    return a[idx] + b[idx * 2];
}

static double helper3(double *arr, int i, int j) {
    return arr[i * 8 + j];
}

/* Function with high register pressure and complex inline assembly */
int test_reload(void) {
    /* Declare many register variables to create register pressure */
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
    
    /* Additional variables without explicit register binding */
    register int v0 = 10, v1 = 20, v2 = 30, v3 = 40, v4 = 50;
    register long v5 = 1000, v6 = 2000, v7 = 3000, v8 = 4000;
    register double v9 = 10.5, v10 = 20.5, v11 = 30.5, v12 = 40.5;
    
    /* Multi-dimensional array with complex indexing */
    int arr[8][8];
    long big_arr[16][4];
    double fp_arr[8][8];
    
    /* Initialize arrays */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 100 + j;
            fp_arr[i][j] = i * 1.5 + j * 0.5;
        }
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 4; j++) {
            big_arr[i][j] = i * 1000L + j * 100L;
        }
    }
    
    /* Create live ranges and register pressure through computations */
    for (int i = 0; i < 100; i++) {
        r0 = r0 * r1 + r2 - r3;
        r1 = r1 * r4 + r5 - r0;
        r2 = r2 * r0 + r1 - r3;
        r3 = r3 * r2 + r4 - r1;
        r4 = r4 * r3 + r5 - r2;
        r5 = r5 * r4 + r0 - r3;
        
        l0 = l0 + l1 * l2 - l3;
        l1 = l1 + l2 * l3 - l0;
        l2 = l2 + l3 * l0 - l1;
        l3 = l3 + l0 * l1 - l2;
        
        d0 = d0 * d1 + d2 - d3;
        d1 = d1 * d2 + d3 - d0;
        d2 = d2 * d3 + d0 - d1;
        d3 = d3 * d0 + d1 - d2;
        
        v0 = v0 + v1 * v2 - v3;
        v1 = v1 + v2 * v3 - v0;
        v2 = v2 + v3 * v0 - v1;
        v3 = v3 + v0 * v1 - v2;
    }
    
    /* COMPLEX INLINE ASSEMBLY BLOCK 1: Many operands with mixed constraints */
    int result1, result2, result3;
    long result4, result5;
    double result6;
    
    /* Force mismatched modes: using SImode variables with DImode constraints */
    asm volatile (
        /* Output operands with early-clobber */
        "=r" (result1),          /* General reg constraint */
        "=&r" (result2),         /* Early-clobber */
        "=m" (arr[2][3]),        /* Memory constraint */
        "=r" (result4),          /* DImode expected but may get SImode */
        "=r" (result5),          /* Another long output */
        "=x" (result6)           /* SSE register */
        :
        /* Input operands with complex addressing */
        : "r" (r0),              /* Input in register */
          "m" (arr[r0 % 8][r1 % 8]),  /* Memory operand with complex index */
          "r" (l0),              /* Long input - potential mode mismatch */
          "i" (12345),           /* Immediate */
          "m" (big_arr[v0 % 16][v1 % 4]), /* Multi-dim array with variable indices */
          "x" (d0),              /* SSE register input */
          "r" (helper1(&v0, &v1, &v2)),  /* Function call in operand */
          "m" (*p0),             /* Pointer dereference */
          "r" (helper2(&l0, &l1, v3 % 8)) /* Another function call */
        : "memory", "eax", "ebx", "ecx", "edx", "esi", "edi",
          "r8", "r9", "r10", "r11", "xmm0", "xmm1", "xmm2", "xmm3",
          "r12", "r13", "r14", "r15", "cc"
    );
    
    /* Use results to prevent dead code elimination */
    v0 = result1 + result2;
    l0 = result4 + result5;
    d0 = result6 * 2.0;
    
    /* COMPLEX INLINE ASSEMBLY BLOCK 2: Input-output operands and array indexing */
    int io1 = 100, io2 = 200;
    long io3 = 3000;
    double io4 = 5.5;
    
    /* Array elements as operands with complex index calculations */
    int idx1 = (r0 + r1) % 8;
    int idx2 = (r2 + r3) % 8;
    int idx3 = (r4 + r5) % 8;
    
    asm volatile (
        /* Input-output operands with '+' constraint */
        "+r" (io1),              /* Input-output */
        "+m" (arr[idx1][idx2]),  /* Memory input-output */
        "+r" (io3),              /* Long input-output - potential mode issue */
        "+x" (io4),              /* SSE input-output */
        /* Output only */
        "=r" (result1),
        "=r" (result2),
        "=m" (fp_arr[idx3][idx1]),
        /* Input only */
        : "r" (v0),
          "m" (big_arr[io1 % 16][io2 % 4]),
          "r" (helper3(&fp_arr[0][0], idx1, idx2)),
          "i" (999),
          "r" (helper1(&arr[idx1][0], &arr[idx2][0], &arr[idx3][0]))
        : "memory", "rax", "rbx", "rcx", "rdx",
          "xmm4", "xmm5", "xmm6", "xmm7", "cc"
    );
    
    /* COMPLEX INLINE ASSEMBLY BLOCK 3: Maximum operand count with clobbers */
    int out1, out2, out3, out4, out5;
    long out6, out7;
    double out8;
    
    /* Create complex index involving multiple variables */
    int complex_idx = (r0 * r1 + r2 * r3 - r4 * r5) % 8;
    
    asm volatile (
        /* 10 operands total */
        "=r" (out1),
        "=&r" (out2),           /* Early-clobber */
        "=m" (arr[complex_idx][(v0+v1)%8]),
        "=r" (out3),
        "=r" (out4),
        "=r" (out5),
        "=r" (out6),            /* Long output */
        "=r" (out7),            /* Another long */
        "=x" (out8),            /* SSE output */
        "=m" (fp_arr[(v2+v3)%8][(v4+v5)%8])
        :
        /* Inputs with various constraints */
        : "r" (io1),
          "m" (big_arr[complex_idx][(v6+v7)%4]),
          "r" (l0),
          "r" (l1),
          "r" (d0),
          "x" (d1),
          "m" (arr[(v8+v9)%8][(v10+v11)%8]),
          "i" (777),
          "r" (helper2(&l2, &l3, complex_idx)),
          "r" (helper1(&out1, &out2, &out3))
        : "memory",
          /* Extensive clobber list */
          "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
          "cc"
    );
    
    /* Final computations using all results */
    int final_sum = 0;
    final_sum += r0 + r1 + r2 + r3 + r4 + r5;
    final_sum += v0 + v1 + v2 + v3 + v4;
    final_sum += result1 + result2 + result3;
    final_sum += io1 + io2;
    final_sum += out1 + out2 + out3 + out4 + out5;
    final_sum += (int)l0 + (int)l1 + (int)l2 + (int)l3;
    final_sum += (int)io3;
    final_sum += (int)out6 + (int)out7;
    final_sum += (int)d0 + (int)d1 + (int)d2 + (int)d3;
    final_sum += (int)io4 + (int)out8;
    
    /* Use array elements */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            final_sum += arr[i][j];
            final_sum += (int)fp_arr[i][j];
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
