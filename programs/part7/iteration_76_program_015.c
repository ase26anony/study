/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow algorithm fixup graph debug output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define KEEP_ALIVE(var) asm volatile("" : : "r"(var))

/* Complex irreducible control flow with goto jumping across loops */
NOINLINE static uint64_t test_irreducible_goto(int iterations, int seed) {
    /* Declare many variables to create register pressure */
    volatile int v0 = seed + 1;
    int v1 = seed + 2, v2 = seed + 3, v3 = seed + 4, v4 = seed + 5;
    int v5 = seed + 6, v6 = seed + 7, v7 = seed + 8, v8 = seed + 9, v9 = seed + 10;
    float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f, f3 = seed * 0.4f;
    double d0 = seed * 0.01, d1 = seed * 0.02, d2 = seed * 0.03, d3 = seed * 0.04;
    long l0 = seed * 100, l1 = seed * 200, l2 = seed * 300, l3 = seed * 400;
    int v10 = seed + 11, v11 = seed + 12, v12 = seed + 13, v13 = seed + 14, v14 = seed + 15;
    int v15 = seed + 16, v16 = seed + 17, v17 = seed + 18, v18 = seed + 19, v19 = seed + 20;
    
    uint64_t checksum = 0;
    int i = 0;
    
    /* Outer loop with label for goto target */
    outer_loop:
    for (; i < iterations; i++) {
        /* Create arithmetic dependency chains */
        v1 = v0 * 3 + v1;
        v2 = v1 / 2 + v2;
        v3 = v2 ^ v3;
        v4 = v3 | v4;
        v5 = v4 & v5;
        
        f0 = f1 * 1.1f + f0;
        f1 = f2 * 0.9f + f1;
        f2 = f3 * 1.2f - f2;
        
        d0 = d1 * 1.01 + d0;
        d1 = d2 * 0.99 + d1;
        
        l0 = l1 + l2 - l0;
        l1 = l2 * 2 + l1;
        
        /* Inner loop */
        for (int j = 0; j < 5; j++) {
            v6 = v5 + v6 + j;
            v7 = v6 - v7 * j;
            v8 = v7 ^ v8;
            v9 = v8 | v9;
            
            f3 = f2 * (j + 1) + f3;
            d2 = d1 / (j + 1) + d2;
            d3 = d2 * 1.5 - d3;
            
            l2 = l1 << (j & 3);
            l3 = l2 >> 1 + l3;
            
            /* More arithmetic to increase register pressure */
            v10 = v9 + v10 * j;
            v11 = v10 - v11 / (j + 1);
            v12 = v11 ^ v12;
            v13 = v12 | v13;
            v14 = v13 & v14;
            v15 = v14 + v15;
            v16 = v15 - v16;
            v17 = v16 * v17;
            v18 = v17 / (v18 ? v18 : 1);
            v19 = v18 ^ v19;
            
            /* Force variables to stay live */
            KEEP_ALIVE(v0); KEEP_ALIVE(v1); KEEP_ALIVE(v2); KEEP_ALIVE(v3);
            KEEP_ALIVE(v4); KEEP_ALIVE(v5); KEEP_ALIVE(v6); KEEP_ALIVE(v7);
            KEEP_ALIVE(v8); KEEP_ALIVE(v9); KEEP_ALIVE(v10); KEEP_ALIVE(v11);
            KEEP_ALIVE(v12); KEEP_ALIVE(v13); KEEP_ALIVE(v14); KEEP_ALIVE(v15);
            KEEP_ALIVE(v16); KEEP_ALIVE(v17); KEEP_ALIVE(v18); KEEP_ALIVE(v19);
            KEEP_ALIVE(f0); KEEP_ALIVE(f1); KEEP_ALIVE(f2); KEEP_ALIVE(f3);
            KEEP_ALIVE(d0); KEEP_ALIVE(d1); KEEP_ALIVE(d2); KEEP_ALIVE(d3);
            KEEP_ALIVE(l0); KEEP_ALIVE(l1); KEEP_ALIVE(l2); KEEP_ALIVE(l3);
            
            /* Irreducible control flow: goto to outer loop from inner loop */
            if ((i * j) % 37 == 0) {
                v0 = (v0 + 1) % 100;
                goto outer_loop;  /* Creates irreducible region */
            }
            
            /* Another goto target inside the inner loop */
            if ((i * j) % 41 == 0) {
                goto inner_label;
            }
        }
        
        inner_label:
        /* Continue with more computations */
        v0 = v19 + v0;
        checksum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                   v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                   (int)f0 + (int)f1 + (int)f2 + (int)f3 +
                   (int)d0 + (int)d1 + (int)d2 + (int)d3 +
                   l0 + l1 + l2 + l3;
    }
    
    return checksum;
}

