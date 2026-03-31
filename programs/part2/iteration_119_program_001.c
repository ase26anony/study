/* reload_stress_test.c
 * Designed to trigger secondary reload initialization in GCC's reload.cc
 * Compile with: gcc -O1 -fno-omit-frame-pointer -g -c reload_stress_test.c
 * Or: gcc -O2 -finline-small-functions -fno-schedule-insns -fno-schedule-insns2 -c reload_stress_test.c
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex addressing modes */
volatile int global_var1 = 1234;
volatile int global_var2 = 5678;
int global_array[100] = {0};

/* Bitfield structure to generate SUBREG RTL */
struct bitfield_struct {
    int full : 32;
    int partial : 16;
    int small : 8;
} bitfield_global;

/* Test 1: Force secondary reloads with restrictive register constraints */
void test_restrictive_constraints(void) {
    int input = 42;
    int output;
    
    /* Force 'eax' register constraint with memory input */
    asm volatile ("movl %1, %%eax\n\t"
                  "movl %%eax, %0"
                  : "=r"(output)
                  : "m"(input)
                  : "%eax");
    
    /* Multiple alternative constraints with fixed register output */
    int a = 10, b = 20;
    asm volatile ("addl %1, %0"
                  : "+a"(a)  /* 'a' constraint forces eax */
                  : "rm"(b)  /* register or memory */
                  : "cc");
    
    /* Complex constraint with immediate value */
    asm volatile ("imull %1, %0"
                  : "+r"(output)
                  : "ir"(100)  /* immediate or register */
                  : "cc");
}

/* Test 2: Register variables with explicit binding */
void test_register_variables(void) {
    /* Bind to specific registers */
    register int reg1 asm("ebx");
    register int reg2 asm("esi");
    register int reg3 asm("edi");
    
    reg1 = global_var1;
    reg2 = global_var2;
    
    /* Force conflict: use register-bound variable in asm requiring different register */
    asm volatile ("movl %1, %%eax\n\t"
                  "addl %%eax, %0"
                  : "+r"(reg3)
                  : "r"(reg1)
                  : "%eax", "cc");
    
    /* More complex register pressure */
    asm volatile (""
                  : 
                  : "r"(reg1), "r"(reg2), "r"(reg3)
                  : "memory");
}

/* Test 3: SUBREG and partial register access patterns */
void test_subreg_patterns(void) {
    /* Bitfield accesses generate SUBREG */
    bitfield_global.full = 0x12345678;
    bitfield_global.partial = (int16_t)bitfield_global.full;  /* Truncation */
    bitfield_global.small = (int8_t)bitfield_global.partial;
    
    /* Explicit truncation operations */
    int32_t large = 0x87654321;
    int16_t medium = (int16_t)large;
    int8_t small = (int8_t)medium;
    
    /* Use truncated values in operations requiring reloads */
    asm volatile ("addw %1, %0"
                  : "+r"(medium)
                  : "rm"(small)
                  : "cc");
    
    /* Access array elements with different sizes */
    global_array[0] = large;
    global_array[1] = medium;
    global_array[2] = small;
}

/* Test 4: Complex addressing modes with volatile */
void test_complex_addressing(void) {
    volatile int* volatile_ptr = &global_var1;
    int index = 5;
    
    /* Memory operand with complex addressing */
    asm volatile ("movl (%1, %2, 4), %%eax\n\t"
                  "addl %%eax, %0"
                  : "+r"(global_var2)
                  : "r"(global_array), "r"(index)
                  : "%eax", "memory", "cc");
    
    /* Multiple memory constraints */
    asm volatile (""
                  : 
                  : "m"(global_var1), "m"(global_var2)
                  : "memory");
}

