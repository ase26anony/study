/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Helper to split basic blocks */
__attribute__((noinline, noclone))
static int compute_branch(int a, int b, int c) {
    if (a > b) {
        return a * c - b;
    } else {
        return b * c + a;
    }
}

__attribute__((noinline, noclone))
static double vector_reduce(v2df v) {
    return v[0] + v[1];
}

/* Main test function with high register pressure */
__attribute__((noinline, noclone))
static volatile long test_remat(volatile int input1, volatile long input2, 
                               volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 */
    volatile int a1 = input1 + 1;
    volatile long b1 = input2 - 1;
    volatile float c1 = input3 * 2.0f;
    volatile double d1 = input4 / 2.0;
    
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double db1, db2, db3, db4, db5, db6, db7, db8, db9, db10;
    
    /* Vector variables */
    v4si vec_int1, vec_int2, vec_int3, vec_result;
    v4sf vec_float1, vec_float2, vec_float3;
    v2df vec_double1, vec_double2;
    
    /* Complex serial computation chain with interdependencies */
    t1 = a1 * 3 + 7;
    t2 = t1 - a1 / 2;
    t3 = t2 * t1 + 11;
    t4 = t3 ^ t2;
    t5 = (t4 << 2) | t3;
    t6 = t5 - t4 * 3;
    t7 = t6 / (t5 & 0xFF) + 1;
    t8 = t7 * t6 - t5;
    t9 = t8 ^ t7;
    t10 = t9 + t8 * 2;
    
    l1 = b1 + t1;
    l2 = l1 * t2 - b1;
    l3 = l2 / (t3 + 1) + l1;
    l4 = l3 ^ l2;
    l5 = l4 << 3;
    l6 = l5 - l4 / 2;
    l7 = l6 * l5 + l3;
    l8 = l7 ^ l6;
    l9 = l8 + l7 * 3;
    l10 = l9 - l8 / 4;
    
    f1 = c1 + t1 * 0.5f;
    f2 = f1 * c1 - t2;
    f3 = f2 / (f1 + 1.0f) * 2.0f;
    f4 = f3 + f2 * 0.75f;
    f5 = f4 - f3 / 1.5f;
    f6 = f5 * f4 + f2;
    f7 = f6 / (f5 + 0.1f);
    f8 = f7 * 3.14f - f6;
    f9 = f8 + f7 * 2.71f;
    f10 = f9 - f8 / 1.618f;
    
    db1 = d1 + l1;
    db2 = db1 * d1 - l2;
    db3 = db2 / (db1 + 1.0) * 3.0;
    db4 = db3 + db2 * 0.5;
    db5 = db4 - db3 / 2.0;
    db6 = db5 * db4 + db2;
    db7 = db6 / (db5 + 0.01);
    db8 = db7 * 2.71828 - db6;
    db9 = db8 + db7 * 1.41421;
    db10 = db9 - db8 / 3.14159;
    
    /* Vector operations */
    vec_int1 = (v4si){t1, t2, t3, t4};
    vec_int2 = (v4si){t5, t6, t7, t8};
    vec_int3 = vec_int1 + vec_int2;
    vec_result = vec_int3 * vec_int1 - vec_int2;
    
    vec_float1 = (v4sf){f1, f2, f3, f4};
    vec_float2 = (v4sf){f5, f6, f7, f8};
    vec_float3 = vec_float1 * vec_float2 + vec_float1;
    
    vec_double1 = (v2df){db1, db2};
    vec_double2 = (v2df){db3, db4};
    
    /* Control flow to create multiple basic blocks */
    if (t10 > 1000) {
        t10 = compute_branch(t10, t9, t8);
        vec_result += (v4si){t10, t9, t8, t7};
    } else {
        t10 = compute_branch(t9, t10, t7);
        vec_result -= (v4si){t7, t8, t9, t10};
    }
    
    /* Inline assembly to clobber registers - x86_64 version */
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
        : /* no outputs */
        : /* no inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
          "memory"
    );
    
    /* More computations after assembly clobber */
    /* Recompute some values in slightly different forms */
    int t1_prime = a1 * 3 + 7;  /* Same as t1 */
    int t2_prime = t1_prime - a1 / 2 + 1;  /* Slightly different from t2 */
    long l1_prime = b1 + t1_prime * 2;  /* Different from l1 */
    
    float f1_recomp = c1 + t1_prime * 0.5f;  /* Same as f1 */
    double db1_recomp = d1 + l1_prime;  /* Different from db1 */
    
    /* Use recomputed values */
    t10 = t10 + t1_prime - t2_prime;
    l10 = l10 + l1_prime;
    f10 = f10 * f1_recomp;
    db10 = db10 / (db1_recomp + 1.0);
    
    /* Vector recomputation */
    v4si vec_int1_prime = (v4si){t1_prime, t2_prime, t3, t4};
    vec_result = vec_result + vec_int1_prime - vec_int1;
    
    /* Switch statement for additional control flow */
    switch (t10 & 0x3) {
        case 0:
            vec_result[0] += t10;
            break;
        case 1:
            vec_result[1] += l10;
            break;
        case 2:
            vec_result[2] += (int)f10;
            break;
        case 3:
            vec_result[3] += (int)db10;
            break;
    }
    
    /* Final reduction */
    volatile long result = t10 + l10 + (long)f10 + (long)db10;
    result += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    result += (long)vector_reduce(vec_double1 + vec_double2);
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
    }
    
    volatile long total = 0;
    volatile int input_seed = 42;
    
    for (volatile int i = 0; i < iterations; i++) {
        /* Vary inputs slightly each iteration */
        volatile int in1 = input_seed + i;
        volatile long in2 = input_seed * 3L - i;
        volatile float in3 = (float)input_seed / (i + 1) + 1.0f;
        volatile double in4 = (double)input_seed * 2.0 / (i + 2) + 0.5;
        
        total += test_remat(in1, in2, in3, in4);
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %ld\n", total);
    return 0;
}
