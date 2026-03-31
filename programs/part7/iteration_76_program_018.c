/* mcf_coverage.c - Program to trigger MCF fixup graph debug output */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define KEEP_ALIVE(var) asm volatile("" : : "r"(var))

/* Function 1: Irreducible loops with goto jumping across boundaries */
NOINLINE static unsigned long test_irreducible_goto(int iterations, int seed) {
    /* Many local variables to create register pressure */
    volatile int a0 = seed + 1, a1 = seed + 2, a2 = seed + 3, a3 = seed + 4;
    volatile int b0 = seed + 5, b1 = seed + 6, b2 = seed + 7, b3 = seed + 8;
    volatile int c0 = seed + 9, c1 = seed + 10, c2 = seed + 11, c3 = seed + 12;
    volatile float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f;
    volatile double d0 = seed * 0.01, d1 = seed * 0.02, d2 = seed * 0.03;
    volatile long l0 = seed * 100L, l1 = seed * 200L, l2 = seed * 300L;
    volatile int x0, x1, x2, x3, x4, x5, x6, x7, x8, x9;
    
    unsigned long checksum = 0;
    int i;
    
    /* Complex irreducible loop structure */
    for (i = 0; i < iterations; i++) {
        /* Label definitions for goto targets */
        loop_start:
        if (i % 3 == 0) {
            /* Jump into middle of inner block */
            goto inner_block;
        }
        
        outer_block:
        a0 = a1 + a2 * a3;
        b0 = b1 - b2 / (b3 + 1);
        f0 = f1 * f2 + (float)a0;
        d0 = d1 - d2 * 0.5;
        l0 = l1 ^ l2;
        KEEP_ALIVE(a0); KEEP_ALIVE(b0); KEEP_ALIVE(f0); KEEP_ALIVE(d0); KEEP_ALIVE(l0);
        
        if (i % 5 == 0) {
            goto exit_path;
        }
        
        inner_block:
        a1 = a0 * a2 - a3;
        b1 = b0 + b2 * b3;
        f1 = f0 / f2 + 1.0f;
        d1 = d0 + d2 * 2.0;
        l1 = l0 | l2;
        KEEP_ALIVE(a1); KEEP_ALIVE(b1); KEEP_ALIVE(f1); KEEP_ALIVE(d1); KEEP_ALIVE(l1);
        
        if (i % 7 == 0) {
            goto outer_block;
        }
        
        middle_block:
        a2 = a1 ^ a0;
        b2 = b1 & b3;
        f2 = f1 - f0;
        d2 = d1 / (d0 + 0.001);
        l2 = l1 + l0;
        KEEP_ALIVE(a2); KEEP_ALIVE(b2); KEEP_ALIVE(f2); KEEP_ALIVE(d2); KEEP_ALIVE(l2);
        
        if (i % 11 == 0) {
            goto loop_start;
        }
        
        exit_path:
        x0 = a0 + b0 + c0;
        x1 = a1 + b1 + c1;
        x2 = a2 + b2 + c2;
        x3 = a3 + b3 + c3;
        checksum += x0 + x1 + x2 + x3 + (int)f0 + (int)d0 + l0;
        
        /* Jump back to create irreducible region */
        if (i % 13 == 0) {
            goto middle_block;
        }
    }
    
    return checksum;
}

