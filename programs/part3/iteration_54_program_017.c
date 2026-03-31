/* test_mcf.c - Comprehensive test to trigger MCF special node printing */
#ifdef __GNUC__
#define FORCE_USE(var) asm volatile("" : : "g"(var))
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define VECTOR_TYPE __attribute__((vector_size(16)))
#else
#define FORCE_USE(var) (void)(var)
#define NOINLINE
#define HOT
#define VECTOR_TYPE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

static int verbose = 0;

/* ========== PATTERN A: ENTRY/EXIT BLOCKS ========== */
HOT NOINLINE static int pattern_a_entry_exit(int iterations, int seed) {
    volatile int result = seed;
    int i, j, k;
    
    /* Create complex control flow with irreducible region */
    if (seed > 0) {
        goto label_irreducible;
    }
    
    for (i = 0; i < iterations; i++) {
        if (i % 3 == 0) {
            /* Deep if-else chain */
            if (i % 5 == 0) {
                if (i % 7 == 0) {
                    result += i * 2;
                } else if (i % 11 == 0) {
                    result -= i * 3;
                } else {
                    result ^= i;
                }
            } else if (i % 13 == 0) {
                for (j = 0; j < 5; j++) {
                    result += j;
                    if (j == 3) break;
                }
            }
        } else if (i % 2 == 0) {
            /* Another nested loop */
            for (k = 0; k < 10; k++) {
                if (k == 5) continue;
                result += k * i;
            }
        }
        
        /* Irreducible region using goto */
        label_irreducible:
        if (i % 17 == 0) {
            result |= 0xFF;
            goto label_inside_loop;
        }
        
        label_inside_loop:
        if (i % 19 == 0) {
            result &= 0x7F;
        }
    }
    
    /* More irreducible control flow */
    if (result > 1000) {
        goto final_label;
    } else if (result > 500) {
        result >>= 2;
    }
    
    final_label:
    return result;
}

/* ========== PATTERN B: NEW_EXIT/NEW_ENTRY with setjmp ========== */
static jmp_buf jump_buffer;
static int jump_counter = 0;

NOINLINE static void helper_b1(int *p) {
    *p += 42;
    FORCE_USE(p);
}

NOINLINE static void helper_b2(int *p) {
    *p *= 3;
    FORCE_USE(p);
}

static int pattern_b_new_nodes(int iterations) {
    int r0 = 1, r1 = 2, r2 = 3, r3 = 4, r4 = 5;
    int r5 = 6, r6 = 7, r7 = 8, r8 = 9, r9 = 10;
    int r10 = 11, r11 = 12, r12 = 13, r13 = 14, r14 = 15, r15 = 16;
    
    volatile int result = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        for (int i = 0; i < iterations; i++) {
            /* Use all registers in complex ways */
            r0 = r1 + r2; r1 = r3 * r4; r2 = r5 ^ r6;
            r3 = r7 | r8; r4 = r9 & r10; r5 = r11 << r12;
            r6 = r13 >> r14; r7 = r15 + i; r8 = r0 * r1;
            r9 = r2 - r3; r10 = r4 / (r5 + 1); r11 = r6 % (r7 + 1);
            r12 = r8 | r9; r13 = r10 ^ r11; r14 = r12 & r13;
            r15 = r14 + r0;
            
            /* Force all variables live */
            FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
            FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
            FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
            FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
            
            /* Occasionally call helpers */
            if (i % 7 == 0) helper_b1(&result);
            if (i % 13 == 0) helper_b2(&result);
            
            /* Trigger longjmp after many iterations */
            if (i == iterations / 2) {
                jump_counter++;
                longjmp(jump_buffer, 1);
            }
        }
    } else {
        /* After longjmp */
        result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + 
                 r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
    }
    
    return result;
}

/* ========== PATTERN C: MIXED PRESSURE with vector ops ========== */
#ifdef __GNUC__
typedef int v4si VECTOR_TYPE;
#else
typedef int v4si[4];
#endif

