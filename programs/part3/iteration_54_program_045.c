/* test_mcf.c - Comprehensive test for GCC's Min-Cost Flow pass special nodes */
#ifdef __GNUC__
#define FORCE_USE(var) asm volatile("" : : "g"(var))
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define REGISTER_VAR(name, reg) register int name asm(reg)
#else
#define FORCE_USE(var) (void)(var)
#define NOINLINE
#define HOT
#define REGISTER_VAR(name, reg) int name
#endif

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

static int verbose = 0;

/* Pattern A: Function targeting ENTRY/EXIT blocks with irreducible region */
HOT NOINLINE
int pattern_a_entry_exit(int iterations, int seed) {
    volatile int result = seed;
    int i, j, k;
    
    /* Create many local variables to increase register pressure */
    int v0 = seed, v1 = seed + 1, v2 = seed + 2, v3 = seed + 3;
    int v4 = seed + 4, v5 = seed + 5, v6 = seed + 6, v7 = seed + 7;
    int v8 = seed + 8, v9 = seed + 9, v10 = seed + 10, v11 = seed + 11;
    int v12 = seed + 12, v13 = seed + 13, v14 = seed + 14, v15 = seed + 15;
    
    /* Label to create irreducible region with goto */
    irreducible_region:
    
    for (i = 0; i < iterations; i++) {
        /* Deeply nested if-else chain */
        if (v0 % 3 == 0) {
            if (v1 % 5 == 0) {
                v2 = v3 * v4 + v5;
                if (v6 > v7) {
                    v8 = v9 - v10;
                    goto jump_target_a;  /* Create irreducible flow */
                } else {
                    v11 = v12 | v13;
                }
            } else if (v2 % 7 == 0) {
                v3 = v4 ^ v5;
                for (j = 0; j < 5; j++) {
                    v6 += v7 * j;
                    if (v8 < v9) break;
                    v10 -= v11;
                }
            }
        } else if (v1 % 11 == 0) {
            v4 = v5 / (v6 + 1);
            jump_target_a:
            v7 = v8 << (v9 & 3);
            if (i % 2 == 0) {
                goto irreducible_region;  /* Back edge creating irreducible region */
            }
        }
        
        /* Complex arithmetic mixing all variables */
        v0 = v1 + v2 * v3 - v4;
        v1 = v5 ^ v6 | v7;
        v2 = v8 * v9 / (v10 + 1);
        v3 = v11 & v12 | v13;
        v4 = v14 - v15 + v0;
        v5 = v1 * v2 % (v3 + 1);
        
        FORCE_USE(v0); FORCE_USE(v1); FORCE_USE(v2); FORCE_USE(v3);
        FORCE_USE(v4); FORCE_USE(v5); FORCE_USE(v6); FORCE_USE(v7);
        FORCE_USE(v8); FORCE_USE(v9); FORCE_USE(v10); FORCE_USE(v11);
        FORCE_USE(v12); FORCE_USE(v13); FORCE_USE(v14); FORCE_USE(v15);
    }
    
    result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + 
             v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    return result;
}

