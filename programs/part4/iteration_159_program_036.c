/* Test program for GCC reorg.cc delay slot filling */
#include <stdio.h>
#include <stdlib.h>

volatile int g_volatile = 0;
int g_accumulator = 0;

/* Optimization barrier */
static int __attribute__((noinline)) barrier(int x) {
    return x ^ 0x55AA55AA;
}

/* Test function 1: Simple conditional jump with safe arithmetic after label */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
static int test_case_1(int a, int b) {
    int temp1 = a + 100;
    int temp2 = b * 2;
    int temp3 = 0;
    int temp4 = 0;
    
    /* Create control flow complexity */
    if (a > 0) {
        temp1 = barrier(temp1);
    }
    
    /* Target pattern: simple conditional jump to label */
    if (a != b) {
        /* Use volatile to prevent constant folding */
        if (g_volatile >= 0) {
            goto target_label_1;
        }
    }
    
    /* Fall-through path */
    temp3 = temp1 & 0xFF;
    return temp3;
    
target_label_1:
    /* Safe, non-jump instruction that can be moved to delay slot */
    /* Uses independent temporaries not used in jump condition */
    temp4 = temp2 + 5;  /* Simple arithmetic, no traps */
    
    /* Use result to prevent elimination */
    return temp4 | 0x1;
}

/* Test function 2: Different variable pattern */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
static int test_case_2(int x, int y) {
    int local_a = x * 3;
    int local_b = y + 7;
    int local_c = 0;
    int local_d = 0;
    
    /* Some preceding code */
    for (int i = 0; i < 2; i++) {
        local_a += i;
    }
    
    /* Another conditional to create basic blocks */
    if (local_a > 10) {
        local_b = barrier(local_b);
    }
    
    /* Target jump pattern */
    if (x < y) {
        if (g_volatile != 1) {  /* Volatile prevents optimization */
            goto target_label_2;
        }
    }
    
    local_c = local_a >> 1;
    return local_c;
    
target_label_2:
    /* Safe instruction using independent variable */
    local_d = local_b ^ 0x3C;  /* Logical operation, no trap */
    
    return local_d & 0x7F;
}

/* Test function 3: More complex surrounding code */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
static int test_case_3(int p, int q) {
    int t1 = p + q;
    int t2 = p - q;
    int t3 = 0;
    int t4 = 0;
    int t5 = 0;
    
    /* Multiple basic blocks */
    switch (p & 3) {
        case 0: t1 += 1; break;
        case 1: t1 += 2; break;
        default: t1 += 3; break;
    }
    
    /* Target jump */
    if ((p ^ q) != 0) {
        /* Use volatile read */
        if (g_volatile < 100) {
            goto target_label_3;
        }
    }
    
    t3 = t1 * 2;
    return t3;
    
target_label_3:
    /* Safe arithmetic with completely independent variable */
    t4 = t2 + 17;  /* t2 not used in jump condition */
    
    /* Another safe operation to create scheduling opportunity */
    t5 = t4 | 0x40;
    
    return t5;
}

/* Test function 4: SPARC-specific pattern */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
static int test_case_4(int m, int n) {
    int r1 = m * m;
    int r2 = n + 42;
    int r3 = 0;
    int r4 = 0;
    
    /* Loop to create more scheduling context */
    for (int j = 0; j < 3; j++) {
        r1 += j;
    }
    
    /* Conditional jump with volatile guard */
    if (m % 2 == 0) {
        if (g_volatile == 0) {
            goto target_label_4;
        }
    }
    
    r3 = r1 & 0x3FF;
    return r3;
    
target_label_4:
    /* Safe bit manipulation */
    r4 = r2 << 1;  /* Shift operation, safe */
    
    return r4 + 1;
}

/* Test function 5: MIPS-specific with multiple temporaries */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
static int test_case_5(int u, int v) {
    int w1 = u | 0x1000;
    int w2 = v & 0x0FFF;
    int w3 = 0;
    int w4 = 0;
    int w5 = 0;
    
    /* Some arithmetic to define variables */
    w5 = u + v;
    
    /* Nested conditionals */
    if (u > v) {
        w1 = barrier(w1);
        if (v != 0) {
            /* Target jump */
            if (g_volatile >= -1) {
                goto target_label_5;
            }
        }
    }
    
    w3 = w5 - 10;
    return w3;
    
target_label_5:
    /* Multiple safe instructions - scheduler might move one */
    w4 = w2 + 23;  /* w2 independent from jump condition */
    
    return w4;
}

int main(void) {
    int result = 0;
    int test_values[][2] = {
        {1, 2}, {3, 3}, {5, 7}, {10, 4}, {0, 8}
    };
    
    /* Initialize volatile */
    g_volatile = rand() % 10;
    
    /* Run test cases multiple times with different inputs */
    for (int i = 0; i < 5; i++) {
        result ^= test_case_1(test_values[i][0], test_values[i][1]);
        result ^= test_case_2(test_values[i][0], test_values[i][1]);
        result ^= test_case_3(test_values[i][0], test_values[i][1]);
        
        /* Architecture-specific tests */
#ifdef __sparc__
        result ^= test_case_4(test_values[i][0], test_values[i][1]);
#endif
#ifdef __mips__
        result ^= test_case_5(test_values[i][0], test_values[i][1]);
#endif
        
        /* Modify volatile to change jump paths */
        g_volatile++;
    }
    
    /* Accumulate results */
    g_accumulator = result;
    
    printf("Result checksum: 0x%08X\n", result);
    return (result == 0) ? 0 : 1;
}
