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
HOT NOINLINE
int pattern_a_entry_exit(int iterations, int seed) {
    volatile int result = seed;
    int i, j, k;
    
    /* Create many local variables to increase register pressure */
    int r0 = seed, r1 = seed + 1, r2 = seed + 2, r3 = seed + 3;
    int r4 = seed + 4, r5 = seed + 5, r6 = seed + 6, r7 = seed + 7;
    int r8 = seed + 8, r9 = seed + 9, r10 = seed + 10, r11 = seed + 11;
    int r12 = seed + 12, r13 = seed + 13, r14 = seed + 14, r15 = seed + 15;
    
    /* Label to create irreducible region with goto */
    irreducible_region:
    
    for (i = 0; i < iterations; i++) {
        /* Deep nested if-else chain */
        if (r0 % 3 == 0) {
            if (r1 % 5 == 0) {
                if (r2 % 7 == 0) {
                    r3 = r0 * r1 + r2;
                    goto middle_block;  /* Create irreducible flow */
                } else if (r2 % 11 == 0) {
                    r4 = r1 * r2 - r0;
                    for (j = 0; j < 5; j++) {
                        r5 += r4 >> j;
                    }
                } else {
                    r6 = r0 ^ r1 ^ r2;
                }
            } else {
                r7 = r0 | r1 | r2;
                if (r7 > 1000) {
                    r8 = r7 / 3;
                }
            }
        } else if (r0 % 4 == 0) {
            r9 = r0 << 2;
            if (r9 < 0) {
                r10 = ~r9;
            }
        }
        
        middle_block:
        
        /* Another level of nesting */
        switch (r3 % 8) {
            case 0: r11 = r0 + r1; break;
            case 1: r12 = r1 + r2; break;
            case 2: r13 = r2 + r3; break;
            case 3: r14 = r3 + r4; break;
            case 4: r15 = r4 + r5; break;
            case 5: r0 = r5 + r6; break;
            case 6: r1 = r6 + r7; break;
            case 7: r2 = r7 + r8; break;
        }
        
        /* Occasionally jump back to create irreducible region */
        if ((i % 13) == 0) {
            goto irreducible_region;
        }
        
        /* Loop with continue to different points */
        for (k = 0; k < 3; k++) {
            if ((r0 + k) % 17 == 0) {
                continue;  /* Creates back-edge */
            }
            r15 += k;
        }
    }
    
    /* Force all variables to be considered live */
    FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
    FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
    FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
    FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
    
    result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + 
             r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
    
    return result % 65536;
}

