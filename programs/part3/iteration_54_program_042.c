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

/* Pattern A: Function targeting ENTRY/EXIT blocks with irreducible region */
HOT NOINLINE
int pattern_a_entry_exit(int iterations, int seed) {
    volatile int result = seed;
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15;
    
    /* Create an irreducible region using goto */
    if (iterations > 0) {
        goto middle_of_loop;
    }
    
start_loop:
    for (int i = 0; i < iterations; i++) {
        /* Deep if-else chain creating many basic blocks */
        if (result & 0x1) {
            r0 = result * 3 + 1;
            r1 = r0 ^ 0x55555555;
            r2 = r1 << 3;
            r3 = r2 | 0x0F0F0F0F;
            FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
            result = r3;
        } else if (result & 0x2) {
            r4 = result / 2;
            r5 = r4 ^ 0xAAAAAAAA;
            r6 = r5 >> 1;
            r7 = r6 & 0x7F7F7F7F;
            FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
            result = r7;
        } else if (result & 0x4) {
            r8 = result + 0x12345678;
            r9 = r8 * 7;
            r10 = r9 - 0x87654321;
            r11 = r10 ^ result;
            FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
            result = r11;
        } else {
            r12 = result | 0xDEADBEEF;
            r13 = r12 & 0xC0FEC0FE;
            r14 = r13 << 5;
            r15 = r14 >> 2;
            FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
            result = r15;
        }
        
        /* Nested switch with continue to different labels */
        switch (result % 8) {
            case 0: goto start_loop;
            case 1: continue;
            case 2: goto middle_of_loop;
            case 3: break;
            case 4: result += i; continue;
            case 5: result -= i; goto start_loop;
            case 6: result ^= i; break;
            case 7: result |= i; goto middle_of_loop;
        }
        
middle_of_loop:
        /* More arithmetic to prevent optimization */
        r0 = result * 11;
        r1 = r0 + 0xCAFEBABE;
        r2 = r1 ^ 0xFEEDFACE;
        FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2);
        result = r2;
        
        /* Another irreducible jump */
        if (i % 3 == 0) {
            goto start_loop;
        }
    }
    
    return result;
}

/* Pattern B: Function using setjmp/longjmp for exceptional edges */
NOINLINE
int pattern_b_new_indices(int depth, int max_depth) {
    static jmp_buf env;
    volatile int result = 0;
    int a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15;
    
    if (setjmp(env) == 0) {
        /* Create many live variables */
        a0 = depth * 2;
        a1 = a0 + 0x11111111;
        a2 = a1 ^ 0x22222222;
        a3 = a2 * 3;
        a4 = a3 | 0x33333333;
        a5 = a4 & 0x44444444;
        a6 = a5 << 2;
        a7 = a6 >> 1;
        a8 = a7 + 0x55555555;
        a9 = a8 * 5;
        a10 = a9 - 0x66666666;
        a11 = a10 ^ 0x77777777;
        a12 = a11 | 0x88888888;
        a13 = a12 & 0x99999999;
        a14 = a13 << 3;
        a15 = a14 >> 2;
        
        FORCE_USE(a0); FORCE_USE(a1); FORCE_USE(a2); FORCE_USE(a3);
        FORCE_USE(a4); FORCE_USE(a5); FORCE_USE(a6); FORCE_USE(a7);
        FORCE_USE(a8); FORCE_USE(a9); FORCE_USE(a10); FORCE_USE(a11);
        FORCE_USE(a12); FORCE_USE(a13); FORCE_USE(a14); FORCE_USE(a15);
        
        result = a15;
        
        /* Recursive call that may longjmp */
        if (depth < max_depth) {
            result += pattern_b_new_indices(depth + 1, max_depth);
        } else {
            /* Trigger longjmp to create exceptional edge */
            longjmp(env, 1);
        }
    } else {
        /* Return path after longjmp */
        result = 0xABCDEF;
    }
    
    return result;
}

