/* test_mcf_coverage.c - Complex CFG generator for MCF pass coverage testing */
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

/* Pattern A: Function targeting ENTRY/EXIT blocks with irreducible region */
HOT NOINLINE
int pattern_a_entry_exit(int iterations, int seed) {
    volatile int result = seed;
    int i, j, k;
    
    /* Create many local variables to increase register pressure */
    int r0 = seed, r1 = seed + 1, r2 = seed + 2, r3 = seed + 3;
    int r4 = seed + 4, r5 = seed + 5, r6 = seed + 6, r7 = seed + 7;
    int r8 = seed + 8, r9 = seed + 9, r10 = seed + 10, r11 = seed + 11;
    int r12 = seed + 12, r13 = seed + 13, r14 = seed + 14, r15 = seed + 15;
    
    /* Create irreducible region using goto */
    if (iterations > 100) {
        goto middle_of_loop;
    }
    
    for (i = 0; i < iterations; i++) {
        /* Deep if-else chain creating many basic blocks */
        if (r0 % 3 == 0) {
            r1 = r0 * 2 + r1;
            if (r1 > 1000) {
                r2 = r1 / 3 + r2;
                for (j = 0; j < 5; j++) {
                    r3 = r2 * j + r3;
                    if (r3 % 7 == 0) {
                        r4 = r3 ^ r4;
                    } else {
                        r5 = r3 | r5;
                    }
                }
            } else {
                r6 = r1 * 7 - r6;
            }
        } else if (r0 % 3 == 1) {
            r7 = r0 * 3 + r7;
            while (r7 < 500) {
                r8 = r7 + r8;
                r7 += 17;
                if (r8 > 300) break;
            }
        } else {
            r9 = r0 * 5 + r9;
            do {
                r10 = r9 - r10;
                r9 -= 13;
            } while (r9 > 0);
        }
        
        middle_of_loop:
        
        /* More complex branching */
        switch (r0 % 8) {
            case 0: r11 = r1 + r2; break;
            case 1: r12 = r3 - r4; break;
            case 2: r13 = r5 * r6; break;
            case 3: r14 = r7 / (r8 ? r8 : 1); break;
            case 4: r15 = r9 ^ r10; break;
            case 5: r0 = r11 | r12; break;
            case 6: r1 = r13 & r14; break;
            case 7: r2 = r15 % 17; break;
        }
        
        r0 = (r0 * 1103515245 + 12345) & 0x7fffffff;
        if (i % 7 == 0) continue;
        if (i % 13 == 0) break;
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
    
    /* Many local variables to increase register pressure */
    int a0 = depth, a1 = depth + 1, a2 = depth + 2, a3 = depth + 3;
    int a4 = depth + 4, a5 = depth + 5, a6 = depth + 6, a7 = depth + 7;
    int a8 = depth + 8, a9 = depth + 9, a10 = depth + 10, a11 = depth + 11;
    int a12 = depth + 12, a13 = depth + 13, a14 = depth + 14, a15 = depth + 15;
    
    if (setjmp(env) == 0) {
        /* Normal path with complex loop */
        for (i = 0; i < depth * 10; i++) {
            /* Large switch with 30+ cases */
            switch (i % 35) {
                case 0: a0 = a1 * 2 + a0; break;
                case 1: a1 = a2 * 3 - a1; break;
                case 2: a2 = a3 * 5 + a2; break;
                case 3: a3 = a4 * 7 - a3; break;
                case 4: a4 = a5 * 11 + a4; break;
                case 5: a5 = a6 * 13 - a5; break;
                case 6: a6 = a7 * 17 + a6; break;
                case 7: a7 = a8 * 19 - a7; break;
                case 8: a8 = a9 * 23 + a8; break;
                case 9: a9 = a10 * 29 - a9; break;
                case 10: a10 = a11 * 31 + a10; break;
                case 11: a11 = a12 * 37 - a11; break;
                case 12: a12 = a13 * 41 + a12; break;
                case 13: a13 = a14 * 43 - a13; break;
                case 14: a14 = a15 * 47 + a14; break;
                case 15: a15 = a0 * 53 - a15; break;
                case 16: a0 = a1 ^ a2; break;
                case 17: a1 = a3 | a4; break;
                case 18: a2 = a5 & a6; break;
                case 19: a3 = a7 << 2; break;
                case 20: a4 = a8 >> 3; break;
                case 21: a5 = a9 + a10; break;
                case 22: a6 = a11 - a12; break;
                case 23: a7 = a13 * a14; break;
                case 24: a8 = a15 / (a0 ? a0 : 1); break;
                case 25: a9 = a1 % (a2 ? a2 : 1); break;
                case 26: a10 = ~a3; break;
                case 27: a11 = a4 + a5 * 2; break;
                case 28: a12 = a6 - a7 / 3; break;
                case 29: a13 = a8 | a9 & a10; break;
                case 30: a14 = a11 ^ a12 << 1; break;
                case 31: a15 = a13 + a14 * 3; break;
                case 32: a0 = a15 - a0 / 5; break;
                case 33: a1 = (a1 + 1) * 7; break;
                case 34: a2 = (a2 - 1) * 11; break;
            }
            
            /* Nested loop with continue/break to different cases */
            for (j = 0; j < 3; j++) {
                if ((i + j) % 7 == 0) continue;
                if ((i + j) % 13 == 0) break;
                if (depth > max_depth / 2) {
                    /* Trigger longjmp to create exceptional edge */
                    longjmp(env, 1);
                }
            }
            
            FORCE_USE(a0); FORCE_USE(a1); FORCE_USE(a2); FORCE_USE(a3);
            FORCE_USE(a4); FORCE_USE(a5); FORCE_USE(a6); FORCE_USE(a7);
            FORCE_USE(a8); FORCE_USE(a9); FORCE_USE(a10); FORCE_USE(a11);
            FORCE_USE(a12); FORCE_USE(a13); FORCE_USE(a14); FORCE_USE(a15);
        }
    } else {
        /* longjmp target - different register usage pattern */
        a0 = a0 * 2; a1 = a1 * 3; a2 = a2 * 5; a3 = a3 * 7;
        a4 = a4 * 11; a5 = a5 * 13; a6 = a6 * 17; a7 = a7 * 19;
        a8 = a8 * 23; a9 = a9 * 29; a10 = a10 * 31; a11 = a11 * 37;
        a12 = a12 * 41; a13 = a13 * 43; a14 = a14 * 47; a15 = a15 * 53;
    }
    
    result = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + 
             a8 + a9 + a10 + a11 + a12 + a13 + a14 + a15;
    return result % 1000;
}

/* Pattern C: Mixed integer and vector register pressure */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE
int pattern_c_mixed_pressure(int iterations) {
    volatile int result = 0;
    int i;
    
    /* Many scalar variables */
    int s0 = 1, s1 = 2, s2 = 3, s3 = 4, s4 = 5, s5 = 6, s6 = 7, s7 = 8;
    int s8 = 9, s9 = 10, s10 = 11, s11 = 12, s12 = 13, s13 = 14, s14 = 15, s15 = 16;
    
#ifdef __GNUC__
    /* Vector variables */
    v4si v0 = {1, 2, 3, 4};
    v4si v1 = {5, 6, 7, 8};
    v4si v2 = {9, 10, 11, 12};
    v4si v3 = {13, 14, 15, 16};
#endif
    
    for (i = 0; i < iterations; i++) {
        /* Large switch mixing scalar and vector operations */
        switch (i % 40) {
            case 0: s0 = s1 + s2; break;
            case 1: s1 = s3 - s4; break;
            case 2: s2 = s5 * s6; break;
            case 3: s3 = s7 / (s8 ? s8 : 1); break;
            case 4: s4 = s9 ^ s10; break;
            case 5: s5 = s11 | s12; break;
            case 6: s6 = s13 & s14; break;
            case 7: s7 = s15 % 17; break;
            case 8: s8 = s0 << 2; break;
            case 9: s9 = s1 >> 3; break;
            case 10: s10 = ~s2; break;
            case 11: s11 = s3 + s4 * 2; break;
            case 12: s12 = s5 - s6 / 3; break;
            case 13: s13 = s7 | s8 & s9; break;
            case 14: s14 = s10 ^ s11 << 1; break;
            case 15: s15 = s12 + s13 * 3; break;
            case 16: s0 = s14 - s15 / 5; break;
            case 17: s1 = (s0 + 1) * 7; break;
            case 18: s2 = (s1 - 1) * 11; break;
            case 19: s3 = s2 * s3 + s4; break;
            case 20: s4 = s5 * s6 - s7; break;
            case 21: s5 = s8 * s9 + s10; break;
            case 22: s6 = s11 * s12 - s13; break;
            case 23: s7 = s14 * s15 + s0; break;
            case 24: s8 = s1 * s2 - s3; break;
            case 25: s9 = s4 * s5 + s6; break;
            case 26: s10 = s7 * s8 - s9; break;
            case 27: s11 = s10 * s11 + s12; break;
            case 28: s12 = s13 * s14 - s15; break;
            case 29: s13 = s0 * s1 + s2; break;
            case 30: s14 = s3 * s4 - s5; break;
            case 31: s15 = s6 * s7 + s8; break;
#ifdef __GNUC__
            case 32: v0 = v0 + v1; break;
            case 33: v1 = v1 - v2; break;
            case 34: v2 = v2 * v3; break;
            case 35: v3 = v0 & v1; break;
            case 36: v0 = v1 | v2; break;
            case 37: v1 = v2 ^ v3; break;
            case 38: v2 = v0 << 1; break;
            case 39: v3 = v1 >> 2; break;
#else
            case 32: case 33: case 34: case 35:
            case 36: case 37: case 38: case 39:
                s0 = s0 * 2; break;
#endif
        }
        
        /* Nested loop with complex control flow */
        for (int j = 0; j < 5; j++) {
            if ((i + j) % 11 == 0) continue;
            if ((i + j) % 19 == 0) break;
            
            /* Mix scalar and vector results */
#ifdef __GNUC__
            if (j % 3 == 0) {
                int* vp = (int*)&v0;
                s0 += vp[0]; s1 += vp[1]; s2 += vp[2]; s3 += vp[3];
            }
#endif
        }
        
        FORCE_USE(s0); FORCE_USE(s1); FORCE_USE(s2); FORCE_USE(s3);
        FORCE_USE(s4); FORCE_USE(s5); FORCE_USE(s6); FORCE_USE(s7);
        FORCE_USE(s8); FORCE_USE(s9); FORCE_USE(s10); FORCE_USE(s11);
        FORCE_USE(s12); FORCE_USE(s13); FORCE_USE(s14); FORCE_USE(s15);
    }
    
#ifdef __GNUC__
    /* Extract results from vectors */
    int* vp0 = (int*)&v0;
    int* vp1 = (int*)&v1;
    int* vp2 = (int*)&v2;
    int* vp3 = (int*)&v3;
    
    result = s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + 
             s8 + s9 + s10 + s11 + s12 + s13 + s14 + s15 +
             vp0[0] + vp0[1] + vp0[2] + vp0[3] +
             vp1[0] + vp1[1] + vp1[2] + vp1[3] +
             vp2[0] + vp2[1] + vp2[2] + vp2[3] +
             vp3[0] + vp3[1] + vp3[2] + vp3[3];
#else
    result = s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + 
             s8 + s9 + s10 + s11 + s12 + s13 + s14 + s15;
#endif
    
    return result % 1000;
}

/* Pattern D: Artificial register conflicts with explicit register variables */
#ifdef __GNUC__
NOINLINE void dummy1(register int x asm ("r10"), register int y asm ("r11")) {
    FORCE_USE(x); FORCE_USE(y);
}

NOINLINE void dummy2(register int a asm ("r10"), register int b asm ("r12")) {
    FORCE_USE(a); FORCE_USE(b);
}

NOINLINE void dummy3(register int p asm ("r11"), register int q asm ("r12")) {
    FORCE_USE(p); FORCE_USE(q);
}
#endif

NOINLINE
int pattern_d_register_conflicts(int n) {
    volatile int result = 0;
    int i;
    
#ifdef __GNUC__
    /* Explicit register variables creating artificial conflicts */
    register int x asm ("r10") = n;
    register int y asm ("r11") = n + 1;
    register int z asm ("r12") = n + 2;
#endif
    
    int a0 = 1, a1 = 2, a2 = 3, a3 = 4, a4 = 5, a5 = 6, a6 = 7, a7 = 8;
    int a8 = 9, a9 = 10, a10 = 11, a11 = 12, a12 = 13, a13 = 14, a14 = 15, a15 = 16;
    
    for (i = 0; i < n; i++) {
        /* Large switch with calls to conflicting register functions */
        switch (i % 30) {
            case 0: a0 = a1 + a2; break;
            case 1: a1 = a3 - a4; break;
            case 2: a2 = a5 * a6; break;
            case 3: a3 = a7 / (a8 ? a8 : 1); break;
            case 4: a4 = a9 ^ a10; break;
            case 5: a5 = a11 | a12; break;
            case 6: a6 = a13 & a14; break;
            case 7: a7 = a15 % 17; break;
            case 8: a8 = a0 << 2; break;
            case 9: a9 = a1 >> 3; break;
            case 10: a10 = ~a2; break;
            case 11: a11 = a3 + a4 * 2; break;
            case 12: a12 = a5 - a6 / 3; break;
            case 13: a13 = a7 | a8 & a9; break;
            case 14: a14 = a10 ^ a11 << 1; break;
            case 15: a15 = a12 + a13 * 3; break;
            case 16: a0 = a14 - a15 / 5; break;
            case 17: a1 = (a0 + 1) * 7; break;
            case 18: a2 = (a1 - 1) * 11; break;
            case 19: a3 = a2 * a3 + a4; break;
            case 20: a4 = a5 * a6 - a7; break;
            case 21: a5 = a8 * a9 + a10; break;
            case 22: a6 = a11 * a12 - a13; break;
            case 23: a7 = a14 * a15 + a0; break;
            case 24: a8 = a1 * a2 - a3; break;
            case 25: a9 = a4 * a5 + a6; break;
            case 26: a10 = a7 * a8 - a9; break;
            case 27: a11 = a10 * a11 + a12; break;
            case 28: a12 = a13 * a14 - a15; break;
            case 29: a13 = a0 * a1 + a2; break;
        }
        
        /* Call functions with conflicting register usage */
#ifdef __GNUC__
        if (i % 7 == 0) dummy1(x, y);
        if (i % 11 == 0) dummy2(y, z);
        if (i % 13 == 0) dummy3(x, z);
        
        /* Modify register variables */
        x = (x * 1103515245 + 12345) & 0x7fffffff;
        y = (y * 1103515245 + 12345) & 0x7fffffff;
        z = (z * 1103515245 + 12345) & 0x7fffffff;
#endif
        
        FORCE_USE(a0); FORCE_USE(a1); FORCE_USE(a2); FORCE_USE(a3);
        FORCE_USE(a4); FORCE_USE(a5); FORCE_USE(a6); FORCE_USE(a7);
        FORCE_USE(a8); FORCE_USE(a9); FORCE_USE(a10); FORCE_USE(a11);
        FORCE_USE(a12); FORCE_USE(a13); FORCE_USE(a14); FORCE_USE(a15);
    }
    
#ifdef __GNUC__
    result = x + y + z + a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + 
             a8 + a9 + a10 + a11 + a12 + a13 + a14 + a15;
#else
    result = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + 
             a8 + a9 + a10 + a11 + a12 + a13 + a14 + a15;
#endif
    
    return result % 1000;
}

/* Main function to drive all patterns with PGO */
int main(int argc, char** argv) {
    volatile int total = 0;
    int i, iterations = 100;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    if (argc > 2 && strcmp(argv[2], "-v") == 0) {
        verbose = 1;
    }
    
    if (verbose) {
        printf("Starting MCF coverage test with %d iterations\n", iterations);
    }
    
    /* Use CPU features to engage target-specific register allocation */
    int use_avx2 = 0;
#ifdef __GNUC__
    use_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 support: %s\n", use_avx2 ? "yes" : "no");
    }
#endif
    
    /* Run all patterns multiple times to generate profile data */
    for (i = 0; i < iterations; i++) {
        int seed = i * 1103515245 + 12345;
        
        /* Pattern A: ENTRY/EXIT blocks */
        total += pattern_a_entry_exit(iterations / 10 + i % 20, seed);
        
        /* Pattern B: NEW_EXIT/NEW_ENTRY with setjmp */
        total += pattern_b_new_indices(i % 50 + 10, 30);
        
        /* Pattern C: Mixed pressure */
        total += pattern_c_mixed_pressure(iterations / 20 + i % 15);
        
        /* Pattern D: Register conflicts */
        total += pattern_d_register_conflicts(iterations / 30 + i % 10);
        
        /* Prevent optimization */
        if (total > 1000000) total = total % 1000000;
    }
    
    if (verbose) {
        printf("Final total: %d\n", total);
    }
    
    /* Ensure result is used */
    FORCE_USE(total);
    
    return total % 256;
}
