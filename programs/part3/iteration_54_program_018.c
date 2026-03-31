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

/* Pattern A: Deeply nested if-else with irreducible region for ENTRY/EXIT blocks */
HOT NOINLINE int pattern_a_irreducible(int iterations, int seed) {
    volatile int result = seed;
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15;
    
    /* Create many local variables to increase register pressure */
    r0 = seed * 1; r1 = seed * 2; r2 = seed * 3; r3 = seed * 4;
    r4 = seed * 5; r5 = seed * 6; r6 = seed * 7; r7 = seed * 8;
    r8 = seed * 9; r9 = seed * 10; r10 = seed * 11; r11 = seed * 12;
    r12 = seed * 13; r13 = seed * 14; r14 = seed * 15; r15 = seed * 16;
    
    /* Labeled block to encourage ENTRY_BLOCK identification */
entry_label:
    if (iterations <= 0) goto exit_label;
    
    /* Complex irreducible region using goto */
    if (seed & 1) {
        r0 += r1;
        goto middle_label;
    } else {
        r2 += r3;
        goto middle_label;
    }
    
middle_label:
    /* Nested if-else chain creating many basic blocks */
    if (r0 > 100) {
        if (r1 < 50) {
            if (r2 == r3) {
                r4 = r5 * r6;
                goto loop_label;
            } else {
                r7 = r8 / (r9 + 1);
                goto loop_label;
            }
        } else {
            r10 = r11 ^ r12;
            if (r13 > r14) {
                r15 = r0 | r1;
                goto loop_label;
            }
        }
    } else {
        if (r4 < r5) {
            r6 = r7 & r8;
        } else {
            r9 = r10 << 2;
        }
    }
    
loop_label:
    /* Loop with continue to different case labels */
    for (int i = 0; i < iterations; i++) {
        if (i & 1) {
            r0 += r1;
            continue;
        } else if (i & 2) {
            r2 -= r3;
            continue;
        } else if (i & 4) {
            r4 *= r5;
            continue;
        }
        
        /* Deep nesting continues */
        switch (i % 8) {
            case 0: r6 = r7 + r8; break;
            case 1: r9 = r10 - r11; break;
            case 2: r12 = r13 * r14; break;
            case 3: r15 = r0 ^ r1; break;
            case 4: r2 = r3 | r4; break;
            case 5: r5 = r6 & r7; break;
            case 6: r8 = r9 << 1; break;
            case 7: r10 = r11 >> 1; break;
        }
        
        if (i == iterations - 1) {
            goto exit_label;
        }
    }
    
exit_label:
    /* Force all variables to be considered live */
    FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
    FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
    FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
    FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
    
    result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + 
             r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
    return result % 1000;
}

/* Pattern B: Large switch with setjmp/longjmp for NEW_EXIT/NEW_ENTRY */
NOINLINE int pattern_b_setjmp_switch(int mode, int count) {
    jmp_buf env;
    volatile int result = 0;
    int r0 = 1, r1 = 2, r2 = 3, r3 = 4, r4 = 5, r5 = 6, r6 = 7, r7 = 8;
    int r8 = 9, r9 = 10, r10 = 11, r11 = 12, r12 = 13, r13 = 14, r14 = 15, r15 = 16;
    
    if (setjmp(env) == 0) {
        /* Large switch statement with 30+ cases */
        for (int i = 0; i < count; i++) {
            switch ((mode + i) % 35) {
                case 0: r0 = r1 + r2; r3 = r4 * r5; break;
                case 1: r1 = r2 - r3; r4 = r5 / (r6 + 1); break;
                case 2: r2 = r3 ^ r4; r5 = r6 | r7; break;
                case 3: r3 = r4 & r5; r6 = r7 << 1; break;
                case 4: r4 = r5 >> 2; r7 = r8 + r9; break;
                case 5: r5 = r6 * r7; r8 = r9 - r10; break;
                case 6: r6 = r7 / (r8 + 1); r9 = r10 ^ r11; break;
                case 7: r7 = r8 | r9; r10 = r11 & r12; break;
                case 8: r8 = r9 << 3; r11 = r12 >> 1; break;
                case 9: r9 = r10 + r11; r12 = r13 * r14; break;
                case 10: r10 = r11 - r12; r13 = r14 / (r15 + 1); break;
                case 11: r11 = r12 ^ r13; r14 = r15 | r0; break;
                case 12: r12 = r13 & r14; r15 = r0 << 2; break;
                case 13: r13 = r14 >> 1; r0 = r1 + r2; break;
                case 14: r14 = r15 * r0; r1 = r2 - r3; break;
                case 15: r15 = r0 / (r1 + 1); r2 = r3 ^ r4; break;
                case 16: r0 = r1 | r2; r3 = r4 & r5; break;
                case 17: r1 = r2 << 1; r4 = r5 >> 2; break;
                case 18: r2 = r3 + r4; r5 = r6 * r7; break;
                case 19: r3 = r4 - r5; r6 = r7 / (r8 + 1); break;
                case 20: r4 = r5 ^ r6; r7 = r8 | r9; break;
                case 21: r5 = r6 & r7; r8 = r9 << 1; break;
                case 22: r6 = r7 >> 1; r9 = r10 + r11; break;
                case 23: r7 = r8 * r9; r10 = r11 - r12; break;
                case 24: r8 = r9 / (r10 + 1); r11 = r12 ^ r13; break;
                case 25: r9 = r10 | r11; r12 = r13 & r14; break;
                case 26: r10 = r11 << 2; r13 = r14 >> 1; break;
                case 27: r11 = r12 + r13; r14 = r15 * r0; break;
                case 28: r12 = r13 - r14; r15 = r0 / (r1 + 1); break;
                case 29: r13 = r14 ^ r15; r0 = r1 | r2; break;
                case 30: r14 = r15 & r0; r1 = r2 << 1; break;
                case 31: r15 = r0 >> 1; r2 = r3 + r4; break;
                case 32: r0 = r1 * r2; r3 = r4 - r5; break;
                case 33: r1 = r2 / (r3 + 1); r4 = r5 ^ r6; break;
                case 34: r2 = r3 | r4; r5 = r6 & r7; break;
            }
            
            /* Non-local jump to create exceptional edge */
            if (i == count / 2) {
                longjmp(env, 1);
            }
            
            /* Force register pressure */
            FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
            FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
        }
    } else {
        /* After longjmp */
        if (verbose) printf("longjmp executed\n");
    }
    
    result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + 
             r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
    return result % 1000;
}

