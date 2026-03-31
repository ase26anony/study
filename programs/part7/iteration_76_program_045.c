/* test_mcf_coverage.c
 * Compile with: gcc -O2 -fsched-pressure -fdump-rtl-sched2 -fno-if-conversion -funroll-loops -fno-tree-vectorize -fschedule-insns2 -o test_mcf test_mcf_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define VOLATILE_VAR volatile
#define NOINLINE __attribute__((noinline))

/* Function 1: Irreducible loop using goto jumps */
NOINLINE static unsigned long long test_irreducible_goto(int iterations, int seed) {
    /* Create many local variables to increase register pressure */
    VOLATILE_VAR int a1 = seed + 1, a2 = seed + 2, a3 = seed + 3, a4 = seed + 4;
    VOLATILE_VAR int b1 = seed * 2, b2 = seed * 3, b3 = seed * 4, b4 = seed * 5;
    VOLATILE_VAR float f1 = seed * 1.1f, f2 = seed * 1.2f, f3 = seed * 1.3f, f4 = seed * 1.4f;
    VOLATILE_VAR double d1 = seed * 2.1, d2 = seed * 2.2, d3 = seed * 2.3, d4 = seed * 2.4;
    VOLATILE_VAR long l1 = seed * 100L, l2 = seed * 200L, l3 = seed * 300L, l4 = seed * 400L;
    VOLATILE_VAR int c1 = 0, c2 = 0, c3 = 0, c4 = 0;
    VOLATILE_VAR float f5 = 0.0f, f6 = 0.0f;
    VOLATILE_VAR double d5 = 0.0, d6 = 0.0;
    
    unsigned long long checksum = 0;
    int i;
    
    /* Create irreducible loop structure with gotos */
    for (i = 0; i < iterations; i++) {
        /* Label definitions for goto targets */
        loop_start:
        if (i % 3 == 0) {
            /* Complex arithmetic to keep variables live */
            a1 = a2 * a3 + a4;
            a2 = a3 * a4 + a1;
            a3 = a4 * a1 + a2;
            a4 = a1 * a2 + a3;
            goto middle_block;
        }
        
        inner_loop:
        if (i % 5 == 0) {
            f1 = f2 * f3 + f4;
            f2 = f3 * f4 + f1;
            f3 = f4 * f1 + f2;
            f4 = f1 * f2 + f3;
            goto loop_end;
        }
        
        middle_block:
        if (i % 7 == 0) {
            d1 = d2 * d3 + d4;
            d2 = d3 * d4 + d1;
            d3 = d4 * d1 + d2;
            d4 = d1 * d2 + d3;
            goto inner_loop;
        }
        
        another_block:
        l1 = l2 * l3 + l4;
        l2 = l3 * l4 + l1;
        l3 = l4 * l1 + l2;
        l4 = l1 * l2 + l3;
        
        /* Force register pressure with inline asm */
        asm volatile("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4));
        asm volatile("" : : "r"(f1), "r"(f2), "r"(f3), "r"(f4));
        asm volatile("" : : "r"(d1), "r"(d2), "r"(d3), "r"(d4));
        asm volatile("" : : "r"(l1), "r"(l2), "r"(l3), "r"(l4));
        
        loop_end:
        /* More arithmetic to create dependency chains */
        c1 = a1 + b1 + (int)f1 + (int)d1 + (int)l1;
        c2 = a2 + b2 + (int)f2 + (int)d2 + (int)l2;
        c3 = a3 + b3 + (int)f3 + (int)d3 + (int)l3;
        c4 = a4 + b4 + (int)f4 + (int)d4 + (int)l4;
        
        f5 = f1 + f2 + f3 + f4;
        f6 = f5 * 2.0f - f1;
        d5 = d1 + d2 + d3 + d4;
        d6 = d5 * 2.0 - d1;
        
        checksum += c1 + c2 + c3 + c4 + (int)f5 + (int)f6 + (int)d5 + (int)d6;
        
        /* Jump back to start with condition - creates irreducible region */
        if (i % 11 == 0) {
            goto loop_start;
        }
        if (i % 13 == 0) {
            goto another_block;
        }
    }
    
    return checksum;
}

/* Function 2: Switch with goto creating complex CFG */
NOINLINE static unsigned long long test_switch_goto(int iterations, int seed) {
    VOLATILE_VAR int v1 = seed, v2 = seed * 2, v3 = seed * 3, v4 = seed * 4;
    VOLATILE_VAR int w1 = 0, w2 = 0, w3 = 0, w4 = 0;
    VOLATILE_VAR float x1 = seed * 0.5f, x2 = seed * 0.6f, x3 = seed * 0.7f, x4 = seed * 0.8f;
    VOLATILE_VAR double y1 = seed * 0.9, y2 = seed * 1.0, y3 = seed * 1.1, y4 = seed * 1.2;
    VOLATILE_VAR long z1 = seed * 50, z2 = seed * 60, z3 = seed * 70, z4 = seed * 80;
    
    unsigned long long checksum = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        int state = (i + seed) % 10;
        
        switch (state) {
            case 0:
                v1 = v2 * v3 - v4;
                v2 = v3 * v4 - v1;
                goto label_a;
                
            case 1:
                v3 = v4 * v1 - v2;
                v4 = v1 * v2 - v3;
                goto label_b;
                
            case 2:
                w1 = v1 + v2 + v3 + v4;
                goto label_c;
                
            case 3:
                w2 = v1 * v2 + v3 * v4;
                goto label_d;
                
            case 4:
                w3 = v2 * v3 + v4 * v1;
                goto label_a;
                
            case 5:
                w4 = v3 * v4 + v1 * v2;
                goto label_b;
                
            case 6:
                x1 = x2 + x3 - x4;
                goto label_c;
                
            case 7:
                x2 = x3 + x4 - x1;
                goto label_d;
                
            case 8:
                x3 = x4 + x1 - x2;
                goto label_a;
                
            case 9:
                x4 = x1 + x2 - x3;
                goto label_b;
                
            default:
                break;
        }
        
        /* Labels that are targets from switch cases */
        label_a:
        y1 = y2 * 1.1 + y3;
        y2 = y3 * 1.2 + y4;
        goto continue_loop;
        
        label_b:
        y3 = y4 * 1.3 + y1;
        y4 = y1 * 1.4 + y2;
        goto continue_loop;
        
        label_c:
        z1 = z2 + z3 * 2;
        z2 = z3 + z4 * 3;
        goto continue_loop;
        
        label_d:
        z3 = z4 + z1 * 4;
        z4 = z1 + z2 * 5;
        /* Fall through to continue_loop */
        
        continue_loop:
        /* More operations to increase register pressure */
        x1 = x1 * 0.9f + x2;
        x2 = x2 * 0.8f + x3;
        x3 = x3 * 0.7f + x4;
        x4 = x4 * 0.6f + x1;
        
        y1 = y1 * 0.95 + y2;
        y2 = y2 * 0.85 + y3;
        y3 = y3 * 0.75 + y4;
        y4 = y4 * 0.65 + y1;
        
        /* Force all variables to be live */
        asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4));
        asm volatile("" : : "r"(w1), "r"(w2), "r"(w3), "r"(w4));
        asm volatile("" : : "r"(x1), "r"(x2), "r"(x3), "r"(x4));
        asm volatile("" : : "r"(y1), "r"(y2), "r"(y3), "r"(y4));
        asm volatile("" : : "r"(z1), "r"(z2), "r"(z3), "r"(z4));
        
        checksum += v1 + v2 + v3 + v4 + w1 + w2 + w3 + w4 + 
                   (int)x1 + (int)x2 + (int)x3 + (int)x4 +
                   (int)y1 + (int)y2 + (int)y3 + (int)y4 +
                   z1 + z2 + z3 + z4;
    }
    
    return checksum;
}

