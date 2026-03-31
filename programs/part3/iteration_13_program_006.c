#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define SEED 42

/* Simple deterministic pseudo-random generator */
static unsigned int lcg = SEED;
static inline int rand_int(int min, int max) {
    lcg = lcg * 1103515245 + 12345;
    return min + (lcg % (max - min + 1));
}

static inline float rand_float(float min, float max) {
    lcg = lcg * 1103515245 + 12345;
    return min + ((float)(lcg % 10001) / 10000.0f) * (max - min);
}

int main() {
    /* Declare arrays with different types */
    int arr_int[N];
    unsigned int arr_uint[N];
    float arr_float[N];
    double arr_double[N];
    signed char arr_char[N];
    
    int out1[N], out2[N], out3[N], out4[N];
    float out_float[N];
    double out_double[N];
    
    /* Initialize arrays with deterministic values */
    lcg = SEED;
    for (int i = 0; i < N; i++) {
        arr_int[i] = rand_int(-1000, 1000);
        arr_uint[i] = (unsigned int)rand_int(0, 2000);
        arr_float[i] = rand_float(-500.0f, 500.0f);
        arr_double[i] = (double)rand_float(-500.0f, 500.0f);
        arr_char[i] = (signed char)rand_int(-128, 127);
    }
    
    /* Loop 1: GT_EXPR (>) with integer mask pattern */
    int threshold1 = 100;
    for (int i = 0; i < N; i++) {
        /* Direct ternary with > comparison */
        out1[i] = (arr_int[i] > threshold1) ? arr_int[i] : 0;
        
        /* Additional mask-based computation */
        out1[i] += (arr_uint[i] > 500) ? 10 : -5;
    }
    
    /* Loop 2: GE_EXPR (>=) with floating-point and logical operators */
    float limit = 0.0f;
    int threshold2 = -200;
    for (int i = 0; i < N; i++) {
        /* Combined condition with >= and logical OR */
        if (arr_float[i] >= limit || arr_int[i] >= threshold2) {
            out_float[i] = arr_float[i] * 2.0f;
        } else {
            out_float[i] = arr_float[i] * 0.5f;
        }
        
        /* Nested conditional with >= */
        out2[i] = (arr_char[i] >= 0) ? 
                  ((arr_int[i] >= -100) ? arr_int[i] : 0) : 
                  -arr_int[i];
    }
    
    /* Loop 3: LT_EXPR (<) with double precision and complex mask */
    double bound = 250.0;
    int low = -300;
    for (int i = 0; i < N; i++) {
        /* Mask selection based on < comparison */
        out_double[i] = (arr_double[i] < bound) ? 
                       arr_double[i] : 
                       bound;
        
        /* Complex predicate with < and logical AND */
        if (arr_int[i] < 0 && arr_double[i] < 0) {
            out3[i] = arr_int[i] * 2;
        } else if (arr_int[i] < 100) {
            out3[i] = arr_int[i];
        } else {
            out3[i] = 0;
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with unsigned and mixed comparisons */
    unsigned int cap = 1500;
    int high = 800;
    for (int i = 0; i < N; i++) {
        /* Multiple <= comparisons in same loop */
        out4[i] = (arr_uint[i] <= cap) ? (int)arr_uint[i] : (int)cap;
        
        /* Nested conditionals with <= */
        if (arr_int[i] <= high) {
            if (arr_float[i] <= 400.0f) {
                out4[i] += (int)(arr_float[i]);
            }
        }
        
        /* Combined condition with <= and >= */
        int temp = (arr_int[i] <= 500 && arr_int[i] >= -500) ? 
                   arr_int[i] : 0;
        out4[i] += temp;
    }
    
    /* Additional loop with all four comparisons in different contexts */
    int results[N];
    for (int i = 0; i < N; i++) {
        int val = 0;
        
        /* Chain of comparisons using all four operators */
        if (arr_int[i] > 0) val += 1;
        if (arr_uint[i] >= 1000) val += 2;
        if (arr_char[i] < 0) val += 4;
        if (arr_int[i] <= -500) val += 8;
        
        /* Mask-based assignment using ternary with mixed operators */
        results[i] = (arr_float[i] > 0.0f) ? 
                    ((arr_double[i] <= 0.0) ? val : val * 2) : 
                    ((arr_int[i] >= 0) ? val / 2 : 0);
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i];
        checksum += (long long)out_float[i];
        checksum += (long long)out_double[i];
        checksum += results[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
