/* Test program to trigger push_reload logic in reload.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Helper functions to force address-taking */
static int helper1(int *a, int *b) { return *a + *b; }
static long helper2(long *a, long *b, long *c) { return *a + *b + *c; }
static double helper3(double *a, double *b) { return *a * *b; }
static void helper4(int *arr, int idx) { arr[idx] = idx * 2; }

/* Complex inline assembly test targeting reload.cc push_reload logic */
int test_reload(void) {
    /* Create high register pressure with many register variables */
    register int r0 asm("eax") = 1;
    register int r1 asm("ebx") = 2;
    register int r2 asm("ecx") = 3;
    register int r3 asm("edx") = 4;
    register int r4 asm("esi") = 5;
    register int r5 asm("edi") = 6;
    register long l0 asm("r8") = 1000L;
    register long l1 asm("r9") = 2000L;
    register long l2 asm("r10") = 3000L;
    register long l3 asm("r11") = 4000L;
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
    register int r14 = 15, r15 = 16, r16 = 17, r17 = 18;
    
    /* Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Create live ranges and register pressure through computations */
    r0 = r1 + r2 * r3 - r4 / (r5 + 1);
    l0 = l1 * l2 + l3 - (r0 * 100);
    d0 = d1 * d2 + d3 / 2.0;
    
    for (int i = 0; i < 4; i++) {
        r6 += arr[i][i] * r7;
        r7 -= arr[i][i+1] / (r8 + 1);
        r8 *= arr[i+1][i] + r9;
        r9 /= arr[i+2][i+2] - r10;
    }
    
    /* First complex inline asm: Many operands with mixed constraints */
    int result1, result2;
    long result3;
    double result4;
    
    asm volatile (
        /* 8 operands with mixed constraints */
        "mov %[in1], %%eax\n\t"
        "add %[in2], %%eax\n\t"
        "imul %[in3], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        "lea (%[in4],%[in5],4), %%ebx\n\t"
        "mov %%ebx, %[out2]\n\t"
        "mov %[in6], %%rcx\n\t"
        "add %[in7], %%rcx\n\t"
        "mov %%rcx, %[out3]\n\t"
        "movsd %[in8], %%xmm4\n\t"
        "addsd %[in9], %%xmm4\n\t"
        "movsd %%xmm4, %[out4]"
        : [out1] "=r" (result1),           /* Output constraint */
          [out2] "=&r" (result2),          /* Early-clobber output */
          [out3] "=r" (result3),           /* Long output */
          [out4] "=x" (result4)            /* XMM register output */
        : [in1] "r" (r0),                  /* Input in register */
          [in2] "m" (arr[1][2]),           /* Input from memory */
          [in3] "ri" (r1),                 /* Register or immediate */
          [in4] "r" (r2),                  /* Register input */
          [in5] "r" (r3),                  /* Register input */
          [in6] "r" (l0),                  /* Long input */
          [in7] "m" (l1),                  /* Long from memory */
          [in8] "x" (d0),                  /* XMM input */
          [in9] "m" (d1)                   /* Double from memory */
        : "eax", "ebx", "rcx", "xmm4", "memory", "cc"
    );
    
    /* Use results to prevent dead code elimination */
    r10 = result1 + result2;
    l2 = result3 + 1000;
    d2 = result4 * 2.0;
    
    /* Second inline asm: Mismatched modes and complex addressing */
    int idx1 = r4 % 8;
    int idx2 = r5 % 8;
    int idx3 = r6 % 8;
    
    /* Call helper function within asm operand */
    int helper_result = helper1(&arr[idx1][idx2], &arr[idx2][idx3]);
    
    asm volatile (
        /* Complex addressing modes with mismatched constraints */
        "mov %[arr_elem1], %%eax\n\t"
        "add %[arr_elem2], %%eax\n\t"
        "sub %[helper_res], %%eax\n\t"
        "imul %[reg_in], %%eax\n\t"
        "mov %%eax, %[out_val1]\n\t"
        /* Force memory operand with register constraint */
        "mov %[mem_in], %%ebx\n\t"
        "add $100, %%ebx\n\t"
        "mov %%ebx, %[out_val2]"
        : [out_val1] "=r" (r11),           /* Output to register variable */
          [out_val2] "=m" (arr[3][4])      /* Output directly to memory */
        : [arr_elem1] "m" (arr[idx1][idx2]),  /* Complex array indexing */
          [arr_elem2] "r" (arr[idx2][idx3]),  /* Mismatch: memory value in reg */
          [helper_res] "i" (helper_result),   /* Immediate from function call */
          [reg_in] "r" (r7),                  /* Register input */
          [mem_in] "r" (arr[4][5])           /* Memory value forced to register */
        : "eax", "ebx", "memory", "cc"
    );
    
    /* Third inline asm: Input-output operands with early clobber */
    int io1 = r12;
    long io2 = l3;
    double io3 = d3;
    
    asm volatile (
        /* Input-output operands with early clobber */
        "add $100, %[io1]\n\t"
        "imul $2, %[io1]\n\t"
        "sub $50, %[io1]\n\t"
        "add %[in10], %[io2]\n\t"
        "sub %[in11], %[io2]\n\t"
        "mulsd %[in12], %[io3]\n\t"
        "addsd %[in13], %[io3]"
        : [io1] "+&r" (io1),               /* Early-clobber input-output */
          [io2] "+r" (io2),                /* Input-output */
          [io3] "+x" (io3)                 /* XMM input-output */
        : [in10] "r" (l0),
          [in11] "m" (l1),
          [in12] "x" (d0),
          [in13] "m" (d1)
        : "cc"
    );
    
    /* More computations using all variables to maintain liveness */
    r12 = io1 + r11;
    l3 = io2 + result3;
    d3 = io3 + result4;
    
    /* Complex array indexing in asm operand with function call */
    int final_idx = (r0 + r1 + r2) % 8;
    helper4(arr[final_idx], final_idx);
    
    asm volatile (
        "mov %[arr_ptr], %%rax\n\t"
        "mov (%[arr_ptr]), %%ebx\n\t"
        "add %[idx_val], %%ebx\n\t"
        "mov %%ebx, %[final_out]"
        : [final_out] "=r" (r13)
        : [arr_ptr] "r" (arr[final_idx]),   /* Array pointer */
          [idx_val] "r" (final_idx)         /* Index value */
        : "rax", "rbx", "memory"
    );
    
    /* Final computation using all results */
    int final_sum = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
                   r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 +
                   (int)l0 + (int)l1 + (int)l2 + (int)l3 +
                   (int)d0 + (int)d1 + (int)d2 + (int)d3 +
                   result1 + result2 + (int)result3 + (int)result4 +
                   io1 + (int)io2 + (int)io3 +
                   arr[0][0] + arr[7][7];
    
    return final_sum;
}

/* Main function to run the test */
int main(void) {
    int result = test_reload();
    printf("Result: %d\n", result);
    return 0;
}
