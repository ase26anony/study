/* test_reload.c - Complex inline assembly to trigger reload.cc logic */
#include <stdio.h>
#include <stdint.h>

/* Helper functions that take addresses */
int helper1(int *a, int *b) {
    return (*a + *b) * 2;
}

long helper2(long *a, long *b, long *c) {
    return *a * *b + *c;
}

double helper3(double *a, double *b) {
    return *a / *b;
}

/* Main test function with high register pressure */
int test_reload(void) {
    /* Declare many register variables to create pressure */
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
    register int *p1 asm("r16") = &r1;
    register long *p2 asm("r17") = &l0;
    register long *p3 asm("r18") = &l1;
    
    /* Multi-dimensional array with complex indexing */
    int arr[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            arr[i][j] = i * 10 + j;
        }
    }
    
    /* Complex arithmetic to create live ranges */
    r0 = r1 + r2 * r3 - r4 / (r5 + 1);
    r1 = r6 ^ r7 | r8 & r9;
    l0 = l1 * l2 + l3 - l4;
    l1 = l2 << 2 | l3 >> 1;
    d0 = d1 * d2 + d3 / d4;
    d1 = d2 - d3 * d4;
    
    /* Complex index calculations */
    int idx1 = (r0 + r1) % 8;
    int idx2 = (r2 * r3) % 8;
    int idx3 = (r4 ^ r5) % 8;
    int idx4 = (r6 | r7) % 8;
    
    /* First inline asm: Many operands with mixed constraints */
    int result1, result2;
    long result3;
    double result4;
    
    asm volatile (
        /* Output operands with early-clobber */
        "=r" (result1)           /* output only */
        "=&r" (result2)          /* early-clobber output */
        "=m" (arr[idx1][idx2])   /* memory output */
        "=r" (result3)           /* long output */
        "=f" (result4)           /* float output */
        
        /* Input operands */
        : "r" (r0)               /* simple register */
        "m" (arr[idx3][idx4])    /* memory input */
        "r" (l0)                 /* mismatched mode: long in int constraint */
        "i" (12345)              /* immediate */
        "r" (d0)                 /* float in general reg constraint - mismatch! */
        
        /* Input-output operands */
        "+r" (r8)                /* read-write */
        "+m" (arr[0][0])         /* read-write memory */
        
        /* Clobbers */
        : "memory", "eax", "ebx", "ecx", "edx", 
          "r8", "r9", "r10", "r11", "r12",
          "xmm0", "xmm1", "xmm2", "xmm3",
          "cc"
        
        /* Additional constraints for specific cases */
        : "%0 = result1, %1 = result2, %2 = arr[idx1][idx2], "
          "%3 = result3, %4 = result4, "
          "%5 = r0, %6 = arr[idx3][idx4], %7 = l0, "
          "%8 = 12345, %9 = d0, %10 = r8, %11 = arr[0][0]"
    );
    
    /* Use results to prevent dead code elimination */
    r0 = result1 + result2;
    l0 = result3 + arr[idx1][idx2];
    d0 = result4 * 2.0;
    
    /* Second inline asm: Function calls in operands with address-taking */
    int func_result1, func_result2;
    long func_result3;
    
    asm volatile (
        /* Complex operand with function call */
        "movl %[call1], %%eax\n\t"
        "addl %[call2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        
        /* Memory operand with complex addressing */
        "movq (%[arrptr], %[idx], 4), %%rbx\n\t"
        "addq %%rbx, %[out3]\n\t"
        
        /* Outputs */
        : [out1] "=r" (func_result1)
          [out2] "=m" (arr[idx2][idx3])
          [out3] "+r" (func_result3)
        
        /* Inputs with function calls creating complex addressing */
        : [call1] "r" (helper1(&r0, &r1))
          [call2] "r" (helper2(&l0, &l1, &l2))
          [arrptr] "r" (arr)
          [idx] "r" (idx1 * 8 + idx2)  /* complex index calculation */
          [dbl] "f" (helper3(&d0, &d1)) /* float from function */
        
        /* Extensive clobber list */
        : "rax", "rbx", "rcx", "rdx", "rdi", "rsi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "memory", "cc"
    );
    
    /* Third inline asm: Mismatched modes and classes */
    int32_t i32;
    int64_t i64;
    float f32;
    double f64;
    
    /* Force mismatches between modes and constraints */
    asm volatile (
        /* DImode value in SImode constraint */
        "movq %[in64], %%rax\n\t"
        "movl %%eax, %[out32]\n\t"
        
        /* SImode value in DImode constraint */
        "movslq %[in32], %%rbx\n\t"
        "movq %%rbx, %[out64]\n\t"
        
        /* Float in integer constraint */
        "movd %[infloat], %%eax\n\t"
        "movd %%eax, %[outfloat]\n\t"
        
        /* Outputs */
        : [out32] "=r" (i32)      /* gets DImode value truncated */
          [out64] "=r" (i64)      /* gets SImode value sign-extended */
          [outfloat] "=f" (f32)   /* gets integer bits reinterpreted */
        
        /* Inputs with deliberate mode/class mismatches */
        : [in64] "r" ((int64_t)0x123456789ABCDEF0)  /* DImode */
          [in32] "r" ((int32_t)0x87654321)          /* SImode */
          [infloat] "r" (0x40490FDA)                /* integer bits of pi */
        
        : "rax", "rbx", "xmm0", "memory", "cc"
    );
    
    /* Use all results in final computation */
    int final_sum = 0;
    final_sum += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9;
    final_sum += l0 + l1 + l2 + l3 + l4;
    final_sum += (int)d0 + (int)d1 + (int)d2 + (int)d3 + (int)d4;
    final_sum += result1 + result2 + func_result1;
    final_sum += arr[0][0] + arr[idx1][idx2] + arr[idx3][idx4];
    final_sum += i32 + (int)i64 + (int)f32;
    final_sum += *p0 + *p1 + (int)*p2 + (int)*p3;
    
    return final_sum;
}

/* Wrapper to prevent optimization */
int main(void) {
    int result = test_reload();
    printf("Result: %d\n", result);
    return 0;
}
