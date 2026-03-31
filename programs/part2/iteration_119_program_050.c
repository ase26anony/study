/* reload_test.c - Test program to exercise GCC's reload pass */
#include <stdio.h>
#include <stdint.h>

/* Global variables to create complex addressing modes */
volatile int global_var = 42;
volatile int global_array[100] = {0};

/* Bitfield structure to generate SUBREG operations */
struct bitfield_struct {
    unsigned int low16 : 16;
    unsigned int high16 : 16;
    volatile unsigned int full32;
} bf;

/* Test 1: Force secondary reloads with fixed register constraints */
void test_fixed_registers(void) {
    /* Bind variables to specific registers */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    register int r3 asm("esi");
    
    volatile int mem_var = 100;
    int result;
    
    /* Complex inline assembly with conflicting constraints */
    asm volatile (
        /* Move from memory to fixed register - may need secondary reload */
        "movl %1, %%ebx\n\t"
        "addl %%ecx, %%ebx\n\t"
        "movl %%ebx, %0"
        : "=rm" (result)
        : "m" (mem_var), "r" (global_var)
        : "ebx", "ecx", "memory"
    );
    
    /* Multiple alternative constraints */
    asm volatile (
        "imull %2, %0"
        : "+&a" (r1), "=&r" (r2)
        : "rm" (global_var), "1" (r3)
        : "cc", "edx"
    );
    
    /* Force reload with immediate value to fixed register */
    asm volatile (
        "addl $0x1234, %0"
        : "+a" (r1)
        :
        : "cc"
    );
}

/* Test 2: Complex addressing modes with memory operands */
void test_complex_addressing(void) {
    volatile int* ptr = &global_var;
    int index = 10;
    int result;
    
    /* Memory operand with displacement - may require secondary reload */
    asm volatile (
        "movl 4(%%esi), %%eax\n\t"
        "addl (%%ebx), %%eax"
        : "=a" (result)
        : "S" (ptr), "b" (&global_array[index])
        : "memory"
    );
    
    /* Multiple memory references with different constraints */
    asm volatile (
        "movl (%1), %%ecx\n\t"
        "addl (%2), %%ecx\n\t"
        "movl %%ecx, %0"
        : "=rm" (result)
        : "r" (&global_var), "m" (global_array[20])
        : "ecx", "memory"
    );
}

/* Test 3: Bitfield operations to generate SUBREG */
void test_bitfield_ops(void) {
    /* Access bitfield - generates SUBREG in RTL */
    bf.low16 = 0xABCD;
    bf.high16 = 0x1234;
    
    volatile uint16_t temp;
    
    /* Force partial register access */
    asm volatile (
        "movw %1, %%ax\n\t"
        "addw $1, %%ax\n\t"
        "movw %%ax, %0"
        : "=rm" (temp)
        : "rm" (bf.low16)
        : "ax"
    );
    
    /* Mix 16-bit and 32-bit operations */
    uint32_t full = bf.full32;
    uint16_t half = (uint16_t)full;  /* Explicit truncation */
    
    asm volatile (
        "addw %1, %0"
        : "+r" (half)
        : "rm" (temp)
        : "cc"
    );
}

/* Test 4: STRICT_LOW_PART patterns */
void test_strict_low_part(void) {
    volatile uint32_t x = 0x12345678;
    uint16_t y;
    
    /* This should generate STRICT_LOW_PART */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movw %%ax, %0"
        : "=rm" (y)
        : "rm" (x)
        : "eax"
    );
    
    /* Arithmetic with partial result */
    asm volatile (
        "addw $100, %0"
        : "+r" (y)
        :
        : "cc"
    );
}

/* Test 5: Multiple reloads with volatile and optimization barriers */
void test_volatile_reloads(void) {
    volatile int v1, v2, v3;
    register int r4 asm("edi");
    register int r5 asm("ebp");
    
    /* Create dependency chain with volatile */
    v1 = global_var;
    asm volatile ("" : "+m" (v1));  /* Optimization barrier */
    
    v2 = v1 + 1;
    asm volatile ("" : "+m" (v2));
    
    /* Complex inline assembly with many constraints */
    asm volatile (
        "movl %2, %%eax\n\t"
        "addl %3, %%eax\n\t"
        "imull %%edi, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "movl %%eax, %1"
        : "=rm" (v3), "=r" (r4)
        : "m" (v1), "rm" (v2), "0" (r5)
        : "eax", "edx", "cc", "memory"
    );
    
    /* Loop to increase reload pressure */
    for (volatile int i = 0; i < 10; i++) {
        asm volatile (
            "addl $1, %0"
            : "+r" (r4)
            :
            : "cc"
        );
    }
}

/* Test 6: Secondary reloads with different register classes */
void test_register_classes(void) {
    double d1 = 3.14, d2 = 2.71;
    int result;
    
    /* Mix integer and floating point - may require special handling */
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "faddp\n\t"
        "fistpl %0"
        : "=m" (result)
        : "m" (d1), "m" (d2)
        : "memory"
    );
    
    /* SSE registers with memory operands */
    float f1 = 1.0f, f2 = 2.0f;
    asm volatile (
        "movss %1, %%xmm0\n\t"
        "addss %2, %%xmm0\n\t"
        "movss %%xmm0, %0"
        : "=m" (f1)
        : "m" (f2), "m" (global_var)
        : "xmm0", "memory"
    );
}

/* Main function that runs all tests */
int main(void) {
    int total = 0;
    
    printf("Starting reload pass stress test...\n");
    
    /* Execute all tests multiple times to increase coverage chance */
    for (int i = 0; i < 5; i++) {
        test_fixed_registers();
        test_complex_addressing();
        test_bitfield_ops();
        test_strict_low_part();
        test_volatile_reloads();
        test_register_classes();
        
        /* Use results to prevent dead code elimination */
        total += global_var + global_array[i] + bf.low16;
    }
    
    printf("Test complete. Total: %d\n", total);
    return total != 0 ? 0 : 1;
}
