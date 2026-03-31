/* reload_stress_test.c
 * Designed to trigger secondary reload initialization in GCC's reload.cc
 * Compile with: gcc -O1 -fno-omit-frame-pointer -g -c reload_stress_test.c
 * Or: gcc -O2 -finline-small-functions -fno-schedule-insns -fno-schedule-insns2 -c reload_stress_test.c
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex data flows */
volatile int global_volatile = 1234;
int global_normal = 5678;
register int reg_var asm("ebx");

/* Bitfield structure to generate SUBREG RTL */
struct bitfield_struct {
    int full : 32;
    int part : 16;
    int small : 8;
} bf;

/* Test 1: Force secondary reloads with fixed register constraints */
void test_fixed_reg_constraints(void) {
    int input = global_volatile;
    int output;
    
    /* Force input from memory to fixed register */
    asm volatile ("movl %1, %%eax\n\t"
                  "movl %%eax, %0"
                  : "=r"(output)
                  : "m"(input)
                  : "%eax", "memory");
    
    /* Multiple alternative constraints with fixed output */
    int in2 = global_normal;
    asm volatile ("addl %1, %0"
                  : "+a"(output)  /* Fixed to eax */
                  : "rm"(in2)     /* Register or memory */
                  : "cc");
    
    global_normal = output;
}

/* Test 2: Complex addressing modes with register binding conflicts */
void test_register_conflicts(void) {
    /* Bind to specific register, then force move to another */
    register int x asm("esi");
    register int y asm("edi");
    
    x = global_volatile;
    y = global_normal;
    
    /* This may require secondary reloads due to register binding */
    asm volatile ("movl %1, %%ecx\n\t"
                  "addl %%ecx, %0"
                  : "+r"(y)
                  : "r"(x)
                  : "%ecx", "cc");
    
    /* Force memory operand with fixed register constraint */
    asm volatile ("imull %1, %0"
                  : "+a"(y)      /* Fixed to eax */
                  : "m"(global_volatile)
                  : "cc");
    
    reg_var = y;  /* Use the global register variable */
}

/* Test 3: SUBREG/STRICT_LOW_PART patterns via bitfields */
void test_bitfield_operations(void) {
    bf.full = global_normal;
    
    /* Accessing bitfields generates SUBREG RTL */
    int16_t partial = bf.part;
    int8_t small = bf.small;
    
    /* Operations that may require partial register accesses */
    asm volatile ("addw %1, %0"
                  : "+r"(partial)
                  : "rm"(small)
                  : "cc");
    
    /* Explicit truncation */
    int32_t large = global_volatile;
    int16_t truncated = (int16_t)large;
    
    /* Use in context requiring reloads */
    asm volatile ("movw %1, %0"
                  : "=r"(truncated)
                  : "rm"(partial)
                  : "cc");
    
    bf.part = truncated;
}

/* Test 4: Multiple reloads with volatile and memory clobbers */
void test_volatile_reloads(void) {
    volatile int v1 = global_volatile;
    volatile int v2 = global_normal;
    int result;
    
    /* Memory barrier to force reloads */
    asm volatile ("" ::: "memory");
    
    /* Complex constraint combination */
    asm volatile ("movl %2, %%eax\n\t"
                  "subl %1, %%eax\n\t"
                  "movl %%eax, %0"
                  : "=rm"(result)
                  : "r"(v1), "m"(v2)
                  : "%eax", "cc", "memory");
    
    /* Multiple outputs with different constraints */
    int out1, out2;
    asm volatile ("movl %3, %%eax\n\t"
                  "movl %%eax, %0\n\t"
                  "movl %4, %%ebx\n\t"
                  "movl %%ebx, %1"
                  : "=r"(out1), "=m"(out2)
                  : "0"(result), "m"(global_volatile), "r"(global_normal)
                  : "%eax", "%ebx", "memory");
    
    global_normal = out1 + out2;
}

/* Test 5: Nested inline assembly with complex operands */
void test_nested_constraints(void) {
    int a = global_volatile;
    int b = global_normal;
    int c = reg_var;
    
    /* Three-operand instruction simulation with constraints */
    asm volatile ("movl %1, %%eax\n\t"
                  "addl %2, %%eax\n\t"
                  "movl %%eax, %0"
                  : "=r"(a)
                  : "0"(a), "rm"(b)
                  : "%eax", "cc");
    
    /* Try to force secondary reload with immediate */
    asm volatile ("subl %1, %0"
                  : "+r"(c)
                  : "i"(255)  /* Immediate constraint */
                  : "cc");
    
    /* Mixed size operations */
    int16_t d = bf.part;
    asm volatile ("addw %1, %0"
                  : "+r"(d)
                  : "rm"(c)
                  : "cc");
    
    reg_var = c;
    bf.part = d;
}

/* Main function that orchestrates all tests */
int main(void) {
    int sum = 0;
    
    /* Initialize bitfield */
    bf.full = 0x12345678;
    bf.part = 0x9ABC;
    bf.small = 0xDE;
    
    /* Initialize register variable */
    reg_var = 1000;
    
    /* Run tests multiple times to increase reload opportunities */
    for (int i = 0; i < 3; i++) {
        test_fixed_reg_constraints();
        test_register_conflicts();
        test_bitfield_operations();
        test_volatile_reloads();
        test_nested_constraints();
        
        sum += global_normal + global_volatile + reg_var + bf.full;
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", sum);
    
    return sum > 0 ? 0 : 1;
}
