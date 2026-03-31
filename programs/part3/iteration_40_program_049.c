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
static int compute_magic(int a, int b, int c) {
    if (a > b) {
        return (a * c) ^ b;
    } else {
        return (b * a) | c;
    }
}

__attribute__((noinline, noclone))
static double vector_reduce(v2df v) {
    double sum = v[0] + v[1];
    if (sum > 1000.0) {
        return sum * 0.5;
    } else {
        return sum * 2.0;
    }
}

/* Main test function with high register pressure */
__attribute__((noinline, noclone))
static volatile int test_remat(volatile int input1, volatile long input2, 
                               volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 */
    volatile int a = input1;
    volatile long b = input2 + 1;
    volatile float c = input3 * 2.0f;
    volatile double d = input4 / 3.0;
    
    /* More variables for register pressure */
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    long l1, l2, l3, l4, l5;
    float f1, f2, f3, f4, f5;
    double d1, d2, d3, d4, d5;
    
    /* Vector variables */
    v4si vec1 = {a, a+1, a+2, a+3};
    v4si vec2 = {input1, input1*2, input1*3, input1*4};
    v4sf vecf1 = {c, c+1.0f, c+2.0f, c+3.0f};
    v4sf vecf2 = {input3, input3*2.0f, input3*3.0f, input3*4.0f};
    v2df vecd1 = {d, d+1.0};
    v2df vecd2 = {input4, input4*2.0};
    
    /* Complex interdependent computation chain */
    t1 = a + (int)b;
    t2 = t1 * (int)(b >> 4);
    t3 = t2 - compute_magic(a, t1, t2);
    
    /* Vector operations */
    vec1 = vec1 + vec2;
    vecf1 = vecf1 * vecf2;
    
    /* More scalar computations */
    l1 = b * t3;
    l2 = l1 + (long)t2 * 17;
    f1 = c * (float)t1;
    f2 = f1 / (float)(t2 + 1);
    
    /* Inline assembly to clobber registers */
    asm volatile (
        "# Clobber many registers\n\t"
        : 
        : 
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", 
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15", "memory"
    );
    
    /* Continue computation after clobber */
    t4 = t3 ^ (t1 >> 3);
    t5 = (t4 * 0x5A827999) & 0x7FFFFFFF;
    
    /* Recompute similar expression (encourages rematerialization) */
    t6 = a + (int)b;  /* Same as t1 computation */
    t7 = t6 * (int)(b >> 4);  /* Same as t2 computation */
    t8 = t7 - compute_magic(a, t6, t7);  /* Same as t3 computation */
    
    /* More operations with different variables */
    d1 = (double)t5 / 1000.0;
    d2 = d1 * input4;
    d3 = vector_reduce(vecd1 + vecd2);
    
    /* Control flow to create multiple basic blocks */
    if (t5 > 1000000) {
        t9 = t5 >> 8;
        f3 = f2 * 3.14159f;
        vec1 = vec1 << 2;
    } else {
        t9 = t5 << 4;
        f3 = f2 / 2.71828f;
        vec1 = vec1 >> 1;
    }
    
    /* Switch statement for more control flow */
    switch (t9 & 0x7) {
        case 0: t10 = t9 + a; break;
        case 1: t10 = t9 - b; break;
        case 2: t10 = t9 * t1; break;
        case 3: t10 = t9 / (a + 1); break;
        case 4: t10 = t9 ^ t2; break;
        case 5: t10 = t9 | t3; break;
        case 6: t10 = t9 & t4; break;
        default: t10 = ~t9; break;
    }
    
    /* Final mixing of all values */
    l3 = (long)t10 * l2;
    f4 = f3 * (float)d3;
    d4 = d2 + (double)f4;
    
    /* More vector operations */
    v4si vec3 = vec1 * vec2;
    v4sf vecf3 = vecf1 + vecf2;
    
    /* Extract and combine results */
    int vec_sum = vec3[0] + vec3[1] + vec3[2] + vec3[3];
    float vecf_sum = vecf3[0] + vecf3[1] + vecf3[2] + vecf3[3];
    
    /* Final result using most variables */
    volatile int result = (t10 + vec_sum) ^ (int)(l3 & 0xFFFFFFFF) 
                         + (int)(f4 * 1000.0f) 
                         + (int)(d4 * 100.0)
                         + (int)vecf_sum;
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 1000;
    }
    
    volatile long total = 0;
    volatile int seed = 42;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Varying inputs to prevent optimization */
        volatile int input1 = seed + i;
        volatile long input2 = seed * i + 12345;
        volatile float input3 = (seed + i) * 1.2345f;
        volatile double input4 = (seed * i) * 3.1415926535;
        
        int result = test_remat(input1, input2, input3, input4);
        total += result;
        
        /* Modify seed to change next iteration's inputs */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    printf("Final result: %ld\n", (long)total);
    return (int)(total & 0x7FFFFFFF);
}
