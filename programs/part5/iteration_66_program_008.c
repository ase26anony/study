/* Test program to trigger push_reload logic in GCC's reload pass */
#include <stdint.h>
#include <stdlib.h>

/* Helper functions that take addresses */
static int helper1(int *a, int *b) { return *a + *b; }
static long helper2(long *a, long *b, long *c) { return *a + *b + *c; }
static double helper3(double *a, double *b) { return *a * *b; }

/* Function to create register pressure and complex addressing */
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
    
    register long l0 asm("eax") = 100;
    register long l1 asm("ebx") = 200;
    register long l2 asm("ecx") = 300;
    register long l3 asm("edx") = 400;
    
    register double d0 asm("xmm0") = 1.1;
    register double d1 asm("xmm1") = 2.2;
    register double d2 asm("xmm2") = 3.3;
    register double d3 asm("xmm3") = 4.4;
    
    register int *p0 asm("rsi") = &r0;
    register int *p1 asm("rdi") = &r1;
    register long *p2 asm("rbp") = &l0;
    register double *p3 asm("rsp") = &d0;
    
    /* Additional variables to increase pressure */
    register int a0 = 10, a1 = 11, a2 = 12, a3 = 13, a4 = 14;
    register long b0 = 1000, b1 = 1001, b2 = 1002, b3 = 1003;
    register double c0 = 10.1, c1 = 10.2, c2 = 10.3, c3 = 10.4;
    
    /* Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Perform arithmetic to create live ranges */
    r0 = r1 + r2 * r3 - r4 / (r5 + 1);
    l0 = l1 * l2 + l3 - b0;
    d0 = d1 * d2 + d3 - c0;
    
    /* Complex inline assembly #1: Many operands with mixed constraints */
    int result1, result2;
    long result3;
    double result4;
    
    asm volatile (
        /* Output operands with different constraints */
        "=r" (result1),      /* General register output */
        "=&r" (result2),     /* Early-clobber general register */
        "=m" (arr[1][2]),    /* Memory output */
        "=r" (result3),      /* Long in general register */
        "=x" (result4)       /* XMM register for double */
        :
        /* Input operands with complex addressing */
        "r" (r0),            /* Simple register */
        "m" (arr[r1][r2]),   /* Memory with register indexing - complex addressing */
        "r" (helper1(&r3, &r4)),  /* Function call in operand */
        "i" (256),           /* Immediate */
        "X" (d0)             /* Any register/memory for double */
        :
        /* Extensive clobber list */
        "memory", "eax", "ebx", "ecx", "edx", "r8", "r9", "r10", "r11",
        "r12", "r13", "r14", "r15", "xmm0", "xmm1", "xmm2", "xmm3",
        "cc"
    );
    
    /* More computations between asm blocks */
    a0 = result1 * 2;
    b0 = result3 + l0;
    c0 = result4 * d1;
    
    /* Complex inline assembly #2: Mismatched modes and array operands */
    int idx1 = r0 % 8;
    int idx2 = r1 % 8;
    int idx3 = r2 % 8;
    
    asm volatile (
        /* Mixed input/output constraints */
        "+r" (a0),           /* Read-write operand */
        "=r" (a1),
        "=&r" (a2),
        "+m" (arr[idx1][idx2]),  /* Read-write memory with complex index */
        "=x" (c1)
        :
        /* Inputs with potential mode mismatches */
        "r" ((long)r3),      /* int in long-sized register - potential mismatch */
        "m" (arr[helper1(&idx1, &idx2) % 8][idx3]), /* Very complex addressing */
        "r" (helper2(&l0, &l1, &l2)),  /* Function call returning long */
        "X" (helper3(&d0, &d1)),       /* Function call returning double */
        "i" (128)
        :
        /* Clobber everything */
        "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rbp",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
        "cc"
    );
    
    /* Complex inline assembly #3: Many operands with early clobber */
    int out1, out2, out3;
    long out4;
    double out5;
    
    asm volatile (
        /* 10 operands total */
        "=&r" (out1),
        "=r" (out2),
        "=&r" (out3),
        "=r" (out4),
        "=x" (out5),
        "+m" (arr[3][4]),
        "=m" (arr[4][5])
        :
        "r" (a0),
        "m" (arr[a1][a2]),    /* Array with register indexing */
        "r" (helper1(&a3, &a4)),
        "r" (b0),
        "X" (c0),
        "i" (64),
        "r" (p0),             /* Pointer in register */
        "m" (*p1)             /* Dereferenced pointer */
        :
        "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rbp",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
        "cc"
    );
    
    /* Use all results to prevent dead code elimination */
    int final_sum = result1 + result2 + a0 + a1 + a2 + out1 + out2 + out3;
    final_sum += (int)result3 + (int)out4;
    final_sum += (int)result4 + (int)out5;
    final_sum += arr[1][2] + arr[3][4] + arr[4][5];
    final_sum += arr[idx1][idx2] + arr[r0 % 8][r1 % 8];
    
    /* More register pressure computations */
    for (int i = 0; i < 4; i++) {
        r0 = r0 * r1 + r2;
        l0 = l0 + l1 * l2;
        d0 = d0 * d1 + d2;
        a0 = a0 + a1 * a2;
        b0 = b0 + b1 - b2;
        c0 = c0 * c1 / c2;
        
        /* Use array with complex indexing in loop */
        final_sum += arr[(r0 + i) % 8][(l0 + i) % 8];
    }
    
    return final_sum;
}

/* Main function to call test */
int main(void) {
    int result = test_reload();
    return result % 256;  /* Return non-zero to prevent optimization */
}
