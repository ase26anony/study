/* test_caller_save.c - Forces GCC to insert save/restore instructions at call sites */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline, noclone))

/* External function that clobbers registers - defined in separate file */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(__m128, __m128);

/* Helper to use variables after calls */
volatile int use_int;
volatile double use_double;
volatile __m128 use_vec;

/* ========== Test 1: Integer register pressure at block end ========== */
NOINLINE int test_integer_pressure(int seed) {
    /* Create many integer variables that must survive across call */
    register int r0 = seed + 1;
    volatile int v0 = r0 * 2;
    register int r1 = v0 + seed;
    volatile int v1 = r1 * 3;
    register int r2 = v1 - seed;
    volatile int v2 = r2 / 2;
    register int r3 = v2 | seed;
    volatile int v3 = r3 & 0xFF;
    register int r4 = v3 ^ seed;
    volatile int v4 = r4 << 2;
    register int r5 = v4 >> 1;
    volatile int v5 = r5 + 100;
    register int r6 = v5 * seed;
    volatile int v6 = r6 % 17;
    register int r7 = v6 | 0xAA;
    volatile int v7 = r7 & 0x55;
    register int r8 = v7 + v6;
    volatile int v8 = r8 * 3;
    register int r9 = v8 - v7;
    volatile int v9 = r9 / 2;
    register int r10 = v9 + v8;
    volatile int v10 = r10 * 5;
    register int r11 = v10 - v9;
    volatile int v11 = r11 | 0xF0;
    register int r12 = v11 & 0x0F;
    volatile int v12 = r12 ^ 0xFF;
    register int r13 = v12 + v11;
    volatile int v13 = r13 * 7;
    register int r14 = v13 - v12;
    volatile int v14 = r14 / 3;
    register int r15 = v14 + v13;
    volatile int v15 = r15 << 1;
    
    /* Clobber many caller-saved registers with inline asm */
    asm volatile("" : : : 
        "rax", "rcx", "rdx", "rsi", "rdi", 
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
        "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
        "xmm12", "xmm13", "xmm14", "xmm15");
    
    /* Call at potential block end - inside if/else */
    int result;
    if (seed % 3 == 0) {
        /* This call is at the end of this basic block */
        foo();  /* Non-inline call */
        
        /* Use all variables after call */
        result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + 
                 r9 + r10 + r11 + r12 + r13 + r14 + r15 +
                 v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 +
                 v9 + v10 + v11 + v12 + v13 + v14 + v15;
    } else if (seed % 3 == 1) {
        /* Different path */
        result = seed * 2;
    } else {
        /* Another path */
        result = seed * 3;
    }
    
    return result;
}

/* ========== Test 2: FP register pressure with complex CFG ========== */
NOINLINE double test_fp_pressure(double seed) {
    /* Many FP variables */
    double d0 = sin(seed);
    double d1 = cos(seed);
    double d2 = d0 * d1;
    double d3 = d2 + seed;
    double d4 = sin(d3);
    double d5 = cos(d4);
    double d6 = d5 * 2.0;
    double d7 = d6 / 1.5;
    double d8 = exp(d7);
    double d9 = log(fabs(d8) + 1.0);
    double d10 = d9 * d8;
    double d11 = d10 - d9;
    double d12 = sqrt(fabs(d11));
    double d13 = d12 * d11;
    double d14 = d13 + d12;
    double d15 = d14 / 2.0;
    double d16 = sin(d15) * cos(d15);
    double d17 = d16 * 3.14159;
    double d18 = d17 - d16;
    double d19 = d18 * 0.5;
    double d20 = d19 + d18;
    
    volatile double vd0 = d0;
    volatile double vd1 = d1;
    volatile double vd2 = d2;
    
    /* Complex control flow with switch */
    double result;
    switch ((int)seed % 5) {
        case 0:
            /* Call at end of this case block */
            bar((int)seed, d0);
            result = d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9;
            break;
        case 1:
            result = d10 + d11 + d12 + d13 + d14;
            break;
        case 2:
            /* Another call site */
            bar((int)(seed * 2), d1);
            result = d15 + d16 + d17 + d18 + d19 + d20;
            break;
        case 3:
            result = d0 * d10 * d20;
            break;
        default:
            /* Call at end of default block */
            bar((int)(seed * 3), d2);
            result = vd0 + vd1 + vd2;
            break;
    }
    
    /* Use variables after switch */
    use_double = d0 + d20;
    return result;
}

