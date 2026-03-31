/* test_mcf.c - Comprehensive test for GCC's Min-Cost Flow pass special nodes */
#ifdef __GNUC__
#define FORCE_USE(var) asm volatile("" : : "g"(var))
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define REG_VAR(name, reg) register int name asm(reg)
#else
#define FORCE_USE(var) (void)(var)
#define NOINLINE
#define HOT
#define REG_VAR(name, reg) int name
#endif

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

static int verbose = 0;

/* Pattern A: Function targeting ENTRY/EXIT blocks with irreducible region */
HOT NOINLINE
int pattern_a_entry_exit(int iterations, int seed) {
    int result = seed;
    int i, j, k;
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15;
    
    /* Create complex initial block that should become ENTRY_BLOCK */
    r0 = seed * 3;
    r1 = seed + 7919;
    r2 = seed ^ 0xDEADBEEF;
    r3 = seed % 997;
    
    /* Irreducible region using goto */
    if (iterations > 100) {
        goto middle_of_loop;
    }
    
    for (i = 0; i < iterations; i++) {
        r4 = i * i;
        r5 = i + r0;
        
        /* Deep if-else chain creating many basic blocks */
        if (i % 3 == 0) {
            r6 = r1 * r2;
            r7 = r3 ^ r4;
            if (i % 7 == 0) {
                r8 = r5 << 2;
                goto middle_of_loop;
            } else if (i % 11 == 0) {
                r9 = r6 >> 1;
                continue;
            }
        } else if (i % 5 == 0) {
            r10 = r2 + r3;
            r11 = r4 - r5;
            for (j = 0; j < 5; j++) {
                r12 = j * r10;
                r13 = r11 ^ j;
                if (j == 3) break;
            }
        } else {
            r14 = r0 * r1;
            r15 = r2 / (r3 + 1);
        }
        
middle_of_loop:
        /* More arithmetic to prevent merging */
        r0 = (r0 + 1) & 0xFFF;
        r1 = r1 * 1103515245 + 12345;
        r2 = r2 ^ r0;
        r3 = r3 + r1;
        
        /* Force all variables live */
        FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
        FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
        FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
        FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
        
        result += r0 + r1 + r2 + r3;
    }
    
    /* Complex exit region */
    for (k = 0; k < 10; k++) {
        result = (result << 3) | (result >> 29);
    }
    
    return result;
}

/* Pattern B: Function using setjmp/longjmp for exceptional edges */
NOINLINE
int pattern_b_new_indices(int depth, int max_depth) {
    static jmp_buf env;
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15;
    int result = 0;
    
    r0 = depth * 2;
    r1 = depth + 100;
    r2 = depth ^ 0xCAFEBABE;
    
    if (setjmp(env) == 0) {
        /* Normal path with many variables */
        for (int i = 0; i < 50; i++) {
            r3 = i * r0;
            r4 = i + r1;
            r5 = i ^ r2;
            
            /* Complex arithmetic creating register pressure */
            r6 = r3 * r4;
            r7 = r4 + r5;
            r8 = r5 ^ r3;
            r9 = r6 % 17;
            r10 = r7 << 1;
            r11 = r8 >> 2;
            r12 = r9 * 3;
            r13 = r10 + 11;
            r14 = r11 ^ 0x55;
            r15 = r12 % 19;
            
            if (i == 25 && depth < max_depth) {
                /* Recursive call creating more pressure */
                result += pattern_b_new_indices(depth + 1, max_depth);
            }
            
            /* Force all live */
            FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2);
            FORCE_USE(r3); FORCE_USE(r4); FORCE_USE(r5);
            FORCE_USE(r6); FORCE_USE(r7); FORCE_USE(r8);
            FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
            FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14);
            FORCE_USE(r15);
            
            result += r3 + r6 + r9 + r12 + r15;
        }
    } else {
        /* longjmp target - different register usage */
        r0 = r0 * 7;
        r1 = r1 + 999;
        r2 = r2 ^ 0x12345678;
        result = r0 + r1 + r2;
    }
    
    /* Trigger longjmp under certain conditions */
    if (depth > 0 && (result % 1000) == 777) {
        longjmp(env, 1);
    }
    
    return result;
}

