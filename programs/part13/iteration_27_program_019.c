/* Test program to trigger reload.cc uncovered block in push_reload */
#include <stdio.h>
#include <stdlib.h>

/* Force specific register usage to create conflicts */
register long reg_var1 asm ("r12");
register int reg_var2 asm ("r13");
register double reg_var3 asm ("xmm8");

/* Volatile variables to prevent optimization and force spills */
volatile int v1 = 1;
volatile int v2 = 2;
volatile int v3 = 3;
volatile int v4 = 4;
volatile int v5 = 5;
volatile int v6 = 6;
volatile int v7 = 7;
volatile int v8 = 8;
volatile int v9 = 9;
volatile int v10 = 10;

volatile float f1 = 1.1f;
volatile float f2 = 2.2f;
volatile float f3 = 3.3f;
volatile double d1 = 1.11;
volatile double d2 = 2.22;
volatile double d3 = 3.33;

int main(void) {
    int result = 0;
    int i, j, k, l, m, n, o, p, q, r;
    short s1, s2, s3;
    char c1, c2, c3;
    float local_f1, local_f2, local_f3;
    double local_d1, local_d2;
    
    /* Initialize explicit register variables */
    reg_var1 = 0x123456789ABCDEF0LL;
    reg_var2 = 0xDEADBEEF;
    reg_var3 = 3.141592653589793;
    
    /* Create many live variables with different types */
    i = v1 + v2;
    j = v3 * v4;
    k = v5 - v6;
    l = v7 / (v8 ? v8 : 1);
    m = v9 ^ v10;
    
    s1 = (short)i;
    s2 = (short)j;
    s3 = (short)k;
    
    c1 = (char)(i & 0xFF);
    c2 = (char)(j & 0xFF);
    c3 = (char)(k & 0xFF);
    
    local_f1 = f1 * 2.0f;
    local_f2 = f2 + f3;
    local_f3 = f1 / f2;
    
    local_d1 = d1 * 2.0;
    local_d2 = d3 - d2;
    
    /* Block 1: Complex inline assembly with conflicting constraints */
block1:
    {
        int temp1 = i + j;
        int temp2 = k + l;
        int *ptr1 = &i;
        int *ptr2 = &j;
        
        /* Inline asm with memory and register constraints that conflict */
        asm volatile (
            "movl %[input1], %%eax\n\t"
            "addl %[input2], %%eax\n\t"
            "movl %%eax, %[output1]\n\t"
            "imull %[input3], %%eax\n\t"
            "clc\n\t"
            : [output1] "=m" (*ptr1), "=&a" (temp1)
            : [input1] "mr" (i), [input2] "mr" (j), [input3] "r" (k)
            : "rax", "rbx", "rcx", "rdx", "cc", "memory"
        );
        
        /* Force the variable to stay live */
        i = temp1 + 1;
    }
    
    /* Block 2: More conflicts with different data types */
block2:
    {
        double temp_d;
        float temp_f;
        long temp_l;
        
        /* Mixed type constraints causing mode mismatches */
        asm volatile (
            "cvtsi2sd %[int_in], %%xmm0\n\t"
            "addsd %[dbl_in], %%xmm0\n\t"
            "movsd %%xmm0, %[dbl_out]\n\t"
            "cvttsd2si %%xmm0, %%eax\n\t"
            : [dbl_out] "=m" (local_d1), "=&a" (temp_l)
            : [int_in] "r" (i), [dbl_in] "xm" (reg_var3)
            : "xmm0", "xmm1", "xmm2", "rax", "cc"
        );
        
        /* Use explicit register variable that gets clobbered */
        asm volatile (
            "movq %[regvar], %%rax\n\t"
            "addq $1, %%rax\n\t"
            "movq %%rax, %[out]\n\t"
            : [out] "=rm" (temp_l)
            : [regvar] "r" (reg_var1)
            : "rax", "rbx", "rcx", "rdx", "r12", "cc"
        );
        
        reg_var1 = temp_l;
    }
    
    /* Block 3: Multiple outputs with early clobber */
block3:
    {
        int out1, out2, out3;
        
        /* '&' means early clobber - can't share registers with inputs */
        asm volatile (
            "movl %[in1], %%ebx\n\t"
            "leal (%%ebx, %[in2]), %%ecx\n\t"
            "movl %%ecx, %[out1]\n\t"
            "imull %%ebx, %%ecx\n\t"
            "movl %%ecx, %[out2]\n\t"
            "addl $0x1234, %%ecx\n\t"
            "movl %%ecx, %[out3]\n\t"
            : [out1] "=&r" (out1), [out2] "=&r" (out2), [out3] "=r" (out3)
            : [in1] "r" (m), [in2] "r" (n)
            : "rax", "rbx", "rcx", "rdx", "cc"
        );
        
        m = out1;
        n = out2;
        o = out3;
    }
    
    /* Block 4: Address taken and used in multiple ways */
block4:
    {
        int array[10];
        int *ptr_array[5];
        int idx = i & 0x7;
        
        for (int x = 0; x < 10; x++) {
            array[x] = x * i + j;
        }
        
        for (int x = 0; x < 5; x++) {
            ptr_array[x] = &array[x * 2];
        }
        
        /* Complex addressing with multiple memory constraints */
        asm volatile (
            "movl (%[base], %[index], 4), %%eax\n\t"
            "addl %[addend], %%eax\n\t"
            "movl %%eax, (%[dest])\n\t"
            "movl %%eax, %[regout]\n\t"
            : [regout] "=r" (p), "=m" (*ptr_array[idx])
            : [base] "r" (array), [index] "r" (idx), 
              [addend] "r" (k), [dest] "r" (ptr_array[idx])
            : "rax", "rbx", "rcx", "memory", "cc"
        );
    }
    
    /* Block 5: Floating point and integer mix */
block5:
    {
        double dtemp;
        int itemp;
        float ftemp;
        
        /* Mixed precision operations */
        asm volatile (
            "cvtsi2sd %[intval], %%xmm0\n\t"
            "mulsd %[dblval], %%xmm0\n\t"
            "cvtsd2si %%xmm0, %%eax\n\t"
            "cvtsi2ss %%eax, %%xmm1\n\t"
            "mulss %[fltval], %%xmm1\n\t"
            : "=a" (itemp), "=x" (dtemp), "=x" (ftemp)
            : [intval] "r" (i), [dblval] "xm" (local_d1), 
              [fltval] "xm" (local_f1)
            : "xmm0", "xmm1", "xmm2", "xmm3", "cc"
        );
        
        local_d1 = dtemp;
        local_f1 = ftemp;
        q = itemp;
    }
    
    /* Block 6: Many live values across goto */
    if (i > 0) {
        goto block7;
    }
    
block6:
    {
        /* Create many overlapping live ranges */
        int t1 = i + j + k + l;
        int t2 = m + n + o + p;
        int t3 = q + r;
        
        asm volatile (
            "movl %[a], %%eax\n\t"
            "addl %[b], %%eax\n\t"
            "addl %[c], %%eax\n\t"
            "movl %%eax, %[out]\n\t"
            : [out] "=rm" (result)
            : [a] "r" (t1), [b] "r" (t2), [c] "r" (t3)
            : "rax", "rbx", "rcx", "rdx", "r8", "r9", "r10", "r11", "cc"
        );
        
        goto final;
    }
    
block7:
    {
        /* Different path with different live values */
        long t4 = reg_var1 + i + j;
        double t5 = reg_var3 + local_d1 + local_d2;
        
        asm volatile (
            "movq %[in1], %%rax\n\t"
            "cvtsi2sdq %%rax, %%xmm0\n\t"
            "addsd %[in2], %%xmm0\n\t"
            "cvtsd2siq %%xmm0, %%rax\n\t"
            "movq %%rax, %[out]\n\t"
            : [out] "=rm" (reg_var1)
            : [in1] "r" (t4), [in2] "xm" (t5)
            : "rax", "rbx", "rcx", "xmm0", "xmm1", "xmm2", "cc"
        );
        
        goto block6;
    }
    
final:
    /* Aggregate results to prevent optimization */
    volatile int checksum = 0;
    checksum += i + j + k + l + m + n + o + p + q + r;
    checksum += (int)reg_var1 + reg_var2;
    checksum += (int)local_f1 + (int)local_f2 + (int)local_f3;
    checksum += (int)local_d1 + (int)local_d2;
    checksum += s1 + s2 + s3 + c1 + c2 + c3;
    checksum += result;
    
    printf("Result: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
