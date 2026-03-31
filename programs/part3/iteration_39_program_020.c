/* Main test file to trigger early rematerialization register replacement */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Volatile to prevent optimization */
volatile int g_iterations = 1000;

/* Struct to force register pressure across function boundaries */
struct MultiReg {
    int a, b, c, d;
    float e, f;
    double g, h;
};

/* Vector types for wide register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* NOINLINE helper functions to prevent inlining */
__attribute__((noinline)) 
struct MultiReg helper1(int a, int b, float c, double d) {
    struct MultiReg result;
    result.a = a + b;
    result.b = a * b;
    result.c = a - b;
    result.d = b - a;
    result.e = c * 2.0f;
    result.f = c / 2.0f;
    result.g = d * 3.0;
    result.h = d / 3.0;
    
    /* Force register clobbering */
    asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "memory");
    return result;
}

__attribute__((noinline))
struct MultiReg helper2(struct MultiReg x, struct MultiReg y) {
    struct MultiReg result;
    result.a = x.a + y.a;
    result.b = x.b * y.b;
    result.c = x.c - y.c;
    result.d = x.d ^ y.d;
    result.e = x.e + y.e;
    result.f = x.f * y.f;
    result.g = x.g - y.g;
    result.h = x.h / y.h;
    
    /* More register clobbering */
    asm volatile("" : : : "r6", "r7", "r8", "r9", "r10", "memory");
    return result;
}

__attribute__((noinline))
v4si vector_op(v4si a, v4si b, v4si c) {
    /* Complex vector operations */
    v4si t1 = a + b;
    v4si t2 = b * c;
    v4si t3 = t1 - t2;
    v4si t4 = t2 ^ t1;
    v4si t5 = t3 & t4;
    
    /* Mix with scalar operations */
    int* p1 = (int*)&t1;
    int* p2 = (int*)&t2;
    for (int i = 0; i < 4; i++) {
        p1[i] = p1[i] + p2[i] * (i + 1);
    }
    
    return t1 + t2 + t3 + t4 + t5;
}

