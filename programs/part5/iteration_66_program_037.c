/* Test program to exercise push_reload logic in GCC's reload pass */
#include <stdint.h>
#include <stdlib.h>

/* Helper functions to force address-taking */
static int helper1(int *a, int *b) { return *a + *b; }
static long helper2(long *a, long *b, long *c) { return *a + *b + *c; }
static double helper3(double *a, double *b) { return *a * *b; }
static void helper4(void *ptr) { *(volatile int*)ptr += 1; }

/* Complex inline assembly test function */
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
    register int *p0 asm("rdi") = &r0;
    register int *p1 asm("rsi") = &r1;
    register long *p2 asm("rbp") = &l0;
    
    /* Additional variables to increase pressure */
    register int v0 asm("eax") = 10;
    register int v1 asm("ebx") = 20;
    register int v2 asm("ecx") = 30;
    register int v3 asm("edx") = 40;
    register int v4 = 50;
    register int v5 = 60;
    register int v6 = 70;
    register int v7 = 80;
    register int v8 = 90;
    register int v9 = 100;
    
    /* Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Complex arithmetic to create live ranges */
    r0 = r1 * r2 + r3 - r4 / (r5 | 1);
    r1 = r6 ^ r7 & r0;
    l0 = l1 * l2 - l3;
    l1 = l0 + l2 * l3;
    d0 = d1 + d2 * d3 - d4 / d5;
    d1 = d0 * d2 - d3 + d4;
    
    /* Force computations with all variables */
    v0 = v1 + v2 - v3 * v4;
    v1 = v5 ^ v6 & v7 | v8;
    v2 = v9 + v0 * v1;
    v3 = v4 + v5 - v6;
    v4 = v7 * v8 / (v9 | 1);
    
    /* First complex inline asm: Many operands with mixed constraints */
    int result1, result2;
    long result3;
    double result4;
    
    asm volatile (
        /* Output operands with early-clobber and mismatched constraints */
        "=r" (result1),          /* General reg constraint */
        "=&r" (result2),         /* Early-clobber general reg */
        "=m" (arr[1][2]),        /* Memory output */
        "=r" (result3),          /* Long in general reg (mismatch) */
        "=x" (result4)           /* XMM register for double */
        :
        /* Input operands with complex addressing and function calls */
        : "r" (helper1(&r0, &r1)),           /* Function result as input */
          "m" (arr[r0 % 8][r1 % 8]),         /* Complex array indexing */
          "r" (l0),                          /* Long in general reg */
          "x" (d0),                          /* Double in XMM reg */
          "i" (255),                         /* Immediate */
          "r" (helper2(&l1, &l2, &l3)),      /* Another function call */
          "m" (arr[v0 % 4][v1 % 4]),         /* More complex indexing */
          "x" (helper3(&d1, &d2)),           /* Double function result */
          "r" (v2 + v3 * v4)                 /* Complex expression */
        :
        /* Extensive clobber list */
        "memory", "eax", "ebx", "ecx", "edx",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
        "cc"
    );
    
    /* Use results to prevent dead code elimination */
    r2 = result1 + result2;
    l2 = result3 + r2;
    d2 = result4 + d0;
    
    /* Second inline asm: Mismatched modes and array operands */
    int idx1 = r0 % 7;
    int idx2 = r1 % 7;
    int idx3 = r2 % 7;
    
    asm volatile (
        /* Mixed constraints with input-output operands */
        "+r" (arr[idx1][idx2]),      /* Input-output with array element */
        "=&r" (v5),                  /* Early-clobber output */
        "+m" (arr[3][4]),            /* Input-output memory */
        "=x" (d3),                   /* Double output */
        "=r" (v6)                    /* Integer output */
        :
        /* Inputs with mismatched modes/classes */
        : "r" ((long)arr[idx3][idx1]),  /* int cast to long (mode mismatch) */
          "x" ((float)d1),              /* double cast to float (class mismatch) */
          "m" (arr[idx2][idx3]),        /* Memory input */
          "i" (4096),                   /* Large immediate */
          "r" (helper1(&v7, &v8)),      /* Function with address-taking */
          "x" (d4),                     /* Double input */
          "r" (v9 * 2)                  /* Scaled index */
        :
        /* More clobbers */
        "memory", "rax", "rbx", "rcx", "rdx",
        "rsi", "rdi", "rbp",
        "xmm8", "xmm9", "xmm10", "xmm11",
        "xmm12", "xmm13", "xmm14", "xmm15",
        "cc"
    );
    
    /* Third asm: Very complex with nested addressing */
    int *ptr_arr[4] = {&arr[0][0], &arr[2][2], &arr[4][4], &arr[6][6]};
    
    asm volatile (
        /* Multiple output constraints */
        "=r" (r3),
        "=m" (arr[(idx1 + idx2) % 8][(idx2 + idx3) % 8]),
        "=&r" (r4),
        "=x" (d4),
        "=r" (v7),
        "=m" (*ptr_arr[idx1 % 4])
        :
        /* Complex inputs with function calls and array indexing */
        : "r" (helper2(&l0, &l1, ptr_arr[idx2 % 4])),
          "m" (arr[arr[idx1][idx2] % 8][arr[idx2][idx3] % 8]),
          "r" ((int64_t)helper3(&d0, &d2)),  /* double to int64_t mismatch */
          "x" (d5),
          "i" (65535),
          "m" (arr[v5 % 8][v6 % 8]),
          "r" (helper4((void*)&v8), v8),     /* void function in expression */
          "x" ((double)r5),                  /* int to double (class change) */
          "r" (v0 + v1 * v2 - v3 / (v4 | 1))
        :
        /* Complete clobber list */
        "memory",
        "rax", "rbx", "rcx", "rdx", "rdi", "rsi", "rbp",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
        "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
        "xmm12", "xmm13", "xmm14", "xmm15",
        "cc"
    );
    
    /* Final computations using all modified values */
    int final_sum = 0;
    final_sum += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
    final_sum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    final_sum += (int)l0 + (int)l1 + (int)l2 + (int)l3;
    final_sum += (int)d0 + (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5;
    
    /* Sum array elements with complex indexing */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            final_sum += arr[(i + j) % 8][(i * j) % 8];
        }
    }
    
    return final_sum;
}

/* Main function to call test */
int main(void) {
    int result = test_reload();
    /* Use result to prevent optimization */
    asm volatile ("" : : "r"(result));
    return result % 256;
}
