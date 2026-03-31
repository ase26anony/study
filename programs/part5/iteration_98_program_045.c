/* test_caller_save.c - Forces GCC to insert save/restore instructions at call sites */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline))

/* External function that clobbers registers - defined in separate file */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(__m128, __m256);

/* Helper to use variables after call to prevent optimization */
volatile int use_result;

/* Test 1: Integer register pressure with call at end of basic block */
NOINLINE int test_integer_pressure(int a, int b) {
    /* Create many integer variables that must survive across call */
    register int r0 = a + 1;
    volatile int v0 = r0;
    register int r1 = r0 * 2 + b;
    volatile int v1 = r1;
    register int r2 = r1 + a * 3;
    volatile int v2 = r2;
    register int r3 = r2 - b * 4;
    volatile int v3 = r3;
    register int r4 = r3 ^ a;
    volatile int v4 = r4;
    register int r5 = r4 | b;
    volatile int v5 = r5;
    register int r6 = r5 & a;
    volatile int v6 = r6;
    register int r7 = r6 << 2;
    volatile int v7 = r7;
    register int r8 = r7 >> 1;
    volatile int v8 = r8;
    register int r9 = r8 + r0;
    volatile int v9 = r9;
    register int r10 = r9 - r1;
    volatile int v10 = r10;
    register int r11 = r10 * r2;
    volatile int v11 = r11;
    register int r12 = r11 / (r3 ? r3 : 1);
    volatile int v12 = r12;
    register int r13 = r12 % (r4 ? r4 : 1);
    volatile int v13 = r13;
    register int r14 = r13 ^ r5;
    volatile int v14 = r14;
    register int r15 = r14 | r6;
    volatile int v15 = r15;
    
    /* Create control flow to put call at end of basic block */
    if (a > b) {
        /* This basic block ends with the call */
        register int r16 = r15 + r7;
        volatile int v16 = r16;
        register int r17 = r16 * r8;
        volatile int v17 = r17;
        
        /* Inline assembly to clobber caller-saved registers */
        asm volatile("" : : : 
            "rax", "rcx", "rdx", "rsi", "rdi", 
            "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
            "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
            "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
            "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* Call at end of basic block */
        foo();
        
        /* Use all variables after call */
        use_result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
                    r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17;
        return use_result;
    } else {
        /* Different path - still use variables */
        return r0 + r1 + r2;
    }
}

/* Test 2: Floating-point pressure with switch statement */
NOINLINE double test_float_pressure(double x, double y) {
    double result = 0.0;
    
    /* Many floating-point variables */
    volatile double d0 = sin(x);
    volatile double d1 = cos(y);
    volatile double d2 = d0 * d1;
    volatile double d3 = d2 + x;
    volatile double d4 = d3 - y;
    volatile double d5 = d4 * d0;
    volatile double d6 = d5 / (d1 + 1.0);
    volatile double d7 = sin(d6);
    volatile double d8 = cos(d7);
    volatile double d9 = d8 * d2;
    volatile double d10 = d9 + d3;
    volatile double d11 = d10 - d4;
    volatile double d12 = d11 * d5;
    volatile double d13 = d12 / d6;
    volatile double d14 = sin(d13);
    volatile double d15 = cos(d14);
    
    /* Switch to create multiple basic blocks */
    switch ((int)x % 4) {
        case 0:
            /* Call at end of this case's basic block */
            bar((int)x, d0);
            result = d0 + d1 + d2 + d3;
            break;
        case 1:
            result = d4 + d5 + d6 + d7;
            bar((int)y, d1);
            break;
        case 2:
            /* More computations then call */
            d8 = sin(d7 + d8);
            d9 = cos(d9 * 2.0);
            bar((int)(x + y), d2);
            result = d8 + d9 + d10 + d11;
            break;
        default:
            /* Call in middle with more live vars */
            d12 = d12 * 3.14159;
            bar((int)(x * y), d3);
            d13 = d13 / 2.71828;
            result = d12 + d13 + d14 + d15;
            break;
    }
    
    /* Use all variables */
    return result + d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 +
           d10 + d11 + d12 + d13 + d14 + d15;
}

/* Test 3: Vector register pressure with loop unrolling */
#ifdef __SSE__
NOINLINE float test_vector_pressure(float *arr, int n) {
    __m128 v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    __m128 v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    float sum = 0.0f;
    
    /* Load many vector values */
    v0 = _mm_loadu_ps(arr);
    v1 = _mm_loadu_ps(arr + 4);
    v2 = _mm_loadu_ps(arr + 8);
    v3 = _mm_loadu_ps(arr + 12);
    v4 = _mm_add_ps(v0, v1);
    v5 = _mm_sub_ps(v2, v3);
    v6 = _mm_mul_ps(v4, v5);
    v7 = _mm_loadu_ps(arr + 16);
    v8 = _mm_loadu_ps(arr + 20);
    v9 = _mm_add_ps(v6, v7);
    v10 = _mm_sub_ps(v8, v9);
    v11 = _mm_mul_ps(v10, v0);
    v12 = _mm_add_ps(v11, v1);
    v13 = _mm_sub_ps(v12, v2);
    v14 = _mm_mul_ps(v13, v3);
    v15 = _mm_add_ps(v14, v4);
    v16 = _mm_sub_ps(v15, v5);
    v17 = _mm_mul_ps(v16, v6);
    v18 = _mm_add_ps(v17, v7);
    v19 = _mm_sub_ps(v18, v8);
    
    /* Partially unrolled loop with call at end of iteration */
    for (int i = 0; i < n; i += 8) {
        /* More vector operations */
        v0 = _mm_add_ps(v0, v19);
        v1 = _mm_sub_ps(v1, v18);
        v2 = _mm_mul_ps(v2, v17);
        v3 = _mm_add_ps(v3, v16);
        
        /* Call that clobbers vector registers */
        if (i % 16 == 0) {
            /* This creates a basic block ending with call */
            baz(v0, _mm256_setzero_ps());
        }
        
        v4 = _mm_sub_ps(v4, v15);
        v5 = _mm_mul_ps(v5, v14);
        v6 = _mm_add_ps(v6, v13);
        v7 = _mm_sub_ps(v7, v12);
        
        /* Another potential call site */
        if (i % 24 == 0) {
            baz(v1, _mm256_set1_ps(1.0f));
        }
    }
    
    /* Extract and sum results */
    float temp[4];
    _mm_storeu_ps(temp, v0);
    sum += temp[0] + temp[1] + temp[2] + temp[3];
    _mm_storeu_ps(temp, v1);
    sum += temp[0] + temp[1] + temp[2] + temp[3];
    _mm_storeu_ps(temp, v19);
    sum += temp[0] + temp[1] + temp[2] + temp[3];
    
    return sum;
}
#endif

/* Test 4: Mixed register pressure in nested loops */
NOINLINE int test_mixed_pressure(int iter) {
    int int_sum = 0;
    double fp_sum = 0.0;
    
    for (int i = 0; i < iter; i++) {
        /* Integer pressure */
        register int i0 = i * 2;
        register int i1 = i0 + 1;
        register int i2 = i1 * 3;
        register int i3 = i2 - i;
        register int i4 = i3 ^ i0;
        register int i5 = i4 | i1;
        register int i6 = i5 & i2;
        register int i7 = i6 << 1;
        register int i8 = i7 >> 2;
        register int i9 = i8 + i3;
        register int i10 = i9 - i4;
        
        /* Floating pressure */
        volatile double d0 = sin(i * 0.1);
        volatile double d1 = cos(i * 0.2);
        volatile double d2 = d0 * d1;
        volatile double d3 = d2 + i * 0.3;
        volatile double d4 = d3 - d0;
        
        /* Call inside loop with many live variables */
        if (i % 3 == 0) {
            /* Basic block ending with call */
            bar(i, d0);
            int_sum += i0 + i1 + i2;
            fp_sum += d0 + d1;
        } else if (i % 3 == 1) {
            int_sum += i3 + i4 + i5;
            fp_sum += d2 + d3;
            /* Call at end of this block */
            bar(i * 2, d1);
        } else {
            /* Call in middle, splitting block */
            bar(i * 3, d2);
            int_sum += i6 + i7 + i8;
            fp_sum += d4;
        }
        
        /* Use remaining variables */
        int_sum += i9 + i10;
    }
    
    return int_sum + (int)fp_sum;
}

/* Test 5: Large switch with calls at case ends */
NOINLINE int test_switch_pressure(int val) {
    int result = 0;
    
    /* Many live variables across switch */
    register int a = val * 2;
    register int b = val + 1;
    register int c = a ^ b;
    register int d = c << 3;
    register int e = d >> 1;
    register int f = e + a;
    register int g = f - b;
    register int h = g * c;
    register int i = h / (d ? d : 1);
    register int j = i % (e ? e : 1);
    
    volatile double x = sin(val);
    volatile double y = cos(val);
    volatile double z = x * y;
    
    switch (val % 8) {
        case 0:
            result = a + b;
            foo();  /* Call at end of case block */
            break;
        case 1:
            foo();  /* Call at beginning */
            result = c + d;
            break;
        case 2:
            result = e + f;
            foo();  /* Call at end */
            break;
        case 3:
            result = g + h;
            /* Multiple calls in one case */
            if (val % 3 == 0) {
                bar(a, x);
            }
            foo();
            break;
        case 4:
            bar(b, y);
            result = i + j;
            break;
        case 5:
            result = (int)(x * 100);
            foo();
            bar(c, z);
            break;
        case 6:
            /* Nested control flow */
            for (int k = 0; k < 3; k++) {
                if (k == 1) {
                    foo();  /* Call at end of inner block */
                }
                result += k;
            }
            break;
        default:
            result = val;
            foo();  /* Call at end */
            break;
    }
    
    /* Use all variables after switch */
    return result + a + b + c + d + e + f + g + h + i + j + (int)(x + y + z);
}

int main(void) {
    int total = 0;
    float arr[64];
    
    /* Initialize array for vector test */
    for (int i = 0; i < 64; i++) {
        arr[i] = (float)i * 0.1f;
    }
    
    /* Run all tests to create multiple call sites with different pressures */
    total += test_integer_pressure(100, 200);
    total += (int)test_float_pressure(1.0, 2.0);
    
#ifdef __SSE__
    total += (int)test_vector_pressure(arr, 64);
#endif
    
    total += test_mixed_pressure(10);
    total += test_switch_pressure(42);
    
    printf("Total: %d\n", total);
    return 0;
}