/* Pattern C: Mixed integer/vector pressure with large switch */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE
int pattern_c_mixed_pressure(int mode, int iterations) {
    volatile int result = 0;
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15;
    
#ifdef __GNUC__
    v4si v0, v1, v2, v3, v4, v5;
#endif
    
    for (int i = 0; i < iterations; i++) {
        /* Vector operations */
#ifdef __GNUC__
        v0 = (v4si){i, i+1, i+2, i+3};
        v1 = (v4si){i*2, i*3, i*4, i*5};
        v2 = v0 + v1;
        v3 = v0 * v1;
        v4 = v2 ^ v3;
        v5 = v4 << 1;
        
        /* Force vector variables live */
        asm volatile("" : : "x"(v0), "x"(v1), "x"(v2), "x"(v3), "x"(v4), "x"(v5));
#endif
        
        /* Large switch with 30+ cases */
        switch ((mode + i) % 35) {
            case 0:
                r0 = i * 11; r1 = r0 + 1; r2 = r1 ^ 2; r3 = r2 << 3;
                FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
                result += r3;
                continue;
            case 1:
                r4 = i * 13; r5 = r4 - 1; r6 = r5 | 4; r7 = r6 >> 2;
                FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
                result += r7;
                break;
            case 2:
                r8 = i * 17; r9 = r8 + 3; r10 = r9 ^ 8; r11 = r10 << 1;
                FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
                result += r11;
                continue;
            case 3:
                r12 = i * 19; r13 = r12 - 5; r14 = r13 | 16; r15 = r14 >> 3;
                FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
                result += r15;
                break;
            case 4:
                r0 = i * 23; r1 = r0 + 7; r2 = r1 ^ 32; r3 = r2 << 4;
                FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
                result += r3;
                continue;
            case 5:
                r4 = i * 29; r5 = r4 - 9; r6 = r5 | 64; r7 = r6 >> 5;
                FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
                result += r7;
                break;
            /* 30 more similar cases... */
            case 6: case 7: case 8: case 9: case 10:
            case 11: case 12: case 13: case 14: case 15:
            case 16: case 17: case 18: case 19: case 20:
            case 21: case 22: case 23: case 24: case 25:
            case 26: case 27: case 28: case 29: case 30:
            case 31: case 32: case 33: case 34:
                /* Generic handling for remaining cases */
                r0 = i * ((mode + i) % 35);
                r1 = r0 + i;
                r2 = r1 ^ mode;
                r3 = r2 << (i % 8);
                r4 = r3 >> ((mode + 1) % 8);
                r5 = r4 | 0xF0F0F0F0;
                r6 = r5 & 0x0F0F0F0F;
                r7 = r6 * 3;
                r8 = r7 + 0x12345678;
                r9 = r8 ^ 0x87654321;
                r10 = r9 << 2;
                r11 = r10 >> 1;
                r12 = r11 | r0;
                r13 = r12 & r1;
                r14 = r13 ^ r2;
                r15 = r14 + r3;
                FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
                FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
                FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
                FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
                result += r15;
                if ((i % 7) == 0) continue;
                break;
        }
        
        /* Additional arithmetic between iterations */
        r0 = result * 2;
        r1 = r0 + 0xCAFE;
        r2 = r1 ^ 0xBABE;
        FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2);
        result = r2;
    }
    
    return result;
}

/* Pattern D: Artificial register conflicts */
NOINLINE
void dummy_helper1(register int x asm("r10"), register int y asm("r11")) {
    FORCE_USE(x); FORCE_USE(y);
}

NOINLINE  
void dummy_helper2(register int a asm("r10"), register int b asm("r12")) {
    FORCE_USE(a); FORCE_USE(b);
}

NOINLINE
int pattern_d_register_conflict(int n) {
    /* Multiple variables trying to use the same register */
    register int x asm("r10") = n;
    register int y asm("r10") = n * 2;  /* Conflict! */
    register int z asm("r11") = n * 3;
    register int w asm("r11") = n * 4;  /* Another conflict! */
    
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15;
    
    /* Force register pressure */
    r0 = x + y;
    r1 = z * w;
    r2 = r0 ^ r1;
    r3 = r2 << 3;
    r4 = r3 | 0xFF;
    r5 = r4 & 0xAA;
    r6 = r5 + x;
    r7 = r6 - y;
    r8 = r7 * z;
    r9 = r8 / (w ? w : 1);
    r10 = r9 ^ 0x55555555;
    r11 = r10 << 1;
    r12 = r11 >> 2;
    r13 = r12 | 0x33333333;
    r14 = r13 & 0xCCCCCCCC;
    r15 = r14 + 0x12345678;
    
    FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
    FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
    FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
    FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
    
    /* Call helpers that also want the same registers */
    dummy_helper1(x, z);
    dummy_helper2(y, w);
    
    return r15;
}

/* Main driver with PGO support */
int main(int argc, char *argv[]) {
    volatile int total = 0;
    int iterations = 100;
    
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
    
    /* Use CPU features to engage target-specific allocation */
    int use_avx2 = 0;
#ifdef __GNUC__
    use_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 support: %s\n", use_avx2 ? "yes" : "no");
    }
#endif
    
    /* Call all pattern functions multiple times */
    for (int i = 0; i < iterations; i++) {
        /* Pattern A - targeting ENTRY/EXIT blocks */
        total += pattern_a_entry_exit(i % 10 + 1, i);
        
        /* Pattern B - targeting NEW_EXIT/NEW_ENTRY indices */
        total += pattern_b_new_indices(0, i % 5 + 1);
        
        /* Pattern C - mixed pressure */
        total += pattern_c_mixed_pressure(i % 8, i % 20 + 5);
        
        /* Pattern D - register conflicts */
        total += pattern_d_register_conflict(i);
        
        /* Add some branching based on results */
        if (total > 1000000) {
            total = total % 1000000;
        }
    }
    
    if (verbose) {
        printf("Final result: %d\n", total);
    }
    
    /* Prevent optimization of total */
    FORCE_USE(total);
    
    return total != 0 ? 0 : 1;
}
