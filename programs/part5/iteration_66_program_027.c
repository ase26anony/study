/* Test program to trigger reload.cc push_reload logic */
#include <stdio.h>
#include <stdint.h>

/* Helper functions to create complex addressing modes */
int helper1(int *a, int *b) { return *a + *b; }
int helper2(long *a, int b) { return (int)(*a + b); }
double helper3(double *a, double *b) { return *a * *b; }
void helper4(int *a, int b, int c) { *a = b * c; }
int helper5(int a, int b, int c, int d) { return a + b + c + d; }

/* Complex inline assembly with register pressure */
void test_reload(void) {
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
    register volatile int v0 asm("ebp") = 0;
    register volatile int v1 asm("esp") = 0;
    
    /* Additional variables without explicit registers */
    register int a = 10, b = 20, c = 30, d = 40, e = 50;
    register int f = 60, g = 70, h = 80, i = 90, j = 100;
    register int k = 110, l = 120, m = 130, n = 140, o = 150;
    register double x = 1.5, y = 2.5, z = 3.5;
    register long la = 1000, lb = 2000, lc = 3000;
    
    /* Phase 2: Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int idx = 0; idx < 8; idx++) {
        for (int jdx = 0; jdx < 8; jdx++) {
            arr[idx][jdx] = idx * 10 + jdx;
        }
    }
    
    /* Phase 3: Arithmetic operations to create live ranges */
    r0 = r1 + r2 * r3 - r4 / (r5 + 1);
    l0 = l1 * l2 + l3 - la;
    d0 = d1 * d2 + d3 - d4 / d5;
    a = b + c - d * e / f;
    x = y * z + 1.0;
    
    /* Complex index calculations */
    int idx1 = (r0 + r1) % 8;
    int idx2 = (r2 * r3) % 8;
    int idx3 = (r4 ^ r5) % 8;
    int idx4 = (r6 | r7) % 8;
    
    /* Phase 4: First complex inline asm with many operands */
    asm volatile (
        /* Output operands with early-clobber */
        "=r" (r0),        /* General reg constraint */
        "=&r" (r1),       /* Early-clobber general */
        "=m" (arr[idx1][idx2]),  /* Memory constraint */
        "=r" (l0),        /* Long in general reg */
        
        /* Input-output operands */
        "+r" (r2),        /* Read-write operand */
        "+m" (arr[idx3][idx4]),  /* Read-write memory */
        
        /* Input operands with mixed constraints */
        "r" (r3),         /* General reg */
        "m" (arr[1][2]),  /* Memory */
        "r" (l1),         /* Long in general reg */
        "i" (12345),      /* Immediate */
        
        /* Clobber list */
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13",
          "xmm0", "xmm1", "xmm2", "xmm3",
          "memory", "cc"
    );
    
    /* Use results to prevent dead code elimination */
    a = r0 + r1;
    arr[idx1][idx2] += l0;
    
    /* Phase 5: Function calls with address-taken arguments */
    int result1 = helper1(&arr[idx1][idx2], &arr[idx3][idx4]);
    long result2 = helper2(&la, result1);
    double result3 = helper3(&d0, &d1);
    
    /* Phase 6: Second inline asm with mismatched modes */
    asm volatile (
        /* Mismatched operand modes */
        "=r" (r4),        /* SImode expected */
        "=r" (l2),        /* DImode in general reg */
        "=x" (d2),        /* DFmode in SSE reg */
        
        /* Complex addressing in input */
        "r" (arr[idx1 + idx2][idx3 + idx4]),  /* Non-simple address */
        "r" (helper5(r0, r1, r2, r3)),        /* Function call result */
        
        /* Input-output with mismatched class */
        "+r" (r5),        /* GENERAL_REGS */
        "+x" (d3),        /* FP_REGS - potential conflict */
        
        /* Memory operand with complex addressing */
        "m" (arr[(r0 % 4)][(r1 % 4)]),
        
        /* Clobbers */
        :
        : "rax", "rbx", "xmm4", "xmm5", "xmm6", "xmm7",
          "r8", "r9", "r10", "r11",
          "memory"
    );
    
    /* More arithmetic to maintain register pressure */
    b = c * d - e / f + g * h;
    y = z * x + d0 - d1;
    la = lb * lc + l0 - l1;
    
    /* Phase 7: Third asm with array indexing in operands */
    int idx5 = (a + b) % 8;
    int idx6 = (c * d) % 8;
    int idx7 = (e ^ f) % 8;
    
    asm volatile (
        /* Array elements as operands */
        "=m" (arr[idx5][idx6]),      /* Output to array */
        "=r" (r6),                   /* Output to register */
        "=&r" (r7),                  /* Early-clobber output */
        
        /* Array inputs with complex indexing */
        "r" (arr[idx6][idx7]),       /* Array element input */
        "r" (arr[(idx5 + 1) % 8][(idx6 + 1) % 8]),
        
        /* Input-output array element */
        "+m" (arr[idx7][idx5]),
        
        /* Register with function call addressing */
        "r" (helper1(&arr[idx5][idx6], &r6)),
        
        /* Additional constraints */
        "r" (la),
        "r" (lb),
        "x" (d4),
        
        /* Extensive clobber list */
        :
        : "rax", "rbx", "rcx", "rdx",
          "rsi", "rdi", "rbp", "rsp",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
          "memory", "cc"
    );
    
    /* Final computations using all variables */
    int final_sum = 0;
    final_sum += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
    final_sum += a + b + c + d + e + f + g + h + i + j;
    final_sum += k + l + m + n + o;
    final_sum += (int)l0 + (int)l1 + (int)l2 + (int)l3;
    final_sum += (int)d0 + (int)d1 + (int)d2 + (int)d3;
    final_sum += arr[0][0] + arr[7][7];
    final_sum += result1 + (int)result2 + (int)result3;
    
    /* Use volatile to ensure computation isn't optimized away */
    v0 = final_sum;
    v1 = v0 * 2;
    
    /* Return value through memory */
    *(volatile int*)&arr[0][0] = v1;
}

/* Main function to call test */
int main(void) {
    test_reload();
    return 0;
}
