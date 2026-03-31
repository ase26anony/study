/* Test program to trigger caller-save insertion during reload */
/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-inline -fno-optimize-sibling-calls -fdump-rtl-all caller-save-test.c external.c -o caller-save-test */

#include <stdio.h>
#include <math.h>

/* External functions that cannot be inlined */
extern void unknown_effect(int a, double b);
extern void another_effect(float f, long l);
extern void memory_barrier(void);

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* Memory barrier to enforce ordering */
void memory_barrier(void) {
    asm volatile("" : : : "memory");
}

/* Function 1: High pressure with mixed integer/float variables */
/* All 15 variables must stay live across the external call */
int high_pressure_call_1(int *ints, double *doubles) {
    /* Unpack into many scalar variables */
    int v1 = ints[0] * 2;
    int v2 = ints[1] + v1;
    int v3 = ints[2] - v2;
    int v4 = ints[3] | v3;
    int v5 = ints[4] & v4;
    
    double d1 = doubles[0] * 3.14159;
    double d2 = doubles[1] + d1;
    double d3 = doubles[2] - d2;
    double d4 = doubles[3] * d3;
    double d5 = doubles[4] / (d4 + 1.0);
    
    float f1 = (float)d1 * 2.5f;
    float f2 = (float)d2 + f1;
    float f3 = (float)d3 - f2;
    
    long l1 = (long)v1 * v2;
    long l2 = (long)v3 + l1;
    
    /* Use all variables in computation before call */
    int pre_sum = v1 + v2 + v3 + v4 + v5;
    double pre_product = d1 * d2 * d3 * d4 * d5;
    float pre_float = f1 + f2 + f3;
    long pre_long = l1 ^ l2;
    
    /* Memory barrier before call */
    memory_barrier();
    
    /* External call that clobbers call-used registers */
    /* This forces caller-save for all live variables */
    unknown_effect(pre_sum, pre_product);
    
    /* Memory barrier after call */
    memory_barrier();
    
    /* Use all variables again after call - they must be restored */
    int post_sum = v5 + v4 + v3 + v2 + v1;
    double post_product = d5 * d4 * d3 * d2 * d1;
    float post_float = f3 + f2 + f1;
    long post_long = l2 | l1;
    
    /* Complex computation mixing all variables */
    double result = (double)post_sum * post_product;
    result += (double)post_float * 100.0;
    result += (double)post_long / 1000.0;
    
    /* Use in conditional to prevent optimization */
    if (result > 1000000.0) {
        return (int)(result / 1000.0);
    } else {
        return (int)result + pre_sum;
    }
}

/* Function 2: Different pattern with more floating point pressure */
double high_pressure_call_2(float *floats, long *longs) {
    /* Even more variables to increase pressure */
    float f1 = floats[0];
    float f2 = floats[1] * f1;
    float f3 = floats[2] + f2;
    float f4 = floats[3] - f3;
    float f5 = floats[4] / f4;
    float f6 = floats[5] * f5;
    float f7 = floats[6] + f6;
    
    long l1 = longs[0];
    long l2 = longs[1] + l1;
    long l3 = longs[2] * l2;
    long l4 = longs[3] | l3;
    long l5 = longs[4] & l4;
    
    double d1 = (double)f1 * 1.234;
    double d2 = (double)f2 + d1;
    double d3 = (double)f3 * d2;
    double d4 = (double)f4 - d3;
    
    int i1 = (int)l1;
    int i2 = (int)l2 + i1;
    int i3 = (int)l3 * i2;
    
    /* Pre-call computation */
    float pre_float_sum = f1 + f2 + f3 + f4 + f5 + f6 + f7;
    long pre_long_prod = l1 * l2 * l3 * l4 * l5;
    double pre_double_avg = (d1 + d2 + d3 + d4) / 4.0;
    
    /* Call with side effect in loop */
    for (int i = 0; i < 3; i++) {
        memory_barrier();
        another_effect(pre_float_sum + i, pre_long_prod);
        memory_barrier();
        
        /* Modify variables slightly to prevent CSE */
        f1 += 0.1f;
        d1 += 0.01;
        l1 += 1;
    }
    
    /* Post-call computation using all variables */
    double result = (double)f1 * d1 * (double)l1;
    result += (double)f2 * d2 * (double)l2;
    result += (double)f3 * d3 * (double)l3;
    result += (double)f4 * d4 * (double)l4;
    result += (double)f5 * (double)l5;
    result += (double)f6 + (double)f7;
    
    return result;
}

