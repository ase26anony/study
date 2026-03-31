/* Test program to trigger push_reload logic in GCC's reload pass */
#include <stdint.h>
#include <stdlib.h>

/* Helper functions to force address-taking */
static int helper1(int *a, int *b) { return *a + *b; }
static long helper2(long *a, long *b, long *c) { return *a + *b + *c; }
static double helper3(double *a, double *b) { return *a * *b; }
static void* helper4(void **arr, int i, int j) { 
    return (char*)arr[i] + j * sizeof(int); 
}

/* Complex inline assembly with mismatched constraints */
void test_reload(void) {
    /* High register pressure: many register variables */
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
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Complex computations to create live ranges */
    r0 = r1 * r2 + r3 - r4;
    r1 = r5 ^ r6 | r7 & r9;
    l0 = l1 * l2 / (l3 + 1);
    l1 = l4 - l5 + l6 * 2;
    d0 = d1 * d2 + d3 / d4;
    d1 = d5 - d6 * 0.5;
    
    /* First complex asm: Many operands with mixed constraints */
    asm volatile (
        /* Output operands with early-clobber */
        "mov %[out1], %[in1]\n\t"
        "add %[out2], %[in2]\n\t"
        "imul %[out3], %[in3]\n\t"
        /* Memory operand with complex addressing */
        "mov %[mem1], %%eax\n\t"
        "add %%eax, %[out4]\n\t"
        /* Input-output operand */
        "add $1, %[inout1]\n\t"
        /* Force register pressure */
        "mov %[in4], %%ebx\n\t"
        "mov %[in5], %%ecx\n\t"
        "add %%ebx, %%ecx\n\t"
        "mov %%ecx, %[out5]"
        : [out1] "=&r" (r0),           /* early-clobber output */
          [out2] "=r" (r1),
          [out3] "=r" (r2),
          [out4] "=m" (arr[r3][r4]),   /* memory output */
          [out5] "=r" (r5),
          [inout1] "+r" (r6)           /* input-output */
        : [in1] "r" (r7),
          [in2] "r" (r8),
          [in3] "r" (r9),
          [in4] "r" (helper1(&r0, &r1)), /* function call in operand */
          [in5] "r" (helper2(&l0, &l1, &l2)),
          [mem1] "m" (arr[helper1(&r2, &r3) % 8][helper1(&r4, &r5) % 8])
        : "eax", "ebx", "ecx", "edx", "esi", "edi",
          "r8", "r9", "r10", "r11", "xmm0", "xmm1",
          "xmm2", "xmm3", "memory", "cc"
    );
    
    /* More computations between asm blocks */
    r0 = r1 + r2 * r3 - r4;
    l0 = l1 * l2 + l3 / l4;
    d0 = d1 + d2 * d3 - d4;
    
    /* Second asm: Mismatched modes and array indexing */
    asm volatile (
        /* DImode value in SImode constraint */
        "mov %[in_long], %%rax\n\t"
        "shl $2, %%rax\n\t"
        "mov %%rax, %[out_arr]\n\t"
        /* Array element with complex index calculation */
        "mov %[arr_elem], %%rbx\n\t"
        "add %%rbx, %[out_sum]\n\t"
        /* FP value in integer register */
        "movq %[fp_val], %%rcx\n\t"
        "add %%rcx, %[out_mixed]"
        : [out_arr] "=m" (arr[(r0 + r1) % 8][(r2 * r3) % 8]), /* complex index */
          [out_sum] "=r" (r7),
          [out_mixed] "=r" (r8)
        : [in_long] "r" (l0),                    /* long in 'r' constraint */
          [arr_elem] "r" (arr[helper1(&r3, &r4)][helper1(&r5, &r6)]),
          [fp_val] "r" ((long)d0)                /* double reinterpreted as long */
        : "rax", "rbx", "rcx", "rdx",
          "rsi", "rdi", "memory", "cc"
    );
    
    /* Third asm: Maximum operand count with nested function calls */
    int idx1 = helper1(&r0, &r1);
    int idx2 = helper1(&r2, &r3);
    long idx3 = helper2(&l0, &l1, &l2);
    
    asm volatile (
        "mov %[in1], %%r10\n\t"
        "mov %[in2], %%r11\n\t"
        "add %%r10, %%r11\n\t"
        "mov %%r11, %[out1]\n\t"
        "mov %[in3], %%r10\n\t"
        "sub %[in4], %%r10\n\t"
        "mov %%r10, %[out2]\n\t"
        "mov %[in5], %%xmm0\n\t"
        "addsd %[in6], %%xmm0\n\t"
        "movq %%xmm0, %[out3]\n\t"
        "mov %[in7], %%rax\n\t"
        "imul %[in8], %%rax\n\t"
        "mov %%rax, %[out4]\n\t"
        "lea (%[in9], %[in10], 2), %%rcx\n\t"
        "mov %%rcx, %[out5]"
        : [out1] "=r" (r0),
          [out2] "=r" (r1),
          [out3] "=r" (r2),
          [out4] "=m" (arr[idx1 % 8][idx2 % 8]),
          [out5] "=r" (r3)
        : [in1] "r" (helper1(&r4, &r5)),
          [in2] "r" (helper1(&r6, &r7)),
          [in3] "r" (helper2(&l3, &l4, &l5)),
          [in4] "r" (helper2(&l6, &l0, &l1)),
          [in5] "x" (d0),
          [in6] "x" (d1),
          [in7] "r" ((long)helper3(&d2, &d3)),
          [in8] "r" ((long)helper3(&d4, &d5)),
          [in9] "r" (idx3),
          [in10] "r" (helper1(&r8, &r9))
        : "rax", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13",
          "r14", "r15", "xmm0", "xmm1", "xmm2",
          "xmm3", "xmm4", "xmm5", "memory", "cc"
    );
    
    /* Use results to prevent dead code elimination */
    int sum = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9;
    sum += l0 + l1 + l2 + l3 + l4 + l5 + l6;
    sum += (int)d0 + (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 + (int)d6;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            sum += arr[i][j];
        }
    }
    
    /* Return value depends on all computations */
    asm volatile ("" : : "r"(sum) : "memory");
}

/* Main function to call test */
int main(void) {
    test_reload();
    return 0;
}
