/* test_mcf_coverage.c - Complex CFG generator for MCF pass coverage */
#ifdef __GNUC__
#define FORCE_USE(var) asm volatile("" : : "g"(var))
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define COLD __attribute__((cold))
#else
#define FORCE_USE(var) (void)(var)
#define NOINLINE
#define HOT
#define COLD
#endif

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

static int verbose = 0;

/* Pattern A: Function targeting ENTRY/EXIT block special indices */
HOT NOINLINE
int pattern_a_entry_exit(int iterations, int seed) {
    volatile int result = seed;
    int i, j, k;
    
    /* Create many local variables to increase register pressure */
    int r0 = seed, r1 = seed + 1, r2 = seed + 2, r3 = seed + 3;
    int r4 = seed + 4, r5 = seed + 5, r6 = seed + 6, r7 = seed + 7;
    int r8 = seed + 8, r9 = seed + 9, r10 = seed + 10, r11 = seed + 11;
    int r12 = seed + 12, r13 = seed + 13, r14 = seed + 14, r15 = seed + 15;
    
    /* Deeply nested if-else chain to create complex CFG */
    if (iterations > 0) {
        if (seed % 2 == 0) {
            for (i = 0; i < iterations; i++) {
                if (i % 3 == 0) {
                    r0 = r1 + r2;
                    r3 = r4 * r5;
                    goto label_a;  /* Create irreducible region */
                } else if (i % 3 == 1) {
                    r6 = r7 - r8;
                    r9 = r10 / (r11 ? r11 : 1);
                    goto label_b;
                } else {
                    r12 = r13 | r14;
                    r15 = r0 ^ r1;
                }
                
                if (i % 5 == 0) {
                    r2 = r3 << 2;
                    r4 = r5 >> 1;
                    continue;
                }
                
            label_a:
                r1 = r2 * r3;
                if (r1 > 1000) break;
                
            label_b:
                r5 = r6 + r7;
                if (r5 < 0) continue;
                
                /* More arithmetic to prevent optimization */
                r8 = r9 * r10 + r11;
                r12 = r13 - r14 * r15;
                r0 = r1 | r2 & r3;
            }
        } else {
            /* Alternative path with different structure */
            for (j = iterations; j > 0; j--) {
                switch (j % 7) {
                    case 0: r0 += r1; break;
                    case 1: r2 -= r3; break;
                    case 2: r4 *= r5; break;
                    case 3: r6 /= (r7 ? r7 : 1); break;
                    case 4: r8 &= r9; break;
                    case 5: r10 |= r11; break;
                    case 6: r12 ^= r13; break;
                }
                
                if (j % 11 == 0) {
                    goto label_a;  /* More irreducible edges */
                }
            }
        }
    }
    
    /* Force all variables to be considered live */
    FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
    FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
    FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
    FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
    
    result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + 
             r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
    
    return result % 1000;
}

/* Pattern B: Function using setjmp/longjmp for exceptional edges */
NOINLINE
int pattern_b_new_indices(int depth, int max_depth) {
    static jmp_buf env;
    volatile int result = 0;
    int i, j;
    
    /* Many scalar variables for register pressure */
    int s0 = 1, s1 = 2, s2 = 3, s3 = 4, s4 = 5, s5 = 6, s6 = 7, s7 = 8;
    int s8 = 9, s9 = 10, s10 = 11, s11 = 12, s12 = 13, s13 = 14, s14 = 15, s15 = 16;
    
    if (setjmp(env) == 0) {
        /* Normal execution path */
        for (i = 0; i < depth; i++) {
            s0 = s1 * s2 + s3;
            s4 = s5 - s6 * s7;
            s8 = s9 / (s10 ? s10 : 1) + s11;
            s12 = s13 | s14 & s15;
            
            /* Complex condition that may trigger longjmp */
            if (s0 > 1000000 || (i > max_depth / 2 && depth > 10)) {
                longjmp(env, 1);  /* Non-local jump creates exceptional edge */
            }
            
            /* Nested loop with more variables */
            for (j = 0; j < 5; j++) {
                int t0 = s0 + j, t1 = s1 - j, t2 = s2 * j, t3 = s3 / (j ? j : 1);
                s0 = t0 + t1;
                s1 = t2 - t3;
                FORCE_USE(t0); FORCE_USE(t1); FORCE_USE(t2); FORCE_USE(t3);
            }
        }
    } else {
        /* longjmp target - different execution path */
        s0 = s1 = s2 = s3 = 0;
        for (i = 0; i < max_depth; i++) {
            s4 += s5 * s6;
            s7 -= s8 / (s9 ? s9 : 1);
            s10 = s11 | s12;
            s13 = s14 ^ s15;
        }
    }
    
    /* Force all variables live */
    FORCE_USE(s0); FORCE_USE(s1); FORCE_USE(s2); FORCE_USE(s3);
    FORCE_USE(s4); FORCE_USE(s5); FORCE_USE(s6); FORCE_USE(s7);
    FORCE_USE(s8); FORCE_USE(s9); FORCE_USE(s10); FORCE_USE(s11);
    FORCE_USE(s12); FORCE_USE(s13); FORCE_USE(s14); FORCE_USE(s15);
    
    result = s0 + s4 + s8 + s12;
    return result % 1000;
}

