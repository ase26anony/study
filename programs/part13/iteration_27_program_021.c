/* Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force specific register usage */
register uint64_t reg_var1 asm ("r12");
register uint64_t reg_var2 asm ("r13");
register uint64_t reg_var3 asm ("r14");

/* Volatile variables to prevent optimization */
volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
volatile float f1, f2, f3, f4, f5;
volatile double d1, d2, d3, d4;
volatile char c1, c2, c3, c4;
volatile short s1, s2, s3, s4;
volatile long long ll1, ll2, ll3, ll4;

/* Function with complex inline assembly to trigger reloads */
int main(void) {
    int result = 0;
    
    /* Initialize volatile variables */
    v1 = 1; v2 = 2; v3 = 3; v4 = 4; v5 = 5;
    v6 = 6; v7 = 7; v8 = 8; v9 = 9; v10 = 10;
    
    f1 = 1.1f; f2 = 2.2f; f3 = 3.3f; f4 = 4.4f; f5 = 5.5f;
    d1 = 1.11; d2 = 2.22; d3 = 3.33; d4 = 4.44;
    
    c1 = 'a'; c2 = 'b'; c3 = 'c'; c4 = 'd';
    s1 = 100; s2 = 200; s3 = 300; s4 = 400;
    ll1 = 1000LL; ll2 = 2000LL; ll3 = 3000LL; ll4 = 4000LL;
    
    /* Initialize register variables */
    reg_var1 = 0x123456789ABCDEF0ULL;
    reg_var2 = 0xFEDCBA9876543210ULL;
    reg_var3 = 0xDEADBEEFCAFEBABEULL;
    
    /* Block 1: Complex inline assembly with conflicting constraints */
block1:
    {
        int temp1 = v1 + v2;
        int temp2 = v3 + v4;
        int temp3 = v5 + v6;
        
        /* Inline assembly with multiple outputs, inputs, and clobbers */
        asm volatile (
            "mov %[in1], %%rax\n\t"
            "add %[in2], %%rax\n\t"
            "mov %%rax, %[out1]\n\t"
            "imul %[in3], %%rax\n\t"
            "mov %%rax, %[out2]\n\t"
            "lea (%[in4],%[in5],2), %%rbx\n\t"
            "mov %%rbx, %[out3]"
            : [out1] "=&r" (temp1),   /* Early clobber - conflicts with inputs */
              [out2] "=r" (temp2),
              [out3] "=m" (v7)        /* Memory output */
            : [in1] "r" (temp1),      /* Register input */
              [in2] "r" (temp2),
              [in3] "r" (temp3),
              [in4] "r" (v8),
              [in5] "r" (v9)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory", "cc"
        );
        
        v1 = temp1;
        v2 = temp2;
        result += v1 + v2 + v7;
    }
    
    /* Block 2: Mixed data types and addressing modes */
block2:
    {
        double dtemp = d1 + d2;
        float ftemp = f1 + f2;
        char ctemp = c1 + c2;
        short stemp = s1 + s2;
        
        /* Take address of variables used in assembly */
        int* pv1 = &v1;
        float* pf1 = &f1;
        
        /* Complex assembly with mixed constraints */
        asm volatile (
            "mov %[in_d], %%xmm0\n\t"
            "addsd %[d2], %%xmm0\n\t"
            "movd %%xmm0, %[out_i]\n\t"
            "mov %[in_f], %%xmm1\n\t"
            "addss %[f2], %%xmm1\n\t"
            "movd %%xmm1, %[out_s]\n\t"
            "mov %[in_c], %%al\n\t"
            "add %[c2], %%al\n\t"
            "mov %%al, %[out_c]"
            : [out_i] "=r" (v3),      /* Integer output */
              [out_s] "=r" (stemp),   /* Short output */
              [out_c] "=m" (c3)       /* Char memory output */
            : [in_d] "x" (dtemp),     /* XMM register input */
              [d2] "m" (d2),          /* Memory input */
              [in_f] "x" (ftemp),
              [f2] "m" (f2),
              [in_c] "r" (ctemp),
              [c2] "r" (c2),
              "m" (*pv1),             /* Extra memory input */
              "m" (*pf1)              /* Extra memory input */
            : "rax", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "memory", "cc"
        );
        
        s3 = stemp;
        result += v3 + s3 + c3;
        
        /* Force goto to create live value spans */
        if (result < 1000)
            goto block3;
        else
            goto block4;
    }
    
    /* Block 3: Register variable conflicts */
block3:
    {
        uint64_t local1 = reg_var1;
        uint64_t local2 = reg_var2;
        uint64_t local3 = reg_var3;
        
        /* Assembly that clobbers our register variables */
        asm volatile (
            "mov $0xAAAAAAAA, %%r12\n\t"   /* Clobber reg_var1's register */
            "mov $0xBBBBBBBB, %%r13\n\t"   /* Clobber reg_var2's register */
            "mov $0xCCCCCCCC, %%r14\n\t"   /* Clobber reg_var3's register */
            "add %%r12, %%r13\n\t"
            "add %%r13, %%r14\n\t"
            "mov %%r14, %[out]"
            : [out] "=m" (ll1)
            : 
            : "r12", "r13", "r14", "rax", "rbx", "rcx", "rdx", 
              "rsi", "rdi", "r8", "r9", "r10", "r11", "memory", "cc"
        );
        
        /* Use the register variables after clobber - forces reload */
        result += (int)(local1 + local2 + local3);
        goto block5;
    }
    
    /* Block 4: More complex constraints */
block4:
    {
        long long lltemp1 = ll2 + ll3;
        long long lltemp2 = ll4;
        
        /* Assembly with "+" constraint (read-write operand) */
        asm volatile (
            "add %[inc], %[acc]\n\t"
            "imul %[mul], %[acc]\n\t"
            "mov %[acc], %[out]"
            : [acc] "+r" (lltemp1),   /* Read-write operand */
              [out] "=m" (ll4)        /* Memory output */
            : [inc] "r" (v4),
              [mul] "r" (v5)
            : "rax", "rdx", "memory", "cc"  /* rdx clobbered by imul */
        );
        
        result += (int)(lltemp1 + lltemp2);
        goto block5;
    }
    
    /* Block 5: Final computations */
block5:
    {
        /* Use all remaining volatile variables */
        int final = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        final += (int)(f1 + f2 + f3 + f4 + f5);
        final += (int)(d1 + d2 + d3 + d4);
        final += c1 + c2 + c3 + c4;
        final += s1 + s2 + s3 + s4;
        final += (int)(ll1 + ll2 + ll3 + ll4);
        
        /* One more assembly with complex constraints */
        int out1, out2;
        asm volatile (
            "mov %[in1], %%eax\n\t"
            "lea (%%eax, %%eax, 2), %%ebx\n\t"
            "mov %%eax, %[out1]\n\t"
            "mov %%ebx, %[out2]"
            : [out1] "=&r" (out1),    /* Early clobber */
              [out2] "=r" (out2)
            : [in1] "r" (final)
            : "rax", "rbx", "cc"
        );
        
        result += out1 + out2;
    }
    
    /* Print result to prevent optimization */
    volatile int checksum = result;
    printf("Result: %d\n", checksum);
    
    return 0;
}