/* Pattern B: setjmp/longjmp for exceptional edges requiring NEW_EXIT/NEW_ENTRY */
NOINLINE
int pattern_b_new_indices(int iterations, int seed) {
    jmp_buf env;
    volatile int result = 0;
    int i, j;
    
    /* Many local variables to increase register pressure */
    int v0 = seed, v1 = seed * 2, v2 = seed * 3, v3 = seed * 4;
    int v4 = seed * 5, v5 = seed * 6, v6 = seed * 7, v7 = seed * 8;
    int v8 = seed * 9, v9 = seed * 10, v10 = seed * 11, v11 = seed * 12;
    int v12 = seed * 13, v13 = seed * 14, v14 = seed * 15, v15 = seed * 16;
    
    if (setjmp(env) == 0) {
        /* Normal execution path */
        for (i = 0; i < iterations; i++) {
            /* Complex arithmetic that uses all variables */
            v0 = v0 * 1103515245 + 12345;
            v1 = v1 * 1664525 + 1013904223;
            v2 = v2 ^ (v2 >> 13);
            v3 = v3 ^ (v3 << 17);
            v4 = v4 ^ (v4 >> 5);
            v5 = (v5 * 7) % 65537;
            v6 = (v6 * 13) % 65537;
            v7 = (v7 * 17) % 65537;
            
            /* Nested loops with breaks */
            for (j = 0; j < 10; j++) {
                v8 += v0 >> j;
                v9 += v1 << j;
                if (v8 > 1000000) {
                    break;
                }
                if (v9 < -1000000) {
                    continue;
                }
                v10 = v8 * v9;
            }
            
            /* Conditionally longjmp to create exceptional edge */
            if ((i > 5) && (v0 % 10007 == 0)) {
                v11 = v0 + v1 + v2;
                v12 = v3 + v4 + v5;
                v13 = v6 + v7 + v8;
                v14 = v9 + v10 + v11;
                v15 = v12 + v13 + v14;
                longjmp(env, 1);
            }
        }
    } else {
        /* longjmp target - different execution path */
        v0 = v15;
        v1 = v14;
        v2 = v13;
        for (i = 0; i < 3; i++) {
            v3 = v0 * v1 + v2;
            v4 = v1 * v2 + v0;
            v5 = v2 * v0 + v1;
        }
    }
    
    /* Force all variables live */
    FORCE_USE(v0); FORCE_USE(v1); FORCE_USE(v2); FORCE_USE(v3);
    FORCE_USE(v4); FORCE_USE(v5); FORCE_USE(v6); FORCE_USE(v7);
    FORCE_USE(v8); FORCE_USE(v9); FORCE_USE(v10); FORCE_USE(v11);
    FORCE_USE(v12); FORCE_USE(v13); FORCE_USE(v14); FORCE_USE(v15);
    
    result = v0 ^ v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ 
             v8 ^ v9 ^ v10 ^ v11 ^ v12 ^ v13 ^ v14 ^ v15;
    
    return result & 0xFFFF;
}

