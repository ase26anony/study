/* mcf_coverage.c - Complex CFG with high register pressure to trigger MCF fixup graph */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MANY_VARS 30
#define ITERATIONS 10000

/* Prevent optimization */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(var))
#define NOINLINE __attribute__((noinline))

/* Complex irreducible CFG with goto jumping across loops */
NOINLINE static unsigned long test_irreducible_goto(int n, unsigned seed) {
    /* Many local variables to create register pressure */
    volatile int a0 = seed + 1, a1 = seed + 2, a2 = seed + 3, a3 = seed + 4;
    volatile int a4 = seed + 5, a5 = seed + 6, a6 = seed + 7, a7 = seed + 8;
    volatile int a8 = seed + 9, a9 = seed + 10, a10 = seed + 11, a11 = seed + 12;
    volatile float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f, f3 = seed * 0.4f;
    volatile double d0 = seed * 0.01, d1 = seed * 0.02, d2 = seed * 0.03, d3 = seed * 0.04;
    volatile long l0 = seed * 100, l1 = seed * 200, l2 = seed * 300, l3 = seed * 400;
    volatile int b0 = 0, b1 = 0, b2 = 0, b3 = 0, b4 = 0, b5 = 0;
    
    unsigned long checksum = 0;
    int i;
    
    /* Outer loop with label that inner loop can jump to */
outer_loop_start:
    for (i = 0; i < n; i++) {
        /* Complex arithmetic chains to keep variables live */
        a0 = a1 + a2; a1 = a3 * a4; a2 = a5 - a6; a3 = a7 / (a8 + 1);
        a4 = a9 ^ a10; a5 = a11 | a0; a6 = a1 & a2; a7 = a3 % (a4 + 1);
        f0 = f1 * f2 + f3; f1 = f2 - f3 * f0; f2 = f3 / (f0 + 0.1f); f3 = f0 + f1 - f2;
        d0 = d1 * 1.1 + d2; d1 = d2 * 0.9 - d3; d2 = d3 / (d0 + 0.01); d3 = d0 * d1 + d2;
        l0 = l1 << 2; l1 = l2 >> 1; l2 = l3 ^ l0; l3 = l1 | l2;
        
        /* Inner loop with goto that jumps to outer loop */
        for (int j = 0; j < 5; j++) {
            b0 = a0 + j; b1 = a1 - j; b2 = a2 * j; b3 = a3 ^ j;
            
            /* Irreducible: jump from inner loop to outer loop label */
            if ((i * j) % 37 == 0) {
                KEEP_ALIVE(a0); KEEP_ALIVE(f0); KEEP_ALIVE(d0); KEEP_ALIVE(l0);
                goto outer_loop_mid;  /* Creates irreducible region */
            }
            
            b4 = b0 + b1; b5 = b2 * b3;
            if ((i + j) % 23 == 0) {
                goto inner_loop_exit;  /* Another jump */
            }
        }
        continue;
        
inner_loop_exit:
        a8 = a9 + a10; a9 = a11 * i; a10 = a0 ^ i;
        if (i % 7 == 0) {
            goto outer_loop_end;  /* Jump to outer loop end */
        }
    }
    goto function_end;
    
outer_loop_mid:
    a11 = a0 * a1; a0 = a2 + a3; a1 = a4 - a5;
    if (i % 11 == 0) {
        goto outer_loop_start;  /* Jump back to start */
    }
    goto outer_loop_end;
    
outer_loop_end:
    f0 = f1 + f2; f1 = f3 * 2.0f; f2 = f0 / 3.0f;
    if (i % 13 == 0) {
        goto outer_loop_start;  /* Another jump creating complexity */
    }
    
function_end:
    /* Aggregate checksum from all variables */
    checksum = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11;
    checksum += (unsigned long)f0 + (unsigned long)f1 + (unsigned long)f2 + (unsigned long)f3;
    checksum += (unsigned long)d0 + (unsigned long)d1 + (unsigned long)d2 + (unsigned long)d3;
    checksum += l0 + l1 + l2 + l3 + b0 + b1 + b2 + b3 + b4 + b5;
    
    return checksum;
}

