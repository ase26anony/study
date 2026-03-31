/* test_caller_save.c - Forces caller-save register spills at block ends */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
__attribute__((noinline)) void foo(void) {
    /* Empty function that will be called */
    volatile int sink = 0;
    (void)sink;
}

__attribute__((noinline)) void bar(void) {
    volatile int sink = 1;
    (void)sink;
}

__attribute__((noinline)) void baz(void) {
    volatile int sink = 2;
    (void)sink;
}

/* Helper to create register pressure */
#define DECLARE_INT_VARS(n) \
    volatile int int##n##_0, int##n##_1, int##n##_2, int##n##_3, int##n##_4; \
    volatile int int##n##_5, int##n##_6, int##n##_7, int##n##_8, int##n##_9;

#define USE_INT_VARS(n) \
    int##n##_0 = n * 1;  int##n##_1 = n * 2;  int##n##_2 = n * 3; \
    int##n##_3 = n * 4;  int##n##_4 = n * 5;  int##n##_5 = n * 6; \
    int##n##_6 = n * 7;  int##n##_7 = n * 8;  int##n##_8 = n * 9; \
    int##n##_9 = n * 10;

/* Test 1: Integer register pressure with call at end of basic block */
__attribute__((noinline)) int test_integer_pressure(int cond) {
    /* Create massive integer register pressure */
    DECLARE_INT_VARS(0) DECLARE_INT_VARS(1) DECLARE_INT_VARS(2)
    DECLARE_INT_VARS(3) DECLARE_INT_VARS(4) DECLARE_INT_VARS(5)
    
    /* Use all variables to make them live */
    USE_INT_VARS(0) USE_INT_VARS(1) USE_INT_VARS(2)
    USE_INT_VARS(3) USE_INT_VARS(4) USE_INT_VARS(5)
    
    /* Additional volatile integers to prevent optimization */
    volatile int v0 = 1, v1 = 2, v2 = 3, v3 = 4, v4 = 5;
    volatile int v5 = 6, v6 = 7, v7 = 8, v8 = 9, v9 = 10;
    
    /* Complex control flow to create basic blocks */
    if (cond > 0) {
        /* This basic block ends with the call */
        int sum = v0 + v1 + v2 + v3 + v4;
        
        /* Clobber many caller-saved registers */
        asm volatile("" : : : 
            "rax", "rcx", "rdx", "rsi", "rdi", 
            "r8", "r9", "r10", "r11", "rbx");
        
        /* Call at potential block end */
        foo();
        
        /* Use variables after call to keep them live across it */
        sum += v5 + v6 + v7 + v8 + v9;
        sum += int0_0 + int5_9;
        return sum;
    } else {
        /* Different path to create CFG complexity */
        bar();
        return cond;
    }
}