/* Pattern C: Mixed integer/vector pressure with large switch */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE
int pattern_c_mixed_pressure(int selector, int iterations) {
    int result = 0;
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15;
    
#ifdef __GNUC__
    v4si v0 = {0, 1, 2, 3};
    v4si v1 = {4, 5, 6, 7};
    v4si v2 = {8, 9, 10, 11};
    v4si v3 = {12, 13, 14, 15};
#endif
    
    for (int i = 0; i < iterations; i++) {
        /* Large switch with 30+ cases */
        switch ((selector + i) % 35) {
            case 0:
                r0 = i * 2;
                r1 = i + 100;
#ifdef __GNUC__
                v0 = v0 + v1;
#endif
                break;
            case 1:
                r2 = i * 3;
                r3 = i + 200;
#ifdef __GNUC__
                v1 = v1 * v2;
#endif
                continue;  /* Back-edge to loop header */
            case 2:
                r4 = i * 5;
                r5 = i + 300;
#ifdef __GNUC__
                v2 = v2 - v3;
#endif
                break;
            case 3:
                r6 = i * 7;
                r7 = i + 400;
#ifdef __GNUC__
                v3 = v3 ^ v0;
#endif
                break;
            case 4:
                r8 = i * 11;
                r9 = i + 500;
#ifdef __GNUC__
                v0 = v0 << 1;
#endif
                break;
            case 5:
                r10 = i * 13;
                r11 = i + 600;
#ifdef __GNUC__
                v1 = v1 >> 1;
#endif
                continue;
            case 6:
                r12 = i * 17;
                r13 = i + 700;
#ifdef __GNUC__
                v2 = v2 + v0;
#endif
                break;
            case 7:
                r14 = i * 19;
                r15 = i + 800;
#ifdef __GNUC__
                v3 = v3 * v1;
#endif
                break;
            case 8:
                r0 = r0 ^ r1;
                r2 = r2 + r3;
#ifdef __GNUC__
                v0 = v0 - v2;
#endif
                break;
            case 9:
                r4 = r4 * r5;
                r6 = r6 % 17;
#ifdef __GNUC__
                v1 = v1 ^ v3;
#endif
                continue;
            case 10:
                r8 = r8 << 2;
                r10 = r10 >> 2;
#ifdef __GNUC__
                v2 = v2 + v1;
#endif
                break;
            case 11:
                r12 = r12 & 0xFF;
                r14 = r14 | 0x55;
#ifdef __GNUC__
                v3 = v3 * v0;
#endif
                break;
            case 12:
                r1 = r1 + r0;
                r3 = r3 - r2;
#ifdef __GNUC__
                v0 = v0 << v1;
#endif
                break;
            case 13:
                r5 = r5 * r4;
                r7 = r7 % 19;
#ifdef __GNUC__
                v1 = v1 >> v2;
#endif
                continue;
            case 14:
                r9 = r9 ^ r8;
                r11 = r11 + r10;
#ifdef __GNUC__
                v2 = v2 - v3;
#endif
                break;
            case 15:
                r13 = r13 & r12;
                r15 = r15 | r14;
#ifdef __GNUC__
                v3 = v3 ^ v0;
#endif
                break;
            case 16:
                r0 = r0 * 3;
                r2 = r2 + 5;
#ifdef __GNUC__
                v0 = v0 + 1;
#endif
                break;
            case 17:
                r4 = r4 % 23;
                r6 = r6 ^ 0xAA;
#ifdef __GNUC__
                v1 = v1 - 2;
#endif
                continue;
            case 18:
                r8 = r8 << 3;
                r10 = r10 >> 3;
#ifdef __GNUC__
                v2 = v2 * 3;
#endif
                break;
            case 19:
                r12 = r12 + r13;
                r14 = r14 - r15;
#ifdef __GNUC__
                v3 = v3 ^ 0xFF;
#endif
                break;
            case 20:
                r1 = r1 * 7;
                r3 = r3 % 29;
#ifdef __GNUC__
                v0 = v0 + v3;
#endif
                break;
            case 21:
                r5 = r5 ^ r6;
                r7 = r7 + r8;
#ifdef __GNUC__
                v1 = v1 * v0;
#endif
                continue;
            case 22:
                r9 = r9 % 31;
                r11 = r11 & 0xCC;
#ifdef __GNUC__
                v2 = v2 - v1;
#endif
                break;
            case 23:
                r13 = r13 << 1;
                r15 = r15 >> 1;
#ifdef __GNUC__
                v3 = v3 + v2;
#endif
                break;
            case 24:
                r0 = r0 + r2;
                r4 = r4 ^ r6;
#ifdef __GNUC__
                v0 = v0 * 5;
#endif
                break;
            case 25:
                r8 = r8 % 37;
                r12 = r12 + r14;
#ifdef __GNUC__
                v1 = v1 - 3;
#endif
                continue;
            case 26:
                r1 = r1 & r3;
                r5 = r5 | r7;
#ifdef __GNUC__
                v2 = v2 ^ 0x33;
#endif
                break;
            case 27:
                r9 = r9 * 11;
                r13 = r13 % 41;
#ifdef __GNUC__
                v3 = v3 << 2;
#endif
                break;
            case 28:
                r2 = r2 + r4;
                r6 = r6 ^ r8;
#ifdef __GNUC__
                v0 = v0 >> 1;
#endif
                break;
            case 29:
                r10 = r10 % 43;
                r14 = r14 + r0;
#ifdef __GNUC__
                v1 = v1 + v3;
#endif
                continue;
            case 30:
                r3 = r3 & r5;
                r7 = r7 | r9;
#ifdef __GNUC__
                v2 = v2 * 7;
#endif
                break;
            case 31:
                r11 = r11 * 13;
                r15 = r15 % 47;
#ifdef __GNUC__
                v3 = v3 - v0;
#endif
                break;
            case 32:
                r0 = r0 ^ r4;
                r8 = r8 + r12;
#ifdef __GNUC__
                v0 = v0 & v1;
#endif
                break;
            case 33:
                r1 = r1 % 53;
                r9 = r9 ^ r13;
#ifdef __GNUC__
                v1 = v1 | v2;
#endif
                continue;
            case 34:
                r2 = r2 * 17;
                r10 = r10 + r14;
#ifdef __GNUC__
                v2 = v2 ^ v3;
#endif
                break;
        }
        
        /* Force all integer registers live */
        FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
        FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
        FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
        FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
        
#ifdef __GNUC__
        /* Force all vector registers live */
        FORCE_USE(v0); FORCE_USE(v1); FORCE_USE(v2); FORCE_USE(v3);
#endif
        
        result += r0 + r4 + r8 + r12;
    }
    
    return result;
}

