/* test_mcf.c - Complex CFG to trigger MCF fixup graph special nodes */
#ifdef __GNUC__
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define USED __attribute__((used))
#define KEEP(var) asm volatile("" : : "g"(var))
#else
#define NOINLINE
#define HOT
#define USED
#define KEEP(var) (void)(var)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

static int verbose = 0;

/* Pattern A: Deeply nested if-else with irreducible region for ENTRY/EXIT blocks */
HOT NOINLINE USED
int pattern_a_irreducible(int iterations, int seed) {
    volatile int result = seed;
    int i, j, k;
    
    /* Create irreducible region with goto */
    if (iterations > 0) {
        goto middle;
    }
    
start:
    for (i = 0; i < iterations; i++) {
        int r0 = i * 2, r1 = i * 3, r2 = i * 5, r3 = i * 7;
        int r4 = r0 + r1, r5 = r1 + r2, r6 = r2 + r3, r7 = r3 + r0;
        
        if (i % 3 == 0) {
            for (j = 0; j < 10; j++) {
                r0 += j; r1 -= j; r2 *= (j + 1); r3 /= (j + 2);
                if (j == 5) goto middle;
            }
        } else if (i % 3 == 1) {
            k = 0;
            while (k < 8) {
                r4 ^= k; r5 |= k; r6 &= ~k; r7 = (r7 << k) | (r7 >> (32 - k));
                if (k == 3) goto start;
                k++;
            }
        } else {
            do {
                r0 = (r0 * 1103515245 + 12345) & 0x7fffffff;
                r1 = (r1 * 1103515245 + 12345) & 0x7fffffff;
                r2 = (r2 * 1103515245 + 12345) & 0x7fffffff;
                r3 = (r3 * 1103515245 + 12345) & 0x7fffffff;
            } while (r0 % 7 != 0);
        }
        
        result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
        KEEP(result);
    }
    
    return result;

middle:
    result = (result * 6364136223846793005ULL + 1442695040888963407ULL) & 0x7fffffff;
    KEEP(result);
    goto start;
}

/* Pattern B: setjmp/longjmp with many variables for NEW_EXIT/NEW_ENTRY */
NOINLINE USED
int pattern_b_setjmp(int depth, int iterations) {
    jmp_buf env;
    volatile int save_point = 0;
    int r0 = 1, r1 = 2, r2 = 3, r3 = 4, r4 = 5, r5 = 6, r6 = 7, r7 = 8;
    int r8 = 9, r9 = 10, r10 = 11, r11 = 12, r12 = 13, r13 = 14, r14 = 15, r15 = 16;
    int result = 0;
    
    if (setjmp(env) == 0) {
        for (int i = 0; i < iterations; i++) {
            /* Complex arithmetic to create register pressure */
            r0 = r0 * 3 + i;
            r1 = r1 * 5 - i;
            r2 = r2 * 7 ^ i;
            r3 = r3 * 11 | i;
            r4 = r4 * 13 & ~i;
            r5 = r5 * 17 + (i << 3);
            r6 = r6 * 19 - (i >> 2);
            r7 = r7 * 23 ^ (i & 0xF);
            r8 = r8 * 29 | (i | 0x10);
            r9 = r9 * 31 & (i ^ 0xFF);
            r10 = r10 * 37 + (i * i);
            r11 = r11 * 41 - (i % 19);
            r12 = r12 * 43 ^ (i / 3);
            r13 = r13 * 47 | (i + 0x100);
            r14 = r14 * 53 & (i - 0x50);
            r15 = r15 * 59 + (i * 0x11);
            
            result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 +
                     r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
            
            if (i == depth) {
                longjmp(env, 1);
            }
            
            KEEP(result);
        }
    } else {
        save_point = 1;
        KEEP(save_point);
    }
    
    return result + save_point;
}

