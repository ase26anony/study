/* Complex CFG program to trigger MCF fixup graph debug output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define NUM_VARS 30
#define MAX_ITER 10000

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* State machine labels for computed goto */
static void* states[] = {
    &&state0, &&state1, &&state2, &&state3, 
    &&state4, &&state5, &&state6, &&state7
};

/* Function 1: Irreducible loops with goto jumping across boundaries */
NOINLINE static uint64_t test_irreducible_goto(int iterations, int seed) {
    /* Many local variables to create register pressure */
    VOLATILE_VAR int a0 = seed + 1, a1 = seed + 2, a2 = seed + 3, a3 = seed + 4;
    VOLATILE_VAR int a4 = seed + 5, a5 = seed + 6, a6 = seed + 7, a7 = seed + 8;
    VOLATILE_VAR int a8 = seed + 9, a9 = seed + 10, a10 = seed + 11, a11 = seed + 12;
    VOLATILE_VAR float f0 = seed * 1.1f, f1 = seed * 1.2f, f2 = seed * 1.3f, f3 = seed * 1.4f;
    VOLATILE_VAR double d0 = seed * 2.1, d1 = seed * 2.2, d2 = seed * 2.3, d3 = seed * 2.4;
    VOLATILE_VAR long l0 = seed * 100L, l1 = seed * 200L, l2 = seed * 300L, l3 = seed * 400L;
    VOLATILE_VAR int b0 = 0, b1 = 0, b2 = 0, b3 = 0, b4 = 0, b5 = 0;
    
    uint64_t checksum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex arithmetic chains keeping variables live */
        a0 = a1 + a2 * a3 - a4;
        a1 = a5 ^ a6 | a7 & a8;
        a2 = a9 * a10 / (a11 + 1);
        a3 = a0 << 2 | a1 >> 3;
        
        f0 = f1 * 1.5f + f2 - f3;
        f1 = f0 * 2.0f / (f2 + 0.5f);
        f2 = f3 + f1 * 0.75f;
        f3 = f2 - f0 * 1.25f;
        
        d0 = d1 * 3.14159 + d2 - d3;
        d1 = d0 / 2.71828 * d2;
        d2 = d3 + d1 * 1.61803;
        d3 = d2 - d0 * 0.70711;
        
        l0 = l1 + l2 * l3;
        l1 = l0 ^ l2 | l3;
        l2 = l3 * l0 / (l1 + 1);
        l3 = l2 << 4 | l1 >> 2;
        
        /* Irreducible loop structure with goto */
        if (i % 7 == 0) {
            goto inner_loop1;
        } else if (i % 11 == 0) {
            goto inner_loop2;
        }
        
        b0 = i & 0xFF;
        b1 = (i >> 8) & 0xFF;
        
    outer_loop:
        for (int j = 0; j < 5; j++) {
            b2 = b0 + b1 * j;
            if (b2 % 3 == 0) {
                goto cross_jump;
            }
            b3 = b2 * 2;
        }
        
        continue;
        
    inner_loop1:
        for (int k = 0; k < 3; k++) {
            b4 = a0 + k;
            if (b4 % 5 == 0) {
                goto outer_loop;  /* Jump to outer loop - creates irreducible region */
            }
            b5 = b4 * 3;
        }
        continue;
        
    inner_loop2:
        for (int m = 0; m < 4; m++) {
            b0 = a1 + m;
            if (b0 % 7 == 0) {
                goto inner_loop1;  /* Jump between inner loops */
            }
            b1 = b0 * 4;
        }
        continue;
        
    cross_jump:
        b0 = b3 + 1;
        goto inner_loop2;  /* Jump from inside loop to another inner loop */
    }
    
    /* Aggregate checksum to prevent dead code elimination */
    checksum = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11;
    checksum += (uint64_t)(f0 + f1 + f2 + f3);
    checksum += (uint64_t)(d0 + d1 + d2 + d3);
    checksum += l0 + l1 + l2 + l3;
    checksum += b0 + b1 + b2 + b3 + b4 + b5;
    
    /* Use inline assembly to mark variables as used */
    asm volatile("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3));
    asm volatile("" : : "r"(f0), "r"(f1), "r"(f2), "r"(f3));
    asm volatile("" : : "r"(d0), "r"(d1), "r"(d2), "r"(d3));
    
    return checksum;
}

