/* test_mcf.c - Complex CFG generator to trigger MCF special node printing */
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
HOT NOINLINE static int pattern_a_entry_exit(int iterations, int seed) {
    volatile int result = seed;
    int i, j, k;
    
    /* Create many local variables to increase register pressure */
    int r0 = seed, r1 = seed + 1, r2 = seed + 2, r3 = seed + 3;
    int r4 = seed + 4, r5 = seed + 5, r6 = seed + 6, r7 = seed + 7;
    int r8 = seed + 8, r9 = seed + 9, r10 = seed + 10, r11 = seed + 11;
    int r12 = seed + 12, r13 = seed + 13, r14 = seed + 14, r15 = seed + 15;
    
    /* Deeply nested if-else chain to create complex CFG */
    if (iterations > 0) {
        if (seed % 3 == 0) {
            for (i = 0; i < iterations; i++) {
                if (i % 2 == 0) {
                    r0 = r1 + r2;
                    r3 = r4 * r5;
                    goto label_a;  /* Create irreducible region */
                } else {
                    r6 = r7 - r8;
                    r9 = r10 / (r11 ? r11 : 1);
                }
                
                for (j = 0; j < 5; j++) {
                    if (j % 3 == 0) {
                        r12 = r13 ^ r14;
                        continue;
                    } else if (j % 3 == 1) {
                        r15 = r0 | r1;
                        break;
                    } else {
                        r2 = r3 & r4;
                    }
                    
                    for (k = 0; k < 3; k++) {
                        r5 = r6 + k;
                        if (k == 1) goto label_b;
                    }
                    
                    label_b:
                    r7 = r8 * j;
                }
                
                label_a:
                r8 = r9 + i;
            }
        } else if (seed % 3 == 1) {
            /* Another complex path */
            int x = 0;
            while (x < iterations) {
                r0 += r1;
                r1 += r2;
                r2 += r3;
                if (x % 7 == 0) goto label_a;
                x++;
            }
        } else {
            /* Yet another path */
            do {
                r3 *= r4;
                r4 *= r5;
                if (r3 > 1000) break;
            } while (r4 < 10000);
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
NOINLINE static int pattern_b_new_exit_entry(int iterations, int seed) {
    jmp_buf env;
    volatile int result = seed;
    int i;
    
    /* Many local variables */
    int v0 = seed, v1 = seed * 2, v2 = seed * 3, v3 = seed * 4;
    int v4 = seed * 5, v5 = seed * 6, v6 = seed * 7, v7 = seed * 8;
    int v8 = seed * 9, v9 = seed * 10, v10 = seed * 11, v11 = seed * 12;
    int v12 = seed * 13, v13 = seed * 14, v14 = seed * 15, v15 = seed * 16;
    
    if (setjmp(env) == 0) {
        /* Normal execution path */
        for (i = 0; i < iterations; i++) {
            v0 = v1 + v2;
            v3 = v4 - v5;
            v6 = v7 * v8;
            v9 = v10 / (v11 ? v11 : 1);
            
            /* Complex condition to sometimes trigger longjmp */
            if ((i * seed) % 13 == 0) {
                v12 = v13 ^ v14;
                v15 = v0 | v1;
                
                /* Create pressure with nested loops */
                for (int j = 0; j < 10; j++) {
                    v2 += v3;
                    v4 += v5;
                    if (j == 5 && (i % 3 == 0)) {
                        longjmp(env, 1);  /* Non-local jump */
                    }
                }
            }
            
            /* More arithmetic */
            v5 = v6 + v7;
            v8 = v9 * v10;
            v11 = v12 - v13;
            v14 = v15 ^ v0;
        }
    } else {
        /* longjmp target - different execution path */
        v0 = v1 * 2;
        v2 = v3 / 2;
        v4 = v5 + 100;
    }
    
    /* Force all variables live */
    FORCE_USE(v0); FORCE_USE(v1); FORCE_USE(v2); FORCE_USE(v3);
    FORCE_USE(v4); FORCE_USE(v5); FORCE_USE(v6); FORCE_USE(v7);
    FORCE_USE(v8); FORCE_USE(v9); FORCE_USE(v10); FORCE_USE(v11);
    FORCE_USE(v12); FORCE_USE(v13); FORCE_USE(v14); FORCE_USE(v15);
    
    result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + 
             v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    return result % 1000;
}

/* Pattern C: Mixed pressure with vector operations and large switch */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE static int pattern_c_mixed_pressure(int iterations, int seed) {
    volatile int result = seed;
    int i;
    
    /* Scalar variables */
    int s0 = seed, s1 = seed + 1, s2 = seed + 2, s3 = seed + 3;
    int s4 = seed + 4, s5 = seed + 5, s6 = seed + 6, s7 = seed + 7;
    int s8 = seed + 8, s9 = seed + 9, s10 = seed + 10, s11 = seed + 11;
    int s12 = seed + 12, s13 = seed + 13, s14 = seed + 14, s15 = seed + 15;
    
#ifdef __GNUC__
    /* Vector variables */
    v4si v0 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v1 = {seed + 4, seed + 5, seed + 6, seed + 7};
    v4si v2 = {seed + 8, seed + 9, seed + 10, seed + 11};
    v4si v3 = {seed + 12, seed + 13, seed + 14, seed + 15};
#endif
    
    for (i = 0; i < iterations; i++) {
        /* Large switch statement with 30+ cases */
        switch ((s0 + i) % 35) {
            case 0:
                s0 = s1 + s2;
                s3 = s4 * s5;
#ifdef __GNUC__
                v0 = v1 + v2;
#endif
                if (i % 3 == 0) continue;
                break;
            case 1:
                s6 = s7 - s8;
                s9 = s10 / (s11 ? s11 : 1);
#ifdef __GNUC__
                v1 = v2 - v3;
#endif
                break;
            case 2:
                s12 = s13 ^ s14;
                s15 = s0 | s1;
#ifdef __GNUC__
                v2 = v3 * v0;
#endif
                if (i % 5 == 0) break;
                /* fall through */
            case 3:
                s2 = s3 & s4;
                s5 = s6 + s7;
#ifdef __GNUC__
                v3 = v0 + v1;
#endif
                break;
            case 4:
                s8 = s9 * s10;
                s11 = s12 - s13;
                break;
            case 5:
                s14 = s15 ^ s0;
                s1 = s2 | s3;
                break;
            case 6:
                s4 = s5 & s6;
                s7 = s8 + s9;
                break;
            case 7:
                s10 = s11 * s12;
                s13 = s14 - s15;
                break;
            case 8:
                s0 = s1 ^ s2;
                s3 = s4 | s5;
                break;
            case 9:
                s6 = s7 & s8;
                s9 = s10 + s11;
                break;
            case 10:
                s12 = s13 * s14;
                s15 = s0 - s1;
                break;
            case 11:
                s2 = s3 ^ s4;
                s5 = s6 | s7;
                break;
            case 12:
                s8 = s9 & s10;
                s11 = s12 + s13;
                break;
            case 13:
                s14 = s15 * s0;
                s1 = s2 - s3;
                break;
            case 14:
                s4 = s5 ^ s6;
                s7 = s8 | s9;
                break;
            case 15:
                s10 = s11 & s12;
                s13 = s14 + s15;
                break;
            case 16:
                s0 = s1 * s2;
                s3 = s4 - s5;
                break;
            case 17:
                s6 = s7 ^ s8;
                s9 = s10 | s11;
                break;
            case 18:
                s12 = s13 & s14;
                s15 = s0 + s1;
                break;
            case 19:
                s2 = s3 * s4;
                s5 = s6 - s7;
                break;
            case 20:
                s8 = s9 ^ s10;
                s11 = s12 | s13;
                break;
            case 21:
                s14 = s15 & s0;
                s1 = s2 + s3;
                break;
            case 22:
                s4 = s5 * s6;
                s7 = s8 - s9;
                break;
            case 23:
                s10 = s11 ^ s12;
                s13 = s14 | s15;
                break;
            case 24:
                s0 = s1 & s2;
                s3 = s4 + s5;
                break;
            case 25:
                s6 = s7 * s8;
                s9 = s10 - s11;
                break;
            case 26:
                s12 = s13 ^ s14;
                s15 = s0 | s1;
                break;
            case 27:
                s2 = s3 & s4;
                s5 = s6 + s7;
                break;
            case 28:
                s8 = s9 * s10;
                s11 = s12 - s13;
                break;
            case 29:
                s14 = s15 ^ s0;
                s1 = s2 | s3;
                break;
            case 30:
                s4 = s5 & s6;
                s7 = s8 + s9;
                break;
            case 31:
                s10 = s11 * s12;
                s13 = s14 - s15;
                break;
            case 32:
                s0 = s1 ^ s2;
                s3 = s4 | s5;
                break;
            case 33:
                s6 = s7 & s8;
                s9 = s10 + s11;
                break;
            case 34:
                s12 = s13 * s14;
                s15 = s0 - s1;
                if (i % 7 == 0) break;
                continue;
        }
        
        /* Additional vector operations outside switch */
#ifdef __GNUC__
        v0 = v0 + v1;
        v2 = v2 * v3;
        v1 = v1 - v0;
        v3 = v3 + v2;
#endif
    }
    
    /* Force all variables live */
    FORCE_USE(s0); FORCE_USE(s1); FORCE_USE(s2); FORCE_USE(s3);
    FORCE_USE(s4); FORCE_USE(s5); FORCE_USE(s6); FORCE_USE(s7);
    FORCE_USE(s8); FORCE_USE(s9); FORCE_USE(s10); FORCE_USE(s11);
    FORCE_USE(s12); FORCE_USE(s13); FORCE_USE(s14); FORCE_USE(s15);
    
#ifdef __GNUC__
    FORCE_USE(v0); FORCE_USE(v1); FORCE_USE(v2); FORCE_USE(v3);
#endif
    
    result = s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + 
             s8 + s9 + s10 + s11 + s12 + s13 + s14 + s15;
    return result % 1000;
}

/* Pattern D: Artificial register conflicts */
NOINLINE static int pattern_d_register_conflict(int iterations, int seed) {
    volatile int result = seed;
    
#ifdef __GNUC__
    /* Explicit register variables */
    register int x asm ("r10") = seed;
    register int y asm ("r11") = seed * 2;
    register int z asm ("r12") = seed * 3;
#endif
    
    int a = seed + 1, b = seed + 2, c = seed + 3, d = seed + 4;
    int e = seed + 5, f = seed + 6, g = seed + 7, h = seed + 8;
    int i = seed + 9, j = seed + 10, k = seed + 11, l = seed + 12;
    int m = seed + 13, n = seed + 14, o = seed + 15, p = seed + 16;
    
    /* Dummy helper functions to force calls */
    auto void dummy1(int *ptr) NOINLINE { *ptr += 1; }
    auto void dummy2(int *ptr) NOINLINE { *ptr *= 2; }
    auto void dummy3(int *ptr) NOINLINE { *ptr ^= 0x55; }
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Create artificial conflicts */
#ifdef __GNUC__
        x = y + z;
        dummy1(&x);
        y = z - x;
        dummy2(&y);
        z = x * y;
        dummy3(&z);
#endif
        
        /* Use all variables in complex pattern */
        a = b + c;
        b = c - d;
        c = d * e;
        d = e / (f ? f : 1);
        e = f ^ g;
        f = g | h;
        g = h & i;
        h = i + j;
        i = j * k;
        j = k - l;
        k = l ^ m;
        l = m | n;
        m = n & o;
        n = o + p;
        o = p * a;
        p = a - b;
        
        /* More complex control flow */
        switch (iter % 8) {
            case 0: dummy1(&a); break;
            case 1: dummy2(&b); break;
            case 2: dummy3(&c); break;
            case 3: dummy1(&d); dummy2(&e); break;
            case 4: dummy2(&f); dummy3(&g); break;
            case 5: dummy3(&h); dummy1(&i); break;
            case 6: dummy1(&j); dummy2(&k); dummy3(&l); break;
            case 7: continue;
        }
    }
    
    /* Force all variables live */
    FORCE_USE(a); FORCE_USE(b); FORCE_USE(c); FORCE_USE(d);
    FORCE_USE(e); FORCE_USE(f); FORCE_USE(g); FORCE_USE(h);
    FORCE_USE(i); FORCE_USE(j); FORCE_USE(k); FORCE_USE(l);
    FORCE_USE(m); FORCE_USE(n); FORCE_USE(o); FORCE_USE(p);
    
#ifdef __GNUC__
    FORCE_USE(x); FORCE_USE(y); FORCE_USE(z);
#endif
    
    result = a + b + c + d + e + f + g + h + 
             i + j + k + l + m + n + o + p;
#ifdef __GNUC__
    result += x + y + z;
#endif
    return result % 1000;
}

/* Main driver with PGO support */
int main(int argc, char *argv[]) {
    volatile int total = 0;
    int i, iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    if (argc > 2 && strcmp(argv[2], "-v") == 0) {
        verbose = 1;
    }
    
    if (verbose) {
        printf("Starting MCF stress test with %d iterations\n", iterations);
        
        /* Check CPU features to engage target-specific heuristics */
        if (__builtin_cpu_supports("avx2")) {
            printf("AVX2 supported - engaging vector register pressure\n");
        }
    }
    
    /* Call each pattern multiple times with different seeds */
    for (i = 0; i < iterations; i++) {
        int seed = i * 123456789;
        
        /* Pattern A - ENTRY/EXIT blocks */
        total += pattern_a_entry_exit(10 + (i % 20), seed);
        
        /* Pattern B - NEW_EXIT/NEW_ENTRY with setjmp */
        if (i % 3 == 0) {
            total += pattern_b_new_exit_entry(5 + (i % 15), seed + 1);
        }
        
        /* Pattern C - Mixed pressure */
        total += pattern_c_mixed_pressure(8 + (i % 12), seed + 2);
        
        /* Pattern D - Register conflicts */
        if (i % 4 == 0) {
            total += pattern_d_register_conflict(6 + (i % 10), seed + 3);
        }
        
        /* Prevent optimization of loop */
        if (i % 100 == 0 && verbose) {
            printf("Progress: %d/%d\n", i, iterations);
        }
    }
    
    if (verbose) {
        printf("Final total: %d\n", total);
    }
    
    /* Return non-deterministic result to prevent optimization */
    return total % 256;
}
