/* mcf_coverage.c - Program to trigger GCC's MCF fixup graph debug output */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Function 1: Irreducible loops with goto jumping across boundaries */
NOINLINE static unsigned long test_irreducible_goto(int iterations, int seed) {
    VOLATILE_VAR int i, j, k;
    VOLATILE_VAR unsigned long checksum = seed;
    
    /* Many local variables to create register pressure */
    VOLATILE_VAR int a0 = seed, a1 = seed+1, a2 = seed+2, a3 = seed+3, a4 = seed+4;
    VOLATILE_VAR int a5 = seed+5, a6 = seed+6, a7 = seed+7, a8 = seed+8, a9 = seed+9;
    VOLATILE_VAR float f0 = seed*0.1, f1 = seed*0.2, f2 = seed*0.3, f3 = seed*0.4;
    VOLATILE_VAR double d0 = seed*0.01, d1 = seed*0.02, d2 = seed*0.03, d3 = seed*0.04;
    VOLATILE_VAR long l0 = seed*10, l1 = seed*20, l2 = seed*30, l3 = seed*40;
    VOLATILE_VAR int b0 = 0, b1 = 0, b2 = 0, b3 = 0, b4 = 0, b5 = 0, b6 = 0, b7 = 0;
    
    /* Labels for irreducible control flow */
    outer_loop_start:
    for (i = 0; i < iterations; i++) {
        inner_loop_start:
        for (j = 0; j < 10; j++) {
            /* Complex arithmetic chains keeping variables live */
            a0 = a1 + a2 * a3 - a4 / (a5 + 1);
            a1 = a2 + a3 * a4 - a5 / (a6 + 1);
            a2 = a3 + a4 * a5 - a6 / (a7 + 1);
            a3 = a4 + a5 * a6 - a7 / (a8 + 1);
            f0 = f1 * 1.1f + f2 * 0.9f - f3 * 0.5f;
            f1 = f2 * 1.2f + f3 * 0.8f - f0 * 0.6f;
            d0 = d1 * 1.01 + d2 * 0.99 - d3 * 0.55;
            d1 = d2 * 1.02 + d3 * 0.98 - d0 * 0.66;
            l0 = l1 * 3 + l2 / 2 - l3 * 4;
            l1 = l2 * 5 + l3 / 3 - l0 * 6;
            
            /* Mark variables as used to prevent optimization */
            asm volatile("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3));
            asm volatile("" : : "r"(f0), "r"(f1), "r"(f2), "r"(f3));
            asm volatile("" : : "r"(d0), "r"(d1), "r"(d2), "r"(d3));
            asm volatile("" : : "r"(l0), "r"(l1), "r"(l2), "r"(l3));
            
            /* Irreducible: jump from inner loop to outer loop start */
            if ((i * j + seed) % 37 == 0) {
                checksum += i * j;
                goto outer_loop_start;  /* Creates irreducible region */
            }
            
            /* Jump to middle of inner loop from different point */
            if ((i * j + seed) % 41 == 0) {
                checksum += a0;
                goto inner_middle;
            }
        }
        
        continue_outer:
        a4 = a5 + a6 * a7 - a8 / (a9 + 1);
        a5 = a6 + a7 * a8 - a9 / (a0 + 1);
        checksum += a4 + a5;
        continue;
        
        inner_middle:
        a6 = a7 + a8 * a9 - a0 / (a1 + 1);
        a7 = a8 + a9 * a0 - a1 / (a2 + 1);
        checksum += a6 + a7;
        goto inner_loop_start;  /* Jump back to loop start */
    }
    
    /* Final computation to use all variables */
    checksum += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9;
    checksum += (int)f0 + (int)f1 + (int)f2 + (int)f3;
    checksum += (int)d0 + (int)d1 + (int)d2 + (int)d3;
    checksum += l0 + l1 + l2 + l3;
    
    return checksum;
}

/* Function 2: Switch with goto creating complex CFG */
NOINLINE static unsigned long test_switch_goto(int iterations, int seed) {
    VOLATILE_VAR int i, state = seed % 5;
    VOLATILE_VAR unsigned long checksum = seed;
    
    /* Another set of many variables */
    VOLATILE_VAR int x0 = seed, x1 = seed*2, x2 = seed*3, x3 = seed*4, x4 = seed*5;
    VOLATILE_VAR int x5 = seed*6, x6 = seed*7, x7 = seed*8, x8 = seed*9, x9 = seed*10;
    VOLATILE_VAR float y0 = seed*0.11, y1 = seed*0.22, y2 = seed*0.33, y3 = seed*0.44;
    VOLATILE_VAR double z0 = seed*0.011, z1 = seed*0.022, z2 = seed*0.033, z3 = seed*0.044;
    VOLATILE_VAR long w0 = seed*11, w1 = seed*22, w2 = seed*33, w3 = seed*44;
    
    /* Labels for switch goto targets */
    label_case0:
    label_case1:
    label_case2:
    label_case3:
    label_case4:
    label_outside_switch:
    
    for (i = 0; i < iterations; i++) {
        /* Long arithmetic chains */
        x0 = x1 * x2 - x3 / (x4 + 1) + x5 % (x6 + 2);
        x1 = x2 * x3 - x4 / (x5 + 1) + x6 % (x7 + 2);
        x2 = x3 * x4 - x5 / (x6 + 1) + x7 % (x8 + 2);
        x3 = x4 * x5 - x6 / (x7 + 1) + x8 % (x9 + 2);
        y0 = y1 * 1.5f - y2 / 2.0f + y3 * 0.75f;
        y1 = y2 * 1.6f - y3 / 2.1f + y0 * 0.85f;
        z0 = z1 * 1.51 - z2 / 2.01 + z3 * 0.751;
        z1 = z2 * 1.61 - z3 / 2.11 + z0 * 0.851;
        w0 = w1 * 7 - w2 / 3 + w3 * 5;
        w1 = w2 * 8 - w3 / 4 + w0 * 6;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(x0), "r"(x1), "r"(x2), "r"(x3), "r"(x4));
        asm volatile("" : : "r"(y0), "r"(y1), "r"(y2), "r"(y3));
        asm volatile("" : : "r"(z0), "r"(z1), "r"(z2), "r"(z3));
        asm volatile("" : : "r"(w0), "r"(w1), "r"(w2), "r"(w3));
        
        /* Switch with goto to labels outside */
        switch (state) {
            case 0:
                x4 = x5 + x6;
                checksum += x4;
                if ((i + seed) % 7 == 0)
                    goto label_case2;  /* Jump to different case label */
                else if ((i + seed) % 11 == 0)
                    goto label_outside_switch;  /* Jump out of switch */
                state = 1;
                break;
                
            case 1:
                x5 = x6 + x7;
                checksum += x5;
                if ((i + seed) % 13 == 0)
                    goto label_case4;
                state = 2;
                break;
                
            case 2:
                x6 = x7 + x8;
                checksum += x6;
                if ((i + seed) % 17 == 0)
                    goto label_case0;
                state = 3;
                break;
                
            case 3:
                x7 = x8 + x9;
                checksum += x7;
                if ((i + seed) % 19 == 0)
                    goto label_case1;
                state = 4;
                break;
                
            case 4:
                x8 = x9 + x0;
                checksum += x8;
                if ((i + seed) % 23 == 0)
                    goto label_outside_switch;
                state = 0;
                break;
        }
        
        label_outside_switch:
        /* More arithmetic */
        x9 = x0 * 2 - x1 + x2 / (x3 + 1);
        checksum += x9;
        state = (state + 1) % 5;
    }
    
    checksum += x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9;
    checksum += (int)y0 + (int)y1 + (int)y2 + (int)y3;
    checksum += (int)z0 + (int)z1 + (int)z2 + (int)z3;
    checksum += w0 + w1 + w2 + w3;
    
    return checksum;
}

