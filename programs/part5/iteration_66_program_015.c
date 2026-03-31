/* Test program to trigger push_reload logic in GCC reload.cc */
#include <stdio.h>
#include <stdint.h>

/* Helper functions to force address-taking and complex expressions */
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

/* Main test function with high register pressure */
int test_reload(void) {
    /* Phase 1: Declare many register variables to create pressure */
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
    register double d0 asm("xmm0") = 1.5;
    register double d1 asm("xmm1") = 2.5;
    register double d2 asm("xmm2") = 3.5;
    register double d3 asm("xmm3") = 4.5;
    register int *p0 asm("r12") = &r0;
    register int *p1 asm("r13") = &r1;
    register long *p2 asm("r14") = &l0;
    register double *p3 asm("r15") = &d0;
    register int r6 = 7, r7 = 8, r8 = 9, r9 = 10;
    register int r10 = 11, r11 = 12, r12 = 13, r13 = 14;
    register int r14 = 15, r15 = 16, r16 = 17, r17 = 18;
    
    /* Phase 2: Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Phase 3: Arithmetic operations to create live ranges */
    r0 = r1 + r2 * r3 - r4 / (r5 + 1);
    l0 = l1 * l2 + l3 - (r0 * 2);
    d0 = d1 * d2 + d3 / (d0 + 1.0);
    
    /* Complex index calculations using register variables */
    int idx1 = (r0 + r1) % 8;
    int idx2 = (r2 * r3) % 8;
    int idx3 = (r4 ^ r5) % 8;
    int idx4 = (r6 | r7) % 8;
    
    /* ASM BLOCK 1: Many operands with mixed constraints */
    asm volatile (
        /* Output operands with early-clobber */
        "=r" (r0),      /* General reg constraint */
        "=&r" (r1),     /* Early-clobber */
        "=m" (arr[idx1][idx2]),  /* Memory output */
        "=r" (l0),      /* Long in general reg */
        
        /* Input operands */
        : "r" (r2),     /* Input in register */
        "m" (arr[idx3][idx4]),   /* Memory input */
        "r" (l1),       /* Long input */
        "i" (123),      /* Immediate */
        "r" (d0),       /* Double in general reg (mismatch!) */
        
        /* Input-output operands */
        "+r" (r3),      /* Read-write */
        "+m" (arr[0][0]), /* Memory read-write */
        
        /* Clobbers */
        : "memory", "eax", "ebx", "ecx", "edx", "esi", "edi",
          "r8", "r9", "r10", "r11", "xmm0", "xmm1", "xmm2", "xmm3"
    );
    
    /* Intermediate computations to force spills */
    r4 = helper1(&r0, &r1);
    l1 = helper2(&l0, &l2, &l3);
    d1 = helper3(&d0, &d2);
    
    /* Update array using helper with address */
    helper4(arr[idx1], idx2);
    
    /* ASM BLOCK 2: Mismatched modes and complex array operands */
    asm volatile (
        /* Mixed mode operands - DImode in SImode constraints */
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movq %[in3], %%xmm0\n\t"  /* Double in vector reg */
        "addsd %[in4], %%xmm0\n\t"
        "movq %%xmm0, %[out2]\n\t"
        
        : [out1] "=r" (r5),        /* SImode output */
          [out2] "=m" (arr[idx3][idx4]),  /* Memory output */
          
        : [in1] "r" (r6),          /* SImode input */
          [in2] "m" (arr[idx1][idx2]),    /* Memory input */
          [in3] "r" (d2),          /* DImode in general reg - mismatch! */
          [in4] "x" (d3),          /* DImode in XMM register */
          
        : "eax", "xmm0", "memory"
    );
    
    /* ASM BLOCK 3: 10 operands with nested function calls in expressions */
    int temp1 = helper1(&r0, &r1);
    long temp2 = helper2(&l0, &l1, &l2);
    
    asm volatile (
        "mov %[in1], %%eax\n\t"
        "imul %[in2], %%eax\n\t"
        "add %[in3], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        "lea (%[in4], %[in5], 2), %%ebx\n\t"
        "mov %%ebx, %[out2]\n\t"
        "mov %[in6], %%ecx\n\t"
        "add %[in7], %%ecx\n\t"
        "mov %%ecx, %[out3]\n\t"
        
        : [out1] "=&r" (r7),      /* Early-clobber output */
          [out2] "=r" (r8),
          [out3] "=m" (arr[1][1]), /* Memory output */
          "=r" (r9),              /* Unnamed output */
          "=r" (r10),             /* Another output */
          
        : [in1] "r" (r2),
          [in2] "r" (r3),
          [in3] "i" (helper1(&r4, &r5)),  /* Nested function call! */
          [in4] "r" (temp1),
          [in5] "r" (temp2),      /* Long in SImode constraint */
          [in6] "m" (arr[idx2][idx3]),
          [in7] "r" (helper1(&arr[idx1][0], &arr[0][idx4])), /* Complex! */
          "r" (r11),              /* Unnamed input */
          "m" (arr[2][2]),        /* Memory input */
          "r" (r12)               /* Another input */
          
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    /* Phase 4: Use results to prevent dead code elimination */
    int sum = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
    sum += l0 + l1 + l2 + l3;
    sum += (int)d0 + (int)d1 + (int)d2 + (int)d3;
    
    /* Complex array access pattern */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            sum += arr[i][j];
            sum += arr[j][i];  /* Transpose access */
        }
    }
    
    return sum;
}

/* Driver to ensure function is called */
int main(void) {
    int result = test_reload();
    printf("Result: %d\n", result);
    return 0;
}
