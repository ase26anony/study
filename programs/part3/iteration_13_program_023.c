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
    return min + ((float)(lcg & 0x7FFF) / 32767.0f) * (max - min);
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
    unsigned int uout[N];
    
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
        /* This should generate GT_EXPR -> BIT_NOT_EXPR, BIT_AND_EXPR pattern */
        out1[i] = (arr_int[i] > threshold1) ? arr_int[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with floating-point mask pattern */
    float limit = 50.0f;
    for (int i = 0; i < N; i++) {
        /* This should generate GE_EXPR -> BIT_NOT_EXPR, BIT_IOR_EXPR pattern */
        fout1[i] = (arr_float[i] >= limit) ? arr_float[i] : limit;
    }
    
    /* Loop 3: LT_EXPR (<) with signed char and nested condition */
    signed char low = -50, high = 50;
    for (int i = 0; i < N; i++) {
        /* Complex predicate that may decompose to LT_EXPR */
        if (arr_char[i] < high && arr_char[i] > low) {
            out2[i] = arr_char[i];
        } else {
            out2[i] = 0;
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with unsigned integer and logical OR */
    unsigned int cap1 = 1000, cap2 = 1500;
    for (int i = 0; i < N; i++) {
        /* LE_EXPR in logical OR context */
        uout[i] = (arr_uint[i] <= cap1 || arr_uint[i] >= cap2) ? arr_uint[i] : 0;
    }
    
    /* Additional loops to ensure all patterns are hit */
    
    /* Loop 5: Mixed GT_EXPR and LT_EXPR in same expression */
    int lower_bound = -200, upper_bound = 200;
    for (int i = 0; i < N; i++) {
        /* Both GT_EXPR and LT_EXPR in && condition */
        out3[i] = (arr_int[i] > lower_bound && arr_int[i] < upper_bound) ? 
                  arr_int[i] * 2 : arr_int[i];
    }
    
    /* Loop 6: LE_EXPR with double precision */
    double dlimit = -100.0;
    for (int i = 0; i < N; i++) {
        /* LE_EXPR with double comparison */
        dout[i] = (arr_double[i] <= dlimit) ? arr_double[i] : dlimit;
    }
    
    /* Loop 7: GE_EXPR with accumulation pattern */
    int sum = 0;
    int ge_threshold = 300;
    for (int i = 0; i < N; i++) {
        /* GE_EXPR controlling accumulation */
        sum += (arr_int[i] >= ge_threshold) ? arr_int[i] : 1;
    }
    
    /* Loop 8: LT_EXPR with ternary in assignment */
    float ftemp[N];
    float flow = -25.0f;
    for (int i = 0; i < N; i++) {
        /* Direct LT_EXPR ternary */
        ftemp[i] = (arr_float[i] < flow) ? arr_float[i] * 2.0f : arr_float[i];
    }
    
    /* Loop 9: GT_EXPR with unsigned comparison */
    unsigned int ugt_threshold = 800;
    for (int i = 0; i < N; i++) {
        /* GT_EXPR with unsigned int */
        out4[i] = (arr_uint[i] > ugt_threshold) ? (int)arr_uint[i] : -1;
    }
    
    /* Loop 10: Complex nested condition with all four operators */
    int a = -400, b = -100, c = 100, d = 400;
    for (int i = 0; i < N; i++) {
        /* Uses all four comparison operators in complex expression */
        int val = arr_int[i];
        if ((val > a && val <= b) || (val >= c && val < d)) {
            fout2[i] = (float)val;
        } else {
            fout2[i] = 0.0f;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i];
        checksum += (long long)uout[i];
        checksum += (long long)fout1[i];
        checksum += (long long)fout2[i];
        checksum += (long long)dout[i];
        checksum += (long long)ftemp[i];
    }
    checksum += sum;
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
