/* Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force spills with many volatile variables */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
volatile double f1 = 1.1, f2 = 2.2, f3 = 3.3, f4 = 4.4;
volatile float f5 = 5.5, f6 = 6.6, f7 = 7.7, f8 = 8.8;

/* Explicit register variables to pin registers */
register int r12_var asm ("r12") = 0x1234;
register int r13_var asm ("r13") = 0x5678;
register int r14_var asm ("r14") = 0x9ABC;
register int r15_var asm ("r15") = 0xDEF0;

int main(void) {
    /* Mixed data types to cause mode mismatches */
    char c1 = 'a', c2 = 'b';
    short s1 = 100, s2 = 200;
    int i1 = 1000, i2 = 2000, i3 = 3000, i4 = 4000;
    long l1 = 50000, l2 = 60000;
    float f_local1 = 1.5, f_local2 = 2.5;
    double d_local1 = 3.14159, d_local2 = 2.71828;
    
    /* Variables that will have their addresses taken */
    int addr_var1 = 111, addr_var2 = 222, addr_var3 = 333;
    double addr_double = 444.444;
    
    /* Force many live values */
    int sum = 0;
    
    /* Block 1: Complex inline assembly with conflicting constraints */
block1:
    {
        int temp1 = v1 + v2;
        int temp2 = v3 * v4;
        
        /* Inline asm with memory and register constraints that conflict */
        asm volatile (
            "mov %[in1], %%eax\n\t"
            "add %[in2], %%eax\n\t"
            "mov %%eax, %[out1]\n\t"
            "imul %[in3], %%ebx\n\t"
            "add %%ebx, %[out2]\n\t"
            : [out1] "=m" (addr_var1), [out2] "=r" (i1)
            : [in1] "r" (temp1), [in2] "m" (v5), [in3] "r" (temp2)
            : "rax", "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        /* Use explicit register variables that get clobbered */
        r12_var = i1 + addr_var1;
        sum += r12_var;
    }
    
    /* Block 2: More conflicts with floating point */
    goto block2;
    
    /* Dead code to create control flow complexity */
    {
        int dummy = v6 * v7;
        dummy += v8;
    }
    
block2:
    {
        /* Take address and use in asm while variable is also in register */
        int* ptr1 = &addr_var2;
        double* dptr = &addr_double;
        
        /* Mixed size operands causing mode issues */
        asm volatile (
            "movswl %w[char_in], %%eax\n\t"
            "addl %[int_in], %%eax\n\t"
            "movl %%eax, %[int_out]\n\t"
            "movsd %[double_in], %%xmm0\n\t"
            "addsd %[double_mem], %%xmm0\n\t"
            "movsd %%xmm0, %[double_out]\n\t"
            : [int_out] "=m" (addr_var2), [double_out] "=m" (*dptr)
            : [char_in] "r" (c1), [int_in] "r" (s1), 
              [double_in] "x" (d_local1), [double_mem] "m" (f1)
            : "rax", "xmm0", "xmm1", "memory", "cc"
        );
        
        /* Force r13_var to be live and clobbered */
        asm volatile (
            "mov %0, %%r13\n\t"
            "add $0x1111, %%r13\n\t"
            : : "r" (r13_var) : "r13"
        );
        
        sum += addr_var2 + (int)addr_double;
    }
    
    /* Block 3: Multiple output operands with earlyclobber */
    goto block3;
    
block3:
    {
        int out1, out2, out3;
        
        /* '&' means earlyclobber - can't share register with inputs */
        asm volatile (
            "mov %[in1], %[out1]\n\t"
            "lea (%[in2], %[in3]), %[out2]\n\t"
            "imul %[in4], %[out3]\n\t"
            : [out1] "=&r" (out1), [out2] "=&r" (out2), [out3] "=r" (out3)
            : [in1] "r" (i2), [in2] "r" (i3), [in3] "r" (i4), [in4] "r" (l1)
            : "cc"
        );
        
        /* Complex expression keeping many values live */
        sum += out1 + out2 + out3 + r14_var + r15_var;
        
        /* Force spill by using all volatiles */
        v1 = out1; v2 = out2; v3 = out3;
        f1 = d_local1; f2 = d_local2;
    }
    
    /* Block 4: Memory operand with register constraint conflict */
block4:
    {
        int index = sum % 10;
        int array[10] = {0,1,2,3,4,5,6,7,8,9};
        
        /* Address computation that might need reload */
        asm volatile (
            "mov (%[base], %[index], 4), %%eax\n\t"
            "add %%eax, %[sum]\n\t"
            : [sum] "+m" (sum)
            : [base] "r" (array), [index] "r" (index)
            : "rax", "memory", "cc"
        );
        
        /* Use floating point with integer, causing potential reloads */
        asm volatile (
            "cvtsi2sd %[int_val], %%xmm0\n\t"
            "addsd %[double_val], %%xmm0\n\t"
            "cvttsd2si %%xmm0, %[result]\n\t"
            : [result] "=r" (i1)
            : [int_val] "r" (sum), [double_val] "x" (f3)
            : "xmm0", "cc"
        );
    }
    
    /* Block 5: Final complex asm with many clobbers */
block5:
    {
        long final_result;
        
        /* Massive clobber list forcing many saves/restores */
        asm volatile (
            "mov %[a], %%rax\n\t"
            "add %[b], %%rax\n\t"
            "mov %%rax, %[c]\n\t"
            "mov %[d], %%rbx\n\t"
            "imul %%rbx, %%rax\n\t"
            : [c] "=m" (final_result)
            : [a] "r" (l2), [b] "r" (sum), [d] "r" (i1)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10",
              "memory", "cc"
        );
        
        sum += (int)final_result;
    }
    
    /* Prevent dead code elimination */
    volatile int checksum = sum;
    printf("Result: %d\n", checksum);
    
    return checksum & 0xFF;
}
