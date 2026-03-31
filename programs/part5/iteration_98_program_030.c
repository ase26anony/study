/* test_caller_save.c - Forces GCC to insert save/restore instructions at call sites */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline, noipa))

/* External function declarations - cannot be inlined */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE double compute(double, double);

/* Helper to create register pressure */
#define CONCAT(a, b) a##b
#define VAR(num) CONCAT(var, num)

/* ==================== Test Function 1: Integer Register Pressure ==================== */
NOINLINE int test_integer_pressure(int seed) {
    /* Create massive integer register pressure */
    volatile int a = seed;
    register int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
    register int r10, r11, r12, r13, r14, r15, r16, r17, r18, r19;
    
    /* Complex computation chain that must survive across call */
    r0 = a + 1;
    r1 = r0 * 2 - a;
    r2 = r1 << 3;
    r3 = r2 ^ r1;
    r4 = r3 + r0 - r2;
    r5 = r4 * 3;
    r6 = r5 / 2;
    r7 = r6 | r5;
    r8 = r7 & r4;
    r9 = r8 ^ r3;
    r10 = r9 + r2;
    r11 = r10 * r1;
    r12 = r11 >> 2;
    r13 = r12 - r0;
    r14 = r13 * 5;
    r15 = r14 + r7;
    r16 = r15 ^ r8;
    r17 = r16 | r9;
    r18 = r17 & r10;
    r19 = r18 - r11;
    
    /* Use control flow to create basic block ending with call */
    if (seed > 100) {
        /* This creates a basic block that ends with the call to foo() */
        
        /* Clobber caller-saved registers with inline asm */
        asm volatile("" : : : 
            "rax", "rcx", "rdx", "rsi", "rdi", 
            "r8", "r9", "r10", "r11", "r12", 
            "r13", "r14", "r15", "xmm0", "xmm1",
            "xmm2", "xmm3", "xmm4", "xmm5", "xmm6",
            "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
            "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* Call at the end of basic block - forces potential BB_END update */
        foo();
        
        /* Use all the register values after call */
        return r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
               r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19;
    } else {
        /* Different path to create CFG complexity */
        return seed * 2;
    }
}

/* ==================== Test Function 2: Floating Point Pressure ==================== */
NOINLINE double test_float_pressure(double seed) {
    /* Create massive FP register pressure */
    volatile double d = seed;
    double f0, f1, f2, f3, f4, f5, f6, f7, f8, f9;
    double f10, f11, f12, f13, f14, f15, f16, f17, f18, f19;
    
    /* Complex FP computation chain */
    f0 = sin(d);
    f1 = cos(d);
    f2 = f0 * f1;
    f3 = f2 + d;
    f4 = sin(f3);
    f5 = cos(f4);
    f6 = f5 * f4;
    f7 = f6 / f3;
    f8 = exp(f7);
    f9 = log(f8 + 1.0);
    f10 = f9 * f8;
    f11 = sqrt(f10);
    f12 = f11 + f9;
    f13 = sin(f12);
    f14 = cos(f13);
    f15 = f14 * f13;
    f16 = f15 / f12;
    f17 = exp(f16);
    f18 = log(f17 + 1.0);
    f19 = f18 * f17;
    
    /* Switch statement to create multiple basic blocks */
    switch ((int)seed % 4) {
        case 0: {
            /* This case ends with a call at block end */
            
            /* Clobber FP registers */
            asm volatile("" : : : 
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15");
            
            /* Call with FP arguments */
            bar((int)seed, f0);
            
            /* Use FP values after call */
            return f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 +
                   f10 + f11 + f12 + f13 + f14 + f15 + f16 + f17 + f18 + f19;
        }
        case 1:
            return f0 * 2.0;
        case 2:
            return f1 * 3.0;
        default:
            return f2 * 4.0;
    }
}

/* ==================== Test Function 3: Mixed Register Pressure ==================== */
NOINLINE double test_mixed_pressure(int i_seed, double d_seed) {
    /* Mixed integer and FP pressure */
    volatile int vi = i_seed;
    volatile double vd = d_seed;
    
    /* Integer variables */
    int i0 = vi + 1, i1 = i0 * 2, i2 = i1 + 3, i3 = i2 * 4, i4 = i3 - 5;
    int i5 = i4 / 2, i6 = i5 | i4, i7 = i6 & i3, i8 = i7 ^ i2, i9 = i8 << 1;
    
    /* Floating point variables */
    double d0 = sin(vd), d1 = cos(vd), d2 = d0 * d1, d3 = d2 + vd, d4 = exp(d3);
    double d5 = log(d4), d6 = sqrt(d5), d7 = d6 * d5, d8 = d7 / d4, d9 = sin(d8);
    
    /* Loop with partial unrolling - creates basic blocks ending with calls */
    double sum = 0.0;
    for (int j = 0; j < 3; j++) {
        if (j == 1) {
            /* This iteration creates a block ending with call */
            
            /* Massive clobber list */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15", "mm0", "mm1",
                "mm2", "mm3", "mm4", "mm5", "mm6", "mm7");
            
            /* Call at potential block end */
            double result = compute(d0, d1);
            
            /* Use both integer and FP values after call */
            sum += result + i0 + i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9;
        } else {
            sum += d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9;
        }
    }
    
    return sum;
}

/* ==================== Test Function 4: Vector Pressure (if available) ==================== */
#ifdef __SSE2__
#include <emmintrin.h>
NOINLINE __m128d test_vector_pressure(double a, double b) {
    /* Create SSE vector register pressure */
    __m128d v0 = _mm_set_pd(a, b);
    __m128d v1 = _mm_add_pd(v0, v0);
    __m128d v2 = _mm_mul_pd(v1, v0);
    __m128d v3 = _mm_sub_pd(v2, v1);
    __m128d v4 = _mm_add_pd(v3, v2);
    __m128d v5 = _mm_mul_pd(v4, v3);
    __m128d v6 = _mm_sub_pd(v5, v4);
    __m128d v7 = _mm_add_pd(v6, v5);
    __m128d v8 = _mm_mul_pd(v7, v6);
    __m128d v9 = _mm_sub_pd(v8, v7);
    
    /* Nested if-else to create complex CFG */
    if (a > 0.0) {
        if (b > 0.0) {
            /* Block ending with call */
            asm volatile("" : : : "xmm0", "xmm1", "xmm2", "xmm3", 
                                   "xmm4", "xmm5", "xmm6", "xmm7");
            foo();
            
            /* Use vector values after call */
            __m128d result = _mm_add_pd(v0, v1);
            result = _mm_add_pd(result, v2);
            result = _mm_add_pd(result, v3);
            result = _mm_add_pd(result, v4);
            result = _mm_add_pd(result, v5);
            result = _mm_add_pd(result, v6);
            result = _mm_add_pd(result, v7);
            result = _mm_add_pd(result, v8);
            return _mm_add_pd(result, v9);
        } else {
            return _mm_add_pd(v0, v1);
        }
    } else {
        return _mm_add_pd(v2, v3);
    }
}
#endif

/* ==================== Main Function ==================== */
int main(void) {
    int total = 0;
    double dtotal = 0.0;
    
    /* Call all test functions to trigger different register pressure scenarios */
    total += test_integer_pressure(150);  /* Takes if branch with call at block end */
    total += test_integer_pressure(50);   /* Takes else branch */
    
    dtotal += test_float_pressure(1.5);   /* Case 0 with call at block end */
    dtotal += test_float_pressure(2.5);   /* Case 1 */
    dtotal += test_float_pressure(3.5);   /* Case 2 */
    dtotal += test_float_pressure(4.5);   /* Case 3 */
    
    dtotal += test_mixed_pressure(100, 2.0);  /* Loop with call in middle iteration */
    
    #ifdef __SSE2__
    __m128d vec_result = test_vector_pressure(1.0, 2.0);
    double vec_array[2];
    _mm_store_pd(vec_array, vec_result);
    dtotal += vec_array[0] + vec_array[1];
    #endif
    
    /* Prevent dead code elimination */
    volatile int final_result = total + (int)dtotal;
    printf("Result: %d\n", final_result);
    
    return 0;
}
