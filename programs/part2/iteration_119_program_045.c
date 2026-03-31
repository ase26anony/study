/* reload_test.c - Test program to trigger secondary reload initialization in GCC's reload pass */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex addressing modes */
volatile int global_volatile_int = 42;
volatile long global_volatile_long = 100;
int global_array[100] = {0};

/* Bitfield structure to generate SUBREG operations */
struct bitfield_struct {
    unsigned int low16 : 16;
    unsigned int high16 : 16;
    volatile unsigned int volatile_field : 8;
} bitfield_global;

/* Test 1: Force secondary reloads with restrictive register constraints */
void test_restrictive_constraints(void) {
    int input = global_volatile_int;
    int output;
    
    /* Force use of specific registers with memory operands */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(output)          /* Output in any register */
        : "m"(input)            /* Input from memory */
        : "%eax", "memory", "cc"
    );
    
    /* Multiple alternative constraints */
    int temp = output;
    asm volatile (
        "imull %1, %0"
        : "+r,a"(temp)          /* Input/output in register or eax specifically */
        : "rm,i"(global_volatile_int)  /* Register, memory, or immediate */
        : "cc"
    );
    
    global_array[0] = temp;
}

/* Test 2: Register-bound variables with conflicting constraints */
void test_register_conflicts(void) {
    /* Bind variables to specific registers */
    register int ebx_var asm("ebx") = global_volatile_int;
    register int ecx_var asm("ecx") = global_volatile_int + 1;
    
    /* Force conflict: use ebx-bound variable where eax is required */
    int result;
    asm volatile (
        "xchgl %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "xchgl %%ebx, %%eax"
        : "=a"(result), "+&b"(ebx_var)
        : "c"(ecx_var)
        : "cc"
    );
    
    /* Complex addressing with register displacement */
    asm volatile (
        "movl (%%ebx, %%ecx, 4), %%eax"
        : "=a"(result)
        : "b"(&global_array), "c"(result % 10)
        : "memory"
    );
}

/* Test 3: SUBREG and partial register access patterns */
void test_subreg_patterns(void) {
    /* Bitfield operations generate SUBREG */
    bitfield_global.low16 = (uint16_t)global_volatile_int;
    bitfield_global.high16 = (uint16_t)(global_volatile_int >> 16);
    
    /* Explicit truncation */
    int32_t full_int = global_volatile_long;
    int16_t half_int = (int16_t)full_int;
    
    /* Use truncated value in restrictive context */
    asm volatile (
        "movw %1, %%ax\n\t"
        "cwtl\n\t"
        "addl %%eax, %0"
        : "+m"(global_array[1])
        : "r"(half_int)
        : "%eax", "cc"
    );
    
    /* STRICT_LOW_PART pattern via inline asm */
    uint32_t combined;
    asm volatile (
        "movl %1, %0\n\t"
        "andl $0xFFFF, %0"
        : "=&r"(combined)
        : "r"(full_int)
        : "cc"
    );
}

/* Test 4: Complex addressing modes requiring secondary reloads */
void test_complex_addressing(void) {
    int index = global_volatile_int & 0xF;
    int scale = 2;
    
    /* Addressing mode that might need secondary reload */
    int value;
    asm volatile (
        "movl (%%eax, %%ebx, %c2), %%ecx"
        : "=c"(value)
        : "a"(&global_array), "b"(index), "i"(scale)
        : "memory"
    );
    
    /* Multiple memory operands with different constraints */
    int src = global_volatile_int;
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "addl %%eax, %0"
        : "=m"(global_array[index])
        : "m"(src)
        : "%eax", "cc", "memory"
    );
}

/* Test 5: Mixed volatile operations with optimization barriers */
void test_volatile_mix(void) {
    volatile int local_volatile = global_volatile_int;
    register int edx_var asm("edx") = 0;
    
    /* Memory clobber forces conservative treatment */
    asm volatile (
        ""
        : "+m"(local_volatile)
        :
        : "memory"
    );
    
    /* Complex operation with volatile and fixed register */
    asm volatile (
        "movl %1, %%edx\n\t"
        "leal (%%edx, %%edx, 4), %%eax\n\t"
        "addl %%eax, %0"
        : "+m"(local_volatile)
        : "r"(global_volatile_int)
        : "%eax", "%edx", "cc"
    );
    
    /* Force spill/reload with volatile */
    edx_var = local_volatile;
    asm volatile (
        "movl %%edx, %%ecx\n\t"
        "shrl $2, %%ecx"
        : "=c"(local_volatile)
        : "d"(edx_var)
        : "cc"
    );
}

/* Test 6: Nested inline assembly with multiple constraints */
void test_nested_constraints(void) {
    int a = global_volatile_int;
    int b = global_volatile_int + 1;
    int c, d;
    
    /* Multiple outputs with different register classes */
    asm volatile (
        "movl %2, %%eax\n\t"
        "movl %3, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "subl %%ebx, %%eax\n\t"
        "movl %%eax, %1"
        : "=r"(c), "=a"(d)      /* One in any reg, one specifically in eax */
        : "m"(a), "r"(b)
        : "%ebx", "cc"
    );
    
    /* Use results in another constrained operation */
    asm volatile (
        "cmpl %1, %0\n\t"
        "setg %%al\n\t"
        "movzbl %%al, %0"
        : "+r"(c)
        : "r"(d)
        : "%eax", "cc"
    );
    
    global_array[2] = c;
}

/* Main function that runs all tests */
int main(void) {
    int result = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i;
    }
    
    /* Run all tests multiple times to increase reload opportunities */
    for (int iteration = 0; iteration < 10; iteration++) {
        test_restrictive_constraints();
        test_register_conflicts();
        test_subreg_patterns();
        test_complex_addressing();
        test_volatile_mix();
        test_nested_constraints();
        
        /* Use results to prevent dead code elimination */
        result += global_array[iteration % 10];
        result += bitfield_global.low16;
    }
    
    printf("Result: %d\n", result);
    return result & 0xFF;  /* Return non-zero to indicate execution */
}
