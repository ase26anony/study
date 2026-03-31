/* test_caller_save.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent inlining to ensure actual calls */
static __attribute__((noinline)) 
int helper_function(int x) {
    return x * 3 + 7;
}

/* Another non-inlineable function */
static __attribute__((noinline))
float float_helper(float x) {
    return x * 1.5f - 2.0f;
}

/* Vector type */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Main test function with caller-save pressure */
static __attribute__((noinline))
int test_caller_save(int iterations, int seed) {
    volatile int vol_var = seed;  /* Force memory operations */
    int result = 0;
    int i;
    
    /* Use many call-clobbered registers across multiple calls */
    for (i = 0; i < iterations; i++) {
        /* Integer computations in call-clobbered registers */
        int a = vol_var + i * 3;
        int b = a * 2 - 5;
        int c = b ^ (a << 2);
        
        /* Clobber integer registers - simulating a function call */
        /* This forces caller-save for rax, rbx, rcx, rdx on x86-64 */
        asm volatile (
            "# Clobber integer regs\n\t"
            "mov $0, %%rax\n\t"
            "mov $0, %%rbx\n\t"
            "mov $0, %%rcx\n\t"
            "mov $0, %%rdx\n\t"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* Use the values after clobber - forces save/restore */
        result += c + helper_function(a);
        
        /* Floating point computations */
        float f1 = (float)a * 1.25f;
        float f2 = (float)b * 0.75f;
        
        /* Clobber floating point/SSE registers */
        asm volatile (
            "# Clobber SSE regs\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            "pxor %%xmm2, %%xmm2\n\t"
            : /* no outputs */
            : /* no inputs */
            : "xmm0", "xmm1", "xmm2", "memory"
        );
        
        /* Use FP values after clobber */
        result += (int)(f1 + f2 + float_helper(f1));
        
        /* Vector computations */
        v4si v1 = {a, b, c, result};
        v4si v2 = {i, i*2, i*3, i*4};
        v4si v3 = v1 + v2;
        
        /* Clobber more vector registers */
        asm volatile (
            "# Clobber more vector regs\n\t"
            "pxor %%xmm3, %%xmm3\n\t"
            "pxor %%xmm4, %%xmm4\n\t"
            "pxor %%xmm5, %%xmm5\n\t"
            : /* no outputs */
            : /* no inputs */
            : "xmm3", "xmm4", "xmm5", "memory"
        );
        
        /* Use vector results */
        int sum = v3[0] + v3[1] + v3[2] + v3[3];
        result += sum + helper_function(sum % 100);
        
        /* Create control flow that might end basic block with call */
        if (result % 7 == 0) {
            /* This call might end a basic block */
            int temp = helper_function(result);
            /* Label to force edge after call */
            asm volatile ("# Label after call %0" : : "r"(temp));
            result ^= temp;
        } else {
            /* Alternative path with different clobber */
            asm volatile (
                "# Alternative clobber\n\t"
                "mov $0, %%r8\n\t"
                "mov $0, %%r9\n\t"
                "mov $0, %%r10\n\t"
                : /* no outputs */
                : /* no inputs */
                : "r8", "r9", "r10", "memory"
            );
            result += float_helper((float)result);
        }
        
        /* Update volatile to prevent optimization */
        vol_var = result % 1000;
    }
    
    return result;
}

/* Another function to create more caller-save contexts */
static __attribute__((noinline))
int complex_calculation(int x, int y) {
    volatile int vx = x;
    volatile int vy = y;
    
    /* Mixed computations */
    int a = vx * 3 - vy;
    float b = (float)a * 0.5f;
    
    /* Clobber registers around calls */
    asm volatile (
        "# Complex clobber set 1\n\t"
        "mov $0, %%rax\n\t"
        "pxor %%xmm0, %%xmm0\n\t"
        : /* no outputs */
        : /* no inputs */
        : "rax", "xmm0", "memory"
    );
    
    int c = helper_function(a);
    
    asm volatile (
        "# Complex clobber set 2\n\t"
        "mov $0, %%rbx\n\t"
        "pxor %%xmm1, %%xmm1\n\t"
        : /* no outputs */
        : /* no inputs */
        : "rbx", "xmm1", "memory"
    );
    
    float d = float_helper(b);
    
    /* Vector operation */
    v4sf v1 = {b, d, (float)c, (float)(a + c)};
    v4sf v2 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v3 = v1 * v2;
    
    /* Final clobber */
    asm volatile (
        "# Final complex clobber\n\t"
        "mov $0, %%rcx\n\t"
        "mov $0, %%rdx\n\t"
        "pxor %%xmm2, %%xmm2\n\t"
        "pxor %%xmm3, %%xmm3\n\t"
        : /* no outputs */
        : /* no inputs */
        : "rcx", "rdx", "xmm2", "xmm3", "memory"
    );
    
    return (int)(v3[0] + v3[1] + v3[2] + v3[3]) + c;
}

int main(int argc, char **argv) {
    int iterations = 100;
    int seed = 12345;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Testing caller-save optimization with %d iterations, seed=%d\n", 
           iterations, seed);
    
    /* Call test function multiple times with different arguments */
    int total = 0;
    total += test_caller_save(iterations, seed);
    total += test_caller_save(iterations / 2, seed * 2);
    total += test_caller_save(iterations / 3, seed * 3);
    
    /* Additional calls with complex calculations */
    total += complex_calculation(seed, iterations);
    total += complex_calculation(total, seed);
    
    printf("Final checksum: %d\n", total);
    printf("Checksum hex: 0x%08x\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total == 0x12345678) {
        printf("Impossible condition\n");
    }
    
    return total != 0 ? 0 : 1;
}
