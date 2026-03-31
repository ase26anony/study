/* Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force specific register usage to create conflicts */
register uint64_t reg_var1 asm ("r12");
register uint64_t reg_var2 asm ("r13");
register uint64_t reg_var3 asm ("r14");

/* Volatile variables to prevent optimization and force spills */
volatile int v1 = 12345;
volatile int v2 = 67890;
volatile int v3 = 11111;
volatile int v4 = 22222;
volatile int v5 = 33333;
volatile int v6 = 44444;
volatile int v7 = 55555;
volatile int v8 = 66666;
volatile float f1 = 3.14159f;
volatile float f2 = 2.71828f;
volatile double d1 = 1.41421356;
volatile double d2 = 1.73205080;

/* Function to create complex live ranges */
void create_live_ranges(int *ptr1, int *ptr2, int *ptr3) {
    /* Multiple variables with different types and sizes */
    char c1 = 'A', c2 = 'B', c3 = 'C';
    short s1 = 1000, s2 = 2000, s3 = 3000;
    int i1 = 40000, i2 = 50000, i3 = 60000;
    long l1 = 70000L, l2 = 80000L, l3 = 90000L;
    float f3 = 4.5f, f4 = 5.5f, f5 = 6.5f;
    double d3 = 7.5, d4 = 8.5, d5 = 9.5;
    
    /* Block 1: Complex inline assembly with conflicting constraints */
block1:
    {
        int temp1 = v1 + v2;
        int temp2 = v3 * v4;
        
        /* Inline assembly with multiple outputs, inputs, and clobbers */
        asm volatile (
            "mov %[in1], %%rax\n\t"
            "add %[in2], %%rax\n\t"
            "mov %%rax, %[out1]\n\t"
            "imul %[in3], %%rax\n\t"
            "mov %%rax, %[out2]\n\t"
            "lea (%[in4],%[in5],2), %%rbx\n\t"
            "mov %%rbx, %[out3]"
            : [out1] "=r" (i1), [out2] "=r" (i2), [out3] "=r" (i3)
            : [in1] "r" (temp1), [in2] "r" (temp2), 
              [in3] "r" (v5), [in4] "r" (v6), [in5] "r" (v7)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "cc", "memory"
        );
        
        /* Use results to keep them live */
        *ptr1 = i1 + i2 + i3;
        v1 = i1 ^ i2;
    }
    
    /* Block 2: More assembly with memory addressing conflicts */
block2:
    {
        int addr_var = 99999;
        int *addr_ptr = &addr_var;
        
        /* Assembly that uses both register and memory constraints */
        asm volatile (
            "movl $0x12345678, %%eax\n\t"
            "movl %%eax, %[mem1]\n\t"
            "addl %[mem2], %%eax\n\t"
            "movl %%eax, %[reg1]\n\t"
            "movq %[addr], %%rbx\n\t"
            "movl (%%rbx), %%ecx\n\t"
            "addl %%ecx, %%eax"
            : [reg1] "=&r" (l1), [mem1] "=m" (v2)
            : [mem2] "m" (v3), [addr] "r" (addr_ptr)
            : "rax", "rbx", "rcx", "rdx", "cc", "memory"
        );
        
        *ptr2 = l1 + addr_var;
        v3 = l1 >> 4;
    }
    
    /* Block 3: Mixed data types and explicit register variables */
block3:
    {
        /* Use the explicit register variables */
        reg_var1 = v4 + v5;
        reg_var2 = v6 * v7;
        reg_var3 = v8 ^ v1;
        
        /* Assembly that clobbers the registers we're using */
        asm volatile (
            "mov %[in1], %%r12\n\t"      /* Clobbers our reg_var1 register */
            "mov %[in2], %%r13\n\t"      /* Clobbers our reg_var2 register */
            "add %%r13, %%r12\n\t"
            "mov %%r12, %[out1]\n\t"
            "mov %[in3], %%r14\n\t"      /* Clobbers our reg_var3 register */
            "imul %%r14, %%r12\n\t"
            "mov %%r12, %[out2]"
            : [out1] "=r" (s1), [out2] "=r" (s2)
            : [in1] "r" (c1), [in2] "r" (c2), [in3] "r" (c3)
            : "r12", "r13", "r14", "r15", "cc"
        );
        
        /* Force reload of register variables */
        v4 = reg_var1 + s1;
        v5 = reg_var2 + s2;
        v6 = reg_var3 ^ s1;
    }
    
    /* Block 4: Complex constraints with earlyclobber */
block4:
    {
        int tmp1 = v1 + v2;
        int tmp2 = v3 + v4;
        int tmp3 = v5 + v6;
        
        /* Multiple outputs with earlyclobber to force separate registers */
        asm volatile (
            "mov %[a], %%rax\n\t"
            "add %[b], %%rax\n\t"
            "mov %%rax, %[x]\n\t"
            "mov %[c], %%rbx\n\t"
            "sub %%rax, %%rbx\n\t"
            "mov %%rbx, %[y]\n\t"
            "imul %[d], %%rbx\n\t"
            "mov %%rbx, %[z]"
            : [x] "=&r" (tmp1), [y] "=&r" (tmp2), [z] "=r" (tmp3)
            : [a] "r" (v7), [b] "r" (v8), [c] "r" (tmp1), [d] "r" (tmp2)
            : "rax", "rbx", "rcx", "rdx", "cc"
        );
        
        *ptr3 = tmp1 + tmp2 + tmp3;
        v7 = tmp1 * tmp2;
        v8 = tmp3 & 0xFF;
    }
    
    /* Block 5: Floating point mixed with integer */
block5:
    {
        float ftmp1 = f1 + f2;
        double dtmp1 = d1 * d2;
        int itmp1 = v1 + v2;
        
        /* Mixed type constraints */
        asm volatile (
            "cvtsi2ss %[int_in], %%xmm0\n\t"
            "addss %[float_in], %%xmm0\n\t"
            "movss %%xmm0, %[float_out]\n\t"
            "cvtsi2sd %[int_in2], %%xmm1\n\t"
            "mulsd %[double_in], %%xmm1\n\t"
            "movsd %%xmm1, %[double_out]"
            : [float_out] "=x" (ftmp1), [double_out] "=x" (dtmp1)
            : [int_in] "r" (itmp1), [float_in] "x" (f3),
              [int_in2] "r" (v3), [double_in] "x" (d3)
            : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "cc"
        );
        
        f1 = ftmp1 + 1.0f;
        d1 = dtmp1 * 1.1;
        v1 = (int)ftmp1 + (int)dtmp1;
    }
}

int main() {
    int result1, result2, result3;
    int checksum = 0;
    
    /* Initialize explicit register variables */
    reg_var1 = 0xDEADBEEF;
    reg_var2 = 0xCAFEBABE;
    reg_var3 = 0x12345678;
    
    /* Create complex control flow with goto */
    int counter = 0;
    
start:
    if (counter++ > 3) goto finish;
    
    /* Call function with complex live ranges */
    create_live_ranges(&result1, &result2, &result3);
    
    /* Modify volatile variables to keep them live */
    v1 = v1 + result1;
    v2 = v2 ^ result2;
    v3 = v3 * result3;
    
    /* Jump back to create loop with live values */
    goto start;
    
finish:
    /* Aggregate results into checksum */
    checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    checksum += (int)f1 + (int)f2 + (int)d1 + (int)d2;
    checksum += result1 + result2 + result3;
    
    /* Print to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
