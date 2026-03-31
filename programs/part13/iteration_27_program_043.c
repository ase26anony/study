/* Test program to trigger reload.cc push_reload uncovered block */
#include <stdio.h>
#include <stdlib.h>

/* Force specific register usage to create conflicts */
register long reg_var1 asm ("r12");
register int reg_var2 asm ("r13");
register double reg_var3 asm ("xmm8");

/* Function to create register pressure */
void create_register_pressure() {
    /* Many volatile variables to prevent optimization */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile long v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float v11 = 11.0f, v12 = 12.0f, v13 = 13.0f;
    volatile double v14 = 14.0, v15 = 15.0, v16 = 16.0;
    
    /* Force computations to keep variables live */
    v1 = v2 + v3;
    v4 = v5 * v6;
    v7 = v8 - v9;
    v11 = v12 * v13;
    v14 = v15 + v16;
    
    /* Complex inline assembly with conflicting constraints */
    asm volatile (
        "mov %[in1], %%rax\n\t"
        "add %[in2], %%rax\n\t"
        "mov %%rax, %[out1]\n\t"
        "mov %[in3], %%rbx\n\t"
        "imul %[in4], %%rbx\n\t"
        "mov %%rbx, %[out2]"
        : [out1] "=r" (v1), [out2] "=r" (v4)
        : [in1] "r" (v2), [in2] "r" (v3), 
          [in3] "r" (v5), [in4] "r" (v6)
        : "rax", "rbx", "rcx", "rdx", "memory", "cc"
    );
}

int main() {
    volatile int checksum = 0;
    
    /* Block 1: Initialize and create initial pressure */
    block1: {
        volatile int a = 100, b = 200, c = 300;
        volatile long d = 400, e = 500;
        volatile float f = 600.0f, g = 700.0f;
        volatile double h = 800.0;
        
        /* Explicit register variables */
        reg_var1 = 12345;
        reg_var2 = 67890;
        reg_var3 = 3.14159;
        
        /* Inline assembly that clobbers our register variables */
        asm volatile (
            "mov %1, %%r12\n\t"      /* Clobber reg_var1's register */
            "add $1, %%r12\n\t"
            "mov %%r12, %0\n\t"
            "pxor %%xmm8, %%xmm8\n\t" /* Clobber reg_var3's register */
            : "=r" (a)
            : "r" (b)
            : "r12", "xmm8", "memory", "cc"
        );
        
        checksum += a + b + c;
        goto block2;
    }
    
    /* Block 2: More complex constraints */
    block2: {
        volatile short s1 = 10, s2 = 20;
        volatile char ch1 = 'A', ch2 = 'B';
        volatile int i1 = 1000, i2 = 2000;
        volatile long l1 = 3000, l2 = 4000;
        
        /* Mixed data types in same asm statement */
        asm volatile (
            "movzwl %w[in1], %%eax\n\t"    /* Zero extend short to long */
            "add %[in2], %%eax\n\t"
            "mov %%eax, %[out1]\n\t"
            "movsbl %b[in3], %%ebx\n\t"    /* Sign extend char to long */
            "add %[in4], %%ebx\n\t"
            "mov %%ebx, %[out2]"
            : [out1] "=r" (i1), [out2] "=r" (i2)
            : [in1] "r" (s1), [in2] "r" (i1),
              [in3] "r" (ch1), [in4] "r" (i2)
            : "rax", "rbx", "rcx", "memory", "cc"
        );
        
        checksum += i1 + i2 + l1 + l2;
        goto block3;
    }
    
    /* Block 3: Addressing mode conflicts */
    block3: {
        volatile int arr[10] = {1,2,3,4,5,6,7,8,9,10};
        volatile int idx = 5;
        volatile int val1 = 100, val2 = 200;
        int *ptr = (int*)&arr[0];
        
        /* Taking address and using in memory constraint */
        asm volatile (
            "mov %[index], %%ecx\n\t"
            "mov (%[base], %%ecx, 4), %%eax\n\t"  /* Memory operand */
            "add %[val], %%eax\n\t"
            "mov %%eax, %[result]"
            : [result] "=r" (val1)
            : [base] "r" (ptr), [index] "r" (idx), 
              [val] "r" (val2), "m" (*ptr)        /* Memory constraint */
            : "rax", "rcx", "memory", "cc"
        );
        
        checksum += val1 + arr[idx];
        goto block4;
    }
    
    /* Block 4: Output operand with earlyclobber and multiple clobbers */
    block4: {
        volatile int x = 42, y = 73, z = 99;
        volatile long big1 = 1000000, big2 = 2000000;
        
        /* Earlyclobber constraint to force separate register */
        asm volatile (
            "mov %[in1], %%rax\n\t"
            "imul %[in2], %%rax\n\t"
            "mov %%rax, %[out1]\n\t"
            "mov %[in3], %%rbx\n\t"
            "add %%rax, %%rbx\n\t"
            "mov %%rbx, %[out2]"
            : [out1] "=&r" (x), [out2] "=r" (y)  /* & = earlyclobber */
            : [in1] "r" (big1), [in2] "r" (big2), [in3] "r" (z)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "memory", "cc"
        );
        
        checksum += x + y + z;
        goto block5;
    }
    
    /* Block 5: Floating point mixed with integer */
    block5: {
        volatile double d1 = 1.5, d2 = 2.5, d3 = 3.5;
        volatile float f1 = 4.5f, f2 = 5.5f;
        volatile int i1 = 100, i2 = 200;
        
        asm volatile (
            "addsd %[din1], %[din2]\n\t"
            "movsd %[din2], %[dout]\n\t"
            "cvttsd2si %[din2], %%eax\n\t"
            "add %[iin], %%eax\n\t"
            "mov %%eax, %[iout]"
            : [dout] "=x" (d3), [iout] "=r" (i2)
            : [din1] "x" (d1), [din2] "x" (d2), 
              [iin] "r" (i1)
            : "rax", "xmm0", "xmm1", "xmm2", "memory", "cc"
        );
        
        checksum += (int)d3 + i2;
        create_register_pressure();
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
