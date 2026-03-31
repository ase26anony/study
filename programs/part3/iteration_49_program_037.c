#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function that uses many arguments */
double recursive_helper(int depth, double a, double b, double c, float d, float e, 
                        int f, int g, int h, double i, float j, int k, double l, float m) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m;
    }
    
    /* Mix up the arguments to create register pressure */
    double temp1 = a * b - c;
    float temp2 = d + e * 2.0f;
    int temp3 = f ^ g | h;
    
    /* Function call within recursion */
    if (temp3 % 3 == 0) {
        printf("Recursive depth %d, temp3=%d\n", depth, temp3);
    }
    
    /* More arithmetic to extend live ranges */
    temp1 += sin(i) * cos(l);
    temp2 += j * m;
    
    /* Inline assembly that clobbers caller-saved registers */
    asm volatile (
        "mov %0, %%eax\n\t"
        "add %1, %%eax\n\t"
        "mov %%eax, %0"
        : "+r" (temp3)
        : "r" (depth)
        : "%eax", "%ecx", "%edx", "memory"
    );
    
    return recursive_helper(depth - 1, 
                           temp1, b * 1.1, c + temp1,
                           temp2, e * 0.9f,
                           temp3, g + 1, h ^ temp3,
                           i * 1.01, j * 1.1f, k * 2,
                           l * 0.99, m * 1.05f);
}

int main() {
    /* Declare and initialize 15 local variables of mixed types */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    
    srand(time(NULL));
    double checksum = 0.0;
    
    /* Loop to create register pressure */
    for (int outer = 0; outer < 3; outer++) {
        /* Block with arithmetic before calls */
        v1 = v2 * v3 + v4;
        f1 = f2 * f3 - f4;
        d1 = d2 / d3 + d4;
        
        /* First function call with mixed arguments */
        checksum += sin(d1) * cos(d2);
        
        /* More arithmetic to keep variables live */
        v2 = v1 ^ v5;
        f2 = f1 + f5 * 2.0f;
        d2 = d1 * 1.1 - d5;
        
        /* Conditional branch - both paths have calls */
        if (outer % 2 == 0) {
            /* Path 1: Call at end of basic block */
            printf("Even iteration: v1=%d, f1=%.2f, d1=%.2f\n", v1, f1, d1);
            /* This call is at block end before goto */
            goto process_block;
        } else {
            /* Path 2: Different call pattern */
            checksum += pow(d3, d4);
            /* Call at block end before label */
            v3 = rand() % 100;
            printf("Random v3: %d\n", v3);
        }
        
        /* Label to create basic block boundary */
        mid_block:
        /* Inline assembly clobbering caller-saved regs */
        asm volatile (
            "mov %0, %%eax\n\t"
            "imul %1, %%eax\n\t"
            "add %%eax, %2"
            : "+r" (v4), "+r" (v5)
            : "r" (checksum)
            : "%eax", "%ecx", "%edx", "%esi", "%edi", "memory"
        );
        
        /* Another function call */
        checksum += fabs(d3 - d4) * tan(d5);
        
        /* Recursive call with many live variables */
        checksum += recursive_helper(2, d1, d2, d3, f1, f2, 
                                    v1, v2, v3, d4, f3, v4, d5, f4);
        
        continue;
        
        /* Target of goto - creates unusual BB structure */
        process_block:
        /* Function call right after label */
        f3 = sinf(f4) * cosf(f5);
        /* This call might be at BB end if followed by condition */
        if (v5 > 10) {
            printf("v5 is large: %d\n", v5);
            /* Another call at conditional block end */
            checksum += log(d4);
            goto mid_block;
        }
        
        /* More arithmetic and calls */
        v5 = v4 * 3 - v2;
        d3 = sqrt(d2 * d2 + d1 * d1);
        
        /* External function call */
        checksum += pow(d4, 2.5) + pow(d5, 1.5);
        
        /* Another inline assembly block */
        asm volatile (
            "mov %0, %%eax\n\t"
            "add %1, %%eax\n\t"
            "mov %%eax, %0\n\t"
            "mov %2, %%ecx\n\t"
            "sub %%ecx, %3"
            : "+r" (v1), "+r" (v2)
            : "r" (v3), "r" (checksum)
            : "%eax", "%ecx", "%edx", "memory"
        );
    }
    
    /* Final computation using all variables */
    checksum += v1 + v2 + v3 + v4 + v5;
    checksum += f1 + f2 + f3 + f4 + f5;
    checksum += d1 + d2 + d3 + d4 + d5;
    
    /* One more function call at the end */
    printf("Final checksum: %.10f\n", checksum);
    
    /* Additional calls in return block */
    if (checksum > 1000.0) {
        checksum = sqrt(checksum);
        printf("Reduced checksum: %.10f\n", checksum);
    }
    
    return (int)(checksum * 1000) % 1000;
}
