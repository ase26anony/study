/* reload_test.c - Designed to trigger secondary reload initialization in GCC's reload pass */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex addressing modes */
volatile int global_var1 = 1234;
volatile int global_var2 = 5678;
volatile long global_long = 999999L;

/* Bit-field structure to generate SUBREG operations */
struct bitfield_struct {
    unsigned int low16 : 16;
    unsigned int high16 : 16;
} volatile bitfield = {0xAAAA, 0x5555};

/* Test 1: Force secondary reloads with fixed register constraints */
void test_fixed_registers() {
    int result;
    int input = global_var1;
    
    /* Force use of specific registers with memory operands */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(result)
        : "m"(input)
        : "%eax", "cc"
    );
    
    /* Multiple alternative constraints with fixed output */
    register int x asm("ebx") = global_var2;
    asm volatile (
        "imull %1, %0"
        : "+a"(result)
        : "rm"(x)
        : "cc"
    );
    
    global_var1 = result;
}

/* Test 2: Complex addressing modes with multiple constraints */
void test_complex_addressing() {
    volatile int arr[10] = {1,2,3,4,5,6,7,8,9,10};
    register int idx asm("esi") = 3;
    int temp;
    
    /* Memory operand with index register - may need secondary reload */
    asm volatile (
        "movl (%1,%2,4), %0"
        : "=r"(temp)
        : "r"(arr), "r"(idx)
        : "memory"
    );
    
    /* Output to specific register class with memory input */
    asm volatile (
        "movl %1, %%ecx\n\t"
        "leal (%%ecx,%%ecx,2), %0"
        : "=r"(temp)
        : "m"(global_var1)
        : "%ecx"
    );
    
    /* Multiple constraints with immediate */
    asm volatile (
        "addl %1, %0"
        : "+rm"(temp)
        : "i"(100), "0"(temp)
        : "cc"
    );
    
    global_var2 = temp;
}

/* Test 3: SUBREG and partial register access patterns */
void test_subreg_patterns() {
    /* Bit-field access generates SUBREG */
    uint16_t low_part = bitfield.low16;
    uint16_t high_part = bitfield.high16;
    
    /* Explicit truncation */
    int32_t large_val = global_long;
    int16_t truncated = (int16_t)large_val;
    
    /* Combine with inline assembly requiring specific register */
    asm volatile (
        "movzwl %1, %0"
        : "=r"(large_val)
        : "r"(truncated)
    );
    
    /* STRICT_LOW_PART pattern via masking */
    int masked = large_val & 0xFFFF;
    
    /* Force reload with register constraint */
    register int reg_val asm("edi") = masked;
    asm volatile (
        "orl %1, %0"
        : "+r"(reg_val)
        : "rm"(low_part)
        : "cc"
    );
    
    bitfield.low16 = reg_val & 0xFFFF;
}

/* Test 4: Secondary reloads via restrictive constraints */
void test_restrictive_constraints() {
    double d1 = 3.14159;
    double d2 = 2.71828;
    double result;
    
    /* x87 floating point constraints - often need secondary reloads */
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "faddp %%st, %%st(1)\n\t"
        "fstpl %0"
        : "=m"(result)
        : "m"(d1), "m"(d2)
    );
    
    /* MMX/SSE constraints with memory operands */
    float f1 = 1.5f, f2 = 2.5f;
    asm volatile (
        "movss %1, %%xmm0\n\t"
        "addss %2, %%xmm0\n\t"
        "movss %%xmm0, %0"
        : "=m"(result)
        : "m"(f1), "m"(f2)
        : "xmm0"
    );
}

/* Test 5: Register binding conflicts */
void test_register_conflicts() {
    /* Bind multiple variables to the same register class */
    register int a asm("eax");
    register int b asm("ebx");
    register int c asm("ecx");
    
    a = global_var1;
    b = global_var2;
    
    /* Force moves between fixed registers */
    asm volatile (
        "xchgl %%eax, %%ebx"
        : "+a"(a), "+b"(b)
    );
    
    /* Complex operation requiring spill/reload */
    asm volatile (
        "movl %%eax, %%ecx\n\t"
        "imull %%ebx, %%ecx\n\t"
        "addl $42, %%ecx"
        : "=c"(c)
        : "a"(a), "b"(b)
        : "cc"
    );
    
    global_var1 = c;
}

/* Test 6: Volatile memory barriers */
void test_memory_barriers() {
    volatile int barrier1 = 0, barrier2 = 0;
    
    /* Memory clobber forces reloads */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0\n\t"
        : "+m"(barrier1)
        : "rm"(global_var1)
        : "%eax", "memory", "cc"
    );
    
    /* Compiler memory barrier */
    asm volatile ("" : : : "memory");
    
    /* Multiple memory outputs */
    asm volatile (
        "movl $999, %0\n\t"
        "movl $888, %1"
        : "=m"(barrier1), "=m"(barrier2)
        :
        : "memory"
    );
}

int main() {
    int sum = 0;
    
    printf("Starting reload stress test...\n");
    
    /* Execute all tests multiple times to increase coverage chance */
    for (int i = 0; i < 3; i++) {
        test_fixed_registers();
        sum += global_var1;
        
        test_complex_addressing();
        sum += global_var2;
        
        test_subreg_patterns();
        sum += bitfield.low16 + bitfield.high16;
        
        test_restrictive_constraints();
        
        test_register_conflicts();
        sum += global_var1;
        
        test_memory_barriers();
    }
    
    printf("Test complete. Checksum: %d\n", sum);
    return sum != 0 ? 0 : 1;
}
