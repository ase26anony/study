/* test_reload.c - Comprehensive test to trigger reload.cc push_reload logic */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_checksum = 0;

/* Function to create complex expressions */
int compute_index(void) {
    static int counter = 0;
    return (counter++ * 37 + 123) & 0xFF;
}

/* Complex function to force register pressure */
int complex_function(int a, int b, int c) {
    return (a * b + c * 7 - 13) ^ 0x55AA55AA;
}

/* ===== Test 1: Inline Assembly with Conflicting Constraints ===== */
void test_asm_constraint_conflict(void) {
    int input1 = 12345;
    int input2 = 67890;
    int output1, output2, output3;
    
    /* Force reload by requiring specific registers */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)      /* Must be in eax */
        : "mr"(input1 + global_counter)  /* Memory or register */
        : 
    );
    
    /* Early clobber to force reload */
    asm volatile (
        "addl %2, %0\n\t"
        "movl %0, %1\n\t"
        : "=&r"(output2), "=r"(output3)  /* & = early clobber */
        : "r"(input2), "0"(output1)
        : 
    );
    
    /* Mixed register classes */
    register int r1 asm("ebx") = 111;
    register int r2 asm("ecx") = 222;
    
    asm volatile (
        "xchgl %0, %1\n\t"
        : "+r"(r1), "+r"(r2)
        : 
    );
    
    global_checksum += output1 + output2 + output3 + r1 + r2;
}

/* ===== Test 2: Built-in Functions with Complex Operands ===== */
void test_builtin_complex_operand(void) {
    unsigned int values[256];
    for (int i = 0; i < 256; i++) {
        values[i] = i * 0x01010101;
    }
    
    /* Complex operand requiring temporary */
    int cnt1 = __builtin_popcount(values[compute_index()] + global_counter);
    
    /* Nested built-in with complex address */
    int cnt2 = __builtin_ctz(__builtin_bswap32(values[compute_index() % 256]));
    
    /* Atomic operation with complex address */
    int atomic_var = 1000;
    __atomic_fetch_add(&atomic_var, values[compute_index() % 64], __ATOMIC_SEQ_CST);
    
    /* Math built-in with function call */
    double x = __builtin_sqrt((double)compute_index() * 2.5 + 1.0);
    
    global_checksum += cnt1 + cnt2 + atomic_var + (int)x;
}

/* ===== Test 3: Register Variable Abuse ===== */
void test_register_variable_abuse(void) {
    /* Declare register variables with specific registers */
    register int reg_a asm("eax");
    register int reg_b asm("ebx");
    register int reg_c asm("ecx");
    register int reg_d asm("edx");
    
    reg_a = 100 + global_counter;
    reg_b = 200 + global_counter;
    reg_c = 300 + global_counter;
    reg_d = 400 + global_counter;
    
    /* Force conflicts by using in different contexts */
    int temp;
    
    /* Try to use register variable where it doesn't fit constraint */
    asm volatile (
        "imull %1, %0\n\t"
        : "+a"(reg_a)        /* Must be eax for imul */
        : "r"(reg_b)         /* Any register */
        : 
    );
    
    /* Mix with memory operations */
    asm volatile (
        "movl %1, %0\n\t"
        "addl $42, %0\n\t"
        : "=r"(temp)
        : "m"(reg_c)         /* Force reg_c into memory */
        : 
    );
    
    /* Complex expression involving register variables */
    reg_d = complex_function(reg_a, reg_b, reg_c);
    
    global_checksum += reg_a + reg_b + reg_c + reg_d + temp;
}

