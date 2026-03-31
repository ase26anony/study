/* Test program to trigger push_reload with secondary reloads in GCC reload pass */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int g_int = 12345;
volatile long long g_llong = 9876543210LL;
volatile float g_float = 3.14159f;
volatile double g_double = 2.718281828459045;
volatile int g_array[10] = {0,1,2,3,4,5,6,7,8,9};

/* Function with complex inline assembly to trigger reloads */
static long long trigger_reloads(int a, long long b, float c, double d, int *ptr) {
    long long accumulator = 0;
    int out1, out2;
    long long out3;
    float out4;
    double out5;
    
    /* ASM 1: Mixed integer types with specific register constraints */
    /* Forces reloads due to mismatched constraints */
    asm volatile (
        /* Move with potential secondary reload for constant */
        "movl %[con], %[out1]\n\t"
        /* Complex addressing mode that might need reload */
        "movl (%[ptr],%[idx],4), %[out2]"
        : [out1] "=r" (out1), [out2] "=r" (out2)
        : [con] "i" (0x12345678),  /* Immediate might need secondary reload */
          [ptr] "r" (ptr),          /* Base pointer */
          [idx] "r" (a & 7)         /* Scaled index */
        : "memory"
    );
    accumulator += out1 + out2;
    
    /* ASM 2: Floating point with integer conversion - forces mode changes */
    /* This creates different inmode/outmode scenarios */
    asm volatile (
        /* Convert float to int using specific instruction */
        "cvttss2si %[in1], %[out3]\n\t"
        /* Use the result in another operation */
        "addl %[in2], %[out3]"
        : [out3] "=r" (out3)
        : [in1] "x" (c),            /* SSE register constraint */
          [in2] "r" (a)             /* General register */
        : /* No clobbers for cvttss2si on x86 */
    );
    accumulator += out3;
    
    /* ASM 3: Multiple outputs with different classes */
    /* Forces multiple reload entries */
    asm volatile (
        /* Two independent moves */
        "mov %[in3], %[out4]\n\t"
        "mov %[in4], %[out5]"
        : [out4] "=r" (out4), [out5] "=r" (out5)
        : [in3] "r" (a),            /* Integer in float out - mode change */
          [in4] "r" (b)             /* Long long in double out */
        : /* No clobbers */
    );
    accumulator += (long long)out4 + (long long)out5;
    
    /* ASM 4: Complex constraints with earlyclobber */
    /* Forces non-combine reloads (nocombine = 1) */
    int temp;
    asm volatile (
        /* Operation that modifies input early */
        "lea (%[in5],%[in5],2), %[temp]\n\t"  /* temp = in5 * 3 */
        "imull %[in6], %[temp]\n\t"           /* temp *= in6 */
        "movl %[temp], %[out6]"               /* output result */
        : [out6] "=r" (out1), [temp] "=&r" (temp)
        : [in5] "0" (out1),   /* Matching constraint - same as out6 */
          [in6] "rm" (a)      /* Register or memory */
        : "cc"                /* Condition codes clobbered */
    );
    accumulator += out1;
    
    /* ASM 5: Vector-style operation (simulated with integers) */
    /* May trigger different register class requirements */
    struct two_ints { int a; int b; } vec_in = {a, a+1}, vec_out;
    asm volatile (
        /* Simulate vector operation */
        "movl %[vec_in_a], %[vec_out_a]\n\t"
        "movl %[vec_in_b], %[vec_out_b]\n\t"
        "addl $1, %[vec_out_a]\n\t"
        "addl $2, %[vec_out_b]"
        : [vec_out_a] "=r" (vec_out.a), [vec_out_b] "=r" (vec_out.b)
        : [vec_in_a] "r" (vec_in.a), [vec_in_b] "r" (vec_in.b)
        : /* No clobbers */
    );
    accumulator += vec_out.a + vec_out.b;
    
    /* ASM 6: Memory constraint with offset that needs computation */
    /* May trigger secondary reload for address calculation */
    int offset = a * sizeof(int);
    asm volatile (
        /* Load from memory with computed offset */
        "movl %[mem](,%[off],1), %[res]"
        : [res] "=r" (out2)
        : [mem] "m" (g_array[0]),  /* Memory constraint */
          [off] "r" (offset)       /* Offset in register */
        : "memory"
    );
    accumulator += out2;
    
    /* ASM 7: String operation with implicit registers */
    /* Forces specific register allocation */
    char str[16] = "test";
    asm volatile (
        /* String store operation */
        "movsb"
        : 
        : "S" (str),    /* Source in ESI */
          "D" (ptr),    /* Destination in EDI */
          "c" (4)       /* Count in ECX */
        : "memory"
    );
    
    return accumulator;
}

int main(int argc, char **argv) {
    /* Use argv to create variable values that can't be constant folded */
    int base = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Initialize variables with non-constant values */
    int int_val = base + g_int;
    long long llong_val = (long long)base * g_llong;
    float float_val = (float)base * g_float;
    double double_val = (double)base * g_double;
    int *ptr = &g_array[base % 10];
    
    /* Call the function that triggers reloads */
    long long result = trigger_reloads(int_val, llong_val, float_val, double_val, ptr);
    
    /* Use the result to prevent optimization */
    printf("Result: %lld\n", result);
    
    /* Additional test with different values */
    result += trigger_reloads(int_val + 1, llong_val - 1, 
                             float_val + 1.0f, double_val - 1.0, 
                             &g_array[(base + 1) % 10]);
    
    printf("Final result: %lld\n", result);
    
    return (int)(result & 0x7FFFFFFF);
}
