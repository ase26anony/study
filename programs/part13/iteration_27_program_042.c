/* Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force spills with many volatile variables */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
volatile double f1 = 1.1, f2 = 2.2, f3 = 3.3, f4 = 4.4;
volatile float f5 = 5.5, f6 = 6.6;

/* Explicit register variables to pin registers */
register int r12_var asm ("r12") = 0x1234;
register int r13_var asm ("r13") = 0x5678;
register int r14_var asm ("r14") = 0x9ABC;
register int r15_var asm ("r15") = 0xDEF0;

int main(void) {
    /* Mixed data types to create mode conflicts */
    char c1 = 'a', c2 = 'b';
    short s1 = 100, s2 = 200;
    int i1 = 1000, i2 = 2000, i3 = 3000, i4 = 4000;
    long l1 = 50000L, l2 = 60000L;
    float f7 = 7.7f, f8 = 8.8f;
    double d1 = 9.9, d2 = 10.10;
    
    /* Force many live values by using all variables */
    int sum = v1 + v2 + v3 + v4 + v5;
    double fsum = f1 + f2 + f3 + f4;
    
    /* Block 1: Complex inline assembly with conflicting constraints */
block1:
    {
        int temp1 = i1 + i2;
        int temp2 = i3 + i4;
        int *ptr1 = &i1;
        int *ptr2 = &i2;
        
        /* Inline assembly with memory and register constraints that conflict */
        asm volatile (
            "mov %[in1], %%eax\n\t"
            "add %[in2], %%eax\n\t"
            "mov %%eax, %[out1]\n\t"
            "imul %[in3], %%eax\n\t"
            "mov %%eax, %[out2]\n\t"
            : [out1] "=m" (i1), [out2] "=r" (temp1)
            : [in1] "r" (temp2), [in2] "m" (v6), [in3] "r" (v7)
            : "rax", "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        /* Use explicit register variables that might be clobbered */
        r12_var = temp1 + r12_var;
        r13_var = temp2 + r13_var;
        
        sum += i1 + temp1;
        goto block2;
    }
    
    /* Unreachable but creates control flow complexity */
    {
        int dummy = v8 + v9;
        asm volatile ("nop" : : : "memory");
    }
    
block2:
    {
        /* More complex assembly with addressing mode conflicts */
        long addr_temp = (long)&l1;
        short *sptr = &s1;
        
        asm volatile (
            "mov %[addr], %%rbx\n\t"
            "mov (%%rbx), %%rax\n\t"
            "add %[val1], %%rax\n\t"
            "mov %%rax, %[out1]\n\t"
            "mov %[in2], %%rcx\n\t"
            "add %%rcx, %[out2]\n\t"
            : [out1] "=m" (l1), [out2] "+m" (l2)
            : [addr] "r" (addr_temp), [val1] "r" (v8), [in2] "r" (v9)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "memory", "cc"
        );
        
        /* Force floating point reloads */
        asm volatile (
            "movsd %[d1], %%xmm0\n\t"
            "addsd %[d2], %%xmm0\n\t"
            "movsd %%xmm0, %[out]\n\t"
            : [out] "=m" (d1)
            : [d1] "m" (d1), [d2] "m" (d2)
            : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory"
        );
        
        fsum += d1 + d2;
        goto block3;
    }
    
block3:
    {
        /* Assembly with output operand that conflicts with input */
        int out_val;
        int in_val = v10 + sum;
        
        asm volatile (
            "lea (%[in], %[in], 2), %[out]\n\t"
            "add $100, %[out]\n\t"
            : [out] "=&r" (out_val)  /* Early clobber - can't share reg with inputs */
            : [in] "r" (in_val)
            : "cc"
        );
        
        /* Mixed size operands causing mode conversions */
        char char_result;
        asm volatile (
            "mov %[in], %%al\n\t"
            "add $32, %%al\n\t"
            "mov %%al, %[out]\n\t"
            : [out] "=r" (char_result)
            : [in] "r" (c1)
            : "rax"
        );
        
        c2 = char_result;
        sum += out_val + char_result;
        goto block4;
    }
    
block4:
    {
        /* Complex assembly spanning multiple clobbered registers */
        int a = r12_var, b = r13_var, c = r14_var, d = r15_var;
        
        asm volatile (
            "mov %[a], %%r12\n\t"
            "mov %[b], %%r13\n\t"
            "add %%r13, %%r12\n\t"
            "mov %%r12, %[out1]\n\t"
            "mov %[c], %%r14\n\t"
            "mov %[d], %%r15\n\t"
            "imul %%r15, %%r14\n\t"
            "mov %%r14, %[out2]\n\t"
            : [out1] "=m" (a), [out2] "=m" (b)
            : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d)
            : "r12", "r13", "r14", "r15", "rax", "rbx", "rcx", "rdx", 
              "rsi", "rdi", "r8", "r9", "r10", "r11", "memory", "cc"
        );
        
        r12_var = a;
        r13_var = b;
        sum += a + b;
        
        /* Final assembly with many operands */
        int x1 = v1, x2 = v2, x3 = v3, x4 = v4, x5 = v5;
        int y1, y2, y3, y4, y5;
        
        asm volatile (
            "mov %[x1], %%eax\n\t"
            "add %[x2], %%eax\n\t"
            "mov %%eax, %[y1]\n\t"
            "mov %[x3], %%ebx\n\t"
            "add %[x4], %%ebx\n\t"
            "mov %%ebx, %[y2]\n\t"
            "mov %[x5], %%ecx\n\t"
            "imul %%eax, %%ecx\n\t"
            "mov %%ecx, %[y3]\n\t"
            "lea (%%rax, %%rbx, 2), %%rdx\n\t"
            "mov %%rdx, %[y4]\n\t"
            "sub %%rcx, %%rdx\n\t"
            "mov %%rdx, %[y5]\n\t"
            : [y1] "=&r" (y1), [y2] "=&r" (y2), [y3] "=&r" (y3),
              [y4] "=&r" (y4), [y5] "=&r" (y5)
            : [x1] "r" (x1), [x2] "r" (x2), [x3] "r" (x3),
              [x4] "r" (x4), [x5] "r" (x5)
            : "rax", "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        sum += y1 + y2 + y3 + y4 + y5;
    }
    
    /* Final volatile store to prevent optimization */
    volatile int final_sum = sum + (int)fsum + c1 + c2 + s1 + s2 + i1 + i2 + l1 + l2;
    
    printf("Result: %d\n", final_sum);
    return 0;
}
