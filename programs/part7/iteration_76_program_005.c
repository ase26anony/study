/* mcf_coverage.c - Program to trigger GCC's MCF algorithm fixup graph debug output */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Function 1: Irreducible loops with goto jumping across loop boundaries */
NOINLINE static unsigned long test_irreducible_goto(int iterations, int seed) {
    /* Declare many local variables to increase register pressure */
    VOLATILE_VAR int a0 = seed + 1, a1 = seed + 2, a2 = seed + 3, a3 = seed + 4;
    VOLATILE_VAR int b0 = seed + 5, b1 = seed + 6, b2 = seed + 7, b3 = seed + 8;
    VOLATILE_VAR int c0 = seed + 9, c1 = seed + 10, c2 = seed + 11, c3 = seed + 12;
    VOLATILE_VAR float f0 = seed * 1.1f, f1 = seed * 1.2f, f2 = seed * 1.3f, f3 = seed * 1.4f;
    VOLATILE_VAR double d0 = seed * 2.1, d1 = seed * 2.2, d2 = seed * 2.3, d3 = seed * 2.4;
    VOLATILE_VAR long l0 = seed * 3L, l1 = seed * 4L, l2 = seed * 5L, l3 = seed * 6L;
    VOLATILE_VAR int e0 = 0, e1 = 0, e2 = 0, e3 = 0, e4 = 0, e5 = 0, e6 = 0, e7 = 0;
    
    unsigned long checksum = 0;
    int i;
    
    /* Labels for irreducible control flow */
    outer_loop_start:
    for (i = 0; i < iterations; i++) {
        /* Complex arithmetic chains keeping variables live */
        a0 = a1 + a2 * a3 - i;
        a1 = a2 + a3 * a0 + i;
        a2 = a3 + a0 * a1 / (i + 1);
        a3 = a0 + a1 * a2 % (i + 2);
        
        b0 = b1 ^ b2 | b3 & i;
        b1 = b2 ^ b3 | b0 & (i + 1);
        b2 = b3 ^ b0 | b1 & (i + 2);
        b3 = b0 ^ b1 | b2 & (i + 3);
        
        f0 = f1 * 1.1f + f2 - f3 / (i + 1.0f);
        f1 = f2 * 1.2f + f3 - f0 / (i + 2.0f);
        f2 = f3 * 1.3f + f0 - f1 / (i + 3.0f);
        f3 = f0 * 1.4f + f1 - f2 / (i + 4.0f);
        
        d0 = d1 * 2.1 + d2 - d3 / (i + 1.0);
        d1 = d2 * 2.2 + d3 - d0 / (i + 2.0);
        d2 = d3 * 2.3 + d0 - d1 / (i + 3.0);
        d3 = d0 * 2.4 + d1 - d2 / (i + 4.0);
        
        l0 = l1 * 3 + l2 - l3 / (i + 1);
        l1 = l2 * 4 + l3 - l0 / (i + 2);
        l2 = l3 * 5 + l0 - l1 / (i + 3);
        l3 = l0 * 6 + l1 - l2 / (i + 4);
        
        /* Create irreducible region with goto jumping into inner loop */
        if ((i & 3) == 0) {
            goto inner_loop_middle;
        }
        
        inner_loop_start:
        for (int j = 0; j < 5; j++) {
            /* More arithmetic to keep variables live */
            e0 = e1 + e2 * e3 - j;
            e1 = e2 + e3 * e0 + j;
            e2 = e3 + e0 * e1 / (j + 1);
            e3 = e0 + e1 * e2 % (j + 2);
            
            e4 = e5 ^ e6 | e7 & j;
            e5 = e6 ^ e7 | e4 & (j + 1);
            e6 = e7 ^ e4 | e5 & (j + 2);
            e7 = e4 ^ e5 | e6 & (j + 3);
            
            inner_loop_middle:
            /* Jump back to outer loop based on condition */
            if ((j & 1) == 0 && (i & 1) == 1) {
                goto outer_loop_continue;
            }
            
            /* Force variable usage with inline asm */
            asm volatile("" : : "r"(a0), "r"(b0), "r"(f0), "r"(d0), "r"(l0));
            asm volatile("" : : "r"(e0), "r"(e1), "r"(e2), "r"(e3));
        }
        
        outer_loop_continue:
        /* Mix all variables into checksum */
        checksum += a0 + a1 + a2 + a3 + b0 + b1 + b2 + b3;
        checksum += (unsigned long)(f0 + f1 + f2 + f3);
        checksum += (unsigned long)(d0 + d1 + d2 + d3);
        checksum += l0 + l1 + l2 + l3 + e0 + e1 + e2 + e3 + e4 + e5 + e6 + e7;
        
        /* Occasionally jump to start of outer loop */
        if ((i & 7) == 7) {
            goto outer_loop_start;
        }
    }
    
    return checksum;
}

