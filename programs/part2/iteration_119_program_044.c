/* reload_test.c - Comprehensive test to trigger secondary reload initialization */
#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex addressing modes */
volatile int global_volatile_int = 42;
volatile long global_volatile_long = 100;
int global_int = 77;
long global_long = 200;

/* Bitfield structure to generate SUBREG RTL */
struct bitfield_struct {
    unsigned int low16 : 16;
    unsigned int high16 : 16;
    volatile unsigned int volatile_field : 8;
};

struct bitfield_struct bitfield_global = {0};

/* Test 1: Force secondary reloads via restrictive register constraints */
void test_restrictive_constraints(void) {
    int result1, result2;
    int input1 = 1234;
    long input2 = 5678;
    
    /* Force 'a' constraint (eax) with memory operand */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(result1)
        : "m"(global_volatile_int)
        : "%eax"
    );
    
    /* Multiple alternative constraints with fixed register output */
    asm volatile (
        "add %1, %0"
        : "+a"(result1)          /* eax only */
        : "rm"(input1)           /* register or memory */
        : "cc"
    );
    
    /* Force secondary reload for 64-bit on 32-bit architectures */
    asm volatile (
        "movq %1, %%rax\n\t"
        "movq %%rax, %0"
        : "=r"(result2)
        : "m"(global_volatile_long)
        : "%rax"
    );
    
    /* Complex constraint with immediate value */
    asm volatile (
        "imul $100, %1, %0"
        : "=a"(result1)          /* must be eax */
        : "rI"(37)               /* register or immediate */
        : "cc"
    );
}

/* Test 2: Register-bound variables causing conflicts */
void test_register_conflicts(void) {
    /* Bind to specific registers */
    register int reg_eax asm("eax");
    register int reg_ebx asm("ebx");
    register int reg_ecx asm("ecx");
    
    int temp1, temp2;
    
    /* Force conflict: variable bound to eax but operation needs ebx */
    reg_eax = global_int;
    reg_ebx = global_volatile_int;
    
    asm volatile (
        "xchg %%ebx, %%eax\n\t"
        "add $1, %%eax"
        : "+a"(reg_eax), "+b"(reg_ebx)
        :
        : "cc"
    );
    
    /* Multiple register constraints causing potential reloads */
    asm volatile (
        "movl %2, %%ecx\n\t"
        "leal (%%eax,%%ebx,1), %%eax\n\t"
        "add %%ecx, %%eax"
        : "+a"(reg_eax)
        : "b"(reg_ebx), "r"(reg_ecx)
        : "%ecx", "cc"
    );
    
    /* Use the results to prevent dead code elimination */
    global_int += reg_eax;
}

/* Test 3: SUBREG and STRICT_LOW_PART patterns */
void test_subreg_patterns(void) {
    int32_t full32 = 0x12345678;
    int16_t half16;
    int8_t byte8;
    
    /* Generate SUBREG for 16-bit access */
    half16 = (int16_t)full32;
    
    /* Force use in arithmetic requiring extension */
    asm volatile (
        "addw %1, %0"
        : "+r"(half16)
        : "rm"(global_volatile_int)
        : "cc"
    );
    
    /* Bitfield operations generate SUBREG */
    bitfield_global.low16 = half16;
    bitfield_global.high16 = global_int & 0xFFFF;
    bitfield_global.volatile_field = 0xAA;
    
    /* Access bitfield causing partial register access */
    uint16_t extracted = bitfield_global.low16;
    
    /* STRICT_LOW_PART pattern via masking */
    asm volatile (
        "andl $0xFFFF, %0"
        : "+r"(full32)
        :
        : "cc"
    );
    
    /* Byte access causing further SUBREG */
    byte8 = (int8_t)full32;
    asm volatile (
        "addb $1, %0"
        : "+r"(byte8)
        :
        : "cc"
    );
    
    global_int += extracted + byte8;
}

/* Test 4: Complex addressing modes with displacement */
void test_complex_addressing(void) {
    int array[100];
    int index = 50;
    int result;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        array[i] = i * 2;
    }
    
    /* Force complex address calculation with fixed register */
    asm volatile (
        "movl (%1,%2,4), %%eax\n\t"
        "addl $100, %%eax"
        : "=a"(result)
        : "r"(array), "r"(index)
        : "cc"
    );
    
    /* Memory operand with large displacement */
    asm volatile (
        "movl 96(%1), %%eax\n\t"
        "subl %2, %%eax"
        : "=a"(result)
        : "r"(array), "r"(global_int)
        : "cc"
    );
    
    /* Multiple memory references */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(result)
        : "m"(array[10]), "m"(array[20])
        : "%eax", "cc"
    );
    
    global_int += result;
}

/* Test 5: Mixed width operations requiring reloads */
void test_mixed_width_ops(void) {
    int64_t big_val = 0x123456789ABCDEF0LL;
    int32_t small_val;
    int16_t smaller_val;
    
    /* 64-bit to 32-bit truncation */
    small_val = (int32_t)big_val;
    
    /* Operation requiring extension */
    asm volatile (
        "movswl %1, %0"
        : "=r"(small_val)
        : "r"(smaller_val)
    );
    
    /* Mixed register sizes in operation */
    asm volatile (
        "addl %1, %k0"  /* %k0 for 32-bit version of 64-bit register */
        : "+r"(big_val)
        : "r"(small_val)
        : "cc"
    );
    
    /* Force reload for float/int conversion */
    double dbl = 3.14159;
    int int_from_dbl;
    
    asm volatile (
        "cvttsd2si %1, %0"
        : "=r"(int_from_dbl)
        : "x"(dbl)
    );
    
    global_int += small_val + int_from_dbl;
}

/* Test 6: Volatile barriers creating reload points */
void test_volatile_barriers(void) {
    volatile int barrier1 = 0;
    volatile int barrier2 = 0;
    int temp;
    
    /* Memory clobber forcing reloads */
    asm volatile (
        "movl %1, %%eax\n\t"
        : "=a"(temp)
        : "m"(barrier1)
        : "memory"
    );
    
    /* Multiple volatile accesses */
    barrier1 = global_int;
    
    asm volatile (
        ""
        : "+m"(barrier1), "+m"(barrier2)
    );
    
    /* Inline asm with many operands */
    int a = 1, b = 2, c = 3, d = 4;
    
    asm volatile (
        "imull %2, %1\n\t"
        "addl %1, %0\n\t"
        "subl %3, %0"
        : "+r"(a), "+r"(b)
        : "r"(c), "r"(d)
        : "cc"
    );
    
    /* Use results */
    barrier2 = a + b;
    global_volatile_int = barrier2;
}

/* Main function orchestrating all tests */
int main(void) {
    int total = 0;
    
    printf("Starting reload stress tests...\n");
    
    /* Run each test multiple times with different values */
    for (int i = 0; i < 10; i++) {
        global_int = i * 100;
        global_volatile_int = i * 50 + 1;
        
        test_restrictive_constraints();
        test_register_conflicts();
        test_subreg_patterns();
        test_complex_addressing();
        test_mixed_width_ops();
        test_volatile_barriers();
        
        total += global_int + global_volatile_int;
    }
    
    /* Final computation using all modified globals */
    total += bitfield_global.low16 + bitfield_global.high16;
    
    printf("Total: %d\n", total);
    
    /* Return non-zero to ensure all code paths matter */
    return total != 0 ? 0 : 1;
}
