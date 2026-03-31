/* Test program to trigger GCC's reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force many reloads by creating register pressure and conflicts */
int main(void) {
    /* Phase 1: Declare many volatile variables to prevent optimization */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    volatile double d1 = 1.11, d2 = 2.22;
    volatile char c1 = 'a', c2 = 'b';
    volatile short s1 = 100, s2 = 200;
    volatile long l1 = 1000L, l2 = 2000L;
    
    /* Non-volatile variables that will span multiple basic blocks */
    int nv1 = 100, nv2 = 200, nv3 = 300, nv4 = 400;
    float nvf1 = 10.5f, nvf2 = 20.5f;
    double nvd1 = 100.5, nvd2 = 200.5;
    
    /* Explicit register variables to pin specific registers */
    register int reg_var1 asm ("r12") = 0x1234;
    register int reg_var2 asm ("r13") = 0x5678;
    register int reg_var3 asm ("r14") = 0x9ABC;
    
    /* Variables for address-taking to create addressing mode conflicts */
    int addr_var1 = 111, addr_var2 = 222, addr_var3 = 333;
    int *addr_ptr1 = &addr_var1, *addr_ptr2 = &addr_var2;
    
    /* Result accumulator */
    volatile int checksum = 0;
    
    /* Block 1: Create initial register pressure */
block1:
    /* Force many live values by doing arithmetic */
    v1 = v2 + v3;
    v4 = v5 * v6;
    f1 = f2 + f3;
    d1 = d2 * 2.0;
    
    /* Inline assembly with many clobbers to force spills */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movq %[in3], %%rbx\n\t"
        "addq %[in4], %%rbx\n\t"
        "movq %%rbx, %[out2]"
        : [out1] "=m" (v7), [out2] "=m" (l1)
        : [in1] "r" (v1), [in2] "r" (v2), 
          [in3] "r" (l1), [in4] "r" (l2)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "memory", "cc"
    );
    
    /* Modify variables to keep them live */
    nv1 = v7 + 10;
    reg_var1 = reg_var1 ^ nv1;
    
    goto block2;
    
    /* Block 2: More complex constraints and mode mismatches */
block2:
    {
        /* Local scope to create additional complexity */
        int local1 = nv1 + nv2;
        short local2 = s1 + s2;
        char local3 = c1 + c2;
        
        /* Inline assembly with mixed types and constraints */
        asm volatile (
            "mov %[in_char], %%al\n\t"
            "movsx %%al, %%eax\n\t"
            "add %[in_short], %%ax\n\t"
            "add %[in_int], %%eax\n\t"
            "mov %%eax, %[out_int]\n\t"
            "mov %[addr_in], %%rbx\n\t"
            "movl (%%rbx), %%ecx\n\t"
            "add %%ecx, %[out_sum]"
            : [out_int] "=m" (v8), [out_sum] "+m" (checksum)
            : [in_char] "r" ((int)local3), 
              [in_short] "r" ((int)local2),
              [in_int] "r" (local1),
              [addr_in] "r" (addr_ptr1)
            : "rax", "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        /* Use explicit register variables in C code */
        reg_var2 = reg_var2 + v8;
        
        /* Create addressing mode conflict */
        asm volatile (
            "movl %[val], %%eax\n\t"
            "addl %%eax, %[mem]"
            : [mem] "+m" (addr_var1)
            : [val] "r" (reg_var2)
            : "rax", "cc"
        );
    }
    
    goto block3;
    
    /* Block 3: Floating point and more register pressure */
block3:
    {
        /* Mix float and int operations */
        float temp_f = nvf1 + nvf2;
        double temp_d = nvd1 * 2.0;
        
        /* Inline assembly that clobbers floating point registers too */
        asm volatile (
            "movss %[fin], %%xmm0\n\t"
            "cvtss2sd %%xmm0, %%xmm1\n\t"
            "addsd %[din], %%xmm1\n\t"
            "cvtsd2ss %%xmm1, %%xmm0\n\t"
            "movss %%xmm0, %[fout]\n\t"
            "movl %[rin], %%eax\n\t"
            "addl $0x100, %%eax\n\t"
            "movl %%eax, %[rout]"
            : [fout] "=m" (f3), [rout] "=m" (v9)
            : [fin] "r" (temp_f), [din] "r" (temp_d), [rin] "r" (reg_var3)
            : "rax", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", 
              "xmm5", "xmm6", "xmm7", "memory", "cc"
        );
        
        /* More arithmetic to keep values live */
        nv3 = v9 * 2;
        nv4 = nv3 + reg_var1;
    }
    
    goto block4;
    
    /* Block 4: Final complex assembly with output reloads */
block4:
    {
        /* Create a situation requiring output reloads */
        int out1, out2, out3;
        
        /* Assembly with early clobber to force separate registers */
        asm volatile (
            "movl %[in1], %%eax\n\t"
            "imull %[in2], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "movl %[in3], %%ebx\n\t"
            "addl %%eax, %%ebx\n\t"
            "movl %%ebx, %[out2]\n\t"
            "leal (%[in4], %[in5]), %%ecx\n\t"
            "movl %%ecx, %[out3]"
            : [out1] "=&r" (out1), [out2] "=&r" (out2), [out3] "=r" (out3)
            : [in1] "r" (nv1), [in2] "r" (nv2), 
              [in3] "r" (nv3), [in4] "r" (nv4), [in5] "r" (reg_var1)
            : "rax", "rbx", "rcx", "rdx", "cc"
        );
        
        checksum += out1 + out2 + out3;
        
        /* One more with memory constraint conflict */
        asm volatile (
            "movl %[in], %%eax\n\t"
            "addl %%eax, %[out]"
            : [out] "+m" (checksum)
            : [in] "r" (addr_var2)
            : "rax", "cc"
        );
    }
    
    /* Final computation using all variables */
    checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += (int)f1 + (int)f2 + (int)f3;
    checksum += (int)d1 + (int)d2;
    checksum += c1 + c2 + s1 + s2 + l1 + l2;
    checksum += nv1 + nv2 + nv3 + nv4;
    checksum += reg_var1 + reg_var2 + reg_var3;
    checksum += addr_var1 + addr_var2 + addr_var3;
    
    /* Print to prevent elimination */
    printf("Final checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
