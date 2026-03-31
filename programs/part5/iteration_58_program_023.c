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
    result ^= g | h;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Target function with high register pressure across a call */
int __attribute__((noinline))
target_function(int input1, int input2, int input3, int input4,
                int input5, int input6, int input7, int input8) {
    
    /* Declare many local variables with mixed types */
    int a = input1 + 1;
    int b = input2 * 2;
    int c = input3 - 3;
    int d = input4 / 4;
    int e = input5 ^ 5;
    int f = input6 | 6;
    int g = input7 & 7;
    int h = input8 << 1;
    
    float fa = (float)a * 1.5f;
    float fb = (float)b / 2.0f;
    double da = (double)c * 3.14159;
    double db = (double)d / 2.71828;
    long la = (long)e * 1000L;
    long lb = (long)f * 2000L;
    
    /* Register suggestion for specific variable */
    register int r12_var asm("r12") = a + b;
    
    /* Complex pre-call computations creating dependencies */
    int t1 = a * b + c;
    int t2 = d - e / (f ? f : 1);
    int t3 = g ^ h;
    float ft1 = fa + fb * 2.0f;
    double dt1 = da - db / 3.0;
    long lt1 = la + lb * 3L;
    
    /* Use all variables before the call */
    r12_var += t1 + t2 + t3;
    ft1 += (float)r12_var;
    dt1 += (double)ft1;
    lt1 += (long)dt1;
    
    /* Memory barrier before call - makes all variables appear live */
    asm volatile("" : : : "memory");
    
    /* Call with many arguments - forces some to stack on x86-64 */
    int call_result = helper_function(t1, t2, t3, (int)ft1, 
                                      (int)dt1, (int)lt1, r12_var, input1);
    
    /* Memory barrier after call */
    asm volatile("" : : : "memory");
    
    /* Complex post-call computations using all variables */
    int post1 = call_result * a + b;
    int post2 = c - d * call_result;
    float post3 = fa * (float)call_result + fb;
    double post4 = da / (double)call_result - db;
    long post5 = la + lb * (long)call_result;
    
    /* More computations to ensure BB doesn't end at call */
    r12_var += post1 + post2;
    ft1 = post3 * 2.0f;
    dt1 = post4 / 2.0;
    lt1 = post5 ^ 12345L;
    
    /* Final result using all variables */
    int final_result = (post1 + post2 + (int)post3 + (int)post4 + 
                       (int)post5 + r12_var + (int)ft1 + (int)dt1 + (int)lt1);
    
    /* Prevent tail-call optimization */
    volatile int barrier = final_result;
    asm volatile("" : "+r"(barrier) : : "memory");
    
    return barrier;
}

/* Main function to repeatedly exercise the target */
int main() {
    srand(time(NULL));
    int total_result = 0;
    
    /* Loop to increase coverage probability */
    for (int i = 0; i < 100000; i++) {
        /* Generate random inputs */
        int inputs[8];
        for (int j = 0; j < 8; j++) {
            inputs[j] = rand() % 1000 + 1;
        }
        
        /* Call target function */
        int result = target_function(inputs[0], inputs[1], inputs[2], inputs[3],
                                     inputs[4], inputs[5], inputs[6], inputs[7]);
        
        total_result += result;
        
        /* Prevent loop optimization */
        if (i % 1000 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Total result: %d\n", total_result);
    
    return 0;
}