/* Function 2: Switch with goto creating complex CFG */
NOINLINE static unsigned long test_switch_goto(int iterations, int seed) {
    VOLATILE_VAR int v[MANY_VARS];
    VOLATILE_VAR float fv[MANY_VARS];
    VOLATILE_VAR double dv[MANY_VARS];
    
    /* Initialize many variables */
    for (int k = 0; k < MANY_VARS; k++) {
        v[k] = seed + k;
        fv[k] = seed * 0.5f + k;
        dv[k] = seed * 0.7 + k;
    }
    
    unsigned long checksum = 0;
    int state = 0;
    
    /* Labels for goto targets */
    label_case_a:
    label_case_b:
    label_case_c:
    label_case_d:
    label_outside_switch:
    
    for (int i = 0; i < iterations; i++) {
        /* Long arithmetic chains using all variables */
        for (int k = 0; k < MANY_VARS - 1; k++) {
            v[k] = v[k+1] * v[k] + i - k;
            fv[k] = fv[k+1] * 1.5f + fv[k] / (i + 1.0f);
            dv[k] = dv[k+1] * 2.5 + dv[k] / (i + 2.0);
        }
        
        /* Complex switch with goto jumping to external labels */
        switch (state) {
            case 0:
                v[0] = v[1] + v[2] * v[3];
                if ((i & 1) == 0) goto label_case_b;
                else goto label_outside_switch;
                break;
                
            case 1:
                v[1] = v[2] - v[3] / v[4];
                goto label_case_c;
                break;
                
            case 2:
                v[2] = v[3] | v[4] & v[5];
                if ((i & 2) == 0) goto label_case_d;
                else goto label_case_a;
                break;
                
            case 3:
                v[3] = v[4] ^ v[5] | v[6];
                goto label_outside_switch;
                break;
                
            default:
                v[4] = v[5] + v[6] - v[7];
                goto label_case_a;
        }
        
        label_outside_switch:
        /* More arithmetic mixing variables */
        for (int k = 0; k < MANY_VARS; k += 2) {
            v[k] = v[k] * 3 + v[k+1];
            fv[k] = fv[k] * 1.7f - fv[k+1];
            dv[k] = dv[k] * 2.3 + dv[k+1];
        }
        
        /* Update state for next iteration */
        state = (state + 1) & 3;
        
        /* Force all variables to be considered live */
        for (int k = 0; k < MANY_VARS; k++) {
            asm volatile("" : : "r"(v[k]), "r"(fv[k]), "r"(dv[k]));
        }
        
        /* Accumulate checksum */
        for (int k = 0; k < MANY_VARS; k++) {
            checksum += v[k] + (unsigned long)fv[k] + (unsigned long)dv[k];
        }
    }
    
    return checksum;
}

