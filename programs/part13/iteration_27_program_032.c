/* Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force spills with many volatile variables */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
volatile float f1 = 1.0f, f2 = 2.0f, f3 = 3.0f, f4 = 4.0f;
volatile double d1 = 1.0, d2 = 2.0, d3 = 3.0;

/* Explicit register variables to force conflicts */
register int r12_var asm ("r12") = 100;
register int r13_var asm ("r13") = 200;
register int r14_var asm ("r14") = 300;
register int r15_var asm ("r15") = 400;

int main(void) {
    /* Non-volatile variables with mixed types */
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    int i1 = 10000, i2 = 20000, i3 = 30000, i4 = 40000;
    long l1 = 50000L, l2 = 60000L;
    float f_local1 = 1.5f, f_local2 = 2.5f;
    double d_local1 = 1.25, d_local2 = 2.25;
    
    /* Force many live values by using all variables */
    int sum = v1 + v2 + v3 + v4 + v5;
    sum += v6 + v7 + v8 + v9 + v10;
    sum += (int)f1 + (int)f2 + (int)f3 + (int)f4;
    sum += (int)d1 + (int)d2 + (int)d3;
    sum += c1 + c2 + s1 + s2;
    sum += i1 + i2 + i3 + i4 + l1 + l2;
    sum += (int)f_local1 + (int)f_local2;
    sum += (int)d_local1 + (int)d_local2;
    sum += r12_var + r13_var + r14_var + r15_var;
    
    /* Block 1: Complex inline assembly with conflicting constraints */
block1:
    {
        int temp1 = sum * 2;
        int temp2 = sum / 2;
        int temp3, temp4;
        
        /* Inline assembly with multiple constraints that conflict */
        asm volatile (
            /* Output operands with early clobber to force separate registers */
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "imull %[in3], %%eax\n\t"
            "movl %%eax, %[out2]\n\t"
            /* Clobber many registers to force spills */
            :
            [out1] "=&r" (temp3),    /* Early clobber - can't share with inputs */
            [out2] "=&r" (temp4)     /* Early clobber */
            :
            [in1] "r" (temp1),       /* Register constraint */
            [in2] "rm" (temp2),      /* Register or memory - may force reload */
            [in3] "rm" (i1)          /* i1 might be in memory */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
              "memory", "cc"
        );
        
        sum = temp3 + temp4;
        /* Modify variables to keep them live */
        i1 += 100;
        i2 += 200;
    }
    
    goto block2;
    
    /* Dead code to create control flow complexity */
    {
        int dummy = sum * 3;
        dummy += v1;
    }
    
block2:
    {
        /* Take address of variables to create addressing mode conflicts */
        int* ptr1 = &i3;
        int* ptr2 = &i4;
        long* ptr3 = &l1;
        
        /* Mixed type inline assembly */
        asm volatile (
            /* Different sized operations */
            "movb %[c1], %%al\n\t"
            "movw %[s1], %%bx\n\t"
            "movl %[i2], %%ecx\n\t"
            "movq %[l1], %%r8\n\t"
            /* Force memory access with address taken */
            "addl (%%rsi), %%ecx\n\t"
            "addl (%%rdi), %%ecx\n\t"
            "movl %%ecx, %[result]\n\t"
            :
            [result] "=r" (sum)
            :
            [c1] "r" ((int)c1),
            [s1] "r" ((int)s1),
            [i2] "r" (i2),
            [l1] "r" (l1),
            [p1] "r" (ptr1),
            [p2] "r" (ptr2)
            : "rax", "rbx", "rcx", "r8", "rsi", "rdi", "memory", "cc"
        );
        
        /* Use explicit register variables that might be clobbered */
        asm volatile (
            "addl %%r12d, %[sum]\n\t"
            "addl %%r13d, %[sum]\n\t"
            : [sum] "+r" (sum)
            : 
            : "r12", "r13", "cc"
        );
    }
    
    goto block3;
    
block3:
    {
        /* Floating point mixed with integer */
        double fp_temp;
        int int_temp;
        
        /* Complex constraints forcing reloads */
        asm volatile (
            /* Convert float to int and back */
            "cvtsd2si %[dbl], %%eax\n\t"
            "addl %[ival], %%eax\n\t"
            "cvtsi2sd %%eax, %%xmm0\n\t"
            "movsd %%xmm0, %[fpout]\n\t"
            "movl %%eax, %[intout]\n\t"
            : [fpout] "=m" (fp_temp),    /* Memory constraint */
              [intout] "=r" (int_temp)    /* Register constraint */
            : [dbl] "xm" (d_local1),     /* SSE register or memory */
              [ival] "rm" (sum)          /* Register or memory */
            : "rax", "xmm0", "xmm1", "xmm2", "xmm3", "memory", "cc"
        );
        
        sum = int_temp + (int)fp_temp;
        
        /* Force more register pressure */
        int a = v1 * v2;
        int b = v3 * v4;
        int c = v5 * v6;
        int d = v7 * v8;
        int e = v9 * v10;
        
        /* Another inline asm with many operands */
        asm volatile (
            "imull %%ebx, %%eax\n\t"
            "addl %%ecx, %%eax\n\t"
            "addl %%edx, %%eax\n\t"
            "addl %%esi, %%eax\n\t"
            : "=a" (sum)
            : "a" (a), "b" (b), "c" (c), "d" (d), "S" (e)
            : "cc"
        );
    }
    
    /* Final computation using all variables */
    volatile int checksum = 0;
    checksum += sum;
    checksum += i1 + i2 + i3 + i4;
    checksum += l1 + l2;
    checksum += (int)f_local1 + (int)f_local2;
    checksum += (int)d_local1 + (int)d_local2;
    checksum += r12_var + r13_var + r14_var + r15_var;
    checksum += c1 + c2 + s1 + s2;
    
    /* Prevent dead code elimination */
    printf("Result: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
