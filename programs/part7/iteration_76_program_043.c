/* mcf_coverage.c - Program to trigger GCC's MCF fixup graph debug output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define VOLATILE(var) volatile var

/* Function 1: Irreducible loops with goto jumping across boundaries */
NOINLINE static uint64_t test_irreducible_goto(int iterations, int seed) {
    /* Many local variables to create register pressure */
    VOLATILE(int) a1 = seed + 1, a2 = seed + 2, a3 = seed + 3, a4 = seed + 4;
    VOLATILE(int) b1 = seed + 5, b2 = seed + 6, b3 = seed + 7, b4 = seed + 8;
    VOLATILE(float) f1 = seed * 1.1f, f2 = seed * 1.2f, f3 = seed * 1.3f, f4 = seed * 1.4f;
    VOLATILE(double) d1 = seed * 2.1, d2 = seed * 2.2, d3 = seed * 2.3, d4 = seed * 2.4;
    VOLATILE(long) l1 = seed * 3L, l2 = seed * 4L, l3 = seed * 5L, l4 = seed * 6L;
    VOLATILE(int) c1 = 0, c2 = 0, c3 = 0, c4 = 0;
    VOLATILE(float) f5 = 0.0f, f6 = 0.0f;
    VOLATILE(double) d5 = 0.0, d6 = 0.0;
    VOLATILE(long) l5 = 0L, l6 = 0L;
    
    uint64_t checksum = 0;
    
    /* Outer loop */
    for (int i = 0; i < iterations; i++) {
        /* Complex arithmetic to keep variables live */
        a1 = a2 * 3 + a3;
        a2 = a4 ^ a1;
        a3 = a1 - a2;
        a4 = a3 * a2 + a1;
        
        f1 = f2 * 1.5f + f3;
        f2 = f4 / 2.0f + f1;
        f3 = f1 - f2 * 0.7f;
        f4 = f3 * f2 + f1;
        
        d1 = d2 * 2.3 + d3;
        d2 = d4 / 1.7 + d1;
        d3 = d1 - d2 * 0.9;
        d4 = d3 * d2 + d1;
        
        l1 = l2 << 3 | l3;
        l2 = l4 ^ l1;
        l3 = l1 - l2;
        l4 = l3 * l2 + l1;
        
        /* Irreducible control flow with goto */
        if (i % 7 == 0) {
            goto inner_label1;
        } else if (i % 11 == 0) {
            goto inner_label2;
        }
        
        /* More arithmetic */
        c1 = a1 + a2;
        c2 = a3 + a4;
        c3 = b1 + b2;
        c4 = b3 + b4;
        
        f5 = f1 + f2;
        f6 = f3 + f4;
        d5 = d1 + d2;
        d6 = d3 + d4;
        l5 = l1 + l2;
        l6 = l3 + l4;
        
        /* Jump into inner loop from outside */
        if (i % 13 == 0) {
            goto loop_middle;
        }
        
        /* Inner loop with multiple entry points */
        for (int j = 0; j < 5; j++) {
            inner_label1:
            /* More operations to increase register pressure */
            a1 = a1 ^ (c1 + j);
            a2 = a2 ^ (c2 - j);
            f1 = f1 + (f5 * j);
            f2 = f2 + (f6 / (j + 1));
            d1 = d1 * (1.0 + j * 0.1);
            d2 = d2 / (1.0 + j * 0.2);
            
            if (j % 2 == 0) {
                goto inner_label2;
            }
            
            loop_middle:
            l1 = l1 | (l5 << j);
            l2 = l2 ^ (l6 >> j);
            
            inner_label2:
            b1 = b1 + (c3 * j);
            b2 = b2 - (c4 / (j + 1));
            f3 = f3 * (1.0f + j * 0.3f);
            f4 = f4 / (1.0f + j * 0.4f);
            
            /* Jump out of inner loop to outer */
            if (j == 3 && i % 17 == 0) {
                goto outer_continue;
            }
        }
        
        outer_continue:
        /* Final mixing */
        checksum += (uint64_t)(a1 + a2 + a3 + a4 + b1 + b2 + b3 + b4);
        checksum += (uint64_t)(f1 + f2 + f3 + f4 + f5 + f6);
        checksum += (uint64_t)(d1 + d2 + d3 + d4 + d5 + d6);
        checksum += l1 + l2 + l3 + l4 + l5 + l6;
    }
    
    /* Use inline assembly to mark variables as used */
    asm volatile("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4));
    asm volatile("" : : "r"(b1), "r"(b2), "r"(b3), "r"(b4));
    asm volatile("" : : "r"(f1), "r"(f2), "r"(f3), "r"(f4));
    asm volatile("" : : "r"(d1), "r"(d2), "r"(d3), "r"(d4));
    
    return checksum;
}

/* Function 2: Switch with goto creating irreducible regions */
NOINLINE static uint64_t test_switch_goto(int iterations, int seed) {
    VOLATILE(int) x1 = seed, x2 = seed * 2, x3 = seed * 3, x4 = seed * 4;
    VOLATILE(int) y1 = seed + 10, y2 = seed + 20, y3 = seed + 30, y4 = seed + 40;
    VOLATILE(float) g1 = seed * 0.5f, g2 = seed * 0.6f, g3 = seed * 0.7f, g4 = seed * 0.8f;
    VOLATILE(double) h1 = seed * 1.5, h2 = seed * 1.6, h3 = seed * 1.7, h4 = seed * 1.8;
    VOLATILE(long) m1 = seed * 10L, m2 = seed * 20L, m3 = seed * 30L, m4 = seed * 40L;
    
    /* Additional variables for more pressure */
    VOLATILE(int) t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 0, t6 = 0, t7 = 0, t8 = 0;
    VOLATILE(float) u1 = 0.0f, u2 = 0.0f, u3 = 0.0f, u4 = 0.0f;
    VOLATILE(double) v1 = 0.0, v2 = 0.0, v3 = 0.0, v4 = 0.0;
    
    uint64_t checksum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Pre-switch computations */
        x1 = x2 ^ x3;
        x2 = x4 | x1;
        x3 = x1 - x2;
        x4 = x3 * x2;
        
        y1 = y2 + y3;
        y2 = y4 - y1;
        y3 = y1 * y2;
        y4 = y3 / (y2 != 0 ? y2 : 1);
        
        g1 = g2 * 1.1f + g3;
        g2 = g4 / 1.2f - g1;
        g3 = g1 * g2;
        g4 = g3 + g2 - g1;
        
        h1 = h2 * 1.3 + h3;
        h2 = h4 / 1.4 - h1;
        h3 = h1 * h2;
        h4 = h3 + h2 - h1;
        
        m1 = m2 << 2 | m3;
        m2 = m4 >> 1 ^ m1;
        m3 = m1 - m2;
        m4 = m3 * m2 + m1;
        
        /* Switch with goto to labels outside */
        switch (i % 8) {
            case 0:
                t1 = x1 + y1;
                t2 = x2 + y2;
                goto label_a;
            case 1:
                t3 = x3 + y3;
                t4 = x4 + y4;
                goto label_b;
            case 2:
                t5 = x1 * y1;
                t6 = x2 * y2;
                goto label_c;
            case 3:
                t7 = x3 * y3;
                t8 = x4 * y4;
                goto label_d;
            case 4:
                u1 = g1 + h1;
                u2 = g2 + h2;
                goto label_a;
            case 5:
                u3 = g3 + h3;
                u4 = g4 + h4;
                goto label_b;
            case 6:
                v1 = g1 * h1;
                v2 = g2 * h2;
                goto label_c;
            case 7:
                v3 = g3 * h3;
                v4 = g4 * h4;
                goto label_d;
        }
        
        label_a:
        /* Complex arithmetic chain */
        x1 = x1 ^ t1;
        x2 = x2 | t2;
        g1 = g1 + u1;
        g2 = g2 - u2;
        m1 = m1 + (t1 * 2);
        m2 = m2 - (t2 / 2);
        
        if (i % 3 == 0) goto label_d;
        
        label_b:
        x3 = x3 ^ t3;
        x4 = x4 | t4;
        g3 = g3 + u3;
        g4 = g4 - u4;
        m3 = m3 + (t3 * 3);
        m4 = m4 - (t4 / 3);
        
        if (i % 5 == 0) goto label_c;
        
        label_c:
        y1 = y1 ^ t5;
        y2 = y2 | t6;
        h1 = h1 + v1;
        h2 = h2 - v2;
        m1 = m1 | (t5 << 1);
        m2 = m2 ^ (t6 >> 1);
        
        if (i % 7 == 0) goto label_a;
        
        label_d:
        y3 = y3 ^ t7;
        y4 = y4 | t8;
        h3 = h3 + v3;
        h4 = h4 - v4;
        m3 = m3 | (t7 << 2);
        m4 = m4 ^ (t8 >> 2);
        
        /* Accumulate checksum */
        checksum += (uint64_t)(x1 + x2 + x3 + x4 + y1 + y2 + y3 + y4);
        checksum += (uint64_t)(g1 + g2 + g3 + g4 + h1 + h2 + h3 + h4);
        checksum += m1 + m2 + m3 + m4;
        checksum += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8;
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(x1), "r"(x2), "r"(x3), "r"(x4));
    asm volatile("" : : "r"(y1), "r"(y2), "r"(y3), "r"(y4));
    asm volatile("" : : "r"(g1), "r"(g2), "r"(g3), "r"(g4));
    
    return checksum;
}

