/* test_reload_coverage.c - Comprehensive test for GCC reload pass coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile global to prevent dead code elimination */
volatile int global_checksum = 0;

/* Complex function to force expression evaluation */
static int compute_index(void) {
    static int counter = 0;
    return (counter++ * 37) & 0xFF;
}

/* Function returning address with complex computation */
static int* get_complex_address(int *base, int offset) {
    return base + (offset * 2) / 3;
}

/* ===== Test 1: Inline Assembly with Constraint Conflicts ===== */
void test_asm_constraint_conflict(void) {
    int input1 = 12345;
    int input2 = 67890;
    int output1, output2, output3;
    
    /* Force reload by requiring specific register for output */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)        /* Must be in eax */
        : "mr"(input1 + 42)    /* Memory or register with complex expression */
        : 
    );
    
    /* Early-clobber constraint forcing reload */
    asm volatile (
        "addl %2, %0\n\t"
        "movl %0, %1\n\t"
        : "=&r"(output2), "=r"(output3)  /* & = early clobber */
        : "r"(input2), "0"(output1)      /* Input tied to output0 */
        : 
    );
    
    /* Mixed register classes causing conflicts */
    register int r1 asm("ebx") = 100;
    register int r2 asm("ecx") = 200;
    
    asm volatile (
        "xchgl %0, %1\n\t"
        : "+r"(r1), "+r"(r2)
        : 
        : 
    );
    
    /* Update checksum */
    global_checksum += output1 + output2 + output3 + r1 + r2;
}

/* ===== Test 2: Built-in Functions with Complex Operands ===== */
void test_builtin_complex_operand(void) {
    static int global_array[256];
    int i, result;
    
    /* Initialize array */
    for (i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    /* Built-in with function call as argument */
    result = __builtin_popcount(global_array[compute_index()] + 0x1234);
    global_checksum += result;
    
    /* Built-in with complex address computation */
    int idx = compute_index();
    result = __builtin_ctz(*(get_complex_address(global_array, idx)));
    global_checksum += result;
    
    /* Atomic built-in with complex address */
    int atomic_var = 1000;
    result = __atomic_fetch_add(get_complex_address(&atomic_var, 5), 42, __ATOMIC_SEQ_CST);
    global_checksum += result;
}

/* ===== Test 3: Register Variable Abuse ===== */
void test_register_variable_abuse(void) {
    /* Declare register variables */
    register int reg1 asm("esi") = 111;
    register int reg2 asm("edi") = 222;
    register double freg asm("xmm0") = 3.14159;
    
    int temp;
    
    /* Force conflict by using register variable in asm requiring different register */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(temp)
        : "r"(reg1)        /* reg1 is in esi, but we need it elsewhere */
        : "%eax"
    );
    
    /* Mix register classes */
    asm volatile (
        "cvtsi2sd %1, %0\n\t"
        : "+x"(freg)       /* xmm register */
        : "r"(reg2)        /* general purpose register */
        : 
    );
    
    global_checksum += temp + (int)freg;
}

/* ===== Test 4: Architecture-Specific Secondary Reload Triggers ===== */
void test_secondary_reload_trigger(void) {
    /* This test is architecture-specific */
    
#if defined(__arm__) || defined(__aarch64__)
    /* ARM: System register access often requires secondary reloads */
    uint32_t control_reg;
    
    asm volatile (
        "mrs %0, cpsr\n\t"
        : "=r"(control_reg)
        : 
        : 
    );
    
    /* NEON to ARM register transfer may need secondary reload */
    float32x4_t neon_vec = {1.0f, 2.0f, 3.0f, 4.0f};
    float arm_float;
    
    asm volatile (
        "vmov.f32 %0, %1[0]\n\t"
        : "=r"(arm_float)
        : "w"(neon_vec)    /* NEON register constraint */
        : 
    );
    
    global_checksum += control_reg + (int)arm_float;
    
#elif defined(__x86_64__) || defined(__i386__)
    /* x86: Control register access requires secondary reloads */
    uint64_t cr0_value;
    
    asm volatile (
        "mov %%cr0, %0\n\t"
        : "=r"(cr0_value)
        : 
        : 
    );
    
    /* x87 FPU stack manipulation */
    double x87_val1 = 1.234, x87_val2 = 5.678;
    double x87_result;
    
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "faddp\n\t"
        "fstpl %0\n\t"
        : "=m"(x87_result)
        : "m"(x87_val1), "m"(x87_val2)
        : 
    );
    
    global_checksum += (int)cr0_value + (int)x87_result;
    
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    /* PowerPC: SPR access requires secondary reloads */
    unsigned long spr_value;
    
    asm volatile (
        "mfspr %0, 0x10\n\t"  /* SPRG0 */
        : "=r"(spr_value)
        : 
        : 
    );
    
    global_checksum += spr_value;
#endif
}

/* ===== Test 5: Mixed Operand Sizes and Addressing Modes ===== */
void test_mixed_operands(void) {
    char small = 0x7F;
    short medium = 0x7FFF;
    long long large = 0x7FFFFFFFFFFFFFFFLL;
    double fp = 123.456;
    
    /* Mixed sizes in same asm statement */
    long long result;
    
    asm volatile (
        "movsbl %1, %%eax\n\t"
        "movswl %2, %%edx\n\t"
        "addl %%edx, %%eax\n\t"
        "cltq\n\t"
        "addq %3, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r"(result)
        : "r"(small), "r"(medium), "r"(large)
        : "%rax", "%rdx", "%eax", "%edx"
    );
    
    /* Memory constraint with complex addressing */
    struct {
        int a;
        int b[10];
        double c;
    } complex_struct;
    
    complex_struct.a = 100;
    for (int i = 0; i < 10; i++) {
        complex_struct.b[i] = i * 10;
    }
    complex_struct.c = 3.14159;
    
    int struct_result;
    
    asm volatile (
        "movl %1, %0\n\t"
        : "=r"(struct_result)
        : "m"(complex_struct.b[compute_index() % 10])  /* Complex memory address */
        : 
    );
    
    global_checksum += (int)result + struct_result + (int)fp;
}

/* ===== Test 6: High Register Pressure ===== */
void test_high_register_pressure(void) {
    /* Many live variables to increase register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    
    /* Chain of operations forcing spills and reloads */
    v1 = v1 + v2;
    v3 = v3 * v4;
    v5 = v5 - v6;
    v7 = v7 / v8;
    v9 = v9 ^ v10;
    v11 = v11 | v12;
    v13 = v13 & v14;
    
    /* Use all in complex expression */
    int complex_result = 
        (v1 * v3) + (v5 * v7) - (v9 * v11) + (v13 * v15) +
        (v2 * v4) - (v6 * v8) + (v10 * v12) - (v14 * v1);
    
    /* Force memory operations */
    asm volatile (
        "movl %1, %%eax\n\t"
        "imull %2, %%eax\n\t"
        "addl %%eax, %0\n\t"
        : "+r"(complex_result)
        : "r"(v2), "r"(v3)
        : "%eax"
    );
    
    global_checksum += complex_result;
}

/* ===== Main Test Driver ===== */
int main(void) {
    int i;
    
    printf("Starting reload pass coverage test...\n");
    
    /* Run tests multiple times to increase coverage probability */
    for (i = 0; i < 1000; i++) {
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_mixed_operands();
        test_high_register_pressure();
        
        /* Prevent loop optimization */
        if (i % 100 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    printf("Final checksum: %d\n", global_checksum);
    printf("Test completed.\n");
    
    return 0;
}