/* Function 3: Computed goto for state machine */
NOINLINE static unsigned long long test_computed_goto(int iterations, int seed) {
    VOLATILE_VAR int s1 = seed, s2 = seed + 100, s3 = seed + 200, s4 = seed + 300;
    VOLATILE_VAR int t1 = 0, t2 = 0, t3 = 0, t4 = 0;
    VOLATILE_VAR float u1 = seed * 0.25f, u2 = seed * 0.35f, u3 = seed * 0.45f, u4 = seed * 0.55f;
    VOLATILE_VAR double v1 = seed * 0.65, v2 = seed * 0.75, v3 = seed * 0.85, v4 = seed * 0.95;
    VOLATILE_VAR long r1 = seed * 25, r2 = seed * 35, r3 = seed * 45, r4 = seed * 55;
    
    /* Labels for computed goto */
    static void* labels[] = {
        &&state0, &&state1, &&state2, &&state3, 
        &&state4, &&state5, &&state6, &&state7
    };
    
    unsigned long long checksum = 0;
    int i;
    int state = 0;
    
    for (i = 0; i < iterations; i++) {
        /* Update state based on iteration */
        state = (state + i + seed) % 8;
        
        /* Computed goto */
        goto *labels[state];
        
        state0:
        s1 = s2 * s3 / (s4 ? s4 : 1);
        s2 = s3 * s4 / (s1 ? s1 : 1);
        t1 = s1 + s2 + s3 + s4;
        goto after_state;
        
        state1:
        s3 = s4 * s1 / (s2 ? s2 : 1);
        s4 = s1 * s2 / (s3 ? s3 : 1);
        t2 = s1 - s2 + s3 - s4;
        goto after_state;
        
        state2:
        u1 = u2 + u3 - u4;
        u2 = u3 + u4 - u1;
        t3 = (int)u1 + (int)u2 + (int)u3 + (int)u4;
        goto after_state;
        
        state3:
        u3 = u4 + u1 - u2;
        u4 = u1 + u2 - u3;
        t4 = (int)u1 * (int)u2 - (int)u3 * (int)u4;
        goto after_state;
        
        state4:
        v1 = v2 * 1.5 + v3;
        v2 = v3 * 1.6 + v4;
        r1 = (long)v1 + (long)v2 + (long)v3 + (long)v4;
        goto after_state;
        
        state5:
        v3 = v4 * 1.7 + v1;
        v4 = v1 * 1.8 + v2;
        r2 = (long)v1 * (long)v2 - (long)v3 * (long)v4;
        goto after_state;
        
        state6:
        r3 = r1 + r2 * r4;
        r4 = r2 + r3 * r1;
        goto after_state;
        
        state7:
        r1 = r3 + r4 * r2;
        r2 = r4 + r1 * r3;
        goto after_state;
        
        after_state:
        /* Long dependency chain to increase register pressure */
        s1 = s1 + t1 - t2 + t3 - t4;
        s2 = s2 - t1 + t2 - t3 + t4;
        s3 = s3 * 2 + t1 + t2;
        s4 = s4 / 2 + t3 + t4;
        
        u1 = u1 * 1.1f + u2;
        u2 = u2 * 1.2f + u3;
        u3 = u3 * 1.3f + u4;
        u4 = u4 * 1.4f + u1;
        
        v1 = v1 * 1.05 + v2;
        v2 = v2 * 1.15 + v3;
        v3 = v3 * 1.25 + v4;
        v4 = v4 * 1.35 + v1;
        
        r3 = r1 + r2 + r3 + r4;
        r4 = r1 * r2 - r3 * r4;
        
        /* Force all variables to stay live */
        asm volatile("" : : "r"(s1), "r"(s2), "r"(s3), "r"(s4));
        asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4));
        asm volatile("" : : "r"(u1), "r"(u2), "r"(u3), "r"(u4));
        asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4));
        asm volatile("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4));
        
        checksum += s1 + s2 + s3 + s4 + t1 + t2 + t3 + t4 +
                   (int)u1 + (int)u2 + (int)u3 + (int)u4 +
                   (int)v1 + (int)v2 + (int)v3 + (int)v4 +
                   r1 + r2 + r3 + r4;
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = 10000;
    int seed = time(NULL);
    unsigned long long total_checksum = 0;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = 10000;
        }
    }
    
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF coverage test with %d iterations, seed=%d\n", iterations, seed);
    
    /* Run all test functions to maximize coverage chance */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    printf("Final checksum: %llu\n", total_checksum);
    
    return 0;
}
