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
    float temp2 = d / e + j;
    int temp3 = f ^ g ^ h ^ k;
    double temp4 = i * l + m;
    
    /* Function call within recursion to create more caller-save opportunities */
    if (temp3 % 3 == 0) {
        printf("Recursive depth %d: %f\n", depth, temp1 + temp4);
    }
    
    /* Inline assembly that clobbers caller-saved registers */
    asm volatile (
        "mov %0, %%eax\n\t"
        "add %1, %%eax\n\t"
        "mov %%eax, %0"
        : "+r" (temp3)
        : "r" (depth)
        : "%eax", "%ecx", "%edx", "memory"
    );
    
    /* Recursive call with shuffled arguments */
    return recursive_helper(depth - 1, 
                           b, c, a,          /* Rotate doubles */
                           e, d,             /* Swap floats */
                           g, h, f,          /* Rotate ints */
                           l, i,             /* Swap doubles */
                           m, j,             /* Swap floats */
                           temp3,            /* Modified int */
                           temp4, temp1);    /* New values */
}

int main() {
    /* Declare and initialize many variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    srand(time(NULL));
    
    /* Loop to create long live ranges */
    for (int outer = 0; outer < 3; outer++) {
        /* Complex arithmetic on all variables before function call */
        v1 = v2 * v3 + v4 - v5;
        v2 = v3 ^ v4 | v5;
        v3 = v4 % (v5 + 1) + v1;
        
        f1 = f2 * f3 / f4 + f5;
        f2 = f3 - f4 * f5;
        f3 = sinf(f4) + cosf(f5);  /* External function call */
        
        d1 = d2 * d3 - d4 / d5;
        d2 = pow(d3, 2.0) + sqrt(d4);  /* External function call */
        d3 = d4 * exp(d5);             /* External function call */
        
        /* Function call with many live variables - will need caller-save */
        printf("Iteration %d: v1=%d, f1=%f, d1=%lf\n", 
               outer, v1, f1, d1);
        
        /* Inline assembly that clobbers multiple caller-saved registers */
        asm volatile (
            "mov %0, %%eax\n\t"
            "imul %1, %%eax\n\t"
            "add %2, %%eax\n\t"
            "mov %%eax, %0\n\t"
            "mov %3, %%ecx\n\t"
            "add %%eax, %%ecx\n\t"
            "mov %%ecx, %3"
            : "+r" (v1), "+r" (v4)
            : "r" (v2), "r" (v5)
            : "%eax", "%ecx", "%edx", "memory"
        );
        
        /* Conditional branch - both paths have function calls */
        if (outer % 2 == 0) {
            /* Path 1: Call at end of basic block */
            d4 = sin(d1) * cos(d2);  /* External function call */
            /* This call is at block end before goto */
            v5 = rand() % 100;       /* External function call */
            goto special_path;
        } else {
            /* Path 2: Multiple calls with arithmetic in between */
            f4 = tanf(f1) + atanf(f2);  /* External function call */
            
            /* Arithmetic between calls */
            v3 = v1 * v2 + v4 - v5;
            f5 = f3 * f4 / 2.0f;
            
            f4 = powf(f3, 2.0f);        /* External function call */
            
            /* More arithmetic */
            d5 = d1 + d2 + d3 + d4;
            goto normal_path;
        }
        
    special_path:
        /* Unusual basic block structure with goto */
        d3 = log(d4 + 1.0);  /* External function call */
        
        /* Recursive call with many arguments - high register pressure */
        double result = recursive_helper(2, d1, d2, d3, f1, f2,
                                        v1, v2, v3, d4, f3, v4, d5, f4);
        
        /* Arithmetic after recursive call */
        v1 = (int)result % 100;
        f1 = (float)fmod(result, 10.0);
        goto loop_end;
        
    normal_path:
        /* Different call pattern */
        printf("Normal path: %d %f %lf\n", v3, f5, d5);
        
        /* Another external function call */
        d2 = fabs(d1 - d5) * 2.0;
        
    loop_end:
        /* More arithmetic to keep variables live */
        v4 = v1 + v2 + v3;
        f3 = f1 + f2 + f4 + f5;
        d4 = d1 + d2 + d3 + d5;
        
        /* Another inline assembly block */
        asm volatile (
            "mov %0, %%eax\n\t"
            "mov %1, %%ebx\n\t"
            "add %%ebx, %%eax\n\t"
            "mov %%eax, %0\n\t"
            "mov %2, %%ecx\n\t"
            "sub %%eax, %%ecx\n\t"
            "mov %%ecx, %2"
            : "+r" (v5), "+r" (v2)
            : "r" (v3)
            : "%eax", "%ebx", "%ecx", "%edx", "memory"
        );
    }
    
    /* Final computation using all variables */
    double checksum = v1 + v2 + v3 + v4 + v5 +
                     f1 + f2 + f3 + f4 + f5 +
                     d1 + d2 + d3 + d4 + d5;
    
    /* One more external function call near the end */
    checksum = fabs(checksum) + sin(checksum);
    
    printf("Final checksum: %lf\n", checksum);
    
    return (int)checksum % 256;
}
