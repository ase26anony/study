/* test_mcf.c - Comprehensive test for GCC's Min-Cost Flow pass special nodes */
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
static volatile int sink = 0;

/* ========== PATTERN A: ENTRY/EXIT BLOCKS ========== */
HOT NOINLINE
int pattern_a_entry_exit(int iterations, int seed) {
    /* Complex irreducible CFG to force ENTRY_BLOCK and EXIT_BLOCK special nodes */
    int result = seed;
    int i = 0;
    
    /* Create irreducible region using goto */
    if (iterations > 100) goto middle;
    
start:
    for (i = 0; i < iterations; i++) {
        int a = i * 2;
        int b = i * 3;
        int c = i * 5;
        
        if (i % 3 == 0) {
            result += a * b;
            if (result > 1000) goto middle;
        } else if (i % 3 == 1) {
            result -= c * a;
            if (result < -100) goto end;
        } else {
            result ^= b * c;
            if ((i & 7) == 0) goto start;
        }
        
        /* Deep if-else chain */
        if (i & 1) {
            if (i & 2) {
                if (i & 4) {
                    if (i & 8) {
                        result += 111;
                    } else {
                        result -= 222;
                    }
                } else {
                    result ^= 333;
                }
            } else {
                if (i & 16) {
                    result |= 444;
                } else {
                    result &= 555;
                }
            }
        }
        
        /* Multiple continue targets */
        if (i % 13 == 0) continue;
        if (i % 17 == 0) continue;
        
        /* Force all variables live */
        FORCE_USE(a);
        FORCE_USE(b);
        FORCE_USE(c);
    }
    
middle:
    /* Another irreducible part */
    for (int j = 0; j < iterations/2; j++) {
        if (j % 2 == 0) goto start;
        result += j * j;
        if (j % 5 == 0) goto end;
    }
    
end:
    return result;
}

/* ========== PATTERN B: NEW_EXIT/NEW_ENTRY with setjmp ========== */
static jmp_buf env;
static int setjmp_counter = 0;

NOINLINE
void dummy_helper(int x) {
    FORCE_USE(x);
}

int pattern_b_new_nodes(int depth, int max_depth) {
    int r0 = 1, r1 = 2, r2 = 3, r3 = 4, r4 = 5;
    int r5 = 6, r6 = 7, r7 = 8, r8 = 9, r9 = 10;
    int r10 = 11, r11 = 12, r12 = 13, r13 = 14, r14 = 15, r15 = 16;
    
    if (setjmp(env) == 0) {
        /* First time through */
        for (int i = 0; i < depth; i++) {
            /* Complex arithmetic on all registers */
            r0 = r0 * r1 + r2;
            r1 = r1 * r3 - r4;
            r2 = r2 * r5 ^ r6;
            r3 = r3 * r7 | r8;
            r4 = r4 * r9 & r10;
            r5 = r5 * r11 + r12;
            r6 = r6 * r13 - r14;
            r7 = r7 * r15 ^ r0;
            r8 = r8 * r1 | r2;
            r9 = r9 * r3 & r4;
            r10 = r10 * r5 + r6;
            r11 = r11 * r7 - r8;
            r12 = r12 * r9 ^ r10;
            r13 = r13 * r11 | r12;
            r14 = r14 * r13 & r14;
            r15 = r15 * r15 + r0;
            
            /* Force all live */
            FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
            FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
            FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
            FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
            
            if (i == depth/2 && depth < max_depth) {
                dummy_helper(i);
                longjmp(env, 1);
            }
        }
    } else {
        /* After longjmp */
        setjmp_counter++;
        r0 = r0 ^ 0xAAAA;
        r1 = r1 ^ 0xBBBB;
    }
    
    return r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + 
           r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
}

/* ========== PATTERN C: MIXED PRESSURE with vector ops ========== */
typedef int v4si VECTOR_TYPE;