/* ===== Test 4: Architecture-Specific Secondary Reload Triggers ===== */
void test_secondary_reload_trigger(void) {
    /* This test is architecture-specific */
    
#if defined(__arm__) || defined(__aarch64__)
    /* ARM: System register access often needs secondary reloads */
    uint32_t control_reg;
    
    asm volatile (
        "mrs %0, cpsr\n\t"
        : "=r"(control_reg)
        : 
        : 
    );
    
    /* NEON/VFP to general purpose register moves */
    float f1 = 3.14159f;
    int int_result;
    
    asm volatile (
        "vmov %0, %1\n\t"
        : "=r"(int_result)
        : "w"(f1)            /* Floating-point register */
        : 
    );
    
    global_checksum += control_reg + int_result;
    
#elif defined(__x86_64__) || defined(__i386__)
    /* x86: Control register access */
    uint32_t cr0_value;
    
    asm volatile (
        "mov %%cr0, %0\n\t"
        : "=r"(cr0_value)
        : 
        : 
    );
    
    /* x87 floating point stack manipulation */
    double d1 = 2.71828;
    double d2 = 1.41421;
    double result;
    
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "faddp\n\t"
        "fstpl %0\n\t"
        : "=m"(result)
        : "m"(d1), "m"(d2)
        : 
    );
    
    /* MMX/SSE register constraints */
    int mmx_data[2] __attribute__((aligned(8))) = {0x12345678, 0x9ABCDEF0};
    
    asm volatile (
        "movq %1, %%mm0\n\t"
        "movq %%mm0, %0\n\t"
        : "=m"(mmx_data)
        : "m"(mmx_data)
        : "%mm0"
    );
    
    global_checksum += cr0_value + (int)result + mmx_data[0];
    
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    /* PowerPC: Special purpose register access */
    uint32_t msr_value;
    
    asm volatile (
        "mfmsr %0\n\t"
        : "=r"(msr_value)
        : 
        : 
    );
    
    global_checksum += msr_value;
#endif
}

/* ===== Test 5: Mixed Operand Sizes and Addressing Modes ===== */
void test_mixed_operand_sizes(void) {
    char c1 = 'A';
    short s1 = 1234;
    int i1 = 56789;
    long long ll1 = 0x123456789ABCDEF0LL;
    
    /* Mixed sizes in same asm */
    long long result;
    
    asm volatile (
        "movsbl %1, %%eax\n\t"
        "movswl %2, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "addl %3, %%eax\n\t"
        "cltq\n\t"
        "addq %4, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=m"(result)
        : "m"(c1), "m"(s1), "r"(i1), "r"(ll1)
        : "%rax", "%rbx", "%eax", "%ebx"
    );
    
    /* Different addressing modes */
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i * 3;
    }
    
    int index = compute_index() % 100;
    int base = 1000;
    
    /* Complex addressing that might need reload */
    asm volatile (
        "movl (%1, %2, 4), %%eax\n\t"
        "addl %3, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m"(array[index])
        : "r"(array), "r"(index), "r"(base)
        : "%eax"
    );
    
    global_checksum += (int)result + array[index % 100];
}

/* ===== Test 6: Register Pressure and Spilling ===== */
void test_register_pressure(void) {
    /* Create many live variables to force spilling */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    
    /* Use all in complex expressions */
    v1 = complex_function(v1, v2, v3);
    v2 = complex_function(v4, v5, v6);
    v3 = complex_function(v7, v8, v9);
    v4 = complex_function(v10, v11, v12);
    v5 = complex_function(v13, v14, v15);
    
    /* Chain computations to keep variables live */
    for (int i = 0; i < 10; i++) {
        v1 = v1 * 3 + v2;
        v2 = v2 * 5 + v3;
        v3 = v3 * 7 + v4;
        v4 = v4 * 11 + v5;
        v5 = v5 * 13 + v1;
    }
    
    /* Force into asm with many operands */
    asm volatile (
        "addl %1, %0\n\t"
        "addl %2, %0\n\t"
        "addl %3, %0\n\t"
        "addl %4, %0\n\t"
        "addl %5, %0\n\t"
        : "+r"(v1)
        : "r"(v2), "r"(v3), "r"(v4), "r"(v5), "r"(global_counter)
        : 
    );
    
    global_checksum += v1 + v2 + v3 + v4 + v5;
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
        test_mixed_operand_sizes();
        test_register_pressure();
        
        /* Prevent loop optimization */
        if (iteration % 100 == 0) {
            printf("Iteration %d, checksum = %d\n", iteration, global_checksum);
        }
    }
    
    printf("Final checksum: %d\n", global_checksum);
    printf("Test completed.\n");
    
    return 0;
}
