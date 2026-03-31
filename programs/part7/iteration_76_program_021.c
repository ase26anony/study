/* mcf_coverage.c - Program to trigger MCF fixup graph debug output */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define KEEP_ALIVE(var) asm volatile("" : : "r"(var))

/* Function 1: Irreducible loops with goto jumping across boundaries */
NOINLINE static unsigned long test_irreducible_goto(int iterations, int seed) {
    /* Declare many variables to increase register pressure */
    volatile int v0 = seed + 1;
    volatile int v1 = seed + 2;
    volatile int v2 = seed + 3;
    volatile int v3 = seed + 4;
    volatile int v4 = seed + 5;
    volatile int v5 = seed + 6;
    volatile int v6 = seed + 7;
    volatile int v7 = seed + 8;
    volatile int v8 = seed + 9;
    volatile int v9 = seed + 10;
    
    volatile float f0 = seed * 0.1f;
    volatile float f1 = seed * 0.2f;
    volatile float f2 = seed * 0.3f;
    volatile float f3 = seed * 0.4f;
    volatile float f4 = seed * 0.5f;
    
    volatile double d0 = seed * 0.01;
    volatile double d1 = seed * 0.02;
    volatile double d2 = seed * 0.03;
    volatile double d3 = seed * 0.04;
    volatile double d4 = seed * 0.05;
    
    volatile long l0 = seed * 100;
    volatile long l1 = seed * 200;
    volatile long l2 = seed * 300;
    volatile long l3 = seed * 400;
    volatile long l4 = seed * 500;
    volatile long l5 = seed * 600;
    volatile long l6 = seed * 700;
    volatile long l7 = seed * 800;
    volatile long l8 = seed * 900;
    volatile long l9 = seed * 1000;
    
    unsigned long checksum = 0;
    int i;
    
    /* Create irreducible loop structure with goto */
    for (i = 0; i < iterations; i++) {
        int mod = i % 7;
        
        /* Label definitions creating multiple entry points */
        loop_start:
        if (mod == 0) {
            /* Complex arithmetic to keep variables live */
            v0 = v1 + v2 * v3 - v4 / (v5 + 1);
            f0 = f1 * f2 + f3 - f4;
            d0 = d1 + d2 * d3 - d4;
            l0 = l1 + l2 * l3 - l4;
            goto middle_of_loop;
        }
        
        if (mod == 1) {
            v1 = v2 + v3 * v4 - v5 / (v6 + 1);
            f1 = f2 * f3 + f4 - f0;
            d1 = d2 + d3 * d4 - d0;
            l1 = l2 + l3 * l4 - l5;
            goto inner_loop;
        }
        
        middle_of_loop:
        if (mod == 2) {
            v2 = v3 + v4 * v5 - v6 / (v7 + 1);
            f2 = f3 * f4 + f0 - f1;
            d2 = d3 + d4 * d0 - d1;
            l2 = l3 + l4 * l5 - l6;
            goto loop_end;
        }
        
        inner_loop:
        if (mod == 3) {
            v3 = v4 + v5 * v6 - v7 / (v8 + 1);
            f3 = f4 * f0 + f1 - f2;
            d3 = d4 + d0 * d1 - d2;
            l3 = l4 + l5 * l6 - l7;
            goto loop_start;  /* Jump back to start - creates irreducible region */
        }
        
        if (mod == 4) {
            v4 = v5 + v6 * v7 - v8 / (v9 + 1);
            f4 = f0 * f1 + f2 - f3;
            d4 = d0 + d1 * d2 - d3;
            l4 = l5 + l6 * l7 - l8;
            goto middle_of_loop;
        }
        
        if (mod == 5) {
            v5 = v6 + v7 * v8 - v9 / (v0 + 1);
            f0 = f1 * f2 + f3 - f4;  /* Reuse variables */
            d0 = d1 + d2 * d3 - d4;
            l5 = l6 + l7 * l8 - l9;
            goto inner_loop;
        }
        
        loop_end:
        if (mod == 6) {
            v6 = v7 + v8 * v9 - v0 / (v1 + 1);
            f1 = f2 * f3 + f4 - f0;
            d1 = d2 + d3 * d4 - d0;
            l6 = l7 + l8 * l9 - l0;
        }
        
        /* More arithmetic to create long dependency chains */
        v7 = v8 + v9 * v0 - v1 / (v2 + 1);
        v8 = v9 + v0 * v1 - v2 / (v3 + 1);
        v9 = v0 + v1 * v2 - v3 / (v4 + 1);
        
        f2 = f3 * f4 + f0 - f1;
        f3 = f4 * f0 + f1 - f2;
        f4 = f0 * f1 + f2 - f3;
        
        d2 = d3 + d4 * d0 - d1;
        d3 = d4 + d0 * d1 - d2;
        d4 = d0 + d1 * d2 - d3;
        
        l7 = l8 + l9 * l0 - l1;
        l8 = l9 + l0 * l1 - l2;
        l9 = l0 + l1 * l2 - l3;
        
        /* Update checksum with all variables */
        checksum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        checksum += (unsigned long)(f0 + f1 + f2 + f3 + f4);
        checksum += (unsigned long)(d0 + d1 + d2 + d3 + d4);
        checksum += l0 + l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9;
        
        /* Mark variables as used to prevent optimization */
        KEEP_ALIVE(v0); KEEP_ALIVE(v1); KEEP_ALIVE(v2); KEEP_ALIVE(v3); KEEP_ALIVE(v4);
        KEEP_ALIVE(v5); KEEP_ALIVE(v6); KEEP_ALIVE(v7); KEEP_ALIVE(v8); KEEP_ALIVE(v9);
        KEEP_ALIVE(f0); KEEP_ALIVE(f1); KEEP_ALIVE(f2); KEEP_ALIVE(f3); KEEP_ALIVE(f4);
        KEEP_ALIVE(d0); KEEP_ALIVE(d1); KEEP_ALIVE(d2); KEEP_ALIVE(d3); KEEP_ALIVE(d4);
        KEEP_ALIVE(l0); KEEP_ALIVE(l1); KEEP_ALIVE(l2); KEEP_ALIVE(l3); KEEP_ALIVE(l4);
        KEEP_ALIVE(l5); KEEP_ALIVE(l6); KEEP_ALIVE(l7); KEEP_ALIVE(l8); KEEP_ALIVE(l9);
    }
    
    return checksum;
}