/* Function 2: Switch with goto creating complex CFG */
NOINLINE static unsigned long test_switch_goto(int iterations, int seed) {
    volatile int v[MANY_VARS];
    volatile float fv[MANY_VARS/2];
    volatile double dv[MANY_VARS/3];
    int i, j;
    unsigned long checksum = 0;
    
    /* Initialize many variables */
    for (j = 0; j < MANY_VARS; j++) {
        v[j] = seed + j * 3;
        if (j < MANY_VARS/2) fv[j] = seed * 0.1f + j;
        if (j < MANY_VARS/3) dv[j] = seed * 0.01 + j * 0.5;
    }
    
    for (i = 0; i < iterations; i++) {
        int state = (i + seed) % 8;
        
        switch (state) {
            case 0:
                v[0] = v[1] + v[2] * v[3];
                v[4] = v[5] - v[6] / (v[7] + 1);
                fv[0] = fv[1] * fv[2] + (float)v[0];
                dv[0] = dv[1] - dv[2];
                goto label_a;
                
            case 1:
                v[1] = v[0] * v[2] - v[3];
                v[5] = v[4] + v[6] * v[7];
                fv[1] = fv[0] / fv[2] + 1.0f;
                dv[1] = dv[0] + dv[2] * 2.0;
                goto label_c;
                
            case 2:
            label_a:
                v[2] = v[1] ^ v[0];
                v[6] = v[5] & v[7];
                fv[2] = fv[1] - fv[0];
                dv[2] = dv[1] / (dv[0] + 0.001);
                goto label_b;
                
            case 3:
                v[3] = v[2] | v[1];
                v[7] = v[6] ^ v[5];
                fv[3] = fv[2] * fv[1];
                dv[3] = dv[2] - dv[1];
                /* Fall through */
                
            case 4:
            label_b:
                v[8] = v[0] + v[1] + v[2] + v[3];
                v[9] = v[4] + v[5] + v[6] + v[7];
                fv[4] = fv[0] + fv[1] + fv[2] + fv[3];
                dv[4] = dv[0] + dv[1] + dv[2] + dv[3];
                goto label_d;
                
            case 5:
            label_c:
                v[10] = v[0] * v[1] - v[2];
                v[11] = v[3] * v[4] / (v[5] + 1);
                fv[5] = fv[1] * fv[3] - fv[2];
                dv[5] = dv[1] * dv[3] / (dv[2] + 0.1);
                /* Fall through */
                
            case 6:
                v[12] = v[6] + v[7] + v[8];
                v[13] = v[9] + v[10] + v[11];
                fv[6] = fv[4] + fv[5];
                dv[6] = dv[4] + dv[5];
                goto label_e;
                
            case 7:
            label_d:
                v[14] = v[8] * v[9] - v[10];
                v[15] = v[11] * v[12] / (v[13] + 1);
                fv[7] = fv[5] * fv[6] - fv[4];
                dv[7] = dv[5] * dv[6] / (dv[4] + 0.1);
                /* Fall through to label_e */
                
            default:
            label_e:
                for (j = 0; j < 8; j++) {
                    KEEP_ALIVE(v[j]);
                    KEEP_ALIVE(fv[j]);
                    if (j < 8) KEEP_ALIVE(dv[j]);
                }
                
                /* Long dependency chain */
                v[16] = v[0] + v[1] - v[2] * v[3] / (v[4] + 1) | v[5] & v[6] ^ v[7];
                v[17] = v[8] + v[9] - v[10] * v[11] / (v[12] + 1) | v[13] & v[14] ^ v[15];
                v[18] = v[16] * v[17] - v[0] + v[1];
                v[19] = v[18] / (v[2] + 1) + v[3] * v[4];
                
                checksum += v[16] + v[17] + v[18] + v[19] + 
                           (int)fv[0] + (int)fv[4] + (int)fv[7] +
                           (int)dv[0] + (int)dv[4] + (int)dv[7];
                
                /* Jump back to different case based on condition */
                if (i % 17 == 0) {
                    goto label_a;
                } else if (i % 19 == 0) {
                    goto label_c;
                }
                break;
        }
    }
    
    return checksum;
}