/* Pattern C: Vector operations combined with large switch */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE int pattern_c_vector_switch(int iterations, int seed) {
    volatile int result = 0;
    int r0 = seed, r1 = seed+1, r2 = seed+2, r3 = seed+3;
    int r4 = seed+4, r5 = seed+5, r6 = seed+6, r7 = seed+7;
    int r8 = seed+8, r9 = seed+9, r10 = seed+10, r11 = seed+11;
    int r12 = seed+12, r13 = seed+13, r14 = seed+14, r15 = seed+15;
    
#ifdef __GNUC__
    /* Vector operations to pressure vector registers */
    v4si v0 = {r0, r1, r2, r3};
    v4si v1 = {r4, r5, r6, r7};
    v4si v2 = {r8, r9, r10, r11};
    v4si v3 = {r12, r13, r14, r15};
    
    for (int i = 0; i < iterations; i++) {
        /* Mix vector and scalar operations */
        switch (i % 32) {
            case 0: v0 = v0 + v1; r0 += r1; break;
            case 1: v1 = v1 - v2; r2 -= r3; break;
            case 2: v2 = v2 * v3; r4 *= r5; break;
            case 3: v3 = v0 ^ v1; r6 ^= r7; break;
            case 4: v0 = v1 | v2; r8 |= r9; break;
            case 5: v1 = v2 & v3; r10 &= r11; break;
            case 6: v2 = v3 << 1; r12 <<= 1; break;
            case 7: v3 = v0 >> 2; r13 >>= 2; break;
            case 8: v0 = v1 + v2; r14 += r15; break;
            case 9: v1 = v2 - v3; r0 -= r1; break;
            case 10: v2 = v3 * v0; r2 *= r3; break;
            case 11: v3 = v0 ^ v1; r4 ^= r5; break;
            case 12: v0 = v1 | v2; r6 |= r7; break;
            case 13: v1 = v2 & v3; r8 &= r9; break;
            case 14: v2 = v3 << 2; r10 <<= 2; break;
            case 15: v3 = v0 >> 1; r11 >>= 1; break;
            case 16: v0 = v1 + v3; r12 += r13; break;
            case 17: v1 = v2 - v0; r14 -= r15; break;
            case 18: v2 = v3 * v1; r0 *= r1; break;
            case 19: v3 = v0 ^ v2; r2 ^= r3; break;
            case 20: v0 = v1 | v3; r4 |= r5; break;
            case 21: v1 = v2 & v0; r6 &= r7; break;
            case 22: v2 = v3 << 3; r8 <<= 3; break;
            case 23: v3 = v0 >> 3; r9 >>= 3; break;
            case 24: v0 = v2 + v3; r10 += r11; break;
            case 25: v1 = v3 - v0; r12 -= r13; break;
            case 26: v2 = v0 * v1; r14 *= r15; break;
            case 27: v3 = v1 ^ v2; r0 ^= r1; break;
            case 28: v0 = v2 | v3; r2 |= r3; break;
            case 29: v1 = v3 & v0; r4 &= r5; break;
            case 30: v2 = v0 << 1; r6 <<= 1; break;
            case 31: v3 = v1 >> 1; r7 >>= 1; break;
        }
        
        /* Force all variables live */
        FORCE_USE(v0); FORCE_USE(v1); FORCE_USE(v2); FORCE_USE(v3);
        FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
        FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
        FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
        FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
    }
    
    /* Extract results from vectors */
    int varr[4];
    memcpy(varr, &v0, sizeof(v0));
    result = varr[0] + varr[1] + varr[2] + varr[3];
#else
    /* Fallback for non-GCC */
    for (int i = 0; i < iterations; i++) {
        r0 = (r0 * 1103515245 + 12345) & 0x7fffffff;
        result = r0 % 1000;
    }
#endif
    
    return result;
}

