/* test_caller_save.c */
#include <math.h>
#include <stdio.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline))

/* External function that clobbers registers */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(__m128);

/* Helper function in separate compilation unit */
void external_func(void);

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* Function 1: Heavy integer register pressure with call at block end */
NOINLINE int test_integer_pressure(int a, int b, int c) {
    volatile int v0 = a;
    register int r0 = v0 + 1;
    register int r1 = r0 * b;
    register int r2 = r1 + c;
    register int r3 = r2 - a;
    register int r4 = r3 * 2;
    register int r5 = r4 / 3;
    register int r6 = r5 << 1;
    register int r7 = r6 ^ r5;
    register int r8 = r7 | r4;
    register int r9 = r8 & r3;
    register int r10 = r9 + 100;
    register int r11 = r10 - 50;
    register int r12 = r11 * r10;
    register int r13 = r12 / 7;
    register int r14 = r13 << 2;
    register int r15 = r14 ^ 0xFF;
    register int r16 = r15 | 0xAA;
    register int r17 = r16 & 0x55;
    register int r18 = r17 + r16;
    register int r19 = r18 - r15;
    register int r20 = r19 * r14;
    
    /* Create basic block structure with call at end */
    if (global_counter > 100) {
        /* This path creates a basic block ending with the call */
        int sum = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
                  r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 + r20;
        
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
        
        /* Use all variables after call */
        return sum + r20;
    } else {
        /* Different path to create CFG */
        return r0 + r20;
    }
}

/* Function 2: Heavy floating-point pressure */
NOINLINE double test_fp_pressure(double x, double y, double z) {
    volatile double vx = x;
    double d0 = sin(vx);
    double d1 = cos(d0);
    double d2 = tan(d1);
    double d3 = d2 * y;
    double d4 = d3 + z;
    double d5 = d4 / 2.0;
    double d6 = d5 * d4;
    double d7 = sqrt(d6);
    double d8 = log(fabs(d7) + 1.0);
    double d9 = exp(d8);
    double d10 = d9 * 1.5;
    double d11 = d10 - 0.5;
    double d12 = d11 * d10;
    double d13 = d12 / 3.14159;
    double d14 = pow(d13, 2.0);
    double d15 = d14 + d13;
    double d16 = d15 * d14;
    double d17 = sin(d16) * cos(d15);
    double d18 = d17 + d16;
    double d19 = d18 * 2.71828;
    double d20 = d19 / d18;
    
    /* Switch statement to create multiple basic blocks */
    switch ((int)x % 4) {
        case 0: {
            /* Call at end of this case block */
            double sum = d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 +
                         d10 + d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20;
            
            /* Clobber FP registers */
            asm volatile("" : : : 
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
                "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
                "rax", "rcx", "rdx");
            
            bar((int)sum, d20);
            return sum + d20;
        }
        case 1:
            return d0 + d1;
        case 2:
            return d2 + d3;
        default:
            return d4 + d5;
    }
}

/* Function 3: Vector/SIMD register pressure */
NOINLINE __m128 test_vector_pressure(__m128 a, __m128 b) {
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
    __m128 v10 = _mm_mul_ps(v9, v7);
    __m128 v11 = _mm_sub_ps(v10, v8);
    __m128 v12 = _mm_add_ps(v11, v9);
    __m128 v13 = _mm_mul_ps(v12, v10);
    __m128 v14 = _mm_sub_ps(v13, v11);
    __m128 v15 = _mm_add_ps(v14, v12);
    
    /* Loop with partial unrolling - call at end of unrolled block */
    float result[4] = {0};
    for (int i = 0; i < 4; i++) {
        if (i % 2 == 0) {
            /* This creates a basic block ending with baz() call */
            __m128 temp = _mm_add_ps(v0, v15);
            
            /* Clobber vector registers */
            asm volatile("" : : : 
                "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7",
                "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15");
            
            baz(temp);
            
            /* Use variables after call */
            v0 = _mm_add_ps(v0, temp);
        } else {
            v15 = _mm_sub_ps(v15, v0);
        }
    }
    
    return _mm_add_ps(v0, v15);
}

/* Function 4: Mixed register pressure with complex control flow */
NOINLINE double test_mixed_pressure(int a, double b, __m128 c) {
    volatile int vi = a;
    volatile double vd = b;
    
    /* Integer pressure */
    int i0 = vi + 1, i1 = i0 * 2, i2 = i1 + 3, i3 = i2 * 4, i4 = i3 + 5;
    int i5 = i4 * 6, i6 = i5 + 7, i7 = i6 * 8, i8 = i7 + 9, i9 = i8 * 10;
    
    /* FP pressure */
    double d0 = sin(vd), d1 = cos(d0), d2 = d1 * 2.0, d3 = d2 + 1.0, d4 = sqrt(d3);
    double d5 = log(d4 + 1.0), d6 = exp(d5), d7 = d6 * 3.14, d8 = d7 / 2.71, d9 = pow(d8, 2.0);
    
    /* Vector pressure */
    __m128 v0 = _mm_add_ps(c, _mm_set1_ps(1.0f));
    __m128 v1 = _mm_mul_ps(v0, c);
    __m128 v2 = _mm_add_ps(v1, v0);
    
    /* Nested if-else to create multiple basic blocks */
    if (vi > 0) {
        if (vd > 0.5) {
            /* Call at end of this inner block */
            double sum = d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9;
            int isum = i0 + i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9;
            
            /* Massive clobber list */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
                "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
                "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7",
                "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15");
            
            external_func();
            
            return sum + isum + ((float*)&v2)[0];
        } else {
            return d0 + i0;
        }
    } else {
        return d1 + i1;
    }
}

/* Main function that calls all test cases */
int main(void) {
    double total = 0.0;
    
    /* Call integer pressure test multiple times */
    for (int i = 0; i < 10; i++) {
        total += test_integer_pressure(i, i+1, i+2);
        global_counter++;
    }
    
    /* Call FP pressure test */
    for (int i = 0; i < 5; i++) {
        total += test_fp_pressure(i * 0.1, i * 0.2, i * 0.3);
    }
    
    /* Call vector pressure test */
    __m128 vec_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 vec_b = _mm_set_ps(0.5f, 1.5f, 2.5f, 3.5f);
    for (int i = 0; i < 3; i++) {
        __m128 result = test_vector_pressure(vec_a, vec_b);
        total += ((float*)&result)[0];
    }
    
    /* Call mixed pressure test */
    for (int i = 0; i < 5; i++) {
        total += test_mixed_pressure(i, i * 0.25, vec_a);
    }
    
    printf("Total: %f\n", total);
    return (int)total % 256;
}
