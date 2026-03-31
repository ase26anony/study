/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Helper function to split basic blocks */
__attribute__((noinline, noclone))
static int complex_transform(int a, int b, int c) {
    if (a > b) {
        return (a * c) + (b << 3);
    } else {
        return (b * c) - (a >> 2);
    }
}

__attribute__((noinline, noclone))
static double vector_compute(v4sf vec1, v4sf vec2) {
    v4sf result = vec1 + vec2 * 2.5f;
    return result[0] + result[1] + result[2] + result[3];
}

/* Main test function with high register pressure */
__attribute__((noinline, noclone))
static volatile long test_remat(volatile int input1, volatile long input2, 
                               volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 distinct ones */
    volatile int a = input1;
    volatile long b = input2;
    volatile float c = input3;
    volatile double d = input4;
    
    /* Integer variables */
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    /* Floating point variables */
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Vector variables */
    v4si vec_int1, vec_int2, vec_int3, vec_int4;
    v4sf vec_float1, vec_float2, vec_float3, vec_float4;
    v2df vec_double1, vec_double2;
    
    /* Initialize vectors */
    vec_int1 = (v4si){a, a+1, a+2, a+3};
    vec_int2 = (v4si){b%100, b%101, b%102, b%103};
    vec_float1 = (v4sf){c, c*1.1f, c*1.2f, c*1.3f};
    vec_float2 = (v4sf){d, (float)d*0.5f, (float)d*0.75f, (float)d*0.25f};
    
    /* Long serial chain of interdependent operations */
    t1 = a + (b % 100);
    t2 = t1 * (a >> 2);
    t3 = t2 - (b % 50);
    t4 = t3 ^ (t1 << 1);
    t5 = t4 | (t2 >> 3);
    t6 = t5 & 0xFFFF;
    t7 = t6 + complex_transform(t1, t2, t3);
    t8 = t7 * t4 - t5;
    t9 = t8 / (t6 ? t6 : 1);
    t10 = t9 ^ t7 ^ t8;
    
    f1 = c + 1.5f;
    f2 = f1 * 2.0f - c;
    f3 = f2 / (f1 + 0.5f);
    f4 = f3 * f1 - f2;
    f5 = f4 + f3 * 0.25f;
    
    d1 = d + 2.5;
    d2 = d1 * 3.0 - d;
    d3 = d2 / (d1 + 1.0);
    d4 = d3 * d1 - d2;
    d5 = d4 + d3 * 0.5;
    
    /* Vector operations consuming wide registers */
    vec_int3 = vec_int1 + vec_int2 * 2;
    vec_int4 = vec_int3 - vec_int1;
    vec_float3 = vec_float1 + vec_float2;
    vec_float4 = vec_float3 * 1.5f - vec_float1;
    
    /* Inline assembly to clobber physical registers */
    /* For x86_64 */
    asm volatile (
        "# Clobber many registers\n\t"
        "nop"
        : /* no outputs */
        : /* no inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15", "memory"
    );
    
    /* Continue with more operations after clobber */
    t11 = t10 + (t9 >> 2);
    t12 = t11 * t8 - t7;
    t13 = t12 & 0xFF;
    t14 = t13 | (t11 << 4);
    t15 = t14 ^ t12;
    
    /* Recompute a complex expression in slightly different form */
    /* This may trigger early rematerialization decisions */
    int recomputed_t1 = a + (b % 100);  /* Same as t1 but recomputed */
    int recomputed_t2 = recomputed_t1 * (a >> 2);  /* Same as t2 */
    
    /* Use recomputed values */
    t16 = recomputed_t2 - (b % 50) + 1;  /* Similar to t3 but +1 */
    t17 = t16 ^ (recomputed_t1 << 2);    /* Similar to t4 but <<2 */
    
    f6 = f5 * 2.0f;
    f7 = f6 - f4;
    f8 = f7 / (f5 + 0.1f);
    f9 = f8 * f6;
    f10 = f9 + f7 * 0.75f;
    
    d6 = d5 * 1.5;
    d7 = d6 - d4;
    d8 = d7 / (d5 + 0.2);
    d9 = d8 * d6;
    d10 = d9 + d7 * 0.6;
    
    /* More vector operations */
    vec_double1 = (v2df){d6, d7};
    vec_double2 = (v2df){d8, d9};
    vec_double1 = vec_double1 + vec_double2 * 0.3;
    
    /* Control flow to create multiple basic blocks */
    if (t15 > 1000) {
        t18 = t15 * 2;
        t19 = t18 - 500;
    } else if (t15 > 500) {
        t18 = t15 * 3;
        t19 = t18 - 250;
    } else {
        t18 = t15 * 4;
        t19 = t18 - 100;
    }
    
    switch (t19 % 5) {
        case 0:
            t20 = t19 + t18;
            break;
        case 1:
            t20 = t19 - t18;
            break;
        case 2:
            t20 = t19 * t18;
            break;
        case 3:
            t20 = t19 / (t18 ? t18 : 1);
            break;
        default:
            t20 = t19 ^ t18;
            break;
    }
    
    /* Use vector helper function */
    double vec_result = vector_compute(vec_float3, vec_float4);
    
    /* Final computation using all major temporaries */
    volatile long final_result = 
        (long)t20 + (long)f10 + (long)d10 + 
        (long)vec_result + vec_int3[0] + vec_int4[1] +
        (long)(vec_double1[0] * 1000);
    
    return final_result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 1000;
    }
    
    volatile long total = 0;
    volatile int input_seed = 42;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        volatile int in1 = input_seed + i;
        volatile long in2 = input_seed * 3L + i * 7L;
        volatile float in3 = (float)input_seed * 0.5f + i * 0.1f;
        volatile double in4 = (double)input_seed * 0.25 + i * 0.05;
        
        total += test_remat(in1, in2, in3, in4);
        
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(i));
    }
    
    printf("Final result: %ld\n", (long)total);
    return (int)(total % 1000);
}
