/* reload_test.c - Test program to trigger reload.cc push_reload uncovered lines */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile int g_int = 12345;
volatile long long g_llong = 9876543210LL;
volatile float g_float = 3.14159f;
volatile double g_double = 2.718281828459045;
volatile int g_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

/* Function with multiple inline asm statements to trigger reloads */
static int trigger_reloads(int a, long long b, float c, double d, int *ptr)
{
    int out1, out2, out3, out4;
    long long out_ll;
    float out_f;
    double out_d;
    int accumulator = 0;
    
    /* ASM 1: Mixed types with specific register constraints
       This forces reloads due to register class mismatches */
    asm volatile (
        /* Move int to output with 'r' constraint - may need secondary reload */
        "movl %1, %0\n\t"
        : "=r" (out1)          /* Output in general register */
        : "irm" (a)            /* Input: immediate, register, or memory */
        : /* no clobbers */
    );
    accumulator += out1;
    
    /* ASM 2: Floating point to integer with memory constraint
       Likely needs secondary reload for address computation */
    asm volatile (
        /* Convert float to int using memory operand */
        "movd %1, %0\n\t"
        : "=r" (out2)          /* Output in general register */
        : "m" (c)              /* Input from memory - address may need reload */
        : /* no clobbers */
    );
    accumulator += out2;
    
    /* ASM 3: Complex addressing mode with multiple constraints
       Forces reload due to addressing mode requirements */
    asm volatile (
        /* Load from array with index */
        "movl (%1, %2, 4), %0\n\t"
        : "=r" (out3)          /* Output */
        : "r" (g_array),       /* Base address - may need reload */
          "r" (a & 7)          /* Scaled index - may need reload */
        : "memory"
    );
    accumulator += out3;
    
    /* ASM 4: 64-bit operation with specific register pair
       May require secondary reload for 64-bit constant */
    asm volatile (
        /* 64-bit add with constant */
        "addq %2, %1\n\t"
        "movq %1, %0\n\t"
        : "=r" (out_ll)        /* 64-bit output */
        : "0" (b),             /* Input/output in same register */
          "irm" (0x1234567890LL) /* Large constant - may need reload */
        : "cc"
    );
    accumulator += (int)out_ll;
    
    /* ASM 5: Floating point operation with memory output
       Forces reload for memory address */
    asm volatile (
        /* Store double to memory */
        "movsd %1, %0\n\t"
        : "=m" (out_d)         /* Memory output - address needs reload */
        : "x" (d)              /* XMM register input */
        : "memory"
    );
    accumulator += (int)out_d;
    
    /* ASM 6: Multiple outputs with different classes
       Creates complex reload scenario */
    asm volatile (
        /* Two different operations */
        "movl %2, %0\n\t"
        "movd %3, %1\n\t"
        : "=r" (out4),         /* General purpose reg output */
          "=x" (out_f)         /* XMM register output */
        : "r" (g_int),         /* Input from global */
          "m" (g_float)        /* Input from memory */
        : /* no clobbers */
    );
    accumulator += out4 + (int)out_f;
    
    /* ASM 7: Inline asm with immediate constraint that may not fit
       Forces constant reload */
    asm volatile (
        "imull %1, %0\n\t"
        : "+r" (accumulator)   /* Read-write operand */
        : "i" (0x00FF00FF)     /* Immediate - may need reload if doesn't fit */
        : "cc"
    );
    
    /* ASM 8: String operation with specific registers
       Forces fixed register allocation */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %2, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (out1)
        : "r" (a),
          "m" (g_array[4])     /* Memory operand */
        : "eax", "ebx", "cc"   /* Clobbers specific registers */
    );
    accumulator += out1;
    
    return accumulator;
}

/* Secondary function with vector-type operations (if supported) */
#ifdef __SSE2__
#include <emmintrin.h>
static int trigger_vector_reloads(__m128i v1, __m128i v2)
{
    __m128i out_v;
    int out_arr[4];
    
    /* Vector operation with memory output
       May require complex address reload */
    asm volatile (
        "paddq %1, %0\n\t"
        "movdqa %0, %2\n\t"
        : "+x" (v1)            /* Read-write XMM register */
        : "xm" (v2),           /* XMM register or memory */
          "m" (out_v)          /* Memory output */
        : "memory"
    );
    
    /* Extract to general purpose registers */
    asm volatile (
        "movd %1, %0\n\t"
        "pextrd $1, %1, %2\n\t"
        "pextrd $2, %1, %3\n\t"
        "pextrd $3, %1, %4\n\t"
        : "=r" (out_arr[0]),
          "=r" (out_arr[1]),
          "=r" (out_arr[2]),
          "=r" (out_arr[3])
        : "x" (v1)             /* XMM register input */
        : /* no clobbers */
    );
    
    return out_arr[0] + out_arr[1] + out_arr[2] + out_arr[3];
}
#endif

int main(int argc, char **argv)
{
    int result = 0;
    
    /* Use argv to create variant inputs preventing constant propagation */
    int base = (argc > 1) ? atoi(argv[1]) : 1000;
    
    /* Initialize test variables with non-constant values */
    int int_val = base + g_int;
    long long llong_val = (long long)base * g_llong;
    float float_val = (float)base * g_float;
    double double_val = (double)base * g_double;
    
    /* Trigger reloads with various operand types */
    result = trigger_reloads(int_val, llong_val, float_val, double_val, g_array);
    
    #ifdef __SSE2__
    /* Add vector reload tests if SSE2 available */
    __m128i vec1 = _mm_set_epi32(base+3, base+2, base+1, base);
    __m128i vec2 = _mm_set_epi32(base+7, base+6, base+5, base+4);
    result += trigger_vector_reloads(vec1, vec2);
    #endif
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d (0x%08x)\n", result, result);
    
    /* Additional test: loop with varying asm to hit different reload paths */
    for (int i = 0; i < 10; i++) {
        int temp;
        /* This asm uses 'g' constraint (general) which can be register, memory, or immediate */
        asm volatile (
            "leal (%1, %1, 2), %0\n\t"  /* 3*i */
            : "=r" (temp)
            : "g" (i)                   /* General constraint - may need different reloads */
            : /* no clobbers */
        );
        result += temp;
    }
    
    printf("Final result: %d\n", result);
    return result != 0;
}
