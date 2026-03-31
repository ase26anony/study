/* Test program to force caller-save insertion during reload */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-omit-frame-pointer -fno-inline -fdump-rtl-reload -fdump-rtl-all caller-save-test.c external.c -o caller-save-test */

#include <stdio.h>
#include <math.h>

/* External functions with side effects - prevent inlining */
extern void unknown_effect1(int, double);
extern void unknown_effect2(float, int, double);
extern void unknown_effect3(double, double, int);
extern int unknown_computation(int, double, float);

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* Function 1: High pressure with mixed integer/float live values */
double high_pressure_call1(int *ints, double *doubles, float *floats) {
    /* Unpack many scalar variables - all must be live across call */
    int v1 = ints[0] * 2 + 1;
    int v2 = ints[1] ^ ints[2];
    int v3 = v1 * v2 - ints[3];
    int v4 = (v2 << 3) | (v3 & 0xFF);
    int v5 = v3 + v4 * 7;
    
    double d1 = doubles[0] * 3.14159;
    double d2 = doubles[1] / 2.71828;
    double d3 = d1 * d2 + doubles[2];
    double d4 = sin(d3) * cos(doubles[3]);
    double d5 = d4 * d3 - d2;
    
    float f1 = floats[0] * 1.5f;
    float f2 = floats[1] + floats[2];
    float f3 = f1 * f2 - floats[3];
    float f4 = f3 / (f2 + 1.0f);
    float f5 = f4 * 2.0f + f1;
    
    /* Additional intermediate values that must stay in registers */
    int v6 = v5 * 3 - v4;
    double d6 = d5 * 2.0 + d4;
    float f6 = f5 * 1.25f;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* External call - clobbers call-used registers */
    unknown_effect1(v3, d3);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Use all live values after call - forcing caller-save restoration */
    double result = (v1 * d1) + (v2 * d2) - (v3 * d3) + (v4 * d4) - (v5 * d5);
    result += (f1 + f2 + f3 + f4 + f5) * 0.5;
    result += v6 * d6 * f6;
    
    /* More computations to increase register pressure */
    for (int i = 0; i < 3; i++) {
        v1 = v1 * 2 + v2;
        d1 = d1 * 1.1 + d2;
        f1 = f1 * 1.05f + f2;
        
        /* Another call inside loop - different pattern */
        if (i % 2 == 0) {
            unknown_effect2(f3, v4, d5);
        }
    }
    
    return result + v1 + d1 + f1;
}

/* Function 2: Different pattern with nested conditionals */
int high_pressure_call2(int *data, double *coeffs, int n) {
    /* Many live variables with complex dependencies */
    int acc1 = data[0];
    int acc2 = data[1];
    int acc3 = data[2];
    int acc4 = data[3];
    int acc5 = data[4];
    
    double sum1 = coeffs[0];
    double sum2 = coeffs[1];
    double sum3 = coeffs[2];
    double sum4 = coeffs[3];
    double sum5 = coeffs[4];
    
    float tmp1 = acc1 * 0.1f;
    float tmp2 = acc2 * 0.2f;
    float tmp3 = acc3 * 0.3f;
    float tmp4 = acc4 * 0.4f;
    float tmp5 = acc5 * 0.5f;
    
    /* Complex conditional with calls at different branches */
    for (int i = 0; i < n; i++) {
        int idx = i % 5;
        
        switch (idx) {
            case 0:
                acc1 = acc1 * 2 + data[i];
                sum1 = sum1 * 1.5 + coeffs[idx];
                unknown_effect1(acc1, sum1);
                tmp1 = tmp1 * 1.1f + tmp2;
                break;
            case 1:
                acc2 = acc2 ^ data[i];
                sum2 = sin(sum2) * coeffs[idx];
                unknown_effect2(tmp2, acc2, sum2);
                tmp2 = tmp2 * 1.2f - tmp3;
                break;
            case 2:
                acc3 = acc3 + data[i] * 3;
                sum3 = cos(sum3) / coeffs[idx];
                /* No call here - different pattern */
                tmp3 = tmp3 * 0.9f + tmp4;
                break;
            case 3:
                acc4 = acc4 | data[i];
                sum4 = sum4 * sum4 - coeffs[idx];
                unknown_effect3(sum4, sum3, acc4);
                tmp4 = tmp4 * 1.3f / tmp5;
                break;
            case 4:
                acc5 = acc5 - data[i];
                sum5 = sqrt(fabs(sum5)) + coeffs[idx];
                /* Another external call */
                asm volatile("" : : : "memory");
                int r = unknown_computation(acc5, sum5, tmp5);
                asm volatile("" : : : "memory");
                acc5 += r;
                tmp5 = tmp5 * 0.8f + tmp1;
                break;
        }
        
        /* Intermediate computation keeping values live */
        if (i % 10 == 0) {
            acc1 = acc1 + acc2 - acc3;
            sum1 = sum1 + sum2 * sum3;
            tmp1 = tmp1 + tmp2 * tmp3;
        }
    }
    
    /* Final computation using all live values */
    int result = acc1 + acc2 + acc3 + acc4 + acc5;
    result += (int)(sum1 + sum2 + sum3 + sum4 + sum5);
    result += (int)(tmp1 + tmp2 + tmp3 + tmp4 + tmp5);
    
    return result;
}

