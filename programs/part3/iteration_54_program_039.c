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

/* Pattern A: Deeply nested if-else with irreducible region for ENTRY/EXIT blocks */
HOT NOINLINE int pattern_a_entry_exit(int iterations, int seed) {
    volatile int result = seed;
    int i, j, k;
    
    /* Create many local variables to increase register pressure */
    int r0 = seed, r1 = seed + 1, r2 = seed + 2, r3 = seed + 3;
    int r4 = seed + 4, r5 = seed + 5, r6 = seed + 6, r7 = seed + 7;
    int r8 = seed + 8, r9 = seed + 9, r10 = seed + 10, r11 = seed + 11;
    int r12 = seed + 12, r13 = seed + 13, r14 = seed + 14, r15 = seed + 15;
    
    /* Label the first block to encourage ENTRY_BLOCK identification */
    start_label:
    
    for (i = 0; i < iterations; i++) {
        /* Deeply nested if-else chain */
        if (r0 % 3 == 0) {
            if (r1 % 5 == 0) {
                for (j = 0; j < 3; j++) {
                    if (r2 % 7 == 0) {
                        r3 = r4 * r5 + r6;
                        r7 = r8 ^ r9 | r10;
                        goto irreducible_region; /* Create irreducible CFG */
                    } else {
                        r11 = r12 - r13 * r14;
                        continue;
                    }
                }
            } else if (r1 % 7 == 0) {
                r15 = r0 * r1 - r2;
                goto another_label;
            }
        } else if (r0 % 5 == 0) {
            r3 = r4 + r5 * r6;
        }
        
        irreducible_region:
        /* This creates an irreducible region when combined with goto above */
        for (k = 0; k < 2; k++) {
            if (r7 % 11 == 0) {
                r8 = r9 + r10 * r11;
                break;
            } else {
                r12 = r13 - r14 / (r15 + 1);
                continue;
            }
        }
        
        another_label:
        r0 = (r0 * 1103515245 + 12345) & 0x7fffffff;
        r1 = (r1 * 1664525 + 1013904223) & 0x7fffffff;
        
        /* Force all variables to be considered live */
        FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
        FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
        FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
        FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
    }
    
    result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + 
             r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
    FORCE_USE(result);
    return result;
}

