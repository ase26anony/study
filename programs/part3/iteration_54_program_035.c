/* test_mcf.c - Complex CFG generator for MCF pass testing */
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

static int verbose = 0;

/* ========== Pattern A: ENTRY/EXIT block stress ========== */
HOT NOINLINE
int pattern_a_entry_exit(int iterations, int seed) {
    int result = seed;
    int i, j, k;
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15;
    
    /* Create irreducible region with goto */
    if (iterations > 100) {
        goto irreducible_start;
    }
    
    for (i = 0; i < iterations; i++) {
        /* Deep if-else chain creating many basic blocks */
        if (i % 3 == 0) {
            r0 = i * 2;
            r1 = seed + r0;
            r2 = r1 * 3;
            if (r2 > 1000) {
                r3 = r2 / 5;
                for (j = 0; j < 10; j++) {
                    r4 = j * r3;
                    if (r4 % 7 == 0) {
                        r5 = r4 + seed;
                        continue;
                    } else {
                        r6 = r4 - seed;
                        break;
                    }
                }
            }
        } else if (i % 3 == 1) {
            r7 = i * i;
            r8 = seed ^ r7;
            r9 = r8 << 2;
            for (k = 0; k < 5; k++) {
                r10 = k * r9;
                if (r10 > 50) {
                    r11 = r10 % 17;
                    goto mid_loop;
                }
            }
        } else {
            r12 = i + seed;
            r13 = r12 * r12;
            r14 = r13 % 19;
            if (r14 == 0) {
                r15 = seed * 31;
                result += r15;
            }
        }
        
mid_loop:
        /* Force all variables live */
        FORCE_USE(r0); FORCE_USE(r1); FORCE_USE(r2); FORCE_USE(r3);
        FORCE_USE(r4); FORCE_USE(r5); FORCE_USE(r6); FORCE_USE(r7);
        FORCE_USE(r8); FORCE_USE(r9); FORCE_USE(r10); FORCE_USE(r11);
        FORCE_USE(r12); FORCE_USE(r13); FORCE_USE(r14); FORCE_USE(r15);
    }
    
    return result;

irreducible_start:
    /* Create irreducible loop */
    int a = 0, b = 0, c = 0;
    while (a < iterations) {
        if (a % 2 == 0) {
            goto label1;
        } else {
            goto label2;
        }
        
label1:
        b += a * 3;
        if (b > 100) goto label3;
        else goto label2;
        
label2:
        c += a * 5;
        if (c > 200) goto label1;
        else goto label3;
        
label3:
        a++;
        if (a % 10 == 0) goto label1;
    }
    
    return b + c;
}

/* ========== Pattern B: NEW_EXIT/NEW_ENTRY with setjmp ========== */
static jmp_buf jump_buffer;
static int jump_counter = 0;

NOINLINE
static void dummy_helper_b1(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * 3 + 7;
    }
    FORCE_USE(arr);
}

NOINLINE
static void dummy_helper_b2(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    FORCE_USE(sum);
}