/* Switch statement with goto creating complex CFG */
NOINLINE static uint64_t test_switch_goto(int iterations, int seed) {
    volatile int state = seed % 5;
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3, e = seed + 4;
    int f = seed + 5, g = seed + 6, h = seed + 7, i = seed + 8, j = seed + 9;
    int k = seed + 10, l = seed + 11, m = seed + 12, n = seed + 13, o = seed + 14;
    int p = seed + 15, q = seed + 16, r = seed + 17, s = seed + 18, t = seed + 19;
    float fa = seed * 0.1f, fb = seed * 0.2f, fc = seed * 0.3f;
    double da = seed * 0.01, db = seed * 0.02, dc = seed * 0.03;
    long la = seed * 100, lb = seed * 200, lc = seed * 300;
    
    uint64_t checksum = 0;
    
    loop_start:
    for (int iter = 0; iter < iterations; iter++) {
        /* Long arithmetic chains */
        a = b * c - d;
        b = c / (e ? e : 1) + f;
        c = d ^ e | f;
        d = e & f + g;
        e = f - g * h;
        f = g + h / (i ? i : 1);
        g = h ^ i & j;
        h = i | j - k;
        i = j * k + l;
        j = k - l / (m ? m : 1);
        k = l ^ m | n;
        l = m & n + o;
        m = n - o * p;
        n = o + p / (q ? q : 1);
        o = p ^ q & r;
        p = q | r - s;
        q = r * s + t;
        r = s - t / (a ? a : 1);
        s = t ^ a | b;
        t = a & b + c;
        
        fa = fb * 1.1f - fc;
        fb = fc / 2.0f + fa;
        fc = fa * 0.9f + fb;
        
        da = db * 1.01 - dc;
        db = dc / 2.0 + da;
        dc = da * 0.99 + db;
        
        la = lb << 1 + lc;
        lb = lc >> 2 + la;
        lc = la * 3 - lb;
        
        KEEP_ALIVE(a); KEEP_ALIVE(b); KEEP_ALIVE(c); KEEP_ALIVE(d); KEEP_ALIVE(e);
        KEEP_ALIVE(f); KEEP_ALIVE(g); KEEP_ALIVE(h); KEEP_ALIVE(i); KEEP_ALIVE(j);
        KEEP_ALIVE(k); KEEP_ALIVE(l); KEEP_ALIVE(m); KEEP_ALIVE(n); KEEP_ALIVE(o);
        KEEP_ALIVE(p); KEEP_ALIVE(q); KEEP_ALIVE(r); KEEP_ALIVE(s); KEEP_ALIVE(t);
        KEEP_ALIVE(fa); KEEP_ALIVE(fb); KEEP_ALIVE(fc);
        KEEP_ALIVE(da); KEEP_ALIVE(db); KEEP_ALIVE(dc);
        KEEP_ALIVE(la); KEEP_ALIVE(lb); KEEP_ALIVE(lc);
        
        /* Switch with goto to different labels - creates irreducible CFG */
        switch (state) {
            case 0:
                a = b + c;
                if (iter % 7 == 0) goto case_2_label;
                state = 1;
                break;
            case 1:
                b = c * d;
                if (iter % 11 == 0) goto loop_start;  /* Jump to outer loop */
                state = 2;
                break;
            case 2:
            case_2_label:
                c = d - e;
                if (iter % 13 == 0) goto case_4_label;
                state = 3;
                break;
            case 3:
                d = e ^ f;
                if (iter % 17 == 0) goto loop_start;
                state = 4;
                break;
            case 4:
            case_4_label:
                e = f | g;
                if (iter % 19 == 0) goto case_0_label;
                state = 0;
                break;
            default:
                state = 0;
        }
        
        case_0_label:
        checksum += a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p + q + r + s + t +
                   (int)fa + (int)fb + (int)fc + (int)da + (int)db + (int)dc + la + lb + lc;
        
        /* Modify state to make control flow unpredictable */
        state = (state * 1103515245 + 12345) & 0x7fffffff;
        state = state % 5;
    }
    
    return checksum;
}

