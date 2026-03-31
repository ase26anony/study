/* Test program to trigger reload.cc uncovered block */
#include <stdio.h>

/* Force many live values across complex control flow */
volatile int v1 = 12345;
volatile int v2 = 67890;
volatile int v3 = 11111;
volatile int v4 = 22222;
volatile int v5 = 33333;
volatile int v6 = 44444;
volatile int v7 = 55555;
volatile int v8 = 66666;
volatile float f1 = 1.234f;
volatile float f2 = 5.678f;
volatile double d1 = 9.876;
volatile double d2 = 5.432;

/* Explicit register variables to pin registers */
register int reg_var1 asm ("r12");
register int reg_var2 asm ("r13");
register int reg_var3 asm ("r14");

int main(void) {
    int result = 0;
    int a, b, c, d, e, f, g, h;
    int *ptr1, *ptr2;
    float f3, f4;
    double d3, d4;
    char ch1, ch2;
    short s1, s2;
    
    /* Initialize many variables to create live ranges */
    a = v1 + 1;
    b = v2 * 2;
    c = v3 - 3;
    d = v4 / 4;
    e = v5 | 0xFF;
    f = v6 & 0x0F;
    g = v7 ^ 0xAA;
    h = v8 << 2;
    
    f3 = f1 * 2.0f;
    f4 = f2 / 2.0f;
    d3 = d1 + 1.0;
    d4 = d2 - 1.0;
    
    ch1 = (char)a;
    ch2 = (char)b;
    s1 = (short)c;
    s2 = (short)d;
    
    /* Initialize explicit register variables */
    reg_var1 = 0xDEADBEEF;
    reg_var2 = 0xCAFEBABE;
    reg_var3 = 0x12345678;
    
    ptr1 = &a;
    ptr2 = &b;
    
    /* Block 1: Complex inline assembly with conflicting constraints */
block1:
    {
        int tmp1, tmp2, tmp3;
        /* Force address computation and register conflicts */
        asm volatile (
            "movl %[input1], %%eax\n\t"
            "addl %[input2], %%eax\n\t"
            "movl %%eax, %[output1]\n\t"
            "imull %[input3], %%eax\n\t"
            "movl %%eax, %[output2]\n\t"
            : [output1] "=r" (tmp1), [output2] "=r" (tmp2)
            : [input1] "r" (a), [input2] "r" (b), [input3] "rm" (c)
            : "rax", "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        /* Modify live variables to keep them active */
        a = tmp1 + v1;
        b = tmp2 + v2;
        
        /* Another asm with memory operand and register constraints */
        asm volatile (
            "movq %[addr], %%rbx\n\t"
            "movl (%%rbx), %%ecx\n\t"
            "addl %%ecx, %[sum]\n\t"
            : [sum] "+r" (result)
            : [addr] "r" (ptr1)
            : "rbx", "rcx", "memory"
        );
    }
    
    /* Block 2: Different data types and modes */
block2:
    {
        short tmp_s;
        char tmp_c;
        
        /* Mixed size operands */
        asm volatile (
            "movw %[in1], %%ax\n\t"
            "movb %[in2], %%cl\n\t"
            "addb %%cl, %%al\n\t"
            "movw %%ax, %[out1]\n\t"
            : [out1] "=r" (tmp_s)
            : [in1] "r" (s1), [in2] "r" (ch1)
            : "rax", "rcx", "cc"
        );
        
        s1 = tmp_s + 1;
        ch1 = tmp_s & 0xFF;
        
        /* Force floating point reloads */
        asm volatile (
            "movss %[fin], %%xmm0\n\t"
            "addss %[fin2], %%xmm0\n\t"
            "movss %%xmm0, %[fout]\n\t"
            : [fout] "=x" (f3)
            : [fin] "x" (f3), [fin2] "x" (f4)
            : "xmm0", "xmm1"
        );
    }
    
    /* Block 3: Explicit register variable conflicts */
block3:
    {
        int local1, local2;
        
        /* Clobber registers used by explicit register variables */
        asm volatile (
            "movl $0x11111111, %%r12d\n\t"
            "movl $0x22222222, %%r13d\n\t"
            "movl $0x33333333, %%r14d\n\t"
            "addl %%r12d, %%r13d\n\t"
            "movl %%r13d, %[out1]\n\t"
            : [out1] "=r" (local1)
            :
            : "r12", "r13", "r14", "cc"
        );
        
        /* Use the explicit register variables after clobber */
        local2 = reg_var1 + reg_var2 + reg_var3;
        result += local1 + local2;
    }
    
    /* Block 4: Complex addressing modes */
block4:
    {
        int index = 2;
        int array[10] = {1,2,3,4,5,6,7,8,9,10};
        
        /* Force address computation with index */
        asm volatile (
            "movl %[idx], %%ecx\n\t"
            "leaq %[arr], %%rbx\n\t"
            "movl (%%rbx,%%rcx,4), %%eax\n\t"
            "addl %%eax, %[res]\n\t"
            : [res] "+r" (result)
            : [arr] "r" (array), [idx] "r" (index)
            : "rax", "rbx", "rcx", "memory"
        );
        
        /* Another with output memory operand */
        asm volatile (
            "movl %[val], (%%rbx)\n\t"
            : "=m" (array[3])
            : [val] "r" (result), "b" (&array[3])
            : "memory"
        );
    }
    
    /* Block 5: Many live values across goto */
block5:
    {
        /* Use all variables to keep them live */
        int sum = a + b + c + d + e + f + g + h;
        sum += (int)f3 + (int)f4 + (int)d3 + (int)d4;
        sum += ch1 + ch2 + s1 + s2;
        
        /* Final complex asm with many constraints */
        asm volatile (
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "movl %[in3], %%ebx\n\t"
            "subl %[in4], %%ebx\n\t"
            "movl %%ebx, %[out2]\n\t"
            : [out1] "=&r" (a), [out2] "=&r" (b)
            : [in1] "r" (sum), [in2] "r" (result),
              [in3] "rm" (reg_var1), [in4] "rm" (reg_var2)
            : "rax", "rbx", "rcx", "rdx", "r8", "r9", "r10", "r11",
              "memory", "cc"
        );
        
        result = a + b;
    }
    
    /* Create control flow that forces live value spans */
    if (result > 1000)
        goto block2;
    
    if (result < 500)
        goto block4;
    
    /* Final checksum to prevent optimization */
    volatile int checksum = result + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    printf("Result: %d, Checksum: %d\n", result, checksum);
    
    return result != 0;
}
