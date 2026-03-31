/* Test program to trigger push_reload logic in GCC reload pass */
#include <stdint.h>
#include <stdlib.h>

/* Helper functions that take addresses */
static int helper1(int *a, int *b) {
    return (*a + *b) * 2;
}

static long helper2(long *a, long *b, long *c) {
    return (*a * *b) / (*c + 1);
}

static double helper3(double *a, double *b) {
    return *a * *b - *a / *b;
}

/* Complex inline assembly with mismatched constraints */
void test_reload(void) {
    /* Create high register pressure with many register variables */
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
    
    /* Additional variables to increase pressure */
    register int v0 = 10, v1 = 11, v2 = 12, v3 = 13, v4 = 14;
    register long v5 = 50, v6 = 51, v7 = 52, v8 = 53;
    register double v9 = 5.0, v10 = 6.0, v11 = 7.0, v12 = 8.0;
    
    /* Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Complex arithmetic to create live ranges */
    r0 = r1 * r2 + r3 - r4;
    l0 = l1 * l2 / (l3 + 1);
    d0 = d1 * d2 - d3 / d1;
    v0 = v1 + v2 * v3 - v4;
    v5 = v6 * v7 + v8;
    v9 = v10 * v11 / v12;
    
    /* First complex asm: Many operands with mixed constraints */
    asm volatile (
        /* Output operands with early-clobber */
        "=r" (r0),        /* General reg constraint */
        "=&r" (r1),       /* Early-clobber */
        "=m" (arr[1][2]), /* Memory constraint */
        /* Input-output operands */
        "+r" (r2),
        "+m" (arr[2][3]),
        /* Input operands with complex addressing */
        "r" (helper1(&arr[r0][r1], &arr[r2][r3])), /* Function call in operand */
        "m" (arr[r4 % 8][r5 % 8]),                 /* Complex array indexing */
        "r" (l0),
        "r" (d0),                                   /* Mismatch: double in int reg */
        "i" (123)                                   /* Immediate */
        : /* No outputs-only here */
        : "memory", "eax", "ebx", "ecx", "edx", "esi", "edi",
          "r8", "r9", "r10", "r11", "xmm0", "xmm1", "xmm2", "xmm3"
    );
    
    /* More computations between asm blocks */
    int idx1 = r0 % 8;
    int idx2 = r1 % 8;
    int idx3 = r2 % 8;
    
    /* Second asm: Mismatched modes and array operands */
    asm volatile (
        /* DImode output but SImode variable - potential mismatch */
        "=r" (l0),        /* Long output in int register? */
        /* Array element as output with complex index */
        "=m" (arr[idx1 * 2][idx2 * 2]),
        /* Input-output with memory constraint */
        "+m" (arr[helper1(&idx1, &idx2) % 8][helper1(&idx2, &idx3) % 8]),
        /* Multiple inputs with function calls */
        "r" (helper2(&l0, &l1, &l2)),
        "r" (helper3(&d0, &d1)),
        "m" (arr[(r0 + r1) % 8][(r2 + r3) % 8]),
        "r" (p0),
        "r" (p1),
        "r" (p2),
        "r" (p3)
        :
        : "memory", "rax", "rbx", "rcx", "rdx",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
    );
    
    /* Third asm: Vector-like constraints with mismatched classes */
    asm volatile (
        /* Attempt to use FP register for integer */
        "=r" (r0),
        "=r" (r1),
        "=&r" (r2),      /* Early-clobber */
        "=m" (arr[3][4]),
        /* Input-output with different mode */
        "+r" (l0),       /* Long in general reg */
        /* Complex array addressing in inputs */
        "m" (arr[helper1(&r0, &r1) % 8][helper1(&r1, &r2) % 8]),
        "r" (helper2(&l0, &l1, &l2) + helper1(&r3, &r4)),
        "m" (arr[(l0 % 8)][(l1 % 8)]),
        "r" (d0),        /* Double in general reg - mismatch */
        "r" (d1),
        "i" (256)
        :
        : "memory", "eax", "ebx", "ecx", "edx", "esi", "edi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
    );
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    sum += r0 + r1 + r2 + r3 + r4 + r5;
    sum += l0 + l1 + l2 + l3;
    sum += (int)d0 + (int)d1 + (int)d2 + (int)d3;
    sum += v0 + v1 + v2 + v3 + v4;
    sum += v5 + v6 + v7 + v8;
    
    /* Complex array usage with results */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            sum += arr[i][j];
        }
    }
    
    /* Final computation that depends on everything */
    asm volatile (
        "addl %1, %0\n"
        "addl %2, %0\n"
        : "+r" (sum)
        : "r" (helper1(&sum, &r0)),
          "m" (arr[sum % 8][0])
        : "cc", "memory"
    );
    
    /* Return value prevents optimization */
    __asm__ __volatile__ ("" : : "r"(sum));
}

/* Main function to call test */
int main(void) {
    for (int i = 0; i < 100; i++) {
        test_reload();
    }
    return 0;
}
