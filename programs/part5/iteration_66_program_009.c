/* reload_test.c - Complex inline assembly to trigger push_reload logic */

#include <stdio.h>
#include <stdint.h>

/* Helper functions that take addresses */
int helper_modify(int *ptr) {
    *ptr = (*ptr * 3) + 7;
    return *ptr;
}

long helper_compute(long *a, long *b) {
    return *a * *b - 42;
}

double helper_double(double *d) {
    return *d * 2.5;
}

/* Complex multi-dimensional array indexing helper */
int* get_array_element(int (*arr)[8][8], int i, int j, int k) {
    return &(*arr)[i][j * k % 8];
}

/* Main test function with high register pressure */
int test_reload(void) {
    /* Phase 1: Declare many register variables to create pressure */
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
    register volatile int v0 asm("ebp") = 999;
    register volatile long v1 asm("esp") = 888;
    
    /* Additional pressure variables */
    register int a0 = 10, a1 = 11, a2 = 12, a3 = 13, a4 = 14;
    register int a5 = 15, a6 = 16, a7 = 17, a8 = 18, a9 = 19;
    register long b0 = 1000, b1 = 1001, b2 = 1002, b3 = 1003;
    register double c0 = 10.1, c1 = 10.2, c2 = 10.3, c3 = 10.4;
    
    /* Phase 2: Multi-dimensional array */
    int arr[8][8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                arr[i][j][k] = i * 64 + j * 8 + k;
            }
        }
    }
    
    /* Phase 3: Complex arithmetic to create live ranges */
    r0 = r1 * r2 + r3 - r4 / (r5 ? r5 : 1);
    l0 = l1 * l2 - l3 + (r0 << 2);
    d0 = d1 * d2 + d3 - d4 / d5;
    
    /* Chain computations to keep variables live */
    for (int i = 0; i < 3; i++) {
        a0 = a1 + a2;
        a1 = a2 * a3;
        a2 = a3 - a4;
        a3 = a4 ^ a5;
        a4 = a5 | a6;
        b0 = b1 + b2;
        b1 = b2 * b3;
        c0 = c1 * c2;
        c1 = c2 + c3;
    }
    
    /* Phase 4: First complex inline asm - Many operands with mixed constraints */
    int result1, result2;
    long result3;
    double result4;
    
    asm volatile (
        /* 10 operands with mixed constraints */
        "mov %[in1], %%eax\n\t"
        "add %[in2], %%eax\n\t"
        "imul %[in3], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        "lea (%[in4], %[in5], 2), %[out2]\n\t"
        "movq %[din1], %%xmm7\n\t"
        "addsd %[din2], %%xmm7\n\t"
        "movq %%xmm7, %[dout]\n\t"
        "mov %[in6], %[out3]\n\t"
        "xor %%ecx, %%ecx\n\t"
        "test %[in7], %[in7]\n\t"
        "cmovnz %[in8], %%ecx\n\t"
        "add %%ecx, %[out1]\n\t"
        : [out1] "=&r" (result1),        /* early-clobber output */
          [out2] "=r" (result2),         /* regular output */
          [out3] "=m" (result3),         /* memory output */
          [dout] "=x" (result4)          /* xmm register output */
        : [in1] "r" (r0),                /* register input */
          [in2] "m" (r1),                /* memory input */
          [in3] "i" (3),                 /* immediate */
          [in4] "r" (l0),                /* mismatched: long in int constraint */
          [in5] "r" (l1),
          [in6] "g" (arr[2][3][4]),      /* complex addressing */
          [din1] "x" (d0),               /* xmm input */
          [din2] "xm" (d1),              /* xmm or memory */
          [in7] "r" (helper_modify(&a0)), /* function call in operand */
          [in8] "r" (helper_compute(&l2, &l3)) /* another function call */
        : "eax", "ecx", "xmm7", "memory", "cc"
    );
    
    /* Use results to prevent dead code elimination */
    r0 = result1 + result2;
    l0 = result3 + r0;
    d0 = result4 * 2.0;
    
    /* Phase 5: Second asm - Mismatched modes and array indexing */
    int idx1 = r0 % 8;
    int idx2 = r1 % 8;
    int idx3 = r2 % 8;
    
    /* Complex array element calculation */
    int *array_ptr = get_array_element(arr, idx1, idx2, idx3);
    
    asm volatile (
        /* Mismatched operand modes */
        "mov %[arr_elem], %%ebx\n\t"
        "add %[int_val], %%ebx\n\t"
        "mov %%ebx, %[out_val]\n\t"
        "movd %[arr_elem2], %%xmm6\n\t"
        "paddd %[vec_val], %%xmm6\n\t"
        "movd %%xmm6, %[out_vec]\n\t"
        /* Force spill/reload with many clobbers */
        "push %%rax\n\t"
        "push %%rbx\n\t"
        "push %%rcx\n\t"
        "push %%rdx\n\t"
        "push %%rsi\n\t"
        "push %%rdi\n\t"
        "push %%r8\n\t"
        "push %%r9\n\t"
        /* Do some work */
        "mov $0x12345678, %%eax\n\t"
        "xor %%ecx, %%ecx\n\t"
        /* Restore */
        "pop %%r9\n\t"
        "pop %%r8\n\t"
        "pop %%rdi\n\t"
        "pop %%rsi\n\t"
        "pop %%rdx\n\t"
        "pop %%rcx\n\t"
        "pop %%rbx\n\t"
        "pop %%rax\n\t"
        : [out_val] "=r" (a0),
          [out_vec] "=r" (a1)
        : [arr_elem] "m" (*array_ptr),      /* memory operand with complex address */
          [int_val] "r" (helper_modify(&r3)), /* function call */
          [arr_elem2] "m" (arr[idx1][idx2 * idx3 % 8][idx1]), /* 3D array indexing */
          [vec_val] "x" (0x00010001)        /* vector immediate */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9",
          "xmm6", "memory", "cc"
    );
    
    /* Phase 6: Third asm - Input-output operands with early clobber */
    int io1 = a0, io2 = a1;
    long io3 = b0;
    
    asm volatile (
        /* Input-output operands */
        "add %[inc], %[io1]\n\t"
        "imul %[io2], %[io1]\n\t"
        "mov %[io1], %[io2]\n\t"
        "shl $4, %[io3]\n\t"
        "add %[mem_in], %[io3]\n\t"
        : [io1] "+&r" (io1),     /* early-clobber input-output */
          [io2] "+r" (io2),      /* regular input-output */
          [io3] "+&m" (io3)      /* memory input-output with early-clobber */
        : [inc] "ri" (10),       /* register or immediate */
          [mem_in] "m" (arr[io1 % 8][io2 % 8][0]) /* complex memory operand */
        : "cc"
    );
    
    /* Phase 7: Final computations using all results */
    int final_sum = 0;
    final_sum += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
    final_sum += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9;
    final_sum += (int)l0 + (int)l1 + (int)l2 + (int)l3;
    final_sum += (int)d0 + (int)d1 + (int)d2 + (int)d3;
    final_sum += result1 + result2 + (int)result3;
    final_sum += io1 + io2 + (int)io3;
    
    /* Use array to prevent optimization */
    for (int i = 0; i < 8; i++) {
        final_sum += arr[i][0][0];
    }
    
    return final_sum;
}

/* Main function to run the test */
int main(void) {
    int result = test_reload();
    printf("Result: %d\n", result);
    return 0;
}
