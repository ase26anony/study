/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-gcse test.c -o test */
/* Additional options for more stress: -O3 -funroll-loops -march=native -fno-schedule-insns */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __OPTIMIZE__
#define USE_ASM_CLOBBER 1
#else
#define USE_ASM_CLOBBER 0
#endif

/* Volatile variables to prevent optimization */
volatile int v1 = 1;
volatile int v2 = 2;
volatile int v3 = 3;
volatile int v4 = 4;
volatile int v5 = 5;
volatile int v6 = 6;
volatile float fv1 = 1.5f;
volatile float fv2 = 2.5f;
volatile float fv3 = 3.5f;

/* External function to create opaque values */
extern int rand(void);

/* Inline assembly to clobber registers and increase pressure */
static inline void clobber_registers(void) {
#if USE_ASM_CLOBBER
    /* Clobber multiple registers to reduce available physical registers */
    asm volatile (
        "nop\n\t"
        : 
        : 
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
    );
#endif
}

/* Complex arithmetic expression creating long dependency chain */
static int complex_expr(int a, int b, int c, int d, int e, int f, int g) {
    /* Multi-step computation with many temporaries */
    int t1 = a * b;
    int t2 = c + d;
    int t3 = e - f;
    int t4 = g % (a + 1);
    int t5 = t1 * t2;
    int t6 = t3 / (t4 + 1);
    int t7 = t5 + t6;
    int t8 = t7 - (a * c);
    int t9 = t8 % (b + 1);
    int t10 = t9 * d;
    
    return t10;
}

/* Floating point version with more register pressure */
static float complex_float_expr(float a, float b, float c, float d, 
                                float e, float f, float g) {
    /* Chain of floating operations */
    float t1 = a * b;
    float t2 = c + d;
    float t3 = e - f;
    float t4 = g / (a + 1.0f);
    float t5 = t1 * t2;
    float t6 = t3 / (t4 + 1.0f);
    float t7 = t5 + t6;
    float t8 = t7 - (a * c);
    float t9 = t8 * d;
    float t10 = t9 / (b + 1.0f);
    
    return t10 * e * f * g;
}

/* Stress function with patterns to trigger early-remat */
int stress_computation(int seed, int iterations) {
    int result = 0;
    
    /* Use volatile variables to force loads/stores */
    int base1 = v1 * v2 + v3;
    int base2 = v4 - v5 * v6;
    
    /* Multi-use temporary value - candidate for rematerialization */
    int multi_use_temp = base1 * base2 + rand() % 100;
    
    /* Loop with volatile control to prevent optimization */
    for (volatile int i = 0; i < iterations; i = i + 1) {
        /* Complex expression creating register pressure */
        int expr1 = complex_expr(
            multi_use_temp + i,
            v1 + i,
            v2 * i,
            v3 - i,
            v4 % (i + 1),
            v5 + multi_use_temp,
            v6 * i
        );
        
        /* Use the temporary in different contexts */
        int use1;
        if (i % 3 == 0) {
            use1 = multi_use_temp * expr1;
        } else if (i % 3 == 1) {
            use1 = multi_use_temp + expr1;
        } else {
            use1 = multi_use_temp - expr1;
        }
        
        /* More complex arithmetic with volatile accesses */
        float fval = complex_float_expr(
            fv1 + i,
            fv2 - i,
            fv3 * i,
            (float)multi_use_temp,
            (float)expr1,
            (float)use1,
            (float)(v1 + v2 + v3)
        );
        
        /* Address computation with multiple offsets - pattern for remat */
        int array[100];
        int *base_ptr = &array[i % 50];
        
        /* Use base pointer with different offsets */
        int val1 = base_ptr[0] + multi_use_temp;
        int val2 = base_ptr[5] * multi_use_temp;
        int val3 = base_ptr[10] - multi_use_temp;
        int val4 = base_ptr[15] / (multi_use_temp + 1);
        
        /* Combine results */
        result += use1 + (int)fval + val1 + val2 + val3 + val4;
        
        /* Clobber registers periodically */
        if (i % 7 == 0) {
            clobber_registers();
        }
        
        /* Opaque function call to prevent analysis */
        int opaque = rand() % 256;
        result ^= opaque;
        
        /* Another multi-use value computed inside loop */
        int inner_temp = (expr1 * opaque) % 1024;
        
        /* Use inner_temp in multiple separated computations */
        result += inner_temp * 2;
        result -= inner_temp / 3;
        result ^= inner_temp % 17;
    }
    
    /* Switch statement to spatially separate uses of multi_use_temp */
    switch (seed % 4) {
        case 0:
            result += multi_use_temp * 2;
            break;
        case 1:
            result += multi_use_temp / 3;
            break;
        case 2:
            result += multi_use_temp % 17;
            break;
        case 3:
            result ^= multi_use_temp;
            break;
    }
    
    return result;
}

/* Second stress function with different patterns */
int stress_computation2(int seed, int iterations) {
    int result = seed;
    
    /* Create long dependency chain with mixed types */
    for (volatile int i = 0; i < iterations; i++) {
        /* Chain of computations that must be kept in registers */
        int a = result + v1;
        int b = a * v2;
        int c = b - v3;
        int d = c % (v4 + 1);
        int e = d ^ v5;
        int f = e + v6;
        int g = f * (i + 1);
        int h = g / (v1 + 1);
        
        /* Use all computed values */
        result = a + b + c + d + e + f + g + h;
        
        /* More register pressure with floating point */
        float fa = (float)a;
        float fb = (float)b;
        float fc = fa * fb;
        float fd = fc / (float)(i + 1);
        result += (int)(fd * 100.0f);
        
        /* Array access pattern that might trigger address remat */
        int arr[64];
        for (int j = 0; j < 8; j++) {
            /* Compute base address once, use with multiple offsets */
            int *ptr = &arr[(i + j) % 64];
            result += ptr[0] + ptr[4] + ptr[8] + ptr[12];
        }
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int seed = time(NULL);
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    srand(seed);
    
    /* Call stress functions multiple times from different contexts */
    int result1 = 0, result2 = 0;
    
    for (int i = 0; i < 3; i++) {
        result1 += stress_computation(seed + i, iterations);
        result2 += stress_computation2(seed + i * 7, iterations / 2);
        
        /* Modify volatile variables between calls */
        v1++;
        v2--;
        fv1 += 0.5f;
    }
    
    int final_result = result1 ^ result2;
    printf("Result: %d (seed: %d, iterations: %d)\n", 
           final_result, seed, iterations);
    
    return final_result % 256;
}
