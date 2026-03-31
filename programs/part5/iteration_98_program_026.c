/* test_caller_save.c - Forces GCC to insert save/restore instructions at call sites */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline, noipa))

/* Helper function in separate compilation unit */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(void*);

/* Global volatile to prevent optimization */
volatile int global_seed = 42;

/* Vector types for SSE/AVX pressure */
typedef float v4sf __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v2df __attribute__((vector_size(16)));
typedef double v4df __attribute__((vector_size(32)));

/* Test 1: Integer register pressure with call at block end */
NOINLINE int test_integer_pressure(int input) {
    volatile int v0 = input + global_seed;
    register int r0 = v0 * 2;
    register int r1 = r0 + input;
    register int r2 = r1 * 3;
    register int r3 = r2 - v0;
    register int r4 = r3 ^ r1;
    register int r5 = r4 | r2;
    register int r6 = r5 & r3;
    register int r7 = r6 << 2;
    register int r8 = r7 >> 1;
    register int r9 = r8 * r7;
    register int r10 = r9 / (r6 + 1);
    register int r11 = r10 % (r5 + 1);
    register int r12 = r11 + r4;
    register int r13 = r12 - r3;
    register int r14 = r13 * r2;
    register int r15 = r14 | r1;
    register int r16 = r15 ^ r0;
    register int r17 = r16 << 3;
    register int r18 = r17 >> 2;
    register int r19 = r18 + r17;
    register int r20 = r19 - r16;
    
    /* Create basic block with call at end */
    if (input > 0) {
        /* More register pressure in this block */
        register int r21 = r20 * 2;
        register int r22 = r21 + 1;
        register int r23 = r22 * 3;
        register int r24 = r23 - 5;
        register int r25 = r24 ^ 0xFF;
        
        /* Inline assembly to clobber caller-saved registers */
        asm volatile("" : : : 
            "rax", "rcx", "rdx", "rsi", "rdi", 
            "r8", "r9", "r10", "r11", "r12",
            "xmm0", "xmm1", "xmm2", "xmm3",
            "xmm4", "xmm5", "xmm6", "xmm7",
            "xmm8", "xmm9", "xmm10", "xmm11",
            "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* Call at potential block end */
        foo();
        
        /* Use all variables after call */
        return r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
               r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 +
               r19 + r20 + r21 + r22 + r23 + r24 + r25;
    } else {
        /* Different path to create CFG */
        return r0 - r1 + r2 - r3;
    }
}

/* Test 2: Floating-point pressure with switch statement */
NOINLINE double test_fp_pressure(double input) {
    volatile double v0 = input + sin(input);
    double d0 = v0 * 2.0;
    double d1 = d0 + cos(input);
    double d2 = d1 * tan(input);
    double d3 = d2 - exp(input);
    double d4 = d3 / (log(fabs(input)) + 1.0);
    double d5 = d4 * sin(d0);
    double d6 = d5 + cos(d1);
    double d7 = d6 * tan(d2);
    double d8 = d7 - exp(d3);
    double d9 = d8 / (log(fabs(d4)) + 1.0);
    double d10 = d9 * sin(d5);
    double d11 = d10 + cos(d6);
    double d12 = d11 * tan(d7);
    double d13 = d12 - exp(d8);
    double d14 = d13 / (log(fabs(d9)) + 1.0);
    double d15 = d14 * sin(d10);
    
    /* Switch creates multiple basic blocks */
    int choice = (int)input % 4;
    double result = 0.0;
    
    switch (choice) {
        case 0:
            /* Call at end of this case block */
            bar(choice, d0);
            result = d0 + d1 + d2;
            break;
        case 1:
            /* More FP pressure before call */
            double d16 = d15 * 1.5;
            double d17 = d16 + 2.5;
            double d18 = d17 * d15;
            bar(choice, d16);
            result = d3 + d4 + d5 + d16 + d17 + d18;
            break;
        case 2:
            /* Vector operations before call */
            v2df vd0 = {d0, d1};
            v2df vd1 = {d2, d3};
            v2df vd2 = vd0 + vd1;
            v2df vd3 = vd0 * vd1;
            bar(choice, vd2[0] + vd3[1]);
            result = d6 + d7 + d8 + vd2[0] + vd2[1];
            break;
        default:
            /* Default path with call */
            bar(choice, d15);
            result = d9 + d10 + d11 + d12 + d13 + d14 + d15;
            break;
    }
    
    return result;
}

/* Test 3: Vector/SIMD pressure with loop unrolling */
NOINLINE float test_vector_pressure(float input) {
    v4sf vec0 = {input, input*2, input*3, input*4};
    v4sf vec1 = vec0 + (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = vec0 * vec1;
    v4sf vec3 = vec1 - vec2;
    v4sf vec4 = vec2 / (vec3 + 1.0f);
    v4sf vec5 = vec3 * vec4;
    v4sf vec6 = vec4 + vec5;
    v4sf vec7 = vec5 - vec6;
    v4sf vec8 = vec6 * vec7;
    v4sf vec9 = vec7 + vec8;
    v4sf vec10 = vec8 - vec9;
    v4sf vec11 = vec9 * vec10;
    v4sf vec12 = vec10 + vec11;
    v4sf vec13 = vec11 - vec12;
    v4sf vec14 = vec12 * vec13;
    v4sf vec15 = vec13 + vec14;
    
    /* Partially unrolled loop with call at end of iteration */
    float sum = 0.0f;
    for (int i = 0; i < 8; i++) {
        /* Create register pressure in loop body */
        v4sf temp0 = vec0 * (float)(i+1);
        v4sf temp1 = vec1 + temp0;
        v4sf temp2 = vec2 * temp1;
        v4sf temp3 = vec3 - temp2;
        
        if (i & 1) {
            /* Call at end of basic block inside loop */
            float* ptr = (float*)&temp3;
            baz(ptr);
            
            /* Use vector elements after call */
            sum += temp0[0] + temp1[1] + temp2[2] + temp3[3];
        } else {
            /* Different path */
            sum += temp3[0] + temp2[1] + temp1[2] + temp0[3];
        }
        
        /* Rotate vectors to create live ranges across iterations */
        v4sf tmp = vec0;
        vec0 = vec1; vec1 = vec2; vec2 = vec3; vec3 = vec4;
        vec4 = vec5; vec5 = vec6; vec6 = vec7; vec7 = vec8;
        vec8 = vec9; vec9 = vec10; vec10 = vec11; vec11 = vec12;
        vec12 = vec13; vec13 = vec14; vec14 = vec15; vec15 = tmp;
    }
    
    return sum;
}

/* Test 4: Mixed pressure with complex control flow */
NOINLINE int test_mixed_pressure(int mode) {
    /* Integer pressure */
    volatile int vi0 = global_seed;
    int i0 = vi0 + 1; int i1 = i0 * 2; int i2 = i1 + 3;
    int i3 = i2 * 4; int i4 = i3 - 5; int i5 = i4 ^ 0xAA;
    int i6 = i5 | 0x55; int i7 = i6 << 1; int i8 = i7 >> 2;
    int i9 = i8 + i7; int i10 = i9 - i6; int i11 = i10 * i5;
    
    /* Floating-point pressure */
    double d0 = sin(vi0); double d1 = cos(vi0);
    double d2 = d0 * d1; double d3 = d1 + d2;
    double d4 = d2 / d3; double d5 = d3 - d4;
    
    /* Vector pressure */
    v4sf v0 = {d0, d1, d2, d3};
    v4sf v1 = v0 + (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = v0 * v1;
    
    /* Nested control flow */
    int result = 0;
    if (mode > 0) {
        if (mode < 10) {
            /* Call at end of inner block */
            bar(mode, d0 + d1);
            result = i0 + i1 + i2 + (int)(d0 * 100);
        } else {
            /* Different path with another call */
            foo();
            result = i3 + i4 + i5 + (int)(d1 * 100);
        }
        
        /* Use more variables after conditional calls */
        result += i6 + i7 + i8 + (int)(v0[0] + v1[1] + v2[2]);
    } else {
        /* Else branch with call at end */
        baz(&i9);
        result = i9 + i10 + i11 + (int)(d2 + d3 + d4 + d5);
    }
    
    return result;
}

int main(void) {
    int total = 0;
    
    /* Call all test functions with different parameters */
    total += test_integer_pressure(global_seed);
    total += (int)test_fp_pressure(3.14159);
    total += (int)test_vector_pressure(1.234f);
    total += test_mixed_pressure(5);
    total += test_mixed_pressure(-2);
    
    /* Vary parameters to exercise different paths */
    for (int i = 0; i < 10; i++) {
        total += test_integer_pressure(i);
        total += (int)test_fp_pressure(i * 0.5);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