/* Helper functions for Pattern D */
NOINLINE void dummy_helper1(int a, int b, int c) {
    volatile int x = a + b + c;
    FORCE_USE(x);
}

NOINLINE void dummy_helper2(int *a, int *b, int *c) {
    *a = *b + *c;
    *b = *c - *a;
    *c = *a * *b;
}

/* Pattern D: Artificial register conflicts with explicit register variables */
NOINLINE
int pattern_d_register_conflict(int iterations) {
    /* Explicit register variables that may conflict */
    register int x asm ("r10") = iterations;
    register int y asm ("r11") = iterations * 2;
    register int z asm ("r12") = iterations * 3;
    
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0, g = 0, h = 0;
    int i, j;
    
    for (i = 0; i < iterations; i++) {
        /* Force register variable usage */
        x = y + z;
        y = z - x;
        z = x * y;
        
        /* Call dummy functions that may clobber registers */
        dummy_helper1(x, y, z);
        
        /* More variables for pressure */
        a = b + c;
        b = c - d;
        c = d * e;
        d = e / (f ? f : 1);
        e = f | g;
        f = g ^ h;
        g = h & a;
        h = a + b;
        
        /* Another function call */
        dummy_helper2(&a, &b, &c);
        
        /* Switch to create multiple paths */
        switch (i % 8) {
            case 0: x += a; break;
            case 1: y -= b; break;
            case 2: z *= c; break;
            case 3: x /= (d ? d : 1); break;
            case 4: y &= e; break;
            case 5: z |= f; break;
            case 6: x ^= g; break;
            case 7: y = z + h; break;
        }
    }
    
    FORCE_USE(x); FORCE_USE(y); FORCE_USE(z);
    FORCE_USE(a); FORCE_USE(b); FORCE_USE(c); FORCE_USE(d);
    FORCE_USE(e); FORCE_USE(f); FORCE_USE(g); FORCE_USE(h);
    
    return (x + y + z + a + b + c + d + e + f + g + h) % 1000;
}

