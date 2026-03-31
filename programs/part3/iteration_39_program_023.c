/* Main test driver with hot loop */
#include <stdint.h>
#include <stdio.h>

/* External helper functions */
extern struct DataPair helper1(int a, int b, int c, int d);
extern struct DataPair helper2(float a, float b, double c, double d);
extern struct DataPair helper3(long a, long b, int64_t c, int64_t d);

/* Volatile to prevent optimization */
volatile int loop_counter = 1000;

/* Complex struct to increase register pressure */
struct DataPair {
    int a;
    float b;
    double c;
    long d;
    int64_t e;
};

/* Vector types for wide register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Force pseudo-register creation with complex expression */
__attribute__((noinline))
static struct DataPair test_function(int seed) {
    /* Many local variables of different types */
    int v1 = seed;
    float v2 = seed * 1.5f;
    double v3 = seed * 2.5;
    long v4 = seed * 3L;
    int64_t v5 = seed * 4LL;
    
    /* Vector variables */
    v4si vec1 = {seed, seed + 1, seed + 2, seed + 3};
    v4sf vec2 = {v2, v2 + 1.0f, v2 + 2.0f, v2 + 3.0f};
    v2df vec3 = {v3, v3 + 1.0};
    
    /* More scalars */
    int v6, v7, v8, v9, v10;
    float v11, v12, v13, v14, v15;
    double v16, v17, v18, v19, v20;
    long v21, v22, v23, v24, v25;
    
    /* Complex interdependent computations */
    v6 = v1 * 2 + (int)v2;
    v7 = v6 - v1;
    v8 = v7 * v6;
    v9 = v8 / (v1 + 1);
    v10 = v9 ^ v8;
    
    /* Float chain */
    v11 = v2 * 3.14f;
    v12 = v11 + v2;
    v13 = v12 * v11;
    v14 = v13 / (v2 + 1.0f);
    v15 = v14 - v13;
    
    /* Double chain */
    v16 = v3 * 2.71828;
    v17 = v16 + v3;
    v18 = v17 * v16;
    v19 = v18 / (v3 + 1.0);
    v20 = v19 - v18;
    
    /* Long chain */
    v21 = v4 * 5L;
    v22 = v21 + v4;
    v23 = v22 * v21;
    v24 = v23 / (v4 + 1L);
    v25 = v24 ^ v23;
    
    /* Vector operations mixed with scalars */
    vec1 = vec1 + (v4si){v6, v7, v8, v9};
    vec2 = vec2 * (v4sf){v11, v12, v13, v14};
    vec3 = vec3 + (v2df){v16, v17};
    
    /* Inline assembly to clobber physical registers and increase pressure */
    asm volatile(
        "# Force register pressure\n"
        "mov r0, %0\n"
        "mov r1, %1\n"
        "add r0, r1, r0\n"
        : 
        : "r" (v10), "r" (v15)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* More operations after assembly to force pseudo-register reload */
    v1 = v10 + v25;
    v2 = v15 + (float)v20;
    v3 = v20 + (double)v25;
    
    /* Pattern to trigger specific RTL replacement: 
       a = b + c; d = a * e; (multiple uses of same pseudo-register) */
    int temp1 = v1 + v6;
    int temp2 = temp1 * v7;  /* temp1 used as operand and destination */
    int temp3 = temp2 - temp1; /* temp1 used again */
    
    float ftemp1 = v2 + v11;
    float ftemp2 = ftemp1 * v12;
    float ftemp3 = ftemp2 - ftemp1;
    
    double dtemp1 = v3 + v16;
    double dtemp2 = dtemp1 * v17;
    double dtemp3 = dtemp2 - dtemp1;
    
    /* Extract vector elements to force more pseudo-registers */
    int ve1 = vec1[0] + vec1[1];
    int ve2 = vec1[2] + vec1[3];
    float vf1 = vec2[0] + vec2[1];
    float vf2 = vec2[2] + vec2[3];
    double vd1 = vec3[0];
    double vd2 = vec3[1];
    
    /* Final computation using all temporaries */
    struct DataPair result;
    result.a = v1 + v6 + v7 + v8 + v9 + v10 + temp1 + temp2 + temp3 + ve1 + ve2;
    result.b = v2 + v11 + v12 + v13 + v14 + v15 + ftemp1 + ftemp2 + ftemp3 + vf1 + vf2;
    result.c = v3 + v16 + v17 + v18 + v19 + v20 + dtemp1 + dtemp2 + dtemp3 + vd1 + vd2;
    result.d = v4 + v21 + v22 + v23 + v24 + v25;
    result.e = v5 + (int64_t)result.a + (int64_t)result.b + (int64_t)result.c + result.d;
    
    return result;
}

int main() {
    struct DataPair total = {0, 0.0f, 0.0, 0L, 0LL};
    
    /* Hot loop to trigger optimization passes */
    for (int i = 0; i < loop_counter; i++) {
        struct DataPair result = test_function(i);
        
        /* Use results to prevent elimination */
        total.a += result.a;
        total.b += result.b;
        total.c += result.c;
        total.d += result.d;
        total.e += result.e;
        
        /* Call helper functions to increase inter-procedural pressure */
        if (i % 3 == 0) {
            struct DataPair h1 = helper1(i, i+1, i+2, i+3);
            total.a += h1.a;
            total.b += h1.b;
        }
        if (i % 5 == 0) {
            struct DataPair h2 = helper2(i*1.0f, i*2.0f, i*3.0, i*4.0);
            total.c += h2.c;
            total.d += h2.d;
        }
        if (i % 7 == 0) {
            struct DataPair h3 = helper3(i*10L, i*20L, i*30LL, i*40LL);
            total.e += h3.e;
        }
    }
    
    printf("Result: a=%d, b=%f, c=%lf, d=%ld, e=%lld\n", 
           total.a, total.b, total.c, total.d, total.e);
    
    return total.a > 0 ? 0 : 1;
}
