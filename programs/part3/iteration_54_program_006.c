/* test_mcf.c - Complex CFG generator to trigger MCF special node printing */
#ifdef __GNUC__
#define FORCE_LIVE(var) asm volatile("" : : "g"(var))
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define COLD __attribute__((cold))
#else
#define FORCE_LIVE(var) (void)(var)
#define NOINLINE
#define HOT
#define COLD
#endif

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

static int verbose = 0;

/* Pattern A: Deep nested if-else with irreducible region for ENTRY/EXIT blocks */
HOT NOINLINE int pattern_a_irreducible(int iterations, int seed) {
    volatile int result = seed;
    int i, j, k;
    
    /* Create complex entry block with many variables */
    int a0 = seed, a1 = seed + 1, a2 = seed + 2, a3 = seed + 3;
    int a4 = seed + 4, a5 = seed + 5, a6 = seed + 6, a7 = seed + 7;
    int a8 = seed + 8, a9 = seed + 9, a10 = seed + 10, a11 = seed + 11;
    int a12 = seed + 12, a13 = seed + 13, a14 = seed + 14, a15 = seed + 15;
    
    /* Label to create irreducible region */
    irreducible_start:
    
    for (i = 0; i < iterations; i++) {
        /* Deeply nested if-else chain */
        if (a0 & 1) {
            if (a1 & 2) {
                for (j = 0; j < 3; j++) {
                    if (a2 & (1 << j)) {
                        a3 += a4 * j;
                        if (a5 > 100) {
                            a6 -= a7;
                            goto irreducible_start; /* Create irreducible CFG */
                        }
                    } else {
                        a8 ^= a9;
                    }
                }
            } else if (a10 < a11) {
                a12 = a13 * a14;
                if (a15 & 0xF0) {
                    continue;
                }
            }
        } else if (a1 & 4) {
            do {
                a2 = a3 ^ a4;
                a5 = a6 + a7;
                k = 0;
                while (k < 2) {
                    a8 += a9 >> k;
                    k++;
                    if (k == 1 && (a10 & 1))
                        break;
                }
            } while (a11-- > 0);
        }
        
        /* More complex branching */
        switch (a0 & 7) {
            case 0: a1 = a2 + a3; break;
            case 1: a4 = a5 - a6; break;
            case 2: a7 = a8 * a9; break;
            case 3: a10 = a11 ^ a12; break;
            case 4: a13 = a14 | a15; break;
            case 5: a0 = a1 & a2; break;
            case 6: a3 = a4 << 2; break;
            case 7: a5 = a6 >> 1; break;
        }
        
        result += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + 
                  a8 + a9 + a10 + a11 + a12 + a13 + a14 + a15;
    }
    
    FORCE_LIVE(a0); FORCE_LIVE(a1); FORCE_LIVE(a2); FORCE_LIVE(a3);
    FORCE_LIVE(a4); FORCE_LIVE(a5); FORCE_LIVE(a6); FORCE_LIVE(a7);
    FORCE_LIVE(a8); FORCE_LIVE(a9); FORCE_LIVE(a10); FORCE_LIVE(a11);
    FORCE_LIVE(a12); FORCE_LIVE(a13); FORCE_LIVE(a14); FORCE_LIVE(a15);
    
    return result;
}

