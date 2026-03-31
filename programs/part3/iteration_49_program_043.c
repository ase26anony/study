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
    
    /* Mix up the arguments to prevent optimization */
    double temp1 = a * b - c;
    float temp2 = d / (e + 1.0f);
    int temp3 = (f ^ g) | h;
    double temp4 = sin(i) * cos(l);
    float temp5 = j * m * 2.0f;
    
    /* Function call within recursion to create more caller-save opportunities */
    if (depth % 2 == 0) {
        printf("Recursive depth %d: %f %f\n", depth, temp1, temp4);
    }
    
    /* Inline assembly that clobbers caller-saved registers */
    asm volatile (
        "mov %0, %%eax\n\t"
        "add %1, %%eax\n\t"
        "mov %%eax, %0"
        : "+r"(temp3)
        : "r"(depth)
        : "%eax", "%ecx", "%edx", "cc"
    );
    
    return recursive_helper(depth - 1, 
                           temp1, b + 1.0, c * 0.9, 
                           temp2, e * 1.1f,
                           temp3, g >> 1, h << 1,
                           temp4, j + 0.5f, k - 1,
                           l * 0.8, temp5);
}

int main() {
    /* Declare and initialize many variables of mixed types */
    double d1 = 1.0, d2 = 2.0, d3 = 3.0, d4 = 4.0;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    int i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, i6 = 6, i7 = 7;
    
    srand(time(NULL));
    
    /* Loop to create long live ranges */
    for (int outer = 0; outer < 3; outer++) {
        /* Block with arithmetic before calls */
        d1 = d1 * 1.5 + sin(d2);
        d2 = d2 / 1.3 + cos(d3);
        d3 = d3 * 0.9 + tan(d4);
        d4 = d4 - 0.1 * pow(d1, 1.5);
        
        f1 = f1 * 1.2f + (float)sin(f2);
        f2 = f2 / 1.4f + (float)cos(f3);
        f3 = f3 * 0.8f + (float)tan(f4);
        f4 = f4 - 0.2f * powf(f1, 1.3f);
        
        i1 = i1 * 3 + rand() % 10;
        i2 = i2 * 5 - rand() % 7;
        i3 = i3 * 7 ^ rand() % 15;
        i4 = i4 * 11 | rand() % 31;
        i5 = i5 * 13 & rand() % 63;
        i6 = i6 * 17 + rand() % 127;
        i7 = i7 * 19 - rand() % 255;
        
        /* Inline assembly clobbering multiple caller-saved registers */
        asm volatile (
            "mov %0, %%eax\n\t"
            "imul %1, %%eax\n\t"
            "add %2, %%eax\n\t"
            "mov %%eax, %0\n\t"
            "mov %3, %%ecx\n\t"
            "add %%eax, %%ecx\n\t"
            "mov %%ecx, %3"
            : "+r"(i1), "+r"(i2)
            : "r"(i3), "r"(i4)
            : "%eax", "%ecx", "%edx", "cc"
        );
        
        /* Function call with many live variables */
        printf("Iteration %d: d1=%f d2=%f f1=%f i1=%d i2=%d\n", 
               outer, d1, d2, f1, i1, i2);
        
        /* Conditional branch creating basic block boundaries */
        if (outer % 2 == 0) {
            /* Path 1: Call at end of basic block */
            d1 = pow(d1, d2) + sin(d3);
            f1 = powf(f1, f2) + sinf(f3);
            
            /* Function call right before label (potential BB_END) */
            double result1 = recursive_helper(2, d1, d2, d3, f1, f2, 
                                             i1, i2, i3, d4, f3, i4, d1, f4);
            
            /* This call is at the end of a basic block before goto */
            printf("Even path result: %f\n", result1);
            
            goto special_block;
        } else {
            /* Path 2: Different call pattern */
            d2 = log(d2) * exp(d4);
            f2 = logf(f2) * expf(f4);
            
            /* Multiple calls in sequence */
            double result2 = sin(d1) + cos(d2);
            result2 += tan(d3) * atan(d4);
            
            printf("Odd path: %f\n", result2);
            
            /* Call to external math function */
            d3 = fmod(d3, 2.5) + fabs(d4);
            
            continue; /* Creates another basic block boundary */
        }
        
special_block:
        /* Unusual basic block structure with goto */
        d4 = d4 * 2.0 - 1.0;
        f4 = f4 * 1.5f - 0.5f;
        
        /* Another inline assembly block */
        asm volatile (
            "mov %0, %%eax\n\t"
            "mov %1, %%ebx\n\t"
            "add %%ebx, %%eax\n\t"
            "mov %%eax, %0"
            : "+r"(i5)
            : "r"(i6)
            : "%eax", "%ebx", "cc"
        );
        
        /* Function call with mixed arguments */
        double result3 = recursive_helper(1, d1, d2, d3, f1, f2,
                                         i5, i6, i7, d4, f3, i4, d2, f4);
        
        /* Arithmetic after call keeps variables live */
        i7 = (int)(result3 * 1000) ^ i1;
        
        /* Another call at potential block end */
        if (outer == 1) {
            printf("Special block result: %f\n", result3);
            /* This could be BB_END if followed by label or return */
        }
    }
    
    /* Final computation using all variables */
    double checksum = d1 + d2 + d3 + d4 + 
                     f1 + f2 + f3 + f4 + 
                     i1 + i2 + i3 + i4 + i5 + i6 + i7;
    
    /* Force all variables to be used in printf */
    printf("Final checksum: %f\n", checksum);
    printf("Variables: d1=%f d2=%f d3=%f d4=%f f1=%f f2=%f f3=%f f4=%f\n",
           d1, d2, d3, d4, f1, f2, f3, f4);
    printf("Integers: i1=%d i2=%d i3=%d i4=%d i5=%d i6=%d i7=%d\n",
           i1, i2, i3, i4, i5, i6, i7);
    
    return (int)checksum % 256;
}
