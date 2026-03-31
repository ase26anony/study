#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// External function with unknown side effects - prevents optimization
extern void unknown_effect(int, double) __attribute__((noinline));
extern void another_effect(float, long) __attribute__((noinline));

// Global volatile to prevent call elimination
volatile int global_counter = 0;

// External function definitions (will be in separate file)
void unknown_effect(int x, double y) {
    // Minimal side effect that can't be optimized away
    asm volatile("" : : "r"(x), "r"(y) : "memory");
    global_counter += x;
}

void another_effect(float f, long l) {
    asm volatile("" : : "r"(f), "r"(l) : "memory");
    global_counter -= l;
}

// Function with high register pressure around calls
// Many variables must stay in call-used registers across calls
int high_pressure_function(int *ints, double *doubles, float *floats) {
    // Unpack into many scalar variables - all must be live across calls
    int v1 = ints[0] * 2;
    int v2 = ints[1] + ints[2];
    int v3 = ints[3] - ints[4];
    int v4 = ints[5] * ints[6];
    int v5 = ints[7] ^ ints[8];
    int v6 = ints[9] | ints[10];
    int v7 = ints[11] & ints[12];
    int v8 = ints[13] << 2;
    int v9 = ints[14] >> 1;
    int v10 = ints[15] + 100;
    
    double d1 = doubles[0] * 3.14159;
    double d2 = doubles[1] / 2.71828;
    double d3 = doubles[2] + doubles[3];
    double d4 = doubles[4] - doubles[5];
    double d5 = doubles[6] * doubles[7];
    
    float f1 = floats[0] * 1.5f;
    float f2 = floats[1] + 2.5f;
    float f3 = floats[2] - 3.5f;
    float f4 = floats[3] / 4.5f;
    
    // Complex computation mixing all variables
    // Forces them to stay in registers
    double temp1 = d1 * v1 + d2 * v2;
    float temp2 = f1 * v3 + f2 * v4;
    int temp3 = v5 * v6 + v7 * v8;
    
    // FIRST CALL SITE - many values live across call
    // This should trigger caller-save insertion
    unknown_effect(v1 + v2, d1 + d2);
    
    // Use results after call - prevents moving computations
    temp1 = temp1 * 2.0 + d3;
    temp2 = temp2 * 1.1f + f3;
    temp3 = temp3 ^ v9;
    
    // Nested control flow with calls inside
    for (int i = 0; i < 10; i++) {
        // Loop creates multiple program points for caller-save
        if (i % 2 == 0) {
            // SECOND CALL SITE inside loop/conditional
            another_effect(f1 + i, v10 * i);
            
            // More computations keeping variables live
            d4 = d4 + d5 * i;
            v3 = v3 + v4 * i;
        } else {
            // THIRD CALL SITE with different live set
            unknown_effect(v6 - i, d2 * i);
            
            f4 = f4 * (1.0f + i * 0.1f);
            v7 = v7 | (v8 << i);
        }
        
        // Complex expression mixing all types
        // Forces register pressure across iteration
        d1 = d1 * (1.0 + temp1 * 0.01);
        f1 = f1 * (1.0f + temp2 * 0.01f);
        v1 = v1 + temp3 * i;
    }
    
    // Switch statement with calls at multiple cases
    // Creates different control flow paths needing caller-save
    int selector = v1 % 4;
    switch (selector) {
        case 0:
            unknown_effect(v2, d3);
            v5 = v5 * 2;
            break;
        case 1:
            another_effect(f2, v3);
            d4 = sqrt(d4);
            break;
        case 2:
            unknown_effect(v4, d5);
            f3 = f3 * f3;
            break;
        case 3:
            another_effect(f4, v6);
            v8 = v8 ^ v7;
            break;
    }
    
    // Final computation using all variables
    // Ensures they remain live through all calls
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += (int)(d1 + d2 + d3 + d4 + d5);
    result += (int)(f1 + f2 + f3 + f4);
    
    return result;
}

// Second high-pressure function with different pattern
double mixed_pressure_function(long *longs, double *doubles, int *ints) {
    // Even more variables to increase pressure
    long l1 = longs[0] * 3;
    long l2 = longs[1] + longs[2];
    long l3 = longs[3] - longs[4];
    long l4 = longs[5] | longs[6];
    long l5 = longs[7] & longs[8];
    
    double d1 = doubles[0];
    double d2 = doubles[1];
    double d3 = doubles[2];
    double d4 = doubles[3];
    double d5 = doubles[4];
    double d6 = doubles[5];
    double d7 = doubles[6];
    double d8 = doubles[7];
    
    int i1 = ints[0];
    int i2 = ints[1];
    int i3 = ints[2];
    int i4 = ints[3];
    int i5 = ints[4];
    
    // Complex pre-call computation
    double sum_d = d1 + d2 + d3 + d4;
    long sum_l = l1 + l2 + l3;
    int sum_i = i1 * i2 + i3 * i4;
    
    // Call in loop with varying conditions
    for (int iter = 0; iter < 5; iter++) {
        // Multiple calls in same basic block
        unknown_effect(i1 + iter, d1 * iter);
        
        // Memory barrier to prevent reordering
        asm volatile("" : : : "memory");
        
        another_effect((float)d2, l1 + iter);
        
        // Computation between calls keeps values live
        d5 = d5 * (1.0 + sin(d6));
        l4 = l4 ^ (l5 << iter);
        i5 = i5 + i1 * iter;
        
        // Another call
        if (iter % 2) {
            unknown_effect(i2, d3);
        } else {
            another_effect((float)d4, l2);
        }
        
        // More computations
        d7 = d7 + cos(d8) * iter;
        l3 = l3 * (2 + iter);
    }
    
    // Final result using all variables
    return sum_d + sum_l + sum_i + d5 + d6 + d7 + d8 + l4 + l5 + i5;
}

int main() {
    // Initialize test data
    int ints[20];
    double doubles[10];
    float floats[10];
    long longs[10];
    
    for (int i = 0; i < 20; i++) ints[i] = rand() % 100;
    for (int i = 0; i < 10; i++) doubles[i] = (double)rand() / RAND_MAX * 100.0;
    for (int i = 0; i < 10; i++) floats[i] = (float)rand() / RAND_MAX * 50.0f;
    for (int i = 0; i < 10; i++) longs[i] = rand() * 100L;
    
    int total = 0;
    double total_d = 0.0;
    
    // Loop to create multiple call sites in caller
    for (int outer = 0; outer < 100; outer++) {
        // Modify inputs slightly each iteration
        ints[0] += outer;
        doubles[0] += outer * 0.1;
        
        // Call first high-pressure function
        total += high_pressure_function(ints, doubles, floats);
        
        // Call second high-pressure function
        total_d += mixed_pressure_function(longs, doubles, ints);
        
        // Prevent loop unrolling
        if (outer % 10 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Result: %d, %f\n", total, total_d);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