NOINLINE
int pattern_c_mixed_pressure(int selector, int iterations) {
    v4si vec0 = {1, 2, 3, 4};
    v4si vec1 = {5, 6, 7, 8};
    v4si vec2 = {9, 10, 11, 12};
    v4si vec3 = {13, 14, 15, 16};
    
    int r0 = 0, r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0, r7 = 0;
    int r8 = 0, r9 = 0, r10 = 0, r11 = 0, r12 = 0, r13 = 0, r14 = 0, r15 = 0;
    
    /* Large switch with vector and scalar operations */
    for (int i = 0; i < iterations; i++) {
        switch ((selector + i) % 35) {
            case 0:
                vec0 = vec0 + vec1;
                r0 = r0 * 2 + 1;
                break;
            case 1:
                vec1 = vec1 - vec2;
                r1 = r1 * 3 - 2;
                break;
            case 2:
                vec2 = vec2 * vec3;
                r2 = r2 ^ 0x1234;
                break;
            case 3:
                vec3 = vec3 & vec0;
                r3 = r3 | 0xABCD;
                break;
            case 4:
                vec0 = vec1 + vec2;
                r4 = r4 + r0;
                break;
            case 5:
                vec1 = vec2 - vec3;
                r5 = r5 - r1;
                break;
            case 6:
                vec2 = vec3 * vec0;
                r6 = r6 * r2;
                break;
            case 7:
                vec3 = vec0 & vec1;
                r7 = r7 & r3;
                break;
            case 8:
                vec0 = vec0 << 1;
                r8 = r8 << 2;
                break;
            case 9:
                vec1 = vec1 >> 1;
                r9 = r9 >> 2;
                break;
            case 10:
                vec2 = vec2 + vec2;
                r10 = r10 + r10;
                break;
            case 11:
                vec3 = vec3 - vec3;
                r11 = r11 - r11;
                break;
            case 12:
                vec0 = vec0 * 3;
                r12 = r12 * 3;
                break;
            case 13:
                vec1 = vec1 / 2;
                r13 = r13 / 2;
                break;
            case 14:
                vec2 = vec2 ^ vec0;
                r14 = r14 ^ r4;
                break;
            case 15:
                vec3 = vec3 | vec1;
                r15 = r15 | r5;
                break;
            case 16:
                vec0 = vec0 & 0xFF;
                r0 = r0 & 0xFF;
                break;
            case 17:
                vec1 = vec1 | 0xFF00;
                r1 = r1 | 0xFF00;
                break;
            case 18:
                vec2 = vec2 ^ 0xFFFF;
                r2 = r2 ^ 0xFFFF;
                break;
            case 19:
                vec3 = vec3 + 1;
                r3 = r3 + 1;
                break;
            case 20:
                vec0 = vec0 - 1;
                r4 = r4 - 1;
                break;
            case 21:
                vec1 = vec1 * 2;
                r5 = r5 * 2;
                break;
            case 22:
                vec2 = vec2 / 2;
                r6 = r6 / 2;
                break;
            case 23:
                vec3 = vec3 << 3;
                r7 = r7 << 3;
                break;
            case 24:
                vec0 = vec0 >> 3;
                r8 = r8 >> 3;
                break;
            case 25:
                vec1 = vec1 + vec3;
                r9 = r9 + r7;
                break;
            case 26:
                vec2 = vec2 - vec0;
                r10 = r10 - r8;
                break;
            case 27:
                vec3 = vec3 * vec1;
                r11 = r11 * r9;
                break;
            case 28:
                vec0 = vec0 & vec2;
                r12 = r12 & r10;
                break;
            case 29:
                vec1 = vec1 | vec3;
                r13 = r13 | r11;
                break;
            case 30:
                vec2 = vec2 ^ vec0;
                r14 = r14 ^ r12;
                break;
            case 31:
                vec3 = vec3 + 100;
                r15 = r15 + 100;
                break;
            case 32:
                vec0 = vec0 - 100;
                r0 = r0 - 100;
                break;
            case 33:
                vec1 = vec1 * 7;
                r1 = r1 * 7;
                break;
            case 34:
                vec2 = vec2 / 7;
                r2 = r2 / 7;
                break;
        }
        
        /* Force all variables live */
        FORCE_USE(vec0); FORCE_USE(vec1); FORCE_USE(vec2); FORCE_USE(vec3);
        FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
        FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
        FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
        FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
        
        /* Complex loop control */
        if (i % 7 == 0) continue;
        if (i % 11 == 0) break;
        if (i % 13 == 0) i += 2;
    }
    
    /* Extract results from vectors */
    int v[4];
    memcpy(v, &vec0, sizeof(vec0));
    return v[0] + v[1] + v[2] + v[3] + r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
}

