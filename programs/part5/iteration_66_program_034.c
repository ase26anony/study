/* Test program to trigger push_reload logic in reload.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper functions that take addresses */
int helper1(int *a, int *b) {
    return (*a + *b) * 2;
}

long helper2(long *a, long *b, long *c) {
    return *a * *b + *c;
}

double helper3(double *a, double *b) {
    return *a / *b + 1.0;
}

void helper4(void *ptr, int size) {
    /* Volatile to prevent optimization */
    volatile char *vptr = (volatile char *)ptr;
    for (int i = 0; i < size; i++) {
        vptr[i] = vptr[i] + 1;
    }
}

/* Complex inline assembly with mismatched constraints */
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
    register double d5 asm("xmm5") = 6.6;
    register int *p0 asm("rsi") = &r0;
    register int *p1 asm("rdi") = &r1;
    register volatile int v0 asm("ebp") = 0;
    register volatile int v1 asm("esp") = 0;
    
    /* Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Create live ranges and register pressure */
    for (int i = 0; i < 100; i++) {
        r0 = r0 + r1 * r2 - r3 / (r4 + 1);
        r1 = r1 ^ r2 | r3 & r4;
        l0 = l0 * l1 + l2 - l3;
        l1 = l1 << 2 | l2 >> 3;
        d0 = d0 * d1 + d2 - d3 / d4;
        d1 = d1 + d2 * d3 - d4;
        
        /* Complex array indexing calculations */
        int idx1 = (r0 + r1) % 8;
        int idx2 = (r2 * r3) % 8;
        int idx3 = (r4 ^ r5) % 8;
        int idx4 = (r6 & r7) % 8;
        
        /* Use array elements in calculations */
        r5 = arr[idx1][idx2] + arr[idx3][idx4];
        r6 = arr[idx2][idx3] * arr[idx4][idx1];
    }
    
    /* FIRST COMPLEX ASM BLOCK: Many operands with mixed constraints */
    int result1, result2, result3;
    long result4;
    double result5;
    
    asm volatile (
        /* Multiple output operands with early-clobber */
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out1]\n\t"
        "imul %[in3], %[out1]\n\t"
        "mov %[in4], %[out2]\n\t"
        "xor %[in5], %[out2]\n\t"
        "mov %[in6], %[out3]\n\t"
        "sub %[in7], %[out3]\n\t"
        "mov %[in8], %[out4]\n\t"
        "shl $3, %[out4]\n\t"
        "movsd %[in9], %[out5]\n\t"
        "addsd %[in10], %[out5]\n\t"
        "mulsd %[in11], %[out5]"
        : [out1] "=&r" (result1),   /* Early-clobber output */
          [out2] "=r" (result2),
          [out3] "=&r" (result3),   /* Early-clobber */
          [out4] "=r" (result4),    /* Long output */
          [out5] "=x" (result5)     /* XMM register output */
        : [in1] "r" (r0),
          [in2] "r" (r1),
          [in3] "r" (r2),
          [in4] "m" (arr[2][3]),    /* Memory constraint */
          [in5] "r" (r3),
          [in6] "r" (r4),
          [in7] "r" (r5),
          [in8] "r" (l0),
          [in9] "x" (d0),           /* XMM input */
          [in10] "x" (d1),
          [in11] "x" (d2)
        : "memory", "rax", "rbx", "rcx", "rdx", "r8", "r9", "r10",
          "r11", "r12", "r13", "r14", "r15", "xmm0", "xmm1", "xmm2",
          "xmm3", "xmm4", "xmm5", "cc"
    );
    
    /* Use results to prevent dead code elimination */
    r0 = result1 + result2;
    l0 = result4 + result3;
    d0 = result5 * 2.0;
    
    /* SECOND ASM BLOCK: Mismatched modes and function calls in operands */
    int array_idx1 = (r0 + r1) & 7;
    int array_idx2 = (r2 + r3) & 7;
    int array_idx3 = (r4 + r5) & 7;
    
    /* Complex addressing with function calls */
    int complex_operand1 = helper1(&arr[array_idx1][array_idx2], 
                                   &arr[array_idx2][array_idx3]);
    long complex_operand2 = helper2(&l0, &l1, &l2);
    double complex_operand3 = helper3(&d0, &d1);
    
    /* Call helper with address of local */
    helper4(&arr[array_idx3][array_idx1], sizeof(int));
    
    int output1, output2;
    double output3;
    
    asm volatile (
        /* Mix of register classes and mismatched sizes */
        "mov %[in1], %%eax\n\t"
        "add %[in2], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        "mov %[in3], %%rbx\n\t"
        "imul %[in4], %%rbx\n\t"
        "mov %%rbx, %[out2]\n\t"
        "movsd %[in5], %%xmm6\n\t"
        "addsd %[in6], %%xmm6\n\t"
        "movsd %%xmm6, %[out3]"
        : [out1] "=r" (output1),      /* Integer output */
          [out2] "=r" (output2),      /* Should be long but constrained as r */
          [out3] "=x" (output3)       /* Double output */
        : [in1] "r" (complex_operand1),
          [in2] "r" (arr[array_idx1][array_idx2]),  /* Array element */
          [in3] "r" (complex_operand2),             /* Long but 'r' constraint */
          [in4] "r" (arr[array_idx3][array_idx1]),  /* Another array element */
          [in5] "x" (complex_operand3),             /* Double input */
          [in6] "x" (d2)                            /* Register double */
        : "memory", "rax", "rbx", "rcx", "rdx", "xmm6", "cc"
    );
    
    /* THIRD ASM BLOCK: Input-output operands and complex constraints */
    int io_var1 = output1;
    long io_var2 = output2;
    double io_var3 = output3;
    
    /* More complex array indexing */
    int idx_a = (io_var1 + r0) % 8;
    int idx_b = (io_var1 * r1) % 8;
    int idx_c = (io_var2 + r2) % 8;
    int idx_d = (io_var2 / (r3 + 1)) % 8;
    
    asm volatile (
        /* Input-output operands with '+' constraint */
        "add %[inc1], %[io1]\n\t"
        "imul %[in1], %[io1]\n\t"
        "sub %[in2], %[io2]\n\t"
        "shl $2, %[io2]\n\t"
        "addsd %[in3], %[io3]\n\t"
        "mulsd %[in4], %[io3]"
        : [io1] "+r" (io_var1),      /* Input-output */
          [io2] "+r" (io_var2),      /* Mismatch: long in 'r' constraint */
          [io3] "+x" (io_var3)       /* Input-output double */
        : [inc1] "r" (arr[idx_a][idx_b]),  /* Complex array addressing */
          [in1] "r" (arr[idx_c][idx_d]),
          [in2] "r" (r4),
          [in3] "x" (d3),
          [in4] "x" (d4)
        : "memory", "rax", "rbx", "xmm7", "cc"
    );
    
    /* Final computation using all results */
    int final_result = 0;
    final_result += result1 + result2 + result3;
    final_result += output1 + output2;
    final_result += io_var1;
    final_result += (int)io_var2;
    final_result += (int)(io_var3 * 100);
    final_result += (int)(result5 * 50);
    
    /* Use all register variables one more time */
    final_result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
    final_result += (int)l0 + (int)l1 + (int)l2 + (int)l3;
    final_result += (int)(d0 + d1 + d2 + d3 + d4 + d5);
    
    /* Use array with complex indexing */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            int idx_i = (i + final_result) % 8;
            int idx_j = (j + final_result) % 8;
            final_result += arr[idx_i][idx_j];
        }
    }
    
    return final_result;
}

/* Main function to run the test */
int main(void) {
    int result = test_reload();
    printf("Result: %d\n", result);
    return 0;
}