/* Switch with goto creating complex CFG */
NOINLINE static unsigned long test_switch_goto(int n, unsigned seed) {
    volatile int v0 = seed, v1 = seed * 2, v2 = seed * 3, v3 = seed * 4;
    volatile int v4 = seed * 5, v5 = seed * 6, v6 = seed * 7, v7 = seed * 8;
    volatile float fv0 = seed * 0.11f, fv1 = seed * 0.22f, fv2 = seed * 0.33f;
    volatile double dv0 = seed * 0.055, dv1 = seed * 0.066, dv2 = seed * 0.077;
    volatile long lv0 = seed * 111, lv1 = seed * 222, lv2 = seed * 333;
    
    unsigned long checksum = 0;
    int state = 0;
    
    for (int i = 0; i < n; i++) {
        /* Long dependency chains */
        v0 = v1 + v2; v1 = v3 * v4; v2 = v5 - v6; v3 = v7 ^ v0;
        v4 = v0 & v1; v5 = v2 | v3; v6 = v4 % (v5 + 1); v7 = v6 * 2;
        fv0 = fv1 * 1.5f + fv2; fv1 = fv2 - fv0 * 0.5f; fv2 = fv0 / (fv1 + 0.1f);
        dv0 = dv1 * 1.01 + dv2; dv1 = dv2 - dv0 * 0.99; dv2 = dv0 / (dv1 + 0.01);
        lv0 = lv1 << (i % 3); lv1 = lv2 >> 1; lv2 = lv0 ^ lv1;
        
        /* Switch with goto to different labels */
        switch (state % 5) {
            case 0:
                v0 = v1 + i;
                if (i % 3 == 0) goto label_a;
                break;
            case 1:
                v1 = v2 * i;
                if (i % 5 == 0) goto label_b;
                break;
            case 2:
                v2 = v3 - i;
                if (i % 7 == 0) goto label_c;
                break;
            case 3:
                v3 = v4 ^ i;
                if (i % 11 == 0) goto label_a;
                break;
            case 4:
                v4 = v5 | i;
                if (i % 13 == 0) goto label_b;
                break;
        }
        
        state = (state * 13 + i) % 100;
        continue;
        
    label_a:
        v5 = v6 + v7; v6 = v0 * v1;
        KEEP_ALIVE(v0); KEEP_ALIVE(fv0); KEEP_ALIVE(dv0); KEEP_ALIVE(lv0);
        if (i % 17 == 0) goto label_c;
        continue;
        
    label_b:
        v7 = v0 ^ v1; v0 = v2 & v3;
        if (i % 19 == 0) goto label_a;
        continue;
        
    label_c:
        v1 = v2 | v3; v2 = v4 + v5;
        if (i % 23 == 0) goto label_b;
        continue;
    }
    
    checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
    checksum += (unsigned long)fv0 + (unsigned long)fv1 + (unsigned long)fv2;
    checksum += (unsigned long)dv0 + (unsigned long)dv1 + (unsigned long)dv2;
    checksum += lv0 + lv1 + lv2;
    
    return checksum;
}

/* Computed goto state machine */
NOINLINE static unsigned long test_computed_goto(int n, unsigned seed) {
    volatile int s0 = seed, s1 = seed + 11, s2 = seed + 22, s3 = seed + 33;
    volatile int s4 = seed + 44, s5 = seed + 55, s6 = seed + 66, s7 = seed + 77;
    volatile float fs0 = seed * 0.123f, fs1 = seed * 0.234f, fs2 = seed * 0.345f;
    volatile double ds0 = seed * 0.0123, ds1 = seed * 0.0234, ds2 = seed * 0.0345;
    volatile long ls0 = seed * 1234, ls1 = seed * 2345, ls2 = seed * 3456;
    
    /* Labels for computed goto */
    static void* labels[] = { &&state0, &&state1, &&state2, &&state3, &&state4, &&state_end };
    
    unsigned long checksum = 0;
    int state = 0;
    
    for (int i = 0; i < n; i++) {
        /* Heavy arithmetic */
        s0 = s1 + s2; s1 = s3 * s4; s2 = s5 - s6; s3 = s7 ^ s0;
        s4 = s0 & s1; s5 = s2 | s3; s6 = s4 % (s5 + 1); s7 = s6 * 3;
        fs0 = fs1 * 2.0f + fs2; fs1 = fs2 - fs0 * 0.3f; fs2 = fs0 / (fs1 + 0.01f);
        ds0 = ds1 * 1.11 + ds2; ds1 = ds2 - ds0 * 0.89; ds2 = ds0 / (ds1 + 0.001);
        ls0 = ls1 << 1; ls1 = ls2 >> 2; ls2 = ls0 ^ ls1;
        
        /* Computed goto - creates complex CFG */
        goto *labels[state % 6];
        
    state0:
        s0 = s1 + i;
        state = (state + 1) % 6;
        KEEP_ALIVE(s0); KEEP_ALIVE(fs0); KEEP_ALIVE(ds0); KEEP_ALIVE(ls0);
        if (i % 4 == 0) goto *labels[2];
        continue;
        
    state1:
        s1 = s2 * i;
        state = (state * 7 + 1) % 6;
        if (i % 6 == 0) goto *labels[3];
        continue;
        
    state2:
        s2 = s3 - i;
        state = (state * 5 + 2) % 6;
        if (i % 8 == 0) goto *labels[4];
        continue;
        
    state3:
        s3 = s4 ^ i;
        state = (state * 3 + 3) % 6;
        if (i % 10 == 0) goto *labels[0];
        continue;
        
    state4:
        s4 = s5 | i;
        state = (state * 11 + 4) % 6;
        if (i % 12 == 0) goto *labels[1];
        continue;
        
    state_end:
        s5 = s6 + s7;
        state = 0;
        if (i % 20 == 0) goto *labels[0];
        continue;
    }
    
    checksum = s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7;
    checksum += (unsigned long)fs0 + (unsigned long)fs1 + (unsigned long)fs2;
    checksum += (unsigned long)ds0 + (unsigned long)ds1 + (unsigned long)ds2;
    checksum += ls0 + ls1 + ls2;
    
    return checksum;
}

int main(int argc, char** argv) {
    int iterations = ITERATIONS;
    unsigned seed = (unsigned)time(NULL);
    unsigned long total_checksum = 0;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = ITERATIONS;
    }
    
    printf("Running MCF coverage test with %d iterations, seed=%u\n", 
           iterations, seed);
    
    /* Run all test functions to increase coverage chance */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    printf("Total checksum: %lu\n", total_checksum);
    
    return 0;
}
