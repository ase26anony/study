/* test_caller_save.c - Forces caller-save register spilling at block ends */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline, noipa))

/* External function declarations */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(__m128);

/* Helper functions in separate compilation unit */
extern void external_func1(void);
extern void external_func2(int);
extern double external_func3(double);

/* Global volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile double global_accumulator = 0.0;

/* ========== Test Case 1: Integer register pressure at block end ========== */
NOINLINE int test_integer_pressure(int cond) {
    /* Create massive integer register pressure */
    register int r0  = cond + 1;
    register int r1  = r0 * 2;
    register int r2  = r1 + cond;
    register int r3  = r2 - r0;
    register int r4  = r3 * r1;
    register int r5  = r4 / (cond | 1);
    register int r6  = r5 ^ r2;
    register int r7  = r6 & r3;
    register int r8  = r7 | r4;
    register int r9  = r8 << 2;
    register int r10 = r9 >> 1;
    register int r11 = r10 + r5;
    register int r12 = r11 * r6;
    register int r13 = r12 - r7;
    register int r14 = r13 ^ r8;
    register int r15 = r14 & r9;
    register int r16 = r15 | r10;
    register int r17 = r16 << 1;
    register int r18 = r17 >> 2;
    register int r19 = r18 + r11;
    register int r20 = r19 * r12;
    
    /* Volatile variables that must survive across call */
    volatile int v0 = r0;
    volatile int v1 = r1;
    volatile int v2 = r2;
    volatile int v3 = r3;
    volatile int v4 = r4;
    volatile int v5 = r5;
    
    /* Complex control flow to create basic block ending with call */
    if (cond > 0) {
        /* This basic block ends with the call to foo() */
        int temp = v0 + v1 + v2 + v3 + v4 + v5;
        
        /* Inline assembly to clobber caller-saved integer registers */
        asm volatile("# Integer clobber" 
                     : 
                     : 
                     : "rax", "rcx", "rdx", "rsi", "rdi", 
                       "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
        
        /* Call at potential block end */
        foo();
        
        /* Use all the variables after call */
        return temp + r13 + r14 + r15 + r16 + r17 + r18 + r19 + r20;
    } else {
        /* Alternative path */
        return cond;
    }
}

/* ========== Test Case 2: FP/SSE register pressure ========== */
NOINLINE double test_fp_pressure(double x, int mode) {
    /* Create massive floating-point register pressure */
    double d0  = sin(x);
    double d1  = cos(x);
    double d2  = d0 * d1;
    double d3  = d2 + x;
    double d4  = d3 * d0;
    double d5  = d4 / (d1 + 1.0);
    double d6  = sqrt(fabs(d5));
    double d7  = exp(d6);
    double d8  = log(d7 + 1.0);
    double d9  = d8 * d3;
    double d10 = d9 - d4;
    double d11 = d10 / d5;
    double d12 = d11 + d6;
    double d13 = d12 * d7;
    double d14 = d13 - d8;
    double d15 = d14 / d9;
    double d16 = d15 + d10;
    double d17 = d16 * d11;
    double d18 = d17 - d12;
    double d19 = d18 / d13;
    double d20 = d19 + d14;
    
    /* SSE/vector variables */
    __m128 v0 = _mm_set_ps(d0, d1, d2, d3);
    __m128 v1 = _mm_set_ps(d4, d5, d6, d7);
    __m128 v2 = _mm_set_ps(d8, d9, d10, d11);
    __m128 v3 = _mm_set_ps(d12, d13, d14, d15);
    __m128 v4 = _mm_set_ps(d16, d17, d18, d19);
    
    volatile double vd0 = d0;
    volatile double vd1 = d1;
    volatile double vd2 = d2;
    
    /* Switch creates multiple basic blocks */
    switch (mode) {
        case 0: {
            /* This block ends with external call */
            double sum = vd0 + vd1 + vd2;
            
            /* Clobber FP/SSE registers */
            asm volatile("# FP clobber" 
                         : 
                         : 
                         : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                           "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                           "xmm12", "xmm13", "xmm14", "xmm15");
            
            external_func3(sum);
            
            /* Use variables after call */
            __m128 temp = _mm_add_ps(v0, v1);
            temp = _mm_add_ps(temp, v2);
            temp = _mm_add_ps(temp, v3);
            temp = _mm_add_ps(temp, v4);
            
            float result[4];
            _mm_store_ps(result, temp);
            return result[0] + result[1] + result[2] + result[3] + d20;
        }
        case 1:
            return d0 + d1;
        case 2:
            return d2 + d3;
        default:
            return d4 + d5;
    }
}

/* ========== Test Case 3: Mixed pressure in loop ========== */
NOINLINE double test_mixed_pressure(int iterations) {
    double total = 0.0;
    
    /* Unrolled loop to create block ending with call */
    for (int i = 0; i < iterations; i++) {
        /* Integer pressure */
        int i0 = i * 2;
        int i1 = i0 + 1;
        int i2 = i1 * 3;
        int i3 = i2 - i0;
        int i4 = i3 ^ i1;
        int i5 = i4 & i2;
        
        /* Floating pressure */
        double d0 = sin(i * 0.1);
        double d1 = cos(i * 0.2);
        double d2 = d0 * d1;
        double d3 = d2 + d0;
        double d4 = d3 * d1;
        
        /* Vector pressure */
        __m128 vec0 = _mm_set_ps(d0, d1, d2, d3);
        __m128 vec1 = _mm_set_ps(d4, d0, d1, d2);
        
        volatile int vi = i0;
        volatile double vd = d0;
        
        /* Conditional that creates block ending with call */
        if (i % 3 == 0) {
            /* This block ends with the call */
            int temp_i = vi + i1 + i2 + i3 + i4 + i5;
            double temp_d = vd + d1 + d2 + d3 + d4;
            
            /* Clobber everything */
            asm volatile("# Mixed clobber" 
                         : 
                         : 
                         : "rax", "rcx", "rdx", "rsi", "rdi",
                           "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5");
            
            bar(temp_i, temp_d);
            
            /* Use after call */
            __m128 result = _mm_add_ps(vec0, vec1);
            float res_arr[4];
            _mm_store_ps(res_arr, result);
            total += res_arr[0] + res_arr[1] + res_arr[2] + res_arr[3];
        } else if (i % 3 == 1) {
            total += d0;
        } else {
            total += d1;
        }
    }
    
    return total;
}

/* ========== Test Case 4: Nested calls with pressure ========== */
NOINLINE double test_nested_pressure(int depth, double x) {
    if (depth <= 0) {
        return x;
    }
    
    /* Register pressure before recursive call */
    double d0 = sin(x);
    double d1 = cos(x);
    double d2 = d0 * d1;
    double d3 = d2 + x;
    double d4 = d3 * d0;
    
    int i0 = depth * 2;
    int i1 = i0 + 1;
    int i2 = i1 * depth;
    int i3 = i2 ^ i0;
    int i4 = i3 & i1;
    
    volatile double vd = d0;
    volatile int vi = i0;
    
    /* Complex expression that might create block ending with call */
    double result = (depth % 2 == 0) ? 
        test_nested_pressure(depth - 1, vd + d1) :
        test_nested_pressure(depth - 2, vd * d2);
    
    /* Use all variables after call */
    return result + d3 + d4 + i1 + i2 + i3 + i4;
}

/* ========== Main driver ========== */
int main(void) {
    double total = 0.0;
    
    /* Run all test cases to trigger different spill scenarios */
    total += test_integer_pressure(global_counter + 1);
    total += test_fp_pressure(3.14159, global_counter % 3);
    total += test_mixed_pressure(10);
    total += test_nested_pressure(5, 1.0);
    
    /* Final call with remaining pressure */
    {
        int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
        double x = 1.1, y = 2.2, z = 3.3, w = 4.4;
        __m128 vec = _mm_set_ps(x, y, z, w);
        
        /* This might trigger the BB_END update */
        if (total > 0) {
            volatile int v = a + b + c + d;
            
            asm volatile("# Final clobber" 
                         : 
                         : 
                         : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9",
                           "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5");
            
            external_func1();
            
            total += v + e + f + g + h + x + y + z + w;
        }
    }
    
    printf("Result: %f\n", total);
    return (total > 0) ? 0 : 1;
}

/* Dummy functions to prevent inlining */
void foo(void) {
    global_counter++;
}

void bar(int a, double b) {
    global_accumulator += b + a;
}

void baz(__m128 v) {
    float f[4];
    _mm_store_ps(f, v);
    global_accumulator += f[0];
}
