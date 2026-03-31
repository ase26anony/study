/* Test program to trigger push_reload logic in GCC's reload pass */
#include <stdio.h>
#include <stdint.h>

/* Helper functions that take addresses - forces complex addressing */
int helper1(int *a, int *b) { return *a + *b; }
long helper2(long *a, long *b, long *c) { return *a * *b + *c; }
double helper3(double *a, double *b) { return *a / *b; }
void helper4(int *arr, int idx) { arr[idx] = idx * 2; }
int* helper5(int *ptr, int offset) { return ptr + offset; }

/* Complex inline assembly with mismatched modes and many operands */
void test_reload() {
    /* Declare many register variables to create high register pressure */
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
    register int r10 = 11, r11 = 12, r12 = 13, r13 = 14;
    register int r14 = 15, r15 = 16, r16 = 17, r17 = 18;
    
    /* Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Perform arithmetic to create live ranges */
    r0 = r1 + r2 * r3 - r4 / (r5 + 1);
    l0 = l1 * l2 + l3 - (r0 * 2);
    d0 = d1 * d2 + d3 / (d0 + 1.0);
    
    /* Complex inline assembly block 1: Many operands with mixed constraints */
    asm volatile (
        /* Output operands with early-clobber */
        "=r" (r0),        /* General reg constraint */
        "=&r" (r1),       /* Early-clobber general reg */
        "=m" (arr[2][3]), /* Memory output */
        /* Input-output operands */
        "+r" (r2),        /* Read-write general reg */
        "+m" (arr[3][4]), /* Read-write memory */
        /* Input operands with complex addressing */
        "r" (helper1(&r3, &r4)),  /* Function call in operand */
        "m" (arr[r5][r6]),        /* Array with register indices */
        "r" (l0),                 /* Long in general reg (mismatch) */
        "i" (123),                /* Immediate */
        "r" (d0)                  /* Double in general reg (mismatch) */
        :
        /* Clobbers */
        : "memory", "eax", "ebx", "ecx", "edx", "esi", "edi",
          "r8", "r9", "r10", "r11", "xmm0", "xmm1", "xmm2", "xmm3"
    );
    
    /* More computations between asm blocks */
    for (int i = 0; i < 4; i++) {
        r10 += arr[i][i] * r11;
        l1 += helper2(&l2, &l3, &l0);
        d1 = helper3(&d2, &d3);
    }
    
    /* Complex inline assembly block 2: Mismatched modes and array indexing */
    int idx1 = r0 % 8;
    int idx2 = r1 % 8;
    int idx3 = r2 % 8;
    
    asm volatile (
        /* Operands with potential mode/class mismatches */
        "=r" (arr[idx1][idx2]),   /* Array element output - SImode */
        "=&r" (l2),               /* Early-clobber long - DImode */
        "+r" (r12),               /* Read-write int */
        "+m" (arr[idx3][idx1]),   /* Read-write array element */
        /* Inputs with complex expressions */
        "r" (helper5(&arr[0][0], idx1 * 8 + idx2)), /* Pointer arithmetic */
        "m" (arr[helper1(&idx1, &idx2) % 8][idx3]), /* Function in index */
        "r" ((long)r3 * r4),      /* 64-bit result in 32-bit reg? */
        "r" (d1),                 /* FP value in general reg */
        "i" (256),                /* Immediate */
        "r" (p0[0])               /* Pointer dereference */
        :
        /* Extensive clobber list */
        : "memory", "rax", "rbx", "rcx", "rdx", 
          "rsi", "rdi", "r8", "r9", "r10", "r11",
          "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
    );
    
    /* Complex inline assembly block 3: Mixed types and constraints */
    asm volatile (
        /* Mixed constraints */
        "=r" (r13),
        "=m" (arr[4][5]),
        "=&r" (r14),
        "+r" (r15),
        "+m" (arr[5][6]),
        /* Inputs with addressing modes */
        "r" (&arr[r16 % 8][r17 % 8]),  /* Address computation */
        "m" (arr[6][7]),
        "r" (helper2(&l0, &l1, &l2)),  /* Function returning long */
        "r" ((int)d2),                 /* FP to int conversion */
        "i" (512)
        :
        /* Clobber everything */
        : "memory", "eax", "ebx", "ecx", "edx",
          "esi", "edi", "r8", "r9", "r10", "r11",
          "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4",
          "xmm5", "xmm6", "xmm7"
    );
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            sum += arr[i][j];
        }
    }
    sum += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
    sum += (int)(l0 + l1 + l2 + l3);
    sum += (int)(d0 + d1 + d2 + d3);
    
    /* Call helper with address-taken arguments */
    helper4(&arr[0][0], sum % 64);
    
    printf("Result: %d\n", sum);
}

/* Main function to call the test */
int main() {
    test_reload();
    return 0;
}
