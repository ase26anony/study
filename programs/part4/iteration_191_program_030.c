/* reload_coverage.c
 * Designed to trigger push_reload with secondary reloads in GCC's reload pass
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variables to prevent optimization */
volatile int g_volatile_int = 12345;
volatile long long g_volatile_ll = 9876543210LL;
volatile float g_volatile_float = 3.14159f;
volatile double g_volatile_double = 2.718281828459045;

/* Function with complex inline assembly to trigger reloads */
static long long trigger_reloads(int a, long long b, float c, double d, 
                                 char* ptr, int arr_val) {
    long long result = 0;
    int out1, out2;
    long long out3;
    float out4;
    double out5;
    
    /* Array to force memory addressing modes */
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = arr_val + i;
    }
    
    /* ASM 1: Mixed integer types with specific register constraints
     * This forces reloads due to register class mismatches */
    asm volatile (
        "movl %[in1], %%eax\n\t"          /* Force use of eax */
        "addl %%eax, %[out1]\n\t"         /* Operation requiring reload */
        "movq %[in2], %%rbx\n\t"          /* Force use of rbx */
        "addq %%rbx, %[out3]\n\t"
        : [out1] "=r" (out1),             /* Output constraint */
          [out3] "=r" (out3)              /* Another output */
        : [in1] "rm" (a),                 /* Input: register or memory */
          [in2] "rm" (b),                 /* Can force secondary reload */
          "0" (g_volatile_int),           /* Matching constraint */
          "1" (g_volatile_ll)             /* Another matching constraint */
        : "eax", "rbx", "memory", "cc"    /* Clobbers to force reloads */
    );
    result += out1 + out3;
    
    /* ASM 2: Floating point with integer conversion
     * Forces mode changes and potential secondary reloads */
    asm volatile (
        "cvtsi2ssl %[int_in], %%xmm0\n\t"  /* Convert int to float */
        "addss %[float_in], %%xmm0\n\t"    /* Add floating point */
        "movss %%xmm0, %[float_out]\n\t"   /* Store result */
        "cvttss2sil %%xmm0, %[int_out]\n\t" /* Convert back to int */
        : [float_out] "=m" (out4),         /* Memory output constraint */
          [int_out] "=r" (out2)            /* Register output */
        : [int_in] "r" (a),                /* Register input */
          [float_in] "xm" (c)              /* SSE register or memory */
        : "xmm0", "cc"
    );
    result += out2 + (long long)out4;
    
    /* ASM 3: Complex addressing mode with displacement
     * Forces secondary reloads for address computation */
    asm volatile (
        "leaq (%[base], %[index], 4), %%rax\n\t"  /* Complex address */
        "movl (%%rax), %%ecx\n\t"                 /* Load from computed address */
        "addl %%ecx, %[out]\n\t"                  /* Use the value */
        : [out] "=r" (out1)
        : [base] "r" (arr),                       /* Base register */
          [index] "r" (a & 7),                    /* Index register */
          "m" (*(int(*)[10])arr)                  /* Memory input */
        : "rax", "ecx", "cc", "memory"
    );
    result += out1;
    
    /* ASM 4: Double precision with specific constraints
     * Forces different register classes */
    asm volatile (
        "movsd %[in1], %%xmm1\n\t"
        "addsd %[in2], %%xmm1\n\t"
        "movsd %%xmm1, %[out]\n\t"
        : [out] "=m" (out5)                      /* Memory output */
        : [in1] "x" (d),                         /* SSE register constraint */
          [in2] "xm" (g_volatile_double)         /* SSE or memory */
        : "xmm1", "cc"
    );
    result += (long long)out5;
    
    /* ASM 5: String operation with implicit length
     * Forces multiple input/output combinations */
    size_t len = strlen(ptr);
    asm volatile (
        "movq %[len], %%rcx\n\t"
        "testq %%rcx, %%rcx\n\t"
        "jz 1f\n\t"
        "0:\n\t"
        "movb (%[str]), %%al\n\t"
        "incb %[out]\n\t"
        "incq %[str]\n\t"
        "loop 0b\n\t"
        "1:\n\t"
        : [out] "+r" (out1),                     /* Read-write operand */
          [str] "+r" (ptr)                       /* Modified input */
        : [len] "r" (len)                        /* Input length */
        : "rax", "rcx", "cc", "memory"
    );
    result += out1;
    
    return result;
}

/* Main function with varied inputs */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <seed_value>\n", argv[0]);
        return 1;
    }
    
    /* Use command line argument as seed for variability */
    int seed = atoi(argv[1]);
    srand(seed);
    
    /* Create varied inputs to prevent constant propagation */
    int int_val = rand() % 1000 + 1;
    long long ll_val = (long long)rand() * rand();
    float float_val = (float)rand() / RAND_MAX * 100.0f;
    double double_val = (double)rand() / RAND_MAX * 200.0;
    
    /* Create a string with variable content */
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "TestString%d", rand() % 10000);
    
    /* Array value for memory operations */
    int arr_val = rand() % 100;
    
    /* Call the function that triggers reloads */
    long long result = trigger_reloads(int_val, ll_val, float_val, 
                                       double_val, buffer, arr_val);
    
    /* Use the result to prevent optimization */
    printf("Result: %lld\n", result);
    
    /* Additional volatile operations to ensure code isn't optimized away */
    g_volatile_int += result;
    g_volatile_ll ^= result;
    
    return (result > 0) ? 0 : 1;
}
