#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Recursive helper function that uses many arguments to increase register pressure */
double recursive_helper(int depth, double a, double b, double c, float d, float e, 
                        int f, int g, int h, double i, float j, int k, double l, float m) {
    if (depth <= 0) {
        return a + b + c + d + e + f + g + h + i + j + k + l + m;
    }
    
    /* Mix arithmetic operations to keep variables live */
    double t1 = a * b - c;
    float t2 = d / e + j;
    int t3 = f ^ g | h;
    double t4 = sin(i) * cos(l);
    float t5 = m * 2.0f - j;
    
    /* Function call within recursion to create more caller-save opportunities */
    if (depth % 2 == 0) {
        printf("Depth %d: t1=%.3f, t2=%.3f\n", depth, t1, t2);
    }
    
    /* Inline assembly that clobbers caller-saved registers */
    __asm__ volatile (
        "movq %0, %%rax\n"
        "addq %1, %%rax\n"
        "movq %%rax, %0"
        : "+r" (t1)
        : "r" (t4)
        : "%rax", "%rcx", "%rdx", "%rsi", "%rdi", "%r8", "%r9", "%r10", "%r11"
    );
    
    return recursive_helper(depth - 1, t1, b + 1.0, c - 0.5, t2, e * 1.1f,
                           t3, g + 1, h - 1, t4, t5, k * 2, l / 2.0, m + 0.3f);
}

int main() {
    /* Declare and initialize many variables of mixed types */
    double d1 = 1.0, d2 = 2.5, d3 = 3.14159, d4 = 4.2, d5 = 5.7;
    float f1 = 6.0f, f2 = 7.3f, f3 = 8.9f, f4 = 9.1f, f5 = 10.5f;
    int i1 = 11, i2 = 12, i3 = 13, i4 = 14, i5 = 15;
    
    srand(time(NULL));
    double checksum = 0.0;
    
    /* Label for goto to create unusual basic block structure */
    loop_start:
    
    /* Outer loop to create register pressure */
    for (int outer = 0; outer < 3; outer++) {
        /* Nested loop with arithmetic on all variables */
        for (int inner = 0; inner < 2; inner++) {
            /* Arithmetic before function call - variables must be kept live */
            d1 = d1 * 1.1 + sin(d2);
            d2 = d2 * 0.9 + cos(d3);
            d3 = d3 + tan(d4) * 0.1;
            d4 = pow(d4, 1.01);
            d5 = d5 / 1.5 + log(fabs(d1) + 1.0);
            
            f1 = f1 * 1.2f + (float)sin(d2);
            f2 = f2 / 1.3f + (float)cos(d3);
            f3 = f3 + (float)tan(d4) * 0.2f;
            f4 = powf(f4, 1.1f);
            f5 = f5 * 0.8f + (float)log(fabs(d5) + 1.0);
            
            i1 = i1 * 3 + rand() % 10;
            i2 = i2 ^ (i3 << 1);
            i3 = i3 | (i4 & 0xFF);
            i4 = i4 + i5 * 2;
            i5 = i5 - i1 / 2;
            
            /* Inline assembly that clobbers multiple caller-saved registers */
            __asm__ volatile (
                "movl %0, %%eax\n"
                "addl %1, %%eax\n"
                "movl %%eax, %0\n"
                "movq %2, %%rcx\n"
                "addq %3, %%rcx\n"
                "movq %%rcx, %2"
                : "+r" (i1), "+r" (d1)
                : "r" (i2), "r" (d2)
                : "%eax", "%ecx", "%edx", "%esi", "%edi", 
                  "%r8", "%r9", "%r10", "%r11", "cc", "memory"
            );
            
            /* Function call with many arguments - forces caller-save */
            checksum += recursive_helper(2, d1, d2, d3, f1, f2, 
                                        i1, i2, i3, d4, f3, i4, d5, f4);
            
            /* More arithmetic after function call - variables must be restored */
            d1 = d1 + checksum * 0.01;
            d2 = d2 - checksum * 0.005;
            f3 = f3 + (float)checksum * 0.02f;
            i4 = i4 ^ (int)checksum;
            
            /* Another function call at a different program point */
            if (inner == 0) {
                printf("Outer=%d, Inner=%d: d1=%.3f, f1=%.3f, i1=%d\n", 
                       outer, inner, d1, f1, i1);
            } else {
                /* Different external function call */
                double power_result = pow(d1, f1);
                printf("Power result: %.3f\n", power_result);
            }
            
            /* Conditional branch where one path has function call at block end */
            if (d1 > 10.0) {
                /* This call is at the end of a basic block (just before loop increment) */
                float trig_result = sinf(f2) * cosf(f3);
                printf("Trig: %.3f\n", trig_result);
                /* Fall through to loop increment */
            } else {
                /* Alternative path with different operations */
                d3 = sqrt(d3 * d4);
                /* Another function call */
                printf("Sqrt result: %.3f\n", d3);
                /* Use goto to create unusual control flow */
                if (outer == 1 && inner == 1) {
                    goto special_case;
                }
            }
        }
        
        /* Function call that might be at BB_END in some compilation scenarios */
        double rand_val = (double)rand() / RAND_MAX;
        printf("Random: %.3f\n", rand_val);
        /* This could be BB_END if followed by label */
        
        continue; /* This creates a basic block boundary */
        
        special_case:
        /* Unreachable code under normal flow, but creates BB structure */
        d5 = d5 * 2.0;
        printf("Special case activated\n");
        goto loop_start; /* Jump back to create more complex CFG */
    }
    
    /* Final computation using all variables */
    checksum = d1 + d2 + d3 + d4 + d5 + 
               f1 + f2 + f3 + f4 + f5 + 
               i1 + i2 + i3 + i4 + i5 + 
               checksum;
    
    /* One more function call before return */
    printf("Final checksum: %.6f\n", checksum);
    
    return (int)(checksum * 1000) % 1000;
}
