/* test_caller_save.c */
/* Compile with: gcc -O2 -fno-omit-frame-pointer -mno-red-zone -fno-schedule-insns -fno-schedule-insns2 test_caller_save.c helper.c -lm -o test */

#include <stdio.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* External non-inlineable functions */
void __attribute__((noinline)) foo(void);
void __attribute__((noinline)) bar(void);
void __attribute__((noinline)) baz(void);

/* Helper function to prevent optimization */
static void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* Test 1: Integer register pressure with call at end of basic block */
void __attribute__((noinline)) test_integer_pressure(int cond) {
    /* Create many integer live variables across a call */
    volatile int v0 = 1;
    register int r0 asm ("r10") = v0 + 1;
    register int r1 asm ("r11") = r0 * 2;
    register int r2 asm ("r12") = r1 + 3;
    register int r3 asm ("r13") = r2 * 4;
    register int r4 asm ("r14") = r3 + 5;
    register int r5 asm ("r15") = r4 * 6;
    int r6 = r5 + 7;
    int r7 = r6 * 8;
    int r8 = r7 + 9;
    int r9 = r8 * 10;
    int r10 = r9 + 11;
    int r11 = r10 * 12;
    int r12 = r11 + 13;
    int r13 = r12 * 14;
    int r14 = r13 + 15;
    int r15 = r14 * 16;
    int r16 = r15 + 17;
    int r17 = r16 * 18;
    int r18 = r17 + 19;
    int r19 = r18 * 20;
    int r20 = r19 + 21;
    int r21 = r20 * 22;
    int r22 = r21 + 23;
    int r23 = r22 * 24;
    int r24 = r23 + 25;
    
    /* Complex control flow to create basic block ending with call */
    if (cond > 0) {
        /* This creates a basic block that ends with the call to foo() */
        int temp = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9;
        temp += r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19;
        temp += r20 + r21 + r22 + r23 + r24;
        
        /* Clobber many caller-saved registers before the call */
        asm volatile("" : : : 
            "rax", "rcx", "rdx", "rsi", "rdi", 
            "r8", "r9", "r10", "r11",
            "xmm0", "xmm1", "xmm2", "xmm3",
            "xmm4", "xmm5", "xmm6", "xmm7",
            "xmm8", "xmm9", "xmm10", "xmm11",
            "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* Call at what may be the end of a basic block */
        foo();
        
        /* Use all variables after call to keep them live */
        v0 = r0 + r1;
        use(&r2); use(&r3); use(&r4); use(&r5);
        use(&r6); use(&r7); use(&r8); use(&r9);
        use(&r10); use(&r11); use(&r12); use(&r13);
        use(&r14); use(&r15); use(&r16); use(&r17);
        use(&r18); use(&r19); use(&r20); use(&r21);
        use(&r22); use(&r23); use(&r24);
    } else {
        /* Alternative path to create CFG complexity */
        bar();
    }
}

/* Test 2: Floating-point register pressure */
void __attribute__((noinline)) test_float_pressure(int mode) {
    /* Many floating-point computations */
    volatile double d0 = 1.0;
    double d1 = sin(d0);
    double d2 = cos(d1);
    double d3 = d1 * d2;
    double d4 = sin(d3);
    double d5 = cos(d4);
    double d6 = d4 * d5;
    double d7 = sin(d6);
    double d8 = cos(d7);
    double d9 = d7 * d8;
    double d10 = sin(d9);
    double d11 = cos(d10);
    double d12 = d10 * d11;
    double d13 = sin(d12);
    double d14 = cos(d13);
    double d15 = d13 * d14;
    double d16 = sin(d15);
    double d17 = cos(d16);
    double d18 = d16 * d17;
    double d19 = sin(d18);
    double d20 = cos(d19);
    
    /* Switch statement to create multiple basic blocks */
    switch (mode) {
        case 0:
            /* Call at potential block end */
            asm volatile("" : : : 
                "xmm0", "xmm1", "xmm2", "xmm3",
                "xmm4", "xmm5", "xmm6", "xmm7",
                "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15");
            foo();
            break;
        case 1:
            bar();
            break;
        case 2:
            baz();
            break;
        default:
            /* More register pressure */
            asm volatile("" : : : 
                "rax", "rcx", "rdx",
                "xmm0", "xmm1", "xmm2", "xmm3");
            foo();
    }
    
    /* Use results to keep them live */
    d0 = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10;
    d0 += d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20;
    use(&d0);
}

/* Test 3: Vector register pressure with SSE/AVX */
void __attribute__((noinline)) test_vector_pressure(int iter) {
    /* Create many vector variables */
    __m128 v0 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 v1 = _mm_add_ps(v0, v0);
    __m128 v2 = _mm_mul_ps(v1, v1);
    __m128 v3 = _mm_add_ps(v2, v2);
    __m128 v4 = _mm_mul_ps(v3, v3);
    __m128 v5 = _mm_add_ps(v4, v4);
    __m128 v6 = _mm_mul_ps(v5, v5);
    __m128 v7 = _mm_add_ps(v6, v6);
    __m128 v8 = _mm_mul_ps(v7, v7);
    __m128 v9 = _mm_add_ps(v8, v8);
    __m128 v10 = _mm_mul_ps(v9, v9);
    __m128 v11 = _mm_add_ps(v10, v10);
    __m128 v12 = _mm_mul_ps(v11, v11);
    
    /* Loop with partial unrolling - call at end of unrolled block */
    for (int i = 0; i < iter; i++) {
        if (i % 3 == 0) {
            /* This may create a block ending with call */
            asm volatile("" : : : 
                "ymm0", "ymm1", "ymm2", "ymm3",
                "ymm4", "ymm5", "ymm6", "ymm7",
                "ymm8", "ymm9", "ymm10", "ymm11",
                "ymm12", "ymm13", "ymm14", "ymm15");
            foo();
            
            /* Keep vectors live */
            v0 = _mm_add_ps(v0, v1);
            v2 = _mm_add_ps(v2, v3);
        } else if (i % 3 == 1) {
            bar();
        } else {
            baz();
        }
    }
    
    /* Use vectors to prevent optimization */
    float result[4];
    _mm_store_ps(result, v0);
    _mm_store_ps(result, v12);
    use(result);
}

/* Test 4: Mixed pressure in nested control flow */
void __attribute__((noinline)) test_mixed_pressure(int depth) {
    /* Mixed integer and float variables */
    volatile int ivars[10];
    volatile double fvars[10];
    
    for (int i = 0; i < 10; i++) {
        ivars[i] = i * depth;
        fvars[i] = sin(i * 0.1);
    }
    
    /* Complex nested if to create interesting CFG */
    if (depth > 0) {
        int sum_i = 0;
        double sum_f = 0.0;
        
        for (int i = 0; i < 10; i++) {
            sum_i += ivars[i];
            sum_f += fvars[i];
        }
        
        if (sum_i > 20) {
            /* Call with many live values */
            register int r0 asm ("r10") = sum_i;
            register int r1 asm ("r11") = sum_i * 2;
            double d0 = sum_f;
            double d1 = sum_f * 2.0;
            
            asm volatile("" : : : 
                "rax", "rcx", "rdx", "rsi", "rdi",
                "r8", "r9", "r10", "r11",
                "xmm0", "xmm1", "xmm2", "xmm3",
                "xmm4", "xmm5", "xmm6", "xmm7");
            
            /* This call should be at block end */
            foo();
            
            /* Use values */
            ivars[0] = r0 + r1;
            fvars[0] = d0 + d1;
        } else {
            bar();
        }
    }
    
    /* Final use */
    use(ivars);
    use(fvars);
}

/* Main driver that calls all tests */
int main(void) {
    int result = 0;
    
    /* Call each test with different parameters to exercise
       different paths through the CFG */
    for (int i = 0; i < 10; i++) {
        test_integer_pressure(i);
        test_float_pressure(i % 4);
        test_vector_pressure(3);
        test_mixed_pressure(i);
        
        /* Simple computation to use results and prevent dead code */
        result += i;
    }
    
    printf("Result: %d\n", result);
    return 0;
}
