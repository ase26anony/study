/* Test program to trigger push_reload logic in GCC reload pass */
#include <stdint.h>
#include <stdlib.h>

/* Helper functions to force address-taking and complex expressions */
static int helper1(int *a, int *b) { return *a + *b; }
static long helper2(long *a, long *b, long *c) { return *a * *b + *c; }
static double helper3(double *a, double *b) { return *a / *b; }
static void* helper4(void **arr, int i, int j) { 
    return (char*)arr[i] + j * sizeof(int); 
}

/* Function to create register pressure and complex reload scenarios */
int test_reload(void) {
    /* Phase 1: Declare many register variables to create pressure */
    register int r0 asm("eax") = 1;
    register int r1 asm("ebx") = 2;
    register int r2 asm("ecx") = 3;
    register int r3 asm("edx") = 4;
    register int r4 asm("esi") = 5;
    register int r5 asm("edi") = 6;
    register long r6 asm("r8") = 7L;
    register long r7 asm("r9") = 8L;
    register long r8 asm("r10") = 9L;
    register long r9 asm("r11") = 10L;
    register double f0 asm("xmm0") = 1.0;
    register double f1 asm("xmm1") = 2.0;
    register double f2 asm("xmm2") = 3.0;
    register double f3 asm("xmm3") = 4.0;
    register int* p0 asm("r12") = &r0;
    register int* p1 asm("r13") = &r1;
    register long* p2 asm("r14") = &r6;
    register double* p3 asm("r15") = &f0;
    register volatile int v0 asm("") = 100;
    register volatile long v1 asm("") = 200;
    register volatile double v2 asm("") = 300.0;
    
    /* Additional variables to increase pressure */
    register int a0 = 11, a1 = 12, a2 = 13, a3 = 14, a4 = 15;
    register long b0 = 21, b1 = 22, b2 = 23, b3 = 24;
    register double c0 = 31.0, c1 = 32.0, c2 = 33.0;
    
    /* Phase 2: Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Phase 3: Arithmetic operations to create live ranges */
    r0 = r1 + r2 * r3 - r4 / (r5 + 1);
    r6 = r7 * r8 + r9 - (r6 >> 2);
    f0 = f1 * f2 + f3 / (f0 + 1.0);
    
    /* Complex index calculations using register variables */
    int idx1 = (r0 * r1 + r2) % 8;
    int idx2 = (r3 * r4 - r5) % 8;
    int idx3 = (r6 * r7 + r8) % 8;
    
    /* Phase 4: First complex inline asm with many operands and mismatches */
    /* This asm forces mismatched modes: mixing SImode and DImode in constraints */
    asm volatile (
        /* Output operands with early-clobber to prevent reuse */
        "=r" (r0),           /* SImode output */
        "=&r" (r6),          /* DImode output with early-clobber */
        "+m" (arr[idx1][idx2]), /* Memory operand that's both read and written */
        "=r" (r1),           /* Another SImode output */
        "=&r" (r7),          /* Another DImode with early-clobber */
        
        /* Input operands with mixed constraints */
        : "r" (r2),          /* SImode input */
        "m" (arr[idx2][idx3]), /* Memory input */
        "r" ((long)r3),      /* DImode input (mismatch with SImode r3) */
        "i" (16),            /* Immediate */
        "r" (f0),            /* FP register in general reg constraint - mismatch! */
        "m" (arr[idx3][idx1]), /* Another memory input */
        "r" (r4),            /* Another SImode input */
        "r" (r8)             /* DImode input */
        
        /* Extensive clobber list */
        : "memory", "eax", "ebx", "ecx", "edx", "esi", "edi",
          "r8", "r9", "r10", "r11", "xmm0", "xmm1", "xmm2", "xmm3",
          "cc"
    );
    
    /* Use results to prevent dead code elimination */
    int sum1 = r0 + r1 + (int)r6 + (int)r7;
    
    /* Phase 5: Function calls with address-taking within asm operands */
    /* This creates complex addressing modes requiring reloads */
    int complex_result;
    asm volatile (
        "=r" (complex_result),
        "=m" (arr[((idx1 + idx2) * idx3) % 8][0]),
        "+r" (r0),
        "+r" (r1)
        :
        : "r" (helper1(&r0, &r1)),  /* Function call in input operand */
          "r" (helper2(&r6, &r7, &r8)), /* Another function call */
          "m" (arr[helper1(&idx1, &idx2) % 8][helper1(&idx2, &idx3) % 8]), /* Complex array indexing */
          "r" ((long)helper4((void**)arr, idx1, idx2)), /* Pointer calculation */
          "i" (sizeof(int) * 8)
        : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "cc"
    );
    
    /* Phase 6: Third asm with vector-like constraints and mode mismatches */
    /* Simulating vector operations with scalar constraints */
    long vec_results[4];
    asm volatile (
        /* Output array elements as separate operands */
        "=r" (vec_results[0]),
        "=r" (vec_results[1]),
        "=&r" (vec_results[2]),  /* Early-clobber */
        "=r" (vec_results[3]),
        
        /* Input with mixed scalar/array elements */
        : "r" (arr[idx1][0]),     /* Array element as input */
          "r" (arr[idx1][1]),
          "m" (arr[idx2]),        /* Whole row as memory operand */
          "r" ((long)arr[idx3][0] << 32 | arr[idx3][1]), /* Packed data */
          "r" (helper3(&f0, &f1)), /* FP function result in integer constraint */
          
          /* Input-output operand with complex addressing */
          "+m" (arr[(r0 * r1) % 8][(r6 * r7) % 8])
        :
        : "memory", "rax", "rbx", "rcx", "rdx",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "cc"
    );
    
    /* Phase 7: More computations using asm results */
    for (int i = 0; i < 4; i++) {
        sum1 += vec_results[i];
    }
    
    /* Complex array access using all computed values */
    int final_idx = (sum1 + complex_result + r0 + (int)r6) % 8;
    int final_sum = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            final_sum += arr[i][j] * (final_idx + 1);
        }
    }
    
    /* Use all register variables in final computation */
    final_sum += r0 + r1 + r2 + r3 + r4 + r5 + a0 + a1 + a2 + a3 + a4;
    final_sum += (int)r6 + (int)r7 + (int)r8 + (int)r9;
    final_sum += (int)f0 + (int)f1 + (int)f2 + (int)f3;
    final_sum += (int)c0 + (int)c1 + (int)c2;
    final_sum += v0 + (int)v1 + (int)v2;
    
    return final_sum + complex_result + sum1;
}

/* Main function to call test and prevent optimization */
int main() {
    int result = test_reload();
    
    /* Additional complexity to prevent optimization */
    volatile int sink = result;
    
    /* More inline asm to potentially trigger additional reloads */
    asm volatile (
        "mov %0, %%eax\n\t"
        "add $1, %%eax\n\t"
        "mov %%eax, %0"
        : "+r" (result)
        :
        : "eax", "cc"
    );
    
    return result % 256;  /* Return non-zero to indicate execution */
}
