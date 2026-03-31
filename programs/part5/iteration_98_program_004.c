/* test_caller_save.c - Forces GCC to insert save/restore instructions at call sites */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
__attribute__((noinline)) void foo(void) {
    /* Empty function that compiler cannot inline */
    asm volatile("" : : : "memory");
}

__attribute__((noinline)) void bar(int x) {
    /* Another non-inlinable function */
    volatile int dummy = x;
    (void)dummy;
}

/* Helper function to create register pressure in integer registers */
__attribute__((noinline)) int pressure_integer(int seed) {
    /* Create massive integer register pressure */
    volatile int v0 = seed + 1;
    register int r1 = v0 * 2;
    volatile int v2 = r1 + seed;
    register int r3 = v2 * 3;
    volatile int v4 = r3 - seed;
    register int r5 = v4 / 2;
    volatile int v6 = r5 + 100;
    register int r7 = v6 * 7;
    volatile int v8 = r7 - 50;
    register int r9 = v8 ^ seed;
    volatile int v10 = r9 | 0xFF;
    register int r11 = v10 << 2;
    volatile int v12 = r11 >> 1;
    register int r13 = v12 + 999;
    volatile int v14 = r13 * 2;
    register int r15 = v14 - 333;
    volatile int v16 = r15 & 0xFFFF;
    register int r17 = v16 + 777;
    volatile int v18 = r17 * 3;
    register int r19 = v18 / 5;
    volatile int v20 = r19 + 1234;
    register int r21 = v20 * 2;
    volatile int v22 = r21 - 4321;
    register int r23 = v22 + 9999;
    volatile int v24 = r23 * 7;
    register int r25 = v24 / 11;
    volatile int v26 = r25 + 13579;
    register int r27 = v26 * 13;
    volatile int v28 = r27 - 24680;
    register int r29 = v28 + 35791;
    
    /* Clobber many caller-saved integer registers */
    asm volatile("" : : : 
        "rax", "rcx", "rdx", "rsi", "rdi", 
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
        "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
    );
    
    /* Call at potential block end */
    foo();
    
    /* Use all variables after call to keep them live */
    return v0 + r1 + v2 + r3 + v4 + r5 + v6 + r7 + v8 + r9 + 
           v10 + r11 + v12 + r13 + v14 + r15 + v16 + r17 + v18 + r19 +
           v20 + r21 + v22 + r23 + v24 + r25 + v26 + r27 + v28 + r29;
}

/* Helper function to create register pressure in floating-point registers */
__attribute__((noinline)) double pressure_float(double seed) {
    /* Create massive FP register pressure */
    volatile double d0 = seed + 1.0;
    double d1 = sin(d0);
    volatile double d2 = cos(d1);
    double d3 = tan(d2);
    volatile double d4 = exp(d3);
    double d5 = log(fabs(d4) + 1.0);
    volatile double d6 = d5 * 2.0;
    double d7 = sin(d6) + cos(d6);
    volatile double d8 = d7 * 3.14159;
    double d9 = atan(d8);
    volatile double d10 = d9 * 2.71828;
    double d11 = sqrt(fabs(d10));
    volatile double d12 = d11 + 100.0;
    double d13 = pow(d12, 2.0);
    volatile double d14 = d13 / 7.0;
    double d15 = sin(d14) * cos(d14);
    volatile double d16 = d15 + 999.0;
    double d17 = log10(fabs(d16) + 1.0);
    volatile double d18 = d17 * 3.0;
    double d19 = exp(d18 / 2.0);
    volatile double d20 = d19 - 500.0;
    
    /* Clobber many FP/vector registers */
    asm volatile("" : : : 
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
        "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
        "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7",
        "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15"
    );
    
    /* Call at potential block end */
    foo();
    
    /* Use all variables after call */
    return d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + 
           d10 + d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20;
}

/* Helper function with vector/SIMD register pressure */
__attribute__((noinline)) __m128 pressure_vector(__m128 seed) {
    /* Create vector register pressure */
    volatile __m128 v0 = _mm_add_ps(seed, _mm_set1_ps(1.0f));
    __m128 v1 = _mm_mul_ps(v0, _mm_set1_ps(2.0f));
    volatile __m128 v2 = _mm_sub_ps(v1, seed);
    __m128 v3 = _mm_add_ps(v2, _mm_set1_ps(3.0f));
    volatile __m128 v4 = _mm_mul_ps(v3, _mm_set1_ps(1.5f));
    __m128 v5 = _mm_div_ps(v4, _mm_set1_ps(2.0f));
    volatile __m128 v6 = _mm_add_ps(v5, _mm_set1_ps(100.0f));
    __m128 v7 = _mm_mul_ps(v6, _mm_set1_ps(0.5f));
    volatile __m128 v8 = _mm_sub_ps(v7, _mm_set1_ps(50.0f));
    __m128 v9 = _mm_add_ps(v8, _mm_set1_ps(25.0f));
    
    /* Clobber vector registers */
    asm volatile("" : : : 
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
        "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
    );
    
    /* Call at potential block end */
    foo();
    
    /* Use vectors after call */
    return _mm_add_ps(_mm_add_ps(v0, v1), 
                     _mm_add_ps(_mm_add_ps(v2, v3), 
                               _mm_add_ps(_mm_add_ps(v4, v5), 
                                         _mm_add_ps(_mm_add_ps(v6, v7), 
                                                   _mm_add_ps(v8, v9)))));
}

