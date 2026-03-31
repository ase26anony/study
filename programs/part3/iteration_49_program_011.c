#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function that uses many arguments to create register pressure */
double recursive_helper(int depth, double a, double b, float c, float d, 
                        int e, int f, double g, float h, int i, 
                        double j, float k, int l, double m, float n) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m + n;
    }
    
    /* Mix operations to keep variables live */
    double t1 = a * b + c * d;
    float t2 = e * f + g * h;
    int t3 = i * l + (int)(j * k);
    double t4 = m * n + t1;
    
    /* Call external functions within recursion */
    double s1 = sin(t1);
    double s2 = pow(t4, 1.5);
    
    /* Inline assembly that clobbers caller-saved registers */
    asm volatile (
        "mov %0, %%eax\n\t"
        "mov %1, %%ecx\n\t"
        "add %%ecx, %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "+r"(t3)
        : "r"(depth)
        : "%eax", "%ecx", "%edx", "cc"
    );
    
    /* Recursive call with shuffled arguments to prevent optimization */
    return recursive_helper(depth - 1, 
                           b, a, d, c, 
                           f, e, h, g, 
                           l, i, k, j, 
                           n, m) + s1 + s2 + t3;
}

int main() {
    /* Declare and initialize 15 local variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    srand(time(NULL));
    double checksum = 0.0;
    
    /* Start with a goto to create unusual basic block structure */
    goto start_block;
    
    /* This label creates a basic block boundary */
    call_block1:
    {
        /* Function call at the end of a basic block (just before goto) */
        printf("Call block 1: v1=%d, f1=%.2f, d1=%.2f\n", v1, f1, d1);
        goto loop_body;
    }
    
    start_block:
    /* Initial arithmetic to make variables live */
    v1 = rand() % 100;
    f1 = sinf((float)v1);
    d1 = pow(v1, 2);
    
    /* Loop creating register pressure */
    for (int outer = 0; outer < 3; outer++) {
        loop_body:
        for (int inner = 0; inner < 2; inner++) {
            /* Extensive arithmetic operations on all variables */
            v1 = v2 * v3 + v4 - v5;
            v2 = v3 * v4 + v5 - v1;
            v3 = v4 * v5 + v1 - v2;
            v4 = v5 * v1 + v2 - v3;
            v5 = v1 * v2 + v3 - v4;
            
            f1 = f2 * f3 + f4 / f5;
            f2 = f3 * f4 + f5 / f1;
            f3 = f4 * f5 + f1 / f2;
            f4 = f5 * f1 + f2 / f3;
            f5 = f1 * f2 + f3 / f4;
            
            d1 = d2 * d3 + d4 - d5;
            d2 = d3 * d4 + d5 - d1;
            d3 = d4 * d5 + d1 - d2;
            d4 = d5 * d1 + d2 - d3;
            d5 = d1 * d2 + d3 - d4;
            
            /* Conditional branch where both paths contain function calls */
            if ((v1 + v2 + v3) % 3 == 0) {
                /* Path 1: Multiple function calls with live variables */
                double p1 = pow(d1, f1);
                double s1 = sin(d2 * M_PI / 180.0);
                printf("Path A: pow=%.4f, sin=%.4f\n", p1, s1);
                
                /* Inline assembly clobbering multiple caller-saved registers */
                asm volatile (
                    "mov %0, %%eax\n\t"
                    "mov %1, %%ebx\n\t"
                    "imul %%ebx, %%eax\n\t"
                    "add %2, %%eax\n\t"
                    "mov %%eax, %0\n\t"
                    : "+r"(v1)
                    : "r"(v2), "r"(v3)
                    : "%eax", "%ebx", "%ecx", "%edx", "cc"
                );
                
                /* Call at the end of basic block before goto */
                if (inner == 1) {
                    goto call_block1;
                }
            } else {
                /* Path 2: Different function calls */
                float cf1 = cosf(f2);
                double cd1 = cos(d3);
                printf("Path B: cosf=%.4f, cos=%.4f\n", cf1, cd1);
                
                /* Another inline assembly with different clobbers */
                asm volatile (
                    "mov %0, %%eax\n\t"
                    "mov %1, %%ecx\n\t"
                    "add %%ecx, %%eax\n\t"
                    "mov %%eax, %0\n\t"
                    : "+r"(v4)
                    : "r"(v5)
                    : "%eax", "%ecx", "%edx", "cc"
                );
                
                /* Call external function */
                int r = rand();
                v5 = (v5 + r) % 1000;
            }
            
            /* Call recursive function with many live arguments */
            if (outer == 1 && inner == 0) {
                double rec_result = recursive_helper(
                    2,  /* depth */
                    d1, d2, f1, f2,
                    v1, v2, d3, f3,
                    v3, d4, f4, v4,
                    d5, f5
                );
                checksum += rec_result;
            }
            
            /* More arithmetic to keep variables live across calls */
            v1 = (v1 * 1103515245 + 12345) & 0x7fffffff;
            f1 = f1 * 1.01f + 0.5f;
            d1 = d1 * 1.01 + 0.5;
        }
        
        /* Function call with many arguments at loop end */
        printf("Loop %d: v1=%d v2=%d v3=%d f1=%.2f f2=%.2f d1=%.2f d2=%.2f\n",
               outer, v1, v2, v3, f1, f2, d1, d2);
    }
    
    /* Another goto to create basic block boundary */
    goto final_block;
    
    /* Unreachable block with function call at end */
    unreachable_block:
    printf("This should not be reached\n");
    return 1;
    
    final_block:
    /* Final computation using all variables */
    checksum += v1 + v2 + v3 + v4 + v5;
    checksum += f1 + f2 + f3 + f4 + f5;
    checksum += d1 + d2 + d3 + d4 + d5;
    
    /* One more function call before return */
    printf("Final checksum: %.10f\n", checksum);
    
    return 0;
}
