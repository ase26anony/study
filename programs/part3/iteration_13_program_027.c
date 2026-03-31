#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define SEED 42

/* Simple deterministic pseudo-random number generator */
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
    int checksum = 0;
    
    /* Initialize arrays with deterministic values */
    lcg = SEED;
    for (int i = 0; i < N; i++) {
        arr_int[i] = rand_int(-1000, 1000);
        arr_uint[i] = (unsigned int)rand_int(0, 2000);
        arr_float[i] = rand_float(-500.0f, 500.0f);
        arr_double[i] = (double)rand_float(-500.0f, 500.0f);
        arr_char[i] = (signed char)rand_int(-128, 127);
    }
    
    /* Loop 1: GT_EXPR (>) with integer type - creates mask pattern */
    const int threshold_gt = 100;
    for (int i = 0; i < N; i++) {
        /* Ternary operator creates mask: (arr_int[i] > threshold) ? arr_int[i] : 0 */
        out1[i] = (arr_int[i] > threshold_gt) ? arr_int[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with unsigned integer - different type */
    const unsigned int limit_ge = 500;
    for (int i = 0; i < N; i++) {
        /* Using if statement that should be converted to mask */
        if (arr_uint[i] >= limit_ge) {
            out2[i] = arr_uint[i];
        } else {
            out2[i] = limit_ge / 2;
        }
    }
    
    /* Loop 3: LT_EXPR (<) with floating point - mixed operations */
    const float bound_lt = 250.0f;
    for (int i = 0; i < N; i++) {
        /* Complex expression with LT comparison */
        fout1[i] = (arr_float[i] < bound_lt) ? arr_float[i] * 2.0f : arr_float[i];
    }
    
    /* Loop 4: LE_EXPR (<=) with double precision */
    const double cap_le = -100.0;
    for (int i = 0; i < N; i++) {
        /* Nested condition with LE */
        if (arr_double[i] <= cap_le) {
            dout[i] = arr_double[i] * 3.0;
        } else if (arr_double[i] <= 0.0) {
            dout[i] = arr_double[i] * 2.0;
        } else {
            dout[i] = arr_double[i];
        }
    }
    
    /* Loop 5: GT_EXPR with signed char and logical AND */
    const char low = -50;
    const char high = 50;
    for (int i = 0; i < N; i++) {
        /* Combined comparison: (arr_char[i] > low && arr_char[i] < high) */
        out3[i] = (arr_char[i] > low && arr_char[i] < high) ? arr_char[i] * 2 : arr_char[i];
    }
    
    /* Loop 6: GE_EXPR with OR operator */
    const int x = -200;
    const int y = 200;
    for (int i = 0; i < N; i++) {
        /* OR condition: (arr_int[i] <= x || arr_int[i] >= y) */
        out4[i] = (arr_int[i] <= x || arr_int[i] >= y) ? 1 : 0;
    }
    
    /* Loop 7: Mixed comparisons in accumulation pattern */
    float sum = 0.0f;
    const float min_val = -300.0f;
    const float max_val = 300.0f;
    for (int i = 0; i < N; i++) {
        /* Accumulate only if value is within range (>= min AND <= max) */
        if (arr_float[i] >= min_val && arr_float[i] <= max_val) {
            sum += arr_float[i];
        }
    }
    fout2[0] = sum;
    
    /* Loop 8: LT_EXPR with while-style condition simulation */
    int j = 0;
    int temp_out[N/2];
    while (j < N/2) {
        /* Using < in loop condition and in body */
        if (arr_int[j*2] < arr_int[j*2 + 1]) {
            temp_out[j] = arr_int[j*2];
        } else {
            temp_out[j] = arr_int[j*2 + 1];
        }
        j++;
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i];
        checksum += (int)fout1[i];
        if (i < N/2) checksum += temp_out[i];
    }
    checksum += (int)sum;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
