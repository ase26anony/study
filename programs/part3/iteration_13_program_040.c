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
    return min + ((float)(lcg & 0x7FFFFFFF) / 0x7FFFFFFF) * (max - min);
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
        /* Pattern: result = (data > threshold) ? data : 0 */
        out1[i] = (arr_int[i] > threshold1) ? arr_int[i] : 0;
        
        /* Additional GT_EXPR in nested conditional */
        if (arr_int[i] > threshold1 && arr_uint[i] > 500) {
            out1[i] *= 2;
        }
    }
    
    /* Loop 2: GE_EXPR (>=) with floating-point mask pattern */
    float limit = 0.0f;
    for (int i = 0; i < N; i++) {
        /* Pattern: result = (data >= limit) ? data : constant */
        out_float[i] = (arr_float[i] >= limit) ? arr_float[i] : -1.0f;
        
        /* Complex predicate with GE_EXPR */
        if (arr_float[i] >= limit || arr_double[i] >= 0.0) {
            out_float[i] += 0.5f;
        }
    }
    
    /* Loop 3: LT_EXPR (<) with mixed types */
    int bound = -200;
    for (int i = 0; i < N; i++) {
        /* Pattern: if (data < bound) mask-based assignment */
        if (arr_int[i] < bound) {
            out2[i] = arr_int[i] * 2;
        } else {
            out2[i] = arr_int[i] / 2;
        }
        
        /* Nested LT_EXPR with logical AND */
        if (arr_char[i] < 0 && arr_int[i] < threshold1) {
            out2[i] -= 10;
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with unsigned integer */
    unsigned int cap = 1000;
    for (int i = 0; i < N; i++) {
        /* Pattern: result = (data <= cap) ? data : cap */
        out3[i] = (arr_uint[i] <= cap) ? (int)arr_uint[i] : (int)cap;
        
        /* Multiple LE_EXPR comparisons */
        if (arr_uint[i] <= cap && arr_uint[i] <= 1500) {
            out3[i] += 5;
        }
    }
    
    /* Loop 5: Additional mixed comparisons in same loop */
    double low = -250.0, high = 250.0;
    for (int i = 0; i < N; i++) {
        /* Using all four comparisons in one complex predicate */
        int mask = 0;
        if (arr_double[i] > low && arr_double[i] < high) {
            mask |= 1;
        }
        if (arr_double[i] >= 0.0 || arr_double[i] <= 100.0) {
            mask |= 2;
        }
        out_double[i] = (mask > 0) ? arr_double[i] : 0.0;
    }
    
    /* Loop 6: While loop with LE_EXPR condition */
    int counter = 0;
    int values[N];
    for (int i = 0; i < N; i++) values[i] = rand_int(0, 100);
    
    while (counter <= N-1) {  /* LE_EXPR in loop condition */
        if (values[counter] > 50) {  /* GT_EXPR inside loop */
            out4[counter] = values[counter] * 3;
        } else if (values[counter] >= 25) {  /* GE_EXPR */
            out4[counter] = values[counter] * 2;
        } else if (values[counter] < 10) {  /* LT_EXPR */
            out4[counter] = 0;
        } else {
            out4[counter] = values[counter];
        }
        counter++;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i];
        checksum += (long long)out_float[i];
        checksum += (long long)out_double[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
