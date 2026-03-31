/* test_reload_coverage.c - Comprehensive test for GCC reload pass coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_checksum = 0;

/* Complex function to force temporary evaluation */
int compute_index(int base) {
    return (base * 37 + 123) % 256;
}

/* Function to create complex address expressions */
int* get_complex_address(int *base, int offset) {
    return base + (offset >> 2);
}

/* ===== Test 1: Inline Assembly with Conflicting Constraints ===== */
void test_asm_constraint_conflict(void) {
    int input1 = 12345;
    int input2 = 67890;
    int output1, output2, output3;
    
    /* Force reload by requiring specific register for output */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)          /* Must be in eax */
        : "mr"(input1 + 42)      /* Memory or register, but not necessarily eax */
        : 
    );
    
    /* Early-clobber constraint forcing reload */
    asm volatile (
        "addl %2, %0\n\t"
        "subl %1, %0\n\t"
        : "=&r"(output2)         /* Early clobber */
        : "r"(input1), "rm"(input2)
        : "cc"
    );
    
    /* Multiple constraints that conflict */
    asm volatile (
        "imull %1, %0\n\t"
        : "=a"(output3)          /* Output in eax */
        : "rm"(input2), "0"(output1)  /* Input in eax or memory, but output already uses eax */
        : "cc", "edx"
    );
    
    global_checksum += output1 + output2 + output3;
}

/* ===== Test 2: Built-in Functions with Complex Operands ===== */
void test_builtin_complex_operand(void) {
    static int data_array[256] = {0};
    int i, result;
    
    /* Initialize array */
    for (i = 0; i < 256; i++) {
        data_array[i] = i * 3;
    }
    
    /* Built-in with complex addressing */
    result = __builtin_popcount(data_array[compute_index(global_counter)]);
    global_checksum += result;
    
    /* Built-in with function call as argument */
    result = __builtin_ctz(compute_index(global_counter) | 1);
    global_checksum += result;
    
    /* Atomic built-in with complex address */
    int *ptr = get_complex_address(data_array, global_counter);
    __atomic_fetch_add(ptr, 1, __ATOMIC_RELAXED);
    
    /* Math built-in with computation */
    double x = global_counter * 0.5;
    double y = __builtin_sqrt(x + 1.0);
    global_checksum += (int)y;
}

/* ===== Test 3: Register Variable Abuse ===== */
void test_register_variable_abuse(void) {
    /* Declare register variables (x86-specific registers) */
    register int r1 asm("ebx");
    register int r2 asm("esi");
    register int r3 asm("edi");
    
    r1 = global_counter + 1;
    r2 = global_counter + 2;
    r3 = global_counter + 3;
    
    int temp;
    
    /* Force conflict: use register variable in asm requiring different register */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(temp)             /* Requires eax */
        : "r"(r1)                /* But r1 is in ebx */
        :
    );
    
    /* Another conflict */
    asm volatile (
        "leal (%1, %2, 2), %0\n\t"
        : "=r"(temp)
        : "r"(r2), "r"(r3)
        :
    );
    
    global_checksum += temp + r1 + r2 + r3;
}

/* ===== Test 4: Architecture-Specific Secondary Reload Triggers ===== */
void test_secondary_reload_trigger(void) {
    /* This test is architecture-specific */
    
#if defined(__arm__) || defined(__aarch64__)
    /* ARM: System register access often requires secondary reloads */
    unsigned int control_reg;
    
    asm volatile (
        "mrs %0, cpsr\n\t"
        : "=r"(control_reg)
        :
        : 
    );
    
    /* Modify and write back */
    control_reg &= ~0x1F;
    control_reg |= 0x13;  /* Supervisor mode */
    
    asm volatile (
        "msr cpsr_c, %0\n\t"
        :
        : "r"(control_reg)
        : "cc"
    );
    
    global_checksum += control_reg;
    
#elif defined(__x86_64__) || defined(__i386__)
    /* x86: Control register access requires secondary reloads */
    unsigned long long cr0;
    
    asm volatile (
        "mov %%cr0, %0\n\t"
        : "=r"(cr0)
        :
        :
    );
    
    /* Force floating point to integer moves which may need secondary reloads */
    double fp_value = 3.14159 * global_counter;
    long long int_result;
    
    asm volatile (
        "movq %1, %0\n\t"
        : "=r"(int_result)
        : "x"(fp_value)          /* XMM register */
        :
    );
    
    global_checksum += (int)(cr0 & 0xFFFFFFFF) + (int)(int_result & 0xFFFFFFFF);
    
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    /* PowerPC: Special purpose register access */
    unsigned int msr;
    
    asm volatile (
        "mfmsr %0\n\t"
        : "=r"(msr)
        :
        :
    );
    
    global_checksum += msr;
#endif
    
    /* Generic test: mixing operand sizes */
    char c = 'A' + (global_counter % 26);
    short s = 1000 + global_counter;
    int i = 1000000 + global_counter;
    long long ll = 1000000000LL + global_counter;
    
    /* Force reloads by mixing sizes in asm */
    long long result;
    asm volatile (
        "movsbl %1, %k0\n\t"     /* Sign extend byte to long */
        "addw %2, %w0\n\t"       /* Add word */
        "addl %3, %k0\n\t"       /* Add doubleword */
        "addq %4, %0\n\t"        /* Add quadword */
        : "=r"(result)
        : "r"(c), "r"(s), "r"(i), "r"(ll)
        : "cc"
    );
    
    global_checksum += (int)(result & 0xFFFFFFFF);
}