/* Pattern B: Large switch with many variables for high register pressure */
NOINLINE int pattern_b_large_switch(int mode, int count) {
    volatile int result = 0;
    int i;
    
    for (i = 0; i < count; i++) {
        /* Many local variables to create register pressure */
        int r0 = i, r1 = i + 1, r2 = i + 2, r3 = i + 3;
        int r4 = i + 4, r5 = i + 5, r6 = i + 6, r7 = i + 7;
        int r8 = i + 8, r9 = i + 9, r10 = i + 10, r11 = i + 11;
        int r12 = i + 12, r13 = i + 13, r14 = i + 14, r15 = i + 15;
        int r16 = i + 16, r17 = i + 17, r18 = i + 18, r19 = i + 19;
        int r20 = i + 20, r21 = i + 21, r22 = i + 22, r23 = i + 23;
        int r24 = i + 24, r25 = i + 25, r26 = i + 26, r27 = i + 27;
        int r28 = i + 28, r29 = i + 29, r30 = i + 30, r31 = i + 31;
        
        /* Large switch with continue/break to different cases */
        switch ((mode + i) % 35) {
            case 0:
                r0 = r1 * r2 + r3;
                r4 = r5 ^ r6 | r7;
                if (r8 > 100) continue;
                break;
            case 1:
                r9 = r10 - r11 * r12;
                r13 = r14 & r15;
                if (r16 & 1) break;
                else continue;
            case 2:
                r17 = r18 + r19 - r20;
                r21 = r22 * r23 >> 2;
                break;
            case 3:
                r24 = r25 | r26 & r27;
                r28 = r29 ^ r30;
                if (r31 < 50) continue;
                break;
            case 4:
                r0 = r1 + r2 * r3;
                r4 = r5 - r6 / 2;
                break;
            case 5:
                r7 = r8 | r9 & r10;
                r11 = r12 ^ r13;
                if (r14 > 200) break;
                continue;
            case 6:
                r15 = r16 + r17 * r18;
                r19 = r20 - r21;
                break;
            case 7:
                r22 = r23 | r24 & r25;
                r26 = r27 ^ r28;
                break;
            case 8:
                r29 = r30 + r31 * r0;
                r1 = r2 - r3;
                if (r4 & 4) continue;
                break;
            case 9:
                r5 = r6 | r7 & r8;
                r9 = r10 ^ r11;
                break;
            case 10:
                r12 = r13 + r14 * r15;
                r16 = r17 - r18;
                break;
            case 11:
                r19 = r20 | r21 & r22;
                r23 = r24 ^ r25;
                if (r26 > 150) continue;
                break;
            case 12:
                r27 = r28 + r29 * r30;
                r31 = r0 - r1;
                break;
            case 13:
                r2 = r3 | r4 & r5;
                r6 = r7 ^ r8;
                break;
            case 14:
                r9 = r10 + r11 * r12;
                r13 = r14 - r15;
                if (r16 & 8) continue;
                break;
            case 15:
                r17 = r18 | r19 & r20;
                r21 = r22 ^ r23;
                break;
            case 16:
                r24 = r25 + r26 * r27;
                r28 = r29 - r30;
                break;
            case 17:
                r31 = r0 | r1 & r2;
                r3 = r4 ^ r5;
                if (r6 > 250) continue;
                break;
            case 18:
                r7 = r8 + r9 * r10;
                r11 = r12 - r13;
                break;
            case 19:
                r14 = r15 | r16 & r17;
                r18 = r19 ^ r20;
                break;
            case 20:
                r21 = r22 + r23 * r24;
                r25 = r26 - r27;
                if (r28 & 16) continue;
                break;
            case 21:
                r29 = r30 | r31 & r0;
                r1 = r2 ^ r3;
                break;
            case 22:
                r4 = r5 + r6 * r7;
                r8 = r9 - r10;
                break;
            case 23:
                r11 = r12 | r13 & r14;
                r15 = r16 ^ r17;
                if (r18 > 300) continue;
                break;
            case 24:
                r19 = r20 + r21 * r22;
                r23 = r24 - r25;
                break;
            case 25:
                r26 = r27 | r28 & r29;
                r30 = r31 ^ r0;
                break;
            case 26:
                r1 = r2 + r3 * r4;
                r5 = r6 - r7;
                if (r8 & 32) continue;
                break;
            case 27:
                r9 = r10 | r11 & r12;
                r13 = r14 ^ r15;
                break;
            case 28:
                r16 = r17 + r18 * r19;
                r20 = r21 - r22;
                break;
            case 29:
                r23 = r24 | r25 & r26;
                r27 = r28 ^ r29;
                if (r30 > 350) continue;
                break;
            case 30:
                r31 = r0 + r1 * r2;
                r3 = r4 - r5;
                break;
            case 31:
                r6 = r7 | r8 & r9;
                r10 = r11 ^ r12;
                break;
            case 32:
                r13 = r14 + r15 * r16;
                r17 = r18 - r19;
                if (r20 & 64) continue;
                break;
            case 33:
                r21 = r22 | r23 & r24;
                r25 = r26 ^ r27;
                break;
            case 34:
                r28 = r29 + r30 * r31;
                r0 = r1 - r2;
                break;
        }
        
        result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 +
                  r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 + r20 +
                  r21 + r22 + r23 + r24 + r25 + r26 + r27 + r28 + r29 + r30 + r31;
        
        /* Force all variables to be considered live */
        FORCE_LIVE(r0); FORCE_LIVE(r1); FORCE_LIVE(r2); FORCE_LIVE(r3);
        FORCE_LIVE(r4); FORCE_LIVE(r5); FORCE_LIVE(r6); FORCE_LIVE(r7);
        FORCE_LIVE(r8); FORCE_LIVE(r9); FORCE_LIVE(r10); FORCE_LIVE(r11);
        FORCE_LIVE(r12); FORCE_LIVE(r13); FORCE_LIVE(r14); FORCE_LIVE(r15);
        FORCE_LIVE(r16); FORCE_LIVE(r17); FORCE_LIVE(r18); FORCE_LIVE(r19);
        FORCE_LIVE(r20); FORCE_LIVE(r21); FORCE_LIVE(r22); FORCE_LIVE(r23);
        FORCE_LIVE(r24); FORCE_LIVE(r25); FORCE_LIVE(r26); FORCE_LIVE(r27);
        FORCE_LIVE(r28); FORCE_LIVE(r29); FORCE_LIVE(r30); FORCE_LIVE(r31);
    }
    
    return result;
}

