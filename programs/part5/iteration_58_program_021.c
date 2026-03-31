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
    
    /* Memory side effect */
    static int counter = 0;
    counter++;
    
    return result + counter;
}

/* Target function with high register pressure across a call */
int __attribute__((noinline))
high_register_pressure(int seed) {
    /* Declare many local variables of mixed types */
    int a = seed * 1;
    int b = seed * 2;
    int c = seed * 3;
    int d = seed * 4;
    int e = seed * 5;
    int f = seed * 6;
    int g = seed * 7;
    int h = seed * 8;
    int i = seed * 9;
    int j = seed * 10;
    
    /* Floating point variables to use FP registers */
    float fa = seed * 1.1f;
    float fb = seed * 2.2f;
    double da = seed * 1.11;
    double db = seed * 2.22;
    
    /* Long variables for more register pressure */
    long la = seed * 100L;
    long lb = seed * 200L;
    
    /* Suggest specific register for one variable */
    register int r12_var asm("r12") = seed * 123;
    
    /* Complex pre-call computations creating dependencies */
    int t1 = a * b + c - d;
    int t2 = e / (f ? f : 1) + g * h;
    float ft1 = fa * fb + (float)c;
    double dt1 = da / (db != 0.0 ? db : 1.0) + (double)d;
    long lt1 = la - lb * (long)e;
    
    /* Use r12_var in computation */
    t1 += r12_var;
    lt1 ^= (long)r12_var;
    
    /* Memory barrier - makes all variables appear live */
    asm volatile("" : : : "memory");
    
    /* Call with many arguments - forces register/stack pressure */
    int helper_result = helper_function(t1, t2, i, j, 
                                       (int)ft1, (int)dt1, 
                                       (int)(lt1 & 0xFFFFFFFF),
                                       seed ^ 0x55AA);
    
    /* Memory barrier after call */
    asm volatile("" : : : "memory");
    
    /* Complex post-call computations using all variables */
    int result = helper_result;
    result += a * 2;
    result -= b / 3;
    result ^= c << 1;
    result += d - e;
    result *= f ^ g;
    result += h * i;
    result -= j;
    
    /* Use floating point results */
    result += (int)(fa * 2.0f);
    result -= (int)(fb / 3.0f);
    result ^= (int)da;
    result += (int)db;
    
    /* Use long results */
    result += (int)(la >> 4);
    result ^= (int)(lb & 0xFF);
    
    /* Ensure r12_var is used after call */
    result += r12_var * 2;
    
    /* More complex expressions to extend live ranges */
    float ft2 = ft1 * 3.14f + fa - fb;
    double dt2 = dt1 * 2.71828 + da - db;
    long lt2 = lt1 * 37L + la - lb;
    
    result += (int)ft2;
    result ^= (int)dt2;
    result += (int)(lt2 & 0xFFFF);
    
    return result;
}

/* Main function to repeatedly exercise the target */
int main() {
    srand(time(NULL));
    int total = 0;
    
    /* Loop to increase coverage probability */
    for (int iteration = 0; iteration < 1000; iteration++) {
        /* Vary inputs to prevent constant propagation */
        int input = rand() % 1000 + 1;
        
        /* Call the high-pressure function */
        int result = high_register_pressure(input);
        
        /* Accumulate result to prevent dead code elimination */
        total += result;
        
        /* Occasionally print to prevent optimization */
        if (iteration % 100 == 0) {
            printf("Iteration %d: result = %d, total = %d\n", 
                   iteration, result, total);
        }
    }
    
    /* Final print to ensure all code is live */
    printf("Final total: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
