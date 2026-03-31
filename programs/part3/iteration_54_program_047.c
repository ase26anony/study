/* test_mcf.c - Program to stress GCC's Min-Cost Flow pass and trigger special node indices */
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

/* ========== PATTERN A: ENTRY/EXIT BLOCKS ========== */
HOT NOINLINE
int pattern_a_entry_exit(int iterations, int seed) {
    volatile int result = seed;
    int i, j, k;
    
    /* Create complex control flow with irreducible region */
    if (seed > 0) {
        goto start_loop;
    } else {
        goto setup_vars;
    }
    
setup_vars:
    {
        int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
        FORCE_USE(a); FORCE_USE(b); FORCE_USE(c); FORCE_USE(d);
        FORCE_USE(e); FORCE_USE(f); FORCE_USE(g); FORCE_USE(h);
        goto start_loop;
    }
    
start_loop:
    for (i = 0; i < iterations; i++) {
        /* Deeply nested if-else chain */
        if (i & 1) {
            if (i & 2) {
                if (i & 4) {
                    if (i & 8) {
                        result += i * 3;
                        goto inner_loop_1;
                    } else {
                        result -= i * 2;
                        goto inner_loop_2;
                    }
                } else {
                    result ^= i;
                    continue;
                }
            } else {
                result |= 0xABCD;
                goto switch_in_loop;
            }
        } else {
            result &= 0xFFFF;
            for (j = 0; j < 5; j++) {
                for (k = 0; k < 3; k++) {
                    result += j * k;
                    if (result > 10000) {
                        goto reduce_result;
                    }
                }
            }
            continue;
        }
        
    inner_loop_1:
        for (j = 0; j < 10; j++) {
            result += (j << 2);
        }
        continue;
        
    inner_loop_2:
        for (j = 0; j < 7; j++) {
            result -= (j << 1);
        }
        continue;
        
    reduce_result:
        result >>= 1;
        continue;
        
    switch_in_loop:
        /* Small switch to add more complexity */
        switch (i & 3) {
            case 0: result += 111; break;
            case 1: result += 222; break;
            case 2: result += 333; break;
            case 3: result += 444; break;
        }
        continue;
    }
    
    return result & 0x7FFF;
}

/* ========== PATTERN B: NEW_EXIT/NEW_ENTRY with setjmp ========== */
static jmp_buf jump_buffer;
static volatile int jump_counter = 0;

NOINLINE
static void helper_b1(int *ptr) {
    *ptr += 100;
    if (jump_counter++ > 50) {
        longjmp(jump_buffer, 1);
    }
}

NOINLINE
static void helper_b2(int *ptr) {
    *ptr *= 2;
    if (jump_counter++ > 100) {
        longjmp(jump_buffer, 2);
    }
}

NOINLINE
int pattern_b_new_indices(int iterations) {
    int r0 = 1, r1 = 2, r2 = 3, r3 = 4, r4 = 5, r5 = 6, r6 = 7, r7 = 8;
    int r8 = 9, r9 = 10, r10 = 11, r11 = 12, r12 = 13, r13 = 14, r14 = 15, r15 = 16;
    int result = 0;
    
    if (setjmp(jump_buffer) == 0) {
        for (int i = 0; i < iterations; i++) {
            /* Many live variables across loop iterations */
            r0 += i; r1 -= i; r2 ^= i; r3 |= i;
            r4 <<= (i & 3); r5 >>= (i & 3);
            r6 = r6 * 3 + i; r7 = r7 / 2 + i;
            r8 = r8 ^ r0; r9 = r9 | r1;
            r10 = r10 & r2; r11 = r11 + r3;
            r12 = r12 - r4; r13 = r13 * r5;
            r14 = r14 ^ r6; r15 = r15 | r7;
            
            result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 +
                     r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
            
            /* Non-inlineable calls that may longjmp */
            if (i & 1) helper_b1(&result);
            if (i & 2) helper_b2(&result);
            
            FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
            FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
            FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
            FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
        }
    } else {
        /* longjmp target - exceptional control flow */
        result = (result & 0xFFFF) | 0x10000;
    }
    
    return result;
}

