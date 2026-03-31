/* test_mcf_coverage.c - Complex CFG generator for MCF pass coverage testing */
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

static volatile int global_sink = 0;
static int verbose = 0;

/* Pattern A: Function targeting ENTRY/EXIT block special indices */
HOT NOINLINE
int pattern_a_entry_exit(int iterations, int seed) {
    int result = seed;
    int i, j, k;
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15;
    
    /* Create irreducible region with goto */
    if (seed % 3 == 0) {
        goto irreducible_label_1;
    } else if (seed % 3 == 1) {
        goto irreducible_label_2;
    }
    
irreducible_label_1:
    for (i = 0; i < iterations; i++) {
        r0 = i * 2;
        r1 = i * 3;
        r2 = i * 5;
        r3 = i * 7;
        r4 = i * 11;
        r5 = i * 13;
        
        /* Deep if-else chain */
        if (r0 % 2 == 0) {
            if (r1 % 3 == 0) {
                if (r2 % 5 == 0) {
                    r6 = r0 + r1 + r2;
                    goto irreducible_label_2;
                } else {
                    r7 = r0 * r1 - r2;
                }
            } else if (r3 % 7 == 0) {
                r8 = r3 ^ r4;
                continue;
            }
        } else if (r4 % 11 == 0) {
            for (j = 0; j < 5; j++) {
                r9 = j * r4;
                r10 = j * r5;
                if (j == 3) break;
            }
        }
        
        /* Another irreducible jump */
        if (i % 7 == 0) goto irreducible_label_1;
        
irreducible_label_2:
        r11 = r0 ^ r1 ^ r2 ^ r3;
        r12 = r4 | r5;
        
        /* Nested loops with continue to different labels */
        for (k = 0; k < 10; k++) {
            r13 = k * r11;
            r14 = k * r12;
            if (k == r0 % 10) continue;
            if (k == r1 % 10) goto irreducible_label_1;
        }
        
        result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12 + r13 + r14;
        FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
        FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
        FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
        FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
    }
    
    return result;
}

