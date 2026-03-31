/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow algorithm
   with fixup graph creation and debug output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining to maintain complex CFG */
#define NOINLINE __attribute__((noinline))

/* Helper to prevent optimization */
static inline void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* Function 1: Irreducible loop with goto jumps */
NOINLINE static uint64_t test_irreducible_goto(int iterations, int seed) {
    /* Many local variables to create register pressure */
    volatile int a1 = seed + 1, a2 = seed + 2, a3 = seed + 3, a4 = seed + 4;
    volatile int b1 = seed + 5, b2 = seed + 6, b3 = seed + 7, b4 = seed + 8;
    volatile float f1 = seed * 1.1f, f2 = seed * 1.2f, f3 = seed * 1.3f;
    volatile double d1 = seed * 2.1, d2 = seed * 2.2, d3 = seed * 2.3;
    volatile long l1 = seed * 3L, l2 = seed * 4L, l3 = seed * 5L;
    volatile int c1 = 0, c2 = 0, c3 = 0, c4 = 0, c5 = 0, c6 = 0;
    volatile int x1 = 0, x2 = 0, x3 = 0, x4 = 0, x5 = 0, x6 = 0;
    volatile float y1 = 0.0f, y2 = 0.0f, y3 = 0.0f;
    volatile double z1 = 0.0, z2 = 0.0;
    
    uint64_t sum = 0;
    int i = 0;
    
    /* Complex irreducible loop structure */
    loop_start:
    if (i >= iterations) goto loop_end;
    
    /* Arithmetic chains to keep variables live */
    a1 = a2 + a3; a2 = a3 + a4; a3 = a4 + a1; a4 = a1 + a2;
    b1 = b2 * b3; b2 = b3 / (b4 ? b4 : 1); b3 = b4 - b1; b4 = b1 ^ b2;
    f1 = f2 + f3; f2 = f3 * 2.0f; f3 = f1 - f2;
    d1 = d2 * 1.5; d2 = d3 / 3.0; d3 = d1 + d2;
    l1 = l2 << 2; l2 = l3 >> 1; l3 = l1 | l2;
    
    /* Create irreducible region with goto */
    if ((i & 3) == 0) goto block_a;
    if ((i & 3) == 1) goto block_b;
    if ((i & 3) == 2) goto block_c;
    goto block_d;
    
    block_a:
    c1 = a1 * b1; c2 = a2 + b2; c3 = a3 ^ b3;
    x1 = f1 > 0 ? 1 : 0; x2 = f2 < 0 ? 1 : 0;
    y1 = f1 * 0.5f; y2 = f2 + 1.0f;
    if ((i & 7) == 0) goto block_b;
    goto block_c;
    
    block_b:
    c4 = b1 - a1; c5 = b2 * a2; c6 = b3 | a3;
    x3 = d1 > 0.0 ? 1 : 0; x4 = d2 < 0.0 ? 1 : 0;
    z1 = d1 * 0.25; z2 = d2 / 2.0;
    if ((i & 5) == 0) goto block_d;
    goto block_a;
    
    block_c:
    c1 = c1 ^ c2; c2 = c2 + c3; c3 = c3 * c4;
    x5 = x1 & x2; x6 = x3 | x4;
    y3 = y1 + y2;
    if ((i & 3) == 0) goto block_d;
    goto block_b;
    
    block_d:
    c4 = c4 - c5; c5 = c5 ^ c6; c6 = c6 & c1;
    z1 = z1 + z2; z2 = z2 * 1.1;
    sum += a1 + a2 + a3 + a4 + b1 + b2 + b3 + b4;
    sum += (int)f1 + (int)f2 + (int)f3;
    sum += (int)d1 + (int)d2;
    sum += c1 + c2 + c3 + c4 + c5 + c6;
    sum += x1 + x2 + x3 + x4 + x5 + x6;
    
    i++;
    goto loop_start;
    
    loop_end:
    /* Force all variables to be used */
    use((void*)&a1); use((void*)&f1); use((void*)&d1); use((void*)&l1);
    return sum;
}