/* Function 2: Switch with goto creating complex CFG */
NOINLINE static uint64_t test_switch_goto(int iterations, int seed) {
    VOLATILE_VAR int x0 = seed, x1 = seed * 2, x2 = seed * 3, x3 = seed * 4;
    VOLATILE_VAR int x4 = seed * 5, x5 = seed * 6, x6 = seed * 7, x7 = seed * 8;
    VOLATILE_VAR float y0 = seed * 0.5f, y1 = seed * 0.6f, y2 = seed * 0.7f, y3 = seed * 0.8f;
    VOLATILE_VAR double z0 = seed * 0.25, z1 = seed * 0.35, z2 = seed * 0.45, z3 = seed * 0.55;
    VOLATILE_VAR long w0 = seed * 10L, w1 = seed * 20L, w2 = seed * 30L, w3 = seed * 40L;
    VOLATILE_VAR int state = 0;
    
    uint64_t checksum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Long dependency chains */
        x0 = x1 + x2 - x3 * x4;
        x1 = x5 ^ x6 | x7 & x0;
        x2 = x3 * x4 / (x5 + 1);
        x3 = x6 << 3 | x7 >> 1;
        x4 = x0 + x1 * x2 - x3;
        x5 = x4 ^ x6 | x7 & x1;
        x6 = x2 * x3 / (x4 + 1);
        x7 = x5 << 2 | x0 >> 2;
        
        y0 = y1 * 1.1f + y2 - y3;
        y1 = y0 * 1.2f / (y2 + 0.1f);
        y2 = y3 + y1 * 0.9f;
        y3 = y2 - y0 * 1.3f;
        
        z0 = z1 * 1.5 + z2 - z3;
        z1 = z0 / 1.6 * z2;
        z2 = z3 + z1 * 1.7;
        z3 = z2 - z0 * 1.8;
        
        w0 = w1 + w2 * w3;
        w1 = w0 ^ w2 | w3;
        w2 = w3 * w0 / (w1 + 1);
        w3 = w2 << 5 | w1 >> 3;
        
        /* Complex switch with goto to different labels */
        switch (i % 8) {
            case 0:
                x0 += 1;
                goto label_a;
            case 1:
                x1 += 2;
                goto label_b;
            case 2:
                x2 += 3;
                goto label_c;
            case 3:
                x3 += 4;
                goto label_d;
            case 4:
                x4 += 5;
                goto label_a;  /* Jump back to earlier label */
            case 5:
                x5 += 6;
                goto label_c;  /* Jump across cases */
            case 6:
                x6 += 7;
                goto label_b;
            case 7:
                x7 += 8;
                goto label_d;
        }
        
    label_a:
        y0 += 0.1f;
        if (i % 3 == 0) goto label_c;
        continue;
        
    label_b:
        y1 += 0.2f;
        if (i % 5 == 0) goto label_d;
        continue;
        
    label_c:
        y2 += 0.3f;
        if (i % 7 == 0) goto label_a;
        continue;
        
    label_d:
        y3 += 0.4f;
        if (i % 11 == 0) goto label_b;
        continue;
    }
    
    checksum = x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7;
    checksum += (uint64_t)(y0 + y1 + y2 + y3);
    checksum += (uint64_t)(z0 + z1 + z2 + z3);
    checksum += w0 + w1 + w2 + w3;
    
    asm volatile("" : : "r"(x0), "r"(x1), "r"(x2), "r"(x3));
    asm volatile("" : : "r"(y0), "r"(y1), "r"(y2), "r"(y3));
    
    return checksum;
}

