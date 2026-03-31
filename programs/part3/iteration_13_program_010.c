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
    return min + ((float)(lcg & 0x7FFFFFFF) / (float)0x7FFFFFFF) * (max - min);
}

int main() {
    /* Declare arrays with different types */
    int arr_int[N];
    unsigned int arr_uint[N];
    float arr_float[N];
    double arr_double[N];
    signed char arr_char[N];
    
    int out1[N], out2[N], out3[N], out4[N];
    float fout1[N], fout2[N];
    double dout[N];
    
    /* Initialize arrays with deterministic values */
    for (int i = 0; i < N; i++) {
        arr_int[i] = rand_int(-100, 100);
        arr_uint[i] = (unsigned int)rand_int(0, 200);
        arr_float[i] = rand_float(-50.0f, 50.0f);
        arr_double[i] = (double)rand_float(-100.0f, 100.0f);
        arr_char[i] = (signed char)rand_int(-128, 127);
    }
    
    /* Loop 1: GT_EXPR (>) with integer mask pattern */
    int threshold1 = 25;
    for (int i = 0; i < N; i++) {
        /* Pattern: mask = (arr_int[i] > threshold1) ? arr_int[i] : 0 */
        out1[i] = (arr_int[i] > threshold1) ? arr_int[i] : 0;
        
        /* Additional GT_EXPR usage with logical AND */
        if (arr_int[i] > threshold1 && arr_uint[i] > 50) {
            out1[i] *= 2;
        }
    }
    
    /* Loop 2: GE_EXPR (>=) with floating-point mask pattern */
    float limit = 10.5f;
    for (int i = 0; i < N; i++) {
        /* Pattern: mask = (arr_float[i] >= limit) ? arr_float[i] : limit */
        fout1[i] = (arr_float[i] >= limit) ? arr_float[i] : limit;
        
        /* Nested GE_EXPR with logical OR */
        if (arr_float[i] >= limit || arr_float[i] <= -limit) {
            fout1[i] += 1.0f;
        }
    }
    
    /* Loop 3: LT_EXPR (<) with signed char and mixed operations */
    signed char bound = -10;
    for (int i = 0; i < N; i++) {
        /* Pattern: mask = (arr_char[i] < bound) ? 100 : arr_char[i] */
        out2[i] = (arr_char[i] < bound) ? 100 : arr_char[i];
        
        /* Complex condition with LT_EXPR */
        if (arr_char[i] < bound && arr_int[i] < 0) {
            out2[i] = -out2[i];
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with double precision and if-else */
    double cap = 75.0;
    for (int i = 0; i < N; i++) {
        /* Using if-else instead of ternary to create mask pattern */
        if (arr_double[i] <= cap) {
            dout[i] = arr_double[i];
        } else {
            dout[i] = cap;
        }
        
        /* Additional LE_EXPR in while-like computation */
        int j = 0;
        while (j < 4 && arr_double[i] <= cap * (j + 1)) {
            dout[i] += 0.5;
            j++;
        }
    }
    
    /* Loop 5: Mixed comparisons with unsigned integers (GE_EXPR and LE_EXPR) */
    unsigned int low = 25, high = 175;
    for (int i = 0; i < N; i++) {
        /* Pattern: (arr_uint[i] >= low && arr_uint[i] <= high) */
        out3[i] = (arr_uint[i] >= low && arr_uint[i] <= high) ? 
                  (int)arr_uint[i] : -1;
    }
    
    /* Loop 6: Nested conditionals with GT_EXPR and LT_EXPR */
    int low2 = -50, high2 = 50;
    for (int i = 0; i < N; i++) {
        /* Pattern: (arr_int[i] > low2 && arr_int[i] < high2) */
        if (arr_int[i] > low2) {
            if (arr_int[i] < high2) {
                out4[i] = arr_int[i] * 2;
            } else {
                out4[i] = arr_int[i];
            }
        } else {
            out4[i] = 0;
        }
    }
    
    /* Loop 7: Floating-point comparisons with GE_EXPR and LE_EXPR in mask */
    float range_low = -20.0f, range_high = 20.0f;
    for (int i = 0; i < N; i++) {
        /* Direct mask computation */
        fout2[i] = (arr_float[i] >= range_low && arr_float[i] <= range_high) ? 
                   arr_float[i] : 0.0f;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i];
        checksum += (long long)fout1[i];
        checksum += (long long)fout2[i];
        checksum += (long long)dout[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