/* Function 3: Deeply nested calls and loops */
float high_pressure_call3(float *base, int iterations) {
    /* Create many float variables */
    float a = base[0];
    float b = base[1];
    float c = base[2];
    float d = base[3];
    float e = base[4];
    float f = base[5];
    float g = base[6];
    float h = base[7];
    float i = base[8];
    float j = base[9];
    
    /* Corresponding double precision versions */
    double da = a * 2.0;
    double db = b * 3.0;
    double dc = c * 4.0;
    double dd = d * 5.0;
    double de = e * 6.0;
    
    /* Integer counters */
    int cnt1 = 0;
    int cnt2 = 0;
    int cnt3 = 0;
    int cnt4 = 0;
    int cnt5 = 0;
    
    /* Complex loop with multiple call sites */
    for (int iter = 0; iter < iterations; iter++) {
        /* Update all variables - keep them live */
        a = a * 1.01f + b;
        b = b * 0.99f - c;
        c = c * 1.02f + d;
        d = d * 0.98f - e;
        e = e * 1.03f + f;
        f = f * 0.97f - g;
        g = g * 1.04f + h;
        h = h * 0.96f - i;
        i = i * 1.05f + j;
        j = j * 0.95f - a;
        
        da = da * 1.1 + db;
        db = db * 0.9 - dc;
        dc = dc * 1.2 + dd;
        dd = dd * 0.8 - de;
        de = de * 1.3 + da;
        
        cnt1 += (int)a;
        cnt2 += (int)b;
        cnt3 += (int)c;
        cnt4 += (int)d;
        cnt5 += (int)e;
        
        /* Call at irregular intervals */
        if (iter % 7 == 0) {
            unknown_effect1(cnt1, da);
        }
        if (iter % 11 == 0) {
            unknown_effect2(b, cnt2, db);
        }
        if (iter % 13 == 0) {
            unknown_effect3(dc, dd, cnt3);
        }
        if (iter % 17 == 0) {
            asm volatile("" : : : "memory");
            int r = unknown_computation(cnt4, de, f);
            asm volatile("" : : : "memory");
            cnt4 += r;
        }
        
        /* Nested loop for additional pressure */
        for (int k = 0; k < 3; k++) {
            float temp = a + b + c + d + e;
            double dtemp = da + db + dc + dd + de;
            
            if (k == 1) {
                unknown_effect2(temp, k, dtemp);
            }
        }
    }
    
    /* Use all variables in final result */
    float result = a + b + c + d + e + f + g + h + i + j;
    result += (float)(da + db + dc + dd + de);
    result += (float)(cnt1 + cnt2 + cnt3 + cnt4 + cnt5);
    
    return result;
}

int main() {
    /* Initialize test data */
    int int_data[20];
    double double_data[20];
    float float_data[20];
    
    for (int i = 0; i < 20; i++) {
        int_data[i] = i * 3 + 1;
        double_data[i] = i * 1.5 + 0.5;
        float_data[i] = i * 0.7f + 0.3f;
    }
    
    double total = 0.0;
    
    /* Loop with multiple high-pressure calls */
    for (int i = 0; i < 100; i++) {
        /* Modify data slightly each iteration */
        int_data[i % 20] += i;
        double_data[i % 20] *= 1.01;
        float_data[i % 20] *= 0.99f;
        
        /* Call different high-pressure functions */
        double r1 = high_pressure_call1(int_data, double_data, float_data);
        int r2 = high_pressure_call2(int_data, double_data, 50);
        float r3 = high_pressure_call3(float_data, 30);
        
        total += r1 + r2 + r3;
        
        /* Update global to prevent dead code elimination */
        global_counter += (int)(r1 + r2 + r3);
    }
    
    printf("Result: %f\n", total);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