/* Pattern B: Large switch with complex loop for NEW_EXIT/NEW_ENTRY nodes */
NOINLINE int pattern_b_new_indices(int mode, int count) {
    volatile int result = 0;
    int i, state = mode;
    
    /* Many local variables for register pressure */
    int v0 = 1, v1 = 2, v2 = 3, v3 = 4, v4 = 5, v5 = 6, v6 = 7, v7 = 8;
    int v8 = 9, v9 = 10, v10 = 11, v11 = 12, v12 = 13, v13 = 14, v14 = 15, v15 = 16;
    int v16 = 17, v17 = 18, v18 = 19, v19 = 20, v20 = 21, v21 = 22, v22 = 23, v23 = 24;
    int v24 = 25, v25 = 26, v26 = 27, v27 = 28, v28 = 29, v29 = 30;
    
    /* Use setjmp/longjmp for exceptional edges */
    jmp_buf env;
    int jmp_val = setjmp(env);
    if (jmp_val != 0) {
        /* Came from longjmp - create complex merge point */
        v0 = v1 * v2 + jmp_val;
        v3 = v4 ^ v5 | v6;
    }
    
    for (i = 0; i < count; i++) {
        /* Large switch with 30+ cases */
        switch (state % 35) {
            case 0:
                v0 = v1 + v2 * v3 - v4;
                v5 = v6 ^ v7 & v8;
                if (i % 7 == 0) continue;
                break;
            case 1:
                v9 = v10 * v11 + v12 - v13;
                v14 = v15 | v16 ^ v17;
                if (i % 11 == 0) break;
                else continue;
            case 2:
                v18 = v19 - v20 * v21 / (v22 + 1);
                v23 = v24 & v25 | v26;
                break;
            case 3:
                v27 = v28 * 3 + v29;
                v0 = v1 - v2;
                if (i % 13 == 0) longjmp(env, i);
                break;
            case 4:
                v3 = v4 * v5 + v6 * v7;
                v8 = v9 ^ v10;
                break;
            case 5:
                v11 = v12 + v13 - v14 * v15;
                v16 = v17 | v18 & v19;
                continue;
            case 6:
                v20 = v21 * v22 + v23;
                v24 = v25 ^ v26;
                break;
            case 7:
                v27 = v28 + v29 * v0;
                v1 = v2 - v3;
                break;
            case 8:
                v4 = v5 * v6 + v7 * v8;
                v9 = v10 ^ v11;
                if (i % 17 == 0) continue;
                break;
            case 9:
                v12 = v13 + v14 - v15 * v16;
                v17 = v18 | v19 & v20;
                break;
            case 10:
                v21 = v22 * v23 + v24;
                v25 = v26 ^ v27;
                continue;
            case 11:
                v28 = v29 + v0 * v1;
                v2 = v3 - v4;
                break;
            case 12:
                v5 = v6 * v7 + v8 * v9;
                v10 = v11 ^ v12;
                break;
            case 13:
                v13 = v14 + v15 - v16 * v17;
                v18 = v19 | v20 & v21;
                if (i % 19 == 0) continue;
                break;
            case 14:
                v22 = v23 * v24 + v25;
                v26 = v27 ^ v28;
                break;
            case 15:
                v29 = v0 + v1 * v2;
                v3 = v4 - v5;
                break;
            case 16:
                v6 = v7 * v8 + v9 * v10;
                v11 = v12 ^ v13;
                break;
            case 17:
                v14 = v15 + v16 - v17 * v18;
                v19 = v20 | v21 & v22;
                continue;
            case 18:
                v23 = v24 * v25 + v26;
                v27 = v28 ^ v29;
                break;
            case 19:
                v0 = v1 + v2 * v3;
                v4 = v5 - v6;
                break;
            case 20:
                v7 = v8 * v9 + v10 * v11;
                v12 = v13 ^ v14;
                if (i % 23 == 0) continue;
                break;
            case 21:
                v15 = v16 + v17 - v18 * v19;
                v20 = v21 | v22 & v23;
                break;
            case 22:
                v24 = v25 * v26 + v27;
                v28 = v29 ^ v0;
                break;
            case 23:
                v1 = v2 + v3 * v4;
                v5 = v6 - v7;
                continue;
            case 24:
                v8 = v9 * v10 + v11 * v12;
                v13 = v14 ^ v15;
                break;
            case 25:
                v16 = v17 + v18 - v19 * v20;
                v21 = v22 | v23 & v24;
                break;
            case 26:
                v25 = v26 * v27 + v28;
                v29 = v0 ^ v1;
                if (i % 29 == 0) continue;
                break;
            case 27:
                v2 = v3 + v4 * v5;
                v6 = v7 - v8;
                break;
            case 28:
                v9 = v10 * v11 + v12 * v13;
                v14 = v15 ^ v16;
                break;
            case 29:
                v17 = v18 + v19 - v20 * v21;
                v22 = v23 | v24 & v25;
                continue;
            case 30:
                v26 = v27 * v28 + v29;
                v0 = v1 ^ v2;
                break;
            case 31:
                v3 = v4 + v5 * v6;
                v7 = v8 - v9;
                break;
            case 32:
                v10 = v11 * v12 + v13 * v14;
                v15 = v16 ^ v17;
                break;
            case 33:
                v18 = v19 + v20 - v21 * v22;
                v23 = v24 | v25 & v26;
                if (i % 31 == 0) continue;
                break;
            case 34:
                v27 = v28 * v29 + v0;
                v1 = v2 ^ v3;
                break;
            default:
                v4 = v5 + v6 * v7;
                v8 = v9 - v10;
                break;
        }
        
        state = (state * 1103515245 + 12345) & 0x7fffffff;
        
        /* Force all variables live */
        FORCE_USE(v0); FORCE_USE(v1); FORCE_USE(v2); FORCE_USE(v3);
        FORCE_USE(v4); FORCE_USE(v5); FORCE_USE(v6); FORCE_USE(v7);
        FORCE_USE(v8); FORCE_USE(v9); FORCE_USE(v10); FORCE_USE(v11);
        FORCE_USE(v12); FORCE_USE(v13); FORCE_USE(v14); FORCE_USE(v15);
        FORCE_USE(v16); FORCE_USE(v17); FORCE_USE(v18); FORCE_USE(v19);
        FORCE_USE(v20); FORCE_USE(v21); FORCE_USE(v22); FORCE_USE(v23);
        FORCE_USE(v24); FORCE_USE(v25); FORCE_USE(v26); FORCE_USE(v27);
        FORCE_USE(v28); FORCE_USE(v29);
    }
    
    result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
             v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
             v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    FORCE_USE(result);
    return result;
}

