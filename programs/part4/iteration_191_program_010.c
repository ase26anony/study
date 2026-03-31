/* reload_test.c - Test program to trigger push_reload uncovered lines */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int g_int = 12345;
volatile long long g_llong = 9876543210LL;
volatile float g_float = 3.14159f;
volatile double g_double = 2.718281828459045;

/* Vector types for SIMD reloads */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function with complex inline assembly to trigger reloads */
static long long trigger_reloads(int a, long long b, float c, double d, 
                                 v4si vec_int, v4sf vec_float) {
    long long result = 0;
    int out1, out2;
    long long out3;
    float out4;
    double out5;
    v4si out_vec_int;
    v4sf out_vec_float;
    
    /* ASM 1: Mixed types with specific register constraints */
    /* This should trigger secondary reloads for floating point */
    asm volatile (
        /* Move int to output with constraint mismatch */
        "movl %1, %0\n\t"
        /* Use the value in computation */
        "addl $100, %0"
        : "=r" (out1)          /* Output in general reg */
        : "m" (g_int)          /* Input from memory - may need secondary reload */
        : "cc"                 /* Clobber flags */
    );
    result += out1;
    
    /* ASM 2: Long long with specific register pair constraint */
    /* x86_64 specific: forces use of specific registers */
    asm volatile (
        "movq %1, %%rax\n\t"   /* Force use of RAX */
        "movq %%rax, %0\n\t"
        "addq $200, %0"
        : "=r" (out3)
        : "m" (g_llong)        /* Memory operand needing reload */
        : "rax", "cc"          /* Clobber RAX and flags */
    );
    result += out3;
    
    /* ASM 3: Floating point with memory constraints */
    /* This often requires secondary reloads */
    asm volatile (
        "movss %1, %%xmm0\n\t"  /* Load float */
        "movss %%xmm0, %0\n\t"
        "addss %2, %0"          /* Add another float */
        : "=x" (out4)           /* Output in XMM register */
        : "m" (g_float),        /* Memory operand */
          "x" (c)               /* XMM register input */
        : "xmm0", "cc"
    );
    result += (long long)out4;
    
    /* ASM 4: Double with complex addressing */
    /* Force address computation reload */
    asm volatile (
        "movsd %1, %%xmm1\n\t"
        "movsd %%xmm1, %0\n\t"
        "addsd %2, %0"
        : "=x" (out5)
        : "m" (*(const double(*)[1])&g_double),  /* Complex address */
          "x" (d)
        : "xmm1", "cc"
    );
    result += (long long)out5;
    
    /* ASM 5: Vector types with specific constraints */
    /* Vector moves often need special handling */
    asm volatile (
        "movdqa %1, %0\n\t"
        "paddd %2, %0"          /* Vector add */
        : "=x" (out_vec_int)
        : "xm" (vec_int),       /* Memory or register */
          "x" (vec_int)         /* Register */
        : "cc"
    );
    
    /* ASM 6: Multiple outputs with different modes */
    /* This creates complex reload scenario */
    asm volatile (
        "mov %2, %0\n\t"        /* Move int */
        "mov %3, %1\n\t"        /* Move another int */
        "addl $42, %0\n\t"
        "subl $24, %1"
        : "=r" (out1), "=r" (out2)  /* Two outputs */
        : "r" (a), "i" (1000)       /* Register and immediate */
        : "cc"
    );
    result += out1 + out2;
    
    /* ASM 7: In/out operand with "+r" constraint */
    /* Creates interesting reload pattern */
    {
        int inout = a;
        asm volatile (
            "imull $3, %0\n\t"
            "addl $7, %0"
            : "+r" (inout)      /* Read-write operand */
            :                   /* No pure inputs */
            : "cc"
        );
        result += inout;
    }
    
    /* ASM 8: Force use of specific register class */
    /* x86 specific: force use of accumulator */
    {
        int tmp;
        asm volatile (
            "movl $999, %%eax\n\t"
            "addl %1, %%eax\n\t"
            "movl %%eax, %0"
            : "=r" (tmp)
            : "r" (a)
            : "eax", "cc"
        );
        result += tmp;
    }
    
    return result;
}

/* Helper to create vector values */
static v4si create_vec_int(int a, int b, int c, int d) {
    v4si vec = {a, b, c, d};
    return vec;
}

static v4sf create_vec_float(float a, float b, float c, float d) {
    v4sf vec = {a, b, c, d};
    return vec;
}

int main(int argc, char *argv[]) {
    /* Use argv to create variant values preventing constant propagation */
    int base = argc > 1 ? atoi(argv[1]) : 100;
    
    /* Initialize test values */
    int int_val = base + g_int;
    long long llong_val = (long long)base * 100 + g_llong;
    float float_val = (float)base / 10.0f + g_float;
    double double_val = (double)base / 3.0 + g_double;
    
    /* Create vector values */
    v4si vec_int = create_vec_int(base, base+1, base+2, base+3);
    v4sf vec_float = create_vec_float(float_val, float_val+1.0f, 
                                      float_val+2.0f, float_val+3.0f);
    
    /* Call the function that triggers reloads */
    long long result = trigger_reloads(int_val, llong_val, float_val, 
                                       double_val, vec_int, vec_float);
    
    /* Use the result to prevent optimization */
    printf("Result: %lld\n", result);
    
    /* Additional volatile operations to ensure code isn't optimized away */
    {
        volatile int check = 0;
        asm volatile ("" : "+r" (check));
    }
    
    return (result > 0) ? 0 : 1;
}
