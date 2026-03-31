/* Test program to stress GCC's reload pass and trigger secondary reload initialization */
#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex addressing modes */
volatile int global_var1 = 1234;
volatile int global_var2 = 5678;
volatile long long global_ll = 0x123456789ABCDEF0LL;

/* Bitfield structure to generate SUBREG operations */
struct bitfield_struct {
    unsigned int a : 4;
    unsigned int b : 12;
    unsigned int c : 16;
} volatile bitfield = {1, 2048, 65535};

/* Function using inline assembly with restrictive register constraints */
void test_restrictive_registers(void) {
    int input = global_var1;
    int output;
    
    /* Force use of specific registers with memory operands */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (output)
        : "m" (input)
        : "%eax", "cc"
    );
    
    /* Multiple alternative constraints */
    asm volatile (
        "imull %1, %0"
        : "+r" (output)
        : "rm" (global_var2)
        : "cc"
    );
    
    global_var1 = output;
}

/* Function using register variables bound to specific registers */
void test_register_variables(void) {
    /* Bind to specific registers that might conflict */
    register int x asm("ebx");
    register int y asm("ecx");
    
    x = global_var1;
    y = global_var2;
    
    /* Force reload by requiring different register */
    asm volatile (
        "addl %%ebx, %%ecx\n\t"
        "movl %%ecx, %%eax"
        : : : "%eax", "%ebx", "%ecx", "cc"
    );
    
    /* Use the result */
    asm volatile (
        "movl %%eax, %0"
        : "=m" (global_var1)
        : : "%eax"
    );
}

/* Function to generate SUBREG operations via bitfields */
void test_bitfield_subreg(void) {
    /* Access bitfields - may generate SUBREG in RTL */
    unsigned int val = bitfield.b;
    
    /* Force truncation to smaller type */
    uint16_t truncated = (uint16_t)(val + bitfield.c);
    
    /* Use in inline assembly with register constraints */
    asm volatile (
        "addw %1, %0"
        : "+r" (truncated)
        : "rm" ((uint16_t)bitfield.a)
        : "cc"
    );
    
    /* Store back to global */
    asm volatile (
        "movw %0, %1"
        : : "r" (truncated), "m" (bitfield.a)
    );
}

/* Function with complex addressing modes and immediate values */
void test_complex_addressing(void) {
    int array[10] = {0};
    int index = global_var1 & 7;
    
    /* Complex addressing with multiple constraints */
    asm volatile (
        "movl (%1, %2, 4), %%eax\n\t"
        "addl $0x1234, %%eax\n\t"
        "movl %%eax, %0"
        : "=rm" (array[index])
        : "r" (array), "r" (index)
        : "%eax", "cc", "memory"
    );
    
    /* Immediate value with restrictive output */
    asm volatile (
        "movl $0xDEADBEEF, %0"
        : "=a" (global_var2)
        : : "cc"
    );
}

/* Function mixing 32-bit and 64-bit operations */
void test_mixed_width(void) {
    long long ll_val = global_ll;
    int int_val;
    
    /* Access lower 32 bits - may generate SUBREG */
    int_val = (int)ll_val;
    
    /* Operations requiring different register classes */
    asm volatile (
        "addl %1, %0\n\t"
        "cltd\n\t"
        "idivl %2"
        : "+a" (int_val)
        : "rm" (global_var1), "rm" (global_var2)
        : "%edx", "cc"
    );
    
    /* 64-bit operation */
    asm volatile (
        "imulq $100, %1, %0"
        : "=r" (ll_val)
        : "0" (ll_val)
        : "cc"
    );
    
    global_ll = ll_val;
}

/* Function with multiple output operands */
void test_multiple_outputs(void) {
    int out1, out2;
    int in1 = global_var1;
    int in2 = global_var2;
    
    /* Multiple outputs with restrictive constraints */
    asm volatile (
        "movl %2, %%eax\n\t"
        "movl %3, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "subl $10, %%ebx\n\t"
        "movl %%eax, %0\n\t"
        "movl %%ebx, %1"
        : "=a" (out1), "=b" (out2)
        : "rm" (in1), "rm" (in2)
        : "cc"
    );
    
    global_var1 = out1;
    global_var2 = out2;
}

/* Function using volatile to prevent optimization */
void test_volatile_operations(void) {
    volatile int v1 = global_var1;
    volatile int v2 = global_var2;
    
    /* Memory barrier */
    asm volatile ("" : : : "memory");
    
    /* Complex operation with volatile operands */
    asm volatile (
        "movl %1, %%eax\n\t"
        "imull %2, %%eax\n\t"
        "movl %%eax, %0"
        : "=rm" (v1)
        : "rm" (v1), "rm" (v2)
        : "%eax", "cc", "memory"
    );
    
    /* Force write-back */
    asm volatile ("" : : "r" (v1), "r" (v2) : "memory");
    
    global_var1 = v1;
}

/* Main function that calls all tests */
int main(void) {
    int i;
    
    printf("Starting reload stress test...\n");
    
    /* Execute tests multiple times to increase chances */
    for (i = 0; i < 10; i++) {
        test_restrictive_registers();
        test_register_variables();
        test_bitfield_subreg();
        test_complex_addressing();
        test_mixed_width();
        test_multiple_outputs();
        test_volatile_operations();
        
        /* Modify globals to create data dependencies */
        global_var1 ^= i;
        global_var2 += i;
        global_ll >>= 1;
    }
    
    /* Compute a result to prevent dead code elimination */
    int result = global_var1 + global_var2 + (int)global_ll + bitfield.b;
    
    printf("Result: %d\n", result);
    return result & 0xFF;
}