int pattern_b_new_indices(int depth, int width) {
    int result = 0;
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    int w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12, w13, w14, w15;
    
    /* Initialize many variables */
    v0 = depth; v1 = width; v2 = v0 * v1; v3 = v2 + 17;
    v4 = v3 % 23; v5 = v4 << 3; v6 = v5 ^ 0x55; v7 = v6 * 31;
    v8 = v7 / 7; v9 = v8 + 11; v10 = v9 % 19; v11 = v10 * 3;
    v12 = v11 + 5; v13 = v12 << 1; v14 = v13 ^ 0xAA; v15 = v14;
    
    w0 = width; w1 = depth; w2 = w0 + w1; w3 = w2 * 5;
    w4 = w3 % 29; w5 = w4 + 13; w6 = w5 << 2; w7 = w6 ^ 0x33;
    w8 = w7 * 7; w9 = w8 / 3; w10 = w9 + 19; w11 = w10 % 31;
    w12 = w11 * 11; w13 = w12 + 7; w14 = w13 << 3; w15 = w14;
    
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        for (int i = 0; i < depth; i++) {
            int arr[16];
            for (int j = 0; j < 16; j++) {
                arr[j] = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 +
                         v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15 +
                         w0 + w1 + w2 + w3 + w4 + w5 + w6 + w7 +
                         w8 + w9 + w10 + w11 + w12 + w13 + w14 + w15 +
                         i * 100 + j;
            }
            
            dummy_helper_b1(arr, 16);
            
            /* Complex condition for longjmp */
            if (i > width && (v0 + v1 + v15) % 37 == 0) {
                jump_counter++;
                if (jump_counter < 3) {
                    longjmp(jump_buffer, 1);
                }
            }
            
            dummy_helper_b2(arr, 16);
            
            /* Update all variables in loop */
            v0 = (v0 + 1) % 256; v1 = (v1 * 3) % 256;
            v2 = (v2 + v0) % 256; v3 = (v3 ^ v1) % 256;
            v4 = (v4 * 5) % 256; v5 = (v5 + 7) % 256;
            v6 = (v6 << 1) % 256; v7 = (v7 / 3) % 256;
            v8 = (v8 + v2) % 256; v9 = (v9 * 11) % 256;
            v10 = (v10 % 17) % 256; v11 = (v11 + 19) % 256;
            v12 = (v12 ^ 0xCC) % 256; v13 = (v13 * 13) % 256;
            v14 = (v14 + 23) % 256; v15 = (v15 % 29) % 256;
            
            w0 = (w0 + 2) % 256; w1 = (w1 * 7) % 256;
            w2 = (w2 + w0) % 256; w3 = (w3 ^ w1) % 256;
            w4 = (w4 * 3) % 256; w5 = (w5 + 11) % 256;
            w6 = (w6 << 2) % 256; w7 = (w7 / 5) % 256;
            w8 = (w8 + w2) % 256; w9 = (w9 * 17) % 256;
            w10 = (w10 % 13) % 256; w11 = (w11 + 29) % 256;
            w12 = (w12 ^ 0xAA) % 256; w13 = (w13 * 19) % 256;
            w14 = (w14 + 31) % 256; w15 = (w15 % 23) % 256;
            
            FORCE_USE(v0); FORCE_USE(v1); FORCE_USE(v2); FORCE_USE(v3);
            FORCE_USE(v4); FORCE_USE(v5); FORCE_USE(v6); FORCE_USE(v7);
            FORCE_USE(v8); FORCE_USE(v9); FORCE_USE(v10); FORCE_USE(v11);
            FORCE_USE(v12); FORCE_USE(v13); FORCE_USE(v14); FORCE_USE(v15);
            FORCE_USE(w0); FORCE_USE(w1); FORCE_USE(w2); FORCE_USE(w3);
            FORCE_USE(w4); FORCE_USE(w5); FORCE_USE(w6); FORCE_USE(w7);
            FORCE_USE(w8); FORCE_USE(w9); FORCE_USE(w10); FORCE_USE(w11);
            FORCE_USE(w12); FORCE_USE(w13); FORCE_USE(w14); FORCE_USE(w15);
        }
    } else {
        /* After longjmp */
        result = v0 + v1 + v15 + w0 + w1 + w15;
    }
    
    return result + depth + width;
}

/* ========== Pattern C: Mixed vector/scalar pressure ========== */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
#endif

