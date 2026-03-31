/* Test program to trigger push_reload logic in reload.cc */
#include <stdint.h>
#include <stdlib.h>

/* Helper functions to force address-taking */
static int helper1(int *a, int *b) { return *a + *b; }
static long helper2(long *a, long *b, long *c) { return *a * *b + *c; }
static double helper3(double *a, double *b) { return *a / *b; }
static void helper4(int *a, int *b, int *c) { *c = *a - *b; }

/* Complex inline assembly with register pressure */
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
    register double d0 asm("xmm0") = 1.0;
    register double d1 asm("xmm1") = 2.0;
    register double d2 asm("xmm2") = 3.0;
    register double d3 asm("xmm3") = 4.0;
    register double d4 asm("xmm4") = 5.0;
    register double d5 asm("xmm5") = 6.0;
    register int *p0 asm("rdi") = &r0;
    register int *p1 asm("rsi") = &r1;
    register long *p2 asm("rbp") = &l0;
    register double *p3 asm("rsp") = &d0;
    
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
        r1 = r1 ^ r4 | r5 & r6;
        r2 = r2 + r7 * r0;
        r3 = r3 - r1 / (r4 + 1);
        l0 = l0 * l1 + l2 - l3;
        l1 = l1 ^ l0 | l2 & l3;
        d0 = d0 * d1 + d2 - d3;
        d1 = d1 / d4 * d5;
        d2 = d2 + d0 * d1;
    }
    
    /* First complex inline asm: Many operands with mixed constraints */
    int result1, result2;
    long result3;
    double result4;
    
    asm volatile (
        /* Output operands with early-clobber */
        "=r" (result1)           /* General reg output */
        "=&r" (result2)          /* Early-clobber general reg */
        "=m" (arr[1][2])         /* Memory output */
        "=r" (result3)           /* Long output in general reg */
        "=f" (result4)           /* Floating point output */
        
        /* Input operands */
        : "r" (r0)               /* General reg input */
        "m" (arr[r1][r2])        /* Memory input with complex addressing */
        "r" (l0)                 /* Long in general reg (mismatch potential) */
        "f" (d0)                 /* Float reg input */
        "i" (123)                /* Immediate */
        
        /* Input-output operands */
        "+r" (r4)                /* Input-output general reg */
        "+m" (arr[3][4])         /* Input-output memory */
        "+&r" (r5)               /* Early-clobber input-output */
        
        /* Clobbers */
        : "memory", "eax", "ebx", "ecx", "edx", 
          "r8", "r9", "r10", "r11", "r12",
          "xmm0", "xmm1", "xmm2", "xmm3",
          "cc"
    );
    
    /* Use results to prevent dead code elimination */
    r0 = result1 + result2;
    l0 = result3 + r0;
    d0 = result4 * 2.0;
    
    /* Second inline asm: Mismatched modes and function calls in operands */
    int idx1 = r0 % 8;
    int idx2 = r1 % 8;
    int idx3 = r2 % 8;
    
    /* Function calls with address-taking in asm operands */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "imull %[in3], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movq %[in4], %%rbx\n\t"
        "addq %[in5], %%rbx\n\t"
        "movq %%rbx, %[out2]\n\t"
        : [out1] "=rm" (arr[idx1][idx2])      /* Output with reg/memory constraint */
          [out2] "=r" (l1)                    /* Long output */
        : [in1] "rm" (helper1(&r0, &r1))      /* Function call result as input */
          [in2] "r" (helper2(&l0, &l1, &l2))  /* Another function call */
          [in3] "i" (256)                     /* Immediate */
          [in4] "m" (arr[helper1(&idx1, &idx2)][idx3])  /* Complex array addressing */
          [in5] "r" (l3)                      /* Long input */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "memory", "cc"
    );
    
    /* Third inline asm: Vector-like operations with mismatched constraints */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec1 = {r0, r1, r2, r3};
    v4si vec2 = {r4, r5, r6, r7};
    v4si vec3;
    
    /* Attempt to use vector in general register constraint (mismatch) */
    asm volatile (
        "movdqa %[v1], %%xmm6\n\t"
        "paddd %[v2], %%xmm6\n\t"
        "movdqa %%xmm6, %[v3]\n\t"
        : [v3] "=x" (vec3)        /* Vector output in xmm register */
        : [v1] "x" (vec1)         /* Vector input */
          [v2] "x" (vec2)         /* Vector input */
          "r" (arr[0][0])         /* Extra general reg input (pressure) */
          "r" (arr[1][1])
          "r" (arr[2][2])
          "r" (arr[3][3])
        : "xmm6", "xmm7", "memory"
    );
    
    /* More computations using asm results */
    helper4(&arr[idx1][idx2], &r0, &r1);
    d0 = helper3(&d0, &d1) + helper3(&d2, &d3);
    
    /* Final complex asm with all types of operands */
    int final_result;
    asm volatile (
        /* Multiple output operands */
        "=r" (final_result)
        "=m" (arr[7][7])
        "=r" (r7)
        
        /* Mixed input operands */
        : "r" (r0)
          "m" (arr[r1][r2 % 8])      /* Complex array indexing */
          "r" ((long)r3)             /* Cast causing potential mode mismatch */
          "f" (d0)                   /* Float in float reg */
          "m" (arr[4][helper1(&idx1, &idx3)])  /* Function call in array index */
          
        /* Input-output with early-clobber */
        "+&r" (r6)
        "+m" (arr[5][5])
        
        /* Extensive clobber list */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10",
          "memory", "cc"
    );
    
    /* Final computation to return meaningful value */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            sum += arr[i][j];
        }
    }
    
    sum += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
    sum += (int)l0 + (int)l1 + (int)l2 + (int)l3;
    sum += (int)d0 + (int)d1 + (int)d2 + (int)d3;
    sum += final_result;
    
    return sum;
}

/* Main function to call test */
int main() {
    return test_reload() % 256;  /* Return non-zero to indicate execution */
}