/* Pattern D: Explicit register variables with artificial conflicts */
NOINLINE int pattern_d_register_conflict(int a, int b, int c) {
    /* Explicit register variables creating artificial conflicts */
#ifdef __GNUC__
    register int x asm("r10") = a;
    register int y asm("r11") = b;
    register int z asm("r12") = c;
#else
    register int x = a;
    register int y = b;
    register int z = c;
#endif
    
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
    
    /* Dummy helper functions to force calls */
    auto NOINLINE int helper1(int p) { return p * 3 + 7; }
    auto NOINLINE int helper2(int p) { return p / 2 - 5; }
    auto NOINLINE int helper3(int p) { return p ^ 0x55aa; }
    
    /* Complex arithmetic using register variables */
    for (int i = 0; i < 100; i++) {
        switch (i % 20) {
            case 0: x = helper1(y); r0 = x + y; break;
            case 1: y = helper2(z); r1 = y + z; break;
            case 2: z = helper3(x); r2 = z + x; break;
            case 3: x = y * z; r3 = helper1(x); break;
            case 4: y = z / (x + 1); r4 = helper2(y); break;
            case 5: z = x ^ y; r5 = helper3(z); break;
            case 6: x = y | z; r6 = x * 2; break;
            case 7: y = z & x; r7 = y / 3; break;
            case 8: z = x << 1; r8 = z ^ 0xff; break;
            case 9: x = y >> 2; r9 = x | 0xaa; break;
            case 10: y = helper1(z); r0 = y - z; break;
            case 11: z = helper2(x); r1 = z - x; break;
            case 12: x = helper3(y); r2 = x - y; break;
            case 13: y = z * x; r3 = helper1(y); break;
            case 14: z = x / (y + 1); r4 = helper2(z); break;
            case 15: x = y ^ z; r5 = helper3(x); break;
            case 16: y = z | x; r6 = y * 3; break;
            case 17: z = x & y; r7 = z / 4; break;
            case 18: x = y << 2; r8 = x ^ 0x55; break;
            case 19: y = z >> 1; r9 = y | 0x33; break;
        }
        
        /* Force register variables to be live across calls */
        FORCE_USE(x); FORCE_USE(y); FORCE_USE(z);
        FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
        FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
        FORCE_USE(r8); FORCE_USE(r9);
    }
    
    return (x + y + z) % 1000;
}

/* Main driver with profile-guided optimization setup */
int main(int argc, char *argv[]) {
    volatile int total = 0;
    int iterations = 1000;
    
    if (argc > 1) {
        verbose = 1;
    }
    
    /* Use CPU features to engage target-specific register allocation */
    int use_avx2 = 0;
#ifdef __GNUC__
    use_avx2 = __builtin_cpu_supports("avx2");
    if (verbose && use_avx2) {
        printf("AVX2 supported, engaging vector register pressure\n");
    }
#endif
    
    /* Call each pattern multiple times with different arguments */
    for (int i = 0; i < iterations; i++) {
        total += pattern_a_irreducible(i % 100, i);
        total += pattern_b_setjmp_switch(i % 10, 50 + (i % 50));
        
        if (use_avx2) {
            total += pattern_c_vector_switch(20 + (i % 30), i);
        } else {
            total += pattern_c_vector_switch(10 + (i % 20), i);
        }
        
        total += pattern_d_register_conflict(i, i*2, i*3);
        
        /* Prevent loop unrolling */
        if (i % 7 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    if (verbose) {
        printf("Total result: %d\n", total);
    }
    
    return total % 256;
}