/* Pattern C: Vector operations combined with large switch */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE
int pattern_c_mixed_pressure(int iterations, int seed) {
    volatile int result = seed;
    int i, temp;
    
    /* Scalar variables */
    int s0 = seed, s1 = seed + 1, s2 = seed + 2, s3 = seed + 3;
    int s4 = seed + 4, s5 = seed + 5, s6 = seed + 6, s7 = seed + 7;
    int s8 = seed + 8, s9 = seed + 9, s10 = seed + 10, s11 = seed + 11;
    int s12 = seed + 12, s13 = seed + 13, s14 = seed + 14, s15 = seed + 15;
    
#ifdef __GNUC__
    /* Vector variables */
    v4si v0 = {seed, seed+1, seed+2, seed+3};
    v4si v1 = {seed+4, seed+5, seed+6, seed+7};
    v4si v2 = {seed+8, seed+9, seed+10, seed+11};
    v4si v3 = {seed+12, seed+13, seed+14, seed+15};
#endif
    
    for (i = 0; i < iterations; i++) {
        /* Large switch statement with 30+ cases */
        switch (s0 % 37) {  /* Prime number to distribute cases */
            case 0:
                s1 = s0 * 3 + 1;
                s2 = s1 / 2;
                s3 = s2 ^ s1;
#ifdef __GNUC__
                v0 = v0 + v1;
#endif
                FORCE_USE(s1); FORCE_USE(s2); FORCE_USE(s3);
                break;
            case 1:
                s4 = s0 << 4;
                s5 = s4 >> 2;
                s6 = s5 | s4;
#ifdef __GNUC__
                v1 = v1 * v0;
#endif
                FORCE_USE(s4); FORCE_USE(s5); FORCE_USE(s6);
                break;
            case 2:
                s7 = s0 % 17;
                s8 = s7 * s0;
                s9 = s8 - s7;
#ifdef __GNUC__
                v2 = v2 - v0;
#endif
                FORCE_USE(s7); FORCE_USE(s8); FORCE_USE(s9);
                break;
            case 3:
                s10 = s0 & 0xFF;
                s11 = s10 ^ 0xAA;
                s12 = s11 << 1;
#ifdef __GNUC__
                v3 = v3 ^ v1;
#endif
                FORCE_USE(s10); FORCE_USE(s11); FORCE_USE(s12);
                break;
            case 4:
                s13 = ~s0;
                s14 = s13 + 1;
                s15 = s14 * 2;
                FORCE_USE(s13); FORCE_USE(s14); FORCE_USE(s15);
                break;
            case 5:
                s1 = s0 * s0;
                s2 = s1 % 65537;
                FORCE_USE(s1); FORCE_USE(s2);
                break;
            case 6:
                s3 = s0 | 0x5555;
                s4 = s3 & 0xAAAA;
                FORCE_USE(s3); FORCE_USE(s4);
                break;
            case 7:
                s5 = s0 + 0x1000;
                s6 = s5 - 0x800;
                FORCE_USE(s5); FORCE_USE(s6);
                break;
            case 8:
                s7 = s0 ^ s1;
                s8 = s7 ^ s2;
                FORCE_USE(s7); FORCE_USE(s8);
                break;
            case 9:
                s9 = s0 * 7;
                s10 = s9 / 3;
                FORCE_USE(s9); FORCE_USE(s10);
                break;
            case 10:
                s11 = s0 << 8;
                s12 = s11 >> 4;
                FORCE_USE(s11); FORCE_USE(s12);
                break;
            case 11:
                s13 = s0 % 19;
                s14 = s13 * 11;
                FORCE_USE(s13); FORCE_USE(s14);
                break;
            case 12:
                s15 = s0 & s1;
                s1 = s15 | s2;
                FORCE_USE(s15); FORCE_USE(s1);
                break;
            case 13:
                s2 = s0 + s1 + s2;
                s3 = s2 * 3;
                FORCE_USE(s2); FORCE_USE(s3);
                break;
            case 14:
                s4 = s0 - s1;
                s5 = s4 * s4;
                FORCE_USE(s4); FORCE_USE(s5);
                break;
            case 15:
                s6 = s0 / 2;
                s7 = s6 * s6;
                FORCE_USE(s6); FORCE_USE(s7);
                break;
            case 16:
                s8 = s0 ^ 0xFFFF;
                s9 = s8 + s0;
                FORCE_USE(s8); FORCE_USE(s9);
                break;
            case 17:
                s10 = s0 * s1 * s2;
                s11 = s10 % 10007;
                FORCE_USE(s10); FORCE_USE(s11);
                break;
            case 18:
                s12 = s0 | s1 | s2;
                s13 = s12 & s3;
                FORCE_USE(s12); FORCE_USE(s13);
                break;
            case 19:
                s14 = s0 << 1;
                s15 = s14 << 1;
                FORCE_USE(s14); FORCE_USE(s15);
                break;
            case 20:
                s1 = s0 >> 4;
                s2 = s1 >> 2;
                FORCE_USE(s1); FORCE_USE(s2);
                break;
            case 21:
                s3 = s0 * 1103515245;
                s4 = s3 + 12345;
                FORCE_USE(s3); FORCE_USE(s4);
                break;
            case 22:
                s5 = s0 & 0xF0F0;
                s6 = s5 | 0x0F0F;
                FORCE_USE(s5); FORCE_USE(s6);
                break;
            case 23:
                s7 = s0 + s1 * 2;
                s8 = s7 + s2 * 3;
                FORCE_USE(s7); FORCE_USE(s8);
                break;
            case 24:
                s9 = s0 % 23;
                s10 = s9 * 17;
                FORCE_USE(s9); FORCE_USE(s10);
                break;
            case 25:
                s11 = s0 ^ s1 ^ s2;
                s12 = s11 ^ s3;
                FORCE_USE(s11); FORCE_USE(s12);
                break;
            case 26:
                s13 = s0 * s0;
                s14 = s13 * s0;
                FORCE_USE(s13); FORCE_USE(s14);
                break;
            case 27:
                s15 = s0 | 0x3333;
                s1 = s15 & 0xCCCC;
                FORCE_USE(s15); FORCE_USE(s1);
                break;
            case 28:
                s2 = s0 + 0x4000;
                s3 = s2 - 0x2000;
                FORCE_USE(s2); FORCE_USE(s3);
                break;
            case 29:
                s4 = s0 % 29;
                s5 = s4 * 23;
                FORCE_USE(s4); FORCE_USE(s5);
                break;
            case 30:
                s6 = s0 << 12;
                s7 = s6 >> 6;
                FORCE_USE(s6); FORCE_USE(s7);
                break;
            case 31:
                s8 = s0 & s1 & s2;
                s9 = s8 | s3;
                FORCE_USE(s8); FORCE_USE(s9);
                break;
            case 32:
                s10 = s0 * 5;
                s11 = s10 / 2;
                FORCE_USE(s10); FORCE_USE(s11);
                break;
            case 33:
                s12 = s0 + s1;
                s13 = s12 + s2;
                s14 = s13 + s3;
                FORCE_USE(s12); FORCE_USE(s13); FORCE_USE(s14);
                break;
            case 34:
                s15 = s0 ^ (s0 >> 1);
                s1 = s15 ^ (s15 >> 2);
                FORCE_USE(s15); FORCE_USE(s1);
                break;
            case 35:
                s2 = s0 * 9;
                s3 = s2 % 65521;
                FORCE_USE(s2); FORCE_USE(s3);
                break;
            case 36:
                s4 = s0 & 0x0FF0;
                s5 = s4 | 0xF00F;
                FORCE_USE(s4); FORCE_USE(s5);
                break;
            default:
                s6 = s0;
                FORCE_USE(s6);
                break;
        }
        
        /* Vector operations in loop */
#ifdef __GNUC__
        if (i % 2 == 0) {
            v0 = v0 + v1;
            v2 = v2 * v3;
        } else {
            v1 = v1 - v0;
            v3 = v3 ^ v2;
        }
#endif
        
        /* Loop control with continue to different switch cases */
        if ((i % 7) == 0) {
            s0 = (s0 * 13 + 7) % 65537;
            continue;
        }
        
        if ((i % 11) == 0) {
            s0 = (s0 * 17 + 11) % 65537;
            /* This creates a back-edge to a different point */
        }
    }
    
    /* Combine all results */
#ifdef __GNUC__
    int vsum[4];
    memcpy(vsum, &v0, sizeof(v0));
    temp = vsum[0] + vsum[1] + vsum[2] + vsum[3];
    memcpy(vsum, &v1, sizeof(v1));
    temp += vsum[0] + vsum[1] + vsum[2] + vsum[3];
    memcpy(vsum, &v2, sizeof(v2));
    temp += vsum[0] + vsum[1] + vsum[2] + vsum[3];
    memcpy(vsum, &v3, sizeof(v3));
    temp += vsum[0] + vsum[1] + vsum[2] + vsum[3];
#else
    temp = 0;
#endif
    
    result = s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + 
             s8 + s9 + s10 + s11 + s12 + s13 + s14 + s15 + temp;
    
    return result & 0xFFFF;
}

