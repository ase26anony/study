/* Test program to trigger push_reload logic in GCC reload.cc */
#include <stdint.h>
#include <stdlib.h>

/* Helper functions to create complex addressing modes */
static int helper1(int *a, int b) { return *a + b; }
static long helper2(long *a, long b, long c) { return *a * b + c; }
static double helper3(double *a, double b) { return *a / b; }
static void helper4(void *ptr, int val) { *(int*)ptr = val; }

/* Complex inline assembly with register pressure */
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
    register int v0 = 10, v1 = 20, v2 = 30, v3 = 40;
    register long v4 = 50, v5 = 60, v6 = 70, v7 = 80;
    register double v8 = 5.5, v9 = 6.6, v10 = 7.7, v11 = 8.8;
    
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
        r2 = r5 - r0;
        l0 = l1 + l2;
        l1 = l3 * l0;
        d0 = d1 * d2;
        d1 = d3 / d0;
        v0 = v1 + v2;
        v1 = v3 * v0;
        v4 = v5 + v6;
        v5 = v7 * v4;
        v8 = v9 * v10;
        v9 = v11 / v8;
    }
    
    /* First complex asm: Many operands with mixed constraints */
    int result1, result2;
    long result3;
    double result4;
    
    asm volatile (
        /* Output operands with early-clobber */
        "=r" (result1),        /* General reg constraint */
        "=&r" (result2),       /* Early-clobber */
        "=r" (result3),        /* Long in general reg */
        "=x" (result4)         /* FP reg constraint */
        :
        /* Input operands with complex addressing */
        : "r" (arr[r0][r1]),           /* Array element with register indexing */
          "m" (arr[r2][r3]),           /* Memory constraint */
          "r" (helper1(&v0, v1)),      /* Function call in operand */
          "r" (helper2(&v4, v5, v6)),  /* Another function call */
          "i" (12345),                 /* Immediate */
          "X" (v8),                    /* Any register/memory */
          "g" (arr[1][2])              /* General - register or memory */
        : "memory", "eax", "ebx", "ecx", "edx", "esi", "edi",
          "r8", "r9", "r10", "r11", "xmm0", "xmm1", "xmm2", "xmm3",
          "cc"
    );
    
    /* Use results to prevent dead code elimination */
    v0 = result1 + result2;
    v4 = result3;
    v8 = result4;
    
    /* Second asm: Mismatched modes and classes */
    int int_val = 1000;
    long long_val = 2000;
    double double_val = 3000.0;
    int64_t int64_val = 4000;
    float float_val = 5000.0f;
    
    /* Force mismatches between operand modes and constraints */
    asm volatile (
        "mov %[out1], %[in1] \n\t"
        "add %[out2], %[in2] \n\t"
        "cvtsi2sd %[out3], %[in3] \n\t"
        : [out1] "=r" (int_val),      /* SImode output */
          [out2] "=r" (long_val),     /* DImode output */
          [out3] "=x" (double_val)    /* DFmode output */
        : [in1] "r" (arr[v0 % 8][v1 % 8]),  /* Complex address */
          [in2] "m" (arr[v2 % 8][v3 % 8]),  /* Memory operand */
          [in3] "r" (int64_val),            /* DI to DF conversion */
          [in4] "x" (float_val)             /* SFmode input */
        : "memory"
    );
    
    /* Third asm: Input-output operands with complex expressions */
    int io1 = 100, io2 = 200;
    long io3 = 300;
    
    asm volatile (
        "addl %[inc], %[io1] \n\t"
        "imul %[io2], %[io1] \n\t"
        "add %[io3], %[io1] \n\t"
        : [io1] "+r" (io1),           /* Input-output */
          [io2] "+&r" (io2),          /* Early-clobber input-output */
          [io3] "+r" (io3)            /* Another input-output */
        : [inc] "ri" (helper1(&io1, io2)),  /* Function call */
          "m" (arr[io1 % 8][io2 % 8]),      /* Complex memory operand */
          "r" (helper3(&v8, v9))            /* FP function call */
        : "memory", "cc", "eax", "ebx"
    );
    
    /* More computations to use all variables */
    int sum = 0;
    sum += r0 + r1 + r2 + r3 + r4 + r5;
    sum += l0 + l1 + l2 + l3;
    sum += (int)d0 + (int)d1 + (int)d2 + (int)d3;
    sum += v0 + v1 + v2 + v3;
    sum += (int)v4 + (int)v5 + (int)v6 + (int)v7;
    sum += (int)v8 + (int)v9 + (int)v10 + (int)v11;
    sum += result1 + result2 + (int)result3 + (int)result4;
    sum += int_val + (int)long_val + (int)double_val;
    sum += io1 + io2 + (int)io3;
    
    /* Use array elements with complex indexing */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            sum += arr[(i + r0) % 8][(j + r1) % 8];
        }
    }
    
    return sum;
}

/* Main function to call test */
int main(void) {
    int result = test_reload();
    /* Use result to prevent optimization */
    asm volatile ("" : : "r" (result));
    return result % 256;
}
