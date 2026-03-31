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
        printf("Depth %d: t1=%.3f, t2=%.3f, t3=%d\n", depth, t1, t2, t3);
    }
    
    /* Recursive call with modified arguments */
    return recursive_helper(depth - 1, 
                           t1, b + 1.0, c * 0.9, 
                           t2, e * 1.1f,
                           t3, g + 2, h - 1,
                           t4, t5, k * 3,
                           l * 0.8, m * 1.2f) * 0.95;
}

int main() {
    /* Declare and initialize many local variables of mixed types */
    double a = 1.23456789;
    double b = 9.87654321;
    double c = 3.14159265;
    float d = 2.71828f;
    float e = 1.41421f;
    int f = 42;
    int g = 123;
    int h = 255;
    double i = 0.57721566;
    float j = 1.61803f;
    int k = 777;
    double l = 1.2020569;
    float m = 0.91596f;
    int n = 999;
    int o = 111;
    
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Label for goto to create unusual basic block structure */
    loop_start:
    
    /* Outer loop to create register pressure */
    for (int iter = 0; iter < 4; iter++) {
        /* Arithmetic operations on all variables before function calls */
        a = sin(a) + cos(b);
        b = pow(b, 1.001);
        c = c * 1.1 - 0.1;
        d = d * 1.2f + 0.3f;
        e = sqrtf(e * 2.0f);
        f = f * 3 + iter;
        g = g ^ (iter * 17);
        h = h | (iter << 4);
        i = log(i + 1.0);
        j = j * 0.9f + 0.1f;
        k = k % 100 + iter;
        l = exp(l * 0.99);
        m = tanf(m * 1.1f);
        n = n - iter * 11;
        o = o + iter * 7;
        
        /* Inline assembly that clobbers multiple caller-saved registers */
        /* Clobber eax, ecx, edx, xmm0, xmm1 */
        asm volatile (
            "mov %0, %%eax\n\t"
            "add %1, %%eax\n\t"
            "mov %%eax, %0\n\t"
            : "+r"(f)
            : "r"(g)
            : "%eax", "%ecx", "%edx"
        );
        
        /* Another inline assembly clobbering floating point registers */
        asm volatile (
            "addsd %1, %0\n\t"
            : "+x"(a)
            : "x"(b)
            : "%xmm0", "%xmm1", "%xmm2"
        );
        
        /* Conditional branch - both paths contain function calls */
        if (iter % 2 == 0) {
            /* Path 1: Call external functions with many live variables */
            printf("Iter %d: a=%.6f, b=%.6f, c=%.6f\n", iter, a, b, c);
            
            /* Call at end of basic block (just before goto) */
            if (iter == 2) {
                double result = sin(a) + cos(b) + tan(c);
                printf("Trig result: %.6f\n", result);
                goto special_path;  /* Creates block boundary with call at end */
            }
            
            /* More arithmetic to keep variables live across calls */
            d = d + sinf(j) * 0.5f;
            e = e * cosf(m);
            
            /* Call to math library function */
            double pow_result = pow(a, 2.0) + pow(b, 1.5);
            f = f + (int)pow_result;
        } else {
            /* Path 2: Different pattern of function calls */
            /* Call with different subset of arguments */
            printf("Iter %d: d=%.3f, e=%.3f, f=%d, g=%d\n", iter, d, e, f, g);
            
            /* Multiple consecutive function calls */
            double rand_val = (double)rand() / RAND_MAX;
            a = a + rand_val;
            
            /* Math function call */
            b = b * exp(rand_val * 0.1);
            
            /* Another printf call */
            printf("Random: %.3f, New b: %.6f\n", rand_val, b);
        }
        
        /* Call recursive helper function - creates deep call chain */
        double rec_result = recursive_helper(
            2,  /* depth */
            a * 0.5, b * 0.5, c * 0.5,
            d, e,
            f, g, h,
            i, j, k,
            l, m
        );
        
        /* Arithmetic after function call to keep variables live */
        c = c + rec_result * 0.01;
        h = h ^ ((int)rec_result);
        
        /* Another external function call */
        double mod_result = fmod(a, 2.5);
        i = i + mod_result;
        
        /* Inline assembly between function calls */
        asm volatile (
            "imul %1, %0\n\t"
            : "+r"(o)
            : "r"(n)
            : "%eax", "%edx"
        );
        
        /* Function call with many arguments - high register pressure */
        printf("Complex: a=%.3f,b=%.3f,c=%.3f,d=%.3f,e=%.3f,f=%d,g=%d\n",
               a, b, c, d, e, f, g);
        
        /* Continue loop */
        continue;
        
        /* Special path label for goto */
        special_path:
        /* This creates a basic block that starts after a goto */
        j = j * 1.5f;
        m = m / 1.3f;
        
        /* Function call at end of this special block */
        printf("Special path: j=%.3f, m=%.3f\n", j, m);
        
        /* Go back to loop */
        if (iter < 3) {
            goto loop_start;
        }
    }
    
    /* Final computation using all variables */
    double checksum = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o;
    
    /* Prevent dead code elimination */
    printf("Final checksum: %.12f\n", checksum);
    
    /* Another conditional with function call at block end */
    if (checksum > 10000.0) {
        printf("Large checksum!\n");
        return 0;  /* Return statement right after call */
    } else {
        printf("Small checksum!\n");
        /* Function call at end of basic block */
        double final_sin = sin(checksum);
        printf("Sine of checksum: %.6f\n", final_sin);
        return 1;
    }
}
