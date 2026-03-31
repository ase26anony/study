/* Test program to trigger reload.cc uncovered lines 1381-1399 */
#include <stdio.h>
#include <stdint.h>

/* Helper functions for address-taken arguments */
int helper1(int *a, int *b) {
    return (*a + *b) * 2;
}

void helper2(long *arr, int idx, int val) {
    arr[idx] = val * 3;
}

double helper3(double *d1, double *d2, double *d3) {
    return *d1 * *d2 + *d3;
}

/* Complex inline assembly with mismatched modes and high register pressure */
int test_reload(void) {
    /* Declare many register variables to create high register pressure */
    register int r0 asm("r0") = 1;
    register int r1 asm("r1") = 2;
    register int r2 asm("r2") = 3;
    register int r3 asm("r3") = 4;
    register int r4 asm("r4") = 5;
    register int r5 asm("r5") = 6;
    register int r6 asm("r6") = 7;
    register int r7 asm("r7") = 8;
    register int r8 asm("r8") = 9;
    register int r9 asm("r9") = 10;
    register long l0 asm("r10") = 100;
    register long l1 asm("r11") = 200;
    register long l2 asm("r12") = 300;
    register long l3 asm("r13") = 400;
    register long l4 asm("r14") = 500;
    register double d0 asm("xmm0") = 1.1;
    register double d1 asm("xmm1") = 2.2;
    register double d2 asm("xmm2") = 3.3;
    register double d3 asm("xmm3") = 4.4;
    register double d4 asm("xmm4") = 5.5;
    register int *p0 asm("r15") = &r0;
    register int *p1 asm("ebx") = &r1;
    register int *p2 asm("esi") = &r2;
    register int *p3 asm("edi") = &r3;
    
    /* Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Perform arithmetic to create live ranges */
    r0 = r1 + r2 * r3 - r4 / (r5 + 1);
    r1 = r6 * r7 - r8 + r9;
    l0 = l1 * l2 + l3 - l4;
    l1 = l2 * 3 + l3 / 2;
    d0 = d1 * d2 + d3 - d4;
    d1 = d2 * 2.5 + d3 / 1.5;
    
    /* Complex inline assembly #1: Many operands with mixed constraints */
    int result1, result2, result3;
    long result4;
    double result5;
    
    asm volatile (
        /* Output operands with early-clobber */
        "=r" (result1),        /* General reg constraint */
        "=&r" (result2),       /* Early-clobber general reg */
        "=m" (arr[2][3]),      /* Memory constraint on array element */
        "=r" (result4),        /* Long in general reg (mismatch potential) */
        "=x" (result5)         /* SSE register for double */
        :
        /* Input operands with complex addressing */
        : "r" (r0),            /* Simple register */
          "m" (arr[r1][r2]),   /* Complex array indexing */
          "r" (helper1(&r3, &r4)),  /* Function call in operand */
          "i" (100),           /* Immediate */
          "X" (d0),            /* Any register (mismatch potential) */
          "r" (l0),            /* Long in general reg */
          "m" (arr[1][2]),     /* Another memory operand */
          "r" (r5)             /* Another register */
        : "memory", "eax", "ebx", "ecx", "edx", "esi", "edi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "cc"
    );
    
    /* More computations between asm blocks */
    r2 = result1 * r3 + r4;
    r3 = helper1(&result2, &r5);
    l2 = l3 * 2 + result4;
    d2 = helper3(&d1, &d3, &result5);
    
    /* Complex inline assembly #2: Input-output operands and mismatched modes */
    int io1 = 100, io2 = 200;
    long io3 = 300;
    double io4 = 400.0;
    
    asm volatile (
        /* Input-output operands with + constraint */
        "+r" (io1),            /* Input-output general reg */
        "+&r" (io2),           /* Early-clobber input-output */
        "+m" (arr[3][4]),      /* Memory input-output */
        "+r" (io3),            /* Long in general reg (mode mismatch) */
        "+x" (io4)             /* Double in SSE reg */
        :
        /* Input operands with complex expressions */
        : "r" (arr[r6][r7]),   /* Dynamic array indexing */
          "m" (arr[helper1(&r8, &r9) % 8][l0 % 8]),  /* Function in index */
          "i" (255),           /* Immediate */
          "g" (d2),            /* General (register or memory) */
          "r" (helper2(&l1, io1, io2))  /* Function call returning void */
        : "memory", "rax", "rbx", "rcx", "rdx",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12",
          "xmm13", "xmm14", "xmm15", "cc"
    );
    
    /* Complex inline assembly #3: Multi-dimensional array in constraints */
    int idx1 = r0 % 8, idx2 = r1 % 8, idx3 = r2 % 8;
    
    asm volatile (
        "=r" (r4),
        "=m" (arr[idx1][idx2]),
        "=r" (l3),
        "=x" (d3)
        : "0" (r3),                    /* Matching constraint */
          "r" (arr[idx2][idx3]),       /* Array element as input */
          "m" (arr[helper1(&idx1, &idx2) % 8][idx3]),  /* Complex address */
          "r" ((long)helper1(&r5, &r6) * 2),  /* Function result */
          "X" (d4),                    /* Any register constraint */
          "i" (4096)                   /* Large immediate */
        : "memory", "eax", "ebx", "ecx", "edx",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4",
          "xmm5", "xmm6", "xmm7", "cc"
    );
    
    /* Use results to prevent dead code elimination */
    int final_sum = result1 + result2 + result3 + io1 + io2 + r4;
    final_sum += arr[2][3] + arr[3][4] + arr[idx1][idx2];
    final_sum += (int)result4 + (int)io3 + (int)result5 + (int)io4 + (int)d3;
    final_sum += helper1(&final_sum, &r0);
    
    return final_sum;
}

/* Main function to call the test */
int main(void) {
    int result = test_reload();
    printf("Result: %d\n", result);
    return 0;
}
