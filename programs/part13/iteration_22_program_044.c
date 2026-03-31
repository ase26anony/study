/* reload_test.c - Comprehensive test to trigger reload.cc push_reload logic */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Function to create complex expressions */
int compute_index(void) {
    static int counter = 0;
    return (counter++ * 37) & 0xFF;
}

/* Complex function to force register pressure */
int complex_calculation(int a, int b, int c) {
    return (a * b) + (c << 2) - (a / (b + 1));
}

/* ===== Test 1: Inline Assembly with Conflicting Constraints ===== */
void test_asm_constraint_conflict(void) {
    int input1 = 12345;
    int input2 = 67890;
    int output1, output2, output3;
    
    /* Force reload by requiring specific hard registers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(output1)      /* Output in any register */
        : "mr"(input1),      /* Memory or register input */
          "mr"(input2)       /* Another memory/register input */
        : "%eax"             /* EAX is clobbered */
    );
    
    /* Early-clobber constraint forcing reload */
    int temp = 100;
    asm volatile (
        "movl %1, %0\n\t"
        "addl $42, %0"
        : "=&r"(output2)     /* Early-clobber - can't overlap inputs */
        : "r"(temp)
        : 
    );
    
    /* Mixed register classes causing conflicts */
    float f1 = 3.14f;
    float f2 = 2.71f;
    float f_result;
    asm volatile (
        "fadds %1, %0"
        : "=t"(f_result)     /* Must be in top of FPU stack */
        : "f"(f1), "0"(f2)   /* Inputs in FPU registers */
        : 
    );
    
    /* Memory constraint with complex addressing */
    int array[100];
    for (int i = 0; i < 100; i++) array[i] = i * 2;
    
    asm volatile (
        "movl (%1), %0"
        : "=r"(output3)
        : "r"(&array[compute_index()])  /* Complex address computation */
        : "memory"
    );
    
    global_result ^= output1 ^ output2 ^ (int)f_result ^ output3;
}

/* ===== Test 2: Built-in Functions with Complex Operands ===== */
void test_builtin_complex_operand(void) {
    unsigned int values[256];
    for (int i = 0; i < 256; i++) values[i] = i * 0x01010101;
    
    /* Builtin with function call in operand */
    int cnt1 = __builtin_popcount(values[compute_index()]);
    
    /* Builtin with arithmetic expression */
    int cnt2 = __builtin_ctz(values[global_counter] + 1);
    
    /* Multiple builtins creating register pressure */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += __builtin_popcount(values[i & 0xFF] ^ global_counter);
    }
    
    /* Atomic builtin with complex address */
    int atomic_var = 0;
    __atomic_fetch_add(&atomic_var, cnt1 + cnt2, __ATOMIC_SEQ_CST);
    
    global_result += cnt1 + cnt2 + sum + atomic_var;
}

/* ===== Test 3: Register Variable Abuse ===== */
void test_register_variable_abuse(void) {
    /* Register variables with specific register constraints */
    register int r1 asm("ebx");
    register int r2 asm("esi");
    register int r3 asm("edi");
    
    r1 = 1000 + global_counter;
    r2 = 2000 + global_counter;
    r3 = 3000 + global_counter;
    
    int result;
    
    /* Force conflict: use register variable in asm requiring different register */
    asm volatile (
        "movl %1, %%ecx\n\t"   /* Move from ebx to ecx */
        "addl %2, %%ecx\n\t"   /* Add esi */
        "subl %3, %%ecx\n\t"   /* Subtract edi */
        "movl %%ecx, %0"
        : "=r"(result)
        : "r"(r1), "r"(r2), "r"(r3)
        : "%ecx"
    );
    
    /* Try to take address (will generate warning but useful for reload) */
    int *ptr;
    asm volatile (
        "leal %1, %0"
        : "=r"(ptr)
        : "r"(r1)
        : 
    );
    
    global_result ^= result + (int)ptr;
}