/* Pattern C: Vector operations with switch for mixed pressure */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE int pattern_c_vector_pressure(int iterations, int seed) {
    volatile int result = seed;
    int i, state = seed;
    
#ifdef __GNUC__
    /* Vector variables for SIMD register pressure */
    v4si vec0 = {seed, seed+1, seed+2, seed+3};
    v4si vec1 = {seed+4, seed+5, seed+6, seed+7};
    v4si vec2 = {seed+8, seed+9, seed+10, seed+11};
    v4si vec3 = {seed+12, seed+13, seed+14, seed+15};
    v4si vec4 = {seed+16, seed+17, seed+18, seed+19};
    v4si vec5 = {seed+20, seed+21, seed+22, seed+23};
#endif
    
    /* Scalar variables for integer register pressure */
    int s0 = seed, s1 = seed+1, s2 = seed+2, s3 = seed+3, s4 = seed+4;
    int s5 = seed+5, s6 = seed+6, s7 = seed+7, s8 = seed+8, s9 = seed+9;
    int s10 = seed+10, s11 = seed+11, s12 = seed+12, s13 = seed+13, s14 = seed+14;
    
    for (i = 0; i < iterations; i++) {
        /* Switch with vector and scalar operations */
        switch (state % 25) {
            case 0:
#ifdef __GNUC__
                vec0 = vec1 + vec2 * vec3;
#endif
                s0 = s1 * s2 + s3;
                if (i % 7 == 0) continue;
                break;
            case 1:
#ifdef __GNUC__
                vec1 = vec2 - vec3;
#endif
                s4 = s5 ^ s6 | s7;
                break;
            case 2:
                s8 = s9 * s10 + s11;
#ifdef __GNUC__
                vec2 = vec3 & vec4;
#endif
                break;
            case 3:
                s12 = s13 - s14 * s0;
#ifdef __GNUC__
                vec3 = vec4 | vec5;
#endif
                if (i % 11 == 0) continue;
                break;
            case 4:
#ifdef __GNUC__
                vec4 = vec5 + vec0;
#endif
                s1 = s2 * s3 + s4;
                break;
            case 5:
                s5 = s6 ^ s7 & s8;
#ifdef __GNUC__
                vec5 = vec0 - vec1;
#endif
                break;
            case 6:
#ifdef __GNUC__
                vec0 = vec1 * vec2;
#endif
                s9 = s10 + s11 - s12;
                continue;
            case 7:
                s13 = s14 * s0 + s1;
#ifdef __GNUC__
                vec1 = vec2 / (vec3 + (v4si){1,1,1,1});
#endif
                break;
            case 8:
#ifdef __GNUC__
                vec2 = vec3 ^ vec4;
#endif
                s2 = s3 | s4 & s5;
                break;
            case 9:
                s6 = s7 + s8 * s9;
#ifdef __GNUC__
                vec3 = vec4 + vec5;
#endif
                if (i % 13 == 0) continue;
                break;
            case 10:
#ifdef __GNUC__
                vec4 = vec5 - vec0;
#endif
                s10 = s11 ^ s12 | s13;
                break;
            case 11:
                s14 = s0 * s1 + s2;
#ifdef __GNUC__
                vec5 = vec0 & vec1;
#endif
                break;
            case 12:
#ifdef __GNUC__
                vec0 = vec1 | vec2;
#endif
                s3 = s4 - s5 * s6;
                continue;
            case 13:
                s7 = s8 ^ s9 & s10;
#ifdef __GNUC__
                vec1 = vec2 + vec3;
#endif
                break;
            case 14:
#ifdef __GNUC__
                vec2 = vec3 * vec4;
#endif
                s11 = s12 + s13 - s14;
                break;
            case 15:
                s0 = s1 * s2 + s3;
#ifdef __GNUC__
                vec3 = vec4 - vec5;
#endif
                if (i % 17 == 0) continue;
                break;
            case 16:
#ifdef __GNUC__
                vec4 = vec5 ^ vec0;
#endif
                s4 = s5 | s6 & s7;
                break;
            case 17:
                s8 = s9 + s10 * s11;
#ifdef __GNUC__
                vec5 = vec0 + vec1;
#endif
                break;
            case 18:
#ifdef __GNUC__
                vec0 = vec1 - vec2;
#endif
                s12 = s13 ^ s14 | s0;
                continue;
            case 19:
                s1 = s2 * s3 + s4;
#ifdef __GNUC__
                vec1 = vec2 & vec3;
#endif
                break;
            case 20:
#ifdef __GNUC__
                vec2 = vec3 | vec4;
#endif
                s5 = s6 - s7 * s8;
                break;
            case 21:
                s9 = s10 ^ s11 & s12;
#ifdef __GNUC__
                vec3 = vec4 + vec5;
#endif
                if (i % 19 == 0) continue;
                break;
            case 22:
#ifdef __GNUC__
                vec4 = vec5 * vec0;
#endif
                s13 = s14 + s0 - s1;
                break;
            case 23:
                s2 = s3 * s4 + s5;
#ifdef __GNUC__
                vec5 = vec0 - vec1;
#endif
                break;
            case 24:
#ifdef __GNUC__
                vec0 = vec1 ^ vec2;
#endif
                s6 = s7 | s8 & s9;
                continue;
        }
        
        state = (state * 1664525 + 1013904223) & 0x7fffffff;
        
        /* Force all variables live */
        FORCE_USE(s0); FORCE_USE(s1); FORCE_USE(s2); FORCE_USE(s3); FORCE_USE(s4);
        FORCE_USE(s5); FORCE_USE(s6); FORCE_USE(s7); FORCE_USE(s8); FORCE_USE(s9);
        FORCE_USE(s10); FORCE_USE(s11); FORCE_USE(s12); FORCE_USE(s13); FORCE_USE(s14);
#ifdef __GNUC__
        FORCE_USE(vec0); FORCE_USE(vec1); FORCE_USE(vec2);
        FORCE_USE(vec3); FORCE_USE(vec4); FORCE_USE(vec5);
#endif
    }
    
#ifdef __GNUC__
    /* Extract results from vectors */
    int vec_sum = vec0[0] + vec0[1] + vec0[2] + vec0[3] +
                  vec1[0] + vec1[1] + vec1[2] + vec1[3] +
                  vec2[0] + vec2[1] + vec2[2] + vec2[3] +
                  vec3[0] + vec3[1] + vec3[2] + vec3[3] +
                  vec4[0] + vec4[1] + vec4[2] + vec4[3] +
                  vec5[0] + vec5[1] + vec5[2] + vec5[3];
    result = vec_sum + s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8 + s9 +
             s10 + s11 + s12 + s13 + s14;
#else
    result = s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8 + s9 +
             s10 + s11 + s12 + s13 + s14;
#endif
    
    FORCE_USE(result);
    return result;
}