/* Pattern C: Mixed pressure with vector operations and large switch */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE
int pattern_c_mixed_pressure(int mode, int iterations) {
    volatile int result = 0;
    int i;
    
    /* Many scalar variables */
    int v0 = 1, v1 = 2, v2 = 3, v3 = 4, v4 = 5, v5 = 6, v6 = 7, v7 = 8;
    int v8 = 9, v9 = 10, v10 = 11, v11 = 12, v12 = 13, v13 = 14, v14 = 15, v15 = 16;
    
#ifdef __GNUC__
    /* Vector variables for additional register pressure */
    v4si vec0 = {1, 2, 3, 4};
    v4si vec1 = {5, 6, 7, 8};
    v4si vec2 = {9, 10, 11, 12};
    v4si vec3 = {13, 14, 15, 16};
#endif
    
    /* Large switch statement with 30+ cases */
    for (i = 0; i < iterations; i++) {
        int case_val = (mode + i) % 35;  /* 35 cases to exceed 30 */
        
        switch (case_val) {
            case 0:
                v0 = v1 + v2; v3 = v4 * v5;
#ifdef __GNUC__
                vec0 = vec1 + vec2;
#endif
                if (i % 3 == 0) continue;
                break;
            case 1:
                v6 = v7 - v8; v9 = v10 / (v11 ? v11 : 1);
#ifdef __GNUC__
                vec1 = vec2 - vec0;
#endif
                if (i % 5 == 0) break;
                /* fall through */
            case 2:
                v12 = v13 | v14; v15 = v0 ^ v1;
#ifdef __GNUC__
                vec2 = vec3 | vec0;
#endif
                break;
            case 3:
                v2 = v3 << 2; v4 = v5 >> 1;
#ifdef __GNUC__
                vec3 = vec0 << 1;
#endif
                if (v2 > 1000) continue;
                break;
            case 4:
                v5 = v6 + v7; v8 = v9 * v10;
#ifdef __GNUC__
                vec0 = vec1 * vec2;
#endif
                break;
            case 5:
                v11 = v12 - v13; v14 = v15 / (v0 ? v0 : 1);
#ifdef __GNUC__
                vec1 = vec2 / (vec3 != (v4si){0} ? vec3 : (v4si){1});
#endif
                break;
            case 6:
                v1 = v2 | v3; v4 = v5 & v6;
#ifdef __GNUC__
                vec2 = vec3 & vec0;
#endif
                break;
            case 7:
                v7 = v8 ^ v9; v10 = v11 + v12;
#ifdef __GNUC__
                vec3 = vec0 ^ vec1;
#endif
                if (i % 7 == 0) continue;
                break;
            case 8:
                v13 = v14 * v15; v0 = v1 - v2;
#ifdef __GNUC__
                vec0 = vec1 + vec2 + vec3;
#endif
                break;
            case 9:
                v3 = v4 / (v5 ? v5 : 1); v6 = v7 << 3;
#ifdef __GNUC__
                vec1 = vec2 << 2;
#endif
                break;
            case 10:
                v8 = v9 >> 2; v10 = v11 | v12;
#ifdef __GNUC__
                vec2 = vec3 >> 1;
#endif
                break;
            case 11:
                v13 = v14 & v15; v1 = v2 ^ v3;
#ifdef __GNUC__
                vec3 = vec0 & vec1;
#endif
                break;
            case 12:
                v4 = v5 + v6 + v7; v8 = v9 * v10 * v11;
#ifdef __GNUC__
                vec0 = vec0 + vec1;
#endif
                break;
            case 13:
                v12 = v13 - v14 - v15; v0 = v1 / (v2 ? v2 : 1);
#ifdef __GNUC__
                vec1 = vec1 - vec2;
#endif
                break;
            case 14:
                v3 = v4 | v5 | v6; v7 = v8 & v9 & v10;
#ifdef __GNUC__
                vec2 = vec2 | vec3;
#endif
                break;
            case 15:
                v11 = v12 ^ v13 ^ v14; v15 = v0 + v1 + v2;
#ifdef __GNUC__
                vec3 = vec3 ^ vec0;
#endif
                break;
            case 16:
                v3 = v4 * v5 * v6 * v7; v8 = v9 - v10 - v11 - v12;
#ifdef __GNUC__
                vec0 = vec0 * vec1;
#endif
                break;
            case 17:
                v13 = v14 / (v15 ? v15 : 1) / (v0 ? v0 : 1); v1 = v2 << 4;
#ifdef __GNUC__
                vec1 = vec1 / (vec2 != (v4si){0} ? vec2 : (v4si){1});
#endif
                break;
            case 18:
                v3 = v4 >> 3; v5 = v6 | v7 | v8 | v9;
#ifdef __GNUC__
                vec2 = vec2 >> 2;
#endif
                break;
            case 19:
                v10 = v11 & v12 & v13 & v14; v15 = v0 ^ v1 ^ v2 ^ v3;
#ifdef __GNUC__
                vec3 = vec3 & vec0 & vec1;
#endif
                break;
            /* Additional cases 20-34 with similar patterns */
            case 20: v4 = v5 + v6; v7 = v8 * v9; break;
            case 21: v10 = v11 - v12; v13 = v14 / (v15 ? v15 : 1); break;
            case 22: v0 = v1 | v2; v3 = v4 & v5; break;
            case 23: v6 = v7 ^ v8; v9 = v10 + v11; break;
            case 24: v12 = v13 * v14; v15 = v0 - v1; break;
            case 25: v2 = v3 / (v4 ? v4 : 1); v5 = v6 << 3; break;
            case 26: v7 = v8 >> 2; v9 = v10 | v11; break;
            case 27: v12 = v13 & v14; v15 = v0 ^ v1; break;
            case 28: v2 = v3 + v4 + v5; v6 = v7 * v8 * v9; break;
            case 29: v10 = v11 - v12 - v13; v14 = v15 / (v0 ? v0 : 1); break;
            case 30: v1 = v2 | v3 | v4; v5 = v6 & v7 & v8; break;
            case 31: v9 = v10 ^ v11 ^ v12; v13 = v14 + v15 + v0; break;
            case 32: v1 = v2 * v3 * v4 * v5; v6 = v7 - v8 - v9 - v10; break;
            case 33: v11 = v12 / (v13 ? v13 : 1) / (v14 ? v14 : 1); v15 = v0 << 4; break;
            case 34: v1 = v2 >> 3; v3 = v4 | v5 | v6 | v7; break;
        }
        
        /* Force all scalar variables live */
        FORCE_USE(v0); FORCE_USE(v1); FORCE_USE(v2); FORCE_USE(v3);
        FORCE_USE(v4); FORCE_USE(v5); FORCE_USE(v6); FORCE_USE(v7);
        FORCE_USE(v8); FORCE_USE(v9); FORCE_USE(v10); FORCE_USE(v11);
        FORCE_USE(v12); FORCE_USE(v13); FORCE_USE(v14); FORCE_USE(v15);
        
#ifdef __GNUC__
        /* Force vector variables live */
        FORCE_USE(vec0); FORCE_USE(vec1); FORCE_USE(vec2); FORCE_USE(vec3);
#endif
    }
    
    /* Compute final result */
    result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + 
             v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    
#ifdef __GNUC__
    /* Add vector components */
    int vec_sum = vec0[0] + vec0[1] + vec0[2] + vec0[3] +
                  vec1[0] + vec1[1] + vec1[2] + vec1[3] +
                  vec2[0] + vec2[1] + vec2[2] + vec2[3] +
                  vec3[0] + vec3[1] + vec3[2] + vec3[3];
    result += vec_sum;
#endif
    
    return result % 1000;
}