/* Function 3: Nested control flow with calls */
int high_pressure_call_3(int iterations, double threshold) {
    int var1 = iterations * 2;
    int var2 = var1 + 1;
    int var3 = var2 * 3;
    int var4 = var3 - 5;
    int var5 = var4 | 0xFF;
    
    double dvar1 = threshold;
    double dvar2 = dvar1 * 1.1;
    double dvar3 = dvar2 / 1.2;
    double dvar4 = dvar3 + 0.5;
    double dvar5 = sin(dvar4);
    
    float fvar1 = (float)dvar1;
    float fvar2 = (float)dvar2 * 2.0f;
    float fvar3 = (float)dvar3 + 1.0f;
    
    long lvar1 = (long)var1 * 1000L;
    long lvar2 = (long)var2 + lvar1;
    
    int result = 0;
    
    /* Complex control flow with calls at different points */
    for (int i = 0; i < iterations; i++) {
        if (i % 3 == 0) {
            /* Call site 1 */
            memory_barrier();
            unknown_effect(var1 + i, dvar1 * i);
            memory_barrier();
            
            var1 += var2;
            dvar1 = dvar2 * i;
        } else if (i % 3 == 1) {
            /* Call site 2 */
            memory_barrier();
            another_effect(fvar1 + i, lvar1);
            memory_barrier();
            
            fvar1 = fvar2 * 0.9f;
            lvar1 = lvar2 >> 1;
        } else {
            /* Call site 3 - both calls */
            memory_barrier();
            unknown_effect(var3, dvar3);
            another_effect(fvar3, lvar2);
            memory_barrier();
            
            var3 = var4 ^ var5;
            dvar3 = cos(dvar4);
        }
        
        /* Use all variables in loop computation */
        result += var1 + var2 + var3 + var4 + var5;
        result += (int)(dvar1 + dvar2 + dvar3 + dvar4 + dvar5);
        result += (int)(fvar1 + fvar2 + fvar3);
        result += (int)(lvar1 + lvar2);
        
        /* Modify variables to keep them live */
        var2++;
        dvar2 += 0.1;
        fvar2 *= 1.01f;
        lvar2 += 2;
    }
    
    return result;
}

/* Main function with loop calling high-pressure functions */
int main(void) {
    /* Initialize test data */
    int int_data[20];
    double double_data[20];
    float float_data[20];
    long long_data[20];
    
    for (int i = 0; i < 20; i++) {
        int_data[i] = i * 3 + 1;
        double_data[i] = (double)i * 1.5;
        float_data[i] = (float)i * 2.0f;
        long_data[i] = (long)i * 1000L;
    }
    
    int total = 0;
    double total_double = 0.0;
    
    /* Loop to ensure caller-save insertion is triggered multiple times */
    for (int iter = 0; iter < 100; iter++) {
        /* Modify data slightly each iteration */
        int_data[iter % 20] += iter;
        double_data[iter % 20] += (double)iter * 0.01;
        
        /* Call different high-pressure functions */
        int result1 = high_pressure_call_1(int_data, double_data);
        double result2 = high_pressure_call_2(float_data, long_data);
        int result3 = high_pressure_call_3(50, (double)iter * 0.1);
        
        total += result1 + result3;
        total_double += result2;
        
        /* Global side effect to prevent dead code elimination */
        global_counter += iter % 7;
    }
    
    printf("Result: %d, %.2f, global=%d\n", 
           total, total_double, global_counter);
    
    return 0;
}
