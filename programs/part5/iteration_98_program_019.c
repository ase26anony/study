/* test_caller_save.c */
/* Compile with: gcc -O2 -fno-omit-frame-pointer -mno-red-zone -fno-schedule-insns -fno-schedule-insns2 test_caller_save.c helper.c -o test -lm */
/* For 32-bit: gcc -O2 -m32 -march=i686 test_caller_save.c helper.c -o test32 -lm */

#include <math.h>
#include <stdint.h>
#include <stdio.h>

/* External non-inlineable functions */
void __attribute__((noinline)) foo(void);
void __attribute__((noinline)) bar(void);
void __attribute__((noinline)) baz(void);

/* Helper to prevent optimization */
static volatile int sink;

/* Test 1: Integer register pressure with call at end of basic block */
int __attribute__((noinline)) test_integer_pressure(int a, int b, int c) {
    /* Create many integer live variables that must survive across call */
    register int r0 = a + 1;
    register int r1 = r0 * 2 + b;
    register int r2 = r1 - c;
    register int r3 = r2 * 3;
    register int r4 = r3 / 2;
    register int r5 = r4 ^ r3;
    register int r6 = r5 | r4;
    register int r7 = r6 & r5;
    register int r8 = r7 << 2;
    register int r9 = r8 >> 1;
    register int r10 = r9 + r8;
    register int r11 = r10 - r9;
    register int r12 = r11 * r10;
    register int r13 = r12 % 7;
    register int r14 = r13 | 0xFF;
    register int r15 = r14 & 0x0F;
    register int r16 = r15 ^ 0x55;
    register int r17 = r16 + 0x100;
    register int r18 = r17 - 0x50;
    register int r19 = r18 * 2;
    register int r20 = r19 / 3;
    
    /* Use volatile to ensure computations aren't optimized away */
    volatile int v0 = r0;
    volatile int v1 = r1;
    volatile int v2 = r2;
    
    /* Complex control flow to create basic blocks */
    if (a > b) {
        /* This creates a basic block ending with the call */
        int temp = r20 + r19 + r18;
        
        /* Inline assembly to clobber caller-saved registers */
        __asm__ volatile (
            "# Clobber integer registers"
            : 
            : 
            : "rax", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        /* Call at what could be end of basic block */
        foo();
        
        /* Use all variables after call - they must be saved/restored */
        return v0 + v1 + v2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 +
               r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 + r20 + temp;
    } else {
        /* Different path to create CFG complexity */
        bar();
        return r0 + r1;
    }
}

/* Test 2: Floating-point register pressure */
double __attribute__((noinline)) test_fp_pressure(double x, double y) {
    /* Many FP variables that must survive across call */
    double d0 = sin(x);
    double d1 = cos(y);
    double d2 = d0 * d1;
    double d3 = d2 + x;
    double d4 = d3 - y;
    double d5 = d4 * 2.0;
    double d6 = d5 / 3.14159;
    double d7 = sin(d6);
    double d8 = cos(d7);
    double d9 = d8 * d7;
    double d10 = d9 + d8;
    double d11 = d10 - d9;
    double d12 = d11 * 1.5;
    double d13 = d12 / 2.0;
    double d14 = sin(d13);
    double d15 = cos(d14);
    double d16 = d15 * d14;
    double d17 = d16 + 1.0;
    double d18 = d17 - 0.5;
    double d19 = d18 * 3.0;
    double d20 = d19 / 4.0;
    
    volatile double vd0 = d0;
    volatile double vd1 = d1;
    
    /* Switch statement creates multiple basic blocks */
    switch ((int)x % 3) {
        case 0: {
            /* Call at potential block end */
            __asm__ volatile (
                "# Clobber FP registers"
                :
                :
                : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                  "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                  "xmm12", "xmm13", "xmm14", "xmm15"
            );
            
            foo();
            
            /* Use FP variables after call */
            return vd0 + vd1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
                   d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20;
        }
        case 1:
            bar();
            return d0 + d1;
        default:
            baz();
            return d0 * d1;
    }
}

/* Test 3: Mixed integer/FP with loop unrolling */
int __attribute__((noinline)) test_mixed_pressure(int n) {
    int sum = 0;
    double fp_sum = 0.0;
    
    /* Partially unrolled loop with calls */
    for (int i = 0; i < n; i++) {
        /* Many live variables in loop */
        int i0 = i * 2;
        int i1 = i0 + 1;
        int i2 = i1 * 3;
        int i3 = i2 - i;
        int i4 = i3 / 2;
        
        double f0 = sin(i * 0.1);
        double f1 = cos(i * 0.2);
        double f2 = f0 * f1;
        double f3 = f2 + f0;
        double f4 = f3 - f1;
        
        volatile int vi = i4;
        volatile double vf = f4;
        
        /* Call inside loop - could be at block end after unrolling */
        if (i % 4 == 0) {
            __asm__ volatile (
                "# Clobber mixed registers"
                :
                :
                : "rax", "rcx", "rdx", "rsi", "rdi",
                  "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
            );
            
            foo();
            
            sum += vi + i0 + i1 + i2 + i3;
            fp_sum += vf + f0 + f1 + f2 + f3;
        } else if (i % 4 == 1) {
            bar();
            sum += i0;
        } else if (i % 4 == 2) {
            baz();
            sum += i1;
        } else {
            /* Another call at potential block end */
            foo();
            sum += i2;
        }
    }
    
    return sum + (int)fp_sum;
}

/* Test 4: Vector pressure (if available) */
#ifdef __SSE2__
#include <xmmintrin.h>
#include <emmintrin.h>

float __attribute__((noinline)) test_vector_pressure(float a, float b, float c, float d) {
    /* Create many vector variables */
    __m128 v0 = _mm_set_ps(a, b, c, d);
    __m128 v1 = _mm_set_ps(b, c, d, a);
    __m128 v2 = _mm_add_ps(v0, v1);
    __m128 v3 = _mm_mul_ps(v0, v1);
    __m128 v4 = _mm_sub_ps(v2, v3);
    __m128 v5 = _mm_set_ps(c, d, a, b);
    __m128 v6 = _mm_add_ps(v4, v5);
    __m128 v7 = _mm_mul_ps(v4, v5);
    __m128 v8 = _mm_sub_ps(v6, v7);
    __m128 v9 = _mm_set_ps(d, a, b, c);
    __m128 v10 = _mm_add_ps(v8, v9);
    
    volatile __m128 vv0 = v0;
    volatile __m128 vv1 = v1;
    
    /* Nested if-else to create complex CFG */
    if (a > 0.5f) {
        if (b > 0.3f) {
            /* Call at potential block end */
            __asm__ volatile (
                "# Clobber vector registers"
                :
                :
                : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                  "xmm6", "xmm7", "xmm8", "xmm9", "xmm10"
            );
            
            foo();
            
            /* Extract and use results */
            float results[4];
            _mm_store_ps(results, _mm_add_ps(vv0, vv1));
            return results[0] + results[1] + results[2] + results[3] +
                   ((float*)&v2)[0] + ((float*)&v3)[0] + ((float*)&v4)[0] +
                   ((float*)&v5)[0] + ((float*)&v6)[0] + ((float*)&v7)[0] +
                   ((float*)&v8)[0] + ((float*)&v9)[0] + ((float*)&v10)[0];
        } else {
            bar();
            return a + b;
        }
    } else {
        baz();
        return c + d;
    }
}
#endif

/* Main driver that calls all tests */
int main(void) {
    int total = 0;
    
    /* Test integer pressure */
    total += test_integer_pressure(100, 50, 25);
    
    /* Test floating-point pressure */
    total += (int)test_fp_pressure(1.0, 2.0);
    
    /* Test mixed pressure */
    total += test_mixed_pressure(20);
    
    #ifdef __SSE2__
    /* Test vector pressure */
    total += (int)test_vector_pressure(0.6f, 0.4f, 0.8f, 0.2f);
    #endif
    
    printf("Total: %d\n", total);
    return 0;
}
