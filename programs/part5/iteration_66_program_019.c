/* Test program to trigger push_reload logic in GCC reload.cc */
#include <stdint.h>
#include <stdlib.h>

/* Helper functions to create complex addressing modes */
static int helper1(int *a, int *b) { return *a + *b; }
static long helper2(long *a, long *b, long *c) { return *a + *b + *c; }
static double helper3(double *a, double *b) { return *a * *b; }
static void* helper4(void **a, int idx) { return a[idx]; }

/* Function with high register pressure and complex inline asm */
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
    register int r6 = 7, r7 = 8, r8 = 9, r9 = 10;
    register long l4 = 500, l5 = 600, l6 = 700;
    register double d4 = 5.0, d5 = 6.0, d6 = 7.0;
    
    /* Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 8 + j;
        }
    }
    
    /* Perform arithmetic to create live ranges */
    r0 = r1 + r2 * r3 - r4 / (r5 + 1);
    l0 = l1 * l2 + l3 - l4;
    d0 = d1 * d2 + d3 / d4 - d5;
    
    /* Complex inline asm #1: Many operands with mixed constraints */
    int result1, result2;
    long result3;
    double result4;
    
    asm volatile (
        /* Output operands with different constraints */
        "=r" (result1),      /* General register output */
        "=&r" (result2),     /* Early-clobber general register */
        "=m" (arr[2][3]),    /* Memory output */
        "=r" (result3),      /* Long in general register */
        "=x" (result4)       /* XMM register output */
        :
        /* Input operands with complex addressing */
        : "r" (r0),          /* Simple register */
          "m" (arr[1][2]),   /* Memory input */
          "r" (helper1(&r1, &r2)),  /* Function call in input */
          "i" (256),         /* Immediate */
          "X" (d0),          /* Any register/memory */
          "g" (l0),          /* General register/memory */
          "rm" (arr[r0][r1]), /* Register or memory with complex index */
          "r" (helper2(&l1, &l2, &l3))  /* Another function call */
        : 
        /* Extensive clobber list */
        "memory", "eax", "ebx", "ecx", "edx", "esi", "edi",
        "r8", "r9", "r10", "r11", "xmm0", "xmm1", "xmm2", "xmm3",
        "cc", "fpsr"
    );
    
    /* More computations between asm blocks */
    r1 = result1 + r6;
    l1 = result3 + l5;
    d1 = result4 * d6;
    
    /* Complex inline asm #2: Mismatched modes and array operands */
    int arr2[4][4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                arr2[i][j][k] = i * 16 + j * 4 + k;
            }
        }
    }
    
    int idx1 = r0 % 4;
    int idx2 = r1 % 4;
    int idx3 = r2 % 4;
    
    /* This asm has mismatched constraints - trying to put 64-bit value in 32-bit reg */
    long long_result;
    double double_result;
    int int_temp;
    
    asm volatile (
        /* Mixed constraints that may cause reloads */
        "=r" (int_temp),           /* int output */
        "=&r" (long_result),       /* long output - potential mode mismatch */
        "+r" (r3),                 /* Input-output operand */
        "=x" (double_result)       /* double output */
        :
        : "m" (arr2[idx1][idx2][idx3]),  /* 3D array element */
          "r" ((long)helper1(&r4, &r5)), /* Function with address-taking */
          "X" (helper3(&d2, &d3)),       /* Double function call */
          "i" (128),
          "g" (arr[r3][r4]),             /* Complex array indexing */
          "r" (p0),                      /* Pointer in register */
          "m" (*p1)                      /* Dereferenced pointer */
        :
        "memory", "rax", "rbx", "rcx", "rdx",
        "xmm4", "xmm5", "xmm6", "xmm7",
        "r12", "r13", "r14", "r15", "cc"
    );
    
    /* Complex inline asm #3: Input-output operands with early clobber */
    int io1 = 100, io2 = 200;
    long io3 = 300;
    
    asm volatile (
        /* Multiple input-output operands */
        "+&r" (io1),      /* Early-clobber input-output */
        "+r" (io2),       /* Regular input-output */
        "+&r" (io3),      /* Early-clobber with different mode */
        "=m" (arr[3][4])  /* Memory output */
        :
        : "r" (helper4((void**)arr, r0)),  /* Complex function call */
          "m" (arr2[1][2][3]),             /* 3D array element */
          "i" (64),
          "g" (arr[io1 % 8][io2 % 8]),     /* Dynamic indexing */
          "r" (&arr[0][0]),                /* Array base address */
          "X" (d4),                        /* Double in any reg/mem */
          "rm" (l6)                        /* Long in reg/mem */
        :
        "memory", "eax", "ebx", "ecx", "edx",
        "rsi", "rdi", "r8", "r9", "r10",
        "xmm8", "xmm9", "xmm10", "xmm11", "cc"
    );
    
    /* Use all results to prevent dead code elimination */
    int final_sum = result1 + result2 + int_temp + io1 + io2;
    final_sum += arr[2][3] + arr[3][4];
    final_sum += (int)long_result + (int)io3;
    final_sum += (int)result3;
    final_sum += (int)result4 + (int)double_result;
    
    /* More register pressure computations */
    for (int i = 0; i < 8; i++) {
        r6 += arr[i][i % 4];
        l4 += arr2[i % 4][(i + 1) % 4][(i + 2) % 4];
        d4 += helper3(&d5, &d6);
    }
    
    return final_sum + r6 + (int)l4 + (int)d4;
}

/* Main function to call test and prevent optimization */
int main(void) {
    int result = test_reload();
    /* Use result to prevent optimization */
    asm volatile ("" : : "r" (result));
    return result % 256;  /* Return non-zero to be useful */
}