/* Function 2: Switch with goto creating complex CFG */
NOINLINE static uint64_t test_switch_goto(int iterations, int seed) {
    /* Another set of many variables */
    volatile int r1 = seed, r2 = seed * 2, r3 = seed * 3, r4 = seed * 4;
    volatile int s1 = seed + 10, s2 = seed + 20, s3 = seed + 30, s4 = seed + 40;
    volatile float t1 = seed * 0.5f, t2 = seed * 0.6f, t3 = seed * 0.7f;
    volatile double u1 = seed * 0.8, u2 = seed * 0.9, u3 = seed * 1.0;
    volatile long v1 = seed * 100L, v2 = seed * 200L, v3 = seed * 300L;
    volatile int w[8] = {0};
    
    uint64_t sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Long arithmetic chains */
        r1 = r2 + r3; r2 = r3 * r4; r3 = r4 ^ r1; r4 = r1 & r2;
        s1 = s2 - s3; s2 = s3 / (s4 ? s4 : 1); s3 = s4 | s1; s4 = s1 ^ s2;
        t1 = t2 * 1.1f; t2 = t3 + 0.5f; t3 = t1 - t2;
        u1 = u2 / 2.0; u2 = u3 * 1.5; u3 = u1 + u2;
        v1 = v2 << 1; v2 = v3 >> 2; v3 = v1 | v2;
        
        /* Complex switch with goto to create irreducible regions */
        switch (i & 7) {
            case 0:
                w[0] = r1 + s1; w[1] = r2 * s2;
                if (t1 > 0.0f) goto label_x;
                goto label_y;
            case 1:
                w[2] = r3 ^ s3; w[3] = r4 & s4;
                if (u1 < 0.0) goto label_z;
                goto label_x;
            case 2:
                w[4] = (int)t1 + (int)t2; w[5] = (int)t3 * 2;
                goto label_y;
            case 3:
                w[6] = (int)u1 | (int)u2; w[7] = (int)u3 & 0xFF;
                if (v1 > 1000) goto label_z;
                goto label_x;
            case 4:
                w[0] = w[0] ^ w[1]; w[1] = w[1] + w[2];
                goto label_z;
            case 5:
                w[2] = w[2] * w[3]; w[3] = w[3] - w[4];
                goto label_x;
            case 6:
                w[4] = w[4] & w[5]; w[5] = w[5] | w[6];
                goto label_y;
            case 7:
                w[6] = w[6] ^ w[7]; w[7] = w[7] + w[0];
                goto label_z;
        }
        
        label_x:
        w[0] = w[0] + 1; w[1] = w[1] - 1;
        t1 = t1 + 0.1f; t2 = t2 - 0.1f;
        goto continue_loop;
        
        label_y:
        w[2] = w[2] * 2; w[3] = w[3] / 2;
        u1 = u1 * 1.1; u2 = u2 / 1.1;
        goto continue_loop;
        
        label_z:
        w[4] = w[4] << 1; w[5] = w[5] >> 1;
        v1 = v1 + 1; v2 = v2 - 1;
        /* Fall through */
        
        continue_loop:
        /* More arithmetic to extend live ranges */
        r1 = r1 ^ w[0]; r2 = r2 + w[1]; r3 = r3 * w[2]; r4 = r4 & w[3];
        s1 = s1 | w[4]; s2 = s2 ^ w[5]; s3 = s3 + w[6]; s4 = s4 - w[7];
        
        sum += r1 + r2 + r3 + r4 + s1 + s2 + s3 + s4;
        sum += (int)t1 + (int)t2 + (int)t3;
        sum += (int)u1 + (int)u2 + (int)u3;
        sum += v1 + v2 + v3;
        for (int j = 0; j < 8; j++) sum += w[j];
    }
    
    use((void*)&r1); use((void*)&t1); use((void*)&u1); use((void*)&v1);
    return sum;
}

