/* Test program to trigger caller-save insertion during reload */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-omit-frame-pointer -fdump-rtl-reload -fdump-rtl-all caller-save-test.c external.c */

#include <stdio.h>
#include <math.h>

/* External function with unknown side effects - prevents optimization */
extern void unknown_effect(int, double);
extern void another_effect(float, long);
extern void mixed_effect(int, float, double, long);

/* Global volatile to prevent dead code elimination */
volatile int global_counter = 0;

/* Function with high register pressure around a call */
/* 15+ live variables across call */
int high_pressure_call_1(int *ints, double *doubles) {
    /* Unpack into many scalar variables */
    int v1 = ints[0] * 2;
    int v2 = ints[1] + ints[2];
    int v3 = ints[3] - ints[4];
    int v4 = ints[5] * ints[6];
    int v5 = ints[7] ^ ints[8];
    
    double d1 = doubles[0] * 3.14159;
    double d2 = doubles[1] + doubles[2];
    double d3 = doubles[3] / doubles[4];
    double d4 = sin(doubles[5]);
    double d5 = cos(doubles[6]);
    
    float f1 = (float)doubles[7];
    float f2 = (float)doubles[8];
    float f3 = f1 * f2;
    
    long l1 = (long)ints[9] * 1000;
    long l2 = (long)ints[10] << 4;
    
    /* Use all variables in computation before call */
    int temp1 = v1 + v2 - v3;
    double temp2 = d1 * d2 - d3;
    float temp3 = f1 + f2 + f3;
    long temp4 = l1 | l2;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* External call - all above variables must be saved/restored */
    unknown_effect(temp1, temp2);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Use variables after call in complex ways */
    v1 = v1 * temp1;
    v2 = v2 + (int)temp2;
    v3 = v3 ^ (int)temp4;
    v4 = v4 - temp1;
    v5 = v5 | (int)l1;
    
    d1 = d1 + temp2;
    d2 = d2 * (double)temp1;
    d3 = d3 - temp2;
    d4 = d4 * sin(temp2);
    d5 = d5 + cos(temp2);
    
    f1 = f1 * temp3;
    f2 = f2 / temp3;
    f3 = f3 + (float)temp2;
    
    l1 = l1 + temp4;
    l2 = l2 ^ (long)temp1;
    
    /* Create checksum from all variables */
    int result = v1 + v2 + v3 + v4 + v5;
    result += (int)d1 + (int)d2 + (int)d3;
    result += (int)f1 + (int)f2 + (int)f3;
    result += (int)(l1 & 0xFFFF) + (int)(l2 & 0xFFFF);
    
    global_counter++;
    return result;
}

/* Second function with different register pressure pattern */
double high_pressure_call_2(float *floats, long *longs) {
    /* Even more variables - mixing types */
    float f1 = floats[0];
    float f2 = floats[1];
    float f3 = floats[2];
    float f4 = floats[3];
    float f5 = floats[4];
    float f6 = floats[5];
    
    long l1 = longs[0];
    long l2 = longs[1];
    long l3 = longs[2];
    long l4 = longs[3];
    
    double d1 = (double)f1 * 1.234;
    double d2 = (double)f2 / 5.678;
    double d3 = (double)f3 + 9.1011;
    double d4 = (double)f4 - 1213.1415;
    
    int i1 = (int)l1 & 0xFF;
    int i2 = (int)l2 | 0x100;
    int i3 = (int)l3 ^ 0x200;
    int i4 = (int)l4 + 300;
    
    /* Complex computation before call */
    for (int j = 0; j < 3; j++) {
        f1 = f1 * 1.1f;
        f2 = f2 + 0.5f;
        d1 = d1 * 1.01;
        i1 = i1 + j;
    }
    
    /* Conditional call site */
    if (i1 > 100) {
        asm volatile("" : : : "memory");
        another_effect(f1, l1);
        asm volatile("" : : : "memory");
    } else {
        asm volatile("" : : : "memory");
        mixed_effect(i1, f2, d1, l2);
        asm volatile("" : : : "memory");
    }
    
    /* Use variables after call */
    double sum = 0.0;
    sum += (double)f1 + (double)f2 + (double)f3;
    sum += (double)f4 + (double)f5 + (double)f6;
    sum += (double)l1 + (double)l2 + (double)l3 + (double)l4;
    sum += d1 + d2 + d3 + d4;
    sum += (double)i1 + (double)i2 + (double)i3 + (double)i4;
    
    return sum;
}

/* Third function with nested loops and calls */
void high_pressure_call_3(int iterations) {
    /* Many local variables */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    float f = 1.1f, g = 2.2f, h = 3.3f;
    double x = 1.234, y = 5.678, z = 9.1011;
    long l1 = 1000, l2 = 2000, l3 = 3000;
    
    /* Loop with call inside */
    for (int i = 0; i < iterations; i++) {
        /* Modify variables in loop */
        a = a * 2 + i;
        b = b + a;
        c = c ^ b;
        d = d - c;
        e = e | d;
        
        f = f * 1.01f;
        g = g + (float)i;
        h = h - 0.5f;
        
        x = sin(x + (double)i);
        y = cos(y * 1.001);
        z = z / 1.5;
        
        l1 = l1 << 1;
        l2 = l2 >> 1;
        l3 = l3 + l1;
        
        /* Switch statement with different call patterns */
        switch (i % 4) {
            case 0:
                asm volatile("" : : : "memory");
                unknown_effect(a, x);
                asm volatile("" : : : "memory");
                break;
            case 1:
                asm volatile("" : : : "memory");
                another_effect(f, l1);
                asm volatile("" : : : "memory");
                break;
            case 2:
                asm volatile("" : : : "memory");
                mixed_effect(b, g, y, l2);
                asm volatile("" : : : "memory");
                break;
            case 3:
                /* Nested condition */
                if (c > 100) {
                    asm volatile("" : : : "memory");
                    unknown_effect(c, z);
                    asm volatile("" : : : "memory");
                }
                break;
        }
        
        /* Use variables after call */
        a = a + (int)x;
        b = b ^ (int)l1;
        f = f * (float)y;
        x = x + (double)l2;
    }
    
    global_counter += a + b + c;
}

int main() {
    int ints[20];
    double doubles[20];
    float floats[20];
    long longs[20];
    
    /* Initialize test data */
    for (int i = 0; i < 20; i++) {
        ints[i] = i * 3 + 1;
        doubles[i] = (double)i * 1.5;
        floats[i] = (float)i * 2.5f;
        longs[i] = (long)i * 1000;
    }
    
    int total = 0;
    double sum = 0.0;
    
    /* Loop with calls to high-pressure functions */
    for (int i = 0; i < 100; i++) {
        /* Modify inputs slightly each iteration */
        ints[0] += i;
        doubles[0] += (double)i * 0.1;
        
        /* Call first high-pressure function */
        total += high_pressure_call_1(ints, doubles);
        
        /* Call second function every other iteration */
        if (i % 2 == 0) {
            sum += high_pressure_call_2(floats, longs);
        }
        
        /* Call third function every 10 iterations */
        if (i % 10 == 0) {
            high_pressure_call_3(5);
        }
    }
    
    printf("Result: %d, Sum: %f, Counter: %d\n", 
           total, sum, global_counter);
    
    return 0;
}
