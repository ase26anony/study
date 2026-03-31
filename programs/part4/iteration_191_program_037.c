/* reload_coverage.c
 * Designed to trigger GCC's reload pass push_reload function
 * with secondary reload initialization.
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int g_volatile_int = 12345;
volatile long long g_volatile_ll = 0x123456789ABCDEFLL;
volatile float g_volatile_float = 3.14159f;
volatile double g_volatile_double = 2.718281828459045;

/* Vector types for additional reload complexity */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function that triggers multiple reload scenarios */
static long long trigger_reloads(int arg_int, long long arg_ll, 
                                 float arg_float, double arg_double,
                                 v4si arg_vec_int, v4sf arg_vec_float) {
    long long accumulator = 0;
    int out_int1, out_int2;
    long long out_ll1, out_ll2;
    float out_float;
    double out_double;
    v4si out_vec_int;
    v4sf out_vec_float;
    
    /* ASM 1: Mixed integer types with specific register constraints */
    /* This forces reloads due to register class mismatches */
    asm volatile (
        /* Move 64-bit value through 32-bit register pair */
        "movl %1, %k0\n\t"
        "shrl $32, %1\n\t"
        "movl %1, %k2"
        : "=&r" (out_int1), "+&r" (out_int2)
        : "r" (arg_ll), "m" (g_volatile_int)
        : "cc"
    );
    accumulator += out_int1 + out_int2;
    
    /* ASM 2: Floating point with integer constraints */
    /* Forces secondary reloads for floating point values */
    asm volatile (
        /* Simulate a conversion operation */
        "movd %1, %0\n\t"
        "psrld $16, %0"
        : "=&x" (out_vec_int)
        : "r" (arg_int), "m" (g_volatile_float)
        : 
    );
    
    /* ASM 3: Memory operand with complex addressing */
    /* May require secondary reload for address computation */
    asm volatile (
        "mov %1, %0\n\t"
        "add %2, %0"
        : "=&r" (out_ll1)
        : "m" (*(volatile long long*)&g_volatile_ll), 
          "ri" (arg_int)
        : "cc"
    );
    accumulator += out_ll1;
    
    /* ASM 4: Multiple outputs with different classes */
    /* Forces initialization of multiple reload entries */
    asm volatile (
        "mov %2, %0\n\t"
        "mov %3, %1"
        : "=r" (out_int2), "=&x" (out_float)
        : "rm" (arg_int), 
          "rm" (arg_float),
          "m" (g_volatile_double)
        : 
    );
    accumulator += out_int2;
    
    /* ASM 5: Vector operations with specific constraints */
    /* May trigger secondary reloads for vector registers */
    asm volatile (
        "movaps %1, %0\n\t"
        "paddd %2, %0"
        : "=x" (out_vec_int)
        : "x" (arg_vec_int),
          "xm" (*(v4si*)&g_volatile_int)
        : 
    );
    
    /* ASM 6: In/out operand with '+' constraint */
    /* Tests reload with input/output in same register */
    int inout = arg_int;
    asm volatile (
        "addl $0x7FFF, %0\n\t"
        "rorl $8, %0"
        : "+r" (inout)
        : 
        : "cc"
    );
    accumulator += inout;
    
    /* ASM 7: Large immediate that may not fit */
    /* Forces reload for constant pool */
    asm volatile (
        "mov %1, %0\n\t"
        "xor $0xFFFFFFFFFFFFFFF0, %0"
        : "=r" (out_ll2)
        : "r" (arg_ll)
        : "cc"
    );
    accumulator += out_ll2;
    
    /* ASM 8: Mixed mode operation */
    /* Tests inmode/outmode initialization */
    asm volatile (
        "cvtsi2sd %1, %0\n\t"
        "addsd %2, %0"
        : "=x" (out_double)
        : "r" (arg_int),
          "xm" (g_volatile_double)
        : 
    );
    
    return accumulator;
}

int main(int argc, char **argv) {
    /* Initialize with argument-derived values to prevent constant propagation */
    int base_int = argc * 1000 + 123;
    long long base_ll = (long long)argc * 1000000LL + 456789LL;
    float base_float = (float)argc * 1.234f + 5.678f;
    double base_double = (double)argc * 9.876 + 5.4321;
    
    /* Initialize vectors */
    v4si vec_int = { base_int, base_int + 1, base_int + 2, base_int + 3 };
    v4sf vec_float = { base_float, base_float + 1.0f, 
                       base_float + 2.0f, base_float + 3.0f };
    
    /* Call the function multiple times with different arguments */
    long long result = 0;
    for (int i = 0; i < 3; i++) {
        result += trigger_reloads(
            base_int + i * 100,
            base_ll + i * 1000LL,
            base_float + i * 10.0f,
            base_double + i * 20.0,
            vec_int,
            vec_float
        );
    }
    
    printf("Result: %lld\n", result);
    return 0;
}
