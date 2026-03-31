/* test_caller_save.c - Forces GCC to insert save/restore instructions at call sites */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline,noipa))

/* External function that clobbers registers - defined in separate file */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(__m128, __m128);

/* Helper to use variables and prevent optimization */
#define USE(var) asm volatile("" : : "r"(var) :)

/* Test 1: Integer register pressure with call at block end */
NOINLINE int test_integer_pressure(int seed) {
    volatile int trigger = seed; /* Force memory access */
    
    /* Create many integer live variables across a call */
    int r0 = trigger + 1;
    int r1 = r0 * 2 + trigger;
    int r2 = r1 - r0 * 3;
    int r3 = r2 ^ r1;
    int r4 = r3 | (r2 << 2);
    int r5 = r4 % (r3 + 1);
    int r6 = r5 & ~r4;
    int r7 = r6 + r5 - r4;
    int r8 = r7 * 2 - r6;
    int r9 = r8 / (r7 + 1);
    int r10 = r9 ^ r8;
    int r11 = r10 | r9;
    int r12 = r11 - r10;
    int r13 = r12 * 3;
    int r14 = r13 + r12;
    int r15 = r14 - r13;
    int r16 = r15 ^ r14;
    int r17 = r16 | r15;
    int r18 = r17 + 1;
    int r19 = r18 * 2;
    int r20 = r19 - r18;
    
    /* Use control flow to create basic block ending with call */
    if (trigger > 0) {
        /* Additional pressure in this block */
        int extra1 = r20 + r19;
        int extra2 = extra1 * r18;
        int extra3 = extra2 ^ extra1;
        
        /* Inline assembly to clobber caller-saved integer registers */
        asm volatile("" : : : 
            "rax", "rcx", "rdx", "rsi", "rdi", 
            "r8", "r9", "r10", "r11", "r12",
            "r13", "r14", "r15", "xmm0", "xmm1",
            "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        /* Call at potential block end */
        foo();
        
        /* Use variables after call - must be preserved */
        r0 += extra3;
        r1 -= extra2;
    } else {
        /* Alternative path to create CFG complexity */
        r0 = 0;
    }
    
    /* Use all variables to prevent optimization */
    int sum = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
              r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 + r20;
    
    USE(sum);
    return sum;
}

/* Test 2: Floating-point pressure with switch statement */
NOINLINE double test_float_pressure(double seed) {
    volatile double vseed = seed;
    
    /* Many floating-point live variables */
    double f0 = sin(vseed);
    double f1 = cos(f0);
    double f2 = f0 * f1 + vseed;
    double f3 = f2 / (f1 + 1.0);
    double f4 = f3 * f2 - f1;
    double f5 = sin(f4) + cos(f3);
    double f6 = f5 * 2.0;
    double f7 = f6 / 3.14159;
    double f8 = exp(f7);
    double f9 = log(f8 + 1.0);
    double f10 = f9 * f8;
    double f11 = sqrt(f10);
    double f12 = f11 + f10;
    double f13 = f12 * 0.5;
    double f14 = sin(f13) * cos(f13);
    double f15 = f14 * 2.0;
    double f16 = f15 - f14;
    double f17 = f16 / 3.0;
    double f18 = exp(f17);
    double f19 = log(f18);
    double f20 = f19 * 10.0;
    
    /* Switch creates multiple basic blocks */
    int choice = (int)vseed % 4;
    double result = 0.0;
    
    switch (choice) {
        case 0:
            /* Call in middle of case */
            bar((int)f0, f1);
            result = f0 + f1;
            break;
        case 1:
            /* High pressure then call at block end */
            {
                double temp = f2 * f3 + f4;
                asm volatile("" : : : 
                    "xmm0", "xmm1", "xmm2", "xmm3", "xmm4",
                    "xmm5", "xmm6", "xmm7", "xmm8", "xmm9",
                    "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
                );
                foo();  /* Potential block end */
                result = temp + f5;
            }
            break;
        case 2:
            result = f6 + f7;
            break;
        default:
            /* Loop with call at end of iteration */
            for (int i = 0; i < 3; i++) {
                double loop_var = f8 + i;
                if (i == 1) {
                    bar((int)loop_var, f9);
                    /* Call may be at block end here */
                }
                result += loop_var;
            }
            break;
    }
    
    /* Use all floats after call */
    double total = f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 +
                   f10 + f11 + f12 + f13 + f14 + f15 + f16 + f17 + f18 + f19 + f20 + result;
    
    USE(total);
    return total;
}

/* Test 3: Vector/SIMD register pressure */
NOINLINE __m128 test_vector_pressure(float seed) {
    /* Create many vector variables */
    __m128 v0 = _mm_set1_ps(seed);
    __m128 v1 = _mm_add_ps(v0, _mm_set1_ps(1.0f));
    __m128 v2 = _mm_mul_ps(v1, _mm_set1_ps(2.0f));
    __m128 v3 = _mm_sub_ps(v2, v1);
    __m128 v4 = _mm_add_ps(v3, v0);
    __m128 v5 = _mm_mul_ps(v4, _mm_set1_ps(0.5f));
    __m128 v6 = _mm_add_ps(v5, v4);
    __m128 v7 = _mm_sub_ps(v6, v5);
    __m128 v8 = _mm_mul_ps(v7, _mm_set1_ps(3.0f));
    __m128 v9 = _mm_add_ps(v8, v7);
    __m128 v10 = _mm_set1_ps(seed * 2.0f);
    __m128 v11 = _mm_add_ps(v10, v9);
    __m128 v12 = _mm_mul_ps(v11, v10);
    __m128 v13 = _mm_sub_ps(v12, v11);
    __m128 v14 = _mm_add_ps(v13, v12);
    
    /* Complex control flow with nested ifs */
    if (seed > 0.5f) {
        __m128 temp1 = _mm_add_ps(v0, v1);
        if (seed < 1.0f) {
            __m128 temp2 = _mm_mul_ps(temp1, v2);
            
            /* Clobber vector registers */
            asm volatile("" : : : 
                "ymm0", "ymm1", "ymm2", "ymm3", "ymm4",
                "ymm5", "ymm6", "ymm7", "ymm8", "ymm9",
                "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15"
            );
            
            /* Call at potential block end */
            baz(temp2, v3);
            
            v4 = _mm_add_ps(v4, temp2);
        } else {
            v4 = _mm_sub_ps(v4, temp1);
        }
    }
    
    /* Use all vectors */
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
    
    return sum;
}

/* Test 4: Mixed pressure in loop with partial unrolling */
NOINLINE double test_mixed_pressure(int iterations) {
    double acc = 0.0;
    int int_acc = 0;
    
    /* Manually unrolled loop */
    for (int i = 0; i < iterations; i++) {
        /* Integer variables */
        int i0 = i * 2;
        int i1 = i0 + 1;
        int i2 = i1 * i0;
        int i3 = i2 ^ i1;
        int i4 = i3 | i2;
        
        /* Float variables */
        double f0 = sin(i * 0.1);
        double f1 = cos(f0);
        double f2 = f0 * f1;
        
        /* Vector variable */
        __m128 v0 = _mm_set1_ps(i * 0.01f);
        
        if (i % 2 == 0) {
            /* Call in one path of conditional */
            asm volatile("" : : : 
                "rax", "rcx", "rdx", "xmm0", "xmm1", "xmm2", "xmm3"
            );
            
            foo();  /* May be at block end */
            
            /* Use variables after call */
            i0 += 1;
            f0 += 1.0;
        }
        
        /* Accumulate results */
        int_acc += i0 + i1 + i2 + i3 + i4;
        
        float vsum[4];
        _mm_store_ps(vsum, v0);
        acc += f0 + f1 + f2 + vsum[0] + vsum[1] + vsum[2] + vsum[3];
    }
    
    return acc + int_acc;
}

int main(void) {
    int total = 0;
    double ftotal = 0.0;
    
    /* Run all tests to create multiple call sites with different pressures */
    total += test_integer_pressure(42);
    ftotal += test_float_pressure(3.14159);
    
    __m128 vresult = test_vector_pressure(0.75f);
    float vsum[4];
    _mm_store_ps(vsum, vresult);
    ftotal += vsum[0] + vsum[1] + vsum[2] + vsum[3];
    
    ftotal += test_mixed_pressure(10);
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d, %f\n", total, ftotal);
    
    return 0;
}