/* ========== PATTERN D: ARTIFICIAL CONFLICT ========== */
NOINLINE void dummy1(register int a asm ("r10"), register int b asm ("r11")) {
    FORCE_USE(a);
    FORCE_USE(b);
}

NOINLINE void dummy2(register int c asm ("r10"), register int d asm ("r12")) {
    FORCE_USE(c);
    FORCE_USE(d);
}

NOINLINE void dummy3(register int e asm ("r11"), register int f asm ("r13")) {
    FORCE_USE(e);
    FORCE_USE(f);
}

int pattern_d_register_conflict(int x, int y) {
    register int a asm ("r10") = x;
    register int b asm ("r11") = y;
    register int c asm ("r12") = x * 2;
    register int d asm ("r13") = y * 3;
    register int e asm ("r10") = x + y;  /* Conflict with a */
    register int f asm ("r11") = x - y;  /* Conflict with b */
    
    int result = 0;
    
    /* Force register conflicts */
    for (int i = 0; i < 100; i++) {
        if (i % 4 == 0) {
            dummy1(a, b);
            a = a * 3 + i;
            b = b * 5 - i;
        } else if (i % 4 == 1) {
            dummy2(c, d);
            c = c ^ 0x1234;
            d = d | 0xABCD;
            e = e + c;  /* Using conflicted register */
        } else if (i % 4 == 2) {
            dummy3(e, f);
            f = f * 7;
            a = a + f;  /* Using conflicted register */
        } else {
            /* All registers used together */
            result += a * b + c * d + e * f;
            a = a ^ b;
            b = b ^ c;
            c = c ^ d;
            d = d ^ e;
            e = e ^ f;
            f = f ^ a;
        }
        
        FORCE_USE(a);
        FORCE_USE(b);
        FORCE_USE(c);
        FORCE_USE(d);
        FORCE_USE(e);
        FORCE_USE(f);
    }
    
    return result + a + b + c + d + e + f;
}

/* ========== MAIN FUNCTION WITH PGO SUPPORT ========== */
int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "-v") == 0) {
        verbose = 1;
    }
    
    int total = 0;
    int iterations = 1000;
    
    /* Use CPU features to engage target-specific allocation */
    int use_avx2 = 0;
#ifdef __GNUC__
    use_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 support: %d\n", use_avx2);
    }
#endif
    
    /* Profile-guided loop - run each pattern multiple times */
    for (int i = 0; i < iterations; i++) {
        /* Pattern A - ENTRY/EXIT blocks */
        int r1 = pattern_a_entry_exit(i % 100 + 50, i);
        sink += r1;
        
        /* Pattern B - NEW_EXIT/NEW_ENTRY with setjmp */
        int r2 = pattern_b_new_nodes(i % 20 + 5, 30);
        sink += r2;
        
        /* Pattern C - Mixed pressure */
        int r3 = pattern_c_mixed_pressure(i, 50);
        sink += r3;
        
        /* Pattern D - Register conflict */
        int r4 = pattern_d_register_conflict(i, i * 2);
        sink += r4;
        
        total += r1 + r2 + r3 + r4;
        
        if (verbose && i % 100 == 0) {
            printf("Iteration %d: total=%d, sink=%d\n", i, total, sink);
        }
    }
    
    if (verbose) {
        printf("Final result: total=%d, sink=%d, setjmp_count=%d\n", 
               total, sink, setjmp_counter);
    }
    
    return total != 0 ? 0 : 1;
}
