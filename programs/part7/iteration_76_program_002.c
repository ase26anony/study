/* mcf_coverage.c - Program to trigger MCF fixup graph debug output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 32
#define DEFAULT_ITERATIONS 10000

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Complex control flow test functions */

NOINLINE static unsigned long test_irreducible_goto(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    VOLATILE_VAR int v0 = seed + 1;
    VOLATILE_VAR int v1 = seed + 2;
    VOLATILE_VAR int v2 = seed + 3;
    VOLATILE_VAR int v3 = seed + 4;
    VOLATILE_VAR int v4 = seed + 5;
    VOLATILE_VAR int v5 = seed + 6;
    VOLATILE_VAR int v6 = seed + 7;
    VOLATILE_VAR int v7 = seed + 8;
    VOLATILE_VAR int v8 = seed + 9;
    VOLATILE_VAR int v9 = seed + 10;
    VOLATILE_VAR float f0 = seed * 1.1f;
    VOLATILE_VAR float f1 = seed * 1.2f;
    VOLATILE_VAR float f2 = seed * 1.3f;
    VOLATILE_VAR float f3 = seed * 1.4f;
    VOLATILE_VAR double d0 = seed * 2.1;
    VOLATILE_VAR double d1 = seed * 2.2;
    VOLATILE_VAR double d2 = seed * 2.3;
    VOLATILE_VAR double d3 = seed * 2.4;
    VOLATILE_VAR long l0 = seed * 3L;
    VOLATILE_VAR long l1 = seed * 4L;
    VOLATILE_VAR long l2 = seed * 5L;
    VOLATILE_VAR long l3 = seed * 6L;
    VOLATILE_VAR unsigned u0 = seed * 7U;
    VOLATILE_VAR unsigned u1 = seed * 8U;
    VOLATILE_VAR unsigned u2 = seed * 9U;
    VOLATILE_VAR unsigned u3 = seed * 10U;
    VOLATILE_VAR char c0 = seed & 0xFF;
    VOLATILE_VAR char c1 = (seed >> 8) & 0xFF;
    VOLATILE_VAR short s0 = seed & 0xFFFF;
    VOLATILE_VAR short s1 = (seed >> 16) & 0xFFFF;
    
    unsigned long checksum = 0;
    int i;
    
    /* Create irreducible loop with goto jumping across loop boundaries */
    for (i = 0; i < iterations; i++) {
        /* Label definitions for goto targets */
        loop_start:
        if (i & 1) {
            /* Complex arithmetic to keep variables live */
            v0 = v1 + v2 * v3 - v4 / (v5 ? v5 : 1);
            f0 = f1 * f2 + f3 - f0;
            d0 = d1 * d2 - d3 + d0;
            l0 = l1 ^ l2 | l3 & l0;
            u0 = u1 + u2 - u3 * u0;
            
            if ((i % 3) == 0) {
                goto inner_loop;  /* Jump into inner loop */
            }
        }
        
        middle_block:
        v1 = v0 * v2 + v3 - v4;
        f1 = f0 + f2 * f3;
        d1 = d0 - d2 + d3;
        l1 = l0 | l2 & l3;
        u1 = u0 ^ u2 + u3;
        
        if ((i % 5) == 0) {
            goto loop_end;  /* Jump out of normal flow */
        }
        
        inner_loop:
        v2 = v1 - v0 + v3 * v4;
        f2 = f1 / (f0 ? f0 : 1.0f) + f3;
        d2 = d1 + d0 - d3;
        l2 = l1 & l0 | l3;
        u2 = u1 * u0 - u3;
        
        if ((i % 7) == 0) {
            goto middle_block;  /* Jump back */
        }
        
        v3 = v2 * v1 + v0 - v4;
        f3 = f2 - f1 + f0;
        d3 = d2 * d1 + d0;
        l3 = l2 ^ l1 & l0;
        u3 = u2 + u1 * u0;
        
        loop_end:
        /* More arithmetic to ensure all variables are used */
        v4 = v0 + v1 + v2 + v3;
        v5 = v4 * 2 - v0;
        v6 = v5 / 3 + v1;
        v7 = v6 ^ v2;
        v8 = v7 | v3;
        v9 = v8 & v4;
        
        /* Use inline assembly to mark variables as used */
        asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3));
        asm volatile("" : : "r"(v4), "r"(v5), "r"(v6), "r"(v7));
        asm volatile("" : : "r"(v8), "r"(v9));
        asm volatile("" : : "r"(f0), "r"(f1), "r"(f2), "r"(f3));
        asm volatile("" : : "r"(d0), "r"(d1), "r"(d2), "r"(d3));
        asm volatile("" : : "r"(l0), "r"(l1), "r"(l2), "r"(l3));
        asm volatile("" : : "r"(u0), "r"(u1), "r"(u2), "r"(u3));
        
        checksum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        checksum += (unsigned long)f0 + (unsigned long)f1 + (unsigned long)f2 + (unsigned long)f3;
        checksum += (unsigned long)d0 + (unsigned long)d1 + (unsigned long)d2 + (unsigned long)d3;
        checksum += l0 + l1 + l2 + l3 + u0 + u1 + u2 + u3;
    }
    
    return checksum;
}