/* Function 3: Computed goto state machine */
NOINLINE static uint64_t test_computed_goto(int iterations, int seed) {
    /* Yet another large set of variables */
    volatile int m1 = seed, m2 = seed * 3, m3 = seed * 5, m4 = seed * 7;
    volatile int n1 = seed + 1, n2 = seed + 3, n3 = seed + 5, n4 = seed + 7;
    volatile float o1 = seed * 0.3f, o2 = seed * 0.5f, o3 = seed * 0.7f;
    volatile double p1 = seed * 0.2, p2 = seed * 0.4, p3 = seed * 0.6;
    volatile long q1 = seed * 11L, q2 = seed * 13L, q3 = seed * 17L;
    volatile int state = 0;
    
    /* Labels for computed goto */
    static void* labels[] = { &&state0, &&state1, &&state2, &&state3, 
                            &&state4, &&state5, &&state6, &&state7 };
    
    uint64_t sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Arithmetic operations */
        m1 = m2 + m3; m2 = m3 * m4; m3 = m4 ^ m1; m4 = m1 & m2;
        n1 = n2 - n3; n2 = n3 / (n4 ? n4 : 1); n3 = n4 | n1; n4 = n1 ^ n2;
        o1 = o2 * 1.2f; o2 = o3 + 0.3f; o3 = o1 - o2;
        p1 = p2 / 1.5; p2 = p3 * 2.0; p3 = p1 + p2;
        q1 = q2 << 2; q2 = q3 >> 1; q3 = q1 | q2;
        
        /* Update state based on complex condition */
        state = (m1 ^ n1 ^ (int)o1 ^ (int)p1 ^ q1) & 7;
        
        /* Computed goto - creates very complex CFG */
        goto *labels[state];
        
        state0:
        m1 = m1 + n1; n1 = n1 - m2;
        o1 = o1 * 1.1f; p1 = p1 / 1.1;
        goto after_state;
        
        state1:
        m2 = m2 * n2; n2 = n2 / (m3 ? m3 : 1);
        o2 = o2 + 0.5f; p2 = p2 * 1.5;
        goto after_state;
        
        state2:
        m3 = m3 ^ n3; n3 = n3 & m4;
        o3 = o3 - 0.2f; p3 = p3 + 0.2;
        goto after_state;
        
        state3:
        m4 = m4 | n4; n4 = n4 ^ m1;
        o1 = o2 * o3; p1 = p2 + p3;
        goto after_state;
        
        state4:
        m1 = m1 << 1; n1 = n1 >> 1;
        o2 = o1 / 2.0f; p2 = p1 * 2.0;
        goto after_state;
        
        state5:
        m2 = m2 + 1; n2 = n2 - 1;
        o3 = o2 + o1; p3 = p2 - p1;
        goto after_state;
        
        state6:
        m3 = m3 * 2; n3 = n3 / 2;
        q1 = q1 ^ q2; q2 = q2 | q3;
        goto after_state;
        
        state7:
        m4 = m4 - 1; n4 = n4 + 1;
        q3 = q1 + q2; q1 = q2 - q3;
        /* Fall through */
        
        after_state:
        /* More operations to extend live ranges */
        int tmp1 = m1 + m2 + m3 + m4;
        int tmp2 = n1 * n2 * n3 * (n4 ? n4 : 1);
        float tmp3 = o1 + o2 + o3;
        double tmp4 = p1 * p2 * p3;
        long tmp5 = q1 | q2 | q3;
        
        sum += tmp1 + tmp2 + (int)tmp3 + (int)tmp4 + tmp5;
        sum += state * 31;
    }
    
    use((void*)&m1); use((void*)&o1); use((void*)&p1); use((void*)&q1);
    return sum;
}

int main(int argc, char **argv) {
    int iterations = 10000;
    int seed = 42;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10000;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF coverage test with %d iterations, seed=%d\n", 
           iterations, seed);
    
    uint64_t total = 0;
    
    /* Call all test functions to increase coverage chance */
    total += test_irreducible_goto(iterations, seed);
    total += test_switch_goto(iterations, seed + 1);
    total += test_computed_goto(iterations, seed + 2);
    
    printf("Result checksum: %lu\n", (unsigned long)total);
    
    return 0;
}
