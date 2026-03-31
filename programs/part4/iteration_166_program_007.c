/* Main test file to force caller-save insertion during reload */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* External functions with side effects to prevent optimization */
extern void unknown_effect_int(int);
extern void unknown_effect_double(double);
extern void unknown_effect_mixed(int, double);
extern int get_global_counter(void);
extern void increment_counter(void);

/* Global volatile to prevent dead code elimination */
volatile int global_volatile = 0;

/* Non-inlinable external function with memory clobber */
void __attribute__((noinline)) external_call_with_side_effect(int a, double b) {
    /* Use asm to clobber memory and registers */
    asm volatile("" : : "r"(a), "r"(b) : "memory");
    global_volatile += a + (int)b;
}

/* Another external function with different signature */
double __attribute__((noinline)) external_math_op(double x, double y, int n) {
    asm volatile("" : : "r"(x), "r"(y), "r"(n) : "memory");
    return x * y + n;
}

/* High pressure function with many live integer variables across call */
int __attribute__((noinline)) high_pressure_int_call(int iter) {
    /* Declare many integer variables that will need registers */
    int v1 = iter * 1;
    int v2 = iter * 2;
    int v3 = iter * 3;
    int v4 = iter * 4;
    int v5 = iter * 5;
    int v6 = iter * 6;
    int v7 = iter * 7;
    int v8 = iter * 8;
    int v9 = iter * 9;
    int v10 = iter * 10;
    int v11 = iter * 11;
    int v12 = iter * 12;
    int v13 = iter * 13;
    int v14 = iter * 14;
    int v15 = iter * 15;
    
    /* Use all variables in computation before call */
    int sum_before = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                     v11 + v12 + v13 + v14 + v15;
    
    /* Complex computation mixing variables */
    int prod1 = v1 * v2 * v3;
    int prod2 = v4 * v5 * v6;
    int prod3 = v7 * v8 * v9;
    int prod4 = v10 * v11 * v12;
    int prod5 = v13 * v14 * v15;
    
    /* Call external function - all variables must be preserved */
    external_call_with_side_effect(sum_before, (double)prod1);
    
    /* Use variables after call in different computations */
    int sum_after = v1 - v2 + v3 - v4 + v5 - v6 + v7 - v8 + v9 - v10 +
                    v11 - v12 + v13 - v14 + v15;
    
    int final_prod = prod1 + prod2 + prod3 + prod4 + prod5;
    
    /* Another call with different arguments */
    unknown_effect_mixed(sum_after, (double)final_prod);
    
    /* Return value using all variables */
    return (sum_before + sum_after + final_prod) & 0xFF;
}

/* High pressure function with mixed int/float variables */
double __attribute__((noinline)) high_pressure_mixed_call(double base) {
    /* Mixed integer and floating point variables */
    double d1 = base * 1.1;
    double d2 = base * 2.2;
    double d3 = base * 3.3;
    double d4 = base * 4.4;
    double d5 = base * 5.5;
    int i1 = (int)(base * 10);
    int i2 = (int)(base * 20);
    int i3 = (int)(base * 30);
    int i4 = (int)(base * 40);
    int i5 = (int)(base * 50);
    float f1 = (float)(base * 0.1);
    float f2 = (float)(base * 0.2);
    float f3 = (float)(base * 0.3);
    float f4 = (float)(base * 0.4);
    float f5 = (float)(base * 0.5);
    
    /* Complex floating point computations */
    double temp1 = d1 * d2 + d3 / d4 - d5;
    double temp2 = sin(d1) * cos(d2) + tan(d3);
    float ftemp = f1 * f2 + f3 * f4 - f5;
    
    /* Integer computations */
    int itemp1 = i1 * i2 + i3 * i4 - i5;
    int itemp2 = (i1 << 3) | (i2 << 2) | (i3 << 1);
    
    /* Call with mixed arguments - forces preservation of both int and FP regs */
    external_call_with_side_effect(itemp1, temp1);
    
    /* More computations after call */
    double dtemp = temp1 * temp2 + ftemp;
    int final_int = itemp1 ^ itemp2 ^ (int)dtemp;
    
    /* Another external call */
    double result = external_math_op(dtemp, temp2, final_int);
    
    /* Use all variables one more time */
    return result + d1 + d2 + d3 + d4 + d5 + i1 + i2 + i3 + i4 + i5 + f1 + f2 + f3 + f4 + f5;
}

/* Function with calls inside complex control flow */
void __attribute__((noinline)) complex_control_flow(int n) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    double x = 1.0, y = 2.0, z = 3.0;
    float f = 4.0f, g = 5.0f;
    
    for (int i = 0; i < n; i++) {
        /* Loop creates multiple call sites needing caller-save */
        if (i % 3 == 0) {
            a = a * 2 + i;
            b = b * 3 - i;
            external_call_with_side_effect(a, (double)b);
            c = c + a - b;
        } else if (i % 3 == 1) {
            x = x * 1.5 + i;
            y = y / 2.0 - i;
            unknown_effect_double(x + y);
            z = sin(x) * cos(y);
        } else {
            f = f * 1.1f + i;
            g = g * 0.9f - i;
            unknown_effect_int((int)(f + g));
            e = e ^ (int)f ^ (int)g;
        }
        
        /* Nested condition with another call */
        switch (i % 4) {
            case 0:
                d = d * a + c;
                external_call_with_side_effect(d, x);
                break;
            case 1:
                e = e * b - d;
                unknown_effect_mixed(e, y);
                break;
            case 2:
                a = a ^ b ^ c;
                external_call_with_side_effect(a, z);
                break;
            case 3:
                b = b + c + d;
                unknown_effect_int(b);
                break;
        }
    }
    
    /* Force use of all variables */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
                       "r"(x), "r"(y), "r"(z), "r"(f), "r"(g) : "memory");
}

/* Main test driver */
int main(void) {
    int total = 0;
    double dtotal = 0.0;
    
    /* Create register pressure in main as well */
    int local1 = 1, local2 = 2, local3 = 3, local4 = 4, local5 = 5;
    double dlocal1 = 1.1, dlocal2 = 2.2, dlocal3 = 3.3;
    float flocal1 = 4.4f, flocal2 = 5.5f;
    
    /* Loop with multiple high-pressure calls */
    for (int i = 0; i < 100; i++) {
        /* Mix different types of high-pressure calls */
        int int_result = high_pressure_int_call(i);
        double mixed_result = high_pressure_mixed_call((double)i);
        
        /* Use results and local variables */
        total += int_result + local1 + local2 + local3;
        dtotal += mixed_result + dlocal1 + dlocal2;
        
        /* Modify locals to keep them live */
        local1 = (local1 * 3) % 100;
        local2 = (local2 + int_result) % 100;
        local3 = local3 ^ int_result;
        dlocal1 = dlocal1 * 1.01 + mixed_result;
        dlocal2 = dlocal2 / 1.01 - mixed_result;
        
        /* Periodic complex control flow */
        if (i % 10 == 0) {
            complex_control_flow(5);
        }
        
        /* Another external call */
        external_call_with_side_effect(local4, dlocal3);
        local4 = (local4 + i) % 256;
        dlocal3 = dlocal3 * 0.99;
    }
    
    /* Final computation using all locals */
    int final = total + (int)dtotal + local1 + local2 + local3 + local4 + local5 +
                (int)dlocal1 + (int)dlocal2 + (int)dlocal3 +
                (int)flocal1 + (int)flocal2;
    
    printf("Result: %d\n", final);
    return final & 0xFF;
}
