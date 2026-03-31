/* Test program to trigger push_reload logic in reload.cc */
#include <stdint.h>
#include <stdlib.h>

/* Helper functions to create complex addressing modes */
static int helper1(int *a, int *b) { return *a + *b; }
static long helper2(long *a, long *b, long *c) { return *a + *b + *c; }
static double helper3(double *a, double *b) { return *a * *b; }
static void helper4(int *a, int b) { *a += b; }
static void helper5(long *a, long b, long c) { *a = b * c; }

/* Function with high register pressure and complex inline asm */
int test_reload(void) {
    /* Declare many register variables to create register pressure */
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
    register long l4 = 500, l5 = 600, l6 = 700, l7 = 800;
    register double d4 = 5.0, d5 = 6.0, d6 = 7.0, d7 = 8.0;
    
    /* Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Perform arithmetic to create live ranges */
    r0 = r1 + r2 * r3 - r4 / (r5 + 1);
    l0 = l1 * l2 + l3 - l4;
    d0 = d1 * d2 + d3 / d4;
    
    /* Complex inline asm block 1: Many operands with mixed constraints */
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
        "r" (r0),           /* Simple register */
        "m" (arr[r1][r2]),  /* Memory with index calculation */
        "r" (helper1(&r3, &r4)),  /* Function call in input */
        "i" (12345),        /* Immediate */
        "r" (l0),           /* Long in register */
        "x" (d0),           /* Double in XMM */
        "m" (arr[helper1(&r5, &r6) % 8][r7 % 8])  /* Complex memory address */
        :
        /* Extensive clobber list */
        "memory", "eax", "ebx", "ecx", "edx", "esi", "edi",
        "r8", "r9", "r10", "r11", "xmm0", "xmm1", "xmm2", "xmm3",
        "cc"
    );
    
    /* More arithmetic between asm blocks */
    r1 = result1 + r2;
    l1 = result3 * l2;
    d1 = result4 + d2;
    
    /* Complex inline asm block 2: Mismatched modes and array indexing */
    int idx1 = r3 % 8;
    int idx2 = r4 % 8;
    int idx3 = r5 % 8;
    
    asm volatile (
        /* Mixed constraints with potential mode mismatches */
        "=r" (arr[idx1][idx2]),      /* int output to memory via register */
        "+&r" (r6),                  /* Early-clobber input-output */
        "=m" (arr[3][4]),            /* Direct memory output */
        "=x" (d2),                   /* Double output */
        "=r" (l3)                    /* Long output */
        :
        /* Inputs with mismatched modes/classes */
        "r" ((long)r7),              /* int cast to long - potential mismatch */
        "m" (arr[idx3][idx1]),       /* Memory input with variable indices */
        "x" ((float)d3),             /* double cast to float - mode mismatch */
        "i" (256),                   /* Immediate */
        "r" (helper2(&l4, &l5, &l6)), /* Function returning long */
        "m" (*p0),                   /* Dereferenced pointer */
        "r" (helper3(&d4, &d5))      /* Function returning double */
        :
        /* Clobbers */
        "memory", "rax", "rbx", "rcx", "rdx",
        "xmm4", "xmm5", "xmm6", "xmm7", "cc"
    );
    
    /* Complex inline asm block 3: Input-output operands and address-taking */
    int io1 = 100;
    long io2 = 200;
    double io3 = 3.14;
    
    asm volatile (
        /* Input-output operands */
        "+r" (io1),                  /* Input-output general register */
        "+&m" (arr[5][6]),           /* Early-clobber memory input-output */
        "+x" (io3),                  /* Input-output XMM register */
        "=r" (r8),                   /* Output */
        "=m" (arr[7][0])             /* Memory output */
        :
        /* Complex inputs with address-taking */
        "r" (helper4(&io1, r9)),     /* Function taking address */
        "m" (arr[helper1(&idx1, &idx2) % 8][helper1(&idx3, &r0) % 8]),
        "r" (helper5(&io2, l6, l7)), /* Function with multiple address args */
        "i" (4096),                  /* Large immediate */
        "X" (d6),                    /* Any register class */
        "g" (arr[2][3])              /* General - could be register or memory */
        :
        /* Full clobber */
        "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
        "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
        "cc"
    );
    
    /* Use results to prevent dead code elimination */
    int sum = result1 + result2 + r6 + r8 + io1;
    sum += arr[1][2] + arr[3][4] + arr[5][6] + arr[7][0];
    sum += (int)result3 + (int)l3 + (int)io2;
    sum += (int)result4 + (int)d2 + (int)io3;
    
    /* More computations with all variables */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            sum += arr[i][j];
        }
    }
    
    sum += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9;
    sum += (int)(l0 + l1 + l2 + l3 + l4 + l5 + l6 + l7);
    sum += (int)(d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7);
    
    return sum;
}

/* Main function to call test and prevent optimization */
int main(void) {
    int result = test_reload();
    /* Use volatile to ensure computation isn't optimized away */
    volatile int dummy = result;
    return dummy % 256;  /* Return non-zero to indicate execution */
}
