/* reload_coverage.c - Test program to exercise GCC's reload pass */
#include <stdio.h>
#include <stdint.h>

/* Global variables to create complex addressing modes */
volatile int global_volatile = 42;
int global_array[100] = {0};
volatile int* volatile volatile_ptr = &global_volatile;

/* Bitfield structure to generate SUBREG operations */
struct bitfield_struct {
    unsigned int a : 5;
    unsigned int b : 12;
    unsigned int c : 15;
} bitfield = {1, 2, 3};

/* Test 1: Force secondary reloads with restrictive register constraints */
void test_restrictive_constraints(void) {
    int input = 1234;
    int output;
    
    /* Force use of specific registers with memory operands */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(output)
        : "m"(input)
        : "%eax", "memory"
    );
    
    /* Multiple alternative constraints with fixed register output */
    register int r1 asm("ebx") = 5678;
    asm volatile (
        "addl %1, %0"
        : "+a"(output)  /* eax only */
        : "rm"(r1)      /* register or memory */
        : "cc"
    );
    
    /* Complex constraint with immediate */
    asm volatile (
        "imull %1, %0"
        : "+r"(output)
        : "rmi"(global_volatile)  /* register, memory, or immediate */
        : "cc"
    );
}

/* Test 2: Register-bound variables with conflicting requirements */
void test_register_conflicts(void) {
    /* Bind variables to specific registers */
    register int x asm("esi") = 100;
    register int y asm("edi") = 200;
    register int z asm("ebx") = 300;
    
    volatile int temp;
    
    /* Force moves between fixed registers */
    asm volatile (
        "movl %1, %0\n\t"
        "addl %2, %0"
        : "=r"(temp)
        : "r"(x), "r"(y)
        : "memory"
    );
    
    /* Use all bound registers in complex expression */
    asm volatile (
        "leal (%1, %2, 2), %0"
        : "=r"(z)
        : "r"(x), "r"(y)
    );
    
    /* Conflict: try to use ebx for input when it's already bound to z */
    asm volatile (
        ""
        : "+m"(global_volatile)
        : "b"(z)  /* ebx constraint */
        : "cc"
    );
}

/* Test 3: SUBREG and partial register access patterns */
void test_subreg_patterns(void) {
    /* Operations that generate SUBREG RTL */
    int32_t full = 0x12345678;
    int16_t half = (int16_t)full;  /* Potential SUBREG */
    int8_t quarter = (int8_t)full;
    
    /* Bitfield operations generate SUBREG accesses */
    bitfield.a = half & 0x1F;
    bitfield.b = (half >> 5) & 0xFFF;
    bitfield.c = full >> 17;
    
    /* Use partial results in arithmetic */
    volatile int result;
    result = half + quarter;
    result = bitfield.b * bitfield.c;
    
    /* STRICT_LOW_PART pattern via inline asm */
    asm volatile (
        "addw %1, %0"
        : "+r"(half)
        : "rm"(bitfield.a)
        : "cc"
    );
}

/* Test 4: Complex addressing modes with multiple memory references */
void test_complex_addressing(void) {
    int index = 10;
    int scale = 2;
    int offset = 5;
    int result;
    
    /* Multiple memory operands with different addressing modes */
    asm volatile (
        "movl (%1, %2, %3), %0\n\t"
        "addl %4, %0"
        : "=r"(result)
        : "r"(global_array), "r"(index), "i"(sizeof(int)), "m"(global_array[offset])
        : "memory"
    );
    
    /* Base + index + displacement */
    asm volatile (
        "imull %1, %0"
        : "+r"(result)
        : "m"(global_array[index * scale + offset])
        : "cc"
    );
    
    /* Pointer chasing with volatile */
    result = *volatile_ptr;
    result += volatile_ptr[global_volatile & 15];
}

/* Test 5: Mixed constraints and spilling scenarios */
void test_mixed_constraints(void) {
    double d1 = 3.14159;
    double d2 = 2.71828;
    int i1, i2, i3;
    
    /* Mix float and integer constraints */
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "faddp\n\t"
        "fistpl %0"
        : "=m"(i1)
        : "m"(d1), "m"(d2)
        : "memory"
    );
    
    /* Many operands to force register pressure */
    asm volatile (
        "movl %1, %0\n\t"
        "addl %2, %0\n\t"
        "subl %3, %0\n\t"
        "andl %4, %0"
        : "=r"(i2)
        : "r"(global_volatile), "i"(100), "m"(global_array[5]), "r"(0xFF)
        : "cc"
    );
    
    /* Output operand with earlyclobber */
    i3 = 999;
    asm volatile (
        "movl %1, %0\n\t"
        "roll $5, %0"
        : "=&r"(i3)  /* earlyclobber - can't share registers with inputs */
        : "r"(i2)
        : "cc"
    );
}

/* Test 6: Nested inline asm and optimization barriers */
void test_nested_reloads(void) {
    volatile int barrier1 = 0, barrier2 = 0;
    int value1 = 123, value2 = 456;
    
    /* Optimization barrier */
    asm volatile ("" : "+g"(barrier1), "+g"(barrier2) : : "memory");
    
    /* Nested constraints */
    asm volatile (
        "pushl %%eax\n\t"
        "movl %1, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "popl %%eax"
        : "=rm"(value1)
        : "irm"(value2)  /* immediate, register, or memory */
        : "memory"
    );
    
    /* Memory clobber to force reloads */
    asm volatile (
        ""
        : 
        : "r"(value1), "r"(value2)
        : "eax", "ebx", "ecx", "edx", "memory", "cc"
    );
}

/* Main function that runs all tests */
int main(void) {
    int total = 0;
    
    /* Run each test multiple times with different values */
    for (int i = 0; i < 10; i++) {
        global_volatile = i * 100;
        global_array[i] = i * 50;
        
        test_restrictive_constraints();
        test_register_conflicts();
        test_subreg_patterns();
        test_complex_addressing();
        test_mixed_constraints();
        test_nested_reloads();
        
        total += global_volatile + global_array[i];
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    /* Final complex asm to ensure all paths are used */
    asm volatile (
        "movl %0, %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %0"
        : "+m"(total)
        : 
        : "%eax", "cc"
    );
    
    return total & 0xFF;  /* Return non-zero to indicate execution */
}