/* Pattern D: Artificial register conflicts with explicit register variables */
NOINLINE
int pattern_d_register_conflict(int x, int y) {
    /* Explicit register variables creating conflicts */
    REG_VAR(reg_a, "r10");
    REG_VAR(reg_b, "r11");
    REG_VAR(reg_c, "r12");
    REG_VAR(reg_d, "r13");
    
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r14, r15;
    
    reg_a = x * 3;
    reg_b = y * 5;
    reg_c = x + y;
    reg_d = x ^ y;
    
    /* Dummy helper calls that won't be inlined */
    auto NOINLINE use_regs(int a, int b, int c, int d) -> void {
        FORCE_USE(a); FORCE_USE(b); FORCE_USE(c); FORCE_USE(d);
    };
    
    auto NOINLINE more_regs(int a, int b, int c, int d, int e, int f) -> void {
        FORCE_USE(a); FORCE_USE(b); FORCE_USE(c);
        FORCE_USE(d); FORCE_USE(e); FORCE_USE(f);
    };
    
    /* Complex loop with many register uses */
    for (int i = 0; i < 100; i++) {
        r0 = i * reg_a;
        r1 = i + reg_b;
        r2 = i ^ reg_c;
        r3 = i % (reg_d + 1);
        
        use_regs(reg_a, reg_b, reg_c, reg_d);
        
        r4 = r0 * r1;
        r5 = r1 + r2;
        r6 = r2 ^ r3;
        r7 = r3 % 17;
        
        more_regs(r0, r1, r2, r3, r4, r5);
        
        r8 = r4 << 2;
        r9 = r5 >> 1;
        r14 = r6 & 0xFF;
        r15 = r7 | 0x55;
        
        /* Reassign register variables creating live-range splits */
        if (i % 7 == 0) {
            reg_a = r8;
            reg_b = r9;
        } else if (i % 11 == 0) {
            reg_c = r14;
            reg_d = r15;
        } else {
            reg_a = reg_b + reg_c;
            reg_b = reg_c ^ reg_d;
            reg_c = reg_d * reg_a;
            reg_d = reg_a % 19;
        }
        
        /* Force everything live */
        FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
        FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
        FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r14); FORCE_USE(r15);
        FORCE_USE(reg_a); FORCE_USE(reg_b); FORCE_USE(reg_c); FORCE_USE(reg_d);
    }
    
    return reg_a + reg_b + reg_c + reg_d;
}

/* Main driver with profile-guided optimization setup */
int main(int argc, char *argv[]) {
    volatile int total_result = 0;  /* Prevent optimization */
    int i, iterations;
    
    if (argc > 1) {
        verbose = atoi(argv[1]);
    }
    
    iterations = 1000;
    if (argc > 2) {
        iterations = atoi(argv[2]);
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
    
    /* Call each pattern multiple times with different arguments */
    for (i = 0; i < iterations; i++) {
        int seed = i * 1103515245 + 12345;
        
        /* Pattern A - targeting ENTRY/EXIT blocks */
        total_result ^= pattern_a_entry_exit(i % 100 + 50, seed);
        
        /* Pattern B - targeting NEW_EXIT/NEW_ENTRY with setjmp */
        if (i % 10 == 0) {
            total_result += pattern_b_new_indices(0, 3);
        }
        
        /* Pattern C - mixed integer/vector pressure */
        total_result += pattern_c_mixed_pressure(i, 20);
        
        /* Pattern D - artificial register conflicts */
        total_result ^= pattern_d_register_conflict(i, i * 3);
        
        /* Occasionally print progress */
        if (verbose && (i % 100 == 0)) {
            printf("Iteration %d, result so far: %d\n", i, total_result);
        }
    }
    
    if (verbose) {
        printf("Final result: %d\n", total_result);
    }
    
    return total_result != 0 ? 0 : 1;
}
