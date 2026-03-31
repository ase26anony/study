#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function that uses many arguments to create register pressure */
double recursive_helper(int depth, double a, double b, float c, float d, 
                        int e, int f, double g, float h, int i, 
                        double j, float k, int l, double m, int n) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m + n;
    }
    
    /* Mix all variables in complex ways */
    double t1 = a * b + c * d;
    float t2 = e * f + g * h;
    int t3 = i * l + n * depth;
    double t4 = j * m + k * depth;
    
    /* Call external functions within recursion */
    double s1 = sin(t1);
    double s2 = pow(t4, 2.0);
    
    /* Inline assembly that clobbers caller-saved registers */
    __asm__ volatile (
        "add %1, %0\n\t"
        "imul %2, %0"
        : "+r" (t3)
        : "r" (depth), "r" (e)
        : "%eax", "%ecx", "%edx", "cc"
    );
    
    /* Recursive call with shuffled arguments */
    return s1 + s2 + recursive_helper(depth - 1,
                                      b, t1, k, t2,
                                      t3, f, s1, h, l,
                                      m, d, n, j, i);
}

int main() {
    /* Declare and initialize 15 local variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    srand(time(NULL));
    double checksum = 0.0;
    
    /* First loop - creates register pressure */
    for (int i = 0; i < 4; i++) {
        /* Arithmetic on all variables before call */
        v1 = v2 * v3 + i;
        v2 = v4 - v5 * i;
        v3 = v1 ^ v2;
        v4 = v5 + rand() % 100;
        v5 = v3 * v4 - i;
        
        f1 = f2 + f3 * i;
        f2 = f4 / (f5 + 1.0f);
        f3 = sinf(f1) + cosf(f2);
        f4 = f5 * 2.0f - f3;
        f5 = f1 + f2 + f3 + f4;
        
        d1 = d2 * d3 + i;
        d2 = d4 / (d5 + 1.0);
        d3 = sin(d1) + cos(d2);
        d4 = d5 * 2.0 - d3;
        d5 = d1 + d2 + d3 + d4;
        
        /* Function call with many live variables */
        printf("Iteration %d: v1=%d, f1=%.2f, d1=%.2f\n", 
               i, v1, f1, d1);
        
        /* Inline assembly clobbering caller-saved registers */
        __asm__ volatile (
            "mov %1, %%eax\n\t"
            "add %2, %%eax\n\t"
            "mov %%eax, %0\n\t"
            "imul %3, %%ecx"
            : "=r" (v1)
            : "r" (v2), "r" (v3), "r" (i)
            : "%eax", "%ecx", "%edx", "cc"
        );
        
        /* Conditional branch - both paths have function calls */
        if (i % 2 == 0) {
            /* Path 1: Call at end of basic block */
            d1 = pow(d2, d3) + sin(d4);
            checksum += d1 + d5;
            
            /* Function call right before goto (potentially at BB end) */
            printf("Even iteration: pow=%.3f\n", pow(d1, 2.0));
            goto process_data;  /* Creates BB boundary */
        } else {
            /* Path 2: Multiple calls in sequence */
            f1 = sinf(f2) * cosf(f3);
            printf("Odd iteration: f1=%.3f\n", f1);
            
            /* Another call with different arguments */
            d2 = pow(f4, 3.0) + tan(d3);
            printf("Math result: %.3f\n", d2);
            
            /* Continue normally */
            checksum += f1 + d2;
        }
        
        /* Label creating basic block boundary */
        process_data:
        
        /* Recursive call with all variables live */
        double rec_result = recursive_helper(
            2,  /* depth */
            d1, d2, f1, f2,
            v1, v2, d3, f3, v3,
            d4, f4, v4, d5, v5
        );
        
        checksum += rec_result;
        
        /* More arithmetic after calls */
        v1 = v2 + v3 * v4;
        f1 = f2 - f3 / f4;
        d1 = d2 * d3 + d4;
        
        /* Another external function call */
        double rand_val = (double)rand() / RAND_MAX;
        d5 = d1 * sin(rand_val) + cos(checksum);
        
        /* Second inline assembly block */
        __asm__ volatile (
            "add %1, %0\n\t"
            "sub %2, %0\n\t"
            "mov %0, %%eax"
            : "+r" (v5)
            : "r" (i), "r" (v4)
            : "%eax", "cc"
        );
    }
    
    /* Final computation using all variables */
    checksum += v1 + v2 + v3 + v4 + v5;
    checksum += f1 + f2 + f3 + f4 + f5;
    checksum += d1 + d2 + d3 + d4 + d5;
    
    /* One more function call near the end */
    printf("Final checksum: %.6f\n", checksum);
    
    /* Additional conditional with call at block end */
    if (checksum > 1000.0) {
        printf("Large checksum detected: %.2f\n", checksum);
        /* Call at potential BB end before return */
        checksum = sqrt(fabs(checksum));
    } else {
        printf("Small checksum: %.2f\n", checksum);
        /* Different call pattern */
        checksum = pow(checksum, 1.5);
    }
    
    return (int)(checksum * 1000) % 1000;
}
