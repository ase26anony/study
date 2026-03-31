/* Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force spills with many volatile variables */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
volatile double f1 = 1.1, f2 = 2.2, f3 = 3.3, f4 = 4.4;
volatile float f5 = 5.5, f6 = 6.6;

/* Explicit register variables to pin registers */
register int r12_var asm ("r12") = 100;
register int r13_var asm ("r13") = 200;
register int r14_var asm ("r14") = 300;
register int r15_var asm ("r15") = 400;

int main(void) {
    /* Local variables with mixed types to create mode conflicts */
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    int i1 = 10000, i2 = 20000, i3 = 30000, i4 = 40000;
    long l1 = 50000L, l2 = 60000L;
    float fl1 = 7.7f, fl2 = 8.8f;
    double d1 = 9.9, d2 = 10.10;
    
    /* Force many live values by using all variables */
    int sum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    sum += (int)f1 + (int)f2 + (int)f3 + (int)f4;
    sum += (int)f5 + (int)f6;
    
    /* Block 1: Complex inline assembly with conflicting constraints */
block1:
    {
        int temp1 = i1 + i2;
        int temp2 = i3 + i4;
        int *ptr1 = &i1;
        int *ptr2 = &i2;
        
        /* Inline assembly with memory and register constraints that conflict */
        asm volatile (
            "movl %[input1], %%eax\n\t"
            "addl %[input2], %%eax\n\t"
            "movl %%eax, %[output]\n\t"
            : [output] "=m" (*ptr1), "=&r" (temp1)  /* Output: memory and earlyclobber reg */
            : [input1] "r" (temp2), [input2] "m" (l1)  /* Input: reg and memory */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
              "memory", "cc"
        );
        
        /* Use results to keep them live */
        sum += temp1 + *ptr1;
        goto block2;
    }

block2:
    {
        /* More complex assembly with different data types */
        short temp_s = s1;
        char temp_c = c1;
        
        /* Assembly with mixed-size operands */
        asm volatile (
            "movw %w[in1], %%ax\n\t"
            "addb %b[in2], %%al\n\t"
            "movw %%ax, %w[out1]\n\t"
            "movl %[in3], %%ebx\n\t"
            "addl %[in4], %%ebx\n\t"
            : [out1] "=r" (temp_s), "=r" (i1)  /* Two register outputs */
            : [in1] "r" (s2), [in2] "r" (c2),  /* Register inputs */
              [in3] "m" (r12_var), [in4] "m" (r13_var)  /* Memory inputs from pinned regs */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
              "r12", "r13", "memory", "cc"
        );
        
        sum += temp_s + i1 + temp_c;
        
        /* Force address computation */
        int *addr_array[4] = {&i1, &i2, &i3, &i4};
        goto block3;
    }

block3:
    {
        /* Floating point mixed with integer */
        double temp_d = d1;
        float temp_f = fl1;
        int temp_i = i2;
        
        /* Assembly using xmm registers */
        asm volatile (
            "movsd %[din], %%xmm0\n\t"
            "addsd %[din2], %%xmm0\n\t"
            "movsd %%xmm0, %[dout]\n\t"
            "movd %[iin], %%xmm1\n\t"
            "cvtsi2sd %%xmm1, %%xmm1\n\t"
            "addsd %%xmm1, %%xmm0\n\t"
            "movsd %%xmm0, %[dout2]\n\t"
            : [dout] "=m" (d2), [dout2] "=m" (temp_d)
            : [din] "m" (f1), [din2] "m" (f2), [iin] "r" (temp_i)
            : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "rax", "rbx", "rcx", "memory", "cc"
        );
        
        sum += (int)temp_d + (int)d2;
        goto block4;
    }

block4:
    {
        /* Very complex constraints with earlyclobber and multiple alternatives */
        int out1, out2, out3;
        int in1 = l1, in2 = l2, in3 = r14_var, in4 = r15_var;
        
        /* Multiple outputs with earlyclobber to force reloads */
        asm volatile (
            "movl %[a1], %%eax\n\t"
            "addl %[a2], %%eax\n\t"
            "movl %%eax, %[o1]\n\t"
            "movl %[a3], %%ebx\n\t"
            "subl %[a4], %%ebx\n\t"
            "movl %%ebx, %[o2]\n\t"
            "imull %%eax, %%ebx\n\t"
            "movl %%ebx, %[o3]\n\t"
            : [o1] "=&r" (out1), [o2] "=&r" (out2), [o3] "=r" (out3)
            : [a1] "r" (in1), [a2] "m" (v1), [a3] "r" (in2), [a4] "m" (v2)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
              "r12", "r13", "r14", "r15", "memory", "cc"
        );
        
        sum += out1 + out2 + out3;
        
        /* Force more register pressure */
        int arr[20];
        for (int j = 0; j < 20; j++) {
            arr[j] = sum + j;
            asm volatile ("" : "+r" (arr[j]));  /* Prevent optimization */
        }
        
        /* Final computation using all live values */
        volatile int checksum = sum + c1 + c2 + s1 + s2 + i1 + i2 + i3 + i4 
                              + l1 + l2 + (int)fl1 + (int)fl2 + (int)d1 
                              + r12_var + r13_var + r14_var + r15_var;
        
        printf("Checksum: %d\n", checksum);
    }
    
    return 0;
}
