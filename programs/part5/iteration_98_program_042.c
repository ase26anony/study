/* test_caller_save.c */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline, noipa))

/* External function that clobbers registers */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(__m128);

/* Helper functions in separate compilation unit */
extern void helper1(void);
extern void helper2(void);

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* Function to create integer register pressure */
NOINLINE int test_integer_pressure(int a, int b, int c) {
    /* Create many integer live variables across a call */
    volatile int v0 = a + 1;
    register int r1 = v0 * 2 + b;
    register int r2 = r1 + c * 3;
    volatile int v3 = r2 - a;
    register int r4 = v3 * 5;
    register int r5 = r4 + b * 7;
    volatile int v6 = r5 - c;
    register int r7 = v6 * 11;
    register int r8 = r7 + a * 13;
    volatile int v9 = r8 - b;
    register int r10 = v9 * 17;
    register int r11 = r10 + c * 19;
    volatile int v12 = r11 - a;
    register int r13 = v12 * 23;
    register int r14 = r13 + b * 29;
    volatile int v15 = r14 - c;
    register int r16 = v15 * 31;
    register int r17 = r16 + a * 37;
    volatile int v18 = r17 - b;
    register int r19 = v18 * 41;
    register int r20 = r19 + c * 43;
    
    /* Inline assembly to clobber caller-saved integer registers */
    asm volatile("" : : : 
        "rax", "rcx", "rdx", "rsi", "rdi", 
        "r8", "r9", "r10", "r11", "r12", 
        "r13", "r14", "r15", "xmm0", "xmm1",
        "xmm2", "xmm3", "xmm4", "xmm5", "xmm6",
        "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
        "xmm12", "xmm13", "xmm14", "xmm15");
    
    /* Call at potential block end */
    if (global_counter > 100) {
        /* This creates a basic block ending with foo() */
        foo();  /* Non-inline call */
        
        /* Use all the variables after call */
        return v0 + r1 + r2 + v3 + r4 + r5 + v6 + r7 + r8 + v9 +
               r10 + r11 + v12 + r13 + r14 + v15 + r16 + r17 + v18 +
               r19 + r20;
    } else {
        /* Alternative path to create CFG */
        helper1();
        return a + b + c;
    }
}

/* Function to create FP register pressure */
NOINLINE double test_fp_pressure(double x, double y, double z) {
    /* Many FP calculations */
    volatile double d0 = sin(x);
    register double d1 = cos(y) * d0;
    register double d2 = tan(z) + d1;
    volatile double d3 = exp(x * y) - d2;
    register double d4 = log(fabs(z)) * d3;
    register double d5 = pow(x, 2.0) + d4;
    volatile double d6 = sqrt(y * z) - d5;
    register double d7 = sin(x * y) * d6;
    register double d8 = cos(y * z) + d7;
    volatile double d9 = tan(x * z) - d8;
    register double d10 = exp(x + y) * d9;
    register double d11 = log(fabs(x - z)) + d10;
    volatile double d12 = pow(y, 3.0) - d11;
    register double d13 = sqrt(x * x + y * y) * d12;
    register double d14 = sin(z * z) + d13;
    volatile double d15 = cos(x * y * z) - d14;
    
    /* Complex control flow with switch */
    int choice = global_counter % 4;
    double result = 0.0;
    
    switch (choice) {
        case 0:
            /* Call at end of this case's block */
            bar(choice, d0);
            result = d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 +
                    d10 + d11 + d12 + d13 + d14 + d15;
            break;
            
        case 1:
            helper2();
            result = x + y + z;
            break;
            
        case 2:
            /* Another call site with pressure */
            foo();
            result = d1 * d3 * d5 * d7 * d9 * d11 * d13 * d15;
            break;
            
        default:
            /* Loop with unrolled call at end */
            for (int i = 0; i < 3; i++) {
                if (i == 2) {
                    /* Call at end of loop body block */
                    bar(i, d2);
                }
                result += d0 * i;
            }
            break;
    }
    
    return result;
}