/* Pattern B: Function using setjmp/longjmp for exceptional edges */
NOINLINE
int pattern_b_new_indices(int iterations, int seed) {
    jmp_buf env;
    int result = seed;
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15;
    
    if (setjmp(env) == 0) {
        for (int i = 0; i < iterations; i++) {
            r0 = i * 2 + seed;
            r1 = i * 3 + seed;
            r2 = i * 5 + seed;
            r3 = i * 7 + seed;
            r4 = i * 11 + seed;
            r5 = i * 13 + seed;
            r6 = i * 17 + seed;
            r7 = i * 19 + seed;
            
            /* Complex arithmetic that might overflow */
            r8 = r0 * r1 / (r2 + 1);
            r9 = r3 * r4 / (r5 + 1);
            r10 = r6 * r7 / (r0 + 1);
            
            /* Trigger longjmp on certain conditions */
            if ((r8 + r9 + r10) % 10007 == 0) {
                longjmp(env, 1);
            }
            
            /* More variables to increase pressure */
            r11 = r8 ^ r9;
            r12 = r9 ^ r10;
            r13 = r10 ^ r8;
            r14 = r11 * r12 - r13;
            r15 = r14 % 997;
            
            result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + 
                     r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
            
            FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
            FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
            FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
            FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
        }
    } else {
        /* longjmp target */
        result = (result * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return result;
}

/* Pattern C: Mixed integer/vector pressure with large switch */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE
int pattern_c_mixed_pressure(int iterations, int selector) {
    int result = 0;
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15;
    
#ifdef __GNUC__
    v4si v0, v1, v2, v3, v4, v5;
    v0 = (v4si){1, 2, 3, 4};
    v1 = (v4si){5, 6, 7, 8};
    v2 = (v4si){9, 10, 11, 12};
#endif
    
    for (int i = 0; i < iterations; i++) {
        /* Large switch statement with 30+ cases */
        switch ((selector + i) % 35) {
            case 0:
                r0 = i * 2; r1 = i * 3; r2 = i * 5;
#ifdef __GNUC__
                v3 = v0 + v1;
#endif
                result += r0 + r1 + r2;
                break;
            case 1:
                r3 = i * 7; r4 = i * 11; r5 = i * 13;
#ifdef __GNUC__
                v4 = v1 * v2;
#endif
                result += r3 + r4 + r5;
                continue; /* Back-edge to loop start */
            case 2:
                r6 = i * 17; r7 = i * 19; r8 = i * 23;
                result += r6 + r7 + r8;
                break;
            case 3:
                r9 = i * 29; r10 = i * 31; r11 = i * 37;
                result += r9 + r10 + r11;
                if (i % 3 == 0) continue;
                break;
            case 4:
                r12 = i * 41; r13 = i * 43; r14 = i * 47;
                result += r12 + r13 + r14;
                break;
            case 5:
                r15 = i * 53; r0 = i * 59; r1 = i * 61;
                result += r15 + r0 + r1;
                break;
            case 6:
                r2 = i * 67; r3 = i * 71; r4 = i * 73;
                result += r2 + r3 + r4;
                continue;
            case 7:
                r5 = i * 79; r6 = i * 83; r7 = i * 89;
                result += r5 + r6 + r7;
                break;
            case 8:
                r8 = i * 97; r9 = i * 101; r10 = i * 103;
                result += r8 + r9 + r10;
                break;
            case 9:
                r11 = i * 107; r12 = i * 109; r13 = i * 113;
                result += r11 + r12 + r13;
                if (i % 5 == 0) continue;
                break;
            case 10:
                r14 = i * 127; r15 = i * 131; r0 = i * 137;
                result += r14 + r15 + r0;
                break;
            case 11:
                r1 = i * 139; r2 = i * 149; r3 = i * 151;
                result += r1 + r2 + r3;
                continue;
            case 12:
                r4 = i * 157; r5 = i * 163; r6 = i * 167;
                result += r4 + r5 + r6;
                break;
            case 13:
                r7 = i * 173; r8 = i * 179; r9 = i * 181;
                result += r7 + r8 + r9;
                break;
            case 14:
                r10 = i * 191; r11 = i * 193; r12 = i * 197;
                result += r10 + r11 + r12;
                if (i % 7 == 0) continue;
                break;
            case 15:
                r13 = i * 199; r14 = i * 211; r15 = i * 223;
                result += r13 + r14 + r15;
                break;
            case 16:
                r0 = i * 227; r1 = i * 229; r2 = i * 233;
                result += r0 + r1 + r2;
                continue;
            case 17:
                r3 = i * 239; r4 = i * 241; r5 = i * 251;
                result += r3 + r4 + r5;
                break;
            case 18:
                r6 = i * 257; r7 = i * 263; r8 = i * 269;
                result += r6 + r7 + r8;
                break;
            case 19:
                r9 = i * 271; r10 = i * 277; r11 = i * 281;
                result += r9 + r10 + r11;
                if (i % 11 == 0) continue;
                break;
            case 20:
                r12 = i * 283; r13 = i * 293; r14 = i * 307;
                result += r12 + r13 + r14;
                break;
            case 21:
                r15 = i * 311; r0 = i * 313; r1 = i * 317;
                result += r15 + r0 + r1;
                continue;
            case 22:
                r2 = i * 331; r3 = i * 337; r4 = i * 347;
                result += r2 + r3 + r4;
                break;
            case 23:
                r5 = i * 349; r6 = i * 353; r7 = i * 359;
                result += r5 + r6 + r7;
                break;
            case 24:
                r8 = i * 367; r9 = i * 373; r10 = i * 379;
                result += r8 + r9 + r10;
                if (i % 13 == 0) continue;
                break;
            case 25:
                r11 = i * 383; r12 = i * 389; r13 = i * 397;
                result += r11 + r12 + r13;
                break;
            case 26:
                r14 = i * 401; r15 = i * 409; r0 = i * 419;
                result += r14 + r15 + r0;
                continue;
            case 27:
                r1 = i * 421; r2 = i * 431; r3 = i * 433;
                result += r1 + r2 + r3;
                break;
            case 28:
                r4 = i * 439; r5 = i * 443; r6 = i * 449;
                result += r4 + r5 + r6;
                break;
            case 29:
                r7 = i * 457; r8 = i * 461; r9 = i * 463;
                result += r7 + r8 + r9;
                if (i % 17 == 0) continue;
                break;
            case 30:
                r10 = i * 467; r11 = i * 479; r12 = i * 487;
                result += r10 + r11 + r12;
                break;
            case 31:
                r13 = i * 491; r14 = i * 499; r15 = i * 503;
                result += r13 + r14 + r15;
                continue;
            case 32:
                r0 = i * 509; r1 = i * 521; r2 = i * 523;
                result += r0 + r1 + r2;
                break;
            case 33:
                r3 = i * 541; r4 = i * 547; r5 = i * 557;
                result += r3 + r4 + r5;
                break;
            case 34:
                r6 = i * 563; r7 = i * 569; r8 = i * 571;
                result += r6 + r7 + r8;
                if (i % 19 == 0) continue;
                break;
            default:
                r9 = i * 577; r10 = i * 587; r11 = i * 593;
                result += r9 + r10 + r11;
                break;
        }
        
#ifdef __GNUC__
        v5 = v3 + v4;
        v0 = v1 + v5;
        v1 = v2 * v5;
#endif
        
        FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
        FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
        FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
        FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
    }
    
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

/* Pattern D: Artificial register conflicts */
NOINLINE
int pattern_d_register_conflict(int iterations, int seed) {
    /* Explicit register variables creating conflicts */
    REGISTER_VAR(conflict_a, "r10");
    REGISTER_VAR(conflict_b, "r11");
    REGISTER_VAR(conflict_c, "r12");
    
    int result = seed;
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15;
    
    conflict_a = seed * 3;
    conflict_b = seed * 5;
    conflict_c = seed * 7;
    
    for (int i = 0; i < iterations; i++) {
        r0 = i * 2 + conflict_a;
        r1 = i * 3 + conflict_b;
        r2 = i * 5 + conflict_c;
        
        /* Call dummy helpers to split live ranges */
        if (i % 3 == 0) {
            dummy_helper1(r0, r1, r2);
            conflict_a = r0 ^ r1;
        }
        
        r3 = i * 7 + conflict_a;
        r4 = i * 11 + conflict_b;
        r5 = i * 13 + conflict_c;
        
        if (i % 5 == 0) {
            dummy_helper2(r3, r4, r5, conflict_a);
            conflict_b = r3 | r4;
        }
        
        r6 = i * 17 + conflict_a;
        r7 = i * 19 + conflict_b;
        r8 = i * 23 + conflict_c;
        
        if (i % 7 == 0) {
            dummy_helper3(r6, r7, r8, conflict_b, conflict_c);
            conflict_c = r6 & r7;
        }
        
        r9 = i * 29 + conflict_a;
        r10 = i * 31 + conflict_b;
        r11 = i * 37 + conflict_c;
        
        r12 = r0 + r3 + r6 + r9;
        r13 = r1 + r4 + r7 + r10;
        r14 = r2 + r5 + r8 + r11;
        r15 = r12 * r13 - r14;
        
        result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + 
                 r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15 +
                 conflict_a + conflict_b + conflict_c;
        
        FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
        FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
        FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
        FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
    }
    
    return result;
}

/* Main driver with profile-guided optimization support */
int main(int argc, char *argv[]) {
    int total = 0;
    int iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 100) iterations = 100;
        if (iterations > 10000) iterations = 10000;
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
    
    /* Call each pattern multiple times with different arguments */
    for (int i = 0; i < 10; i++) {
        int seed = i * 1234567;
        
        /* Pattern A - targeting ENTRY/EXIT blocks */
        total += pattern_a_entry_exit(iterations / 10, seed);
        
        /* Pattern B - targeting NEW_EXIT/NEW_ENTRY with setjmp */
        total += pattern_b_new_indices(iterations / 20, seed + 1);
        
        /* Pattern C - mixed pressure with large switch */
        total += pattern_c_mixed_pressure(iterations / 5, seed + 2);
        
        /* Pattern D - artificial register conflicts */
        total += pattern_d_register_conflict(iterations / 8, seed + 3);
        
        /* Force use of AVX2 detection result */
        if (use_avx2) {
            total = (total * 1103515245 + 12345) & 0x7fffffff;
        }
    }
    
    /* Store result in volatile to prevent optimization */
    global_sink = total;
    
    if (verbose) {
        printf("Final result: %d\n", total);
        printf("Global sink: %d\n", global_sink);
    }
    
    return 0;
}
