/* reload_coverage.c - Test program to exercise GCC's reload pass */
#include <stdio.h>
#include <stdint.h>

/* Global variables to create complex addressing modes */
volatile int global_var = 42;
volatile int global_array[10] = {0};
volatile int *global_ptr = &global_var;

/* Bit-field structure to generate SUBREG operations */
struct bitfield_struct {
    unsigned int low16 : 16;
    unsigned int high16 : 16;
    volatile unsigned int full32;
} bitfield_global;

/* Test 1: Complex addressing modes with restrictive register constraints */
void test_complex_addressing(void) {
    volatile int local_volatile = 100;
    int result;
    
    /* Force secondary reload by requiring specific register with memory operand */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %0"
        : "=m"(local_volatile)      /* Output in memory */
        : "m"(global_var)           /* Input from memory */
        : "eax", "cc", "memory"
    );
    
    /* Multiple alternative constraints with fixed register output */
    register int reg_var asm("ebx") = 50;
    asm volatile (
        "imull %2, %1\n\t"
        "addl %1, %0"
        : "+a"(result)              /* EAX register constraint */
        : "r"(reg_var), "rm"(global_var)  /* EBX register + memory alternative */
        : "cc", "edx"
    );
    
    /* Complex constraint with immediate */
    asm volatile (
        "leal (%1, %2, 4), %%ecx\n\t"
        "movl %%ecx, %0"
        : "=r"(result)
        : "r"(reg_var), "i"(5)      /* Immediate constraint */
        : "ecx"
    );
}

/* Test 2: Register-bound variables with conflicting constraints */
void test_register_conflicts(void) {
    /* Bind variables to specific registers */
    register int a asm("esi");
    register int b asm("edi");
    register int c asm("ebx");
    
    a = 10;
    b = 20;
    c = 30;
    
    volatile int mem_temp;
    
    /* Force reload by using register-bound variable in conflicting context */
    asm volatile (
        "xchgl %0, %1\n\t"
        "addl %2, %0"
        : "+r"(a), "+r"(b)
        : "m"(global_array[2])      /* Memory operand forcing reload */
        : "cc"
    );
    
    /* Use all bound registers in complex expression */
    asm volatile (
        "movl %1, %%eax\n\t"
        "subl %2, %%eax\n\t"
        "imull %3, %%eax\n\t"
        "movl %%eax, %0"
        : "=m"(mem_temp)
        : "r"(a), "r"(b), "rm"(c)   /* Mixed register/memory constraints */
        : "eax", "edx", "cc", "memory"
    );
}

/* Test 3: Bit-field operations generating SUBREG patterns */
void test_bitfield_operations(void) {
    struct bitfield_struct local_bf;
    local_bf.low16 = 0x1234;
    local_bf.high16 = 0x5678;
    local_bf.full32 = 0x9ABCDEF0;
    
    /* Operations on bit-fields generate SUBREG accesses */
    volatile uint16_t temp16;
    temp16 = local_bf.low16;        /* Potential SUBREG extraction */
    local_bf.high16 = temp16 + 1;   /* Potential SUBREG insertion */
    
    /* Mix with inline assembly */
    asm volatile (
        "movzwl %1, %%eax\n\t"
        "addw $1, %%ax\n\t"
        "movw %%ax, %0"
        : "=m"(local_bf.low16)
        : "m"(local_bf.high16)
        : "eax", "cc", "memory"
    );
    
    /* Explicit truncation operations */
    uint32_t large_val = 0x12345678;
    uint16_t truncated;
    asm volatile (
        "movl %1, %%eax\n\t"
        "movw %%ax, %0"
        : "=rm"(truncated)          /* Register or memory output */
        : "rm"(large_val)           /* Register or memory input */
        : "eax"
    );
}

/* Test 4: Multiple output operands with restrictive classes */
void test_multiple_outputs(void) {
    int out1, out2, out3;
    volatile int in1 = 1, in2 = 2, in3 = 3;
    
    /* Multiple outputs with different register requirements */
    asm volatile (
        "movl %3, %%eax\n\t"
        "movl %4, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "movl %5, %%ecx\n\t"
        "subl %%ecx, %%ebx\n\t"
        "movl %%ebx, %1\n\t"
        "imull %%eax, %%ecx\n\t"
        "movl %%ecx, %2"
        : "=a"(out1), "=b"(out2), "=r"(out3)  /* Fixed and general registers */
        : "m"(in1), "m"(in2), "m"(in3)
        : "ecx", "cc", "memory"
    );
    
    /* Early clobber with memory operand */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "addl $1, %0"
        : "=&r"(out1)               /* Early clobber */
        : "m"(global_var)
        : "eax", "cc"
    );
}

/* Test 5: Complex memory addressing with displacement */
void test_complex_memory_addressing(void) {
    int index = 3;
    volatile int result;
    
    /* Complex addressing mode that might need secondary reload */
    asm volatile (
        "movl global_array(,%1,4), %%eax\n\t"
        "addl $10, %%eax\n\t"
        "movl %%eax, %0"
        : "=rm"(result)             /* Can be register or memory */
        : "r"(index)                /* Index in register */
        : "eax", "cc", "memory"
    );
    
    /* Pointer arithmetic with memory output */
    int *ptr = (int*)global_array;
    asm volatile (
        "movl (%1, %2, 4), %%eax\n\t"
        "movl %%eax, (%0)"
        : 
        : "r"(ptr), "r"(index), "m"(*ptr)  /* Memory input */
        : "eax", "memory"
    );
}

/* Test 6: Mixed-size operations forcing mode conversions */
void test_mixed_size_operations(void) {
    int8_t byte_var = 127;
    int16_t short_var = 32000;
    int32_t int_var = 1000000;
    volatile int64_t long_result;
    
    /* Mixed size operations requiring mode conversions */
    asm volatile (
        "movsbl %1, %%eax\n\t"
        "movswl %2, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "cltd\n\t"
        "idivl %3\n\t"
        "movl %%eax, %%ecx\n\t"
        "movl %%edx, %%ebx"
        : "=c"((int32_t)long_result)  /* Partial result in ECX */
        : "m"(byte_var), "m"(short_var), "rm"(int_var)
        : "eax", "edx", "ebx", "cc", "memory"
    );
    
    /* Float to int conversion (if supported) */
    volatile float f = 3.14f;
    int int_from_float;
    asm volatile (
        "flds %1\n\t"
        "fistpl %0"
        : "=m"(int_from_float)
        : "m"(f)
        : "st", "memory"
    );
}

/* Main function that runs all tests */
int main(void) {
    int total = 0;
    
    printf("Starting reload pass coverage tests...\n");
    
    /* Run each test multiple times with different values */
    for (int i = 0; i < 3; i++) {
        global_var = 100 + i * 50;
        global_array[i] = i * 20;
        
        test_complex_addressing();
        total += global_var;
        
        test_register_conflicts();
        total += global_array[2];
        
        test_bitfield_operations();
        total += bitfield_global.low16;
        
        test_multiple_outputs();
        total += i;
        
        test_complex_memory_addressing();
        total += global_array[3];
        
        test_mixed_size_operations();
        total += (int)global_var;
    }
    
    printf("Tests completed. Total: %d\n", total);
    return total > 0 ? 0 : 1;
}