/* Function to create vector register pressure */
NOINLINE __m128 test_vector_pressure(__m128 a, __m128 b, __m128 c) {
    /* Many vector operations */
    volatile __m128 v0 = _mm_add_ps(a, b);
    register __m128 v1 = _mm_mul_ps(v0, c);
    register __m128 v2 = _mm_sub_ps(v1, a);
    volatile __m128 v3 = _mm_add_ps(v2, b);
    register __m128 v4 = _mm_mul_ps(v3, c);
    register __m128 v5 = _mm_sub_ps(v4, a);
    volatile __m128 v6 = _mm_add_ps(v5, b);
    register __m128 v7 = _mm_mul_ps(v6, c);
    register __m128 v8 = _mm_sub_ps(v7, a);
    volatile __m128 v9 = _mm_add_ps(v8, b);
    register __m128 v10 = _mm_mul_ps(v9, c);
    register __m128 v11 = _mm_sub_ps(v10, a);
    
    /* Create basic block ending with call */
    if (_mm_movemask_ps(a) > 0) {
        /* Inline assembly clobbering vector registers */
        asm volatile("" : : : 
            "ymm0", "ymm1", "ymm2", "ymm3", "ymm4",
            "ymm5", "ymm6", "ymm7", "ymm8", "ymm9",
            "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15");
        
        /* Call at block end */
        baz(v0);
        
        /* Use vectors after call */
        return _mm_add_ps(_mm_add_ps(v0, v1), 
                         _mm_add_ps(_mm_add_ps(v2, v3),
                                   _mm_add_ps(_mm_add_ps(v4, v5),
                                             _mm_add_ps(_mm_add_ps(v6, v7),
                                                       _mm_add_ps(_mm_add_ps(v8, v9),
                                                                 _mm_add_ps(v10, v11))))));
    } else {
        return _mm_setzero_ps();
    }
}

/* Function with mixed pressure in loop */
NOINLINE double test_mixed_pressure(int n) {
    double sum = 0.0;
    volatile double base = 1.234;
    
    /* Unrolled loop to create multiple basic blocks */
    for (int i = 0; i < n; i++) {
        /* Integer pressure */
        register int i0 = i * 2;
        volatile int i1 = i0 + 1;
        register int i2 = i1 * 3;
        
        /* FP pressure */
        volatile double d0 = sin(base * i);
        register double d1 = cos(d0 * i);
        
        /* Vector pressure if available */
        __m128 vec = _mm_set_ps(i, i+1, i+2, i+3);
        volatile __m128 vsum = _mm_add_ps(vec, vec);
        
        if (i % 4 == 0) {
            /* Call at end of this block */
            bar(i0, d0);
            
            /* Use variables after call */
            sum += i0 + i1 + i2 + d0 + d1 + 
                   ((float*)&vsum)[0] + ((float*)&vsum)[1];
        } else if (i % 4 == 1) {
            foo();
            sum -= d1;
        } else if (i % 4 == 2) {
            helper1();
            sum *= 1.1;
        } else {
            /* Last case - call at end of block */
            baz(vec);
            sum /= 2.0;
        }
    }
    
    return sum;
}

int main(void) {
    int total = 0;
    double fp_total = 0.0;
    
    /* Test integer pressure */
    for (int i = 0; i < 10; i++) {
        total += test_integer_pressure(i, i+1, i+2);
        global_counter++;
    }
    
    /* Test FP pressure */
    for (int i = 0; i < 5; i++) {
        fp_total += test_fp_pressure(i * 0.1, i * 0.2, i * 0.3);
        global_counter++;
    }
    
    /* Test vector pressure */
    __m128 vec_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 vec_b = _mm_set_ps(5.0f, 6.0f, 7.0f, 8.0f);
    __m128 vec_c = _mm_set_ps(9.0f, 10.0f, 11.0f, 12.0f);
    __m128 vec_result = test_vector_pressure(vec_a, vec_b, vec_c);
    
    /* Test mixed pressure */
    double mixed_result = test_mixed_pressure(20);
    
    /* Prevent dead code elimination */
    volatile int dummy = total;
    volatile double dummy_fp = fp_total + mixed_result + 
                              ((float*)&vec_result)[0];
    
    printf("Result: %d, %f\n", dummy, dummy_fp);
    return 0;
}
