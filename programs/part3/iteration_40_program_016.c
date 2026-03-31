/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* GCC vector extensions for additional register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Helper function to split basic blocks */
__attribute__((noinline, noclone))
static int complex_transform(int a, int b, float c, double d) {
    if (a > b) {
        return (int)((a * b) + (c * d));
    } else {
        return (int)((a + b) - (c / d));
    }
}

/* Another helper with switch statement */
__attribute__((noinline, noclone))
static double conditional_operation(int mode, double x, double y, double z) {
    double result;
    switch (mode % 4) {
        case 0: result = x + y * z; break;
        case 1: result = x - y / z; break;
        case 2: result = x * y + z; break;
        case 3: result = x / y - z; break;
        default: result = x + y + z; break;
    }
    return result;
}

/* Main test function implementing all requirements */
__attribute__((noinline, noclone))
static volatile long test_remat(volatile int input1, volatile long input2, 
                               volatile float input3, volatile double input4) {
    /* Requirement 1: Many local variables of mixed types */
    int a1 = input1 + 1;
    int a2 = input1 * 2;
    int a3 = input1 - 3;
    int a4 = input1 / 4;
    int a5 = input1 % 5;
    int a6 = input1 << 1;
    int a7 = input1 >> 2;
    int a8 = ~input1;
    int a9 = input1 | 0xFF;
    int a10 = input1 & 0x0F;
    
    long b1 = input2 + 1000;
    long b2 = input2 * 2000;
    long b3 = input2 - 3000;
    long b4 = input2 / 4000;
    long b5 = input2 % 5000;
    long b6 = input2 << 2;
    long b7 = input2 >> 3;
    long b8 = ~input2;
    long b9 = input2 | 0xFFFF;
    long b10 = input2 & 0x0FFF;
    
    float c1 = input3 + 1.5f;
    float c2 = input3 * 2.5f;
    float c3 = input3 - 3.5f;
    float c4 = input3 / 4.5f;
    float c5 = input3 * input3;
    float c6 = 1.0f / input3;
    float c7 = input3 + input3;
    float c8 = input3 - input3;
    float c9 = input3 * 3.14159f;
    float c10 = input3 / 2.71828f;
    
    double d1 = input4 + 1.5;
    double d2 = input4 * 2.5;
    double d3 = input4 - 3.5;
    double d4 = input4 / 4.5;
    double d5 = input4 * input4;
    double d6 = 1.0 / input4;
    double d7 = input4 + input4;
    double d8 = input4 - input4;
    double d9 = input4 * 3.14159265358979;
    double d10 = input4 / 2.71828182845904;
    
    /* Vector variables for additional register pressure */
    v4si v1 = {a1, a2, a3, a4};
    v4si v2 = {a5, a6, a7, a8};
    v4sf v3 = {c1, c2, c3, c4};
    v4sf v4 = {c5, c6, c7, c8};
    v2df v5 = {d1, d2};
    v2df v6 = {d3, d4};
    
    /* Requirement 2: Long serial chain of interdependent operations */
    /* First computation chain - used multiple times */
    long chain1 = a1 + b1;
    float chain2 = chain1 * c1;
    double chain3 = chain2 + d1;
    int chain4 = (int)chain3 * a2;
    long chain5 = chain4 + b2;
    float chain6 = chain5 * c2;
    double chain7 = chain6 + d2;
    
    /* Use chain7 in multiple places */
    double temp1 = chain7 * 1.1;
    double temp2 = chain7 / 1.2;
    double temp3 = chain7 + 1.3;
    
    /* Requirement 5: Control flow splitting */
    if (chain7 > 1000.0) {
        chain4 = complex_transform(a3, a4, c3, d3);
        temp1 = conditional_operation(chain4 % 4, temp1, temp2, temp3);
    } else {
        chain4 = complex_transform(a5, a6, c4, d4);
        temp2 = conditional_operation(chain4 % 4, temp2, temp3, temp1);
    }
    
    /* More computation using the same chain7 value (forcing potential remat) */
    double temp4 = chain7 * 2.0 - temp1;
    double temp5 = chain7 / 2.0 + temp2;
    double temp6 = chain7 + chain7 - temp3;
    
    /* Vector operations */
    v4si v7 = v1 + v2;
    v4si v8 = v1 * v2;
    v4sf v9 = v3 + v4;
    v4sf v10 = v3 * v4;
    v2df v11 = v5 + v6;
    v2df v12 = v5 * v6;
    
    /* Requirement 3: Inline assembly clobbering registers */
    /* x86_64 version - clobber many registers */
    asm volatile (
        "# Artificial register pressure\n\t"
        "nop"
        : /* no outputs */
        : /* no inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", 
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15", "memory"
    );
    
    /* ARM version (commented out - use appropriate for target)
    asm volatile (
        "# Artificial register pressure\n\t"
        "nop"
        : 
        : 
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14",
          "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
          "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15",
          "memory"
    );
    */
    
    /* Continue computation after clobber */
    /* Recomputation of chain7 in slightly different form (Requirement 2) */
    double chain7_recomp = (a1 + b1) * c1 + d1;
    chain7_recomp = chain7_recomp * a2 + b2;
    chain7_recomp = chain7_recomp * c2 + d2;
    
    /* Use the recomputed value */
    double temp7 = chain7_recomp * temp4;
    double temp8 = chain7_recomp / temp5;
    double temp9 = chain7_recomp + temp6;
    
    /* More vector operations */
    v4si v13 = v7 + v8;
    v4si v14 = v7 * v8;
    v4sf v15 = v9 + v10;
    v4sf v16 = v9 * v10;
    v2df v17 = v11 + v12;
    v2df v18 = v11 * v12;
    
    /* Final complex computation using all temporaries */
    double final1 = temp1 + temp2 + temp3 + temp4 + temp5 + temp6;
    double final2 = temp7 + temp8 + temp9;
    double final3 = (double)(v13[0] + v13[1] + v13[2] + v13[3]);
    double final4 = (double)(v15[0] + v15[1] + v15[2] + v15[3]);
    double final5 = v17[0] + v17[1];
    
    /* Requirement 2: Loop with recomputation */
    volatile int loop_counter = 5; /* Small but enough for analysis */
    volatile long accumulator = 0;
    
    for (int i = 0; i < loop_counter; i++) {
        /* Compute a value from complex expression */
        double loop_val = (a1 * i) + (b1 / (i + 1)) + (c1 * i) + (d1 * i * i);
        
        /* Use in multiple statements */
        accumulator += (long)(loop_val * 1000);
        accumulator -= (long)(loop_val / 100);
        accumulator ^= (long)(loop_val * loop_val);
        
        /* Recomputation in slightly different form */
        double loop_val_recomp = (a1 * (i + 1)) + (b1 / (i + 2)) + 
                                (c1 * (i + 1)) + (d1 * (i + 1) * (i + 1));
        
        /* Use recomputed value */
        accumulator += (long)(loop_val_recomp * 500);
        accumulator |= (long)(loop_val_recomp);
        
        /* More control flow */
        switch (i % 3) {
            case 0:
                accumulator += complex_transform(a1 + i, a2 - i, c1, d1);
                break;
            case 1:
                accumulator += (long)conditional_operation(i, d2, d3, d4);
                break;
            case 2:
                accumulator ^= v13[i % 4];
                break;
        }
    }
    
    /* Final result using all computed values */
    volatile long result = accumulator + 
                         (long)(final1 + final2 + final3 + final4 + final5) +
                         (long)(v1[0] + v2[1] + v3[2] + v4[3]) +
                         (long)(v5[0] + v6[1]) +
                         a10 + b10 + (long)c10 + (long)d10;
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 100;
    }
    
    volatile long total = 0;
    volatile int input_seed = 42;
    
    for (volatile int i = 0; i < iterations; i++) {
        /* Vary inputs slightly each iteration */
        volatile int in1 = input_seed + i;
        volatile long in2 = input_seed * 1000L + i * 37L;
        volatile float in3 = (float)input_seed / 7.0f + (float)i * 0.1f;
        volatile double in4 = (double)input_seed / 11.0 + (double)i * 0.01;
        
        total += test_remat(in1, in2, in3, in4);
        
        /* Prevent optimization */
        asm volatile("" : "+r"(total));
    }
    
    printf("Result: %ld\n", total);
    return 0;
}
