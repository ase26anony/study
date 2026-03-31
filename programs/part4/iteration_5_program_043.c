#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_func(int a, int b, float c, double d, int e, float f) {
    volatile int result = a + b + (int)c + (int)d + e + (int)f;
    return result;
}

/* Another dummy function with different signature */
__attribute__((noinline))
double dummy_func2(double a, double b, int c, float d, int e, float f) {
    volatile double result = a * b + c + d + e + f;
    return result;
}

int main(void) {
    /* Force many live scalar variables with volatile to prevent optimization */
    volatile int v1 = rand() % 100;
    volatile int v2 = rand() % 100;
    volatile int v3 = rand() % 100;
    volatile int v4 = rand() % 100;
    volatile int v5 = rand() % 100;
    volatile int v6 = rand() % 100;
    volatile int v7 = rand() % 100;
    volatile int v8 = rand() % 100;
    volatile int v9 = rand() % 100;
    volatile int v10 = rand() % 100;
    
    volatile float f1 = (float)rand() / RAND_MAX;
    volatile float f2 = (float)rand() / RAND_MAX;
    volatile float f3 = (float)rand() / RAND_MAX;
    volatile float f4 = (float)rand() / RAND_MAX;
    volatile float f5 = (float)rand() / RAND_MAX;
    
    volatile double d1 = (double)rand() / RAND_MAX;
    volatile double d2 = (double)rand() / RAND_MAX;
    volatile double d3 = (double)rand() / RAND_MAX;
    volatile double d4 = (double)rand() / RAND_MAX;
    
    /* Non-volatile variables that will be heavily used */
    int nv1 = v1 + 1;
    int nv2 = v2 + 2;
    int nv3 = v3 + 3;
    int nv4 = v4 + 4;
    int nv5 = v5 + 5;
    int nv6 = v6 + 6;
    int nv7 = v7 + 7;
    int nv8 = v8 + 8;
    int nv9 = v9 + 9;
    int nv10 = v10 + 10;
    
    float nf1 = f1 * 2.0f;
    float nf2 = f2 * 3.0f;
    float nf3 = f3 * 4.0f;
    float nf4 = f4 * 5.0f;
    float nf5 = f5 * 6.0f;
    
    double nd1 = d1 * 2.0;
    double nd2 = d2 * 3.0;
    double nd3 = d3 * 4.0;
    double nd4 = d4 * 5.0;
    
    /* Array with complex addressing */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i * 2;
    }
    
    /* Explicit register variables - will conflict with compiler's choices */
    register int reg_var1 asm("ax") = v1 + v2;
    register int reg_var2 asm("bx") = v3 + v4;
    /* Note: Actual register names depend on architecture, but the constraint
       will force the compiler to work around these allocations */
    
    volatile int loop_counter = 0;
    volatile int accumulator = 0;
    volatile int loop_limit = 100; /* Force loop to execute */
    
    /* Complex loop with high register pressure */
    while (loop_counter < loop_limit) {
        /* Complex interdependent expressions mixing types */
        int temp1 = nv1 * nv2 + (int)(nf1 * 100.0f) + (int)nd1;
        int temp2 = nv3 ^ nv4 | (nv5 << 2) + (int)(nf2 * 50.0f);
        
        /* Type conversions forcing register moves */
        float ftemp1 = (float)temp1 + nf3 + (float)reg_var1;
        double dtemp1 = (double)temp2 + nd2 + (double)reg_var2;
        
        /* Complex pointer arithmetic with non-offsettable addresses */
        int idx1 = (temp1 + temp2) % 128;
        int idx2 = (temp1 * 2 + temp2) % 128;
        
        /* Non-simple addressing: array[idx + constant] where idx is complex */
        int mem1 = array[idx1 + 64];  /* May require address reload */
        int mem2 = array[idx2 * 2 + 32];  /* Complex addressing */
        
        /* Mixed size accesses */
        short short_val = (short)(mem1 + mem2);
        char char_val = (char)(mem1 - mem2);
        int int_val = short_val * char_val + mem1;
        
        /* Function call clobbers registers */
        int func_result = dummy_func(temp1, temp2, ftemp1, dtemp1, int_val, nf4);
        
        /* Inline assembly with clobbers - increases register pressure */
        asm volatile (
            "# Dummy assembly to clobber registers\n"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
            /* Clobber many registers to force spills/reloads */
        );
        
        /* More complex calculations after clobber */
        double dtemp2 = nd3 * (double)func_result + (double)int_val;
        float ftemp2 = nf5 * (float)dtemp2 + (float)char_val;
        
        /* Second function call */
        double dresult = dummy_func2(dtemp1, dtemp2, int_val, ftemp2, temp1, nf1);
        
        /* Update accumulator with volatile to prevent elimination */
        accumulator += (int)dresult + func_result + int_val + mem1 + mem2;
        
        /* Modify variables to create data dependencies across iterations */
        nv1 = nv2 + 1;
        nv2 = nv3 ^ nv1;
        nv3 = nv4 | nv2;
        nv4 = nv5 + nv3;
        nv5 = nv6 * 2;
        nv6 = nv7 - nv5;
        nv7 = nv8 & nv6;
        nv8 = nv9 | nv7;
        nv9 = nv10 ^ nv8;
        nv10 = accumulator % 1000;
        
        nf1 = nf2 * 1.1f;
        nf2 = nf3 + nf1;
        nf3 = nf4 - nf2;
        nf4 = nf5 * nf3;
        nf5 = (float)accumulator * 0.01f;
        
        nd1 = nd2 + 0.5;
        nd2 = nd3 * nd1;
        nd3 = nd4 - nd2;
        nd4 = (double)accumulator * 0.001;
        
        /* Update explicit register variables - may force reloads */
        reg_var1 = reg_var1 + nv1;
        reg_var2 = reg_var2 ^ nv2;
        
        loop_counter++;
    }
    
    printf("Final accumulator: %d\n", accumulator);
    return 0;
}