/* Pattern B: Function using setjmp/longjmp for exceptional edges */
NOINLINE
int pattern_b_new_indices(int depth, int max_depth) {
    static jmp_buf env;
    volatile int result = 0;
    int i, j;
    
    /* Many local variables to pressure registers */
    int w0 = 1, w1 = 2, w2 = 3, w3 = 4, w4 = 5;
    int w5 = 6, w6 = 7, w7 = 8, w8 = 9, w9 = 10;
    int w10 = 11, w11 = 12, w12 = 13, w13 = 14, w14 = 15, w15 = 16;
    
    if (setjmp(env) == 0) {
        /* Normal path with complex loop */
        for (i = 0; i < depth; i++) {
            /* Nested loops increase back-edge complexity */
            for (j = 0; j < 10; j++) {
                w0 = w1 + w2 * w3;
                w1 = w4 - w5 / (w6 + 1);
                w2 = w7 & w8 | w9;
                w3 = w10 ^ w11 << (w12 & 3);
                w4 = w13 * w14 % (w15 + 1);
                
                /* Conditional longjmp creates exceptional edge */
                if (i > max_depth / 2 && j == 5) {
                    longjmp(env, 1);
                }
                
                /* More arithmetic to prevent optimization */
                w5 = w0 * w1 + w2;
                w6 = w3 - w4 * w5;
                w7 = w6 | w8 & w9;
                w8 = w10 ^ w11 + w12;
                w9 = w13 * w14 - w15;
                
                FORCE_USE(w0); FORCE_USE(w1); FORCE_USE(w2); FORCE_USE(w3);
                FORCE_USE(w4); FORCE_USE(w5); FORCE_USE(w6); FORCE_USE(w7);
                FORCE_USE(w8); FORCE_USE(w9); FORCE_USE(w10); FORCE_USE(w11);
                FORCE_USE(w12); FORCE_USE(w13); FORCE_USE(w14); FORCE_USE(w15);
            }
        }
    } else {
        /* longjmp target - different execution path */
        w0 = w1 * w2;
        w1 = w3 + w4;
        w2 = w5 - w6;
    }
    
    result = w0 + w1 + w2 + w3 + w4 + w5 + w6 + w7 + 
             w8 + w9 + w10 + w11 + w12 + w13 + w14 + w15;
    return result;
}