/* Function 3: Computed goto for state machine */
NOINLINE static unsigned long test_computed_goto(int iterations, int seed) {
    VOLATILE_VAR int i;
    VOLATILE_VAR unsigned long checksum = seed;
    
    /* Yet another set of variables */
    VOLATILE_VAR int r0 = seed, r1 = seed+11, r2 = seed+22, r3 = seed+33, r4 = seed+44;
    VOLATILE_VAR int r5 = seed+55, r6 = seed+66, r7 = seed+77, r8 = seed+88, r9 = seed+99;
    VOLATILE_VAR float s0 = seed*0.15, s1 = seed*0.25, s2 = seed*0.35, s3 = seed*0.45;
    VOLATILE_VAR double t0 = seed*0.015, t1 = seed*0.025, t2 = seed*0.035, t3 = seed*0.045;
    VOLATILE_VAR long u0 = seed*15, u1 = seed*25, u2 = seed*35, u3 = seed*45;
    
    /* Labels for computed goto */
    static void* labels[] = {
        &&state_a, &&state_b, &&state_c, 
        &&state_d, &&state_e, &&state_f
    };
    
    void* next_state = labels[seed % 6];
    
    for (i = 0; i < iterations; i++) {
        /* Arithmetic operations */
        r0 = r1 * 3 + r2 - r3 / (r4 + 1);
        r1 = r2 * 4 + r3 - r4 / (r5 + 1);
        r2 = r3 * 5 + r4 - r5 / (r6 + 1);
        r3 = r4 * 6 + r5 - r6 / (r7 + 1);
        s0 = s1 * 2.1f - s2 / 1.5f + s3 * 0.8f;
        s1 = s2 * 2.2f - s3 / 1.6f + s0 * 0.9f;
        t0 = t1 * 2.01 - t2 / 1.51 + t3 * 0.81;
        t1 = t2 * 2.02 - t3 / 1.61 + t0 * 0.91;
        u0 = u1 * 9 + u2 - u3 / 5;
        u1 = u2 * 10 + u3 - u0 / 6;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r4));
        asm volatile("" : : "r"(s0), "r"(s1), "r"(s2), "r"(s3));
        asm volatile("" : : "r"(t0), "r"(t1), "r"(t2), "r"(t3));
        asm volatile("" : : "r"(u0), "r"(u1), "r"(u2), "r"(u3));
        
        /* Computed goto */
        goto *next_state;
        
        state_a:
            r4 = r5 + r6 * r7;
            checksum += r4;
            next_state = labels[(i + 1) % 6];
            continue;
            
        state_b:
            r5 = r6 + r7 * r8;
            checksum += r5;
            next_state = labels[(i + 2) % 6];
            continue;
            
        state_c:
            r6 = r7 + r8 * r9;
            checksum += r6;
            next_state = labels[(i + 3) % 6];
            continue;
            
        state_d:
            r7 = r8 + r9 * r0;
            checksum += r7;
            next_state = labels[(i + 4) % 6];
            continue;
            
        state_e:
            r8 = r9 + r0 * r1;
            checksum += r8;
            next_state = labels[(i + 5) % 6];
            continue;
            
        state_f:
            r9 = r0 + r1 * r2;
            checksum += r9;
            next_state = labels[i % 6];
            continue;
    }
    
    checksum += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9;
    checksum += (int)s0 + (int)s1 + (int)s2 + (int)s3;
    checksum += (int)t0 + (int)t1 + (int)t2 + (int)t3;
    checksum += u0 + u1 + u2 + u3;
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = 10000;
    int seed = 42;
    unsigned long total_checksum = 0;
    
    /* Parse command line for iteration count */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10000;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF coverage test with %d iterations, seed=%d\n", 
           iterations, seed);
    
    /* Call all test functions to trigger different CFG patterns */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    /* Additional calls with different seeds */
    total_checksum += test_irreducible_goto(iterations / 2, seed + 100);
    total_checksum += test_switch_goto(iterations / 2, seed + 101);
    
    printf("Final checksum: %lu\n", total_checksum);
    
    return 0;
}
