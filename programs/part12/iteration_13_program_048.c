#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function with high register pressure - marked to prevent optimizations
__attribute__((noinline, noipa))
static volatile int high_pressure_compute(volatile int a, volatile int b, 
                                         volatile int c, volatile int d,
                                         volatile int iterations) {
    volatile int result = 0;
    
    for (volatile int i = 0; i < iterations; i++) {
        // Declare many local variables to create register pressure
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
        
        // Complex expression that will be reused - candidate for rematerialization
        int complex_expr = (a * b) + (c << 2) - (d & 0xFF);
        
        // First set of independent calculations
        v1 = a + b + i;
        v2 = b * c - i;
        v3 = c ^ d;
        v4 = d | a;
        v5 = (a << 3) + (b >> 1);
        v6 = (b & 0xF0) * c;
        v7 = d - a + complex_expr;  // First use of complex expression
        v8 = (a * c) + (b * d);
        v9 = (c << 4) | (d >> 2);
        v10 = complex_expr + v1;    // Second use - copy of complex_expr
        
        // Memory barrier to prevent reordering
        __asm__ volatile ("" : : : "memory");
        
        // Second set of calculations
        v11 = v1 * v2 + v3;
        v12 = v4 - v5 * v6;
        v13 = (v7 & 0xFF) + complex_expr;  // Third use
        v14 = v8 ^ v9;
        v15 = v10 << 2;
        v16 = (v11 + v12) * v13;
        v17 = v14 | v15;
        v18 = complex_expr - v16;   // Fourth use
        v19 = v17 & 0x7F;
        v20 = v18 * v19;
        
        // Conditional branch to split basic blocks
        volatile int condition = a + i;
        if (condition & 1) {
            // True branch calculations
            v21 = v20 + complex_expr;  // Fifth use
            v22 = v19 * 3;
            v23 = (v21 << 1) + v22;
            v24 = complex_expr >> 2;   // Sixth use
            v25 = v23 | v24;
            v26 = v25 ^ 0x55;
            v27 = (v26 + complex_expr) & 0xFF;  // Seventh use
            v28 = v27 * 7;
            v29 = complex_expr - v28;  // Eighth use
            v30 = v29 & 0x3F;
            
            result += v30;
        } else {
            // False branch calculations
            v21 = v20 - complex_expr;  // Ninth use
            v22 = v19 / 2;
            v23 = (v21 >> 1) | v22;
            v24 = complex_expr & 0x7F;  // Tenth use
            v25 = v23 ^ v24;
            v26 = v25 + 0xAA;
            v27 = (v26 * complex_expr) >> 3;  // Eleventh use
            v28 = v27 | 0xF0;
            v29 = complex_expr + v28;   // Twelfth use
            v30 = v29 % 256;
            
            result -= v30;
        }
        
        // More calculations after the conditional
        int temp1 = complex_expr * 2;   // Thirteenth use
        int temp2 = v30 + temp1;
        int temp3 = (complex_expr << 1) | temp2;  // Fourteenth use
        int temp4 = temp3 ^ complex_expr;         // Fifteenth use
        
        // Another memory barrier
        __asm__ volatile ("" : : : "memory");
        
        // Final calculations using all variables
        int final1 = v1 + v2 + v3 + v4 + v5;
        int final2 = v6 + v7 + v8 + v9 + v10;
        int final3 = v11 + v12 + v13 + v14 + v15;
        int final4 = v16 + v17 + v18 + v19 + v20;
        int final5 = v21 + v22 + v23 + v24 + v25;
        int final6 = v26 + v27 + v28 + v29 + v30;
        
        // Use complex_expr multiple times in final computation
        int final_sum = final1 + final2 + final3 + final4 + 
                       final5 + final6 + temp4 + 
                       complex_expr + (complex_expr >> 1) + 
                       (complex_expr & 0x1F);  // Sixteenth, seventeenth, eighteenth uses
        
        result += final_sum;
        
        // Modify inputs slightly for next iteration
        a ^= 1;
        b += i;
        c -= 1;
        d |= i;
    }
    
    return result;
}

int main(void) {
    srand(time(NULL));
    
    // Use volatile to prevent constant propagation
    volatile int a = rand() % 100 + 1;
    volatile int b = rand() % 100 + 1;
    volatile int c = rand() % 100 + 1;
    volatile int d = rand() % 100 + 1;
    volatile int iterations = 10;  // Small number to avoid overflow
    
    printf("Starting computation with a=%d, b=%d, c=%d, d=%d, iterations=%d\n",
           a, b, c, d, iterations);
    
    volatile int result = high_pressure_compute(a, b, c, d, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}