/* Pattern C: Vector operations with large switch */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE USED
int pattern_c_vector_switch(int selector, int iterations) {
    volatile int result = 0;
    
#ifdef __GNUC__
    v4si vec0 = {1, 2, 3, 4};
    v4si vec1 = {5, 6, 7, 8};
    v4si vec2 = {9, 10, 11, 12};
    v4si vec3 = {13, 14, 15, 16};
#endif
    
    for (int iter = 0; iter < iterations; iter++) {
        int r0 = iter, r1 = iter * 2, r2 = iter * 3, r3 = iter * 4;
        int r4 = iter * 5, r5 = iter * 6, r6 = iter * 7, r7 = iter * 8;
        int r8 = iter * 9, r9 = iter * 10, r10 = iter * 11, r11 = iter * 12;
        int r12 = iter * 13, r13 = iter * 14, r14 = iter * 15, r15 = iter * 16;
        
        /* Large switch with 30+ cases */
        switch ((selector + iter) % 35) {
            case 0:
                r0 = r1 + r2; r1 = r3 - r4; r2 = r5 * r6; r3 = r7 / (r8 + 1);
                r4 = r9 ^ r10; r5 = r11 | r12; r6 = r13 & r14; r7 = r15 << 2;
#ifdef __GNUC__
                vec0 = vec0 + vec1;
#endif
                break;
            case 1:
                r8 = r0 * 3; r9 = r1 * 5; r10 = r2 * 7; r11 = r3 * 11;
                r12 = r4 * 13; r13 = r5 * 17; r14 = r6 * 19; r15 = r7 * 23;
#ifdef __GNUC__
                vec1 = vec1 * vec2;
#endif
                break;
            case 2:
                r0 = r8 ^ r9; r1 = r10 | r11; r2 = r12 & r13; r3 = r14 << r15;
                r4 = r0 >> 1; r5 = r1 >> 2; r6 = r2 >> 3; r7 = r3 >> 4;
                break;
            case 3:
                r8 = r4 + r5 + r6 + r7; r9 = r0 - r1 - r2 - r3;
                r10 = r4 * r5 * 2; r11 = r6 * r7 * 3;
                r12 = r8 / (r9 + 1); r13 = r10 % (r11 + 1);
                break;
            case 4:
                r14 = (r0 & 0xF) | (r1 & 0xF0) | (r2 & 0xF00);
                r15 = (r3 & 0xF000) | (r4 & 0xF0000) | (r5 & 0xF00000);
                r0 = r14 ^ r15; r1 = ~r14; r2 = ~r15;
                break;
            case 5:
                r3 = r6 + r7 + r8 + r9; r4 = r10 + r11 + r12 + r13;
                r5 = r3 * r4; r6 = r3 / (r4 + 1); r7 = r3 % (r4 + 1);
                break;
            case 6:
                r8 = r0 << 1; r9 = r1 << 2; r10 = r2 << 3; r11 = r3 << 4;
                r12 = r4 << 5; r13 = r5 << 6; r14 = r6 << 7; r15 = r7 << 8;
                break;
            case 7:
                r0 = r8 >> 1; r1 = r9 >> 2; r2 = r10 >> 3; r3 = r11 >> 4;
                r4 = r12 >> 5; r5 = r13 >> 6; r6 = r14 >> 7; r7 = r15 >> 8;
                break;
            case 8:
                r8 = r0 | r1 | r2 | r3; r9 = r4 & r5 & r6 & r7;
                r10 = r8 ^ r9; r11 = ~r8; r12 = ~r9;
                break;
            case 9:
                r13 = r0 + r4 + r8 + r12; r14 = r1 + r5 + r9 + r13;
                r15 = r2 + r6 + r10 + r14; r0 = r3 + r7 + r11 + r15;
                break;
            case 10:
                r1 = r13 * 0x9e3779b9; r2 = r14 * 0x9e3779b9;
                r3 = r15 * 0x9e3779b9; r4 = r0 * 0x9e3779b9;
                r5 = r1 ^ r2; r6 = r3 ^ r4; r7 = r5 ^ r6;
                break;
            case 11:
                r8 = (r0 + r1) * (r2 + r3); r9 = (r4 + r5) * (r6 + r7);
                r10 = (r8 + r9) / 2; r11 = (r8 - r9) / 2;
                break;
            case 12:
                r12 = r0 ^ r1 ^ r2 ^ r3 ^ r4 ^ r5 ^ r6 ^ r7;
                r13 = r8 ^ r9 ^ r10 ^ r11 ^ r12;
                r14 = ~r12; r15 = ~r13;
                break;
            case 13:
                r0 = r12 + r13; r1 = r12 - r13; r2 = r12 * r13;
                r3 = r12 / (r13 + 1); r4 = r12 % (r13 + 1);
                break;
            case 14:
                r5 = (r0 << 1) | (r1 >> 31); r6 = (r1 << 1) | (r2 >> 31);
                r7 = (r2 << 1) | (r3 >> 31); r8 = (r3 << 1) | (r4 >> 31);
                break;
            case 15:
                r9 = r5 + r6 + r7 + r8; r10 = r5 * r6 * r7 * r8;
                r11 = r9 / (r10 + 1); r12 = r9 % (r10 + 1);
                break;
            case 16:
                r13 = r0 & r4 & r8 & r12; r14 = r1 & r5 & r9 & r13;
                r15 = r2 & r6 & r10 & r14; r0 = r3 & r7 & r11 & r15;
                break;
            case 17:
                r1 = r13 | r14 | r15 | r0; r2 = r1 ^ 0xAAAAAAAA;
                r3 = r2 ^ 0x55555555; r4 = r3 ^ 0x33333333;
                break;
            case 18:
                r5 = r0 * 3 + r1 * 5 + r2 * 7 + r3 * 11;
                r6 = r4 * 13 + r5 * 17 + r6 * 19 + r7 * 23;
                r7 = r8 * 29 + r9 * 31 + r10 * 37 + r11 * 41;
                break;
            case 19:
                r8 = r12 * 43 + r13 * 47 + r14 * 53 + r15 * 59;
                r9 = r5 + r6 + r7 + r8; r10 = r9 / 4;
                break;
            case 20:
                r11 = (r0 << r1) | (r2 >> r3); r12 = (r4 << r5) | (r6 >> r7);
                r13 = (r8 << r9) | (r10 >> r11); r14 = (r12 << r13) | (r14 >> r15);
                break;
            case 21:
                r15 = r0 + r1 - r2 + r3 - r4 + r5 - r6 + r7;
                r0 = r8 + r9 - r10 + r11 - r12 + r13 - r14 + r15;
                break;
            case 22:
                r1 = (r0 * 1103515245 + 12345) & 0x7fffffff;
                r2 = (r1 * 1103515245 + 12345) & 0x7fffffff;
                r3 = (r2 * 1103515245 + 12345) & 0x7fffffff;
                r4 = (r3 * 1103515245 + 12345) & 0x7fffffff;
                break;
            case 23:
                r5 = r1 ^ r2 ^ r3 ^ r4; r6 = r5 << 1;
                r7 = r5 >> 1; r8 = r6 | r7;
                break;
            case 24:
                r9 = r0 * r1 * r2; r10 = r3 * r4 * r5;
                r11 = r6 * r7 * r8; r12 = r9 * r10 * r11;
                break;
            case 25:
                r13 = r12 / 1000; r14 = r12 % 1000;
                r15 = r13 + r14; r0 = r13 - r14;
                break;
            case 26:
                r1 = (r0 & 0xFF) << 24 | (r1 & 0xFF) << 16 |
                     (r2 & 0xFF) << 8 | (r3 & 0xFF);
                r2 = (r4 & 0xFF) << 24 | (r5 & 0xFF) << 16 |
                     (r6 & 0xFF) << 8 | (r7 & 0xFF);
                break;
            case 27:
                r3 = r1 + r2; r4 = r1 - r2; r5 = r1 * r2;
                r6 = r1 / (r2 + 1); r7 = r1 % (r2 + 1);
                break;
            case 28:
                r8 = ~r3; r9 = ~r4; r10 = ~r5; r11 = ~r6;
                r12 = r8 & r9; r13 = r10 | r11;
                break;
            case 29:
                r14 = r12 ^ r13; r15 = (r14 << 16) | (r14 >> 16);
                r0 = r15 ^ 0xDEADBEEF; r1 = r0 ^ 0xCAFEBABE;
                break;
            case 30:
                r2 = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
                r3 = r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
                break;
            case 31:
                r4 = r2 * r3; r5 = r2 / (r3 + 1);
                r6 = r2 % (r3 + 1); r7 = r4 + r5 + r6;
                break;
            case 32:
                r8 = (r0 << 5) ^ (r1 << 4) ^ (r2 << 3) ^ (r3 << 2);
                r9 = (r4 << 1) ^ (r5 >> 1) ^ (r6 >> 2) ^ (r7 >> 3);
                break;
            case 33:
                r10 = r8 + r9; r11 = r8 - r9;
                r12 = r8 * r9; r13 = r8 / (r9 + 1);
                break;
            case 34:
                r14 = r10 ^ r11 ^ r12 ^ r13;
                r15 = (r14 * 6364136223846793005ULL) & 0x7fffffff;
                break;
        }
        
        result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 +
                 r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
        
#ifdef __GNUC__
        vec0 = vec0 + vec1;
        vec2 = vec2 * vec3;
        int vec_sum = vec0[0] + vec0[1] + vec0[2] + vec0[3] +
                     vec1[0] + vec1[1] + vec1[2] + vec1[3] +
                     vec2[0] + vec2[1] + vec2[2] + vec2[3];
        result += vec_sum;
#endif
        
        KEEP(result);
        
        /* Loop control with continue to different cases */
        if (iter % 7 == 0) {
            continue;
        } else if (iter % 13 == 0) {
            selector = (selector * 3 + 1) % 35;
            continue;
        } else if (iter % 17 == 0) {
            break;
        }
    }
    
    return result;
}

