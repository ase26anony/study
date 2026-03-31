/* test_caller_save.c - Program to trigger caller-save register spill insertion */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline, noipa))

/* External function that clobbers registers - defined in separate file */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(__m128, __m128);

/* Helper to use variables after calls */
volatile int use_result;

/* Test 1: Integer register pressure with call at end of basic block */
NOINLINE int test_integer_pressure(int seed) {
    /* Create many integer variables that must survive across call */
    volatile int v0 = seed;
    register int r1 asm ("r12") = v0 + 1;
    register int r2 asm ("r13") = r1 * 2;
    register int r3 asm ("r14") = r2 + seed;
    register int r4 asm ("r15") = r3 ^ 0x1234;
    int r5 = r4 - seed;
    int r6 = r5 * 3;
    int r7 = r6 / 2;
    int r8 = r7 | 0xFF;
    int r9 = r8 << 3;
    int r10 = r9 >> 1;
    int r11 = r10 + r1;
    int r12 = r11 * r2;
    int r13 = r12 ^ r3;
    int r14 = r13 + r4;
    int r15 = r14 - r5;
    int r16 = r15 & r6;
    int r17 = r16 | r7;
    int r18 = r17 ^ r8;
    int r19 = r18 + r9;
    int r20 = r19 * r10;
    int r21 = r20 - r11;
    int r22 = r21 ^ r12;
    int r23 = r22 + r13;
    int r24 = r23 * r14;
    int r25 = r24 - r15;
    int r26 = r25 ^ r16;
    int r27 = r26 + r17;
    int r28 = r27 * r18;
    int r29 = r28 - r19;
    int r30 = r29 ^ r20;
    
    /* Complex control flow to create basic block ending with call */
    if (seed % 3 == 0) {
        /* This basic block ends with the call to foo() */
        int temp = r1 + r2 + r3 + r4 + r5;
        
        /* Inline asm to clobber caller-saved registers */
        asm volatile("# Clobber integer regs" : : : 
            "rax", "rcx", "rdx", "rsi", "rdi", 
            "r8", "r9", "r10", "r11", "xmm0", 
            "xmm1", "xmm2", "xmm3", "xmm4", "xmm5");
        
        /* Call at potential end of basic block */
        foo();
        
        /* Use variables after call - forces saves */
        use_result = r30 + temp;
        return r21 + r22 + r23 + use_result;
    } else if (seed % 3 == 1) {
        /* Different path */
        return r1 + r2;
    } else {
        /* Another path - creates more CFG complexity */
        int sum = 0;
        for (int i = 0; i < 5; i++) {
            sum += r1 + i;
            if (i == 3) {
                /* Nested call in loop */
                asm volatile("# Clobber more" : : : 
                    "rbx", "rbp", "r12", "r13", "r14", "r15",
                    "xmm6", "xmm7", "xmm8", "xmm9");
                bar(sum, r2 * 0.5);
            }
        }
        return sum + r3;
    }
}

/* Test 2: Floating-point register pressure */
NOINLINE double test_float_pressure(double seed) {
    /* Many floating-point variables */
    volatile double d0 = seed;
    double d1 = sin(d0);
    double d2 = cos(d1);
    double d3 = d1 * d2;
    double d4 = d3 + seed;
    double d5 = exp(d4);
    double d6 = log(fabs(d5) + 1.0);
    double d7 = d6 * 2.0;
    double d8 = d7 - d1;
    double d9 = d8 / 3.0;
    double d10 = pow(d9, 2.0);
    double d11 = sqrt(d10);
    double d12 = d11 + d2;
    double d13 = sin(d12);
    double d14 = cos(d13);
    double d15 = d14 * d3;
    double d16 = d15 - d4;
    double d17 = exp(d16);
    double d18 = log(fabs(d17) + 1.0);
    double d19 = d18 * 4.0;
    double d20 = d19 + d5;
    double d21 = d20 / 2.0;
    double d22 = pow(d21, 1.5);
    double d23 = sqrt(d22);
    double d24 = d23 - d6;
    double d25 = sin(d24);
    double d26 = cos(d25);
    double d27 = d26 * d7;
    double d28 = d27 + d8;
    double d29 = exp(d28);
    double d30 = log(fabs(d29) + 1.0);
    
    /* Switch statement to create complex CFG */
    int choice = (int)seed % 4;
    double result = 0.0;
    
    switch (choice) {
        case 0:
            /* Call at end of this case's basic block */
            asm volatile("# Clobber FP regs" : : : 
                "xmm0", "xmm1", "xmm2", "xmm3",
                "xmm4", "xmm5", "xmm6", "xmm7",
                "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15");
            foo();
            result = d30 + d1 + d2;
            break;
            
        case 1:
            result = d3 + d4 + d5;
            break;
            
        case 2:
            /* Loop with call at end of iteration */
            for (int i = 0; i < 3; i++) {
                double temp = d6 + i;
                if (i == 1) {
                    bar(i, temp);
                }
                result += temp;
            }
            break;
            
        case 3:
        default:
            /* Another call site */
            asm volatile("# More clobbering" : : : 
                "rax", "rcx", "rdx", "xmm0", "xmm1");
            foo();
            result = d7 * d8;
            break;
    }
    
    /* Use all variables after control flow */
    use_result = (int)(d9 + d10 + d11 + d12 + d13 + d14 + d15 + 
                      d16 + d17 + d18 + d19 + d20 + d21 + d22 + 
                      d23 + d24 + d25 + d26 + d27 + d28 + d29);
    
    return result + d30;
}