/* Dummy helper functions for Pattern D */
NOINLINE int helper1(int a, int b) { return a + b; }
NOINLINE int helper2(int a, int b) { return a - b; }
NOINLINE int helper3(int a, int b) { return a * b; }
NOINLINE int helper4(int a, int b) { return a ^ b; }

/* Pattern D: Explicit register variables for artificial conflicts */
NOINLINE
int pattern_d_register_conflict(int iterations, int seed) {
    volatile int result = seed;
    int i;
    
    /* Explicit register variables that conflict */
#ifdef __GNUC__
    register int x asm ("r10") = seed;
    register int y asm ("r11") = seed + 1;
    register int z asm ("r12") = seed + 2;
    /* Try to use same register for different purposes */
    register int temp asm ("r10");
#else
    int x = seed, y = seed + 1, z = seed + 2, temp;
#endif
    
    int a = seed * 2, b = seed * 3, c = seed * 4, d = seed * 5;
    int e = seed * 6, f = seed * 7, g = seed * 8, h = seed * 9;
    
    for (i = 0; i < iterations; i++) {
        /* Call dummy helpers to force register saves/restores */
        a = helper1(x, y);
        b = helper2(y, z);
        
        /* Artificial conflict: use same conceptual register for different values */
#ifdef __GNUC__
        temp = x + y;
        asm volatile("" : "+r"(temp));
        x = temp;
        
        temp = y + z;
        asm volatile("" : "+r"(temp));
        y = temp;
        
        temp = z + x;
        asm volatile("" : "+r"(temp));
        z = temp;
#else
        temp = x + y; x = temp;
        temp = y + z; y = temp;
        temp = z + x; z = temp;
#endif
        
        c = helper3(a, b);
        d = helper4(c, x);
        
        /* More complex operations */
        e = (a * b + c * d) % 65537;
        f = (x ^ y ^ z) & 0xFF;
        g = (e << 4) | (f >> 4);
        h = helper1(g, helper2(e, f));
        
        /* Loop with varying back-edges */
        switch (i % 13) {
            case 0: x = helper1(x, a); break;
            case 1: y = helper2(y, b); break;
            case 2: z = helper3(z, c); break;
            case 3: x = helper4(x, d); break;
            case 4: y = helper1(y, e); break;
            case 5: z = helper2(z, f); break;
            case 6: x = helper3(x, g); break;
            case 7: y = helper4(y, h); break;
            case 8: z = helper1(z, a); break;
            case 9: x = helper2(x, b); break;
            case 10: y = helper3(y, c); break;
            case 11: z = helper4(z, d); break;
            case 12: x = y + z; y = z + x; z = x + y; break;
        }
        
        /* Force variables live across calls */
        FORCE_USE(a); FORCE_USE(b); FORCE_USE(c); FORCE_USE(d);
        FORCE_USE(e); FORCE_USE(f); FORCE_USE(g); FORCE_USE(h);
    }
    
#ifdef __GNUC__
    result = x + y + z + a + b + c + d + e + f + g + h;
#else
    result = x + y + z + a + b + c + d + e + f + g + h;
#endif
    
    return result & 0xFFFF;
}