/* Test 5: Mixed constraints and spilling */
void test_mixed_constraints(void) {
    int a, b, c, d, e, f;
    
    /* Create register pressure */
    a = global_var1;
    b = global_var2;
    c = a + b;
    d = b - a;
    e = c * d;
    f = e / 2;
    
    /* Inline asm with many operands and constraints */
    asm volatile ("imull %2, %1\n\t"
                  "addl %1, %0\n\t"
                  "subl %3, %0"
                  : "+&r"(f), "+&r"(e)
                  : "rm"(d), "rm"(c)
                  : "cc");
    
    /* Force memory spill with 'g' constraint */
    asm volatile ("movl %1, %0"
                  : "=g"(global_array[10])
                  : "g"(f));
}

/* Test 6: Nested inline assembly with clobbers */
void test_nested_asm(void) {
    int x = 100, y = 200, z = 300;
    
    /* First asm with many clobbers */
    asm volatile ("movl %1, %%ecx\n\t"
                  "movl %2, %%edx\n\t"
                  "leal (%%ecx, %%edx, 2), %0"
                  : "=r"(z)
                  : "r"(x), "r"(y)
                  : "%ecx", "%edx", "cc");
    
    /* Second asm using result with different constraints */
    asm volatile ("cmpl %1, %0\n\t"
                  "setg %%al\n\t"
                  "movzbl %%al, %0"
                  : "+r"(z)
                  : "ir"(250)
                  : "%eax", "cc");
}

/* Test 7: Loop with varying constraints to stress reload */
void test_loop_reloads(void) {
    int i;
    volatile int sum = 0;
    
    for (i = 0; i < 10; i++) {
        int temp = global_array[i];
        
        /* Alternate between different constraints in loop */
        if (i % 2 == 0) {
            asm volatile ("addl %1, %0"
                          : "+a"(sum)  /* Force eax */
                          : "rm"(temp)
                          : "cc");
        } else {
            asm volatile ("subl %1, %0"
                          : "+r"(sum)  /* General register */
                          : "rm"(temp)
                          : "cc");
        }
        
        /* Memory barrier to prevent optimization */
        asm volatile ("" : : : "memory");
    }
}

/* Test 8: Function with many parameters to force register spills */
static int __attribute__((noinline))
test_many_params(int p1, int p2, int p3, int p4, int p5, 
                 int p6, int p7, int p8, int p9, int p10) {
    /* Use all parameters in complex asm */
    int result;
    
    asm volatile ("addl %1, %0\n\t"
                  "addl %2, %0\n\t"
                  "addl %3, %0\n\t"
                  "addl %4, %0\n\t"
                  "addl %5, %0\n\t"
                  "addl %6, %0\n\t"
                  "addl %7, %0\n\t"
                  "addl %8, %0\n\t"
                  "addl %9, %0\n\t"
                  "addl %10, %0"
                  : "=r"(result)
                  : "r"(p1), "r"(p2), "r"(p3), "r"(p4), "r"(p5),
                    "r"(p6), "r"(p7), "r"(p8), "r"(p9), "r"(p10)
                  : "cc");
    
    return result;
}

/* Main function that runs all tests */
int main(void) {
    int total = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 2;
    }
    
    /* Run all tests to stress reload pass */
    test_restrictive_constraints();
    total += global_var2;
    
    test_register_variables();
    total += global_var1;
    
    test_subreg_patterns();
    total += bitfield_global.partial;
    
    test_complex_addressing();
    total += global_array[10];
    
    test_mixed_constraints();
    total += global_array[20];
    
    test_nested_asm();
    total += global_array[30];
    
    test_loop_reloads();
    total += global_array[0];
    
    /* Test with many parameters */
    int many_param_result = test_many_params(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    total += many_param_result;
    
    /* Final computation to ensure values are used */
    asm volatile ("movl %1, %%eax\n\t"
                  "addl $999, %%eax\n\t"
                  "movl %%eax, %0"
                  : "=m"(global_array[99])
                  : "r"(total)
                  : "%eax", "memory");
    
    printf("Result: %d\n", global_array[99]);
    return global_array[99] % 256;
}
