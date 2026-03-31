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
    /* Local variables with mixed types to create mode conflicts */
    char c1 = 'A', c2 = 'B', c3 = 'C';
    short s1 = 100, s2 = 200, s3 = 300;
    int i1 = 1000, i2 = 2000, i3 = 3000, i4 = 4000, i5 = 5000;
    long l1 = 6000L, l2 = 7000L, l3 = 8000L;
    float fl1 = 1.5f, fl2 = 2.5f, fl3 = 3.5f;
    double d1 = 10.5, d2 = 20.5, d3 = 30.5;
    
    /* Force many values to be live */
    volatile int checksum = 0;
    
    /* Block 1: Complex inline assembly with conflicting constraints */
block1:
    {
        int temp1 = i1 + v1;
        int temp2 = i2 + v2;
        int temp3 = i3 + v3;
        
        /* Inline asm with multiple outputs, early clobber, and memory operands */
        asm volatile (
            "mov %[in1], %%eax\n\t"
            "add %[in2], %%eax\n\t"
            "mov %%eax, %[out1]\n\t"
            "imul %[in3], %%eax\n\t"
            "mov %%eax, %[out2]\n\t"
            "lea (%[mem1]), %%ebx\n\t"
            "add %%ebx, %[out3]"
            : [out1] "=r" (i1), [out2] "=&r" (i2), [out3] "+m" (v4)
            : [in1] "r" (temp1), [in2] "r" (temp2), [in3] "r" (temp3),
              [mem1] "m" (v5)
            : "rax", "rbx", "rcx", "memory", "cc"
        );
        
        checksum += i1 + i2 + v4;
        goto block2;
    }
    
    /* Unreachable code to create more control flow complexity */
    {
        int dummy = 0;
        dummy++;
    }
    
block2:
    {
        /* Use explicit register variables that will be clobbered */
        int use_r12 = r12_var;
        int use_r13 = r13_var;
        
        /* Inline asm that clobbers pinned registers */
        asm volatile (
            "mov %0, %%r12\n\t"
            "mov %1, %%r13\n\t"
            "add %%r13, %%r12\n\t"
            "mov %%r12, %2\n\t"
            "xor %%rcx, %%rcx\n\t"
            "mov %%rcx, %%r14"
            : "=m" (v6)
            : "r" (use_r12), "r" (use_r13)
            : "r12", "r13", "r14", "rcx", "memory", "cc"
        );
        
        /* Force reload of r12_var by using it after clobber */
        r12_var = v6 + 1;
        
        /* Mixed type operations to cause mode reloads */
        double dtemp = d1 + f1;
        float ftemp = fl1 + f5;
        
        asm volatile (
            "cvtsd2ss %[din], %%xmm0\n\t"
            "cvtss2sd %[fin], %%xmm1\n\t"
            "addsd %%xmm1, %%xmm0\n\t"
            "movsd %%xmm0, %[dout]\n\t"
            "cvttsd2si %%xmm0, %%eax\n\t"
            "mov %%eax, %[iout]"
            : [dout] "=m" (d2), [iout] "=m" (i4)
            : [din] "m" (dtemp), [fin] "m" (ftemp)
            : "xmm0", "xmm1", "rax", "memory", "cc"
        );
        
        checksum += v6 + (int)d2 + i4;
        goto block3;
    }
    
block3:
    {
        /* Create addressing mode conflicts */
        int* ptr1 = &i5;
        int* ptr2 = &v7;
        
        /* Use address in memory constraint while also using value in register */
        asm volatile (
            "mov (%[addr]), %%eax\n\t"
            "add %[val], %%eax\n\t"
            "mov %%eax, (%[addr])\n\t"
            "mov %%eax, %[out]"
            : [out] "=r" (l1)
            : [addr] "r" (ptr1), [val] "r" (v8)
            : "rax", "memory", "cc"
        );
        
        /* Another asm with complex constraints */
        long ltemp = l2 + l3;
        
        asm volatile (
            "mov %[in1], %%rax\n\t"
            "add %[in2], %%rax\n\t"
            "mov %%rax, %[out1]\n\t"
            "mov %[in3], %%rbx\n\t"
            "sub %%rbx, %%rax\n\t"
            "mov %%rax, %[out2]"
            : [out1] "=&r" (l2), [out2] "=m" (v9)
            : [in1] "r" (ltemp), [in2] "r" (v10), [in3] "m" (v1)
            : "rax", "rbx", "memory", "cc"
        );
        
        checksum += l1 + l2 + v9;
        goto block4;
    }
    
block4:
    {
        /* Force many live values across asm block */
        int live1 = c1 + c2 + c3;
        int live2 = s1 + s2 + s3;
        int live3 = fl2 + fl3;
        double live4 = d3 + f2 + f3 + f4;
        
        /* Large clobber list to force many spills */
        asm volatile (
            "mov %[a], %%eax\n\t"
            "add %[b], %%eax\n\t"
            "mov %%eax, %[x]\n\t"
            "mov %[c], %%ebx\n\t"
            "imul %%ebx, %%eax\n\t"
            "mov %%eax, %[y]\n\t"
            "mov %[d], %%ecx\n\t"
            "add %%ecx, %%eax\n\t"
            "mov %%eax, %[z]"
            : [x] "=m" (v1), [y] "=m" (v2), [z] "=m" (v3)
            : [a] "r" (live1), [b] "r" (live2), 
              [c] "r" (live3), [d] "r" ((int)live4)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "memory", "cc"
        );
        
        /* Use all variables to keep them live */
        checksum += v1 + v2 + v3 + live1 + live2 + (int)live3 + (int)live4;
        
        /* Final computation using all volatile variables */
        checksum += f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8;
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0;
}