/* Computed goto state machine */
NOINLINE static uint64_t test_computed_goto(int iterations, int seed) {
    /* Labels for computed goto */
    static void* labels[] = {
        &&state_a, &&state_b, &&state_c, &&state_d, &&state_e,
        &&state_f, &&state_g, &&state_h
    };
    
    /* Many variables for register pressure */
    int vars[MANY_VARS];
    float fvars[10];
    double dvars[10];
    long lvars[10];
    
    for (int i = 0; i < MANY_VARS; i++) vars[i] = seed + i;
    for (int i = 0; i < 10; i++) {
        fvars[i] = seed * (i + 1) * 0.1f;
        dvars[i] = seed * (i + 1) * 0.01;
        lvars[i] = seed * (i + 1) * 100;
    }
    
    int state = seed % 8;
    uint64_t checksum = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Jump to current state */
        goto *labels[state];
        
        state_a:
            /* Complex arithmetic on many variables */
            for (int i = 0; i < MANY_VARS - 1; i++) {
                vars[i] = vars[i] * 3 + vars[i + 1];
            }
            for (int i = 0; i < 9; i++) {
                fvars[i] = fvars[i] * 1.1f + fvars[i + 1];
                dvars[i] = dvars[i] * 1.01 + dvars[i + 1];
                lvars[i] = lvars[i] + lvars[i + 1] * 2;
            }
            state = (iter % 3 == 0) ? 1 : 7;
            goto end_state;
        
        state_b:
            for (int i = 0; i < MANY_VARS - 1; i++) {
                vars[i] = vars[i] / (vars[i + 1] ? vars[i + 1] : 1) + i;
            }
            for (int i = 0; i < 9; i++) {
                fvars[i] = fvars[i] - fvars[i + 1] * 0.5f;
                dvars[i] = dvars[i] / (dvars[i + 1] + 1.0);
                lvars[i] = lvars[i] ^ lvars[i + 1];
            }
            state = (iter % 5 == 0) ? 2 : 0;
            goto end_state;
        
        state_c:
            for (int i = 0; i < MANY_VARS - 1; i++) {
                vars[i] = vars[i] ^ vars[i + 1] | i;
            }
            for (int i = 0; i < 9; i++) {
                fvars[i] = fvars[i + 1] * 2.0f - fvars[i];
                dvars[i] = dvars[i + 1] * 1.5 - dvars[i];
                lvars[i] = lvars[i] << (lvars[i + 1] & 3);
            }
            state = (iter % 7 == 0) ? 3 : 1;
            goto end_state;
        
        state_d:
            for (int i = 0; i < MANY_VARS - 1; i++) {
                vars[i] = vars[i] | vars[i + 1] & (i + 1);
            }
            for (int i = 0; i < 9; i++) {
                fvars[i] = fvars[i] + fvars[i + 1] / 3.0f;
                dvars[i] = dvars[i] - dvars[i + 1] / 4.0;
                lvars[i] = lvars[i] >> (lvars[i + 1] & 1);
            }
            state = (iter % 11 == 0) ? 4 : 2;
            goto end_state;
        
        state_e:
            for (int i = 0; i < MANY_VARS - 1; i++) {
                vars[i] = vars[i] & vars[i + 1] ^ (i * 2);
            }
            for (int i = 0; i < 9; i++) {
                fvars[i] = fvars[i + 1] - fvars[i] * 0.8f;
                dvars[i] = dvars[i + 1] + dvars[i] * 0.9;
                lvars[i] = lvars[i] | lvars[i + 1];
            }
            state = (iter % 13 == 0) ? 5 : 3;
            goto end_state;
        
        state_f:
            for (int i = 0; i < MANY_VARS - 1; i++) {
                vars[i] = (vars[i] + vars[i + 1]) * (i + 3);
            }
            for (int i = 0; i < 9; i++) {
                fvars[i] = fvars[i] * fvars[i + 1] * 0.1f;
                dvars[i] = dvars[i] / (dvars[i + 1] + 0.5);
                lvars[i] = lvars[i] & ~lvars[i + 1];
            }
            state = (iter % 17 == 0) ? 6 : 4;
            goto end_state;
        
        state_g:
            for (int i = 0; i < MANY_VARS - 1; i++) {
                vars[i] = vars[i] - vars[i + 1] / (i + 2);
            }
            for (int i = 0; i < 9; i++) {
                fvars[i] = fvars[i + 1] / (fvars[i] + 0.1f);
                dvars[i] = dvars[i + 1] * dvars[i] * 0.01;
                lvars[i] = lvars[i] + (lvars[i + 1] << 2);
            }
            state = (iter % 19 == 0) ? 7 : 5;
            goto end_state;
        
        state_h:
            for (int i = 0; i < MANY_VARS - 1; i++) {
                vars[i] = vars[i + 1] % ((vars[i] & 0xFF) + 1);
            }
            for (int i = 0; i < 9; i++) {
                fvars[i] = (fvars[i] + fvars[i + 1]) * 0.5f;
                dvars[i] = (dvars[i] - dvars[i + 1]) * 0.5;
                lvars[i] = lvars[i] ^ (lvars[i + 1] >> 1);
            }
            state = (iter % 23 == 0) ? 0 : 6;
            goto end_state;
        
        end_state:
        /* Keep all variables alive */
        for (int i = 0; i < MANY_VARS; i++) KEEP_ALIVE(vars[i]);
        for (int i = 0; i < 10; i++) {
            KEEP_ALIVE(fvars[i]);
            KEEP_ALIVE(dvars[i]);
            KEEP_ALIVE(lvars[i]);
        }
        
        /* Update checksum */
        for (int i = 0; i < MANY_VARS; i++) checksum += vars[i];
        for (int i = 0; i < 10; i++) {
            checksum += (int)fvars[i] + (int)dvars[i] + lvars[i];
        }
        
        /* Make state transitions unpredictable */
        state = (state * 1664525 + 1013904223) & 0x7fffffff;
        state = state % 8;
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = 10000;
    int seed = 42;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10000;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF coverage test with iterations=%d, seed=%d\n", iterations, seed);
    
    uint64_t total_checksum = 0;
    
    /* Run all test functions to increase coverage chances */
    total_checksum += test_irreducible_goto(iterations, seed);
    printf("  test_irreducible_goto completed\n");
    
    total_checksum += test_switch_goto(iterations, seed + 1);
    printf("  test_switch_goto completed\n");
    
    total_checksum += test_computed_goto(iterations, seed + 2);
    printf("  test_computed_goto completed\n");
    
    printf("Total checksum: %lu\n", (unsigned long)total_checksum);
    
    return 0;
}