/* Main driver with profile-guided optimization support */
int main(int argc, char *argv[]) {
    volatile int total = 0;
    int i, iterations;
    
    if (argc > 1) {
        verbose = atoi(argv[1]);
        iterations = (argc > 2) ? atoi(argv[2]) : 1000;
    } else {
        iterations = 1000;
    }
    
    if (verbose) {
        printf("Starting MCF stress test with %d iterations\n", iterations);
    }
    
    /* Use CPU features to engage target-specific register allocation */
#ifdef __GNUC__
    int has_avx2 = __builtin_cpu_supports("avx2");
    if (verbose && has_avx2) {
        printf("AVX2 supported, engaging vector register pressure\n");
    }
#endif
    
    /* Call each pattern multiple times with different seeds */
    for (i = 0; i < iterations; i++) {
        int seed = i * 1103515245 + 12345;
        
        /* Pattern A: ENTRY/EXIT block stress */
        total += pattern_a_entry_exit(10 + (i % 20), seed);
        
        /* Pattern B: NEW_EXIT/NEW_ENTRY via setjmp */
        if (i % 3 == 0) {
            total += pattern_b_new_indices(5 + (i % 15), seed ^ 0x5A5A);
        }
        
        /* Pattern C: Mixed scalar/vector pressure */
        total += pattern_c_mixed_pressure(8 + (i % 12), seed ^ 0xA5A5);
        
        /* Pattern D: Register conflict stress */
        if (i % 4 == 0) {
            total += pattern_d_register_conflict(6 + (i % 10), seed ^ 0x3C3C);
        }
        
        /* Prevent optimization of loop */
        if (total > 0x7FFFFFFF) {
            total = total & 0x7FFFFFFF;
        }
    }
    
    if (verbose) {
        printf("Final result: %d\n", total & 0xFFFF);
    }
    
    /* Return non-zero to ensure execution */
    return (total & 0xFFFF) != 0;
}
