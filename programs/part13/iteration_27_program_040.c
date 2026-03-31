/* Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force many live values across complex control flow */
int main(void) {
    /* Volatile variables to prevent optimization and force spills */
    volatile int v1 = 0x12345678;
    volatile int v2 = 0x9ABCDEF0;
    volatile int v3 = 0x13579BDF;
    volatile int v4 = 0x2468ACE0;
    volatile int v5 = 0x11111111;
    volatile int v6 = 0x22222222;
    volatile int v7 = 0x33333333;
    volatile int v8 = 0x44444444;
    
    /* Non-volatile variables with different types */
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    int i1 = 100000, i2 = 200000;
    long l1 = 300000L, l2 = 400000L;
    float f1 = 3.14f, f2 = 2.71f;
    double d1 = 3.14159, d2 = 2.71828;
    
    /* Explicit register variables - pin to specific registers */
    register int reg_var1 asm ("r12") = 0x55555555;
    register int reg_var2 asm ("r13") = 0x66666666;
    register int reg_var3 asm ("r14") = 0x77777777;
    
    /* Variables for address-taking */
    int addr_var1 = 0x88888888;
    int addr_var2 = 0x99999999;
    int *ptr1 = &addr_var1;
    int *ptr2 = &addr_var2;
    
    /* Complex arithmetic to create many live values */
    v1 = v2 + v3;
    v4 = v5 * v6;
    i1 = i2 - 50000;
    l1 = l2 >> 2;
    f1 = f2 * 2.0f;
    d1 = d2 / 2.0;
    
    /* Block 1: First inline asm with register constraints */
block1:
    {
        int temp1 = v1 + v2;
        int temp2 = v3 + v4;
        
        /* Complex inline asm with conflicting constraints */
        asm volatile (
            "mov %[in1], %%eax\n\t"
            "add %[in2], %%eax\n\t"
            "mov %%eax, %[out1]\n\t"
            "imul %[in3], %%ebx\n\t"
            "add %%ebx, %[out2]\n\t"
            : [out1] "=r" (temp1), [out2] "=r" (temp2)
            : [in1] "r" (v1), [in2] "r" (v2), [in3] "r" (v3)
            : "rax", "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        v5 = temp1;
        v6 = temp2;
        reg_var1 = reg_var1 + temp1;  /* Force r12 to be live */
    }
    
    /* Block 2: Memory addressing conflicts */
block2:
    {
        int mem_temp;
        
        /* Take address and use in memory constraint while also using register */
        asm volatile (
            "movl (%[addr]), %%eax\n\t"
            "addl $0x10, %%eax\n\t"
            "movl %%eax, (%[addr])\n\t"
            "movl %%eax, %[out]\n\t"
            : [out] "=r" (mem_temp)
            : [addr] "r" (ptr1), "m" (*ptr1)
            : "rax", "memory", "cc"
        );
        
        addr_var1 = mem_temp;
        
        /* Another asm that clobbers explicit register variables */
        asm volatile (
            "mov %0, %%r12\n\t"      /* Clobber reg_var1's register */
            "add $0x20, %%r12\n\t"
            "mov %%r12, %0\n\t"
            : "+r" (reg_var1)
            :
            : "r12", "cc"
        );
    }
    
    /* Block 3: Mixed data types and modes */
block3:
    {
        char char_result;
        short short_result;
        int int_result;
        
        /* Different sized operands in same asm */
        asm volatile (
            "mov %[in_char], %%al\n\t"
            "movsx %%al, %%eax\n\t"
            "add %[in_short], %%ax\n\t"
            "add %[in_int], %%eax\n\t"
            "mov %%al, %[out_char]\n\t"
            "mov %%ax, %[out_short]\n\t"
            "mov %%eax, %[out_int]\n\t"
            : [out_char] "=r" (char_result),
              [out_short] "=r" (short_result),
              [out_int] "=r" (int_result)
            : [in_char] "r" (c1),
              [in_short] "r" (s1),
              [in_int] "r" (i1)
            : "rax", "rdx", "cc"
        );
        
        c2 = char_result;
        s2 = short_result;
        i2 = int_result;
    }
    
    /* Block 4: More complex constraints with early clobber */
block4:
    {
        int out1, out2, out3;
        
        /* '&' means early clobber - can't share register with inputs */
        asm volatile (
            "mov %[in1], %%eax\n\t"
            "lea (%%eax, %%eax, 2), %%ebx\n\t"
            "mov %%ebx, %[out1]\n\t"
            "mov %[in2], %%ecx\n\t"
            "sub %%ecx, %%ebx\n\t"
            "mov %%ebx, %[out2]\n\t"
            "imul %%ecx, %%eax\n\t"
            "mov %%eax, %[out3]\n\t"
            : [out1] "=&r" (out1),  /* Early clobber */
              [out2] "=&r" (out2),  /* Early clobber */
              [out3] "=r" (out3)
            : [in1] "r" (v7),
              [in2] "r" (v8)
            : "rax", "rbx", "rcx", "rdx", "cc"
        );
        
        v7 = out1;
        v8 = out2;
        reg_var2 = out3;  /* Use another pinned register */
    }
    
    /* Block 5: Floating point mixed with integer */
block5:
    {
        float float_temp;
        int int_temp;
        
        /* Force moves between float and integer registers */
        asm volatile (
            "movd %[in_float], %%xmm0\n\t"
            "cvttss2si %%xmm0, %%eax\n\t"
            "add $100, %%eax\n\t"
            "mov %%eax, %[out_int]\n\t"
            "cvtsi2ss %%eax, %%xmm1\n\t"
            "movd %%xmm1, %[out_float]\n\t"
            : [out_int] "=r" (int_temp),
              [out_float] "=r" (float_temp)
            : [in_float] "r" (f1)
            : "rax", "xmm0", "xmm1", "cc"
        );
        
        f2 = float_temp;
        v1 = int_temp;
    }
    
    /* Create control flow with goto to keep many values live */
    if (v1 > 0) {
        goto block1;
    } else if (v2 > 0) {
        goto block2;
    } else if (v3 > 0) {
        goto block3;
    } else if (v4 > 0) {
        goto block4;
    }
    
    /* Final computation using all variables to prevent DCE */
    volatile int checksum = 0;
    checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    checksum += c1 + c2 + s1 + s2 + i1 + i2 + l1 + l2;
    checksum += (int)f1 + (int)f2 + (int)d1 + (int)d2;
    checksum += reg_var1 + reg_var2 + reg_var3;
    checksum += addr_var1 + addr_var2;
    
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0;
}