/* Function 3: Computed goto simulating state machine */
NOINLINE static unsigned long test_computed_goto(int iterations, int seed) {
    /* Declare many variables of different types */
    VOLATILE_VAR int x0 = seed, x1 = seed+1, x2 = seed+2, x3 = seed+3;
    VOLATILE_VAR int y0 = seed+4, y1 = seed+5, y2 = seed+6, y3 = seed+7;
    VOLATILE_VAR float z0 = seed*0.3f, z1 = seed*0.4f, z2 = seed*0.5f, z3 = seed*0.6f;
    VOLATILE_VAR long w0 = seed*10L, w1 = seed*11L, w2 = seed*12L, w3 = seed*13L;
    VOLATILE_VAR double t0 = seed*0.9, t1 = seed*1.0, t2 = seed*1.1, t3 = seed*1.2;
    
    /* Additional variables for more register pressure */
    VOLATILE_VAR int extra[8];
    for (int k = 0; k < 8; k++) extra[k] = seed + 20 + k;
    
    /* Labels for computed goto */
    static void* labels[] = {
        &&state0, &&state1, &&state2, &&state3,
        &&state4, &&state5, &&state6, &&state7
    };
    
    unsigned long checksum = 0;
    int state = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex arithmetic keeping all variables live */
        x0 = x1 * x2 + x3 / (i + 1) - extra[0];
        x1 = x2 * x3 + x0 / (i + 2) - extra[1];
        x2 = x3 * x0 + x1 / (i + 3) - extra[2];
        x3 = x0 * x1 + x2 / (i + 4) - extra[3];
        
        y0 = y1 ^ y2 | y3 & extra[4];
        y1 = y2 ^ y3 | y0 & extra[5];
        y2 = y3 ^ y0 | y1 & extra[6];
        y3 = y0 ^ y1 | y2 & extra[7];
        
        z0 = z1 * 1.1f + z2 - z3 / (i * 0.1f + 1.0f);
        z1 = z2 * 1.2f + z3 - z0 / (i * 0.2f + 1.0f);
        z2 = z3 * 1.3f + z0 - z1 / (i * 0.3f + 1.0f);
        z3 = z0 * 1.4f + z1 - z2 / (i * 0.4f + 1.0f);
        
        w0 = w1 * 2 + w2 - w3 / (i + 1);
        w1 = w2 * 3 + w3 - w0 / (i + 2);
        w2 = w3 * 4 + w0 - w1 / (i + 3);
        w3 = w0 * 5 + w1 - w2 / (i + 4);
        
        t0 = t1 * 0.7 + t2 - t3 / (i * 0.5 + 1.0);
        t1 = t2 * 0.8 + t3 - t0 / (i * 0.6 + 1.0);
        t2 = t3 * 0.9 + t0 - t1 / (i * 0.7 + 1.0);
        t3 = t0 * 1.0 + t1 - t2 / (i * 0.8 + 1.0);
        
        /* Update extra variables */
        for (int k = 0; k < 8; k++) {
            extra[k] = extra[k] * 3 + i - k;
        }
        
        /* Computed goto based on state */
        goto *labels[state];
        
        state0:
            x0 = x0 + y0 * 2;
            state = 1;
            goto after_state;
        state1:
            x1 = x1 + y1 * 3;
            state = 2;
            goto after_state;
        state2:
            x2 = x2 + y2 * 4;
            state = 3;
            goto after_state;
        state3:
            x3 = x3 + y3 * 5;
            state = 4;
            goto after_state;
        state4:
            y0 = y0 ^ x0;
            state = 5;
            goto after_state;
        state5:
            y1 = y1 ^ x1;
            state = 6;
            goto after_state;
        state6:
            y2 = y2 ^ x2;
            state = 7;
            goto after_state;
        state7:
            y3 = y3 ^ x3;
            state = 0;
            goto after_state;
        
        after_state:
        /* Force all variables to stay live */
        asm volatile("" : : 
            "r"(x0), "r"(x1), "r"(x2), "r"(x3),
            "r"(y0), "r"(y1), "r"(y2), "r"(y3),
            "r"(z0), "r"(z1), "r"(z2), "r"(z3),
            "r"(w0), "r"(w1), "r"(w2), "r"(w3),
            "r"(t0), "r"(t1), "r"(t2), "r"(t3)
        );
        
        for (int k = 0; k < 8; k++) {
            asm volatile("" : : "r"(extra[k]));
        }
        
        /* Update checksum */
        checksum += x0 + x1 + x2 + x3 + y0 + y1 + y2 + y3;
        checksum += (unsigned long)(z0 + z1 + z2 + z3);
        checksum += w0 + w1 + w2 + w3;
        checksum += (unsigned long)(t0 + t1 + t2 + t3);
        for (int k = 0; k < 8; k++) checksum += extra[k];
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = 10000;
    int seed = 42;
    
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
    
    /* Run all test functions to trigger different CFG patterns */
    unsigned long total_checksum = 0;
    
    total_checksum += test_irreducible_goto(iterations, seed);
    printf("test_irreducible_goto completed, checksum so far: %lu\n", total_checksum);
    
    total_checksum += test_switch_goto(iterations, seed + 1);
    printf("test_switch_goto completed, checksum so far: %lu\n", total_checksum);
    
    total_checksum += test_computed_goto(iterations, seed + 2);
    printf("test_computed_goto completed, checksum so far: %lu\n", total_checksum);
    
    printf("Final checksum: %lu\n", total_checksum);
    
    return 0;
}