/* Pattern C: setjmp/longjmp for exceptional edges */
static jmp_buf jump_buffer;
NOINLINE int pattern_c_setjmp(int iterations) {
    volatile int result = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        int v0 = i, v1 = i * 2, v2 = i * 3, v3 = i * 4;
        int v4 = i * 5, v5 = i * 6, v6 = i * 7, v7 = i * 8;
        int v8 = i * 9, v9 = i * 10, v10 = i * 11, v11 = i * 12;
        
        if (setjmp(jump_buffer) == 0) {
            /* Normal path with complex computations */
            v0 = v1 + v2 * v3;
            v4 = v5 ^ v6 | v7;
            v8 = v9 & v10 + v11;
            
            /* Occasionally longjmp to create exceptional edge */
            if ((i & 0xF) == 0) {
                longjmp(jump_buffer, 1);
            }
            
            result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11;
        } else {
            /* Exception path */
            v0 = v1 - v2;
            v3 = v4 * v5;
            v6 = v7 | v8;
            v9 = v10 ^ v11;
            
            result -= v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11;
        }
        
        FORCE_LIVE(v0); FORCE_LIVE(v1); FORCE_LIVE(v2); FORCE_LIVE(v3);
        FORCE_LIVE(v4); FORCE_LIVE(v5); FORCE_LIVE(v6); FORCE_LIVE(v7);
        FORCE_LIVE(v8); FORCE_LIVE(v9); FORCE_LIVE(v10); FORCE_LIVE(v11);
    }
    
    return result;
}

/* Pattern D: Vector operations mixed with scalar pressure */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE int pattern_d_vector_mix(int count) {
    volatile int result = 0;
    int i;
    
    for (i = 0; i < count; i++) {
        /* Scalar variables */
        int s0 = i, s1 = i + 1, s2 = i + 2, s3 = i + 3;
        int s4 = i + 4, s5 = i + 5, s6 = i + 6, s7 = i + 7;
        int s8 = i + 8, s9 = i + 9, s10 = i + 10, s11 = i + 11;
        int s12 = i + 12, s13 = i + 13, s14 = i + 14, s15 = i + 15;
        
#ifdef __GNUC__
        /* Vector variables */
        v4si v0 = {i, i+1, i+2, i+3};
        v4si v1 = {i+4, i+5, i+6, i+7};
        v4si v2 = {i+8, i+9, i+10, i+11};
        v4si v3 = {i+12, i+13, i+14, i+15};
        
        /* Mixed vector and scalar operations */
        v0 = v0 + v1 * v2;
        v3 = v3 - v0;
        
        /* Extract elements to scalars */
        s0 += v0[0]; s1 += v0[1]; s2 += v0[2]; s3 += v0[3];
        s4 += v1[0]; s5 += v1[1]; s6 += v1[2]; s7 += v1[3];
        s8 += v2[0]; s9 += v2[1]; s10 += v2[2]; s11 += v2[3];
        s12 += v3[0]; s13 += v3[1]; s14 += v3[2]; s15 += v3[3];
#endif
        
        /* Complex switch with vector-scalar mixing */
        switch (i % 20) {
            case 0: s0 = s1 + s2 * s3; break;
            case 1: s4 = s5 ^ s6 | s7; break;
            case 2: s8 = s9 & s10 + s11; break;
            case 3: s12 = s13 - s14 * s15; break;
            case 4: s0 = s1 | s2 & s3; break;
            case 5: s4 = s5 + s6 - s7; break;
            case 6: s8 = s9 * s10 >> 2; break;
            case 7: s12 = s13 ^ s14 | s15; break;
            case 8: s0 = s1 & s2 + s3; break;
            case 9: s4 = s5 - s6 * s7; break;
            case 10: s8 = s9 | s10 ^ s11; break;
            case 11: s12 = s13 + s14 & s15; break;
            case 12: s0 = s1 * s2 - s3; break;
            case 13: s4 = s5 | s6 + s7; break;
            case 14: s8 = s9 ^ s10 * s11; break;
            case 15: s12 = s13 & s14 - s15; break;
            case 16: s0 = s1 + s2 | s3; break;
            case 17: s4 = s5 * s6 ^ s7; break;
            case 18: s8 = s9 - s10 & s11; break;
            case 19: s12 = s13 | s14 * s15; break;
        }
        
        result += s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8 + s9 + s10 +
                  s11 + s12 + s13 + s14 + s15;
        
        FORCE_LIVE(s0); FORCE_LIVE(s1); FORCE_LIVE(s2); FORCE_LIVE(s3);
        FORCE_LIVE(s4); FORCE_LIVE(s5); FORCE_LIVE(s6); FORCE_LIVE(s7);
        FORCE_LIVE(s8); FORCE_LIVE(s9); FORCE_LIVE(s10); FORCE_LIVE(s11);
        FORCE_LIVE(s12); FORCE_LIVE(s13); FORCE_LIVE(s14); FORCE_LIVE(s15);
    }
    
    return result;
}

