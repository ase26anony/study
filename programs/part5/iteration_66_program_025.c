/* Test program to trigger reload.cc lines 1381-1399 */
#include <stdio.h>
#include <stdint.h>

/* Helper functions to create complex addressing modes */
int helper1(int *a, int *b) { return *a + *b; }
long helper2(long *a, long *b, long *c) { return *a + *b + *c; }
double helper3(double *a, double *b) { return *a * *b; }
void helper4(int *a, int b) { *a += b; }
int helper5(int a, int b, int c, int d, int e) { return a + b + c + d + e; }

/* Complex inline assembly with register pressure */
int test_reload(void) {
    /* 1. Declare many register variables to create pressure */
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
    register int *p0 asm("rsi") = &r0;
    register int *p1 asm("rdi") = &r1;
    register float f0 asm("xmm4") = 5.5f;
    register float f1 asm("xmm5") = 6.6f;
    register int r8 = 9, r9 = 10, r10 = 11, r11 = 12;
    register int r12 = 13, r13 = 14, r14 = 15, r15 = 16;
    
    /* 2. Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* 3. Arithmetic operations to create live ranges */
    r0 = r1 + r2 * r3 - r4 / (r5 + 1);
    l0 = l1 * l2 + l3 - (r0 * 2);
    d0 = d1 * d2 - d3 + (double)r1;
    f0 = f1 * 2.0f + (float)r2;
    
    /* Complex index calculations */
    int idx1 = (r0 + r1 * r2 - r3) % 8;
    int idx2 = (r4 + r5 * r6 - r7) % 8;
    int idx3 = (r8 + r9 * r10 - r11) % 8;
    int idx4 = (r12 + r13 * r14 - r15) % 8;
    
    /* 4. First inline asm: Many operands with mixed constraints */
    int result1, result2;
    long result3;
    double result4;
    
    asm volatile (
        /* Output operands with different constraints */
        "=r" (result1),          /* General register output */
        "=&r" (result2),         /* Early-clobber output */
        "=m" (arr[idx1][idx2]),  /* Memory output */
        "=r" (result3),          /* Long output */
        "=x" (result4)           /* SSE register output */
        :
        /* Input operands with complex addressing */
        : "r" (r0),                     /* Simple register */
          "m" (arr[idx3][idx4]),        /* Memory input */
          "r" (helper1(&r1, &r2)),      /* Function call in operand */
          "i" (10),                     /* Immediate */
          "X" (d0),                     /* Any register */
          "r" (l0),                     /* Long in register */
          "m" (arr[helper1(&idx1, &idx2) % 8][idx3])  /* Complex array indexing */
        : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "cc"
    );
    
    /* 5. Use results to prevent dead code elimination */
    r0 += result1;
    r1 += result2;
    l0 += result3;
    d0 += result4;
    
    /* 6. Second inline asm: Mismatched modes and input-output operands */
    int io1 = 100, io2 = 200;
    long io3 = 300;
    double io4 = 400.0;
    
    asm volatile (
        /* Input-output operands with mismatched constraints */
        "+r" (io1),              /* Input-output in general reg */
        "+&r" (io2),             /* Early-clobber input-output */
        "+m" (arr[io1 % 8][io2 % 8]),  /* Memory input-output */
        "+r" (io3),              /* Long input-output (mismatch potential) */
        "+x" (io4)               /* SSE input-output */
        :
        : "r" (helper2(&l0, &l1, &l2)),  /* Complex function call */
          "m" (arr[helper5(r0, r1, r2, r3, r4) % 8][idx4]),  /* Multi-dim array */
          "i" (20),                      /* Immediate */
          "X" (f0),                      /* Float in any reg */
          "r" (helper3(&d0, &d1))        /* Double function */
        : "memory", "eax", "ebx", "ecx", "edx",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
          "cc"
    );
    
    /* 7. Third inline asm: Very complex with nested addressing */
    int final_result;
    int *ptr1 = &arr[idx1][idx2];
    int *ptr2 = &arr[idx3][idx4];
    
    asm volatile (
        "=r" (final_result)
        : "0" (io1 + io2),               /* Input tied to output */
          "r" (ptr1),                    /* Pointer in register */
          "m" (*ptr2),                   /* Memory dereference */
          "r" (helper4(&io1, io2)),      /* Void function call */
          "X" (io4),                     /* Double in any reg */
          "r" (arr[io1 % 8][helper1(&io2, &r0) % 8]),  /* Complex array element */
          "i" (30),                      /* Immediate */
          "m" (arr[(io1*io2) % 8][(io3*r0) % 8]),  /* More complex indexing */
          "r" (io3 >> 4)                 /* Shifted value */
        : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10",
          "cc"
    );
    
    /* 8. Final computation using all variables */
    int sum = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 +
              r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15 +
              (int)l0 + (int)l1 + (int)l2 + (int)l3 +
              (int)d0 + (int)d1 + (int)d2 + (int)d3 +
              (int)f0 + (int)f1 +
              result1 + result2 + (int)result3 + (int)result4 +
              io1 + io2 + (int)io3 + (int)io4 +
              final_result +
              arr[0][0] + arr[7][7];
    
    return sum;
}

/* Main function to call the test */
int main() {
    int result = test_reload();
    printf("Result: %d\n", result);
    return 0;
}