NOINLINE static unsigned long test_switch_goto(int iterations, int seed) {
    VOLATILE_VAR int a0 = seed, a1 = seed+1, a2 = seed+2, a3 = seed+3;
    VOLATILE_VAR int b0 = seed+4, b1 = seed+5, b2 = seed+6, b3 = seed+7;
    VOLATILE_VAR int c0 = seed+8, c1 = seed+9, c2 = seed+10, c3 = seed+11;
    VOLATILE_VAR int d0 = seed+12, d1 = seed+13, d2 = seed+14, d3 = seed+15;
    VOLATILE_VAR float fa = seed*1.5f, fb = seed*2.5f, fc = seed*3.5f;
    VOLATILE_VAR double da = seed*4.5, db = seed*5.5, dc = seed*6.5;
    
    unsigned long checksum = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        int mod = i % 13;
        
        /* Switch with goto jumping to labels outside switch */
        switch (mod) {
            case 0:
                a0 = a1 * a2 + a3;
                fa = fb * 2.0f - fc;
                da = db + dc * 0.5;
                goto label_outside_switch;
                
            case 1:
                a1 = a0 - a2 * a3;
                fb = fa / 3.0f + fc;
                db = da - dc;
                goto middle_label;
                
            case 2:
                a2 = a1 + a0 - a3;
                fc = fb - fa;
                dc = db * da;
                goto loop_bottom;
                
            case 3:
                a3 = a2 * a1 + a0;
                fa = fc + fb;
                da = dc - db;
                /* fall through */
                
            case 4:
                b0 = a0 + a1 - a2 * a3;
                fb = fa * fc;
                db = da / 2.0;
                goto label_outside_switch;
                
            case 5:
                b1 = b0 ^ a0 & a1;
                fc = fb - fa;
                dc = db + da;
                goto middle_label;
                
            default:
                b2 = b1 | b0 ^ a2;
                fa = fc * fb;
                da = dc - db;
                break;
        }
        
        /* Normal flow continues here for some cases */
        b3 = b2 + b1 - b0;
        fb = fa + fc;
        db = da * dc;
        
        middle_label:
        c0 = b3 * b2 + b1 - b0;
        fc = fb / fa;
        dc = db - da;
        
        if ((i % 11) == 0) {
            goto loop_bottom;
        }
        
        label_outside_switch:
        c1 = c0 ^ b3 & b2;
        fa = fc + fb;
        da = dc * db;
        
        c2 = c1 | c0 - b1;
        fb = fa * 1.1f;
        db = da + 2.0;
        
        loop_bottom:
        c3 = c2 + c1 * c0;
        fc = fb - fa;
        dc = db / da;
        
        /* Use all variables in computation */
        d0 = a0 + a1 + a2 + a3;
        d1 = b0 + b1 + b2 + b3;
        d2 = c0 + c1 + c2 + c3;
        d3 = d0 * d1 - d2;
        
        /* Force register usage */
        asm volatile("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3));
        asm volatile("" : : "r"(b0), "r"(b1), "r"(b2), "r"(b3));
        asm volatile("" : : "r"(c0), "r"(c1), "r"(c2), "r"(c3));
        asm volatile("" : : "r"(d0), "r"(d1), "r"(d2), "r"(d3));
        asm volatile("" : : "r"(fa), "r"(fb), "r"(fc));
        asm volatile("" : : "r"(da), "r"(db), "r"(dc));
        
        checksum += a0 + a1 + a2 + a3 + b0 + b1 + b2 + b3;
        checksum += c0 + c1 + c2 + c3 + d0 + d1 + d2 + d3;
        checksum += (unsigned long)fa + (unsigned long)fb + (unsigned long)fc;
        checksum += (unsigned long)da + (unsigned long)db + (unsigned long)dc;
    }
    
    return checksum;
}