/* Function 3: Computed goto state machine */
NOINLINE static uint64_t test_computed_goto(int iterations, int seed) {
    VOLATILE_VAR int r0 = seed, r1 = seed + 1, r2 = seed + 2, r3 = seed + 3;
    VOLATILE_VAR int r4 = seed + 4, r5 = seed + 5, r6 = seed + 6, r7 = seed + 7;
    VOLATILE_VAR int r8 = seed + 8, r9 = seed + 9, r10 = seed + 10, r11 = seed + 11;
    VOLATILE_VAR float s0 = seed * 0.11f, s1 = seed * 0.22f, s2 = seed * 0.33f, s3 = seed * 0.44f;
    VOLATILE_VAR double t0 = seed * 0.111, t1 = seed * 0.222, t2 = seed * 0.333, t3 = seed * 0.444;
    VOLATILE_VAR long u0 = seed * 111L, u1 = seed * 222L, u2 = seed * 333L, u3 = seed * 444L;
    
    void* state_ptr = &&state0;
    uint64_t checksum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Extensive arithmetic operations */
        r0 = r1 + r2 * r3 - r4;
        r1 = r5 ^ r6 | r7 & r8;
        r2 = r9 * r10 / (r11 + 1);
        r3 = r0 << 1 | r1 >> 4;
        r4 = r2 + r3 * r5 - r6;
        r5 = r7 ^ r8 | r9 & r10;
        r6 = r11 * r0 / (r1 + 1);
        r7 = r2 << 3 | r3 >> 2;
        r8 = r4 + r5 * r6 - r7;
        r9 = r8 ^ r9 | r10 & r11;
        r10 = r0 * r1 / (r2 + 1);
        r11 = r3 << 4 | r4 >> 1;
        
        s0 = s1 * 1.01f + s2 - s3;
        s1 = s0 * 1.02f / (s2 + 0.01f);
        s2 = s3 + s1 * 0.99f;
        s3 = s2 - s0 * 1.03f;
        
        t0 = t1 * 1.001 + t2 - t3;
        t1 = t0 / 1.002 * t2;
        t2 = t3 + t1 * 0.999;
        t3 = t2 - t0 * 1.004;
        
        u0 = u1 + u2 * u3;
        u1 = u0 ^ u2 | u3;
        u2 = u3 * u0 / (u1 + 1);
        u3 = u2 << 6 | u1 >> 4;
        
        /* Computed goto based on complex condition */
        int idx = (r0 + r1 + r2 + r3 + i) % 8;
        goto *states[idx];
        
    state0:
        r0 += 1;
        state_ptr = &&state1;
        goto *state_ptr;
        
    state1:
        r1 += 2;
        if (i % 3 == 0) state_ptr = &&state3;
        else state_ptr = &&state2;
        goto *state_ptr;
        
    state2:
        r2 += 3;
        state_ptr = &&state4;
        goto *state_ptr;
        
    state3:
        r3 += 4;
        if (i % 5 == 0) state_ptr = &&state0;
        else state_ptr = &&state5;
        goto *state_ptr;
        
    state4:
        r4 += 5;
        state_ptr = &&state6;
        goto *state_ptr;
        
    state5:
        r5 += 6;
        if (i % 7 == 0) state_ptr = &&state2;
        else state_ptr = &&state7;
        goto *state_ptr;
        
    state6:
        r6 += 7;
        state_ptr = &&state0;
        goto *state_ptr;
        
    state7:
        r7 += 8;
        state_ptr = &&state1;
        continue;  /* Break the computed goto chain */
    }
    
    checksum = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11;
    checksum += (uint64_t)(s0 + s1 + s2 + s3);
    checksum += (uint64_t)(t0 + t1 + t2 + t3);
    checksum += u0 + u1 + u2 + u3;
    
    asm volatile("" : : "r"(r0), "r"(r1), "r"(r2), "r"(r3));
    asm volatile("" : : "r"(s0), "r"(s1), "r"(s2), "r"(s3));
    asm volatile("" : : "r"(t0), "r"(t1), "r"(t2), "r"(t3));
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = MAX_ITER;
    int seed = 42;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = MAX_ITER;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF stress test with %d iterations, seed=%d\n", iterations, seed);
    
    uint64_t total_checksum = 0;
    
    /* Run all test functions to increase coverage chance */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    /* Additional calls with different parameters */
    total_checksum += test_irreducible_goto(iterations / 2, seed + 100);
    total_checksum += test_switch_goto(iterations / 3, seed + 200);
    total_checksum += test_computed_goto(iterations / 4, seed + 300);
    
    printf("Final checksum: %lu\n", total_checksum);
    
    return 0;
}