/* Pattern D: Explicit register variables with artificial conflicts */
NOINLINE USED
int helper1(int x) { return x * 3 + 1; }
int helper2(int x) { return x * 5 - 2; }
int helper3(int x) { return x * 7 ^ 3; }
int helper4(int x) { return x * 11 | 4; }

NOINLINE USED
int pattern_d_register_conflict(int iterations) {
    /* Force register conflicts */
#ifdef __GNUC__
    register int a asm ("r10");
    register int b asm ("r11");
    register int c asm ("r12");
    register int d asm ("r13");
#else
    int a, b, c, d;
#endif
    
    volatile int result = 0;
    
    a = 1; b = 2; c = 3; d = 4;
    
    for (int i = 0; i < iterations; i++) {
        /* Artificial live-range splits */
        int tmp1 = helper1(a);
        int tmp2 = helper2(b);
        int tmp3 = helper3(c);
        int tmp4 = helper4(d);
        
        a = tmp1 + i;
        b = tmp2 - i;
        c = tmp3 ^ i;
        d = tmp4 | i;
        
        /* More register pressure */
        int r0 = a * 2, r1 = b * 3, r2 = c * 5, r3 = d * 7;
        int r4 = r0 + r1, r5 = r1 + r2, r6 = r2 + r3, r7 = r3 + r0;
        int r8 = r4 * r5, r9 = r5 * r6, r10 = r6 * r7, r11 = r7 * r4;
        int r12 = r8 ^ r9, r13 = r9 ^ r10, r14 = r10 ^ r11, r15 = r11 ^ r8;
        
        result += a + b + c + d + r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 +
                 r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
        
        KEEP(result);
        
        /* Force spill/reload */
        if (i % 5 == 0) {
            int spill1 = helper1(a + b);
            int spill2 = helper2(c + d);
            int spill3 = helper3(r0 + r1);
            int spill4 = helper4(r2 + r3);
            
            a = spill1; b = spill2; c = spill3; d = spill4;
        }
    }
    
    return result;
}

/* Main function with profile feedback and CPU feature detection */
int main(int argc, char *argv[]) {
    volatile int total = 0;
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
        printf("Starting MCF stress test with %d iterations\n", iterations);
    }
    
    /* Use CPU feature detection to engage target-specific register allocation */
    int use_avx2 = 0;
#ifdef __GNUC__
    use_avx2 = __builtin_cpu_supports("avx2");
    if (verbose) {
        printf("AVX2 supported: %d\n", use_avx2);
    }
#endif
    
    /* Call each pattern multiple times with different arguments */
    for (int i = 0; i < 10; i++) {
        total += pattern_a_irreducible(iterations / 10, i * 12345);
        total += pattern_b_setjmp(i * 3, iterations / 20);
        total += pattern_c_vector_switch(i * 7, iterations / 15);
        total += pattern_d_register_conflict(iterations / 25);
        
        KEEP(total);
    }
    
    if (verbose) {
        printf("Total result: %d\n", total);
    }
    
    return total & 0xFF;  /* Return non-zero but limited range */
}