/* ========== PATTERN C: MIXED PRESSURE with vector ops ========== */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE
int pattern_c_mixed_pressure(int iterations, int selector) {
    int result = 0;
    
#ifdef __GNUC__
    v4si vec0 = {1, 2, 3, 4};
    v4si vec1 = {5, 6, 7, 8};
    v4si vec2 = {9, 10, 11, 12};
    v4si vec3 = {13, 14, 15, 16};
#endif
    
    int r0 = 1, r1 = 2, r2 = 3, r3 = 4, r4 = 5, r5 = 6, r6 = 7, r7 = 8;
    int r8 = 9, r9 = 10, r10 = 11, r11 = 12, r12 = 13, r13 = 14, r14 = 15, r15 = 16;
    
    for (int i = 0; i < iterations; i++) {
        /* Large switch with 30+ cases */
        switch ((selector + i) % 35) {
            case 0:
                r0 += i; r1 -= i; result += r0 * r1;
#ifdef __GNUC__
                vec0 += vec1;
                result += vec0[0];
#endif
                FORCE_USE(r0); FORCE_USE(r1);
                break;
            case 1:
                r2 ^= i; r3 |= i; result += r2 & r3;
#ifdef __GNUC__
                vec1 *= vec2;
                result += vec1[1];
#endif
                FORCE_USE(r2); FORCE_USE(r3);
                break;
            case 2:
                r4 <<= 1; r5 >>= 1; result += r4 | r5;
                FORCE_USE(r4); FORCE_USE(r5);
                break;
            case 3:
                r6 = r6 * 3 + i; result += r6;
                FORCE_USE(r6);
                break;
            case 4:
                r7 = r7 / 2 + i; result += r7;
                FORCE_USE(r7);
                break;
            case 5:
                r8 = r8 ^ r0; result += r8;
                FORCE_USE(r8);
                break;
            case 6:
                r9 = r9 | r1; result += r9;
                FORCE_USE(r9);
                break;
            case 7:
                r10 = r10 & r2; result += r10;
                FORCE_USE(r10);
                break;
            case 8:
                r11 = r11 + r3; result += r11;
                FORCE_USE(r11);
                break;
            case 9:
                r12 = r12 - r4; result += r12;
                FORCE_USE(r12);
                break;
            case 10:
                r13 = r13 * r5; result += r13;
                FORCE_USE(r13);
                break;
            case 11:
                r14 = r14 ^ r6; result += r14;
                FORCE_USE(r14);
                break;
            case 12:
                r15 = r15 | r7; result += r15;
                FORCE_USE(r15);
                break;
            case 13:
                r0 += r8; r1 += r9; result += r0 + r1;
                FORCE_USE(r0); FORCE_USE(r1);
                break;
            case 14:
                r2 += r10; r3 += r11; result += r2 + r3;
                FORCE_USE(r2); FORCE_USE(r3);
                break;
            case 15:
                r4 += r12; r5 += r13; result += r4 + r5;
                FORCE_USE(r4); FORCE_USE(r5);
                break;
            case 16:
                r6 += r14; r7 += r15; result += r6 + r7;
                FORCE_USE(r6); FORCE_USE(r7);
                break;
            case 17:
#ifdef __GNUC__
                vec0 += vec2;
                result += vec0[2];
#endif
                r8 += i * 2; result += r8;
                FORCE_USE(r8);
                break;
            case 18:
#ifdef __GNUC__
                vec1 += vec3;
                result += vec1[3];
#endif
                r9 += i * 3; result += r9;
                FORCE_USE(r9);
                break;
            case 19:
                r10 += i * 4; result += r10;
                FORCE_USE(r10);
                break;
            case 20:
                r11 += i * 5; result += r11;
                FORCE_USE(r11);
                break;
            case 21:
                r12 += i * 6; result += r12;
                FORCE_USE(r12);
                break;
            case 22:
                r13 += i * 7; result += r13;
                FORCE_USE(r13);
                break;
            case 23:
                r14 += i * 8; result += r14;
                FORCE_USE(r14);
                break;
            case 24:
                r15 += i * 9; result += r15;
                FORCE_USE(r15);
                break;
            case 25:
                r0 -= i; r1 -= i * 2; result += r0 - r1;
                FORCE_USE(r0); FORCE_USE(r1);
                break;
            case 26:
                r2 -= i * 3; r3 -= i * 4; result += r2 - r3;
                FORCE_USE(r2); FORCE_USE(r3);
                break;
            case 27:
                r4 -= i * 5; r5 -= i * 6; result += r4 - r5;
                FORCE_USE(r4); FORCE_USE(r5);
                break;
            case 28:
                r6 -= i * 7; r7 -= i * 8; result += r6 - r7;
                FORCE_USE(r6); FORCE_USE(r7);
                break;
            case 29:
                r8 -= i * 9; r9 -= i * 10; result += r8 - r9;
                FORCE_USE(r8); FORCE_USE(r9);
                break;
            case 30:
                r10 -= i * 11; r11 -= i * 12; result += r10 - r11;
                FORCE_USE(r10); FORCE_USE(r11);
                break;
            case 31:
                r12 -= i * 13; r13 -= i * 14; result += r12 - r13;
                FORCE_USE(r12); FORCE_USE(r13);
                break;
            case 32:
                r14 -= i * 15; r15 -= i * 16; result += r14 - r15;
                FORCE_USE(r14); FORCE_USE(r15);
                break;
            case 33:
#ifdef __GNUC__
                vec2 = vec0 + vec1;
                result += vec2[0] + vec2[1];
#endif
                r0 = r1 = i; result += r0 + r1;
                FORCE_USE(r0); FORCE_USE(r1);
                break;
            case 34:
#ifdef __GNUC__
                vec3 = vec0 * vec1;
                result += vec3[2] + vec3[3];
#endif
                r2 = r3 = i * 2; result += r2 + r3;
                FORCE_USE(r2); FORCE_USE(r3);
                break;
            default:
                result += i;
                break;
        }
        
        /* Complex loop control */
        if (result > 1000000) {
            result >>= 2;
            continue;
        } else if (result < 0) {
            result = -result;
            continue;
        } else if ((i & 15) == 0) {
            /* Jump to different switch cases */
            selector = (selector + 5) % 35;
            continue;
        }
    }
    
    return result;
}

