/* reload_stress.c - Stress test for GCC's reload pass */
#include <stdint.h>

/* Global variables to create complex addressing modes */
volatile int g1 = 1;
volatile int g2 = 2;
volatile long g3 = 3;
volatile long g4 = 4;

/* Bitfield structure for SUBREG generation */
struct bitfields {
    int a : 5;
    int b : 12;
    int c : 15;
    volatile int d : 8;
};

volatile struct bitfields bf = {1, 2, 3, 4};

/* Function using register variables with explicit binding */
int test_register_vars(void) {
    /* Bind to specific registers (x86-64 example) */
    register int64_t r1 asm("rbx");
    register int64_t r2 asm("r12");
    register int64_t r3 asm("r13");
    
    r1 = g1;
    r2 = g2;
    r3 = g3;
    
    /* Force reloads by using the same variable in different contexts */
    int result;
    
    /* Complex inline asm with fixed register constraints */
    asm volatile (
        /* Move from memory to fixed register, may need secondary reload */
        "mov %[mem1], %%rax\n\t"
        "add %%rbx, %%rax\n\t"
        "mov %%rax, %[out]"
        : [out] "=rm" (result)
        : [mem1] "m" (g4), "r" (r1)
        : "rax", "cc", "memory"
    );
    
    /* Another asm with multiple alternative constraints */
    asm volatile (
        "imul %[in2], %[in1]\n\t"
        "add $0x1234, %[in1]"
        : [in1] "+r,a,m" (r2)
        : [in2] "r,i,m" (r3)
        : "cc"
    );
    
    return result + r2;
}

/* Function using complex addressing modes */
int test_addressing_modes(int n) {
    volatile int arr[10] = {0};
    int sum = 0;
    
    /* Force memory operands with different scales */
    for (int i = 0; i < n; i++) {
        int temp;
        /* Memory operand with displacement */
        asm volatile (
            "movl %[base], %[temp]\n\t"
            "addl %[index], %[temp]"
            : [temp] "=r" (temp)
            : [base] "m" (arr[i]), 
              [index] "rm" (i)
            : "cc"
        );
        sum += temp;
    }
    
    /* Use bitfields to generate SUBREG accesses */
    int bf_sum = bf.a + bf.b + bf.c;
    
    /* Force partial register access */
    int16_t low_part = (int16_t)sum;
    int32_t extended;
    
    /* STRICT_LOW_PART pattern */
    asm volatile (
        "movswl %w1, %0"
        : "=r" (extended)
        : "r" (low_part)
    );
    
    return extended + bf_sum;
}

/* Function with multiple output constraints */
void test_multi_output(void) {
    int out1, out2, out3;
    int in1 = g1, in2 = g2;
    
    /* Complex asm with multiple outputs and fixed registers */
    asm volatile (
        "mov %[in1], %%eax\n\t"
        "mov %[in2], %%ebx\n\t"
        "lea (%%eax, %%ebx, 2), %%ecx\n\t"
        "mov %%eax, %[o1]\n\t"
        "mov %%ebx, %[o2]\n\t"
        "mov %%ecx, %[o3]"
        : [o1] "=rm" (out1),
          [o2] "=rm" (out2),
          [o3] "=rm" (out3)
        : [in1] "rm" (in1),
          [in2] "rm" (in2)
        : "eax", "ebx", "ecx", "cc"
    );
    
    g1 = out1 + out2 + out3;
}

/* Function using immediate values with register constraints */
int test_immediate_constraints(void) {
    int result;
    volatile int mem_val = 42;
    
    /* Immediate input with fixed register output */
    asm volatile (
        "mov $0xDEADBEEF, %%eax\n\t"
        "add %[mem], %%eax\n\t"
        "sub $0x1000, %%eax"
        : "=a" (result)
        : [mem] "m" (mem_val)
        : "cc"
    );
    
    /* Multiple alternative constraints */
    asm volatile (
        "and %[imm], %[val]"
        : [val] "+r,m" (result)
        : [imm] "i,r,m" (0xFF)
        : "cc"
    );
    
    return result;
}

/* Main function that combines all tests */
int main(void) {
    int total = 0;
    
    /* Test 1: Register variables with explicit binding */
    total += test_register_vars();
    
    /* Test 2: Complex addressing modes */
    total += test_addressing_modes(5);
    
    /* Test 3: Multiple output constraints */
    test_multi_output();
    total += g1;
    
    /* Test 4: Immediate constraints */
    total += test_immediate_constraints();
    
    /* Additional inline asm with memory clobber to force more reloads */
    asm volatile (
        ""
        : 
        : 
        : "memory"
    );
    
    /* Use the result to prevent dead code elimination */
    volatile int sink = total;
    
    return sink & 0xFF;  /* Return non-zero to indicate execution */
}