/* Function 2: Switch with goto creating complex CFG */
NOINLINE static unsigned long test_switch_goto(int iterations, int seed) {
    volatile int vars[MANY_VARS];
    volatile float fvars[MANY_VARS];
    volatile double dvars[MANY_VARS];
    unsigned long checksum = 0;
    int i, j;
    
    /* Initialize arrays with different values */
    for (j = 0; j < MANY_VARS; j++) {
        vars[j] = seed + j * 3;
        fvars[j] = seed * 0.1f + j * 0.3f;
        dvars[j] = seed * 0.01 + j * 0.03;
    }
    
    for (i = 0; i < iterations; i++) {
        int state = i % 11;
        
        switch (state) {
            case 0:
                for (j = 0; j < MANY_VARS - 1; j++) {
                    vars[j] = vars[j+1] * 2 - vars[j];
                    fvars[j] = fvars[j+1] * 1.5f - fvars[j];
                    dvars[j] = dvars[j+1] * 1.7 - dvars[j];
                }
                goto update_checksum;  /* Jump out of switch */
                
            case 1:
                for (j = MANY_VARS - 1; j > 0; j--) {
                    vars[j] = vars[j-1] * 3 + vars[j];
                    fvars[j] = fvars[j-1] * 2.1f + fvars[j];
                    dvars[j] = dvars[j-1] * 2.3 + dvars[j];
                }
                goto alternate_path;
                
            case 2:
                vars[0] = vars[1] + vars[2] * vars[3];
                fvars[0] = fvars[1] + fvars[2] * fvars[3];
                dvars[0] = dvars[1] + dvars[2] * dvars[3];
                goto middle_case;
                
            case 3:
            middle_case:
                vars[4] = vars[5] - vars[6] / (vars[7] + 1);
                fvars[4] = fvars[5] - fvars[6] / (fvars[7] + 1.0f);
                dvars[4] = dvars[5] - dvars[6] / (dvars[7] + 1.0);
                goto case_4;
                
            case 4:
            case_4:
                for (j = 8; j < 15; j++) {
                    vars[j] = vars[j-1] + vars[j-2] - vars[j-3];
                }
                goto update_checksum;
                
            case 5:
            alternate_path:
                vars[10] = vars[11] * vars[12] >> 1;
                goto case_6;
                
            case 6:
            case_6:
                for (j = 12; j < 20; j++) {
                    fvars[j] = fvars[j-1] * 0.9f + fvars[j-2] * 0.1f;
                }
                goto switch_end;
                
            case 7:
                dvars[15] = dvars[16] * 0.8 + dvars[17] * 0.2;
                goto middle_case;  /* Jump back into middle of switch */
                
            case 8:
                vars[18] = vars[19] ^ vars[0];
                goto alternate_path;
                
            case 9:
                for (j = 0; j < MANY_VARS; j += 2) {
                    vars[j] = vars[j] * 2 + 1;
                }
                goto update_checksum;
                
            case 10:
            switch_end:
                vars[20] = vars[21] | vars[22];
                break;
        }
        
        update_checksum:
        for (j = 0; j < MANY_VARS; j++) {
            checksum += vars[j] + (unsigned long)fvars[j] + (unsigned long)dvars[j];
            KEEP_ALIVE(vars[j]);
            KEEP_ALIVE(fvars[j]);
            KEEP_ALIVE(dvars[j]);
        }
    }
    
    return checksum;
}

