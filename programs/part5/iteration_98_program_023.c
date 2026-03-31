/* test_caller_save.c - Forces GCC to insert save/restore instructions at call sites */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline, noipa))

/* External function that clobbers registers - defined in separate file */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(__m128, __m256);

/* Helper to use variables after calls */
volatile int use_result;

/* Test 1: Integer register pressure with call at end of basic block */
NOINLINE int test_integer_pressure(int a, int b) {
    /* Create many integer variables that must survive across call */
    register int r0  = a + 1;
    volatile int r1  = r0 * 2;
    register int r2  = r1 + b;
    volatile int r3  = r2 - a;
    register int r4  = r3 * 3;
    volatile int r5  = r4 / 2;
    register int r6  = r5 ^ r0;
    volatile int r7  = r6 | r2;
    register int r8  = r7 & r3;
    volatile int r9  = r8 << 2;
    register int r10 = r9 >> 1;
    volatile int r11 = r10 + r4;
    register int r12 = r11 - r5;
    volatile int r13 = r12 * r6;
    register int r14 = r13 / 7;
    volatile int r15 = r14 ^ r7;
    register int r16 = r15 | r8;
    volatile int r17 = r16 & r9;
    register int r18 = r17 + r10;
    volatile int r19 = r18 - r11;
    register int r20 = r19 * r12;
    
    /* Inline assembly to clobber caller-saved integer registers */
    asm volatile("" : : : 
        "rax", "rcx", "rdx", "rsi", "rdi", 
        "r8", "r9", "r10", "r11", "r12", 
        "r13", "r14", "r15", "xmm0", "xmm1",
        "xmm2", "xmm3", "xmm4", "xmm5"
    );
    
    /* Call at potential end of basic block */
    if (a > b) {
        /* This creates a basic block ending with foo() */
        foo();
        
        /* Use all variables after call to keep them live */
        use_result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
                    r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 + r20;
        return use_result;
    } else {
        /* Alternative path - creates CFG complexity */
        bar(r0, (double)r1);
        return r20;
    }
}

/* Test 2: Floating-point pressure with switch statement */
NOINLINE double test_fp_pressure(double x, double y) {
    double result = 0.0;
    
    /* Many FP variables */
    volatile double d0 = sin(x);
    volatile double d1 = cos(y);
    volatile double d2 = d0 * d1;
    volatile double d3 = d1 / d0;
    volatile double d4 = d2 + d3;
    volatile double d5 = d4 - x;
    volatile double d6 = d5 * y;
    volatile double d7 = sin(d6);
    volatile double d8 = cos(d7);
    volatile double d9 = d8 * d0;
    volatile double d10 = d9 / d1;
    volatile double d11 = d10 + d2;
    volatile double d12 = d11 - d3;
    volatile double d13 = d12 * d4;
    volatile double d14 = d13 / d5;
    volatile double d15 = d14 + d6;
    
    /* Switch creates multiple basic blocks */
    int choice = (int)x % 4;
    switch (choice) {
        case 0:
            /* Call at end of this case's basic block */
            foo();
            result = d0 + d1;
            break;
        case 1:
            bar((int)d2, d3);
            result = d4 + d5;
            break;
        case 2:
            /* More register pressure before call */
            volatile double extra1 = d6 * d7;
            volatile double extra2 = d8 / d9;
            foo();
            result = extra1 + extra2;
            break;
        default:
            /* Call with all variables live */
            asm volatile("" : : : 
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                "xmm6", "xmm7", "xmm8", "xmm9", "xmm10",
                "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
            );
            foo();
            result = d10 + d11 + d12 + d13 + d14 + d15;
    }
    
    /* Use variables to keep them live */
    use_result = (int)(d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 +
                      d10 + d11 + d12 + d13 + d14 + d15);
    return result;
}

#ifdef __SSE__
/* Test 3: Vector register pressure with loop unrolling */
NOINLINE __m128 test_vector_pressure(__m128 a, __m128 b) {
    /* Many vector variables */
    __m128 v0 = _mm_add_ps(a, b);
    __m128 v1 = _mm_mul_ps(v0, a);
    __m128 v2 = _mm_sub_ps(v1, b);
    __m128 v3 = _mm_add_ps(v2, v0);
    __m128 v4 = _mm_mul_ps(v3, v1);
    __m128 v5 = _mm_sub_ps(v4, v2);
    __m128 v6 = _mm_add_ps(v5, v3);
    __m128 v7 = _mm_mul_ps(v6, v4);
    __m128 v8 = _mm_sub_ps(v7, v5);
    __m128 v9 = _mm_add_ps(v8, v6);
    
    /* Partially unrolled loop - creates basic blocks ending with calls */
    float sum = 0.0f;
    for (int i = 0; i < 8; i++) {
        if (i & 1) {
            /* Call at end of basic block inside loop */
            float temp[4];
            _mm_store_ps(temp, v0);
            bar((int)temp[0], temp[1]);
            
            /* Use vector variables */
            __m128 t = _mm_add_ps(v1, v2);
            sum += ((float*)&t)[0];
        } else {
            /* Alternative path */
            foo();
            __m128 t = _mm_add_ps(v3, v4);
            sum += ((float*)&t)[1];
        }
        
        /* Rotate vectors to keep them all live */
        __m128 tmp = v0;
        v0 = v1; v1 = v2; v2 = v3; v3 = v4;
        v4 = v5; v5 = v6; v6 = v7; v7 = v8;
        v8 = v9; v9 = tmp;
    }
    
    /* Clobber all vector registers */
    asm volatile("" : : : 
        "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5",
        "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11",
        "ymm12", "ymm13", "ymm14", "ymm15"
    );
    
    return _mm_set1_ps(sum);
}
#endif

/* Test 4: Mixed pressure in nested conditionals */
NOINLINE int test_mixed_pressure(int x, double y) {
    volatile int i0 = x * 2;
    volatile double d0 = y * 3.14;
    volatile int i1 = i0 + 5;
    volatile double d1 = d0 / 2.0;
    
    /* Complex conditional structure */
    if (x > 0) {
        if (y > 0) {
            volatile int i2 = i1 * 3;
            volatile double d2 = sin(d1);
            
            /* Call at end of inner block */
            foo();
            
            /* Use variables */
            return i2 + (int)d2;
        } else {
            volatile int i3 = i1 / 2;
            volatile double d3 = cos(d1);
            
            /* Another call site */
            bar(i3, d3);
            
            return i3 - (int)d3;
        }
    } else {
        /* Different register pressure profile */
        register int r0 = x + 100;
        register int r1 = r0 * 2;
        register int r2 = r1 - 50;
        
        asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi");
        foo();
        
        return r0 + r1 + r2;
    }
}

/* Main driver that calls all tests */
int main(void) {
    int total = 0;
    
    /* Test 1: Integer pressure */
    total += test_integer_pressure(100, 200);
    
    /* Test 2: FP pressure */
    total += (int)test_fp_pressure(1.0, 2.0);
    
#ifdef __SSE__
    /* Test 3: Vector pressure */
    __m128 a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 b = _mm_set_ps(5.0f, 6.0f, 7.0f, 8.0f);
    __m128 vres = test_vector_pressure(a, b);
    total += (int)((float*)&vres)[0];
#endif
    
    /* Test 4: Mixed pressure */
    total += test_mixed_pressure(50, 3.14159);
    
    printf("Total: %d\n", total);
    return 0;
}
