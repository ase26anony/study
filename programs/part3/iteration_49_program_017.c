#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function that uses many arguments to create register pressure */
double recursive_helper(int depth, double a, double b, double c, float d, float e, 
                        int f, int g, int h, double i, float j, int k, double l, float m) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m;
    }
    
    /* Mix in some external function calls */
    double sin_val = sin(a + b);
    double pow_val = pow(c, 2.0);
    
    /* Inline assembly that clobbers caller-saved registers */
    int temp_f = f;
    int temp_g = g;
    __asm__ volatile (
        "addl %%ebx, %%eax\n\t"
        "imull %%ecx, %%edx"
        : "+a"(temp_f), "+d"(temp_g)
        : "b"(depth), "c"(h)
        : "%ebx", "%ecx", "%edx"
    );
    f = temp_f;
    g = temp_g;
    
    /* More arithmetic to extend live ranges */
    a = a * 1.1 + sin_val;
    b = b / 1.2 - pow_val;
    c = c + depth * 0.5;
    d = d * 1.3f;
    e = e / 1.4f;
    
    /* Recursive call with modified arguments */
    return recursive_helper(depth - 1, a, b, c, d, e, f, g, h, i, j, k, l, m) * 0.95;
}

int main() {
    /* Declare and initialize many local variables of mixed types */
    double a = 1.0, b = 2.0, c = 3.0, i = 9.0, l = 12.0;
    float d = 4.0f, e = 5.0f, j = 10.0f, m = 13.0f;
    int f = 6, g = 7, h = 8, k = 11, n = 14, o = 15, p = 16;
    
    /* Seed random number generator */
    srand(time(NULL));
    
    double total = 0.0;
    int iteration;
    
    /* Outer loop to create extended live ranges */
    for (iteration = 0; iteration < 4; iteration++) {
        /* Perform arithmetic on all variables before any call */
        a += iteration * 0.1;
        b -= iteration * 0.2;
        c *= 1.0 + iteration * 0.05;
        d += sin(a);
        e += cos(b);
        f += rand() % 10;
        g -= iteration;
        h *= (iteration + 1);
        i = pow(i, 1.01);
        j = j * 1.1f;
        k += k / 2;
        l = sqrt(l + iteration);
        m = m * 0.9f;
        n = n ^ iteration;
        o = o + n;
        p = p * 2 - iteration;
        
        /* Conditional branch - both paths contain function calls */
        if (iteration % 2 == 0) {
            /* Path 1: Call external functions with many arguments */
            printf("Iteration %d: a=%.3f, b=%.3f, c=%.3f\n", iteration, a, b, c);
            
            /* Function call with multiple arguments - creates register pressure */
            double sin_result = sin(a + b + c);
            double pow_result = pow(d * e, 2.0);
            
            /* Inline assembly clobbering caller-saved registers */
            int temp_n = n;
            int temp_o = o;
            __asm__ volatile (
                "movl %%eax, %%ebx\n\t"
                "addl %%ecx, %%edx\n\t"
                "xorl %%esi, %%edi"
                : "+b"(temp_n), "+d"(temp_o)
                : "a"(f), "c"(g), "S"(h), "D"(iteration)
                : "%eax", "%ecx", "%edx", "%esi", "%edi"
            );
            n = temp_n;
            o = temp_o;
            
            /* Call at the end of a basic block (just before goto) */
            total += sin_result + pow_result;
            goto process_block;
        } else {
            /* Path 2: Different call pattern */
            /* Call external function */
            double log_val = log(fabs(c) + 1.0);
            
            /* More arithmetic between calls */
            a = a * 1.5 - log_val;
            b = b / 1.6 + log_val;
            
            /* Another external function call */
            double exp_result = exp(d + e);
            
            /* Call recursive helper with many arguments */
            double recursive_result = recursive_helper(
                2, a, b, c, d, e, f, g, h, i, j, k, l, m
            );
            
            total += recursive_result + exp_result;
            
            /* Function call right before label (potential BB_END) */
            printf("Alt path: total=%.3f\n", total);
            goto alternate_block;
        }
        
    process_block:
        /* Block with operations after goto */
        i += total * 0.01;
        j -= total * 0.02f;
        
        /* Another function call */
        double cos_result = cos(i + j);
        total += cos_result;
        
        continue;
        
    alternate_block:
        /* Alternate block with different operations */
        k += (int)total;
        l = l * 1.05;
        
        /* Function call at block end */
        m = m + (float)sin(l);
        
        /* Inline assembly with different clobbered registers */
        double temp_a = a;
        __asm__ volatile (
            "addsd %1, %0\n\t"
            "mulsd %2, %0"
            : "+x"(temp_a)
            : "x"(b), "x"(c)
            : "%xmm0", "%xmm1", "%xmm2"
        );
        a = temp_a;
    }
    
    /* Final computation using all variables */
    double checksum = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
    
    /* Add some more operations and a final call */
    checksum = checksum * 1.1 - total;
    
    /* One more external function call */
    checksum = fabs(checksum) + sin(checksum);
    
    /* Final printf to prevent dead code elimination */
    printf("Final checksum: %.6f\n", checksum);
    
    return (int)(checksum * 1000) % 1000;
}
