/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper function with many arguments */
int __attribute__((noinline, noclone)) 
helper_function(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Complex computation to prevent optimization */
    volatile int result = 0;
    result += a * b;
    result -= c / (d ? d : 1);
    result += e * f;
    result ^= g << (h & 3);
    
    /* Force side effects */
    static int counter = 0;
    counter++;
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    return result + counter;
}

/* Target function with high register pressure */
int __attribute__((noinline))
target_function(int input1, int input2, int input3, int input4) {
    /* Declare many local variables of mixed types */
    int a = input1 + 1;
    int b = input2 * 2;
    int c = input3 - 3;
    int d = input4 / 4;
    long e = (long)input1 * input2;
    long f = (long)input3 * input4;
    float g = (float)input1 / (input2 ? input2 : 1);
    float h = (float)input3 / (input4 ? input4 : 1);
    double i = (double)input1 * 1.5;
    double j = (double)input2 * 2.5;
    
    /* Additional variables to increase pressure */
    int k = a + b;
    int l = c - d;
    long m = e + f;
    float n = g * 2.0f;
    double o = i / 1.5;
    
    /* Suggest specific register usage (x86-64 specific) */
    register int forced_reg asm("r12") = input1 + input2;
    register long forced_reg2 asm("r13") = e * 2;
    
    /* Complex pre-call computations creating dependencies */
    int pre1 = a * b + c;
    int pre2 = d - (c ? c : 1) / (b ? b : 1);
    long pre3 = e + f * 2;
    float pre4 = g * h + 1.0f;
    double pre5 = i - j / 2.0;
    
    /* Use all variables before call to make them live */
    int t1 = pre1 + k;
    int t2 = pre2 - l;
    long t3 = pre3 + m;
    float t4 = pre4 * n;
    double t5 = pre5 + o;
    
    /* Memory barrier before call - forces all vars to be live */
    asm volatile("" : : : "memory");
    
    /* Call with many arguments - forces some to stack on x86-64 */
    int result = helper_function(
        t1, t2, (int)t3, (int)(t5 * 100),
        forced_reg, (int)(t4 * 10),
        input1, input2
    );
    
    /* Memory barrier after call */
    asm volatile("" : : : "memory");
    
    /* Complex post-call computations using all variables */
    /* This ensures the call is NOT at the end of the basic block */
    int post1 = result + a + b;
    long post2 = (long)post1 * e;
    float post3 = (float)post2 * g;
    double post4 = (double)post3 + i;
    
    /* Use all remaining variables */
    post1 += c + d;
    post2 += f;
    post3 += h;
    post4 += j;
    
    /* More computations to extend basic block after call */
    int final1 = post1 + k + l;
    long final2 = post2 + m;
    float final3 = post3 + n;
    double final4 = post4 + o;
    
    /* Use forced register variables */
    final1 += forced_reg;
    final2 += forced_reg2;
    
    /* Final computation using everything */
    int final_result = (int)(final1 + final2 + final3 + final4);
    
    return final_result;
}

/* Another helper to increase complexity */
double __attribute__((noinline, noclone))
helper_double(double a, double b, double c, double d, 
              double e, double f, double g, double h) {
    volatile double result = a;
    result += b * c;
    result -= d / e;
    result *= f + g;
    result /= h + 1.0;
    
    asm volatile("" : : : "memory");
    return result;
}

/* Function with mixed float/int register pressure */
float __attribute__((noinline))
mixed_pressure_function(float f1, float f2, float f3, float f4,
                        int i1, int i2, int i3, int i4) {
    /* Create many float variables */
    float a = f1 * 1.1f;
    float b = f2 * 2.2f;
    float c = f3 * 3.3f;
    float d = f4 * 4.4f;
    float e = a + b;
    float f = c - d;
    float g = a * c;
    float h = b / (d ? d : 1.0f);
    
    /* And many int variables */
    int w = i1 + 1;
    int x = i2 * 2;
    int y = i3 - 3;
    int z = i4 / 4;
    int u = w + x;
    int v = y - z;
    
    /* Pre-call computation */
    float pre_f = a + b + c + d + e + f + g + h;
    int pre_i = w + x + y + z + u + v;
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Call that uses both float and int registers */
    double call_result = helper_double(
        (double)pre_f, (double)pre_i,
        (double)a, (double)b,
        (double)c, (double)d,
        (double)e, (double)f
    );
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Extensive post-call computations */
    float post1 = (float)call_result + a;
    float post2 = post1 * b;
    float post3 = post2 - c;
    float post4 = post3 / (d ? d : 1.0f);
    
    int post5 = (int)post4 + w;
    int post6 = post5 * x;
    int post7 = post6 - y;
    int post8 = post7 / (z ? z : 1);
    
    /* Use all variables again */
    float final_f = post1 + post2 + post3 + post4 + e + f + g + h;
    int final_i = post5 + post6 + post7 + post8 + u + v;
    
    return final_f + (float)final_i;
}

int main() {
    srand(time(NULL));
    int total_result = 0;
    
    /* Create varying inputs to prevent constant propagation */
    int inputs[100];
    for (int i = 0; i < 100; i++) {
        inputs[i] = rand() % 100 + 1;
    }
    
    /* Repeatedly call target functions to exercise the code path */
    for (int i = 0; i < 50; i++) {
        /* Call with different inputs each time */
        int idx = i % 100;
        total_result += target_function(
            inputs[(idx) % 100],
            inputs[(idx + 1) % 100],
            inputs[(idx + 2) % 100],
            inputs[(idx + 3) % 100]
        );
        
        /* Also call mixed pressure function */
        float float_result = mixed_pressure_function(
            (float)inputs[(idx + 4) % 100] / 10.0f,
            (float)inputs[(idx + 5) % 100] / 10.0f,
            (float)inputs[(idx + 6) % 100] / 10.0f,
            (float)inputs[(idx + 7) % 100] / 10.0f,
            inputs[(idx + 8) % 100],
            inputs[(idx + 9) % 100],
            inputs[(idx + 10) % 100],
            inputs[(idx + 11) % 100]
        );
        
        total_result += (int)float_result;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %d\n", total_result);
    
    return 0;
}
