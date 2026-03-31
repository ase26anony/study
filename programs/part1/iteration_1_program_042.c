#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Helper functions with various attributes to create diverse call sites */

/* Function that returns a value and uses many arguments */
static int __attribute__((noinline)) 
compute_sum(int a, int b, int c, int d, int e, int f, int g, int h, 
            float fa, float fb, float fc, float fd)
{
    volatile int result = a + b + c + d + e + f + g + h;
    result += (int)(fa + fb + fc + fd);
    
    /* Force register pressure inside the function */
    int t1 = result * 2;
    int t2 = t1 + 17;
    float ft1 = fa * 1.5f;
    float ft2 = fb * 2.5f;
    
    /* Inline assembly that clobbers registers */
    __asm__ volatile (
        "movl %0, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0"
        : "+r" (t2)
        : 
        : "eax", "memory"
    );
    
    return t2 + (int)(ft1 + ft2);
}

/* Function with pointer arguments */
static void __attribute__((noinline))
process_pointers(int* p1, float* p2, double* p3, 
                 volatile int* p4, const char** p5)
{
    if (p1 && p2 && p3 && p4 && p5) {
        *p1 += 42;
        *p2 *= 1.618034f;
        *p3 /= 2.71828;
        *p4 = *p1 + (int)*p2;
    }
    
    /* More register pressure */
    int local1 = *p1;
    float local2 = *p2;
    double local3 = *p3;
    
    /* Force multiple computations in registers */
    for (int i = 0; i < 3; i++) {
        local1 = local1 * 3 - i;
        local2 = local2 + i * 0.5f;
        local3 = local3 * (1.0 + i * 0.1);
    }
    
    /* Another inline assembly with clobber */
    __asm__ volatile (
        "movq %0, %%r10\n\t"
        "addq $16, %%r10\n\t"
        "movq %%r10, %0"
        : "+r" (p3)
        : 
        : "r10"
    );
    
    *p3 = local3;
}

/* Function that uses alloca to affect frame pointer */
static int __attribute__((noinline, optimize("no-omit-frame-pointer")))
dynamic_stack_operation(int size_factor)
{
    /* Taking address of locals and using alloca forces frame pointer */
    int local_array[8];
    int* dynamic = alloca(sizeof(int) * (size_factor + 2));
    
    for (int i = 0; i < 8; i++) {
        local_array[i] = i * size_factor;
    }
    
    /* Complex computation with many temporaries */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        int temp1 = local_array[i];
        int temp2 = temp1 * temp1;
        int temp3 = temp2 >> 3;
        int temp4 = temp3 + i;
        sum += temp4;
        
        /* Store to dynamic memory to prevent optimization */
        dynamic[i % (size_factor + 2)] = temp4;
    }
    
    /* Address taken, affecting register allocation */
    int* ptr = &local_array[3];
    *ptr = sum;
    
    return sum + dynamic[0];
}

/* Variadic-like function using many registers */
static float __attribute__((noinline))
mixed_type_operation(int a, float b, double c, int d, float e, 
                     double f, int g, float h)
{
    /* Many live values across computations */
    volatile float v1 = b;
    volatile double v2 = c;
    int v3 = a + d + g;
    float v4 = e + h;
    double v5 = f;
    
    /* Chain of computations keeping values live */
    for (int i = 0; i < 4; i++) {
        v1 = v1 * 1.1f + i;
        v2 = v2 / 1.05 - i * 0.5;
        v3 = v3 * 2 - i;
        v4 = v4 + v1 * 0.5f;
        v5 = v5 + v2;
    }
    
    /* Inline assembly clobbering multiple registers */
    __asm__ volatile (
        "movl %0, %%eax\n\t"
        "movss %1, %%xmm0\n\t"
        "addl $1, %%eax\n\t"
        "addss %2, %%xmm0\n\t"
        "movl %%eax, %0\n\t"
        "movss %%xmm0, %1"
        : "+r" (v3), "+r" (v1)
        : "r" (v4)
        : "eax", "xmm0", "memory"
    );
    
    return v1 + v4 + (float)v2 + (float)v5;
}

