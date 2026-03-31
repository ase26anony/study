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
static double vector_helper(v2df v1, v2df v2) {
    v2df result = v1 + v2;
    return result[0] * result[1];
}

/* Main test function implementing all requirements */
__attribute__((noinline, noclone))
static volatile long test_remat(volatile int input1, volatile long input2, 
                               volatile float input3, volatile double input4) {
    /* Requirement 1: Many local variables of mixed types */
    volatile int a = input1;
    volatile long b = input2;
    volatile float c = input3;
    volatile double d = input4;
    
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Requirement 5: Vector variables */
    v4si vec_int1, vec_int2, vec_int3;
    v4sf vec_float1, vec_float2, vec_float3;
    v2df vec_double1, vec_double2, vec_double3;
    
    /* Initialize vectors */
    vec_int1 = (v4si){a, a+1, a+2, a+3};
    vec_int2 = (v4si){b&0xFF, (b>>8)&0xFF, (b>>16)&0xFF, (b>>24)&0xFF};
    vec_float1 = (v4sf){c, c*2, c*3, c*4};
    vec_double1 = (v2df){d, d*1.5};
    
    /* Requirement 1: Long serial chain of interdependent operations */
    t1 = a + (b & 0xFF);
    t2 = t1 * (a >> 4);
    t3 = t2 - (b & 0xF0F0);
    t4 = t3 ^ (t1 << 3);
    t5 = t4 | (t2 >> 2);
    t6 = t5 & ~t3;
    t7 = t6 + complex_transform(t1, t2, t3);
    t8 = t7 * (t4 % 31);
    t9 = t8 - (t5 ^ 0xAAAA);
    t10 = t9 | (t6 << 1);
    
    l1 = b + t1;
    l2 = l1 * t2;
    l3 = l2 - t3;
    l4 = l3 ^ t4;
    l5 = l4 | t5;
    l6 = l5 & ~l1;
    l7 = l6 + (l2 >> 4);
    l8 = l7 * (l3 % 17);
    l9 = l8 - (l4 ^ 0x5555);
    l10 = l9 | (l5 << 2);
    
    f1 = c + t1;
    f2 = f1 * t2;
    f3 = f2 - t3;
    f4 = f3 / (t4 + 1.0f);
    f5 = f4 * f1;
    f6 = f5 - f2;
    f7 = f6 + f3;
    f8 = f7 * f4;
    f9 = f8 / (f5 + 0.5f);
    f10 = f9 - f6;
    
    d1 = d + l1;
    d2 = d1 * l2;
    d3 = d2 - l3;
    d4 = d3 / (l4 + 1.0);
    d5 = d4 * d1;
    d6 = d5 - d2;
    d7 = d6 + d3;
    d8 = d7 * d4;
    d9 = d8 / (d5 + 0.5);
    d10 = d9 - d6;
    
    /* Requirement 4: Vector operations */
    vec_int3 = vec_int1 + vec_int2;
    vec_int3 = vec_int3 * (v4si){t1, t2, t3, t4};
    vec_int3 = vec_int3 - vec_int1;
    
    vec_float2 = vec_float1 * (v4sf){f1, f2, f3, f4};
    vec_float3 = vec_float2 + vec_float1;
    vec_float3 = vec_float3 / (v4sf){2.0f, 3.0f, 4.0f, 5.0f};
    
    vec_double2 = vec_double1 * (v2df){d1, d2};
    vec_double3 = vec_double2 + vec_double1;
    
    /* Requirement 3: Inline assembly to clobber registers */
    /* x86_64 version */
    asm volatile (
        "# Clobber many registers to increase pressure\n\t"
        "mov $0, %%rax\n\t"
        "mov $0, %%rbx\n\t"
        "mov $0, %%rcx\n\t"
        "mov $0, %%rdx\n\t"
        "mov $0, %%rsi\n\t"
        "mov $0, %%rdi\n\t"
        "mov $0, %%r8\n\t"
        "mov $0, %%r9\n\t"
        "mov $0, %%r10\n\t"
        "mov $0, %%r11\n\t"
        "mov $0, %%r12\n\t"
        "mov $0, %%r13\n\t"
        "mov $0, %%r14\n\t"
        "mov $0, %%r15\n\t"
        "pxor %%xmm0, %%xmm0\n\t"
        "pxor %%xmm1, %%xmm1\n\t"
        "pxor %%xmm2, %%xmm2\n\t"
        "pxor %%xmm3, %%xmm3\n\t"
        "pxor %%xmm4, %%xmm4\n\t"
        "pxor %%xmm5, %%xmm5\n\t"
        "pxor %%xmm6, %%xmm6\n\t"
        "pxor %%xmm7, %%xmm7\n\t"
        "pxor %%xmm8, %%xmm8\n\t"
        "pxor %%xmm9, %%xmm9\n\t"
        "pxor %%xmm10, %%xmm10\n\t"
        "pxor %%xmm11, %%xmm11\n\t"
        "pxor %%xmm12, %%xmm12\n\t"
        "pxor %%xmm13, %%xmm13\n\t"
        "pxor %%xmm14, %%xmm14\n\t"
        "pxor %%xmm15, %%xmm15"
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
          "memory"
    );
    
    /* ARM version (commented out, use for ARM targets)
    asm volatile (
        "# Clobber many ARM registers\n\t"
        "mov r0, #0\n\t"
        "mov r1, #0\n\t"
        "mov r2, #0\n\t"
        "mov r3, #0\n\t"
        "mov r4, #0\n\t"
        "mov r5, #0\n\t"
        "mov r6, #0\n\t"
        "mov r7, #0\n\t"
        "mov r8, #0\n\t"
        "mov r9, #0\n\t"
        "mov r10, #0\n\t"
        "vmov.i32 q0, #0\n\t"
        "vmov.i32 q1, #0\n\t"
        "vmov.i32 q2, #0\n\t"
        "vmov.i32 q3, #0\n\t"
        "vmov.i32 q4, #0\n\t"
        "vmov.i32 q5, #0\n\t"
        "vmov.i32 q6, #0\n\t"
        "vmov.i32 q7, #0"
        :
        :
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10",
          "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "memory"
    );
    */
    
    /* Requirement 5: Control flow to split basic blocks */
    int switch_var = t7 & 0x7;
    double extra = 0.0;
    
    switch (switch_var) {
        case 0:
            extra = d1 + d2 + d3;
            break;
        case 1:
            extra = d4 * d5 - d6;
            break;
        case 2:
            extra = vector_helper(vec_double2, vec_double3);
            break;
        case 3:
            extra = (double)(t8 + t9 + t10);
            break;
        case 4:
            extra = (double)(l7 * l8) / (l9 + 1.0);
            break;
        case 5:
            extra = f7 + f8 + f9;
            break;
        case 6:
            extra = (double)(vec_int3[0] + vec_int3[1] + vec_int3[2]);
            break;
        default:
            extra = d10 * 2.0;
            break;
    }
    
    /* Requirement 2: Recompute a value in slightly different form */
    /* First computation - complex expression */
    long complex_val = (t10 * l10) + ((t9 & 0xFFF) << 4) - (l8 % 77);
    
    /* Use complex_val in multiple statements */
    double use1 = (double)complex_val * d9;
    float use2 = (float)complex_val / f10;
    int use3 = complex_val ^ 0xDEADBEEF;
    
    /* More operations that might make complex_val live across many insns */
    v4si vec_use = vec_int3 + (v4si){use3, use3>>8, use3>>16, use3>>24};
    double d_use = d8 + use1 + extra;
    
    /* Recompute similar value later in same iteration */
    long recomputed_val = (t10 * l10) + ((t9 & 0xFFE) << 4) - (l8 % 77) + 1;
    
    /* Use recomputed value */
    double final_d = d_use + (double)recomputed_val;
    float final_f = use2 + (float)recomputed_val;
    
    /* Requirement 5: More control flow */
    if (final_d > 1000.0) {
        final_d = final_d / 2.0;
        final_f = final_f * 1.5f;
    } else if (final_d < -1000.0) {
        final_d = final_d * 1.5;
        final_f = final_f / 2.0f;
    } else {
        final_d = final_d + extra;
        final_f = final_f - (float)extra;
    }
    
    /* Combine everything into final result */
    volatile long result = (long)(final_d + final_f + use1 + d10 + f10 + 
                                 t10 + l10 + vec_use[0] + vec_use[1] + 
                                 vec_use[2] + vec_use[3]);
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 100;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 100;
    }
    
    volatile long total = 0;
    volatile int input_seed = 12345;
    
    /* Requirement 2: Loop with volatile trip count */
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        volatile int in1 = input_seed + i;
        volatile long in2 = input_seed * 17L + i * 23L;
        volatile float in3 = (float)(input_seed % 100) / 3.14f + i * 0.1f;
        volatile double in4 = (double)(input_seed % 200) / 6.28 + i * 0.05;
        
        long result = test_remat(in1, in2, in3, in4);
        total += result;
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Final result: %ld\n", (long)total);
    return (int)(total & 0x7FFFFFFF);
}