/* Main function with profile-guided optimization setup */
int main(int argc, char *argv[]) {
    volatile int total_result = 0;
    int i, iterations;
    
    /* Determine iterations based on input or default */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    } else {
        iterations = 100;
    }
    
    if (argc > 2 && strcmp(argv[2], "-v") == 0) {
        verbose = 1;
    }
    
    if (verbose) {
        printf("Starting MCF stress test with %d iterations\n", iterations);
    }
    
    /* Check CPU features to engage target-specific heuristics */
    int has_avx2 = 0;
#ifdef __GNUC__
    has_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 support: %s\n", has_avx2 ? "yes" : "no");
    }
#endif
    
    /* Call each pattern function multiple times with different arguments */
    for (i = 0; i < iterations; i++) {
        int seed = i * 12345 + 6789;
        
        /* Pattern A - targeting ENTRY/EXIT blocks */
        total_result += pattern_a_entry_exit(i % 50 + 10, seed);
        
        /* Pattern B - targeting NEW_EXIT/NEW_ENTRY with setjmp */
        if (i % 3 == 0) {
            total_result += pattern_b_new_indices(i % 20 + 5, 15);
        }
        
        /* Pattern C - mixed pressure with vectors and large switch */
        total_result += pattern_c_mixed_pressure(i % 10, i % 20 + 5);
        
        /* Pattern D - register conflicts */
        if (i % 4 == 0) {
            total_result += pattern_d_register_conflict(i % 30 + 10);
        }
        
        /* Occasionally add more complex patterns */
        if (i % 7 == 0) {
            /* Combine patterns for extra complexity */
            int temp = pattern_a_entry_exit(5, seed) + 
                      pattern_c_mixed_pressure(seed % 5, 3);
            total_result += temp % 1000;
        }
    }
    
    if (verbose) {
        printf("Final result: %d\n", total_result % 1000);
    }
    
    /* Force result to be used */
    FORCE_USE(total_result);
    
    return total_result % 1000;
}