NOINLINE static int pattern_c_mixed_pressure(int selector, int iterations) {
    /* Scalar pressure variables */
    int s0 = 1, s1 = 2, s2 = 3, s3 = 4, s4 = 5, s5 = 6, s6 = 7, s7 = 8;
    int s8 = 9, s9 = 10, s10 = 11, s11 = 12, s12 = 13, s13 = 14, s14 = 15, s15 = 16;
    
    /* Vector pressure variables */
#ifdef __GNUC__
    v4si v0 = {1, 2, 3, 4};
    v4si v1 = {5, 6, 7, 8};
    v4si v2 = {9, 10, 11, 12};
    v4si v3 = {13, 14, 15, 16};
#endif
    
    volatile int result = 0;
    
    /* Large switch with 30+ cases */
    for (int i = 0; i < iterations; i++) {
        switch ((selector + i) % 35) {
            case 0:
                s0 = s1 + s2; s1 = s3 * s4; s2 = s5 ^ s6;
                result += s0 + s1 + s2;
#ifdef __GNUC__
                v0 = v1 + v2;
#endif
                break;
            case 1:
                s3 = s7 | s8; s4 = s9 & s10; s5 = s11 << s12;
                result += s3 + s4 + s5;
#ifdef __GNUC__
                v1 = v2 - v0;
#endif
                break;
            case 2:
                s6 = s13 >> s14; s7 = s15 + i; s8 = s0 * s1;
                result += s6 + s7 + s8;
                break;
            case 3:
                s9 = s2 - s3; s10 = s4 / (s5 + 1); s11 = s6 % (s7 + 1);
                result += s9 + s10 + s11;
#ifdef __GNUC__
                v2 = v0 * v1;
#endif
                break;
            case 4:
                s12 = s8 | s9; s13 = s10 ^ s11; s14 = s12 & s13;
                result += s12 + s13 + s14;
                break;
            case 5:
                s15 = s14 + s0; s0 = s1 - s2; s1 = s3 * s4;
                result += s15 + s0 + s1;
#ifdef __GNUC__
                v3 = v0 + v1 + v2;
#endif
                break;
            case 6:
                s2 = s5 | s6; s3 = s7 & s8; s4 = s9 ^ s10;
                result += s2 + s3 + s4;
                break;
            case 7:
                s5 = s11 << s12; s6 = s13 >> s14; s7 = s15 * i;
                result += s5 + s6 + s7;
                break;
            case 8:
                s8 = s0 + s1; s9 = s2 - s3; s10 = s4 * s5;
                result += s8 + s9 + s10;
                break;
            case 9:
                s11 = s6 | s7; s12 = s8 & s9; s13 = s10 ^ s11;
                result += s11 + s12 + s13;
#ifdef __GNUC__
                v0 = v1 | v2;
#endif
                break;
            /* 25 more cases following similar pattern */
            case 10: case 11: case 12: case 13: case 14:
            case 15: case 16: case 17: case 18: case 19:
            case 20: case 21: case 22: case 23: case 24:
            case 25: case 26: case 27: case 28: case 29:
            case 30: case 31: case 32: case 33: case 34:
                /* Each case has unique arithmetic */
                s0 = s1 + (i % case_number);
                s1 = s2 * (case_number + 1);
                s2 = s3 ^ (case_number * 2);
                result += s0 + s1 + s2 + case_number;
#ifdef __GNUC__
                if (case_number % 4 == 0) {
                    v0[case_number % 4] = v1[case_number % 4] + v2[case_number % 4];
                }
#endif
                break;
        }
        
        /* Force all scalars live */
        FORCE_USE(s0); FORCE_USE(s1); FORCE_USE(s2); FORCE_USE(s3);
        FORCE_USE(s4); FORCE_USE(s5); FORCE_USE(s6); FORCE_USE(s7);
        FORCE_USE(s8); FORCE_USE(s9); FORCE_USE(s10); FORCE_USE(s11);
        FORCE_USE(s12); FORCE_USE(s13); FORCE_USE(s14); FORCE_USE(s15);
        
#ifdef __GNUC__
        /* Force all vectors live */
        FORCE_USE(v0); FORCE_USE(v1); FORCE_USE(v2); FORCE_USE(v3);
#endif
        
        /* Complex loop control */
        if (i % 11 == 0) continue;
        if (i % 23 == 0) break;
        if (i % 29 == 0) {
            i += 2;
            continue;
        }
    }
    
    return result;
}

