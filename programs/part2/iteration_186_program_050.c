/* mcf_coverage.c - Program to trigger MCF pass debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to keep all computations */
static volatile int global_seed = 42;

/* Large function with high register pressure and complex control flow */
static int __attribute__((noinline)) 
complex_mcf_function(int iterations, int mode) 
{
    /* Declare many local variables to create register pressure */
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    volatile int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    volatile int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    volatile float f0, f1, f2, f3, f4;
    volatile double d0, d1, d2;
    volatile void *p0, *p1, *p2, *p3;
    
    int result = 0;
    int i, j;
    
    /* Initialize variables with complex dependencies */
    v0 = global_seed + iterations;
    v1 = v0 * 3;
    v2 = v1 - 17;
    v3 = v2 / 2;
    v4 = v3 ^ 0xABCD;
    
    /* Create data flow dependencies */
    for (i = 0; i < iterations; i++) {
        /* Complex arithmetic chain */
        v5 = v4 + i;
        v6 = v5 * v3;
        v7 = v6 - v2;
        v8 = v7 ^ v1;
        v9 = v8 & 0xFF;
        
        /* Floating point operations to pressure FP registers */
        f0 = (float)v9 * 1.5f;
        f1 = f0 + (float)v8;
        f2 = f1 * 2.0f;
        f3 = f2 - 1.0f;
        f4 = f3 / 3.0f;
        
        /* Double precision */
        d0 = (double)f4;
        d1 = d0 * 1.6180339887;
        d2 = d1 + (double)i;
        
        /* Pointer arithmetic */
        p0 = (void*)((long)v9 * sizeof(int));
        p1 = p0 + v8;
        p2 = p1 - v7;
        p3 = p2;
        
        /* Inline assembly with register clobbering */
        /* Force register allocator to work around clobbered registers */
        asm volatile (
            "# Force register pressure\n"
            "mov %0, %0\n"
            :
            : "r" (v9)
            : "memory"
        );
        
        /* More complex control flow based on mode */
        switch (mode) {
            case 0:
                v10 = v9 + 1;
                v11 = v10 * 2;
                /* Fall through */
            case 1:
                v12 = v11 - 3;
                v13 = v12 | 0xF0;
                break;
            case 2:
                v14 = v9 << 2;
                v15 = v14 >> 1;
                v16 = v15 & 0x7F;
                break;
            case 3:
                v17 = v9 * v9;
                v18 = v17 % 256;
                v19 = v18 + 100;
                break;
            case 4:
                v20 = v9 ^ 0xAA;
                v21 = v20 + 55;
                v22 = v21 * 3;
                break;
            case 5:
                v23 = v9 / 2;
                v24 = v23 + 77;
                v25 = v24 - 33;
                break;
            case 6:
                v26 = v9 & 0x55;
                v27 = v26 << 1;
                v28 = v27 | 0x01;
                break;
            case 7:
                v29 = v9 + v8 + v7;
                result += v29;
                break;
            default:
                v10 = v9;
                v11 = v10;
                v12 = v11;
                break;
        }
        
        /* Nested conditionals creating many basic blocks */
        if (i % 3 == 0) {
            if (v9 > 100) {
                v13 = v9 - 50;
                if (v13 < 25) {
                    v14 = v13 * 4;
                    goto label_a;
                } else {
                    v14 = v13 / 2;
                }
            } else {
                v13 = v9 + 50;
            }
            v15 = v13 + v14;
        } else if (i % 3 == 1) {
            v16 = v9 * 3;
            if (v16 > 200) {
                v17 = v16 - 100;
            } else {
                v17 = v16 + 100;
            }
            v15 = v17;
        } else {
            v18 = v9 ^ 0xFF;
            v15 = v18;
        }
        
        /* Loop with break/continue creating exit edges */
        for (j = 0; j < 5; j++) {
            if (v15 + j > 250) {
                result += 10;
                break;  /* Creates exit edge */
            }
            if (v15 + j < 50) {
                result -= 5;
                continue;  /* Creates back edge */
            }
            result += v15 + j;
        }
        
        /* Update variables for next iteration */
        v4 = v15;
        v3 = v4 - i;
        v2 = v3 * 2;
        v1 = v2 ^ 0x1234;
        v0 = v1;
        
        /* Another inline assembly with specific register clobbering */
        /* For x86: */
        asm volatile (
            "# Clobber multiple registers\n"
            "movl %0, %%eax\n"
            "movl %1, %%ebx\n"
            "addl %%ebx, %%eax\n"
            "movl %%eax, %0\n"
            : "+r" (result)
            : "r" (v0)
            : "eax", "ebx", "memory"
        );
        
        /* For ARM (commented out, uncomment if targeting ARM):
        asm volatile (
            "# Clobber ARM registers\n"
            "mov r0, %0\n"
            "mov r1, %1\n"
            "add r0, r0, r1\n"
            "mov %0, r0\n"
            : "+r" (result)
            : "r" (v0)
            : "r0", "r1", "memory"
        );
        */
        
        label_a:
        /* Empty label for goto target */
        ;
    }
    
    /* Final complex computation */
    result = result + v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    
    /* Mix in floating point results */
    result += (int)f0 + (int)f1 + (int)f2 + (int)f3 + (int)f4;
    result += (int)d0 + (int)d1 + (int)d2;
    result += (int)(long)p0 + (int)(long)p1 + (int)(long)p2 + (int)(long)p3;
    
    return result;
}

/* Another complex function to ensure interprocedural analysis doesn't simplify too much */
static int __attribute__((noinline))
another_complex_function(int x, int y)
{
    volatile int a, b, c, d, e, f, g, h;
    
    a = x * y;
    b = a + 123;
    c = b - 456;
    d = c ^ 0x789;
    e = d << 2;
    f = e >> 1;
    g = f | 0xABC;
    h = g & 0xDEF;
    
    /* Complex conditional with many branches */
    if (x > y) {
        if (x > 1000) {
            a = x / y;
            goto merge_point;
        } else if (x > 500) {
            a = x % y;
        } else {
            a = x + y;
        }
    } else {
        if (y > 1000) {
            a = y / x;
        } else if (y > 500) {
            a = y % x;
        } else {
            a = y - x;
        }
    }
    
    merge_point:
    
    /* Use inline assembly */
    asm volatile (
        "# More register pressure\n"
        "mov %0, %0\n"
        : "+r" (a)
        :
        : "memory"
    );
    
    return a + b + c + d + e + f + g + h;
}

int main(void)
{
    int total = 0;
    int i;
    
    printf("Starting MCF coverage test...\n");
    
    /* Call with different parameters to prevent constant propagation */
    for (i = 0; i < 10; i++) {
        int mode = i % 8;
        int iterations = 5 + (i % 3);
        
        int result1 = complex_mcf_function(iterations, mode);
        int result2 = another_complex_function(i, iterations);
        
        total += result1 + result2;
        
        printf("Iteration %d: result1=%d, result2=%d, total=%d\n", 
               i, result1, result2, total);
    }
    
    printf("Final checksum: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total > 1000000) {
        printf("Unexpectedly large result\n");
    }
    
    return total != 0 ? 0 : 1;
}
