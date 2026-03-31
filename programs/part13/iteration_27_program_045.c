/* Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force spills by using many volatile variables */
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
    /* Local variables with different types and sizes */
    char c1 = 'a', c2 = 'b', c3 = 'c';
    short s1 = 100, s2 = 200, s3 = 300;
    int i1 = 1000, i2 = 2000, i3 = 3000, i4 = 4000;
    long l1 = 50000L, l2 = 60000L;
    float f_local1 = 1.5, f_local2 = 2.5;
    double d_local1 = 3.14159, d_local2 = 2.71828;
    
    /* Pointer variables for addressing mode conflicts */
    int *ptr1 = &i1, *ptr2 = &i2, *ptr3 = &i3;
    volatile int *vol_ptr = &v1;
    
    /* Complex live value setup */
    int sum = 0;
    
    /* Block 1: Create many live values */
block1:
    v1 = v2 + v3;
    v4 = v5 * v6;
    f1 = f2 + f3;
    f4 = f5 * f6;
    
    /* Inline assembly with conflicting constraints */
    /* Using memory operand and register operand for same variable */
    asm volatile (
        "movl %[mem1], %%eax\n\t"
        "addl %%eax, %[reg1]\n\t"
        "movl %[reg1], %[mem1]\n\t"
        : [mem1] "+m" (i1), [reg1] "+&r" (i2)
        : 
        : "rax", "rbx", "rcx", "rdx", "memory", "cc"
    );
    
    /* Force use of pinned registers */
    r12_var = i1 + i2;
    r13_var = r12_var * 2;
    
    /* Block 2: More complex assembly with clobbers */
block2:
    /* Mixed type operands in same assembly */
    asm volatile (
        "mov %[char1], %%al\n\t"
        "mov %[short1], %%bx\n\t"
        "addw %%bx, %%ax\n\t"
        "mov %%ax, %[int1]\n\t"
        : [int1] "=r" (i3)
        : [char1] "r" ((int)c1), [short1] "r" ((int)s1)
        : "rax", "rbx", "rcx", "cc"
    );
    
    /* Use volatile variables to prevent optimization */
    v7 = v8 + v9;
    v10 = v1 * v2;
    
    /* Block 3: Assembly with output constraints that conflict */
block3:
    {
        int tmp1, tmp2, tmp3;
        /* Multiple outputs with early clobber */
        asm volatile (
            "movl $100, %0\n\t"
            "movl $200, %1\n\t"
            "addl %0, %1\n\t"
            "movl %1, %2\n\t"
            : "=&r" (tmp1), "=&r" (tmp2), "=r" (tmp3)
            :
            : "cc"
        );
        sum += tmp1 + tmp2 + tmp3;
    }
    
    /* Block 4: Force addressing mode conflicts */
block4:
    {
        int addr_conflict;
        /* Taking address and using in memory constraint */
        asm volatile (
            "movl (%[ptr]), %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, (%[ptr])\n\t"
            "movl %%eax, %[out]\n\t"
            : [out] "=r" (addr_conflict)
            : [ptr] "r" (ptr1), "m" (*ptr1)
            : "rax", "memory", "cc"
        );
        sum += addr_conflict;
    }
    
    /* Block 5: More register pressure */
block5:
    {
        double d1, d2;
        float f1_local, f2_local;
        
        /* Mixed floating point and integer */
        asm volatile (
            "movsd %[din1], %%xmm0\n\t"
            "movsd %[din2], %%xmm1\n\t"
            "addsd %%xmm1, %%xmm0\n\t"
            "movsd %%xmm0, %[dout1]\n\t"
            "cvtsd2ss %%xmm0, %%xmm2\n\t"
            "movss %%xmm2, %[fout1]\n\t"
            : [dout1] "=m" (d1), [fout1] "=m" (f1_local)
            : [din1] "m" (d_local1), [din2] "m" (d_local2)
            : "xmm0", "xmm1", "xmm2", "memory"
        );
        
        sum += (int)d1 + (int)f1_local;
    }
    
    /* Block 6: Long clobber list to force many spills */
block6:
    {
        int a = 100, b = 200, c = 300;
        asm volatile (
            "mov %[a], %%eax\n\t"
            "add %[b], %%eax\n\t"
            "imul %[c], %%eax\n\t"
            "mov %%eax, %[a]\n\t"
            : [a] "+r" (a)
            : [b] "r" (b), [c] "r" (c)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", 
              "r11", "memory", "cc"
        );
        sum += a;
    }
    
    /* Block 7: Use explicit register variables that get clobbered */
block7:
    {
        int saved_r12 = r12_var;
        int saved_r13 = r13_var;
        
        /* Assembly that clobbers pinned registers */
        asm volatile (
            "mov $0x1111, %%r12\n\t"
            "mov $0x2222, %%r13\n\t"
            "add %%r12, %%r13\n\t"
            "mov %%r13, %[out]\n\t"
            : [out] "=r" (i4)
            :
            : "r12", "r13", "cc"
        );
        
        /* Restore (compiler may need reloads) */
        r12_var = saved_r12;
        r13_var = saved_r13;
        sum += i4;
    }
    
    /* Block 8: Complex control flow with live values */
block8:
    v1 = sum;
    v2 = v1 * 2;
    
    if (v1 > 1000) {
        goto block9;
    } else {
        goto block10;
    }
    
block9:
    {
        /* More assembly with constraints */
        long result;
        asm volatile (
            "mov %[in1], %%rax\n\t"
            "imul %[in2], %%rax\n\t"
            "mov %%rax, %[out]\n\t"
            : [out] "=r" (result)
            : [in1] "r" (l1), [in2] "r" (l2)
            : "rax", "rdx", "cc"
        );
        sum += (int)result;
        goto final;
    }
    
block10:
    {
        /* Alternative path */
        asm volatile (
            "mov $999, %%eax\n\t"
            "mov %%eax, %[out]\n\t"
            : [out] "=r" (sum)
            :
            : "rax", "cc"
        );
        goto final;
    }
    
final:
    /* Use all variables to keep them live */
    sum += c1 + c2 + c3;
    sum += s1 + s2 + s3;
    sum += i1 + i2 + i3 + i4;
    sum += l1 + l2;
    sum += (int)f_local1 + (int)f_local2;
    sum += (int)d_local1 + (int)d_local2;
    sum += r12_var + r13_var + r14_var + r15_var;
    
    /* Final volatile store to prevent dead code elimination */
    volatile int final_sum = sum;
    
    printf("Result: %d\n", final_sum);
    
    return 0;
}