/* Pattern D: Explicit register variables for artificial conflicts */
NOINLINE int pattern_d_register_conflict(int iterations, int seed) {
    volatile int result = seed;
    int i;
    
    /* Explicit register variables that conflict */
#ifdef __GNUC__
    register int r10_var asm("r10") = seed;
    register int r11_var asm("r11") = seed + 1;
    /* Note: We can't bind multiple variables to same register directly,
       but we can create conflicts through use patterns */
#endif
    
    /* Many local variables */
    int a0 = seed, a1 = seed+1, a2 = seed+2, a3 = seed+3, a4 = seed+4;
    int a5 = seed+5, a6 = seed+6, a7 = seed+7, a8 = seed+8, a9 = seed+9;
    int b0 = seed+10, b1 = seed+11, b2 = seed+12, b3 = seed+13, b4 = seed+14;
    int b5 = seed+15, b6 = seed+16, b7 = seed+17, b8 = seed+18, b9 = seed+19;
    
    /* Dummy helper functions to force spills */
    auto void dummy_helper1(int x, int y, int z) NOINLINE {
        FORCE_USE(x); FORCE_USE(y); FORCE_USE(z);
    }
    
    auto void dummy_helper2(int x, int y, int z, int w) NOINLINE {
        FORCE_USE(x); FORCE_USE(y); FORCE_USE(z); FORCE_USE(w);
    }
    
    for (i = 0; i < iterations; i++) {
        /* Complex arithmetic with many intermediate values */
        int t0 = a0 * a1 + a2;
        int t1 = a3 ^ a4 | a5;
        int t2 = a6 - a7 * a8;
        int t3 = a9 & b0 | b1;
        int t4 = b2 * b3 + b4;
        int t5 = b5 ^ b6 & b7;
        int t6 = b8 - b9 * a0;
        int t7 = a1 | a2 & a3;
        
        /* Call helpers to force register spills */
        dummy_helper1(t0, t1, t2);
        dummy_helper2(t3, t4, t5, t6);
        
        /* Rotate values to create live range splits */
        a0 = a1; a1 = a2; a2 = a3; a3 = a4; a4 = a5;
        a5 = a6; a6 = a7; a7 = a8; a8 = a9; a9 = b0;
        b0 = b1; b1 = b2; b2 = b3; b3 = b4; b4 = b5;
        b5 = b6; b6 = b7; b7 = b8; b8 = b9; b9 = t0;
        
#ifdef __GNUC__
        /* Use explicit register variables in conflicting ways */
        r10_var = r10_var * 1103515245 + 12345;
        r11_var = r11_var * 1664525 + 1013904223;
        a0 ^= r10_var;
        a1 += r11_var;
#endif
        
        /* Force all variables live */
        FORCE_USE(a0); FORCE_USE(a1); FORCE_USE(a2); FORCE_USE(a3); FORCE_USE(a4);
        FORCE_USE(a5); FORCE_USE(a6); FORCE_USE(a7); FORCE_USE(a8); FORCE_USE(a9);
        FORCE_USE(b0); FORCE_USE(b1); FORCE_USE(b2); FORCE_USE(b3); FORCE_USE(b4);
        FORCE_USE(b5); FORCE_USE(b6); FORCE_USE(b7); FORCE_USE(b8); FORCE_USE(b9);
        FORCE_USE(t0); FORCE_USE(t1); FORCE_USE(t2); FORCE_USE(t3);
        FORCE_USE(t4); FORCE_USE(t5); FORCE_USE(t6); FORCE_USE(t7);
    }
    
#ifdef __GNUC__
    result = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 +
             b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 +
             r10_var + r11_var;
#else
    result = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 +
             b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9;
#endif
    
    FORCE_USE(result);
    return result;
}

