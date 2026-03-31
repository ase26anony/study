/* mcf_coverage.c - Program to trigger GCC's MCF fixup graph debug output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define KEEP_ALIVE asm volatile("" : : "r"(var))

/* Complex irreducible CFG with goto jumping across loops */
NOINLINE unsigned long test_irreducible_goto(int iterations, int seed) {
    /* Many local variables to create register pressure */
    volatile int v0 = seed + 1, v1 = seed + 2, v2 = seed + 3, v3 = seed + 4;
    volatile int v4 = seed + 5, v5 = seed + 6, v6 = seed + 7, v7 = seed + 8;
    volatile int v8 = seed + 9, v9 = seed + 10, v10 = seed + 11, v11 = seed + 12;
    volatile float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f;
    volatile float f3 = seed * 0.4f, f4 = seed * 0.5f, f5 = seed * 0.6f;
    volatile double d0 = seed * 0.01, d1 = seed * 0.02, d2 = seed * 0.03;
    volatile double d3 = seed * 0.04, d4 = seed * 0.05, d5 = seed * 0.06;
    volatile long l0 = seed * 100L, l1 = seed * 200L, l2 = seed * 300L;
    volatile long l3 = seed * 400L, l4 = seed * 500L, l5 = seed * 600L;
    
    unsigned long checksum = 0;
    int i, j;
    
    /* Outer loop with label that can be jumped into from inner loop */
    outer_loop:
    for (i = 0; i < iterations; i++) {
        /* Complex arithmetic chains keeping variables live */
        v0 = v1 + v2 * v3 - v4 / (v5 + 1);
        v1 = v2 + v3 * v4 - v5 / (v6 + 1);
        v2 = v3 + v4 * v5 - v6 / (v7 + 1);
        f0 = f1 * 1.1f + f2 * 0.9f - f3 / (f4 + 0.5f);
        f1 = f2 * 1.2f + f3 * 0.8f - f4 / (f5 + 0.5f);
        d0 = d1 * 1.01 + d2 * 0.99 - d3 / (d4 + 0.1);
        d1 = d2 * 1.02 + d3 * 0.98 - d4 / (d5 + 0.1);
        l0 = l1 * 3 + l2 / 2 - l3 * 4 + l4;
        l1 = l2 * 4 + l3 / 3 - l4 * 5 + l5;
        
        /* Inner loop with goto that jumps to outer loop label */
        for (j = 0; j < 10; j++) {
            /* More arithmetic operations */
            v3 = v4 + v5 * v6 - v7 / (v8 + 1);
            v4 = v5 + v6 * v7 - v8 / (v9 + 1);
            f2 = f3 * 1.3f + f4 * 0.7f - f5 / (f0 + 0.5f);
            d2 = d3 * 1.03 + d4 * 0.97 - d5 / (d0 + 0.1);
            l2 = l3 * 5 + l4 / 4 - l5 * 6 + l0;
            
            /* Irreducible control flow: jump to outer loop from inner loop */
            if ((i * j + seed) % 37 == 0) {
                checksum += v0 + v1 + v2 + (int)f0 + (int)d0 + l0;
                goto outer_loop;  /* Creates irreducible region */
            }
            
            if ((i * j + seed) % 41 == 0) {
                checksum += v3 + v4 + v5 + (int)f1 + (int)d1 + l1;
                goto middle_label;  /* Jump to label between loops */
            }
        }
        
        continue;  /* Normal loop continuation */
        
        middle_label:
        /* Code block between loops that's reachable via goto */
        v5 = v6 + v7 * v8 - v9 / (v10 + 1);
        v6 = v7 + v8 * v9 - v10 / (v11 + 1);
        f3 = f4 * 1.4f + f5 * 0.6f - f0 / (f1 + 0.5f);
        d3 = d4 * 1.04 + d5 * 0.96 - d0 / (d1 + 0.1);
        l3 = l4 * 6 + l5 / 5 - l0 * 7 + l1;
        checksum += v6 + v7 + (int)f2 + (int)d2 + l2;
    }
    
    /* Aggregate results from all variables */
    checksum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11;
    checksum += (int)f0 + (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
    checksum += (int)d0 + (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5;
    checksum += l0 + l1 + l2 + l3 + l4 + l5;
    
    return checksum;
}

/* Switch statement with goto creating complex CFG */
NOINLINE unsigned long test_switch_goto(int iterations, int seed) {
    volatile int vars[MANY_VARS];
    volatile float fvars[MANY_VARS/2];
    volatile double dvars[MANY_VARS/2];
    int i, state = seed;
    unsigned long checksum = 0;
    
    /* Initialize many variables */
    for (i = 0; i < MANY_VARS; i++) vars[i] = seed + i;
    for (i = 0; i < MANY_VARS/2; i++) fvars[i] = seed * (i + 1) * 0.1f;
    for (i = 0; i < MANY_VARS/2; i++) dvars[i] = seed * (i + 1) * 0.01;
    
    for (i = 0; i < iterations; i++) {
        /* Long arithmetic chains */
        vars[0] = vars[1] * vars[2] - vars[3] + vars[4] / (vars[5] + 1);
        vars[1] = vars[2] * vars[3] - vars[4] + vars[5] / (vars[6] + 1);
        vars[2] = vars[3] * vars[4] - vars[5] + vars[6] / (vars[7] + 1);
        fvars[0] = fvars[1] * 1.5f - fvars[2] + fvars[3] / (fvars[4] + 0.1f);
        dvars[0] = dvars[1] * 1.05 - dvars[2] + dvars[3] / (dvars[4] + 0.01);
        
        /* Switch with goto to different labels */
        switch (state % 7) {
            case 0:
                vars[3] = vars[4] + vars[5] * 2;
                goto label_a;
            case 1:
                vars[4] = vars[5] + vars[6] * 3;
                goto label_b;
            case 2:
                vars[5] = vars[6] + vars[7] * 4;
                goto label_c;
            case 3:
                vars[6] = vars[7] + vars[8] * 5;
                goto label_a;
            case 4:
                vars[7] = vars[8] + vars[9] * 6;
                goto label_b;
            case 5:
                vars[8] = vars[9] + vars[10] * 7;
                goto label_c;
            case 6:
                vars[9] = vars[10] + vars[11] * 8;
                /* Fall through */
        }
        
        /* Default continuation */
        vars[10] = vars[11] + vars[12] * 9;
        continue;
        
        label_a:
        vars[11] = vars[12] + vars[13] * 10;
        fvars[1] = fvars[2] * 2.0f - fvars[3];
        checksum += vars[0] + vars[1];
        continue;
        
        label_b:
        vars[12] = vars[13] + vars[14] * 11;
        dvars[1] = dvars[2] * 1.1 - dvars[3];
        checksum += vars[2] + vars[3];
        continue;
        
        label_c:
        vars[13] = vars[14] + vars[15] * 12;
        fvars[2] = fvars[3] * 2.5f - fvars[4];
        checksum += vars[4] + vars[5];
        /* Jump back to switch or continue */
        if (i % 3 == 0) {
            state = (state * 1103515245 + 12345) & 0x7fffffff;
            continue;
        }
    }
    
    /* Aggregate all variables */
    for (i = 0; i < MANY_VARS; i++) checksum += vars[i];
    for (i = 0; i < MANY_VARS/2; i++) checksum += (int)fvars[i];
    for (i = 0; i < MANY_VARS/2; i++) checksum += (int)dvars[i];
    
    return checksum;
}

/* Computed goto state machine */
NOINLINE unsigned long test_computed_goto(int iterations, int seed) {
    volatile int a = seed, b = seed * 2, c = seed * 3, d = seed * 4;
    volatile int e = seed * 5, f = seed * 6, g = seed * 7, h = seed * 8;
    volatile float fa = seed * 0.1f, fb = seed * 0.2f, fc = seed * 0.3f;
    volatile double da = seed * 0.01, db = seed * 0.02, dc = seed * 0.03;
    volatile long la = seed * 1000L, lb = seed * 2000L, lc = seed * 3000L;
    
    unsigned long checksum = 0;
    int i, state = 0;
    
    /* Label array for computed goto */
    static void* states[] = { &&state0, &&state1, &&state2, &&state3, 
                            &&state4, &&state5, &&state6, &&state7 };
    
    for (i = 0; i < iterations; i++) {
        /* Complex arithmetic before goto */
        a = b + c * d - e / (f + 1);
        b = c + d * e - f / (g + 1);
        c = d + e * f - g / (h + 1);
        fa = fb * 1.1f + fc * 0.9f - fa / (fb + 0.5f);
        da = db * 1.01 + dc * 0.99 - da / (db + 0.1);
        la = lb * 3 + lc / 2 - la * 4 + lb;
        
        /* Computed goto - creates very complex CFG */
        goto *states[state % 8];
        
        state0:
            d = e + f * g - h / (a + 1);
            fb = fc * 1.2f + fa * 0.8f - fb / (fc + 0.5f);
            state = (state + 1) % 8;
            checksum += a + b;
            continue;
            
        state1:
            e = f + g * h - a / (b + 1);
            db = dc * 1.02 + da * 0.98 - db / (dc + 0.1);
            state = (state * 3 + 1) % 8;
            checksum += c + d;
            continue;
            
        state2:
            f = g + h * a - b / (c + 1);
            fc = fa * 1.3f + fb * 0.7f - fc / (fa + 0.5f);
            state = (state * 5 + 2) % 8;
            checksum += e + f;
            continue;
            
        state3:
            g = h + a * b - c / (d + 1);
            dc = da * 1.03 + db * 0.97 - dc / (da + 0.1);
            state = (state * 7 + 3) % 8;
            checksum += g + h;
            continue;
            
        state4:
            h = a + b * c - d / (e + 1);
            lb = lc * 5 + la / 4 - lb * 6 + lc;
            state = (state * 11 + 5) % 8;
            checksum += (int)fa + (int)fb;
            continue;
            
        state5:
            a = b + c * d - e / (f + 1);
            lc = la * 6 + lb / 5 - lc * 7 + la;
            state = (state * 13 + 7) % 8;
            checksum += (int)fc + (int)da;
            continue;
            
        state6:
            b = c + d * e - f / (g + 1);
            la = lb * 7 + lc / 6 - la * 8 + lb;
            state = (state * 17 + 11) % 8;
            checksum += (int)db + (int)dc;
            continue;
            
        state7:
            c = d + e * f - g / (h + 1);
            checksum += la + lb + lc;
            state = (state * 19 + 13) % 8;
            /* Fall through to next iteration */
    }
    
    checksum += a + b + c + d + e + f + g + h;
    checksum += (int)fa + (int)fb + (int)fc;
    checksum += (int)da + (int)db + (int)dc;
    checksum += la + lb + lc;
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = 10000;
    int seed = time(NULL);
    unsigned long total_checksum = 0;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10000;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF coverage test with iterations=%d, seed=%d\n", 
           iterations, seed);
    
    /* Run all test functions with different CFG patterns */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    printf("Final checksum: %lu\n", total_checksum);
    
    return 0;
}