/* Complex control flow to create basic blocks ending with calls */
__attribute__((noinline)) int complex_control_flow(int x) {
    int result = 0;
    
    /* Switch creates multiple basic blocks */
    switch (x % 5) {
        case 0: {
            /* This block ends with a call after register pressure */
            volatile int a = x + 1;
            volatile int b = a * 2;
            volatile int c = b + x;
            volatile int d = c * 3;
            volatile int e = d - x;
            
            /* Clobber registers */
            asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi");
            
            /* Call at block end */
            bar(a + b + c + d + e);
            
            result = a;
            break;
        }
        case 1: {
            /* Different register pressure profile */
            volatile double f = sin(x);
            volatile double g = cos(x);
            volatile double h = f * g;
            
            asm volatile("" : : : "xmm0", "xmm1", "xmm2", "xmm3");
            
            /* Call at block end */
            foo();
            
            result = (int)(f + g + h);
            break;
        }
        case 2: {
            /* Loop with partial unrolling creates block ending with call */
            volatile int sum = 0;
            for (int i = 0; i < 3; i++) {
                volatile int t1 = x + i;
                volatile int t2 = t1 * i;
                volatile int t3 = t2 + x;
                
                if (i == 2) {
                    /* Call at end of loop body block */
                    asm volatile("" : : : "rax", "rcx", "rdx");
                    bar(t3);
                }
                sum += t3;
            }
            result = sum;
            break;
        }
        case 3: {
            /* Nested if-else structure */
            volatile int p = x * x;
            if (p > 100) {
                volatile int q = p / 2;
                volatile int r = q + 50;
                
                asm volatile("" : : : "r8", "r9", "r10");
                
                /* Call at end of if block */
                foo();
                
                result = r;
            } else {
                volatile int s = p * 3;
                result = s;
            }
            break;
        }
        default: {
            /* Default case with mixed pressure */
            volatile int u = x + 100;
            volatile double v = sin(u);
            __m128 w = _mm_set1_ps((float)v);
            
            asm volatile("" : : : "rax", "xmm0", "xmm1", "xmm2");
            
            /* Call at block end */
            bar(u);
            
            float wsum;
            _mm_store_ss(&wsum, w);
            result = u + (int)wsum;
            break;
        }
    }
    
    return result;
}

/* Function with call at tail of basic block due to early return */
__attribute__((noinline)) int tail_call_block(int x, int y) {
    volatile int a = x * y;
    volatile int b = a + 100;
    volatile int c = b * 2;
    
    if (x > y) {
        volatile int d = c - 50;
        volatile int e = d / 3;
        
        /* Massive register pressure */
        register int r0 = e + 1, r1 = r0 * 2, r2 = r1 + e, r3 = r2 * 3;
        register int r4 = r3 - e, r5 = r4 / 2, r6 = r5 + 100, r7 = r6 * 7;
        register int r8 = r7 - 50, r9 = r8 ^ e, r10 = r9 | 0xFF;
        
        /* Clobber all caller-saved registers */
        asm volatile("" : : : 
            "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
            "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
        );
        
        /* Call at end of basic block before return */
        foo();
        
        return r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
    }
    
    return c;
}

int main(void) {
    int total = 0;
    
    /* Test 1: Integer register pressure */
    total += pressure_integer(42);
    
    /* Test 2: Floating-point register pressure */
    total += (int)pressure_float(3.14159);
    
    /* Test 3: Vector register pressure */
    __m128 vec = _mm_set1_ps(1.0f);
    __m128 vec_result = pressure_vector(vec);
    float vec_sum;
    _mm_store_ss(&vec_sum, vec_result);
    total += (int)vec_sum;
    
    /* Test 4: Complex control flow */
    for (int i = 0; i < 10; i++) {
        total += complex_control_flow(i);
    }
    
    /* Test 5: Tail call blocks */
    total += tail_call_block(100, 50);
    total += tail_call_block(50, 100);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return 0;
}
