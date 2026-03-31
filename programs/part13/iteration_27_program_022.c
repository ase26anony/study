/* Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force compiler to generate reloads through complex inline assembly */
int main(void) {
    /* Volatile variables to prevent optimization and force register pressure */
    volatile int v1 = 0x12345678;
    volatile int v2 = 0x9ABCDEF0;
    volatile int v3 = 0x11111111;
    volatile int v4 = 0x22222222;
    volatile int v5 = 0x33333333;
    volatile int v6 = 0x44444444;
    volatile int v7 = 0x55555555;
    volatile int v8 = 0x66666666;
    volatile int v9 = 0x77777777;
    volatile int v10 = 0x88888888;
    
    /* Non-volatile variables for additional register pressure */
    int nv1 = 0xAAAAAAAA;
    int nv2 = 0xBBBBBBBB;
    int nv3 = 0xCCCCCCCC;
    int nv4 = 0xDDDDDDDD;
    int nv5 = 0xEEEEEEEE;
    
    /* Floating point variables to increase pressure on FP registers */
    volatile double f1 = 3.14159;
    volatile double f2 = 2.71828;
    volatile float f3 = 1.41421f;
    volatile float f4 = 1.73205f;
    
    /* Explicit register variables to pin specific registers */
    register int r12_var asm ("r12") = 0x1234;
    register int r13_var asm ("r13") = 0x5678;
    register int r14_var asm ("r14") = 0x9ABC;
    register int r15_var asm ("r15") = 0xDEF0;
    
    /* Variables with different sizes/types for mode mismatches */
    volatile char c1 = 'A';
    volatile short s1 = 0x1234;
    volatile long long ll1 = 0x1122334455667788ULL;
    
    /* Pointer variables for addressing mode conflicts */
    int *ptr1 = &nv1;
    int *ptr2 = &nv2;
    
    /* Force many live values across basic blocks */
    int result = 0;
    
    /* Block 1: Create initial register pressure */
block1:
    {
        /* Complex inline assembly with conflicting constraints */
        asm volatile (
            "mov %[in1], %%eax\n\t"
            "add %[in2], %%eax\n\t"
            "mov %%eax, %[out1]\n\t"
            "imul %[in3], %%eax\n\t"
            "mov %%eax, %[out2]\n\t"
            : [out1] "=r" (nv1), [out2] "=r" (nv2)
            : [in1] "r" (v1), [in2] "r" (v2), [in3] "r" (v3)
            : "rax", "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        /* Use explicit register variables to keep them live */
        r12_var += nv1;
        r13_var += nv2;
    }
    
    /* Block 2: More complex assembly with memory operands */
block2:
    {
        int temp1, temp2;
        
        /* Assembly with memory constraint and register constraint conflict */
        asm volatile (
            "movl %[mem1], %%ebx\n\t"
            "addl %%ebx, %[reg1]\n\t"
            "movl %[reg1], %[mem2]\n\t"
            : [reg1] "+&r" (temp1), [mem2] "=m" (v4)
            : [mem1] "m" (v3)
            : "rbx", "rcx", "memory", "cc"
        );
        
        /* Mixed size operands */
        asm volatile (
            "movb %[char1], %%al\n\t"
            "movw %[short1], %%bx\n\t"
            "addw %%bx, %%ax\n\t"
            "movb %%al, %[outc]\n\t"
            : [outc] "=m" (c1)
            : [char1] "m" (c1), [short1] "m" (s1)
            : "rax", "rbx", "cc"
        );
        
        v5 = temp1 + r12_var;
    }
    
    /* Block 3: Force reloads with clobbered explicit registers */
block3:
    {
        /* This assembly clobbers registers we've pinned variables to */
        asm volatile (
            "mov $0x1234, %%r12\n\t"
            "mov $0x5678, %%r13\n\t"
            "add %%r12, %%r13\n\t"
            "mov %%r13, %[out]\n\t"
            : [out] "=m" (v6)
            :
            : "r12", "r13", "r14", "r15", "memory", "cc"
        );
        
        /* After clobbering, use the register variables - forcing reloads */
        nv3 = r12_var + r13_var + r14_var + r15_var;
    }
    
    /* Block 4: Complex addressing and multiple outputs */
block4:
    {
        long long temp_ll;
        
        /* Multiple output operands with earlyclobber */
        asm volatile (
            "movq %[in1], %%rax\n\t"
            "movq %[in2], %%rbx\n\t"
            "addq %%rbx, %%rax\n\t"
            "movq %%rax, %[out1]\n\t"
            "imulq $2, %%rax\n\t"
            "movq %%rax, %[out2]\n\t"
            : [out1] "=&r" (temp_ll), [out2] "=m" (ll1)
            : [in1] "r" (ll1), [in2] "m" (v7)
            : "rax", "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        /* Use pointer with addressing */
        *ptr1 = temp_ll & 0xFFFFFFFF;
        *ptr2 = (temp_ll >> 32) & 0xFFFFFFFF;
    }
    
    /* Block 5: Floating point mixed with integer */
block5:
    {
        double ftemp;
        int itemp;
        
        /* Mixed type constraints */
        asm volatile (
            "movsd %[fin1], %%xmm0\n\t"
            "movsd %[fin2], %%xmm1\n\t"
            "addsd %%xmm1, %%xmm0\n\t"
            "cvttsd2si %%xmm0, %%eax\n\t"
            "mov %%eax, %[iout]\n\t"
            "movsd %%xmm0, %[fout]\n\t"
            : [iout] "=r" (itemp), [fout] "=m" (ftemp)
            : [fin1] "m" (f1), [fin2] "m" (f2)
            : "rax", "xmm0", "xmm1", "xmm2", "memory", "cc"
        );
        
        v8 = itemp;
        f3 = ftemp;
    }
    
    /* Block 6: Final computations with all live values */
block6:
    {
        /* Use all variables to keep them live */
        result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        result += nv1 + nv2 + nv3 + nv4 + nv5;
        result += r12_var + r13_var + r14_var + r15_var;
        result += c1 + s1 + (ll1 & 0xFFFFFFFF);
        result += (int)f1 + (int)f2 + (int)f3 + (int)f4;
        result += *ptr1 + *ptr2;
    }
    
    /* Prevent dead code elimination */
    volatile int checksum = result;
    printf("Result: %d\n", checksum);
    
    return 0;
}