/* Function 3: Computed goto simulating state machine */
NOINLINE static unsigned long test_computed_goto(int iterations, int seed) {
    static void* labels[] = {
        &&state_a, &&state_b, &&state_c, 
        &&state_d, &&state_e, &&state_f,
        &&state_g, &&state_h, &&state_i
    };
    
    volatile int a = seed, b = seed * 2, c = seed * 3, d = seed * 4;
    volatile int e = seed * 5, f = seed * 6, g = seed * 7, h = seed * 8;
    volatile int i = seed * 9, j = seed * 10, k = seed * 11, l = seed * 12;
    volatile int m = seed * 13, n = seed * 14, o = seed * 15, p = seed * 16;
    volatile int q = seed * 17, r = seed * 18, s = seed * 19, t = seed * 20;
    
    volatile double da = seed * 0.1, db = seed * 0.2, dc = seed * 0.3;
    volatile double dd = seed * 0.4, de = seed * 0.5, df = seed * 0.6;
    
    unsigned long checksum = 0;
    int state = 0;
    int step;
    
    for (step = 0; step < iterations; step++) {
        /* Jump to current state */
        goto *labels[state];
        
        state_a:
            a = b + c - d * e / (f + 1);
            da = db + dc - dd;
            state = (state + 1) % 9;
            goto next_step;
            
        state_b:
            b = c + d - e * f / (g + 1);
            db = dc + dd - de;
            state = (state + 2) % 9;
            goto next_step;
            
        state_c:
            c = d + e - f * g / (h + 1);
            dc = dd + de - df;
            state = (state + 3) % 9;
            goto state_d;  /* Direct jump to another state */
            
        state_d:
            d = e + f - g * h / (i + 1);
            dd = de + df - da;
            state = (state + 4) % 9;
            goto next_step;
            
        state_e:
            e = f + g - h * i / (j + 1);
            de = df + da - db;
            state = (state + 5) % 9;
            goto state_a;  /* Jump back */
            
        state_f:
            f = g + h - i * j / (k + 1);
            df = da + db - dc;
            state = (state + 6) % 9;
            goto next_step;
            
        state_g:
            g = h + i - j * k / (l + 1);
            da = db + dc - dd;
            state = (state + 7) % 9;
            goto state_c;
            
        state_h:
            h = i + j - k * l / (m + 1);
            db = dc + dd - de;
            state = (state + 8) % 9;
            goto next_step;
            
        state_i:
            i = j + k - l * m / (n + 1);
            dc = dd + de - df;
            state = 0;
            goto next_step;
            
        next_step:
            /* Long dependency chain */
            j = k + l - m * n / (o + 1);
            k = l + m - n * o / (p + 1);
            l = m + n - o * p / (q + 1);
            m = n + o - p * q / (r + 1);
            n = o + p - q * r / (s + 1);
            o = p + q - r * s / (t + 1);
            
            dd = de + df - da;
            de = df + da - db;
            df = da + db - dc;
            
            /* Update checksum */
            checksum += a + b + c + d + e + f + g + h + i + j + k + l + m + n + o;
            checksum += (unsigned long)(da + db + dc + dd + de + df);
            
            /* Keep all variables alive */
            KEEP_ALIVE(a); KEEP_ALIVE(b); KEEP_ALIVE(c); KEEP_ALIVE(d);
            KEEP_ALIVE(e); KEEP_ALIVE(f); KEEP_ALIVE(g); KEEP_ALIVE(h);
            KEEP_ALIVE(i); KEEP_ALIVE(j); KEEP_ALIVE(k); KEEP_ALIVE(l);
            KEEP_ALIVE(m); KEEP_ALIVE(n); KEEP_ALIVE(o); KEEP_ALIVE(p);
            KEEP_ALIVE(q); KEEP_ALIVE(r); KEEP_ALIVE(s); KEEP_ALIVE(t);
            KEEP_ALIVE(da); KEEP_ALIVE(db); KEEP_ALIVE(dc);
            KEEP_ALIVE(dd); KEEP_ALIVE(de); KEEP_ALIVE(df);
    }
    
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
    
    printf("Running MCF coverage test with %d iterations, seed=%d\n", 
           iterations, seed);
    
    /* Run all test functions to trigger different CFG patterns */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    printf("Final checksum: %lu\n", total_checksum);
    
    return 0;
}