/* Pattern E: Explicit register variables for artificial conflicts */
NOINLINE int pattern_e_register_conflict(int iterations) {
    volatile int result = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
#ifdef __GNUC__
        /* Explicit register variables that may conflict */
        register int x asm ("r10") = i;
        register int y asm ("r11") = i * 2;
        register int z asm ("r12") = i * 3;
        register int w asm ("r13") = i * 4;
#else
        int x = i, y = i * 2, z = i * 3, w = i * 4;
#endif
        
        int a = i + 1, b = i + 2, c = i + 3, d = i + 4;
        int e = i + 5, f = i + 6, g = i + 7, h = i + 8;
        int j = i + 9, k = i + 10, l = i + 11, m = i + 12;
        int n = i + 13, o = i + 14, p = i + 15, q = i + 16;
        
        /* Complex operations forcing spills */
        x = y + z * w;
        a = b ^ c | d;
        e = f & g + h;
        j = k - l * m;
        n = o | p ^ q;
        
        /* Nested loops with register pressure */
        for (int t = 0; t < 3; t++) {
            x = x + a * t;
            y = y ^ b + t;
            z = z & c - t;
            w = w | d * t;
        }
        
        result += x + y + z + w + a + b + c + d + e + f + g + h + j + k + l + m + n + o + p + q;
        
        FORCE_LIVE(x); FORCE_LIVE(y); FORCE_LIVE(z); FORCE_LIVE(w);
        FORCE_LIVE(a); FORCE_LIVE(b); FORCE_LIVE(c); FORCE_LIVE(d);
        FORCE_LIVE(e); FORCE_LIVE(f); FORCE_LIVE(g); FORCE_LIVE(h);
        FORCE_LIVE(j); FORCE_LIVE(k); FORCE_LIVE(l); FORCE_LIVE(m);
        FORCE_LIVE(n); FORCE_LIVE(o); FORCE_LIVE(p); FORCE_LIVE(q);
    }
    
    return result;
}

/* Dummy helper functions for Pattern D */
NOINLINE int helper1(int a, int b) { return a + b; }
NOINLINE int helper2(int a, int b) { return a - b; }
NOINLINE int helper3(int a, int b) { return a * b; }
NOINLINE int helper4(int a, int b) { return a ^ b; }

/* Main function to drive all patterns with profile feedback */
int main(int argc, char **argv) {
    volatile int total = 0;
    int i, iterations = 100;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 10000) iterations = 10000;
    }
    
    if (argc > 2 && strcmp(argv[2], "-v") == 0) {
        verbose = 1;
    }
    
    if (verbose) {
        printf("Starting MCF stress test with %d iterations\n", iterations);
    }
    
    /* Use CPU features to engage target-specific register allocation */
    int use_avx2 = 0;
#ifdef __GNUC__
    use_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 support: %s\n", use_avx2 ? "yes" : "no");
    }
#endif
    
    /* Run all patterns multiple times to build profile data */
    for (i = 0; i < iterations; i++) {
        int seed = i * 12345;
        
        /* Pattern A: Irreducible region */
        total += pattern_a_irreducible(10 + (i % 5), seed);
        
        /* Pattern B: Large switch */
        total += pattern_b_large_switch(i % 7, 5 + (i % 3));
        
        /* Pattern C: setjmp/longjmp */
        total += pattern_c_setjmp(3 + (i % 4));
        
        /* Pattern D: Vector mix */
        total += pattern_d_vector_mix(8 + (i % 6));
        
        /* Pattern E: Register conflicts */
        total += pattern_e_register_conflict(6 + (i % 5));
        
        /* Call dummy helpers to prevent inlining */
        total += helper1(i, i+1);
        total += helper2(i, i+2);
        total += helper3(i, i+3);
        total += helper4(i, i+4);
        
        /* Occasionally print progress */
        if (verbose && (i % (iterations/10)) == 0 && i > 0) {
            printf("Progress: %d/%d\n", i, iterations);
        }
    }
    
    if (verbose) {
        printf("Total result: %d\n", total);
    }
    
    /* Ensure result is used */
    FORCE_LIVE(total);
    
    return total != 0 ? 0 : 1;
}
