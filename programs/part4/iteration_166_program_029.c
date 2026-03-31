/* caller_save_test.c - Main test program to trigger caller-save insertion */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* External functions that cannot be inlined */
extern void unknown_effect_int(int x);
extern void unknown_effect_double(double x);
extern void unknown_effect_mixed(int a, double b);
extern int external_computation(int a, int b, double c, double d);

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* Function 1: High integer register pressure around call */
int high_pressure_int_call(int iter) {
    /* Declare many integer variables that must stay in registers */
    int v1 = iter * 2;
    int v2 = iter + 100;
    int v3 = iter ^ 0x55AA55AA;
    int v4 = iter * 3 + 1;
    int v5 = iter << 4;
    int v6 = iter | 0xFF00FF00;
    int v7 = iter - 50;
    int v8 = iter % 17;
    int v9 = iter * iter;
    int v10 = ~iter;
    int v11 = iter + v1;
    int v12 = v2 * v3;
    int v13 = v4 ^ v5;
    int v14 = v6 + v7;
    int v15 = v8 * v9;
    
    /* Use all variables in computation before call */
    int sum_before = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    sum_before += v11 + v12 + v13 + v14 + v15;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* External call that clobbers call-used registers */
    unknown_effect_int(sum_before);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Use all variables again after call - they must be restored */
    int sum_after = v1 * 2 + v2 * 3 + v3 * 4 + v4 * 5 + v5 * 6;
    sum_after += v6 * 7 + v7 * 8 + v8 * 9 + v9 * 10 + v10 * 11;
    sum_after += v11 * 12 + v12 * 13 + v13 * 14 + v14 * 15 + v15 * 16;
    
    /* Complex conditional to prevent dead code elimination */
    if (sum_after > 1000000) {
        return sum_before + sum_after;
    } else {
        return sum_before - sum_after;
    }
}

/* Function 2: Mixed integer and floating-point pressure */
double high_pressure_mixed_call(double base) {
    /* Mixed types to use both GPRs and FPRs */
    int i1 = (int)base;
    int i2 = i1 * 2;
    int i3 = i1 + 100;
    int i4 = i1 ^ 0x12345678;
    int i5 = i1 << 3;
    
    double d1 = base * 1.1;
    double d2 = base * 2.2;
    double d3 = base * 3.3;
    double d4 = base * 4.4;
    double d5 = base * 5.5;
    double d6 = sin(base);
    double d7 = cos(base);
    double d8 = sqrt(fabs(base));
    double d9 = d1 + d2;
    double d10 = d3 * d4;
    
    /* Computation using all variables */
    double mix_before = d1 + d2 + d3 + d4 + d5;
    mix_before += d6 + d7 + d8 + d9 + d10;
    mix_before += i1 + i2 + i3 + i4 + i5;
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Call that uses both int and double parameters */
    unknown_effect_mixed(i1, d1);
    
    /* Another external call */
    unknown_effect_double(mix_before);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* More computation after calls - variables must be preserved */
    double mix_after = (d1 * i1) + (d2 * i2) + (d3 * i3) + (d4 * i4) + (d5 * i5);
    mix_after += sin(d6) + cos(d7) + sqrt(d8) + (d9 * d10);
    
    /* Complex control flow */
    for (int i = 0; i < 5; i++) {
        mix_after += external_computation(i1 + i, i2 - i, d1 + i, d2 - i);
    }
    
    return mix_before + mix_after;
}

/* Function 3: Nested loops with calls */
void nested_loop_calls(int n) {
    double result = 0.0;
    
    for (int i = 0; i < n; i++) {
        /* Create register pressure in loop */
        int a = i * 2;
        int b = i + 10;
        int c = i ^ 0xAA;
        int d = i * i;
        int e = i % 13;
        
        double x = sin(i * 0.1);
        double y = cos(i * 0.2);
        double z = sqrt(i + 1.0);
        double w = x * y + z;
        
        /* Conditional with call inside */
        if (i % 3 == 0) {
            asm volatile("" : : : "memory");
            unknown_effect_mixed(a, x);
            asm volatile("" : : : "memory");
            result += a * x;
        } else if (i % 3 == 1) {
            asm volatile("" : : : "memory");
            unknown_effect_int(b);
            asm volatile("" : : : "memory");
            result += b * y;
        } else {
            asm volatile("" : : : "memory");
            unknown_effect_double(z);
            asm volatile("" : : : "memory");
            result += c * z;
        }
        
        /* Use all variables after conditional calls */
        result += d * w + e;
        
        /* Inner loop with another call */
        for (int j = 0; j < 3; j++) {
            int inner = (i * j) + a + b + c;
            double inner_d = x + y + z + w + j;
            
            asm volatile("" : : : "memory");
            external_computation(inner, inner + 1, inner_d, inner_d * 2);
            asm volatile("" : : : "memory");
            
            result += inner * inner_d;
        }
    }
    
    /* Prevent optimization */
    global_counter += (int)result;
}

/* Function 4: Switch statement with calls at multiple points */
int switch_with_calls(int value) {
    int result = 0;
    
    /* Variables live across switch */
    int a = value * 2;
    int b = value + 100;
    int c = value ^ 0xDEADBEEF;
    double x = value * 1.234;
    double y = value / 3.14159;
    double z = sqrt(value + 10.0);
    
    switch (value % 5) {
        case 0:
            asm volatile("" : : : "memory");
            unknown_effect_int(a);
            result = a + (int)x;
            break;
        case 1:
            asm volatile("" : : : "memory");
            unknown_effect_mixed(b, y);
            result = b + (int)y;
            break;
        case 2:
            asm volatile("" : : : "memory");
            unknown_effect_double(z);
            result = c + (int)z;
            break;
        case 3:
            /* Multiple calls in one case */
            asm volatile("" : : : "memory");
            unknown_effect_int(a);
            unknown_effect_mixed(b, x);
            result = a + b + (int)(x + y);
            break;
        case 4:
            asm volatile("" : : : "memory");
            external_computation(a, b, x, y);
            result = external_computation(b, c, y, z);
            break;
    }
    
    /* Use all variables after switch */
    return result + a + b + c + (int)(x + y + z);
}

int main(void) {
    int total = 0;
    double total_d = 0.0;
    
    /* Loop to create multiple call sites */
    for (int i = 0; i < 100; i++) {
        /* Call function with high integer pressure */
        total += high_pressure_int_call(i);
        
        /* Call function with mixed pressure */
        total_d += high_pressure_mixed_call(i * 0.5);
        
        /* Call nested loop function every 10 iterations */
        if (i % 10 == 0) {
            nested_loop_calls(5);
        }
        
        /* Call switch function */
        total += switch_with_calls(i);
    }
    
    /* Use results to prevent optimization */
    printf("Integer total: %d\n", total);
    printf("Double total: %f\n", total_d);
    printf("Global counter: %d\n", global_counter);
    
    return (total > 0) ? 0 : 1;
}
