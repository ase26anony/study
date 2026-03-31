/* reload_coverage.c - Test program to cover reload.cc push_reload initialization */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int g_int = 12345;
volatile long long g_llong = 9876543210LL;
volatile float g_float = 3.14159f;
volatile double g_double = 2.718281828459045;

/* Function with complex inline assembly to trigger reloads */
static long long trigger_reloads(int a, long long b, float c, double d) {
    int out1, out2;
    long long out3, out4;
    float out5;
    double out6;
    long long accumulator = 0;
    
    /* ASM 1: Mixed modes with specific register constraints - likely needs reload */
    asm volatile (
        /* Move int to output with 'r' constraint, but use complex addressing */
        "movl %[input1], %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %[output1]\n\t"
        : [output1] "=r" (out1)        /* Output in register */
        : [input1] "mr" (a)            /* Memory or register - may need reload */
        : "eax", "cc"                  /* Clobber eax and flags */
    );
    accumulator += out1;
    
    /* ASM 2: 64-bit operation with mismatched constraints - may need secondary reload */
    asm volatile (
        /* 64-bit operation requiring specific register pairing */
        "movq %[input2], %%rax\n\t"
        "imulq $0x12345678, %%rax, %%rax\n\t"
        "movq %%rax, %[output2]\n\t"
        : [output2] "=&r" (out2)       /* Early clobber output */
        : [input2] "rim" (b)           /* Register, immediate, or memory */
        : "rax", "rdx", "cc"           /* Clobber rax, rdx (imul uses rdx), flags */
    );
    accumulator += out2;
    
    /* ASM 3: Floating point with integer conversion - different modes */
    asm volatile (
        /* Convert float to int with specific constraints */
        "cvttss2si %[input3], %%eax\n\t"
        "addl $50, %%eax\n\t"
        "movl %%eax, %[output3]\n\t"
        : [output3] "=r" (out3)        /* Integer output */
        : [input3] "xm" (c)            /* SSE register or memory */
        : "eax", "cc"
    );
    accumulator += out3;
    
    /* ASM 4: Double precision with memory constraint - may need address reload */
    asm volatile (
        /* Load double, modify, store */
        "movsd %[input4], %%xmm0\n\t"
        "addsd %[input5], %%xmm0\n\t"
        "movsd %%xmm0, %[output4]\n\t"
        : [output4] "=m" (out4)        /* Memory output */
        : [input4] "m" (d),            /* Memory input */
          [input5] "m" (g_double)      /* Global variable - may need address reload */
        : "xmm0", "cc"
    );
    /* Convert double to long long for accumulator */
    out5 = (long long)out4;
    accumulator += out5;
    
    /* ASM 5: Multiple outputs with early clobber - high reload pressure */
    asm volatile (
        /* Complex operation with multiple outputs */
        "movl %[in1], %%eax\n\t"
        "movl %[in2], %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "imull %%ebx, %%eax\n\t"
        "movl %%eax, %[out2]\n\t"
        : [out1] "=&r" (out5),         /* Early clobber - conflicts with inputs */
          [out2] "=&r" (out6)          /* Another early clobber */
        : [in1] "mr" (g_int),          /* Global variable - may need address computation */
          [in2] "mr" (a)               /* Parameter - mixing constraints */
        : "eax", "ebx", "cc"
    );
    accumulator += (long long)out5 + (long long)out6;
    
    /* ASM 6: Vector-style operation (simulated) with specific constraints */
    {
        long long vec_in[2] = {b, accumulator};
        long long vec_out[2];
        
        asm volatile (
            /* Simulate vector operation */
            "movq %[in1], %%rax\n\t"
            "movq %[in2], %%rbx\n\t"
            "addq %%rbx, %%rax\n\t"
            "movq %%rax, %[out1]\n\t"
            "subq $100, %%rax\n\t"
            "movq %%rax, %[out2]\n\t"
            : [out1] "=m" (vec_out[0]),  /* Memory outputs */
              [out2] "=m" (vec_out[1])
            : [in1] "mr" (vec_in[0]),    /* Array elements - may need address reload */
              [in2] "mr" (vec_in[1])
            : "rax", "rbx", "cc"
        );
        accumulator += vec_out[0] + vec_out[1];
    }
    
    return accumulator;
}

int main(int argc, char *argv[]) {
    /* Use argv to create variant inputs preventing constant propagation */
    int base = (argc > 1) ? atoi(argv[1]) : 1000;
    
    /* Initialize test variables with non-constant values */
    int int_val = base + g_int;
    long long llong_val = (long long)base * 1000 + g_llong;
    float float_val = (float)base / 3.0f + g_float;
    double double_val = (double)base / 7.0 + g_double;
    
    /* Trigger the reload-intensive function */
    long long result = trigger_reloads(int_val, llong_val, float_val, double_val);
    
    /* Use the result to prevent optimization */
    printf("Result: %lld\n", result);
    
    /* Additional volatile asm to ensure all paths are used */
    asm volatile ("" : : "r" (result) : "memory");
    
    return (result > 0) ? 0 : 1;
}