NOINLINE static unsigned long test_computed_goto(int iterations, int seed) {
    /* State machine using computed goto */
    static void* labels[] = {
        &&state0, &&state1, &&state2, &&state3,
        &&state4, &&state5, &&state6, &&state7
    };
    
    VOLATILE_VAR int x0 = seed, x1 = seed*2, x2 = seed*3, x3 = seed*4;
    VOLATILE_VAR int y0 = seed*5, y1 = seed*6, y2 = seed*7, y3 = seed*8;
    VOLATILE_VAR int z0 = seed*9, z1 = seed*10, z2 = seed*11, z3 = seed*12;
    VOLATILE_VAR long long ll0 = seed*100LL, ll1 = seed*200LL;
    VOLATILE_VAR long long ll2 = seed*300LL, ll3 = seed*400LL;
    
    unsigned long checksum = 0;
    int state = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Jump to current state */
        goto *labels[state];
        
        state0:
            x0 = x1 + x2 * x3;
            y0 = y1 ^ y2 | y3;
            ll0 = ll1 + ll2 - ll3;
            state = (i % 3) ? 1 : 4;
            goto state_end;
            
        state1:
            x1 = x0 - x2 + x3;
            y1 = y0 & y2 ^ y3;
            ll1 = ll0 * ll2 + ll3;
            state = (i % 5) ? 2 : 5;
            goto state_end;
            
        state2:
            x2 = x1 * x0 - x3;
            y2 = y1 | y0 & y3;
            ll2 = ll1 - ll0 + ll3;
            state = (i % 7) ? 3 : 6;
            goto state_end;
            
        state3:
            x3 = x2 + x1 - x0;
            y3 = y2 ^ y1 | y0;
            ll3 = ll2 * ll1 - ll0;
            state = (i % 11) ? 0 : 7;
            goto state_end;
            
        state4:
            z0 = x0 + y0 + x1;
            state = (i % 13) ? 5 : 1;
            goto state_end;
            
        state5:
            z1 = x1 - y1 + x2;
            state = (i % 17) ? 6 : 2;
            goto state_end;
            
        state6:
            z2 = x2 * y2 - x3;
            state = (i % 19) ? 7 : 3;
            goto state_end;
            
        state7:
            z3 = x3 / (y3 ? y3 : 1) + x0;
            state = (i % 23) ? 4 : 0;
            /* fall through */
            
        state_end:
            /* Complex arithmetic using all variables */
            x0 = x0 ^ x1 | x2 & x3;
            x1 = x1 + x2 - x3 * x0;
            x2 = x2 * x3 + x0 - x1;
            x3 = x3 / (x0 ? x0 : 1) + x1 - x2;
            
            y0 = y0 & y1 | y2 ^ y3;
            y1 = y1 - y2 + y3 * y0;
            y2 = y2 / (y0 ? y0 : 1) + y1 - y3;
            y3 = y3 * y0 + y1 ^ y2;
            
            z0 = z0 + z1 + z2 + z3;
            z1 = z0 * 2 - z1;
            z2 = z1 / 3 + z2;
            z3 = z2 ^ z3 | z0;
            
            ll0 = ll0 + ll1 - ll2 * ll3;
            ll1 = ll1 ^ ll2 | ll3 & ll0;
            ll2 = ll2 * ll3 + ll0 - ll1;
            ll3 = ll3 / (ll0 ? ll0 : 1) + ll1 - ll2;
            
            /* Force all variables to be live */
            asm volatile("" : : "r"(x0), "r"(x1), "r"(x2), "r"(x3));
            asm volatile("" : : "r"(y0), "r"(y1), "r"(y2), "r"(y3));
            asm volatile("" : : "r"(z0), "r"(z1), "r"(z2), "r"(z3));
            asm volatile("" : : "r"(ll0), "r"(ll1), "r"(ll2), "r"(ll3));
            
            checksum += x0 + x1 + x2 + x3 + y0 + y1 + y2 + y3;
            checksum += z0 + z1 + z2 + z3;
            checksum += (unsigned long)(ll0 >> 32) + (unsigned long)(ll0 & 0xFFFFFFFF);
            checksum += (unsigned long)(ll1 >> 32) + (unsigned long)(ll1 & 0xFFFFFFFF);
            checksum += (unsigned long)(ll2 >> 32) + (unsigned long)(ll2 & 0xFFFFFFFF);
            checksum += (unsigned long)(ll3 >> 32) + (unsigned long)(ll3 & 0xFFFFFFFF);
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = DEFAULT_ITERATIONS;
    unsigned long total_checksum = 0;
    int seed = 42;  /* Fixed seed for reproducibility */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = DEFAULT_ITERATIONS;
        }
    }
    
    printf("Running MCF coverage test with %d iterations...\n", iterations);
    
    /* Run all test functions to trigger different CFG patterns */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    /* Additional test with mixed patterns */
    total_checksum += test_irreducible_goto(iterations / 2, seed + 3);
    total_checksum += test_switch_goto(iterations / 2, seed + 4);
    
    printf("Final checksum: %lu\n", total_checksum);
    
    return 0;
}
