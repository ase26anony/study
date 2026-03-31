/* reload_test.c - Stress GCC's reload pass to cover secondary reload initialization */
#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex addressing modes */
volatile int global_var = 42;
volatile int global_array[4] = {1, 2, 3, 4};

/* Bitfield structure to generate SUBREG RTL */
struct bitfield_struct {
    int full : 32;
    int partial : 16;
    int small : 8;
} bitfield;

/* Test 1: Force secondary reloads via restrictive register constraints */
void test_restrictive_constraints(void) {
    int input = global_var;
    int output;
    
    /* Force input from memory to specific register class */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(output)
        : "m"(input)
        : "%eax", "memory"
    );
    
    /* Multiple alternative constraints with fixed register output */
    register int reg_var asm("ebx") = output;
    int temp;
    
    asm volatile (
        "addl %2, %1\n\t"
        "movl %1, %0"
        : "=a"(temp), "+r"(reg_var)
        : "rm"(global_var)
        : "cc"
    );
    
    global_var = temp;
}

/* Test 2: Register-bound variables with conflicting requirements */
void test_register_conflicts(void) {
    /* Bind to specific registers */
    register int a asm("esi");
    register int b asm("edi");
    register int c asm("ebx");
    
    a = global_array[0];
    b = global_array[1];
    
    /* Force move between differently-bound registers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0"
        : "+r"(c)
        : "r"(a)
        : "%eax", "cc"
    );
    
    /* Complex addressing with register constraints */
    asm volatile (
        "imull %1, %0"
        : "+r"(b)
        : "rm"(c)
        : "%eax", "%edx", "cc"
    );
    
    global_array[2] = b;
}

/* Test 3: SUBREG/STRICT_LOW_PART patterns via bitfields and truncation */
void test_subreg_patterns(void) {
    /* Bitfield access generates SUBREG */
    bitfield.full = global_var;
    bitfield.partial = (int16_t)global_var;
    bitfield.small = (int8_t)global_var;
    
    /* Explicit truncation operations */
    int32_t large = 0x12345678;
    int16_t medium = (int16_t)large;
    int8_t small = (int8_t)large;
    
    /* Use truncated values in ways that may require partial register reloads */
    volatile int16_t *ptr = (int16_t*)&large;
    medium = *ptr;
    
    /* Inline asm with byte/word constraints */
    asm volatile (
        "movw %w1, %w0"
        : "=r"(medium)
        : "r"(large)
        : "memory"
    );
    
    global_array[3] = medium + small;
}

/* Test 4: Complex addressing modes with multiple memory operands */
void test_complex_addressing(void) {
    int index = global_var & 3;
    int result;
    
    /* Memory-to-memory with register constraint */
    asm volatile (
        "movl (%1, %2, 4), %%eax\n\t"
        "addl %%eax, %0"
        : "+m"(global_array[index])
        : "r"(global_array), "r"(index)
        : "%eax", "cc", "memory"
    );
    
    /* Multiple memory inputs with fixed output register */
    asm volatile (
        "movl %1, %%eax\n\t"
        "subl %2, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(result)
        : "m"(global_var), "m"(global_array[0])
        : "%eax", "cc"
    );
    
    global_var = result;
}

/* Test 5: Mixed volatile operations with optimization barriers */
void test_volatile_mix(void) {
    volatile int v1, v2, v3;
    
    /* Prevent optimization of reload sequences */
    asm volatile ("" : : : "memory");
    
    v1 = global_var;
    v2 = global_array[1];
    
    /* Complex volatile asm with multiple constraints */
    asm volatile (
        "movl %1, %%ecx\n\t"
        "leal (%%ecx, %2, 2), %%eax\n\t"
        "movl %%eax, %0"
        : "=rm"(v3)
        : "r"(v1), "r"(v2)
        : "%eax", "%ecx", "cc", "memory"
    );
    
    /* Force spill/reload with memory clobber */
    asm volatile (
        ""
        : "+r"(v3)
        :
        : "memory"
    );
    
    global_array[0] = v3;
}

/* Test 6: Nested inline asm with secondary output reloads */
void test_nested_reloads(void) {
    int in1 = global_var;
    int in2 = global_array[2];
    int out1, out2;
    
    /* First asm produces value for second asm */
    asm volatile (
        "movl %2, %%eax\n\t"
        "addl %3, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "movl %%eax, %1"
        : "=r"(out1), "=m"(out2)
        : "r"(in1), "rm"(in2)
        : "%eax", "cc", "memory"
    );
    
    /* Use both outputs with different constraints */
    asm volatile (
        "xorl %2, %0"
        : "+a"(out1)
        : "0"(out1), "r"(out2)
        : "cc"
    );
    
    global_var = out1;
}

/* Main function that runs all tests */
int main(void) {
    int i, sum = 0;
    
    /* Run tests multiple times to increase reload opportunities */
    for (i = 0; i < 10; i++) {
        test_restrictive_constraints();
        test_register_conflicts();
        test_subreg_patterns();
        test_complex_addressing();
        test_volatile_mix();
        test_nested_reloads();
        
        /* Accumulate results to prevent dead code elimination */
        sum += global_var + global_array[i & 3];
    }
    
    printf("Result: %d\n", sum);
    return sum & 0xFF; /* Return non-zero to ensure execution */
}
