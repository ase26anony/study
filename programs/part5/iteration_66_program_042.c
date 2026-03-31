/* Test program to exercise push_reload logic in reload.cc */
#include <stdio.h>
#include <stdint.h>

/* Helper functions that take addresses - forces complex addressing */
int helper1(int *a, int *b) {
    return (*a + *b) * 2;
}

long helper2(long *a, long *b, long *c) {
    return *a * *b + *c;
}

double helper3(double *a, double *b) {
    return *a / *b + 1.0;
}

void helper4(int *arr, int idx) {
    arr[idx] = idx * 2;
}

/* Complex inline assembly with register pressure */
__attribute__((noinline))
int test_reload() {
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
    register volatile int v0 asm("ebp") = 42;
    register volatile long v1 asm("esp") = 99;
    
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
        r1 = r1 ^ r2 | r3 & r4;
        r2 = r2 + r3 * r4 / (r5 + 1);
        r3 = r3 - r4 + r5 - r6;
        r4 = r4 * r5 % (r6 + 1);
        
        l0 = l0 + l1 * l2 - l3;
        l1 = l1 ^ l2 | l3;
        l2 = l2 * 3 + l0 / 2;
        
        d0 = d0 * d1 + d2 - d3;
        d1 = d1 / d2 * d3 + d4;
        d2 = d2 - d3 + d4 - d5;
    }
    
    int result = 0;
    
    /* First complex asm: Many operands with mixed constraints */
    asm volatile (
        /* Output operands with early-clobber */
        "=r" (r0),        /* General reg constraint */
        "=&r" (r1),       /* Early-clobber general */
        "=m" (arr[1][2]), /* Memory output */
        "=r" (l0),        /* Long in general reg */
        
        /* Input operands */
        : "r" (r2),       /* General reg input */
        "m" (arr[3][4]),  /* Memory input */
        "r" (l1),         /* Long in general reg */
        "i" (123),        /* Immediate */
        "r" (d0),         /* Double in general reg (mismatch!) */
        
        /* Input-output operands */
        "+r" (r3),        /* Read-write general */
        "+m" (arr[2][3]), /* Read-write memory */
        "+r" (l2),        /* Read-write long */
        
        /* Clobbers */
        : "memory", "eax", "ebx", "ecx", "edx", "r8", "r9", "r10", 
          "r11", "xmm0", "xmm1", "xmm2", "xmm3", "cc"
    );
    
    /* Use results to prevent dead code elimination */
    result += r0 + r1 + r3;
    result += arr[1][2] + arr[2][3];
    
    /* Second asm: Mismatched modes and function calls in operands */
    int idx1 = r4 % 8;
    int idx2 = r5 % 8;
    
    asm volatile (
        /* Complex addressing with function calls */
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "imull %[in3], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "leal (%[in4], %[in5], 2), %%ebx\n\t"
        "movl %%ebx, %[out2]\n\t"
        
        : [out1] "=r" (arr[idx1][idx2]),  /* Array element output */
          [out2] "=m" (arr[idx1+1][idx2]) /* Memory output */
        
        : [in1] "r" (helper1(&r0, &r1)),  /* Function call in input */
          [in2] "m" (arr[helper1(&r2, &r3) % 8][helper2(&l0, &l1, &l2) % 8]), /* Complex array indexing */
          [in3] "i" (sizeof(double)),     /* Immediate with size */
          [in4] "r" (r6),
          [in5] "r" (helper4(arr[idx2], idx1), r7) /* Function with side effect */
        
        : "eax", "ebx", "ecx", "memory", "cc"
    );
    
    /* Third asm: Vector-like operations with mismatched classes */
    typedef int v4si __attribute__((vector_size(16)));
    register v4si vec0 asm("xmm6") = {1, 2, 3, 4};
    register v4si vec1 asm("xmm7") = {5, 6, 7, 8};
    
    asm volatile (
        /* Attempt to use vector in general purpose register */
        "movd %[vec], %%eax\n\t"
        "addl %[scalar], %%eax\n\t"
        "movd %%eax, %[vec]\n\t"
        
        : [vec] "+r" (vec0)  /* Mismatch: vector in general reg */
        : [scalar] "r" (r0)  /* Scalar in general reg */
        : "eax", "xmm6", "xmm7", "memory"
    );
    
    /* Use vector results */
    int vec_sum = vec0[0] + vec0[1] + vec0[2] + vec0[3];
    
    /* More computations to maintain register pressure */
    for (int i = 0; i < 50; i++) {
        r0 = helper1(&r0, &r1);
        r1 = helper1(&r1, &r2);
        r2 = helper1(&r2, &r3);
        
        l0 = helper2(&l0, &l1, &l2);
        l1 = helper2(&l1, &l2, &l3);
        
        d0 = helper3(&d0, &d1);
        d1 = helper3(&d1, &d2);
        
        /* Complex array indexing in computations */
        int idx = (r0 + r1 + r2) % 8;
        arr[idx][idx] = helper1(&arr[idx][0], &arr[0][idx]);
    }
    
    /* Final sum using all variables */
    result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
    result += l0 + l1 + l2 + l3;
    result += (int)d0 + (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5;
    result += vec_sum;
    
    /* Sum array elements */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            result += arr[i][j];
        }
    }
    
    return result;
}

/* Main function to call test */
int main() {
    int result = test_reload();
    printf("Result: %d\n", result);
    return 0;
}
