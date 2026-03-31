/* Test program to trigger secondary reload initialization in GCC's reload pass */
#include <stdio.h>
#include <stdint.h>

/* Global variables to create complex addressing modes */
volatile int global_var1 = 42;
volatile int global_var2 = 100;
volatile long global_long = 999999L;

/* Bit-field structure to generate SUBREG RTL patterns */
struct bitfield_struct {
    unsigned int low16 : 16;
    unsigned int high16 : 16;
} bitfield = {12345, 54321};

/* Function using inline assembly with restrictive register constraints */
void test_restrictive_constraints(void) {
    int input, output;
    
    /* Force secondary reload: memory -> specific register */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(output)
        : "m"(global_var1)
        : "%eax", "memory"
    );
    
    /* Multiple alternative constraints with fixed register output */
    int temp = output + 1;
    asm volatile (
        "addl %1, %0"
        : "+a"(temp)          /* 'a' constraint for eax register */
        : "rm"(global_var2)   /* register or memory */
        : "cc"
    );
    
    /* Complex constraint with immediate value */
    asm volatile (
        "subl %1, %0"
        : "+r"(temp)
        : "i"(5)              /* immediate constraint */
        : "cc"
    );
}

/* Function using register variables bound to specific hardware registers */
void test_register_variables(void) {
    /* Bind variables to specific registers */
    register int reg1 asm("ebx");
    register int reg2 asm("ecx");
    register int reg3 asm("esi");
    
    /* Force conflicts between register-bound variables */
    reg1 = global_var1;
    reg2 = global_var2;
    
    /* Inline assembly that requires moving between fixed registers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0"
        : "+r"(reg1)
        : "r"(reg2)
        : "%eax", "cc"
    );
    
    /* Use all register variables in a complex expression */
    reg3 = reg1 + reg2;
    
    /* Memory clobber to prevent optimization */
    asm volatile ("" : : : "memory");
}

/* Function to generate SUBREG patterns via bit-field operations */
void test_subreg_patterns(void) {
    /* Access bit-fields - may generate SUBREG RTL */
    unsigned int low_part = bitfield.low16;
    unsigned int high_part = bitfield.high16;
    
    /* Explicit truncation operations */
    int32_t full_int = 0x12345678;
    int16_t truncated = (int16_t)full_int;  /* Likely generates SUBREG */
    
    /* Use truncated value in inline assembly */
    asm volatile (
        "movw %1, %%ax\n\t"
        "cwtl\n\t"
        "addl %%eax, %0"
        : "+r"(low_part)
        : "r"(truncated)
        : "%eax", "cc"
    );
    
    /* Masking operation for partial register access */
    int masked = full_int & 0xFFFF;
    
    /* Complex expression with mixed sizes */
    bitfield.low16 = (low_part + masked) & 0xFFFF;
}

/* Function with complex addressing modes and memory operands */
void test_complex_addressing(void) {
    int array[100];
    static int static_array[50];
    volatile int *volatile_ptr = &global_var1;
    
    /* Force memory addressing with displacement */
    for (int i = 0; i < 10; i++) {
        array[i] = i * 2;
    }
    
    /* Inline assembly with memory operand and index */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        asm volatile (
            "addl %1, %0"
            : "+r"(sum)
            : "m"(array[i])   /* Memory with index - complex addressing */
            : "cc"
        );
    }
    
    /* Pointer arithmetic with volatile */
    int val = *volatile_ptr;
    asm volatile (
        "imull %1, %0"
        : "+r"(val)
        : "r"(sum)
        : "cc"
    );
    
    /* Use static array with potential complex addressing */
    static_array[20] = val;
}

/* Function using STRICT_LOW_PART-like patterns */
void test_strict_low_part(void) {
    uint64_t big_val = 0x123456789ABCDEF0ULL;
    uint32_t low_part, high_part;
    
    /* Extract low and high parts */
    low_part = (uint32_t)big_val;
    high_part = (uint32_t)(big_val >> 32);
    
    /* Operations that might use partial registers */
    asm volatile (
        "addl %1, %0"
        : "+r"(low_part)
        : "r"(high_part)
        : "cc"
    );
    
    /* Recombine with shift */
    big_val = ((uint64_t)high_part << 32) | low_part;
    
    /* Use in another operation */
    asm volatile (
        ""
        : "+r"(big_val)
        :
        : "memory"
    );
}

/* Main function that combines all tests */
int main(void) {
    int result = 0;
    
    printf("Starting reload stress test...\n");
    
    /* Execute all test functions multiple times */
    for (int i = 0; i < 3; i++) {
        test_restrictive_constraints();
        test_register_variables();
        test_subreg_patterns();
        test_complex_addressing();
        test_strict_low_part();
        
        /* Accumulate some result to prevent dead code elimination */
        result += global_var1 + global_var2 + bitfield.low16;
    }
    
    printf("Test completed. Result: %d\n", result);
    
    /* Use result to prevent optimization */
    asm volatile ("" : : "r"(result) : "memory");
    
    return result != 0 ? 0 : 1;
}
