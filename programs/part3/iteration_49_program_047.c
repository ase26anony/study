#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function that uses many arguments to increase register pressure */
double recursive_helper(int depth, double a, double b, double c, float d, float e, 
                        int f, int g, int h, double i, float j, int k, int l, int m) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m;
    }
    
    /* Mix operations to keep variables live */
    double t1 = a * b - c;
    float t2 = d / e + j;
    int t3 = f ^ g | h;
    double t4 = i * 2.0 + (double)j;
    int t5 = k + l * m;
    
    /* Function call within recursion to force caller-save */
    if (depth % 2 == 0) {
        printf("Depth %d: t1=%.3f t2=%.3f\n", depth, t1, t2);
    } else {
        t1 = sin(t1) + cos(t2);
    }
    
    /* Recursive call with modified arguments */
    return recursive_helper(depth - 1, t1, b + 1.0, c - 0.5, t2, e * 1.1f,
                           t3, g + 1, h - 1, t4, j * 0.9f, t5, l, m) * 0.95;
}

int main() {
    /* Declare and initialize many variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    srand(time(NULL));
    
    /* Loop to create register pressure */
    for (int outer = 0; outer < 3; outer++) {
        /* Block with arithmetic before calls */
        v1 = v2 * v3 + v4;
        f1 = f2 / f3 * f4;
        d1 = d2 + d3 - d4;
        
        /* Inline assembly that clobbers caller-saved registers */
        asm volatile (
            "addl %1, %0\n\t"
            : "+r"(v1) : "r"(v2) : "%eax", "%ecx", "%edx", "%esi", "%edi"
        );
        
        /* Function call with many live variables */
        printf("Iteration %d: v1=%d f1=%.3f d1=%.3f\n", outer, v1, f1, d1);
        
        /* More arithmetic to keep variables live across calls */
        v5 = v6 + v1 * 2;
        f2 = f1 + sinf(f3);
        d2 = d1 * 1.5 + pow(d3, 1.5);
        
        /* Conditional branch - both paths contain function calls */
        if (outer % 2 == 0) {
            /* Path 1: Call at end of basic block */
            d3 = sin(d2) * cos(d1);
            f3 = tanf(f2) + 1.0f;
            v6 = rand() % 100 + v5;  /* Function call at block end */
            goto special_path;       /* Creates unusual BB structure */
        } else {
            /* Path 2: Multiple calls interleaved with arithmetic */
            v3 = abs(v2 - v1);
            d4 = log(d3 + 1.0);
            v4 = v3 * v5 + rand();   /* Call in middle */
            f4 = sqrtf(f3 * f2);
        }
        
        /* Continue normal execution */
        v2 = v4 | v3 & v6;
        d5 = exp(d4 * 0.5);
        
        /* Another function call */
        double result = pow(d5, 2.0) + (double)v2;
        printf("Power result: %.3f\n", result);
        
        /* Recursive call with many arguments */
        double rec_result = recursive_helper(2, d1, d2, d3, f1, f2,
                                           v1, v2, v3, d4, f3, v4, v5, v6);
        
        /* Arithmetic after recursive call */
        v1 += (int)rec_result;
        f1 += (float)fmod(rec_result, 2.0);
        d1 += rec_result * 0.1;
        
        continue;
        
    special_path:
        /* Unusual basic block created by goto */
        /* Function call at end of this basic block */
        d4 = atan2(d3, d2) * 180.0 / 3.14159265;
        v5 = (int)(d4) + v6;
        printf("Special path: d4=%.3f v5=%d\n", d4, v5);
        
        /* Another inline assembly block */
        asm volatile (
            "movl %1, %%eax\n\t"
            "imull %2, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r"(v6) : "r"(v5), "r"(v4) : "%eax", "%ecx", "%edx"
        );
    }
    
    /* Final computation using all variables */
    double checksum = d1 + d2 + d3 + d4 + d5 + 
                     f1 + f2 + f3 + f4 +
                     v1 + v2 + v3 + v4 + v5 + v6;
    
    /* One more function call before return */
    printf("Final checksum: %.6f\n", checksum);
    
    /* Additional conditional with call at block end */
    if (checksum > 100.0) {
        v1 = (int)sqrt(checksum);
        printf("Large checksum: %d\n", v1);  /* Call at end of BB */
    } else {
        v1 = (int)log(checksum + 1.0);
        printf("Small checksum: %d\n", v1);  /* Call at end of BB */
    }
    
    return v1 % 256;
}