/* ========== PATTERN D: ARTIFICIAL CONFLICT ========== */
#ifdef __GNUC__
register int conflict_var1 asm ("r10");
register int conflict_var2 asm ("r10");
#else
int conflict_var1;
int conflict_var2;
#endif

NOINLINE static void dummy_helper1(int x) {
    FORCE_USE(x);
}

NOINLINE static void dummy_helper2(int x) {
    FORCE_USE(x);
}

NOINLINE static void dummy_helper3(int x) {
    FORCE_USE(x);
}

static int pattern_d_artificial_conflict(int iterations) {
    int local1 = 1, local2 = 2, local3 = 3, local4 = 4;
    int local5 = 5, local6 = 6, local7 = 7, local8 = 8;
    volatile int result = 0;
    
#ifdef __GNUC__
    /* Create artificial register conflicts */
    conflict_var1 = local1;
    dummy_helper1(conflict_var1);
    
    conflict_var2 = local2;
    dummy_helper2(conflict_var2);
    
    /* Force conflict_var1 to be live across helper calls */
    local3 = conflict_var1 + conflict_var2;
#endif
    
    for (int i = 0; i < iterations; i++) {
        /* Use many locals in complex patterns */
        local1 = local2 * local3;
        local2 = local4 ^ local5;
        local3 = local6 | local7;
        local4 = local8 << (i % 8);
        local5 = local1 >> (i % 4);
        local6 = local2 + local3;
        local7 = local4 - local5;
        local8 = local6 * local7;
        
        /* Call helpers to split live ranges */
        if (i % 3 == 0) dummy_helper1(local1);
        if (i % 5 == 0) dummy_helper2(local2);
        if (i % 7 == 0) dummy_helper3(local3);
        
        /* Re-assign conflicting register variable */
#ifdef __GNUC__
        if (i % 11 == 0) {
            conflict_var1 = local4;
            conflict_var2 = local5;
            local6 = conflict_var1 + conflict_var2;
        }
#endif
        
        /* Force all locals live */
        FORCE_USE(local1); FORCE_USE(local2); FORCE_USE(local3); FORCE_USE(local4);
        FORCE_USE(local5); FORCE_USE(local6); FORCE_USE(local7); FORCE_USE(local8);
        
        result += local1 + local2 + local3 + local4 + 
                  local5 + local6 + local7 + local8;
    }
    
    return result;
}

/* ========== MAIN DRIVER ========== */
int main(int argc, char *argv[]) {
    volatile int total_result = 0;
    int i;
    
    if (argc > 1 && strcmp(argv[1], "-v") == 0) {
        verbose = 1;
    }
    
    /* Use profile feedback to guide optimization */
    for (i = 0; i < 1000; i++) {
        /* Pattern A - targets ENTRY/EXIT blocks */
        total_result += pattern_a_entry_exit(100 + (i % 50), i);
        
        /* Pattern B - targets NEW_EXIT/NEW_ENTRY nodes */
        if (i % 10 == 0) {
            total_result += pattern_b_new_nodes(50 + (i % 30));
        }
        
        /* Pattern C - mixed scalar/vector pressure */
        total_result += pattern_c_mixed_pressure(i % 35, 20 + (i % 15));
        
        /* Pattern D - artificial register conflicts */
        total_result += pattern_d_artificial_conflict(30 + (i % 20));
    }
    
    /* Engage target-specific heuristics */
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        if (verbose) printf("AVX2 supported - engaging vector heuristics\n");
        total_result += 1;
    }
#endif
    
    if (verbose) {
        printf("Total result: %d\n", total_result);
        printf("Jump counter: %d\n", jump_counter);
    }
    
    /* Prevent optimization of final result */
    FORCE_USE(total_result);
    
    return total_result != 0 ? 0 : 1;
}
