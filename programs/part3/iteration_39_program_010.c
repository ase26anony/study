/* Main test driver with hot loop to force register pressure */
#include <stdint.h>
#include <stdio.h>

/* Volatile to prevent optimization */
volatile int loop_count = 1000;

/* Forward declarations for helper functions */
struct MultiReg {
    int a, b, c, d;
    float e, f;
    double g, h;
};

struct MultiReg __attribute__((noinline)) helper1(int a, int b, float c, double d);
struct MultiReg __attribute__((noinline)) helper2(long a, long b, float c, double d);
struct MultiReg __attribute__((noinline)) helper3(int a, int b, int c, int d, int e, int f);

/* Vector types for additional register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Test function with extreme register pressure */
int __attribute__((noinline)) test_function(int seed) {
    /* Many local variables of different types */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d1, d2, d3, d4, d5, d6;
    long l1, l2, l3, l4, l5;
    v4si v1, v2, v3, v4;
    v4sf vf1, vf2, vf3;
    v2df vd1, vd2;
    
    /* Initialize with seed to prevent constant propagation */
    a1 = seed;
    f1 = seed * 1.5f;
    d1 = seed * 2.5;
    l1 = seed * 3L;
    
    /* Complex chain of interdependent computations */
    /* This creates many pseudo-registers with multiple uses */
    a2 = a1 + 1;
    a3 = a2 * 2;
    a4 = a3 - a1;      /* a3 used as operand */
    a5 = a4 / 2;       /* a4 used as operand */
    a6 = a5 | a3;      /* a5 and a3 used as operands */
    a7 = a6 & a2;      /* a6 used as operand */
    a8 = a7 ^ a4;      /* a7 used as operand */
    a9 = a8 << 2;      /* a8 used as operand */
    a10 = a9 >> 1;     /* a9 used as operand */
    
    /* Floating point chain */
    f2 = f1 + 1.0f;
    f3 = f2 * 2.0f;
    f4 = f3 - f1;      /* f3 used as operand */
    f5 = f4 / 2.0f;    /* f4 used as operand */
    f6 = f5 + f3;      /* f5 and f3 used as operands */
    f7 = f6 * f2;      /* f6 used as operand */
    f8 = f7 - f4;      /* f7 used as operand */
    
    /* Double precision chain */
    d2 = d1 + 1.0;
    d3 = d2 * 2.0;
    d4 = d3 - d1;      /* d3 used as operand */
    d5 = d4 / 2.0;     /* d4 used as operand */
    d6 = d5 + d3;      /* d5 and d3 used as operands */
    
    /* Long integer chain */
    l2 = l1 + 1L;
    l3 = l2 * 2L;
    l4 = l3 - l1;      /* l3 used as operand */
    l5 = l4 / 2L;      /* l4 used as operand */
    
    /* Vector operations - use wide registers */
    v1 = (v4si){a1, a2, a3, a4};
    v2 = (v4si){a5, a6, a7, a8};
    v3 = v1 + v2;
    v4 = v3 * v1;      /* v3 used as operand */
    
    vf1 = (v4sf){f1, f2, f3, f4};
    vf2 = (v4sf){f5, f6, f7, f8};
    vf3 = vf1 + vf2;
    
    vd1 = (v2df){d1, d2};
    vd2 = (v2df){d3, d4};
    vd1 = vd1 + vd2;
    
    /* Inline assembly to clobber physical registers and force spilling */
    /* Clobber multiple registers to increase pressure */
    asm volatile(
        "# Force register pressure\n"
        "mov r0, %0\n"
        "mov r1, %1\n"
        "add r0, r1, r0\n"
        : 
        : "r" (a10), "r" (l5)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* More interdependent operations after assembly */
    int b1 = a10 + l5;
    int b2 = b1 * a9;
    int b3 = b2 - a8;
    int b4 = b3 / a7;
    int b5 = b4 | a6;
    int b6 = b5 & a5;
    int b7 = b6 ^ a4;
    int b8 = b7 << a3;
    int b9 = b8 >> a2;
    int b10 = b9 + a1;
    
    /* Call helper functions to create cross-function pressure */
    struct MultiReg mr1 = helper1(a1, a2, f1, d1);
    struct MultiReg mr2 = helper2(l1, l2, f2, d2);
    struct MultiReg mr3 = helper3(b1, b2, b3, b4, b5, b6);
    
    /* Final computation using all temporaries */
    int result = a10 + b10 + (int)f8 + (int)d6 + (int)l5;
    result += mr1.a + mr1.b + mr1.c + mr1.d;
    result += mr2.a + mr2.b;
    result += mr3.a + mr3.b + mr3.c + mr3.d;
    
    /* Use vector results */
    int vsum = v3[0] + v3[1] + v3[2] + v3[3];
    result += vsum;
    
    return result;
}

int main() {
    int total = 0;
    int count = loop_count;  /* Use volatile to prevent optimization */
    
    /* Hot loop to trigger register pressure and rematerialization */
    for (int i = 0; i < count; i++) {
        /* Vary seed to prevent constant propagation */
        int seed = i * 3 + 1;
        total += test_function(seed);
        
        /* Additional computation in loop to increase pressure */
        if (i % 7 == 0) {
            total -= test_function(seed / 2);
        }
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