NOINLINE
int pattern_c_mixed_pressure(int mode, int iterations) {
    int result = 0;
    
#ifdef __GNUC__
    v4si vec0 = {1, 2, 3, 4};
    v4si vec1 = {5, 6, 7, 8};
    v4si vec2 = {9, 10, 11, 12};
    v4si vec3 = {13, 14, 15, 16};
#endif
    
    int s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15;
    int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15;
    
    s0 = mode; s1 = iterations; s2 = s0 * s1; s3 = s2 + 1;
    s4 = s3 % 97; s5 = s4 << 1; s6 = s5 ^ 0xFF; s7 = s6 * 3;
    s8 = s7 / 2; s9 = s8 + 5; s10 = s9 % 89; s11 = s10 * 7;
    s12 = s11 + 11; s13 = s12 << 2; s14 = s13 ^ 0x55; s15 = s14;
    
    for (int i = 0; i < iterations; i++) {
        /* Large switch with 30+ cases */
        switch ((s0 + i) % 35) {
            case 0:
                t0 = s0 * 2; t1 = s1 + t0; t2 = t1 * 3;
#ifdef __GNUC__
                vec0 = vec0 + vec1;
#endif
                result += t2;
                break;
            case 1:
                t3 = s2 / 5; t4 = s3 + t3; t5 = t4 % 17;
#ifdef __GNUC__
                vec1 = vec1 * vec2;
#endif
                result += t5;
                break;
            case 2:
                t6 = s4 << 3; t7 = s5 ^ t6; t8 = t7 * 7;
#ifdef __GNUC__
                vec2 = vec2 - vec0;
#endif
                result += t8;
                break;
            case 3:
                t9 = s6 / 11; t10 = s7 + t9; t11 = t10 % 19;
#ifdef __GNUC__
                vec3 = vec3 + vec1;
#endif
                result += t11;
                break;
            case 4:
                t12 = s8 * 13; t13 = s9 + t12; t14 = t13 << 1;
#ifdef __GNUC__
                vec0 = vec0 * vec3;
#endif
                result += t14;
                break;
            case 5:
                t15 = s10 % 23; t0 = s11 + t15; t1 = t0 * 5;
#ifdef __GNUC__
                vec1 = vec1 + vec2;
#endif
                result += t1;
                break;
            case 6:
                t2 = s12 ^ 0xAA; t3 = s13 + t2; t4 = t3 / 3;
#ifdef __GNUC__
                vec2 = vec2 * vec0;
#endif
                result += t4;
                break;
            case 7:
                t5 = s14 << 2; t6 = s15 + t5; t7 = t6 % 29;
#ifdef __GNUC__
                vec3 = vec3 - vec1;
#endif
                result += t7;
                break;
            case 8:
                t8 = s0 * 17; t9 = s1 + t8; t10 = t9 ^ 0xCC;
                result += t10;
                break;
            case 9:
                t11 = s2 / 7; t12 = s3 + t11; t13 = t12 * 11;
                result += t13;
                break;
            case 10:
                t14 = s4 % 31; t15 = s5 + t14; t0 = t15 << 3;
                result += t0;
                break;
            case 11:
                t1 = s6 * 19; t2 = s7 + t1; t3 = t2 / 5;
                result += t3;
                break;
            case 12:
                t4 = s8 ^ 0x33; t5 = s9 + t4; t6 = t5 % 37;
                result += t6;
                break;
            case 13:
                t7 = s10 << 1; t8 = s11 + t7; t9 = t8 * 23;
                result += t9;
                break;
            case 14:
                t10 = s12 / 13; t11 = s13 + t10; t12 = t11 ^ 0x99;
                result += t12;
                break;
            case 15:
                t13 = s14 % 41; t14 = s15 + t13; t15 = t14 << 2;
                result += t15;
                break;
            case 16:
                t0 = s0 * 29; t1 = s1 + t0; t2 = t1 / 17;
                result += t2;
                break;
            case 17:
                t3 = s2 ^ 0x66; t4 = s3 + t3; t5 = t4 % 43;
                result += t5;
                break;
            case 18:
                t6 = s4 << 4; t7 = s5 + t6; t8 = t7 * 31;
                result += t8;
                break;
            case 19:
                t9 = s6 / 19; t10 = s7 + t9; t11 = t10 ^ 0xBB;
                result += t11;
                break;
            case 20:
                t12 = s8 % 47; t13 = s9 + t12; t14 = t13 << 1;
                result += t14;
                break;
            case 21:
                t15 = s10 * 37; t0 = s11 + t15; t1 = t0 / 23;
                result += t1;
                break;
            case 22:
                t2 = s12 ^ 0x44; t3 = s13 + t2; t4 = t3 % 53;
                result += t4;
                break;
            case 23:
                t5 = s14 << 3; t6 = s15 + t5; t7 = t6 * 41;
                result += t7;
                break;
            case 24:
                t8 = s0 / 29; t9 = s1 + t8; t10 = t9 ^ 0x22;
                result += t10;
                break;
            case 25:
                t11 = s2 % 59; t12 = s3 + t11; t13 = t12 << 2;
                result += t13;
                break;
            case 26:
                t14 = s4 * 43; t15 = s5 + t14; t0 = t15 / 31;
                result += t0;
                break;
            case 27:
                t1 = s6 ^ 0x88; t2 = s7 + t1; t3 = t2 % 61;
                result += t3;
                break;
            case 28:
                t4 = s8 << 5; t5 = s9 + t4; t6 = t5 * 47;
                result += t6;
                break;
            case 29:
                t7 = s10 / 37; t8 = s11 + t7; t9 = t8 ^ 0x11;
                result += t9;
                break;
            case 30:
                t10 = s12 % 67; t11 = s13 + t10; t12 = t11 << 3;
                result += t12;
                break;
            case 31:
                t13 = s14 * 53; t14 = s15 + t13; t15 = t14 / 41;
                result += t15;
                break;
            case 32:
                t0 = s0 ^ 0x77; t1 = s1 + t0; t2 = t1 % 71;
                result += t2;
                break;
            case 33:
                t3 = s2 << 6; t4 = s3 + t3; t5 = t4 * 59;
                result += t5;
                break;
            case 34:
                t6 = s4 / 43; t7 = s5 + t6; t8 = t7 ^ 0xEE;
                result += t8;
                break;
        }
        
        /* Update scalar variables */
        s0 = (s0 + 1) % 100; s1 = (s1 * 3) % 100;
        s2 = (s2 + s0) % 100; s3 = (s3 ^ s1) % 100;
        s4 = (s4 * 5) % 100; s5 = (s5 + 7) % 100;
        s6 = (s6 << 1) % 100; s7 = (s7 / 3) % 100;
        s8 = (s8 + s2) % 100; s9 = (s9 * 11) % 100;
        s10 = (s10 % 17) % 100; s11 = (s11 + 19) % 100;
        s12 = (s12 ^ 0xCC) % 100; s13 = (s13 * 13) % 100;
        s14 = (s14 + 23) % 100; s15 = (s15 % 29) % 100;
        
        /* Force all variables live */
        FORCE_USE(s0); FORCE_USE(s1); FORCE_USE(s2); FORCE_USE(s3);
        FORCE_USE(s4); FORCE_USE(s5); FORCE_USE(s6); FORCE_USE(s7);
        FORCE_USE(s8); FORCE_USE(s9); FORCE_USE(s10); FORCE_USE(s11);
        FORCE_USE(s12); FORCE_USE(s13); FORCE_USE(s14); FORCE_USE(s15);
        FORCE_USE(t0); FORCE_USE(t1); FORCE_USE(t2); FORCE_USE(t3);
        FORCE_USE(t4); FORCE_USE(t5); FORCE_USE(t6); FORCE_USE(t7);
        FORCE_USE(t8); FORCE_USE(t9); FORCE_USE(t10); FORCE_USE(t11);
        FORCE_USE(t12); FORCE_USE(t13); FORCE_USE(t14); FORCE_USE(t15);
#ifdef __GNUC__
        FORCE_USE(vec0); FORCE_USE(vec1); FORCE_USE(vec2); FORCE_USE(vec3);
#endif
        
        /* Loop control with continue to different cases */
        if (i % 7 == 0) {
            continue;
        } else if (i % 13 == 0) {
            i++;
            continue;
        }
    }
    
    return result;
}

