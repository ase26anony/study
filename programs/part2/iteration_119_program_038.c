/* reload_test.c - Test program to trigger secondary reload initialization in GCC's reload pass */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex addressing modes */
volatile int global_var1 = 42;
volatile int global_var2 = 100;
volatile long long global_ll = 0x123456789ABCDEF0LL;

/* Bitfield structure to generate SUBREG RTL */
struct bitfield_struct {
    unsigned int a : 4;
    unsigned int b : 12;
    unsigned int c : 16;
} volatile bitfield = {1, 2048, 32768};

/* Test 1: Force secondary reloads via fixed register constraints with memory operands */
void test_fixed_reg_with_memory() {
    int result;
    int input = 777;
    
    /* Force eax register constraint with memory operand */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0"
        : "=rm" (result)
        : "m" (input)
        : "%eax", "cc"
    );
    
    /* Multiple alternative constraints with fixed register output */
    asm volatile (
        "imull %1, %0"
        : "+a" (result)
        : "rm" (global_var1)
        : "cc"
    );
}

/* Test 2: Register-bound variables with conflicting constraints */
void test_register_binding_conflict() {
    /* Bind variables to specific registers */
    register int reg1 asm("ebx");
    register int reg2 asm("ecx");
    
    reg1 = global_var1;
    reg2 = global_var2;
    
    /* Force move between fixed registers through memory */
    asm volatile (
        "movl %%ebx, %0\n\t"
        "movl %0, %%ecx"
        : "=m" (global_var1)
        : 
        : "memory"
    );
    
    /* Complex operation requiring reloads */
    asm volatile (
        "addl %%ecx, %%ebx\n\t"
        "movl %%ebx, %%eax"
        : 
        : 
        : "%eax", "%ebx", "%ecx", "cc"
    );
}

/* Test 3: SUBREG generation via bitfield operations */
void test_subreg_generation() {
    /* Access bitfields - generates SUBREG RTL */
    unsigned int val = bitfield.b;
    unsigned int val2 = bitfield.c;
    
    /* Force register constraint on SUBREG result */
    asm volatile (
        "addl %1, %0"
        : "+r" (val)
        : "r" (val2)
        : "cc"
    );
    
    bitfield.a = (unsigned int)val & 0xF;
}

/* Test 4: STRICT_LOW_PART via truncation operations */
void test_strict_low_part() {
    int32_t large_val = 0x12345678;
    
    /* Explicit truncation to 16-bit */
    int16_t truncated = (int16_t)large_val;
    
    /* Use truncated value in operation requiring full register */
    asm volatile (
        "addw $100, %0"
        : "+r" (truncated)
        : 
        : "cc"
    );
    
    /* Combine with memory operand */
    asm volatile (
        "movw %1, %%ax\n\t"
        "addw %%ax, %0"
        : "+m" (global_var1)
        : "r" (truncated)
        : "%ax", "cc"
    );
}

/* Test 5: Complex addressing modes with multiple constraints */
void test_complex_addressing() {
    int array[10] = {0};
    int index = 3;
    int value = 999;
    
    /* Complex constraint with memory, register, and immediate alternatives */
    asm volatile (
        "movl %1, (%2, %3, 4)"
        : 
        : "r" (value), "r" (array), "r" (index)
        : "memory"
    );
    
    /* Force secondary reload with immediate operand */
    asm volatile (
        "movl $0xABCD, %%eax\n\t"
        "addl %%eax, %0"
        : "+m" (array[0])
        : 
        : "%eax", "cc", "memory"
    );
}

/* Test 6: Mixed size operations requiring reloads */
void test_mixed_sizes() {
    long long ll_val = global_ll;
    int int_val;
    
    /* 64-bit to 32-bit truncation with register constraint */
    asm volatile (
        "movq %1, %%rax\n\t"
        "movl %%eax, %0"
        : "=r" (int_val)
        : "m" (ll_val)
        : "%rax"
    );
    
    /* Different size constraints */
    asm volatile (
        "addl %1, %0"
        : "+r" (int_val)
        : "rm" (global_var1)
        : "cc"
    );
}

/* Test 7: Multiple output operands with conflicting constraints */
void test_multiple_outputs() {
    int out1, out2;
    int in1 = 100, in2 = 200;
    
    /* Two outputs with different register classes */
    asm volatile (
        "movl %2, %%eax\n\t"
        "movl %3, %%ebx\n\t"
        "addl $10, %%eax\n\t"
        "subl $5, %%ebx\n\t"
        "movl %%eax, %0\n\t"
        "movl %%ebx, %1"
        : "=r" (out1), "=r" (out2)
        : "rm" (in1), "rm" (in2)
        : "%eax", "%ebx", "cc"
    );
}

/* Test 8: Volatile memory barriers creating reload pressure */
void test_volatile_barriers() {
    volatile int barrier1 = 0;
    volatile int barrier2 = 0;
    
    /* Memory clobber forcing reloads */
    asm volatile ("" : : : "memory");
    
    /* Use volatile in constrained asm */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0"
        : "+m" (barrier1)
        : "r" (barrier2)
        : "%eax", "cc", "memory"
    );
    
    /* Multiple volatile accesses */
    asm volatile (
        ""
        : "+m" (barrier1), "+m" (barrier2)
        : 
        : "memory"
    );
}

/* Main function that runs all tests */
int main() {
    int i, sum = 0;
    
    /* Run tests multiple times to increase reload opportunities */
    for (i = 0; i < 10; i++) {
        test_fixed_reg_with_memory();
        test_register_binding_conflict();
        test_subreg_generation();
        test_strict_low_part();
        test_complex_addressing();
        test_mixed_sizes();
        test_multiple_outputs();
        test_volatile_barriers();
        
        /* Use results to prevent dead code elimination */
        sum += global_var1 + global_var2 + bitfield.a + bitfield.b;
    }
    
    printf("Test complete. Checksum: %d\n", sum);
    return sum > 0 ? 0 : 1;
}