/* ===== Test 4: Architecture-Specific Secondary Reload Triggers ===== */
void test_secondary_reload_trigger(void) {
    /* This test is architecture-specific */
    
#ifdef __arm__
    /* ARM: System register access often needs secondary reloads */
    unsigned int control_reg;
    asm volatile (
        "mrc p15, 0, %0, c1, c0, 0"
        : "=r"(control_reg)
        : 
        : 
    );
    
    /* NEON to ARM register moves */
    float32x4_t neon_vec = {1.0f, 2.0f, 3.0f, 4.0f};
    float arm_float;
    asm volatile (
        "vmov.f32 %0, %1[0]"
        : "=r"(arm_float)
        : "w"(neon_vec)
        : 
    );
    
    global_result += control_reg + (int)arm_float;
    
#elif defined(__x86_64__)
    /* x86: Control register access */
    unsigned long cr0;
    asm volatile (
        "mov %%cr0, %0"
        : "=r"(cr0)
        : 
        : 
    );
    
    /* x87 FPU to MMX/SSE register moves */
    double x87_val = 3.1415926535;
    long long mmx_val;
    asm volatile (
        "fldl %1\n\t"
        "fistpll %0"
        : "=m"(mmx_val)
        : "m"(x87_val)
        : "st"
    );
    
    global_result += (int)cr0 + (int)mmx_val;
    
#elif defined(__aarch64__)
    /* AArch64: System registers */
    unsigned long tpidr_el0;
    asm volatile (
        "mrs %0, tpidr_el0"
        : "=r"(tpidr_el0)
        : 
        : 
    );
    
    global_result += (int)tpidr_el0;
#endif
}

/* ===== Test 5: Mixed Size Operands and Addressing Modes ===== */
void test_mixed_operands(void) {
    char c1 = 'A' + global_counter;
    short s1 = 1000 + global_counter;
    int i1 = 1000000 + global_counter;
    long long ll1 = 1000000000LL + global_counter;
    
    /* Mixed sizes in same asm statement */
    long long result;
    asm volatile (
        "movsbl %1, %%eax\n\t"
        "movswl %2, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "addl %3, %%eax\n\t"
        "cltq\n\t"
        "addq %4, %%rax\n\t"
        "movq %%rax, %0"
        : "=r"(result)
        : "r"(c1), "r"(s1), "r"(i1), "r"(ll1)
        : "%rax", "%rbx", "%eax", "%ebx"
    );
    
    /* Complex addressing mode requirement */
    struct {
        int a;
        int b[10];
        char c;
    } mystruct;
    
    mystruct.a = 42;
    for (int i = 0; i < 10; i++) mystruct.b[i] = i * 100;
    mystruct.c = 'X';
    
    int struct_result;
    asm volatile (
        "movl 4+%1, %0"  /* Access mystruct.b[0] with offset */
        : "=r"(struct_result)
        : "r"(&mystruct)
        : "memory"
    );
    
    global_result += (int)result + struct_result;
}

/* ===== Test 6: High Register Pressure Scenario ===== */
void test_high_register_pressure(void) {
    /* Many live variables to force spilling and reloading */
    int v1 = global_counter * 1;
    int v2 = global_counter * 2;
    int v3 = global_counter * 3;
    int v4 = global_counter * 4;
    int v5 = global_counter * 5;
    int v6 = global_counter * 6;
    int v7 = global_counter * 7;
    int v8 = global_counter * 8;
    int v9 = global_counter * 9;
    int v10 = global_counter * 10;
    
    /* Chain of operations using all variables */
    int sum = 0;
    sum += complex_calculation(v1, v2, v3);
    sum += complex_calculation(v4, v5, v6);
    sum += complex_calculation(v7, v8, v9);
    sum += v10;
    
    /* Inline asm using multiple variables */
    int asm_result;
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "addl %3, %%eax\n\t"
        "addl %4, %%eax\n\t"
        "addl %5, %%eax\n\t"
        "addl %6, %%eax\n\t"
        "addl %7, %%eax\n\t"
        "addl %8, %%eax\n\t"
        "addl %9, %%eax\n\t"
        "addl %10, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(asm_result)
        : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5),
          "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10)
        : "%eax"
    );
    
    global_result += sum + asm_result;
}

/* ===== Main Test Driver ===== */
int main(void) {
    printf("Starting reload coverage test...\n");
    
    /* Run tests multiple times to increase coverage probability */
    for (int iteration = 0; iteration < 1000; iteration++) {
        global_counter = iteration;
        
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_mixed_operands();
        test_high_register_pressure();
        
        /* Prevent optimization of global_result */
        asm volatile ("" : : "r"(global_result) : "memory");
    }
    
    printf("Final result: %d\n", global_result);
    printf("Test completed.\n");
    
    return 0;
}