/* ========== Pattern D: Artificial register conflicts ========== */
NOINLINE
static void dummy_helper_d1(REGISTER_VAR(a, "r10"), 
                           REGISTER_VAR(b, "r11")) {
    FORCE_USE(a);
    FORCE_USE(b);
}

NOINLINE
static void dummy_helper_d2(REGISTER_VAR(c, "r10"), 
                           REGISTER_VAR(d, "r12")) {
    FORCE_USE(c);
    FORCE_USE(d);
}

NOINLINE
static void dummy_helper_d3(REGISTER_VAR(e, "r11"), 
                           REGISTER_VAR(f, "r12")) {
    FORCE_USE(e);
    FORCE_USE(f);
}

int pattern_d_register_conflict(int n) {
    REGISTER_VAR(x, "r10") = n * 2;
    REGISTER_VAR(y, "r11") = n * 3;
    REGISTER_VAR(z, "r12") = n * 5;
    
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        /* Create artificial conflicts */
        if (i % 4 == 0) {
            dummy_helper_d1(x, y);
            x = x + y + i;
            result += x;
        } else if (i % 4 == 1) {
            dummy_helper_d2(z, x);
            y = y + z + i;
            result += y;
        } else if (i % 4 == 2) {
            dummy_helper_d3(y, z);
            z = z + x + i;
            result += z;
        } else {
            /* Use all three in conflicting ways */
            int temp = x + y + z;
            dummy_helper_d1(x, y);
            dummy_helper_d2(z, x);
            dummy_helper_d3(y, z);
            result += temp;
        }
        
        /* Additional arithmetic to increase pressure */
        int a = x * 7, b = y * 11, c = z * 13;
        int d = a + b, e = b + c, f = c + a;
        int g = d * e, h = e * f, j = f * d;
        int k = g % 97, l = h % 89, m = j % 83;
        
        FORCE_USE(a); FORCE_USE(b); FORCE_USE(c);
        FORCE_USE(d); FORCE_USE(e); FORCE_USE(f);
        FORCE_USE(g); FORCE_USE(h); FORCE_USE(j);
        FORCE_USE(k); FORCE_USE(l); FORCE_USE(m);
        
        /* Complex loop control */
        switch (i % 8) {
            case 0: x = (x + 1) % 256; break;
            case 1: y = (y * 3) % 256; break;
            case 2: z = (z + 5) % 256; break;
            case 3: x = (x ^ y) % 256; break;
            case 4: y = (y ^ z) % 256; break;
            case 5: z = (z ^ x) % 256; break;
            case 6: x = (x + y + z) % 256; break;
            case 7: i += (x + y + z) % 3; break;
        }
    }
    
    return result;
}

