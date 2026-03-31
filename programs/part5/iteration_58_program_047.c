/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper function with many arguments */
int __attribute__((noinline, noclone)) 
helper_function(int a, int b, int c, int d, int e, int f, int g, int h) {
    volatile int result = 0;
    /* Force side effects and prevent optimization */
    asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d));
    result = a * b + c * d - e * f + g * h;
    /* Memory barrier to ensure all arguments are used */
    asm volatile("" : : : "memory");
    return result;
}

/* Target function with high register pressure across a call */
int __attribute__((noinline))
target_function(int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8) {
    /* Declare many local variables of mixed types */
    int a = p1 * 2;
    int b = p2 + 3;
    int c = p3 - 4;
    int d = p4 / 2;
    int e = p5 * 3;
    int f = p6 + 7;
    int g = p7 - 1;
    int h = p8 * 2;
    
    float fa = p1 * 1.5f;
    float fb = p2 * 2.5f;
    double da = p3 * 3.14159;
    double db = p4 * 2.71828;
    long la = p5 * 1000L;
    long lb = p6 * 2000L;
    
    /* Suggest specific register for a variable (x86-64: r12 is call-clobbered) */
    register int critical_var asm("r12") = p1 + p2 + p3;
    
    /* Complex pre-call computations creating web of dependencies */
    int t1 = a * b + c;
    int t2 = d - e / (f ? f : 1);
    float ft1 = fa * fb + (float)c;
    double dt1 = da / (db ? db : 1.0) + (double)e;
    long lt1 = la - lb * 2;
    
    /* Use critical_var in computation */
    critical_var = critical_var * 2 + t1;
    
    /* Memory barrier before call - makes all variables appear live */
    asm volatile("" : : : "memory");
    
    /* Call with many arguments - some will go on stack on x86-64 */
    int call_result = helper_function(t1, t2, a, b, c, d, e, f);
    
    /* Memory barrier after call */
    asm volatile("" : : : "memory");
    
    /* Post-call computations using all variables */
    int t3 = call_result + g * h;
    float ft2 = ft1 + (float)call_result * 0.5f;
    double dt2 = dt1 - (double)call_result * 0.25;
    long lt2 = lt1 + (long)call_result * 3L;
    
    /* More complex computations ensuring variables stay live */
    int result = t3 + (int)ft2 + (int)dt2 + (int)(lt2 % 1000);
    result += critical_var;  /* Use register-suggested variable */
    
    /* Additional arithmetic to extend basic block after call */
    result = result * 2 - (a + b + c + d);
    result = result / ((e + f + g + h) ? (e + f + g + h) : 1);
    result += (int)(fa + fb);
    result += (int)(da + db);
    result += (int)(la + lb);
    
    return result;
}

/* Another helper to increase complexity */
double __attribute__((noinline, noclone))
helper_double(double x, double y, double z) {
    asm volatile("" : : : "memory");
    return x * y + z;
}

/* Function with mixed types and multiple calls */
int __attribute__((noinline))
complex_function(int base) {
    int v1 = base * 3;
    int v2 = base + 7;
    int v3 = base - 2;
    int v4 = base / 2;
    int v5 = base * 5;
    int v6 = base + 11;
    int v7 = base - 3;
    int v8 = base * 7;
    
    float fv1 = (float)v1 * 1.1f;
    float fv2 = (float)v2 * 2.2f;
    double dv1 = (double)v3 * 3.3;
    double dv2 = (double)v4 * 4.4;
    
    /* First computation */
    int tmp1 = v1 * v2 + v3 - v4;
    float ftmp = fv1 + fv2;
    double dtmp = dv1 - dv2;
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* First call */
    int res1 = helper_function(tmp1, v5, v6, v7, v8, v1, v2, v3);
    
    /* Computation between calls */
    int tmp2 = res1 + v4 * v5 - v6;
    ftmp = ftmp * 2.0f + (float)res1;
    dtmp = helper_double(dtmp, (double)res1, dv2);
    
    /* Second call */
    int res2 = helper_function(tmp2, v7, v8, v1, v2, v3, v4, v5);
    
    /* Final computation */
    int final = res1 + res2 + (int)ftmp + (int)dtmp;
    final = final * 3 - (v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8);
    
    return final;
}

int main() {
    srand(time(NULL));
    int total = 0;
    
    /* Initialize with random values to prevent constant propagation */
    int inputs[8];
    for (int i = 0; i < 8; i++) {
        inputs[i] = rand() % 100 + 1;
    }
    
    /* Repeatedly exercise the target functions */
    for (int i = 0; i < 10000; i++) {
        /* Modify inputs slightly each iteration */
        for (int j = 0; j < 8; j++) {
            inputs[j] = (inputs[j] * 13 + 17) % 1000;
        }
        
        /* Call target function */
        total += target_function(inputs[0], inputs[1], inputs[2], inputs[3],
                                inputs[4], inputs[5], inputs[6], inputs[7]);
        
        /* Also call complex function */
        total += complex_function(inputs[0] + i);
        
        /* Prevent loop unrolling from simplifying too much */
        if (i % 100 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return 0;
}