/* Test 3: Vector register pressure with SSE/AVX */
NOINLINE __m128 test_vector_pressure(float seed) {
    /* Many vector variables */
    __m128 v0 = _mm_set1_ps(seed);
    __m128 v1 = _mm_add_ps(v0, _mm_set1_ps(1.0f));
    __m128 v2 = _mm_mul_ps(v1, _mm_set1_ps(2.0f));
    __m128 v3 = _mm_sub_ps(v2, v0);
    __m128 v4 = _mm_div_ps(v3, _mm_set1_ps(3.0f));
    __m128 v5 = _mm_add_ps(v4, v1);
    __m128 v6 = _mm_mul_ps(v5, v2);
    __m128 v7 = _mm_sub_ps(v6, v3);
    __m128 v8 = _mm_div_ps(v7, v4);
    __m128 v9 = _mm_add_ps(v8, v5);
    __m128 v10 = _mm_mul_ps(v9, v6);
    __m128 v11 = _mm_sub_ps(v10, v7);
    __m128 v12 = _mm_div_ps(v11, v8);
    __m128 v13 = _mm_add_ps(v12, v9);
    __m128 v14 = _mm_mul_ps(v13, v10);
    __m128 v15 = _mm_sub_ps(v14, v11);
    
    /* Unrolled loop creating multiple basic blocks */
    __m128 accum = _mm_setzero_ps();
    
    /* Partially unrolled loop with call at end of block */
    for (int i = 0; i < 8; i += 2) {
        /* First iteration */
        __m128 t1 = _mm_add_ps(v0, _mm_set1_ps(i));
        accum = _mm_add_ps(accum, t1);
        
        /* Second iteration - different computation */
        __m128 t2 = _mm_mul_ps(v1, _mm_set1_ps(i + 1));
        accum = _mm_add_ps(accum, t2);
        
        if (i == 4) {
            /* Call at potential block end */
            asm volatile("# Clobber vector regs" : : : 
                "ymm0", "ymm1", "ymm2", "ymm3",
                "ymm4", "ymm5", "ymm6", "ymm7",
                "ymm8", "ymm9", "ymm10", "ymm11",
                "ymm12", "ymm13", "ymm14", "ymm15");
            baz(v15, accum);
        }
    }
    
    /* Use all vector variables */
    __m128 sum = _mm_add_ps(v0, v1);
    sum = _mm_add_ps(sum, v2);
    sum = _mm_add_ps(sum, v3);
    sum = _mm_add_ps(sum, v4);
    sum = _mm_add_ps(sum, v5);
    sum = _mm_add_ps(sum, v6);
    sum = _mm_add_ps(sum, v7);
    sum = _mm_add_ps(sum, v8);
    sum = _mm_add_ps(sum, v9);
    sum = _mm_add_ps(sum, v10);
    sum = _mm_add_ps(sum, v11);
    sum = _mm_add_ps(sum, v12);
    sum = _mm_add_ps(sum, v13);
    sum = _mm_add_ps(sum, v14);
    sum = _mm_add_ps(sum, v15);
    
    return _mm_add_ps(sum, accum);
}

/* Test 4: Mixed register pressure in nested calls */
NOINLINE double test_mixed_pressure(int i_seed, double d_seed) {
    /* Mixed integer and float variables */
    volatile int iv0 = i_seed;
    volatile double dv0 = d_seed;
    
    int ir1 = iv0 * 2;
    double dr1 = sin(dv0);
    int ir2 = ir1 + 5;
    double dr2 = cos(dr1);
    int ir3 = ir2 ^ 0xABCD;
    double dr3 = dr1 * dr2;
    int ir4 = ir3 << 2;
    double dr4 = dr3 + dv0;
    int ir5 = ir4 - 17;
    double dr5 = exp(dr4);
    
    /* Complex if-else chain with calls at ends */
    if (ir1 > 100) {
        int temp = ir2 + ir3;
        asm volatile("# Mixed clobber 1" : : : 
            "rax", "rbx", "rcx", "xmm0", "xmm1", "xmm2");
        foo();
        return dr1 + dr2 + temp;
    } else if (ir2 < 50) {
        double temp = dr3 * dr4;
        asm volatile("# Mixed clobber 2" : : : 
            "rdx", "rsi", "rdi", "xmm3", "xmm4", "xmm5");
        bar(ir4, temp);
        return dr5 + ir5;
    } else {
        /* This block ends with a call */
        int sum = ir1 + ir2 + ir3 + ir4 + ir5;
        double prod = dr1 * dr2 * dr3 * dr4 * dr5;
        
        /* Force register pressure */
        asm volatile("# Massive clobber" : : : 
            "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
            "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
            "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* Call at block end */
        foo();
        
        /* Use all variables after call */
        use_result = sum + (int)prod;
        return dr1 + dr2 + dr3 + dr4 + dr5 + use_result;
    }
}

/* Main driver that calls all tests */
int main(int argc, char **argv) {
    int total = 0;
    double dtotal = 0.0;
    float fseed = 1.5f;
    
    /* Call all test functions multiple times with different seeds */
    for (int i = 0; i < 10; i++) {
        total += test_integer_pressure(i * 7);
        dtotal += test_float_pressure(i * 0.7);
        
        __m128 vec_result = test_vector_pressure(fseed + i * 0.3f);
        float vec_vals[4];
        _mm_store_ps(vec_vals, vec_result);
        dtotal += vec_vals[0] + vec_vals[1] + vec_vals[2] + vec_vals[3];
        
        dtotal += test_mixed_pressure(i * 11, i * 0.11);
    }
    
    /* Use results to prevent optimization */
    printf("Result: %d, %f\n", total, dtotal);
    return (total > 0) ? 0 : 1;
}