/* ===== Test 5: Memory Constraints and Addressing Modes ===== */
void test_memory_addressing_conflicts(void) {
    struct {
        int a;
        int b[10];
        int c;
    } data_struct;
    
    int array[100];
    int index = global_counter % 100;
    
    /* Initialize */
    for (int i = 0; i < 100; i++) {
        array[i] = i * 7;
    }
    data_struct.a = 111;
    data_struct.c = 222;
    for (int i = 0; i < 10; i++) {
        data_struct.b[i] = i * 11;
    }
    
    int result1, result2, result3;
    
    /* Complex addressing that may not satisfy simple constraints */
    asm volatile (
        "movl (%1, %2, 4), %0\n\t"
        : "=r"(result1)
        : "r"(array), "r"(index)
        : "memory"
    );
    
    /* Structure member with offset */
    asm volatile (
        "movl 4(%1), %0\n\t"     /* Access data_struct.b[0] */
        : "=r"(result2)
        : "r"(&data_struct)
        : "memory"
    );
    
    /* Multiple memory constraints */
    asm volatile (
        "movl %1, %0\n\t"
        "addl %2, %0\n\t"
        : "=r"(result3)
        : "m"(array[index]), "m"(data_struct.b[3])
        : 
    );
    
    global_checksum += result1 + result2 + result3;
}

/* ===== Test 6: Floating Point Constraints ===== */
void test_floating_point_reloads(void) {
    double a = 1.5 + global_counter;
    double b = 2.7 + global_counter;
    double c = 3.9 + global_counter;
    double results[4];
    
    /* Force floating point register pressure */
    asm volatile (
        "addsd %1, %0\n\t"
        "mulsd %2, %0\n\t"
        "subsd %3, %0\n\t"
        : "=x"(results[0])
        : "x"(a), "x"(b), "x"(c)
    );
    
    /* Mix x87 and SSE constraints on x86 */
#if defined(__i386__) && !defined(__x86_64__)
    /* 32-bit x86 with x87 FPU */
    double x87_result;
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "faddp\n\t"
        "fstpl %0\n\t"
        : "=m"(x87_result)
        : "m"(a), "m"(b)
        : "st", "st(1)"
    );
    global_checksum += (int)x87_result;
#endif
    
    /* Vector constraints that may need reloads */
    typedef double v2df __attribute__((vector_size(16)));
    v2df v1 = {a, b};
    v2df v2 = {b, c};
    v2df v3;
    
    asm volatile (
        "addpd %1, %0\n\t"
        "mulpd %2, %0\n\t"
        : "=x"(v3)
        : "x"(v1), "x"(v2)
    );
    
    global_checksum += (int)results[0] + (int)v3[0] + (int)v3[1];
}

/* ===== Main Test Driver ===== */
int main(void) {
    printf("Starting reload pass coverage test...\n");
    
    /* Run tests multiple times to increase coverage probability */
    for (int iteration = 0; iteration < 1000; iteration++) {
        global_counter = iteration;
        
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_memory_addressing_conflicts();
        test_floating_point_reloads();
        
        /* Prevent loop optimization */
        asm volatile ("" : : : "memory");
    }
    
    printf("Final checksum: %d\n", global_checksum);
    printf("Test completed.\n");
    
    return 0;
}