/* Function 3: Computed goto state machine */
NOINLINE static uint64_t test_computed_goto(int iterations, int seed) {
    VOLATILE(int) s1 = seed, s2 = seed + 100, s3 = seed + 200, s4 = seed + 300;
    VOLATILE(int) r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0, r7 = 0, r8 = 0;
    VOLATILE(float) z1 = seed * 0.25f, z2 = seed * 0.35f, z3 = seed * 0.45f, z4 = seed * 0.55f;
    VOLATILE(double) w1 = seed * 0.75, w2 = seed * 0.85, w3 = seed * 0.95, w4 = seed * 1.05;
    VOLATILE(long) k1 = seed * 100L, k2 = seed * 200L, k3 = seed * 300L, k4 = seed * 400L;
    
    /* Additional pressure variables */
    VOLATILE(int) p1 = 0, p2 = 0, p3 = 0, p4 = 0, p5 = 0, p6 = 0, p7 = 0, p8 = 0;
    VOLATILE(float) q1 = 0.0f, q2 = 0.0f, q3 = 0.0f, q4 = 0.0f;
    VOLATILE(double) e1 = 0.0, e2 = 0.0, e3 = 0.0, e4 = 0.0;
    
    /* Labels for computed goto */
    static void* labels[] = {
        &&state0, &&state1, &&state2, &&state3,
        &&state4, &&state5, &&state6, &&state7
    };
    
    uint64_t checksum = 0;
    int state = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Update state based on complex condition */
        state = (s1 + s2 + s3 + s4 + i) % 8;
        
        /* Jump to state handler */
        goto *labels[state];
        
        state0:
        r1 = s1 * 2 + s2;
        r2 = s3 / 2 + s4;
        z1 = z2 * 1.5f + z3;
        w1 = w2 * 2.5 + w3;
        k1 = k2 << 1 | k3;
        p1 = r1 ^ r2;
        q1 = z1 * 0.9f;
        e1 = w1 * 1.1;
        goto state_end;
        
        state1:
        r3 = s2 * 3 - s1;
        r4 = s4 / 3 - s3;
        z2 = z3 * 2.5f - z1;
        w2 = w3 * 3.5 - w1;
        k2 = k3 << 2 ^ k4;
        p2 = r3 | r4;
        q2 = z2 / 0.9f;
        e2 = w2 / 1.1;
        goto state_end;
        
        state2:
        r5 = s3 * 4 ^ s2;
        r6 = s1 / 4 ^ s4;
        z3 = z4 * 3.5f + z2;
        w3 = w4 * 4.5 + w2;
        k3 = k4 << 3 & k1;
        p3 = r5 & r6;
        q3 = z3 * 1.1f;
        e3 = w3 * 0.9;
        goto state_end;
        
        state3:
        r7 = s4 * 5 | s3;
        r8 = s2 / 5 | s1;
        z4 = z1 * 4.5f - z3;
        w4 = w1 * 5.5 - w3;
        k4 = k1 << 4 | k2;
        p4 = r7 ^ r8;
        q4 = z4 / 1.1f;
        e4 = w4 / 0.9;
        goto state_end;
        
        state4:
        s1 = s1 ^ r1;
        s2 = s2 | r2;
        z1 = z1 + q1;
        w1 = w1 - e1;
        k1 = k1 + p1;
        p5 = s1 * r1;
        goto state_end;
        
        state5:
        s3 = s3 ^ r3;
        s4 = s4 | r4;
        z2 = z2 + q2;
        w2 = w2 - e2;
        k2 = k2 + p2;
        p6 = s3 * r3;
        goto state_end;
        
        state6:
        s1 = s1 & r5;
        s2 = s2 ^ r6;
        z3 = z3 + q3;
        w3 = w3 - e3;
        k3 = k3 + p3;
        p7 = s1 + r5;
        goto state_end;
        
        state7:
        s3 = s3 & r7;
        s4 = s4 ^ r8;
        z4 = z4 + q4;
        w4 = w4 - e4;
        k4 = k4 + p4;
        p8 = s3 + r7;
        goto state_end;
        
        state_end:
        /* Mix all values */
        s1 = s1 + (i % 256);
        s2 = s2 - (i % 128);
        s3 = s3 ^ (i % 64);
        s4 = s4 | (i % 32);
        
        /* Long dependency chain */
        z1 = z1 * 1.01f + z2;
        z2 = z2 * 0.99f - z3;
        z3 = z3 * 1.02f + z4;
        z4 = z4 * 0.98f - z1;
        
        w1 = w1 * 1.03 + w2;
        w2 = w2 * 0.97 - w3;
        w3 = w3 * 1.04 + w4;
        w4 = w4 * 0.96 - w1;
        
        k1 = k1 + (k2 >> 1);
        k2 = k2 - (k3 << 1);
        k3 = k3 ^ (k4 >> 2);
        k4 = k4 | (k1 << 2);
        
        checksum += (uint64_t)(s1 + s2 + s3 + s4);
        checksum += (uint64_t)(z1 + z2 + z3 + z4 + w1 + w2 + w3 + w4);
        checksum += k1 + k2 + k3 + k4;
        checksum += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
        checksum += p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8;
    }
    
    /* Mark variables as used */
    asm volatile("" : : "r"(s1), "r"(s2), "r"(s3), "r"(s4));
    asm volatile("" : : "r"(z1), "r"(z2), "r"(z3), "r"(z4));
    asm volatile("" : : "r"(w1), "r"(w2), "r"(w3), "r"(w4));
    
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
    
    /* Run all test functions */
    uint64_t checksum1 = test_irreducible_goto(iterations, seed);
    uint64_t checksum2 = test_switch_goto(iterations, seed + 1);
    uint64_t checksum3 = test_computed_goto(iterations, seed + 2);
    
    /* Aggregate and print final checksum */
    uint64_t total = checksum1 + checksum2 + checksum3;
    printf("Checksum: %lu\n", (unsigned long)total);
    
    return 0;
}