/* Pattern C: Mixed pressure with vector operations and large switch */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE
int pattern_c_mixed_pressure(int selector, int iterations) {
    volatile int result = 0;
    int i;
    
    /* Scalar variables */
    int s0 = 1, s1 = 2, s2 = 3, s3 = 4, s4 = 5;
    int s5 = 6, s6 = 7, s7 = 8, s8 = 9, s9 = 10;
    int s10 = 11, s11 = 12, s12 = 13, s13 = 14, s14 = 15, s15 = 16;
    
#ifdef __GNUC__
    /* Vector variables */
    v4si v0 = {1, 2, 3, 4};
    v4si v1 = {5, 6, 7, 8};
    v4si v2 = {9, 10, 11, 12};
    v4si v3 = {13, 14, 15, 16};
#endif
    
    for (i = 0; i < iterations; i++) {
        /* Large switch statement with 30+ cases */
        switch ((selector + i) % 35) {
            case 0:
                s0 = s1 + s2 * s3;
                s1 = s4 - s5;
#ifdef __GNUC__
                v0 = v1 + v2;
#endif
                break;
            case 1:
                s2 = s3 * s4 / (s5 + 1);
                s3 = s6 ^ s7;
#ifdef __GNUC__
                v1 = v2 - v3;
#endif
                break;
            case 2:
                s4 = s5 | s6 & s7;
                s5 = s8 << (s9 & 3);
#ifdef __GNUC__
                v2 = v3 * v0;
#endif
                break;
            case 3:
                s6 = s7 + s8 * s9;
                s7 = s10 - s11;
                break;
            case 4:
                s8 = s9 * s10 / (s11 + 1);
                s9 = s12 ^ s13;
                break;
            case 5:
                s10 = s11 | s12 & s13;
                s11 = s14 << (s15 & 3);
                break;
            case 6:
                s12 = s13 + s14 * s15;
                s13 = s0 - s1;
                break;
            case 7:
                s14 = s15 * s0 / (s1 + 1);
                s15 = s2 ^ s3;
                break;
            case 8:
                s0 = s1 | s2 & s3;
                s1 = s4 << (s5 & 3);
                break;
            case 9:
                s2 = s3 + s4 * s5;
                s3 = s6 - s7;
                break;
            case 10:
                s4 = s5 * s6 / (s7 + 1);
                s5 = s8 ^ s9;
                break;
            case 11:
                s6 = s7 | s8 & s9;
                s7 = s10 << (s11 & 3);
                break;
            case 12:
                s8 = s9 + s10 * s11;
                s9 = s12 - s13;
                break;
            case 13:
                s10 = s11 * s12 / (s13 + 1);
                s11 = s14 ^ s15;
                break;
            case 14:
                s12 = s13 | s14 & s15;
                s13 = s0 << (s1 & 3);
                break;
            case 15:
                s14 = s15 + s0 * s1;
                s15 = s2 - s3;
                break;
            case 16:
                s0 = s1 * s2 / (s3 + 1);
                s1 = s4 ^ s5;
                break;
            case 17:
                s2 = s3 | s4 & s5;
                s3 = s6 << (s7 & 3);
                break;
            case 18:
                s4 = s5 + s6 * s7;
                s5 = s8 - s9;
                break;
            case 19:
                s6 = s7 * s8 / (s9 + 1);
                s7 = s10 ^ s11;
                break;
            case 20:
                s8 = s9 | s10 & s11;
                s9 = s12 << (s13 & 3);
                break;
            case 21:
                s10 = s11 + s12 * s13;
                s11 = s14 - s15;
                break;
            case 22:
                s12 = s13 * s14 / (s15 + 1);
                s13 = s0 ^ s1;
                break;
            case 23:
                s14 = s15 | s0 & s1;
                s15 = s2 << (s3 & 3);
                break;
            case 24:
                s0 = s1 + s2 * s3;
                s1 = s4 - s5;
                break;
            case 25:
                s2 = s3 * s4 / (s5 + 1);
                s3 = s6 ^ s7;
                break;
            case 26:
                s4 = s5 | s6 & s7;
                s5 = s8 << (s9 & 3);
                break;
            case 27:
                s6 = s7 + s8 * s9;
                s7 = s10 - s11;
                break;
            case 28:
                s8 = s9 * s10 / (s11 + 1);
                s9 = s12 ^ s13;
                break;
            case 29:
                s10 = s11 | s12 & s13;
                s11 = s14 << (s15 & 3);
                break;
            case 30:
                s12 = s13 + s14 * s15;
                s13 = s0 - s1;
                break;
            case 31:
                s14 = s15 * s0 / (s1 + 1);
                s15 = s2 ^ s3;
                break;
            case 32:
                s0 = s1 | s2 & s3;
                s1 = s4 << (s5 & 3);
                break;
            case 33:
                s2 = s3 + s4 * s5;
                s3 = s6 - s7;
                break;
            case 34:
                s4 = s5 * s6 / (s7 + 1);
                s5 = s8 ^ s9;
                break;
        }
        
        /* Loop control with continue to different cases */
        if (i % 7 == 0) continue;
        if (i % 11 == 0) break;
        
        FORCE_USE(s0); FORCE_USE(s1); FORCE_USE(s2); FORCE_USE(s3);
        FORCE_USE(s4); FORCE_USE(s5); FORCE_USE(s6); FORCE_USE(s7);
        FORCE_USE(s8); FORCE_USE(s9); FORCE_USE(s10); FORCE_USE(s11);
        FORCE_USE(s12); FORCE_USE(s13); FORCE_USE(s14); FORCE_USE(s15);
#ifdef __GNUC__
        FORCE_USE(v0); FORCE_USE(v1); FORCE_USE(v2); FORCE_USE(v3);
#endif
    }
    
    result = s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + 
             s8 + s9 + s10 + s11 + s12 + s13 + s14 + s15;
#ifdef __GNUC__
    result += v0[0] + v1[0] + v2[0] + v3[0];
#endif
    return result;
}

/* Dummy helper functions for Pattern D */
NOINLINE void dummy_helper1(int a, int b, int c) {
    FORCE_USE(a); FORCE_USE(b); FORCE_USE(c);
}

NOINLINE void dummy_helper2(int a, int b, int c, int d) {
    FORCE_USE(a); FORCE_USE(b); FORCE_USE(c); FORCE_USE(d);
}

NOINLINE void dummy_helper3(int a, int b, int c, int d, int e) {
    FORCE_USE(a); FORCE_USE(b); FORCE_USE(c); FORCE_USE(d); FORCE_USE(e);
}