/* Function 3: Computed goto simulating state machine */
NOINLINE static unsigned long test_computed_goto(int iterations, int seed) {
    static void* labels[] = { &&state0, &&state1, &&state2, &&state3, 
                             &&state4, &&state5, &&state6, &&state7 };
    
    /* Many variables with different types */
    volatile int s0 = seed, s1 = seed * 2, s2 = seed * 3, s3 = seed * 4;
    volatile int t0 = seed + 10, t1 = seed + 20, t2 = seed + 30, t3 = seed + 40;
    volatile float u0 = seed * 0.123f, u1 = seed * 0.456f, u2 = seed * 0.789f;
    volatile double w0 = seed * 0.0123, w1 = seed * 0.0456, w2 = seed * 0.0789;
    volatile long z0 = seed * 1000L, z1 = seed * 2000L, z2 = seed * 3000L;
    volatile int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
    
    unsigned long checksum = 0;
    int state = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Jump to current state */
        goto *labels[state];
        
        state0:
            s0 = s1 + s2 * s3;
            t0 = t1 - t2 / (t3 + 1);
            u0 = u1 * u2 + (float)s0;
            w0 = w1 - w2;
            z0 = z1 ^ z2;
            state = (state + 1) % 8;
            goto next_op;
            
        state1:
            s1 = s0 * s2 - s3;
            t1 = t0 + t2 * t3;
            u1 = u0 / u2 + 1.0f;
            w1 = w0 + w2 * 2.0;
            z1 = z0 | z2;
            state = (state + 3) % 8;
            goto next_op;
            
        state2:
            s2 = s1 ^ s0;
            t2 = t1 & t3;
            u2 = u1 - u0;
            w2 = w1 / (w0 + 0.001);
            z2 = z1 + z0;
            state = (state + 5) % 8;
            goto next_op;
            
        state3:
            s3 = s2 | s1;
            t3 = t2 ^ t1;
            u0 = u2 * u1;
            w0 = w2 - w1;
            z0 = z2 & z1;
            state = (state + 7) % 8;
            goto next_op;
            
        state4:
            r0 = s0 + s1 + s2 + s3;
            r1 = t0 + t1 + t2 + t3;
            r2 = (int)(u0 + u1 + u2);
            r3 = (int)(w0 + w1 + w2);
            r4 = (int)(z0 + z1 + z2);
            state = (state + 2) % 8;
            goto next_op;
            
        state5:
            r5 = s0 * s1 - s2;
            r6 = t0 * t1 / (t2 + 1);
            r7 = (int)(u0 * u1 - u2);
            r8 = (int)(w0 * w1 / (w2 + 0.1));
            r9 = (int)(z0 * z1 / (z2 + 1));
            state = (state + 4) % 8;
            goto next_op;
            
        state6:
            s0 = r0 + r1 - r2 * r3 / (r4 + 1) | r5 & r6 ^ r7;
            s1 = r2 + r3 - r4 * r5 / (r6 + 1) | r7 & r8 ^ r9;
            s2 = s0 * s1 - r0 + r1;
            s3 = s2 / (r2 + 1) + r3 * r4;
            state = (state + 6) % 8;
            goto next_op;
            
        state7:
            checksum += s0 + s1 + s2 + s3 + t0 + t1 + t2 + t3 +
                       (int)u0 + (int)u1 + (int)u2 +
                       (int)w0 + (int)w1 + (int)w2 +
                       (int)z0 + (int)z1 + (int)z2;
            state = (i * seed) % 8;
            goto next_op;
            
        next_op:
            /* Keep all variables alive */
            KEEP_ALIVE(s0); KEEP_ALIVE(s1); KEEP_ALIVE(s2); KEEP_ALIVE(s3);
            KEEP_ALIVE(t0); KEEP_ALIVE(t1); KEEP_ALIVE(t2); KEEP_ALIVE(t3);
            KEEP_ALIVE(u0); KEEP_ALIVE(u1); KEEP_ALIVE(u2);
            KEEP_ALIVE(w0); KEEP_ALIVE(w1); KEEP_ALIVE(w2);
            KEEP_ALIVE(z0); KEEP_ALIVE(z1); KEEP_ALIVE(z2);
            KEEP_ALIVE(r0); KEEP_ALIVE(r1); KEEP_ALIVE(r2); KEEP_ALIVE(r3);
            KEEP_ALIVE(r4); KEEP_ALIVE(r5); KEEP_ALIVE(r6); KEEP_ALIVE(r7);
            KEEP_ALIVE(r8); KEEP_ALIVE(r9);
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = 10000;
    int seed = 42;
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
    
    /* Run all test functions to maximize coverage chance */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    printf("Total checksum: %lu\n", total_checksum);
    
    return 0;
}