/* The main test function with high register pressure */
__attribute__((noinline, optimize("O3")))
int test_function(int seed) {
    /* Declare many local variables of different types */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d1, d2, d3, d4, d5, d6;
    long l1, l2, l3, l4, l5;
    
    /* Vector variables */
    v4si v1, v2, v3, v4, v5;
    v4sf vf1, vf2, vf3;
    v2df vd1, vd2;
    
    /* Initialize with seed */
    a1 = seed;
    a2 = seed * 2;
    a3 = seed + 1;
    a4 = seed - 1;
    a5 = seed * 3;
    a6 = seed / 2;
    a7 = seed ^ 0x55;
    a8 = seed | 0xAA;
    a9 = seed & 0xFF;
    a10 = seed << 2;
    
    f1 = seed * 1.0f;
    f2 = seed * 2.0f;
    f3 = seed * 3.0f;
    f4 = seed * 4.0f;
    f5 = seed * 5.0f;
    f6 = seed * 6.0f;
    f7 = seed * 7.0f;
    f8 = seed * 8.0f;
    
    d1 = seed * 1.0;
    d2 = seed * 2.0;
    d3 = seed * 3.0;
    d4 = seed * 4.0;
    d5 = seed * 5.0;
    d6 = seed * 6.0;
    
    l1 = seed * 100L;
    l2 = seed * 200L;
    l3 = seed * 300L;
    l4 = seed * 400L;
    l5 = seed * 500L;
    
    /* Initialize vectors */
    v1 = (v4si){seed, seed+1, seed+2, seed+3};
    v2 = (v4si){seed+4, seed+5, seed+6, seed+7};
    v3 = (v4si){seed+8, seed+9, seed+10, seed+11};
    v4 = (v4si){seed+12, seed+13, seed+14, seed+15};
    v5 = (v4si){seed+16, seed+17, seed+18, seed+19};
    
    vf1 = (v4sf){f1, f2, f3, f4};
    vf2 = (v4sf){f5, f6, f7, f8};
    vf3 = vf1 + vf2;
    
    vd1 = (v2df){d1, d2};
    vd2 = (v2df){d3, d4};
    
    /* Complex chain of interdependent computations */
    /* This creates many pseudo-registers with multiple uses */
    
    /* Chain 1: Integer operations */
    a1 = a2 + a3;           /* Pseudo-reg for a1 created */
    a4 = a1 * a5;           /* a1 used here - potential for remat */
    a6 = a4 - a7;
    a8 = a6 ^ a9;
    a10 = a8 | a1;          /* a1 used again */
    
    /* Chain 2: Float operations with integer dependencies */
    f1 = (float)a1 + f2;    /* a1 used here */
    f3 = f1 * f4;
    f5 = f3 - f6;
    f7 = f5 / f8;
    f2 = f7 + (float)a4;    /* a4 used here */
    
    /* Chain 3: Double operations */
    d1 = (double)f1 + d2;   /* f1 used here */
    d3 = d1 * d4;
    d5 = d3 - d6;
    d2 = d5 / d1;           /* d1 used here */
    
    /* Chain 4: Long operations */
    l1 = (long)a10 * l2;    /* a10 used here */
    l3 = l1 + l4;
    l5 = l3 - l2;
    l2 = l5 ^ l1;           /* l1 used here */
    
    /* Chain 5: Vector operations mixed with scalars */
    v1 = v2 + v3;
    v4 = v1 * v5;           /* v1 used here */
    v2 = v4 - v3;
    v5 = v2 ^ v1;           /* v1 used again */
    
    /* Mix vector and scalar */
    int* vp = (int*)&v1;
    for (int i = 0; i < 4; i++) {
        vp[i] = vp[i] + a1 + i;  /* a1 used in loop */
    }
    
    /* More complex dependencies */
    a2 = a1 * 2;            /* a1 used */
    a3 = a2 + a4;           /* a2 and a4 used */
    a5 = a3 - a6;
    a7 = a5 ^ a8;
    a9 = a7 | a10;
    
    f4 = f1 * 2.0f;         /* f1 used */
    f6 = f4 + f3;
    f8 = f6 - f7;
    f2 = f8 / f5;
    
    /* Call helper functions to increase inter-procedural pressure */
    struct MultiReg r1 = helper1(a1, a2, f1, d1);
    struct MultiReg r2 = helper1(a3, a4, f2, d2);
    struct MultiReg r3 = helper2(r1, r2);
    
    /* More vector operations */
    v3 = vector_op(v1, v2, v4);
    
    /* Artificial register pressure with inline assembly */
    asm volatile(
        "/* Clobber many registers to force spilling */\n\t"
        : : : 
        "r0", "r1", "r2", "r3", "r4", "r5",
        "r6", "r7", "r8", "r9", "r10", "r11",
        "r12", "r13", "r14", "r15",
        "memory"
    );
    
    /* Final computation using all variables to ensure they're live */
    int result = 
        a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
        (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6 + (int)f7 + (int)f8 +
        (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 + (int)d6 +
        (int)l1 + (int)l2 + (int)l3 + (int)l4 + (int)l5 +
        r1.a + r1.b + r1.c + r1.d + (int)r1.e + (int)r1.f + (int)r1.g + (int)r1.h +
        r2.a + r2.b + r2.c + r2.d + (int)r2.e + (int)r2.f + (int)r2.g + (int)r2.h +
        r3.a + r3.b + r3.c + r3.d + (int)r3.e + (int)r3.f + (int)r3.g + (int)r3.h;
    
    /* Sum vector elements */
    int* v1p = (int*)&v1;
    int* v2p = (int*)&v2;
    int* v3p = (int*)&v3;
    int* v4p = (int*)&v4;
    int* v5p = (int*)&v5;
    
    for (int i = 0; i < 4; i++) {
        result += v1p[i] + v2p[i] + v3p[i] + v4p[i] + v5p[i];
    }
    
    return result;
}

int main() {
    int total = 0;
    int iterations = g_iterations;
    
    printf("Starting early rematerialization test...\n");
    
    for (int i = 0; i < iterations; i++) {
        /* Call test function repeatedly */
        total += test_function(i);
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %d\n", total);
    printf("Test completed.\n");
    
    return total != 0 ? 0 : 1;
}