/* Pattern D: Artificial register conflicts with explicit register variables */
NOINLINE
int pattern_d_register_conflict(int iterations) {
    volatile int result = 0;
    int i;
    
    /* Explicit register variables creating artificial conflicts */
    REGISTER_VAR(r0, "r10");
    REGISTER_VAR(r1, "r11");
    REGISTER_VAR(r2, "r12");
    REGISTER_VAR(r3, "r13");
    
    r0 = 1; r1 = 2; r2 = 3; r3 = 4;
    
    /* Additional local variables */
    int x0 = 5, x1 = 6, x2 = 7, x3 = 8;
    int x4 = 9, x5 = 10, x6 = 11, x7 = 12;
    int x8 = 13, x9 = 14, x10 = 15, x11 = 16;
    
    for (i = 0; i < iterations; i++) {
        /* Complex arithmetic mixing register and regular variables */
        r0 = r1 + r2 * r3;
        x0 = x1 - x2 / (x3 + 1);
        
        /* Call dummy helpers to force spills/restores */
        dummy_helper1(r0, x0, i);
        
        r1 = r2 | r3 & x0;
        x1 = x2 ^ x3 << (x4 & 3);
        
        dummy_helper2(r1, x1, x2, i);
        
        r2 = r3 * x0 / (x1 + 1);
        x2 = x3 + x4 * x5;
        
        dummy_helper3(r2, x2, x3, x4, i);
        
        r3 = x0 ^ x1 | x2;
        x3 = x4 - x5 * x6;
        
        /* More arithmetic operations */
        x4 = x5 * x6 / (x7 + 1);
        x5 = x6 | x7 & x8;
        x6 = x7 ^ x8 << (x9 & 3);
        x7 = x8 * x9 / (x10 + 1);
        x8 = x9 | x10 & x11;
        
        FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
        FORCE_USE(x0); FORCE_USE(x1); FORCE_USE(x2); FORCE_USE(x3);
        FORCE_USE(x4); FORCE_USE(x5); FORCE_USE(x6); FORCE_USE(x7);
        FORCE_USE(x8); FORCE_USE(x9); FORCE_USE(x10); FORCE_USE(x11);
    }
    
    result = r0 + r1 + r2 + r3 + x0 + x1 + x2 + x3 + 
             x4 + x5 + x6 + x7 + x8 + x9 + x10 + x11;
    return result;
}

/* Main function with profile-guided optimization setup */
int main(int argc, char *argv[]) {
    volatile int total_result = 0;
    int i, iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 100) iterations = 100;
        if (iterations > 10000) iterations = 10000;
    }
    
    if (argc > 2 && strcmp(argv[2], "-v") == 0) {
        verbose = 1;
    }
    
    if (verbose) {
        printf("Starting MCF stress test with %d iterations\n", iterations);
    }
    
    /* Use CPU feature detection to engage target-specific register allocation */
    int use_avx2 = 0;
#ifdef __GNUC__
    use_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 support: %s\n", use_avx2 ? "yes" : "no");
    }
#endif
    
    /* Call each pattern multiple times with different arguments */
    for (i = 0; i < iterations; i++) {
        int seed = i * 1234567;
        
        /* Pattern A: Targeting ENTRY/EXIT blocks */
        total_result += pattern_a_entry_exit(i % 100 + 1, seed);
        
        /* Pattern B: Targeting NEW_EXIT/NEW_ENTRY with setjmp */
        if (i % 3 == 0) {
            total_result += pattern_b_new_indices(i % 50 + 1, 30);
        }
        
        /* Pattern C: Mixed pressure with vector operations */
        if (use_avx2 || (i % 5 == 0)) {
            total_result += pattern_c_mixed_pressure(i, i % 20 + 1);
        }
        
        /* Pattern D: Artificial register conflicts */
        total_result += pattern_d_register_conflict(i % 10 + 1);
        
        /* Prevent loop unrolling from simplifying CFG */
        if (i % 13 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    if (verbose) {
        printf("Total result: %d\n", total_result);
    }
    
    return total_result != 0 ? 0 : 1;
}