/* ========== Test 3: Vector register pressure in loop ========== */
NOINLINE __m128 test_vector_pressure(float seed) {
    /* Many vector variables */
    __m128 v0 = _mm_set1_ps(seed);
    __m128 v1 = _mm_set1_ps(seed * 2.0f);
    __m128 v2 = _mm_add_ps(v0, v1);
    __m128 v3 = _mm_mul_ps(v2, v0);
    __m128 v4 = _mm_sub_ps(v3, v1);
    __m128 v5 = _mm_set1_ps(seed * 3.0f);
    __m128 v6 = _mm_add_ps(v4, v5);
    __m128 v7 = _mm_mul_ps(v6, _mm_set1_ps(0.5f));
    __m128 v8 = _mm_add_ps(v7, v0);
    __m128 v9 = _mm_sub_ps(v8, v1);
    __m128 v10 = _mm_mul_ps(v9, v2);
    __m128 v11 = _mm_add_ps(v10, v3);
    __m128 v12 = _mm_sub_ps(v11, v4);
    __m128 v13 = _mm_mul_ps(v12, v5);
    __m128 v14 = _mm_add_ps(v13, v6);
    __m128 v15 = _mm_sub_ps(v14, v7);
    
    volatile __m128 vv0 = v0;
    volatile __m128 vv1 = v1;
    
    /* Loop with partial unrolling - call at end of unrolled block */
    __m128 accum = _mm_setzero_ps();
    for (int i = 0; i < 4; i++) {
        /* Manual unrolling */
        if (i == 0) {
            baz(v0, v1);
            accum = _mm_add_ps(accum, v0);
        } else if (i == 1) {
            baz(v2, v3);
            accum = _mm_add_ps(accum, v2);
        } else if (i == 2) {
            /* This call is at the end of this basic block */
            baz(v4, v5);
            accum = _mm_add_ps(accum, v4);
        } else {
            baz(v6, v7);
            accum = _mm_add_ps(accum, v6);
        }
    }
    
    /* Use all vectors after loop */
    __m128 result = _mm_add_ps(accum, v8);
    result = _mm_add_ps(result, v9);
    result = _mm_add_ps(result, v10);
    result = _mm_add_ps(result, v11);
    result = _mm_add_ps(result, v12);
    result = _mm_add_ps(result, v13);
    result = _mm_add_ps(result, v14);
    result = _mm_add_ps(result, v15);
    
    use_vec = result;
    return result;
}

/* ========== Test 4: Mixed pressure with nested calls ========== */
NOINLINE double test_mixed_pressure(int i_seed, double d_seed) {
    /* Integer pressure */
    register int ir0 = i_seed;
    register int ir1 = ir0 * 2;
    register int ir2 = ir1 + 1;
    register int ir3 = ir2 | 0xFF;
    register int ir4 = ir3 ^ ir2;
    register int ir5 = ir4 << 1;
    
    /* FP pressure */
    double dr0 = sin(d_seed);
    double dr1 = cos(d_seed);
    double dr2 = dr0 * dr1;
    double dr3 = dr2 + d_seed;
    
    /* Vector pressure */
    __m128 vr0 = _mm_set1_ps((float)d_seed);
    __m128 vr1 = _mm_add_ps(vr0, vr0);
    
    volatile int vi = ir0;
    volatile double vd = dr0;
    volatile __m128 vv = vr0;
    
    /* Complex control flow with call at end of one path */
    double result;
    if (i_seed > 100) {
        if (d_seed > 0.5) {
            /* Call at end of this nested block */
            foo();
            bar(ir0, dr0);
            result = dr0 + dr1 + dr2 + dr3;
        } else {
            result = dr0 - dr1;
        }
    } else {
        if (d_seed < -0.5) {
            result = dr2 * dr3;
        } else {
            /* Another call at block end */
            bar(ir1, dr1);
            result = dr0 + dr3;
        }
    }
    
    /* Use all variables */
    use_int = ir0 + ir1 + ir2 + ir3 + ir4 + ir5 + vi;
    use_double = dr0 + dr1 + dr2 + dr3 + vd + result;
    use_vec = _mm_add_ps(vr0, vr1);
    
    return result;
}

/* ========== Main driver ========== */
int main(void) {
    int total = 0;
    double dtotal = 0.0;
    __m128 vtotal = _mm_setzero_ps();
    
    /* Test multiple call sites with different pressure profiles */
    for (int i = 0; i < 10; i++) {
        total += test_integer_pressure(i);
        dtotal += test_fp_pressure((double)i * 0.1);
        
        __m128 vres = test_vector_pressure((float)i * 0.2f);
        float vsum[4];
        _mm_store_ps(vsum, vres);
        total += (int)(vsum[0] + vsum[1] + vsum[2] + vsum[3]);
        
        dtotal += test_mixed_pressure(i, (double)i * 0.3);
    }
    
    /* Ensure results are used */
    printf("Result: %d, %f\n", total, dtotal);
    
    /* Use volatile globals */
    printf("Globals: %d, %f\n", use_int, use_double);
    
    return 0;
}