/* Main function with profile-guided optimization setup */
int main(int argc, char *argv[]) {
    volatile int total = 0;
    int i, iterations = 100;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 100;
    }
    if (argc > 2 && strcmp(argv[2], "-v") == 0) {
        verbose = 1;
    }
    
    if (verbose) {
        printf("Starting MCF coverage test with %d iterations\n", iterations);
    }
    
    /* Use CPU feature detection to engage target-specific allocation */
    int use_avx2 = 0;
#ifdef __GNUC__
    use_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 support: %s\n", use_avx2 ? "yes" : "no");
    }
#endif
    
    /* Call each pattern multiple times with different arguments */
    for (i = 0; i < iterations; i++) {
        /* Pattern A: ENTRY/EXIT blocks */
        int r1 = pattern_a_entry_exit(10 + (i % 5), i * 3 + 1);
        total += r1;
        
        /* Pattern B: NEW_EXIT/NEW_ENTRY nodes */
        int r2 = pattern_b_new_indices(i % 100, 15 + (i % 7));
        total += r2;
        
        /* Pattern C: Vector pressure */
        int r3 = pattern_c_vector_pressure(8 + (i % 3), i * 5 + 2);
        total += r3;
        
        /* Pattern D: Register conflicts */
        int r4 = pattern_d_register_conflict(12 + (i % 4), i * 7 + 3);
        total += r4;
        
        if (verbose && (i % 20 == 0)) {
            printf("Iteration %d: results = %d, %d, %d, %d\n", i, r1, r2, r3, r4);
        }
    }
    
    if (verbose) {
        printf("Total accumulated: %d\n", total);
    }
    
    /* Prevent optimization of the final result */
    FORCE_USE(total);
    
    return total != 0 ? 0 : 1;
}