/* ========== PATTERN D: ARTIFICIAL CONFLICT ========== */
#ifdef __GNUC__
register int reg_x asm ("r10");
register int reg_y asm ("r11");
#else
int reg_x, reg_y;
#endif

NOINLINE static void dummy1(int a, int b, int c) {
    FORCE_USE(a); FORCE_USE(b); FORCE_USE(c);
}

NOINLINE static void dummy2(int a, int b, int c, int d) {
    FORCE_USE(a); FORCE_USE(b); FORCE_USE(c); FORCE_USE(d);
}

NOINLINE static void dummy3(int a, int b, int c, int d, int e) {
    FORCE_USE(a); FORCE_USE(b); FORCE_USE(c); FORCE_USE(d); FORCE_USE(e);
}

NOINLINE
int pattern_d_artificial_conflict(int iterations) {
    int result = 0;
    
#ifdef __GNUC__
    /* Force use of specific registers */
    int local_x __asm__ ("r10");
    int local_y __asm__ ("r11");
    int local_z __asm__ ("r12");
    
    local_x = 1;
    local_y = 2;
    local_z = 3;
#else
    int local_x = 1, local_y = 2, local_z = 3;
#endif
    
    for (int i = 0; i < iterations; i++) {
        /* Artificial register pressure */
        reg_x = local_x + i;
        reg_y = local_y * i;
        
        /* Many non-inlineable calls with register arguments */
        dummy1(reg_x, reg_y, local_z);
        dummy2(reg_x, reg_y, local_z, i);
        dummy3(reg_x, reg_y, local_z, i, result);
        
        /* Complex arithmetic spreading values across registers */
        local_x = reg_x ^ reg_y;
        local_y = reg_x | local_z;
        local_z = reg_y & i;
        
        result += local_x + local_y + local_z + reg_x + reg_y;
        
        /* Switch to create multiple paths */
        switch (i & 7) {
            case 0: local_x += 1; break;
            case 1: local_y += 2; break;
            case 2: local_z += 3; break;
            case 3: reg_x += 4; break;
            case 4: reg_y += 5; break;
            case 5: local_x -= 1; break;
            case 6: local_y -= 2; break;
            case 7: local_z -= 3; break;
        }
        
        FORCE_USE(local_x); FORCE_USE(local_y); FORCE_USE(local_z);
        FORCE_USE(reg_x); FORCE_USE(reg_y);
    }
    
    return result;
}

/* ========== MAIN FUNCTION with PGO support ========== */
int main(int argc, char *argv[]) {
    volatile int total_result = 0;
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
    
    /* Check CPU features to engage target-specific register allocation */
    int has_avx2 = 0;
#ifdef __GNUC__
    has_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 support: %s\n", has_avx2 ? "yes" : "no");
    }
#endif
    
    /* Call each pattern multiple times with different arguments */
    for (i = 0; i < 5; i++) {
        if (verbose && i % 10 == 0) {
            printf("Iteration %d\n", i);
        }
        
        /* Pattern A - targeting ENTRY/EXIT blocks */
        total_result += pattern_a_entry_exit(iterations / (i + 1), i * 12345);
        
        /* Pattern B - targeting NEW_EXIT/NEW_ENTRY */
        jump_counter = 0;
        total_result += pattern_b_new_indices(iterations / ((i % 3) + 1));
        
        /* Pattern C - mixed pressure */
        total_result += pattern_c_mixed_pressure(iterations / ((i % 4) + 1), i * 54321);
        
        /* Pattern D - artificial conflict */
        total_result += pattern_d_artificial_conflict(iterations / ((i % 5) + 1));
        
        /* Prevent optimization */
        FORCE_USE(total_result);
    }
    
    if (verbose) {
        printf("Final result: %d\n", total_result);
    }
    
    return (total_result & 255) == 0 ? 0 : 1;
}
