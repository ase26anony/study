/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper function with many arguments */
int __attribute__((noinline, noclone)) 
helper_function(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Force side effects */
    volatile int result = 0;
    result = a + b - c * d + e / (f ? f : 1) - g + h;
    
    /* Memory barrier to prevent optimization */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Function with high register pressure across a call */
int __attribute__((noinline))
high_register_pressure(int base) {
    /* Declare many local variables of mixed types */
    int a = base + 1;
    int b = base + 2;
    int c = base + 3;
    int d = base + 4;
    int e = base + 5;
    int f = base + 6;
    int g = base + 7;
    int h = base + 8;
    int i = base + 9;
    int j = base + 10;
    
    /* Floating point variables to use FP registers */
    float fa = base * 1.1f;
    float fb = base * 1.2f;
    float fc = base * 1.3f;
    
    double da = base * 2.1;
    double db = base * 2.2;
    
    long la = base * 100L;
    long lb = base * 200L;
    
    /* Register variable to suggest specific register usage */
    register int r12_var asm("r12") = base * 3;
    
    /* Complex pre-call computations creating web of dependencies */
    int t1 = a * b + c - d;
    int t2 = e / (f ? f : 1) + g - h;
    float ft1 = fa * fb + fc;
    double dt1 = da / (db != 0 ? db : 1.0);
    long lt1 = la + lb * 2;
    
    /* Use the register variable in computation */
    r12_var = r12_var * 2 + t1;
    
    /* Memory barrier before call - makes all variables appear live */
    asm volatile("" : : : "memory");
    
    /* Call with many arguments - forces some to stack on x86-64 */
    int call_result = helper_function(t1, t2, i, j, 
                                      (int)ft1, (int)dt1, 
                                      (int)(lt1 & 0xFFFFFFFF), r12_var);
    
    /* Memory barrier after call */
    asm volatile("" : : : "memory");
    
    /* Complex post-call computations using all variables */
    int result = call_result;
    result += a * 2 - b / 3 + c;
    result += (int)(fa * 2.0f - fb / 3.0f + fc);
    result += (int)(da * 2.0 - db / 3.0);
    result += (int)((la - lb) / 100);
    result += d + e - f * g + h;
    result += i * j - t1 + t2;
    result += r12_var;  /* Keep register variable live */
    
    /* More computations to ensure basic block continues after call */
    float ft2 = ft1 * 2.0f - fa + fb;
    double dt2 = dt1 / 2.0 + da - db;
    long lt2 = lt1 * 3 - la + lb;
    
    result += (int)ft2 + (int)dt2 + (int)(lt2 & 0xFF);
    
    /* Return depends on all computations */
    return result;
}

/* Another helper to increase register pressure differently */
double __attribute__((noinline, noclone))
helper_double(double a, double b, double c, double d,
              double e, double f, double g, double h) {
    volatile double result = a + b - c * d + e / f - g + h;
    asm volatile("" : : : "memory");
    return result;
}

/* Function mixing float and int computations */
int mixed_computations(int seed) {
    int x1 = seed * 11;
    int x2 = seed * 13;
    int x3 = seed * 17;
    int x4 = seed * 19;
    int x5 = seed * 23;
    int x6 = seed * 29;
    int x7 = seed * 31;
    int x8 = seed * 37;
    
    double y1 = seed * 1.11;
    double y2 = seed * 1.13;
    double y3 = seed * 1.17;
    double y4 = seed * 1.19;
    
    /* Pre-call computations */
    double pre1 = y1 * y2 + y3 - y4;
    double pre2 = y4 / y1 + y2 * y3;
    int ipre1 = x1 * x2 + x3 - x4;
    int ipre2 = x5 / (x6 ? x6 : 1) + x7 - x8;
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Call with mixed arguments */
    double dresult = helper_double(pre1, pre2, y1, y2, y3, y4, 
                                   (double)ipre1, (double)ipre2);
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Post-call computations - straight line code */
    int result = (int)dresult;
    result += x1 * 3 - x2 / 2 + x3;
    result += x4 + x5 - x6 * x7 + x8;
    result += (int)(y1 * 3.0 - y2 / 2.0 + y3);
    result += (int)(y4 * pre1 - pre2 / dresult);
    
    /* More arithmetic, no jumps or early returns */
    double post1 = dresult * 2.0 - pre1 + pre2;
    double post2 = y1 * y2 / y3 + y4;
    result += (int)(post1 * post2);
    
    return result;
}

int main() {
    srand(time(NULL));
    int total = 0;
    
    /* Loop to repeatedly exercise the functions */
    for (int i = 0; i < 10000; i++) {
        int input = rand() % 100 + 1;
        
        /* Alternate between functions to exercise different paths */
        if (i % 2 == 0) {
            total += high_register_pressure(input);
        } else {
            total += mixed_computations(input);
        }
        
        /* Prevent loop optimization */
        asm volatile("" : : "r"(total) : "memory");
    }
    
    printf("Final result: %d\n", total);
    return total != 0 ? 0 : 1;
}
