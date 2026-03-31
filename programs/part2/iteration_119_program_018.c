/* reload_test.c - Test program to trigger secondary reload initialization in GCC's reload pass */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex addressing modes */
volatile int global_var = 42;
volatile int global_array[100] = {0};
volatile struct {
    int a;
    int b:16;  /* Bit-field for SUBREG generation */
    int c:16;
} bitfield_struct = {0};

/* Test 1: Force secondary reloads with restrictive register constraints */
void test_restrictive_constraints(void) {
    int input = 123;
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
    register int x asm("ebx") = 456;
    asm volatile (
        "addl %1, %0"
        : "+a"(output)  /* eax only */
        : "rm"(x)       /* register or memory */
        : "cc"
    );
    
    /* Complex constraint with immediate */
    asm volatile (
        "imull %1, %0"
        : "+a"(output)
        : "rmi"(789)    /* register, memory, or immediate */
        : "cc"
    );
}

/* Test 2: Register-bound variables with conflicting requirements */
void test_register_conflicts(void) {
    /* Bind variables to specific registers */
    register int r1 asm("esi") = 1;
    register int r2 asm("edi") = 2;
    register int r3 asm("ebx") = 3;
    
    volatile int mem_var = 100;
    
    /* Force moves between specific registers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0\n\t"
        "movl %2, %%ecx\n\t"
        "subl %%ecx, %0"
        : "+r"(r1)
        : "r"(r2), "m"(mem_var)
        : "%eax", "%ecx", "cc", "memory"
    );
    
    /* Chain of operations requiring register shuffling */
    int temp;
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "movl %2, %%ebx\n\t"
        "addl %%ebx, %0"
        : "=r"(temp)
        : "r"(r3), "m"(global_var)
        : "%eax", "%ebx", "memory"
    );
}

/* Test 3: SUBREG and partial register access patterns */
void test_subreg_patterns(void) {
    /* Bit-field access generates SUBREG */
    bitfield_struct.b = 0xABCD;
    bitfield_struct.c = 0x1234;
    
    /* Access partial registers */
    int32_t full = 0x12345678;
    int16_t partial;
    
    /* This should generate SUBREG */
    partial = (int16_t)full;
    
    /* Use partial in restrictive context */
    asm volatile (
        "movw %1, %%ax\n\t"
        "addw $100, %%ax\n\t"
        "movw %%ax, %0"
        : "=r"(partial)
        : "r"(partial)
        : "%ax", "cc"
    );
    
    /* Mixed-size operations */
    volatile int16_t v16 = 1000;
    volatile int32_t v32 = 2000000;
    
    asm volatile (
        "movswl %1, %%eax\n\t"
        "addl %%eax, %0"
        : "+m"(v32)
        : "r"(v16)
        : "%eax", "cc", "memory"
    );
}

/* Test 4: Complex addressing modes with memory operands */
void test_complex_addressing(void) {
    int index = 50;
    int scale = 4;
    int offset = 100;
    
    /* Complex memory addressing that might need secondary reloads */
    asm volatile (
        "movl (%1, %2, %3), %%eax\n\t"
        "addl %4, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(global_array[0])
        : "r"(global_array), "r"(index), "r"(scale), "i"(offset)
        : "%eax", "memory"
    );
    
    /* Multiple memory inputs with restrictive output */
    int result;
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0"
        : "=a"(result)  /* eax only */
        : "m"(global_var), "m"(global_array[10])
        : "memory"
    );
}

/* Test 5: Inline assembly with output/input overlap */
void test_input_output_overlap(void) {
    int value = 777;
    
    /* Output overlaps with input register */
    asm volatile (
        "addl $111, %0"
        : "+a"(value)   /* Input/output in eax */
        :               /* No separate inputs */
        : "cc"
    );
    
    /* Multiple constraints that conflict */
    int in1 = 100, in2 = 200, out;
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(out), "+&r"(in1)  /* Early clobber on in1 */
        : "r"(in2)
        : "%eax", "cc"
    );
}

/* Test 6: Force reloads with volatile and optimization barriers */
void test_volatile_reloads(void) {
    volatile int v1 = 1;
    volatile int v2 = 2;
    volatile int v3 = 3;
    
    /* Memory clobber forces reloads */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0"
        : "=m"(v3)
        : "r"(v1), "r"(v2)
        : "%eax", "cc", "memory"
    );
    
    /* Series of volatile operations */
    asm volatile ("" : "+m"(v1) : : "memory");
    asm volatile ("" : "+m"(v2) : : "memory");
    
    /* Mix with register constraints */
    register int r asm("ecx") = v1;
    asm volatile (
        "addl %%ecx, %0"
        : "+m"(v3)
        : /* ecx already bound */
        : "cc", "memory"
    );
}

/* Main function that runs all tests */
int main(void) {
    int total = 0;
    
    printf("Starting reload stress tests...\n");
    
    /* Run each test multiple times to increase coverage chances */
    for (int i = 0; i < 10; i++) {
        test_restrictive_constraints();
        test_register_conflicts();
        test_subreg_patterns();
        test_complex_addressing();
        test_input_output_overlap();
        test_volatile_reloads();
        
        /* Use results to prevent dead code elimination */
        total += global_var + global_array[0] + bitfield_struct.b;
    }
    
    printf("Tests completed. Total: %d\n", total);
    return total != 0 ? 0 : 1;
}