/* Test 2: Floating-point pressure with switch statement */
__attribute__((noinline)) double test_fp_pressure(int mode) {
    /* Create many FP live values */
    volatile double d0 = 1.0, d1 = 2.0, d2 = 3.0, d3 = 4.0, d4 = 5.0;
    volatile double d5 = 6.0, d6 = 7.0, d7 = 8.0, d8 = 9.0, d9 = 10.0;
    volatile double d10 = 11.0, d11 = 12.0, d12 = 13.0, d13 = 14.0, d14 = 15.0;
    
    /* Use math functions to force FP register usage */
    d0 = sin(d0); d1 = cos(d1); d2 = sin(d2); d3 = cos(d3); d4 = sin(d4);
    d5 = cos(d5); d6 = sin(d6); d7 = cos(d7); d8 = sin(d8); d9 = cos(d9);
    
    /* Switch creates multiple basic blocks */
    double result = 0.0;
    switch (mode) {
        case 0:
            /* Call at end of this case block */
            result = d0 + d1 + d2;
            
            /* Clobber FP/vector registers */
            asm volatile("" : : : 
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15");
            
            foo();
            
            /* Use FP values after call */
            result += d3 + d4 + d5 + d6;
            break;
            
        case 1:
            result = d7 * d8 * d9;
            bar();
            result += d10;
            break;
            
        default:
            result = d11 + d12 + d13 + d14;
            baz();
            result *= 2.0;
            break;
    }
    
    return result;
}

/* Test 3: Vector/SIMD pressure with loop unrolling */
__attribute__((noinline)) float test_vector_pressure(int iterations) {
    /* Use GCC vector extensions */
    typedef float v4sf __attribute__((vector_size(16)));
    typedef float v8sf __attribute__((vector_size(32)));
    
    /* Many vector variables */
    v4sf vec0 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec1 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec2 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec3 = {13.0f, 14.0f, 15.0f, 16.0f};
    v4sf vec4 = {17.0f, 18.0f, 19.0f, 20.0f};
    v4sf vec5 = {21.0f, 22.0f, 23.0f, 24.0f};
    v4sf vec6 = {25.0f, 26.0f, 27.0f, 28.0f};
    v4sf vec7 = {29.0f, 30.0f, 31.0f, 32.0f};
    
    /* Partially unrolled loop */
    float sum = 0.0f;
    for (int i = 0; i < iterations; i++) {
        /* Heavy vector computations */
        vec0 = vec0 + vec1;
        vec1 = vec1 * vec2;
        vec2 = vec2 - vec3;
        vec3 = vec3 / vec4;
        vec4 = vec4 + vec5;
        vec5 = vec5 * vec6;
        
        if (i == iterations / 2) {
            /* Call in the middle of loop body - creates basic block */
            
            /* Clobber all vector registers */
            asm volatile("" : : : 
                "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5",
                "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11",
                "ymm12", "ymm13", "ymm14", "ymm15");
            
            bar();
            
            /* Continue using vectors after call */
            vec6 = vec6 + vec7;
            vec7 = vec7 * vec0;
        }
        
        /* Extract and accumulate */
        float temp[4];
        memcpy(temp, &vec0, sizeof(temp));
        sum += temp[0] + temp[1] + temp[2] + temp[3];
    }
    
    return sum;
}

/* Test 4: Mixed pressure in nested control flow */
__attribute__((noinline)) int test_mixed_pressure(int x, int y) {
    /* Integer pressure */
    register int r0 asm("r10") = x;
    register int r1 asm("r11") = y;
    volatile int v[20];
    
    for (int i = 0; i < 20; i++) {
        v[i] = i * i;
    }
    
    /* FP pressure */
    volatile double d0 = x * 0.1, d1 = y * 0.2, d2 = (x+y) * 0.3;
    
    /* Complex nested if to create CFG with block ends at calls */
    int result = 0;
    if (x > 0) {
        if (y > 0) {
            /* First call site with register pressure */
            r0 = r0 * 2 + r1;
            d0 = sin(d0) + cos(d1);
            
            /* Clobber mixed registers */
            asm volatile("" : : : 
                "rax", "rcx", "rdx", "rsi", "rdi",
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4");
            
            foo();
            
            result = r0 + (int)d0;
        } else {
            /* Different path, different call */
            for (int i = 0; i < 10; i++) {
                v[i] += v[i+10];
            }
            
            bar();
            
            result = v[0] + v[10];
        }
        
        /* Another call at block end after if-else */
        d2 = d2 * 2.0;
        baz();
        result += (int)d2;
    } else {
        /* Alternative path with its own pressure */
        double d3 = 0.0;
        for (int i = 0; i < 5; i++) {
            d3 += sin(v[i] * 0.1);
        }
        
        foo();
        result = (int)d3;
    }
    
    return result;
}

int main(void) {
    int total = 0;
    
    /* Test all scenarios to increase coverage chance */
    total += test_integer_pressure(1);   /* Call at if-block end */
    total += test_integer_pressure(-1);  /* Different path */
    
    double fp_result = test_fp_pressure(0);  /* Call at switch case end */
    total += (int)fp_result;
    
    fp_result = test_fp_pressure(1);        /* Different case */
    total += (int)fp_result;
    
    fp_result = test_fp_pressure(2);        /* Default case */
    total += (int)fp_result;
    
    float vec_result = test_vector_pressure(10);
    total += (int)vec_result;
    
    total += test_mixed_pressure(10, 20);
    total += test_mixed_pressure(-5, 30);
    
    printf("Total: %d\n", total);
    return 0;
}