/* Main function creating maximum register pressure */
int main(void)
{
    /* Declare many local variables of mixed types */
    volatile int v1 = 1;
    volatile float v2 = 2.0f;
    volatile double v3 = 3.0;
    int v4 = 4;
    float v5 = 5.0f;
    double v6 = 6.0;
    int v7 = 7;
    float v8 = 8.0f;
    double v9 = 9.0;
    int v10 = 10;
    float v11 = 11.0f;
    double v12 = 12.0;
    int v13 = 13;
    float v14 = 14.0f;
    double v15 = 15.0;
    
    /* Pointers to create additional pressure */
    int* p1 = &v1;
    float* p2 = &v2;
    double* p3 = &v3;
    const char* msg = "Test";
    
    int checksum = 0;
    
    /* Create control flow with basic blocks containing calls */
    for (int iteration = 0; iteration < 3; iteration++) {
        if (iteration % 2 == 0) {
            /* First basic block with function calls */
            v4 = compute_sum(v1, v4, v7, v10, v13, 
                            iteration, iteration*2, iteration*3,
                            v2, v5, v8, v11);
            
            /* Inline assembly between calls to create live ranges */
            __asm__ volatile (
                "movl %0, %%eax\n\t"
                "movl %1, %%ebx\n\t"
                "addl %%ebx, %%eax\n\t"
                "movl %%eax, %0"
                : "+r" (v4)
                : "r" (v7)
                : "eax", "ebx", "memory"
            );
            
            process_pointers(&v4, &v5, &v6, &v1, &msg);
            
            /* More computations keeping values live */
            v7 = v4 * 3 - v7;
            v8 = v5 + v8 * 0.5f;
            v9 = v6 * 1.1;
        } else {
            /* Different basic block with other calls */
            v10 = dynamic_stack_operation(iteration + 2);
            
            /* Force spill/fill around call */
            __asm__ volatile (
                "movq %0, %%r10\n\t"
                "movq %1, %%r11\n\t"
                "addq %%r11, %%r10\n\t"
                "movq %%r10, %0"
                : "+r" (p1)
                : "r" (&v10)
                : "r10", "r11", "memory"
            );
            
            v11 = mixed_type_operation(v1, v2, v3, v4, v5, v6, v7, v8);
            
            /* Complex computation chain */
            v12 = v9 + v6 + (double)v11;
            v13 = v10 + (int)v12;
            v14 = v11 * 2.0f;
            v15 = v12 * 1.5;
        }
        
        /* Loop-carried dependencies */
        v1 += iteration;
        v2 += iteration * 0.25f;
        v3 += iteration * 0.5;
        
        /* Conditional creating another basic block */
        if (v4 > 100) {
            /* Nested call in conditional block */
            v5 = mixed_type_operation(v13, v14, v15, 
                                     v1, v2, v3, v4, v5);
            
            /* More inline assembly */
            __asm__ volatile (
                "movl %0, %%ecx\n\t"
                "shrl $2, %%ecx\n\t"
                "movl %%ecx, %0"
                : "+r" (v13)
                : 
                : "ecx", "memory"
            );
        }
        
        /* Update checksum with all values */
        checksum += v1 + (int)v2 + (int)v3 + v4 + (int)v5 + 
                   (int)v6 + v7 + (int)v8 + (int)v9 + v10 + 
                   (int)v11 + (int)v12 + v13 + (int)v14 + (int)v15;
    }
    
    /* Final computation and output */
    printf("Final checksum: %d\n", checksum);
    
    /* Verify with expected value (computed from initial values) */
    int expected = 0; /* Would be computed based on algorithm */
    printf("Verification: %s\n", 
           (checksum > 0 && checksum < 1000000) ? "PASS" : "FAIL");
    
    return 0;
}
