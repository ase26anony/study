/* reload_test.c - Complex inline assembly to trigger reload.cc push_reload logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper functions that take addresses - for requirement #5 */
int helper_modify_int(int *ptr, int delta) {
    *ptr += delta;
    return *ptr;
}

long helper_modify_long(long *ptr, long delta) {
    *ptr ^= delta;
    return *ptr;
}

double helper_modify_double(double *ptr, double factor) {
    *ptr *= factor;
    return *ptr;
}

void helper_complex_op(int *a, long *b, double *c) {
    *a = (*a * 3) / 2;
    *b = (*b << 2) | 0x7F;
    *c = (*c + 1.0) / 2.0;
}

/* Main test function with high register pressure */
int test_reload(void) {
    /* Requirement #3: Many local register variables */
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
    register int t0 = 10, t1 = 11, t2 = 12, t3 = 13;
    register int t4 = 14, t5 = 15, t6 = 16, t7 = 17;
    register int t8 = 18, t9 = 19;
    
    /* Requirement #2: Multi-dimensional array */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Requirement #3: Arithmetic operations to create live ranges */
    r0 = r1 * r2 + r3 - r4;
    r1 = r0 ^ r5 | r2;
    l0 = l1 * l2 / l3;
    l1 = l0 << 3 | l2 >> 2;
    d0 = d1 * d2 + d3;
    d1 = d0 / d2 - d3;
    
    for (int i = 0; i < 5; i++) {
        t0 = t1 + t2;
        t1 = t2 * t3;
        t2 = t3 - t4;
        t3 = t4 ^ t5;
        t4 = t5 | t6;
        t5 = t6 & t7;
        t6 = t7 + t8;
        t7 = t8 * t9;
    }
    
    /* ASM Block 1: Many operands with mixed constraints - Requirement #1, #4 */
    /* Mismatched modes: using SImode values with DImode constraints */
    asm volatile (
        /* Output operands with early-clobber */
        "=r" (r0),         /* simple output */
        "=&r" (r1),        /* early-clobber output */
        "+r" (r2),         /* read-write operand */
        "=m" (arr[1][2]),  /* memory output */
        /* Input operands */
        : "r" (r3),        /* register input */
          "m" (arr[2][3]), /* memory input */
          "r" ((long)r4),  /* mismatched mode: int in long constraint */
          "i" (256),       /* immediate */
        /* Clobbers - Requirement #4 */
        : "memory", "eax", "ebx", "ecx", "edx", "esi", "edi",
          "r8", "r9", "r10", "r11", "r12", "r13",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
    );
    
    /* Use results to prevent dead code elimination */
    r3 = r0 + r1 + r2;
    arr[0][0] = arr[1][2] + arr[2][3];
    
    /* ASM Block 2: Complex addressing with function calls - Requirement #5, #6 */
    /* Using array elements with complex indexing */
    int idx1 = r0 % 8;
    int idx2 = r1 % 8;
    int idx3 = r2 % 8;
    
    asm volatile (
        /* Output with memory constraint */
        "=m" (arr[idx1][idx2]),
        "=r" (l0),
        "=&r" (l1),
        /* Input-output */
        "+r" (l2),
        /* Inputs with complex expressions */
        : "r" (arr[idx2][idx3]),  /* array element as input */
          "r" (helper_modify_int(&arr[idx3][idx1], r3)),  /* function call in operand */
          "m" (arr[(idx1 + idx2) % 8][(idx3 * 2) % 8]),  /* complex array indexing */
          "r" ((int64_t)d0),  /* mismatched: double in integer constraint */
        /* Extensive clobbers */
        : "memory", "rax", "rbx", "rcx", "rdx", 
          "rsi", "rdi", "r8", "r9", "r10", "r11",
          "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4",
          "xmm5", "xmm6", "xmm7", "xmm8", "xmm9"
    );
    
    /* ASM Block 3: Maximum operand count with mixed types - Requirement #1 */
    /* 10 operands mixing all constraint types */
    register int out1 asm("eax");
    register long out2 asm("rbx");
    register double out3;
    int in1 = 100;
    long in2 = 200;
    double in3 = 3.14;
    
    asm volatile (
        "=r" (out1),
        "=&r" (out2),
        "=r" (r4),
        "=m" (arr[7][7]),
        "+r" (r5),
        "+m" (arr[0][7]),
        : "r" (in1),
          "m" (arr[4][4]),
          "r" (helper_modify_long(&l3, in2)),  /* function call */
          "r" ((int)in3),  /* mismatched mode */
          "i" (4096),
          "X" (arr[idx1][idx2] + arr[idx2][idx3])  /* complex address expression */
        : "memory", "cc",
          "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11"
    );
    
    /* Use all variables in final computation to prevent optimization */
    int sum = r0 + r1 + r2 + r3 + r4 + r5;
    sum += t0 + t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9;
    sum += (int)l0 + (int)l1 + (int)l2 + (int)l3;
    sum += (int)d0 + (int)d1 + (int)d2 + (int)d3;
    
    /* Complex array usage with function calls */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            sum += helper_modify_int(&arr[i][j], i + j);
            if ((i + j) % 3 == 0) {
                helper_complex_op(&arr[i][j], &l0, &d0);
            }
        }
    }
    
    /* Final ASM with vector mode mismatch - Requirement #2 */
    /* Attempt to use vector mode in general register constraint */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec = {1, 2, 3, 4};
    int scalar = 42;
    
    asm volatile (
        "=r" (scalar),
        "+r" (r0),
        : "r" (vec[0]),  /* vector element in general register */
          "r" (vec[1]),
          "m" (vec),     /* whole vector in memory */
          "r" (helper_modify_double(&d1, 2.0))
        : "memory", "xmm0", "xmm1", "xmm2", "xmm3",
          "rax", "rbx", "rcx", "rdx"
    );
    
    sum += scalar + r0 + (int)d1;
    
    return sum % 1000;  /* Return value to prevent dead code elimination */
}

/* Wrapper to ensure function is called */
int main(void) {
    int result = test_reload();
    printf("Result: %d\n", result);
    
    /* Additional test with different optimization contexts */
    volatile int v1 = test_reload();
    volatile int v2 = test_reload();
    
    return (result + v1 + v2) % 255;
}
