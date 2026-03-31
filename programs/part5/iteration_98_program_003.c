/* test_caller_save.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline))

/* External function declarations */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(__m128);

/* Helper functions in separate compilation unit */
extern void helper1(void);
extern void helper2(int);
extern double helper3(double);

/* Global volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile double global_double = 3.14159;

/* Function 1: Heavy integer register pressure with call at block end */
NOINLINE int test_integer_pressure(int seed) {
    /* Create many integer variables that must survive across call */
    register int r0 = seed + 1;
    volatile int v0 = r0 * 2;
    register int r1 = v0 + seed;
    volatile int v1 = r1 * 3;
    register int r2 = v1 - seed;
    volatile int v2 = r2 / 2;
    register int r3 = v2 | seed;
    volatile int v3 = r3 ^ 0xABCD;
    register int r4 = v3 & 0xFF;
    volatile int v4 = r4 << 2;
    register int r5 = v4 >> 1;
    volatile int v5 = r5 + 100;
    register int r6 = v5 * 2;
    volatile int v6 = r6 - 50;
    register int r7 = v6 % 17;
    volatile int v7 = r7 | 0x1000;
    register int r8 = v7 + seed;
    volatile int v8 = r8 * 2;
    register int r9 = v8 - 1;
    volatile int v9 = r9 ^ seed;
    register int r10 = v9 + 1000;
    volatile int v10 = r10;
    register int r11 = v10 * 3;
    volatile int v11 = r11;
    register int r12 = v11 - 500;
    volatile int v12 = r12;
    register int r13 = v12 | 0xFFFF;
    volatile int v13 = r13;
    register int r14 = v13 & 0x7FFF;
    volatile int v14 = r14;
    register int r15 = v14 + seed;
    volatile int v15 = r15;
    
    /* Complex control flow to create basic block ending with call */
    if (seed % 3 == 0) {
        /* This block ends with the call to foo() */
        int temp = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + 
                   r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
        
        /* Inline assembly to clobber caller-saved integer registers */
        asm volatile("" : : : 
            "rax", "rcx", "rdx", "rsi", "rdi", 
            "r8", "r9", "r10", "r11", "r12", 
            "r13", "r14", "r15", "xmm0", "xmm1",
            "xmm2", "xmm3", "xmm4", "xmm5", "xmm6",
            "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
            "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* Call at potential block end */
        foo();
        
        /* Use all variables after call to keep them live */
        return temp + v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + 
               v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    } else if (seed % 3 == 1) {
        /* Different path */
        return r0 + r1;
    } else {
        /* Another path */
        return r2 + r3;
    }
}

/* Function 2: Heavy floating-point register pressure */
NOINLINE double test_fp_pressure(double seed) {
    /* Many FP variables */
    volatile double d0 = sin(seed);
    volatile double d1 = cos(seed);
    volatile double d2 = tan(seed);
    volatile double d3 = exp(seed);
    volatile double d4 = log(seed + 1.0);
    volatile double d5 = sqrt(seed + 2.0);
    volatile double d6 = d0 * d1;
    volatile double d7 = d2 + d3;
    volatile double d8 = d4 - d5;
    volatile double d9 = d6 * d7;
    volatile double d10 = d8 / d9;
    volatile double d11 = sin(d10);
    volatile double d12 = cos(d11);
    volatile double d13 = d11 * d12;
    volatile double d14 = d13 + seed;
    volatile double d15 = d14 * 2.0;
    volatile double d16 = d15 - 1.0;
    volatile double d17 = sin(d16);
    volatile double d18 = cos(d17);
    volatile double d19 = d17 + d18;
    volatile double d20 = d19 * 3.14159;
    
    /* Switch statement to create complex CFG */
    switch ((int)seed % 4) {
        case 0: {
            /* Call at end of this case block */
            double sum = d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 +
                        d10 + d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20;
            
            /* Clobber FP registers */
            asm volatile("" : : : 
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15",
                "rax", "rcx", "rdx");
            
            bar((int)seed, sum);
            
            /* Use results after call */
            return sum + d0 - d1 + d2 - d3;
        }
        case 1:
            return d0 + d1;
        case 2:
            return d2 + d3;
        default:
            return d4 + d5;
    }
}

/* Function 3: Vector register pressure */
NOINLINE __m128 test_vector_pressure(float seed) {
    /* Many vector variables */
    __m128 v0 = _mm_set1_ps(seed);
    __m128 v1 = _mm_set1_ps(seed * 2.0f);
    __m128 v2 = _mm_set1_ps(seed * 3.0f);
    __m128 v3 = _mm_add_ps(v0, v1);
    __m128 v4 = _mm_mul_ps(v1, v2);
    __m128 v5 = _mm_sub_ps(v3, v4);
    __m128 v6 = _mm_set1_ps(seed * 4.0f);
    __m128 v7 = _mm_set1_ps(seed * 5.0f);
    __m128 v8 = _mm_add_ps(v5, v6);
    __m128 v9 = _mm_mul_ps(v7, v8);
    __m128 v10 = _mm_set1_ps(seed * 6.0f);
    __m128 v11 = _mm_set1_ps(seed * 7.0f);
    __m128 v12 = _mm_add_ps(v9, v10);
    __m128 v13 = _mm_mul_ps(v11, v12);
    __m128 v14 = _mm_set1_ps(seed * 8.0f);
    __m128 v15 = _mm_set1_ps(seed * 9.0f);
    
    /* Loop with call at end of unrolled iteration */
    float result[4] = {0};
    for (int i = 0; i < 2; i++) {
        if (i == 0) {
            /* First iteration - heavy register pressure */
            __m128 temp = _mm_add_ps(v0, _mm_add_ps(v1, _mm_add_ps(v2, v3)));
            temp = _mm_add_ps(temp, _mm_add_ps(v4, _mm_add_ps(v5, v6)));
            temp = _mm_add_ps(temp, _mm_add_ps(v7, _mm_add_ps(v8, v9)));
            temp = _mm_add_ps(temp, _mm_add_ps(v10, _mm_add_ps(v11, v12)));
            temp = _mm_add_ps(temp, _mm_add_ps(v13, _mm_add_ps(v14, v15)));
            
            /* Clobber vector registers */
            asm volatile("" : : : 
                "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5",
                "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11",
                "ymm12", "ymm13", "ymm14", "ymm15");
            
            /* Call at potential block end */
            baz(temp);
            
            _mm_store_ps(result, temp);
        } else {
            /* Second iteration */
            __m128 temp2 = _mm_mul_ps(v0, v1);
            _mm_store_ps(result, temp2);
        }
    }
    
    return _mm_load_ps(result);
}

/* Function 4: Mixed register pressure with nested calls */
NOINLINE double test_mixed_pressure(int i_seed, double d_seed) {
    volatile int ivars[10];
    volatile double dvars[10];
    __m128 vvars[5];
    
    /* Initialize many variables of different types */
    for (int i = 0; i < 10; i++) {
        ivars[i] = i_seed + i;
        dvars[i] = d_seed * i;
        if (i < 5) {
            vvars[i] = _mm_set1_ps((float)(d_seed + i));
        }
    }
    
    /* Complex control flow with multiple basic blocks */
    double result = 0.0;
    
    if (i_seed > 100) {
        /* Block with call at end */
        for (int i = 0; i < 10; i++) {
            result += ivars[i] + dvars[i];
            if (i < 5) {
                float v[4];
                _mm_store_ps(v, vvars[i]);
                result += v[0] + v[1] + v[2] + v[3];
            }
        }
        
        /* Massive clobber list */
        asm volatile("" : : : 
            "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
            "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
            "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
            "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
            "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7",
            "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15");
        
        /* Call external helper */
        helper2(i_seed);
        
        return result;
    } else {
        /* Different path without call at block end */
        return d_seed * 2.0;
    }
}

/* Main function that calls all test cases */
int main(void) {
    int total = 0;
    double fp_total = 0.0;
    
    /* Call test functions multiple times with different seeds */
    for (int i = 0; i < 100; i++) {
        total += test_integer_pressure(i);
        fp_total += test_fp_pressure((double)i + 0.5);
        
        __m128 vec_result = test_vector_pressure((float)i * 0.1f);
        float vec_floats[4];
        _mm_store_ps(vec_floats, vec_result);
        fp_total += vec_floats[0] + vec_floats[1] + vec_floats[2] + vec_floats[3];
        
        fp_total += test_mixed_pressure(i, (double)i * 0.25);
        
        /* Call helpers to increase pressure */
        helper1();
        helper3(fp_total);
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d, %f\n", total, fp_total);
    return total > 0 ? 0 : 1;
}

/* Dummy function definitions to satisfy linker */
NOINLINE void foo(void) {
    global_counter++;
}

NOINLINE void bar(int x, double y) {
    global_double += y + x;
}

NOINLINE void baz(__m128 v) {
    float f[4];
    _mm_store_ps(f, v);
    global_double += f[0] + f[1] + f[2] + f[3];
}
