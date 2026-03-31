/* reload_stress_test.c
 * Designed to trigger secondary reload initialization in GCC's reload pass
 * Compile with: gcc -O1 -fno-omit-frame-pointer -g -c reload_stress_test.c
 * Or: gcc -O2 -finline-small-functions -fno-schedule-insns -fno-schedule-insns2 -c reload_stress_test.c
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex addressing modes */
volatile int global_var1 = 1234;
volatile int global_var2 = 5678;
volatile long long global_ll = 0x123456789ABCDEF0LL;

/* Bitfield structure to generate SUBREG RTL */
struct bitfield_struct {
    int a : 4;
    int b : 12;
    int c : 16;
} bitfield = {1, 2047, 32767};

/* Function using inline assembly with restrictive register constraints */
void test_secondary_reloads(void) {
    int input = 42;
    int output;
    
    /* Force secondary reload: memory -> specific register */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(output)          /* Output in any register */
        : "m"(global_var1)      /* Input from memory */
        : "%eax", "memory"      /* Clobber eax and memory */
    );
    
    /* Multiple alternative constraints */
    int x = 100, y = 200;
    asm volatile (
        "addl %1, %0"
        : "+r"(x)               /* Read-write operand in register */
        : "rm"(y)               /* Register or memory */
        : "cc"
    );
    
    /* Complex constraint with immediate */
    asm volatile (
        "imull %1, %0"
        : "+a"(x)               /* Must be in eax */
        : "rmi"(123)            /* Register, memory, or immediate */
        : "cc"
    );
}

/* Function using register variables bound to specific registers */
void test_register_binding(void) {
    /* Bind to specific registers */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    
    r1 = global_var1;
    r2 = global_var2;
    
    /* Force conflict: ebx-bound variable used in asm requiring different reg */
    asm volatile (
        "movl %%ecx, %%eax\n\t"
        "addl %%ebx, %%eax"
        : "=a"(r1)              /* Output in eax, but r1 is bound to ebx! */
        : "b"(r1), "c"(r2)      /* Inputs in ebx and ecx */
        : "cc"
    );
    
    /* This creates a reload situation */
    global_var1 = r1;
}

/* Function to generate SUBREG/STRICT_LOW_PART patterns */
void test_subreg_patterns(void) {
    /* Bitfield accesses generate SUBREG */
    int val = bitfield.a + bitfield.b * bitfield.c;
    
    /* Explicit truncation */
    int32_t src = 0x12345678;
    int16_t dst16 = (int16_t)src;  /* May generate SUBREG */
    int8_t dst8 = (int8_t)src;
    
    /* Use in arithmetic to force register allocation */
    volatile int16_t v16 = dst16;
    volatile int8_t v8 = dst8;
    
    /* Masking operation */
    int masked = src & 0xFFFF;  /* STRICT_LOW_PART pattern */
    
    /* Inline asm with partial register access */
    asm volatile (
        "movw %w1, %w0"         /* Use 16-bit register names */
        : "=r"(dst16)
        : "r"(src)
        : "cc"
    );
}

/* Function with complex memory addressing */
void test_complex_addressing(void) {
    volatile int array[100];
    static volatile int static_array[50];
    
    /* Various addressing modes */
    for (int i = 0; i < 10; i++) {
        /* Base + index addressing */
        array[i * 3 + 5] = global_var1 + i;
        
        /* Scaled index */
        static_array[i] = array[i * 2] * 3;
        
        /* Force spill/reload with volatile */
        asm volatile ("" : "+m"(array[i]), "+m"(static_array[i]));
    }
    
    /* Pointer arithmetic with different types */
    char *char_ptr = (char *)array;
    int *int_ptr = (int *)array;
    
    /* Mixed pointer access */
    for (int i = 0; i < 20; i++) {
        char_ptr[i] = int_ptr[i] & 0xFF;
    }
}

/* Function using floating point to trigger different register classes */
void test_float_reloads(void) {
    volatile double d1 = 3.14159;
    volatile double d2 = 2.71828;
    double result;
    
    /* FP register constraints (x86-specific) */
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "faddp\n\t"
        "fstpl %0"
        : "=m"(result)
        : "m"(d1), "m"(d2)
        : "st", "st(1)", "memory"
    );
    
    /* Mix FP and integer */
    int int_val = (int)result;
    asm volatile (
        "fildl %1\n\t"
        "fadd %%st(0), %%st(0)\n\t"
        "fistpl %0"
        : "=m"(global_var1)
        : "m"(int_val)
        : "st", "memory"
    );
}

/* Main function that calls all tests */
int main(void) {
    int sum = 0;
    
    printf("Starting reload stress tests...\n");
    
    /* Execute all test functions multiple times */
    for (int i = 0; i < 3; i++) {
        test_secondary_reloads();
        test_register_binding();
        test_subreg_patterns();
        test_complex_addressing();
        test_float_reloads();
        
        /* Accumulate results to prevent dead code elimination */
        sum += global_var1 + global_var2 + bitfield.a + bitfield.b;
    }
    
    printf("Tests completed. Sum: %d\n", sum);
    return sum != 0 ? 0 : 1;
}

/* Additional helper functions to create more reload contexts */
static inline int force_inline_reload(int x, int y) {
    int result;
    /* Inline function with asm and multiple constraints */
    asm volatile (
        "leal (%1, %2, 2), %0"
        : "=r"(result)
        : "r"(x), "r"(y)
    );
    return result;
}

void test_inline_function(void) {
    /* Call inline function in loop to create pressure */
    for (int i = 0; i < 100; i++) {
        global_var1 = force_inline_reload(global_var1, i);
    }
}

/* Unused function with complex constraints to be compiled but not called */
void __attribute__((noinline)) unused_complex_constraints(void) {
    register long long ll1 asm("rax");
    register long long ll2 asm("rdx");
    
    ll1 = global_ll;
    ll2 = global_ll >> 32;
    
    /* 64-bit operation with specific register constraints */
    asm volatile (
        "addq %1, %0\n\t"
        "rorq $32, %0"
        : "+r"(ll1)
        : "r"(ll2)
        : "cc"
    );
    
    global_ll = ll1;
}
