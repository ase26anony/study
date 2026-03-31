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
    
    /* Mix operations and calls */
    double temp1 = a * b - c;
    float temp2 = d + e * 2.0f;
    int temp3 = f ^ g | h;
    
    /* External function call within recursion */
    double sin_val = sin(temp1);
    
    /* More arithmetic */
    temp1 = temp1 + i * 0.5;
    temp2 = temp2 * j - m;
    temp3 = temp3 + k * 3;
    
    /* Inline assembly clobbering caller-saved registers */
    asm volatile (
        "add %1, %0\n\t"
        "imul %2, %0"
        : "+r"(temp3)
        : "r"(g), "r"(h)
        : "%eax", "%ecx", "%edx", "cc"
    );
    
    /* Recursive call with shuffled arguments */
    return sin_val + recursive_helper(depth - 1, 
                                     b, c, a, 
                                     e, d, 
                                     g, h, f,
                                     i * 0.8, j * 1.1, k + 1,
                                     l * sin_val, m * 0.9f);
}

int main() {
    /* Declare and initialize many variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    srand(time(NULL));
    double checksum = 0.0;
    
    /* Loop creating long live ranges */
    for (int outer = 0; outer < 3; outer++) {
        /* Block with arithmetic before calls */
        v1 = v2 * v3 + v4;
        v5 = v6 ^ v1;
        f1 = f2 * 3.14f - f3;
        f4 = f1 + f2 / 2.0f;
        d1 = d2 * 1.5 + d3;
        d4 = d5 - d1 * 0.3;
        d3 = d4 / 2.0;
        
        /* External function call 1 - printf */
        printf("Iteration %d: v1=%d, f1=%.2f, d1=%.3f\n", 
               outer, v1, f1, d1);
        
        /* Inline assembly clobbering multiple caller-saved registers */
        asm volatile (
            "mov %1, %%eax\n\t"
            "add %2, %%eax\n\t"
            "imul %3, %%eax\n\t"
            "mov %%eax, %0\n\t"
            : "=r"(v2)
            : "r"(v3), "r"(v4), "r"(v5)
            : "%eax", "%ecx", "%edx", "cc"
        );
        
        /* Conditional branch - both paths contain function calls */
        if (outer % 2 == 0) {
            /* Path 1: Call at end of basic block */
            d2 = pow(d1, 2.0) + d3;
            f3 = sinf(f4) * cosf(f1);
            v3 = rand() % 100 + v6;  /* Call at block end before goto */
            
            /* Jump to create unusual BB structure */
            goto compute_block;
        } else {
            /* Path 2: Different call pattern */
            d5 = sqrt(d4) * 2.0;
            f2 = expf(f3 / 2.0f);
            v4 = abs(v5 - v2);  /* Another call */
            
            /* Continue normally */
        }
        
        /* Label creating basic block boundary */
        compute_block:
        
        /* More arithmetic mixing */
        v6 = v1 + v2 * v3 - v4;
        f1 = f3 + f4 * f2;
        d1 = d2 + d3 * d4 - d5;
        
        /* External function call 2 - math function */
        double cos_val = cos(d1);
        d2 = d2 + cos_val;
        
        /* Inline assembly with different clobbers */
        asm volatile (
            "xchg %1, %0\n\t"
            "sub %2, %0"
            : "+r"(v5), "+r"(v6)
            : "r"(v1)
            : "%eax", "%ebx", "cc"
        );
        
        /* Recursive call with many live variables */
        double rec_result = recursive_helper(2, 
                                           d1, d2, d3,
                                           f1, f2,
                                           v1, v2, v3,
                                           d4, f3, v4,
                                           d5, f4);
        
        /* External function call 3 - printf with many args */
        printf("Recursive result: %.6f (v5=%d, v6=%d)\n", 
               rec_result, v5, v6);
        
        /* Arithmetic after calls */
        v1 = v2 + v3 * 2;
        f4 = f1 * 1.1f - f2;
        d3 = d4 / 1.7 + d5;
        
        /* Another conditional with call at block end */
        if (v1 > 100) {
            d4 = log(d3 + 1.0);
            f3 = powf(f4, 1.5f);
            v2 = rand() % 50;  /* Call right before block end */
        }
        
        /* Update checksum */
        checksum += v1 + v2 + v3 + v4 + v5 + v6 +
                   f1 + f2 + f3 + f4 +
                   d1 + d2 + d3 + d4 + d5;
        
        /* Loop update with arithmetic */
        v1 += outer;
        v2 *= (outer + 1);
        f1 += outer * 0.5f;
        d1 *= (1.0 + outer * 0.1);
    }
    
    /* Final external function call */
    printf("Final checksum: %.15f\n", checksum);
    
    /* Use goto to create another BB boundary with call at end */
    if (checksum > 1000.0) {
        goto final_print;
    }
    
    /* More arithmetic */
    d1 = d2 * 2.0 - d3;
    
    final_print:
    /* One more call at what could be BB_END */
    printf("Program completed successfully.\n");
    
    return (int)(checksum * 1000) % 100;
}
