#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function that uses many arguments */
double recursive_helper(int depth, double a, double b, float c, float d, 
                       int e, int f, double g, float h, int i, 
                       double j, float k, int l, double m, float n) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m + n;
    }
    
    /* Create register pressure in recursion */
    double t1 = a * b + c * d;
    float t2 = e * f + g * h;
    int t3 = i * j + k * l;
    
    /* Function call within recursion */
    printf("Depth %d: %f %f\n", depth, t1, (double)t2);
    
    /* Inline assembly clobbering caller-saved registers */
    asm volatile (
        "mov %0, %%eax\n\t"
        "add %1, %%eax\n\t"
        "mov %%eax, %0"
        : "+r"(e)
        : "r"(f)
        : "%eax", "%ecx", "%edx"
    );
    
    return recursive_helper(depth - 1, 
                           t1, b * 0.9, t2, d * 1.1,
                           e + t3, f - 1, g * 0.8, h * 1.2,
                           i / 2, j * 1.5, k * 0.7, l * 2,
                           m + depth, n * 0.95);
}

int main() {
    /* Declare and initialize many variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    srand(time(NULL));
    
    /* Loop creating register pressure */
    for (int outer = 0; outer < 3; outer++) {
        /* Complex arithmetic on all variables before call */
        v1 = v1 * 2 + outer;
        v2 = v2 / 2 + v1;
        v3 = v3 * 3 - outer;
        v4 = v4 + v1 * v2;
        v5 = v5 - v3 * 2;
        
        f1 = f1 * 1.5f + outer;
        f2 = f2 / 1.3f + f1;
        f3 = f3 * 2.0f - outer * 0.5f;
        f4 = f4 + f1 * f2;
        f5 = f5 - f3 * 1.1f;
        
        d1 = d1 * 1.7 + outer;
        d2 = d2 / 1.4 + d1;
        d3 = d3 * 2.1 - outer * 0.3;
        d4 = d4 + d1 * d2;
        d5 = d5 - d3 * 1.2;
        
        /* Function call with many live variables */
        printf("Iteration %d: v1=%d, f1=%f, d1=%f\n", 
               outer, v1, f1, d1);
        
        /* More arithmetic between calls */
        v1 = v1 + (int)sin(d1);
        f2 = f2 + (float)cos(d2);
        d3 = d3 + pow(d4, 1.5);
        
        /* Inline assembly clobbering multiple caller-saved registers */
        asm volatile (
            "mov %0, %%eax\n\t"
            "imul %1, %%eax\n\t"
            "add %%eax, %2\n\t"
            "mov %3, %%ecx\n\t"
            "add %%ecx, %%eax"
            : "+r"(v1), "+r"(v2), "+r"(v3)
            : "r"(v4)
            : "%eax", "%ecx", "%edx", "memory"
        );
        
        /* Conditional branch creating basic block boundary */
        if (outer % 2 == 0) {
            /* Path with function call at end of basic block */
            double result = pow(d1, d2) + sin(d3) - cos(d4);
            printf("Even: pow=%f, sin=%f\n", pow(d1, d2), sin(d3));
            /* Function call right before goto - potential BB_END */
            f4 = f4 + (float)tan(d5);
            goto process_data;  /* Creates BB boundary after call */
        } else {
            /* Different path with different call pattern */
            float result = (float)(log(d1) + exp(d2));
            printf("Odd: log=%f, exp=%f\n", log(d1), exp(d2));
            /* Another call before block end */
            v5 = v5 + (int)(d3 * 100);
        }
        
        /* Label creating basic block structure */
        process_data:
        
        /* Recursive call with many arguments */
        double rec_result = recursive_helper(
            2,  /* depth */
            d1 + d2, d3 * 0.5, 
            f1 + f2, f3 * 1.5,
            v1 * v2, v3 + v4,
            d4 / 2.0, f4 * 0.8,
            v5 % 7, d5 * 1.1,
            f5 * 0.9, v1 % 3,
            d1 * d2, f3 * f4
        );
        
        /* More arithmetic after recursive call */
        v1 = v1 + (int)rec_result;
        f1 = f1 + (float)fmod(rec_result, 2.0);
        d1 = d1 + rec_result * 0.1;
        
        /* Another inline assembly block */
        asm volatile (
            "mov %0, %%eax\n\t"
            "cvtsi2sd %%eax, %%xmm0\n\t"
            "addsd %1, %%xmm0\n\t"
            "cvtsd2si %%xmm0, %0"
            : "+r"(v2)
            : "m"(d2)
            : "%eax", "%xmm0", "%xmm1"
        );
        
        /* Call to external math function */
        d3 = sin(d4) + cos(d5);
        
        /* Complex loop with nested conditions */
        for (int inner = 0; inner < 2; inner++) {
            /* More register pressure in inner loop */
            int temp = v1 * inner + v2;
            float ftemp = f1 * inner + f2;
            double dtemp = d1 * inner + d2;
            
            /* Function call in loop */
            if (inner == 0) {
                printf("Inner %d: %d %f %f\n", inner, temp, ftemp, dtemp);
            }
            
            /* Random call to force spills */
            v3 = v3 + rand() % 10;
            
            /* More arithmetic */
            f3 = f3 + ftemp * 0.5f;
            d4 = d4 + dtemp * 0.3;
        }
    }
    
    /* Final computation using all variables */
    double checksum = 
        v1 + v2 + v3 + v4 + v5 +
        f1 + f2 + f3 + f4 + f5 +
        d1 + d2 + d3 + d4 + d5;
    
    /* One more function call before return */
    printf("Final checksum: %f\n", checksum);
    
    /* Additional call right before return - potential BB_END */
    checksum = checksum + sin(checksum) * 0.01;
    
    return (int)(checksum * 1000) % 100;
}
