/* test_caller_save.c */
#include <stdio.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline))

/* External function declarations */
NOINLINE void foo(void);
NOINLINE void bar(int);
NOINLINE double baz(double);
NOINLINE void vector_op(__m128*);

/* Helper functions in separate compilation unit */
extern void external_func1(void);
extern void external_func2(int);
extern double external_func3(double, double);

/* Global volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile double global_double = 3.14159;

/* Test function 1: Integer register pressure with call at block end */
NOINLINE int test_integer_pressure(int a, int b, int c) {
    /* Create many integer live variables across a call */
    register int r0 = a + 1;
    register int r1 = r0 * 2 + b;
    register int r2 = r1 - c;
    register int r3 = r2 * 3;
    register int r4 = r3 / 2;
    register int r5 = r4 + a;
    register int r6 = r5 - b;
    register int r7 = r6 * c;
    register int r8 = r7 + 100;
    register int r9 = r8 - 50;
    register int r10 = r9 * 2;
    register int r11 = r10 + r0;
    register int r12 = r11 - r1;
    register int r13 = r12 * r2;
    register int r14 = r13 / 3;
    register int r15 = r14 + r3;
    register int r16 = r15 - r4;
    register int r17 = r16 * r5;
    register int r18 = r17 + r6;
    register int r19 = r18 - r7;
    register int r20 = r19 * r8;
    
    /* Volatile variables that must survive across call */
    volatile int v0 = r0;
    volatile int v1 = r1;
    volatile int v2 = r2;
    volatile int v3 = r3;
    volatile int v4 = r4;
    volatile int v5 = r5;
    
    /* Complex control flow to create basic block ending with call */
    if (a > b) {
        /* More computations to increase register pressure */
        int t0 = v0 + v1;
        int t1 = t0 * v2;
        int t2 = t1 - v3;
        int t3 = t2 + v4;
        int t4 = t3 * v5;
        
        /* Inline assembly to clobber caller-saved registers */
        asm volatile("" : : : 
            "rax", "rcx", "rdx", "rsi", "rdi", 
            "r8", "r9", "r10", "r11", "r12",
            "r13", "r14", "r15", "xmm0", "xmm1",
            "xmm2", "xmm3", "xmm4", "xmm5", "xmm6",
            "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
            "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* Call at the end of this basic block */
        foo();
        
        /* Use all variables after call to keep them live */
        return r20 + t4 + v0 + v1 + v2 + v3 + v4 + v5;
    } else {
        /* Different path to create CFG complexity */
        bar(c);
        return r20 + a + b + c;
    }
}

/* Test function 2: Floating-point register pressure */
NOINLINE double test_fp_pressure(double x, double y, double z) {
    /* Many FP computations */
    double d0 = sin(x);
    double d1 = cos(y);
    double d2 = tan(z);
    double d3 = d0 * d1;
    double d4 = d2 + d3;
    double d5 = exp(d4);
    double d6 = log(fabs(d5));
    double d7 = d6 * d0;
    double d8 = d7 - d1;
    double d9 = d8 / d2;
    double d10 = d9 + d3;
    double d11 = d10 * d4;
    double d12 = d11 - d5;
    double d13 = d12 + d6;
    double d14 = d13 * d7;
    double d15 = d14 / d8;
    double d16 = d15 + d9;
    double d17 = d16 * d10;
    double d18 = d17 - d11;
    double d19 = d18 + d12;
    double d20 = d19 * d13;
    
    volatile double vd0 = d0;
    volatile double vd1 = d1;
    volatile double vd2 = d2;
    volatile double vd3 = d3;
    volatile double vd4 = d4;
    
    /* Switch statement to create multiple basic blocks */
    switch ((int)x % 4) {
        case 0: {
            /* Call at end of this case block */
            double result = baz(d20);
            
            /* Clobber FP registers */
            asm volatile("" : : : 
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4",
                "xmm5", "xmm6", "xmm7", "xmm8", "xmm9",
                "xmm10", "xmm11", "xmm12", "xmm13", "xmm14",
                "xmm15", "rax", "rcx", "rdx");
            
            external_func1();
            return result + vd0 + vd1 + vd2 + vd3 + vd4;
        }
        case 1:
            return d20 + sin(y);
        case 2:
            return d20 * cos(z);
        default:
            return d20;
    }
}

/* Test function 3: Vector register pressure */
NOINLINE __m128 test_vector_pressure(__m128 a, __m128 b, __m128 c) {
    /* Many vector operations */
    __m128 v0 = _mm_add_ps(a, b);
    __m128 v1 = _mm_mul_ps(v0, c);
    __m128 v2 = _mm_sub_ps(v1, a);
    __m128 v3 = _mm_add_ps(v2, b);
    __m128 v4 = _mm_mul_ps(v3, c);
    __m128 v5 = _mm_sub_ps(v4, v0);
    __m128 v6 = _mm_add_ps(v5, v1);
    __m128 v7 = _mm_mul_ps(v6, v2);
    __m128 v8 = _mm_sub_ps(v7, v3);
    __m128 v9 = _mm_add_ps(v8, v4);
    __m128 v10 = _mm_mul_ps(v9, v5);
    __m128 v11 = _mm_sub_ps(v10, v6);
    __m128 v12 = _mm_add_ps(v11, v7);
    __m128 v13 = _mm_mul_ps(v12, v8);
    __m128 v14 = _mm_sub_ps(v13, v9);
    __m128 v15 = _mm_add_ps(v14, v10);
    
    volatile __m128 vv0 = v0;
    volatile __m128 vv1 = v1;
    volatile __m128 vv2 = v2;
    
    /* Loop with partial unrolling - call at end of unrolled block */
    for (int i = 0; i < 4; i++) {
        if (i == 2) {
            /* High register pressure before call */
            __m128 t0 = _mm_add_ps(v15, vv0);
            __m128 t1 = _mm_mul_ps(t0, vv1);
            __m128 t2 = _mm_sub_ps(t1, vv2);
            
            /* Clobber vector registers */
            asm volatile("" : : : 
                "ymm0", "ymm1", "ymm2", "ymm3", "ymm4",
                "ymm5", "ymm6", "ymm7", "ymm8", "ymm9",
                "ymm10", "ymm11", "ymm12", "ymm13", "ymm14",
                "ymm15", "rax", "rcx", "rdx", "rsi", "rdi");
            
            vector_op(&t2);
            
            v15 = _mm_add_ps(v15, t2);
        }
        v15 = _mm_add_ps(v15, _mm_set1_ps(1.0f));
    }
    
    return v15;
}

/* Test function 4: Mixed register pressure with nested calls */
NOINLINE double test_mixed_pressure(int n, double x) {
    double result = x;
    
    /* Unrolled loop creating multiple basic blocks */
    for (int i = 0; i < n; i += 4) {
        if (i % 8 == 0) {
            /* Integer pressure */
            int i0 = i + global_counter;
            int i1 = i0 * 2;
            int i2 = i1 + 1;
            int i3 = i2 * 3;
            int i4 = i3 - i0;
            int i5 = i4 / 2;
            
            /* FP pressure */
            double d0 = sin(result);
            double d1 = cos(d0);
            double d2 = d1 * global_double;
            double d3 = d2 + result;
            
            volatile int vi0 = i0;
            volatile int vi1 = i1;
            volatile double vd0 = d0;
            volatile double vd1 = d1;
            
            /* Call external function */
            double temp = external_func3(d3, x);
            
            /* Use all variables */
            result += temp + vi0 + vi1 + vd0 + vd1 + i5;
        } else {
            result = sqrt(fabs(result));
        }
    }
    
    return result;
}

/* Main function that calls all test cases */
int main(void) {
    int int_result = 0;
    double fp_result = 0.0;
    __m128 vec_result;
    
    /* Call integer pressure test */
    for (int i = 0; i < 10; i++) {
        int_result += test_integer_pressure(i, i*2, i*3);
    }
    
    /* Call FP pressure test */
    for (int i = 1; i <= 5; i++) {
        fp_result += test_fp_pressure(i*0.1, i*0.2, i*0.3);
    }
    
    /* Call vector pressure test */
    __m128 a = _mm_setr_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 b = _mm_setr_ps(0.5f, 1.5f, 2.5f, 3.5f);
    __m128 c = _mm_setr_ps(2.0f, 3.0f, 4.0f, 5.0f);
    vec_result = test_vector_pressure(a, b, c);
    
    /* Call mixed pressure test */
    fp_result += test_mixed_pressure(20, 2.0);
    
    /* Use results to prevent dead code elimination */
    float vec_sum[4];
    _mm_store_ps(vec_sum, vec_result);
    
    printf("Results: int=%d, fp=%f, vec=[%f,%f,%f,%f]\n",
           int_result, fp_result,
           vec_sum[0], vec_sum[1], vec_sum[2], vec_sum[3]);
    
    return 0;
}

/* Dummy function definitions to satisfy references */
void foo(void) {
    global_counter++;
}

void bar(int x) {
    global_double += x;
}

double baz(double x) {
    return x * 2.0;
}

void vector_op(__m128* v) {
    *v = _mm_mul_ps(*v, _mm_set1_ps(2.0f));
}