/* ========== Main driver ========== */
int main(int argc, char **argv) {
    volatile int total = 0;
    int i;
    
    if (argc > 1 && strcmp(argv[1], "-v") == 0) {
        verbose = 1;
    }
    
    /* Use profile data if available */
    int iterations = 1000;
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations < 100) iterations = 100;
        if (iterations > 10000) iterations = 10000;
    }
    
    /* Check for CPU features to engage target-specific allocation */
    int has_avx2 = 0;
#ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) {
        has_avx2 = 1;
        if (verbose) printf("AVX2 supported, engaging vector register allocation\n");
    }
#endif
    
    /* Run all patterns multiple times */
    for (i = 0; i < iterations; i++) {
        /* Pattern A - ENTRY/EXIT blocks */
        int r1 = pattern_a_entry_exit(i % 100 + 50, i * 3);
        total += r1;
        
        /* Pattern B - NEW_EXIT/NEW_ENTRY with setjmp */
        jump_counter = 0;
        int r2 = pattern_b_new_indices(i % 20 + 5, i % 30 + 10);
        total += r2;
        
        /* Pattern C - Mixed pressure */
        int r3 = pattern_c_mixed_pressure(i % 10, i % 50 + 20);
        total += r3;
        
        /* Pattern D - Register conflicts */
        int r4 = pattern_d_register_conflict(i % 40 + 10);
        total += r4;
        
        if (verbose && i % 100 == 0) {
            printf("Iteration %d: results = %d, %d, %d, %d, total = %d\n",
                   i, r1, r2, r3, r4, total);
        }
    }
    
    if (verbose) {
        printf("Final total: %d\n", total);
    }
    
    /* Prevent optimization of total */
    FORCE_USE(total);
    
    return total != 0 ? 0 : 1;
}
