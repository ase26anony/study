/* reload_test.c - Test program to trigger push_reload logic in GCC reload pass */

#include <stdint.h>
#include <stdlib.h>

/* Helper functions to create complex addressing modes */
static int helper1(int *a, int b) { return *a + b; }
static long helper2(long *a, long b, long c) { return *a * b + c; }
static double helper3(double *a, double b) { return *a / b; }
static void* helper4(void** arr, int i, int j) { 
    return (void*)((uintptr_t)arr[i] + j * sizeof(int)); 
}

/* Function to test reload logic with high register pressure */
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
    register double d0 asm("xmm0") = 1.0;
    register double d1 asm("xmm1") = 2.0;
    register double d2 asm("xmm2") = 3.0;
    register double d3 asm("xmm3") = 4.0;
    register int* p0 asm("r12") = &r0;
    register int* p1 asm("r13") = &r1;
    register long* p2 asm("r14") = &l0;
    register double* p3 asm("r15") = &d0;
    register int r6 = 7, r7 = 8, r8 = 9, r9 = 10;
    register long l4 = 5000L, l5 = 6000L;
    register double d4 = 5.0, d5 = 6.0;
    
    /* Phase 2: Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Phase 3: Arithmetic operations to create live ranges */
    r0 = r1 + r2 * r3 - r4 / (r5 + 1);
    l0 = l1 * l2 + l3 - l4;
    d0 = d1 * d2 - d3 / d4 + d5;
    
    for (int i = 0; i < 5; i++) {
        r6 = r6 * 2 + r7;
        r7 = r7 + r8 - r9;
        l4 = l4 + l5 * i;
        d4 = d4 * 1.1 + d5;
    }
    
    /* Phase 4: Complex inline assembly block 1 - Many operands with mixed constraints */
    int result1, result2;
    long result3;
    double result4;
    
    asm volatile (
        /* Output operands with early-clobber */
        "=r" (result1)           /* Output only */
        "=&r" (result2)          /* Early-clobber output */
        "=m" (arr[1][2])         /* Memory output */
        "=r" (result3)           /* Long output */
        "=f" (result4)           /* Floating output */
        
        /* Input operands */
        : "r" (r0)               /* Simple register */
        "m" (arr[r1][r2])        /* Memory with register indexing */
        "r" (helper1(&r3, r4))   /* Function call in operand */
        "i" (16)                 /* Immediate */
        "r" (l0)                 /* Long in general reg */
        "f" (d0)                 /* Float in FP reg */
        "m" (arr[3][4])          /* Another memory operand */
        "r" ((int)helper3(&d1, d2)) /* Cast function result */
        
        /* Input-output operands */
        "+r" (r5)                /* Read-write operand */
        "+m" (arr[2][3])         /* Read-write memory */
        "+&r" (r6)               /* Early-clobber read-write */
        
        /* Clobbers */
        : "memory", "eax", "ebx", "ecx", "edx", "esi", "edi",
          "r8", "r9", "r10", "r11", "xmm0", "xmm1", "xmm2", "xmm3",
          "cc"
    );
    
    /* Use results to prevent dead code elimination */
    r0 = result1 + result2;
    l0 = result3 + (long)result4;
    
    /* Phase 5: Complex inline assembly block 2 - Mismatched modes and array indexing */
    int idx1 = r1 % 8;
    int idx2 = r2 % 8;
    int idx3 = r3 % 8;
    
    asm volatile (
        /* Operands with potential mode/class mismatches */
        "=r" (arr[idx1][idx2])           /* int output, might need reload */
        "=m" (arr[idx3][idx1])           /* memory output */
        "=&r" (r7)                       /* early-clobber */
        
        : "r" ((long)arr[idx2][idx3])    /* int array element as long - MODE MISMATCH */
        "m" (arr[(r4 % 8)][(r5 % 8)])    /* complex memory address */
        "r" (helper2(&l1, l2, l3))       /* function returning long */
        "i" (0xFFFFFFFF)                 /* large immediate */
        
        /* Input-output with complex addressing */
        "+m" (arr[(r6 % 8)][(r7 % 8)])   /* read-write with dynamic indices */
        "+r" (r8)                        /* read-write register */
        
        /* Force FP register use with integer */
        "f" ((double)r9)                 /* int in FP reg - CLASS MISMATCH */
        
        : "memory", "rax", "rbx", "rcx", "rdx",
          "rsi", "rdi", "r8", "r9", "r10",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4",
          "cc"
    );
    
    /* Phase 6: Third assembly block with pointer arithmetic */
    void* ptr_array[4] = {&r0, &r1, &r2, &r3};
    int complex_idx = (r0 + r1 + r2) % 4;
    
    asm volatile (
        /* Multiple output constraints */
        "=r" (r9)
        "=m" (arr[complex_idx][0])
        "=r" (l5)
        "=f" (d5)
        
        /* Complex inputs with address-taking */
        : "r" (helper4(ptr_array, complex_idx, r3))  /* returns pointer */
        "m" (*((int*)ptr_array[complex_idx]))        /* dereferenced pointer */
        "r" (&arr[r4 % 8][r5 % 8])                   /* address of array element */
        "i" (sizeof(double))                         /* size constant */
        
        /* Input-output with memory constraint */
        "+m" (arr[0][complex_idx])
        "+&r" (l4)                                   /* early-clobber long */
        
        : "memory", "rax", "rbx", "rcx", "rdx",
          "rsi", "rdi", "r8", "r9", "r10", "r11",
          "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "cc"
    );
    
    /* Phase 7: Use all results in final computation */
    int final_sum = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            final_sum += arr[i][j];
        }
    }
    
    final_sum += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9;
    final_sum += (int)l0 + (int)l1 + (int)l2 + (int)l3 + (int)l4 + (int)l5;
    final_sum += (int)d0 + (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5;
    final_sum += result1 + result2 + (int)result3 + (int)result4;
    
    return final_sum;
}

/* Main function to call test and prevent optimization */
int main(void) {
    int result = test_reload();
    
    /* Use result to prevent dead code elimination */
    asm volatile ("" : : "r" (result));
    
    return result % 256;  /* Return non-zero to indicate execution */
}
