/* test_reload_coverage.c - Comprehensive test for GCC reload pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* Volatile global to prevent optimization */
volatile int g_checksum = 0;

/* Complex function to force expression evaluation */
static int compute_index(void) {
    static int counter = 0;
    return (counter++ * 37) & 0xFF;
}

/* Global arrays to force memory operands */
int global_array[256] = {0};
float global_float_array[256] = {0.0f};
double global_double_array[256] = {0.0};

/* ===== Test 1: Inline Assembly with Conflicting Constraints ===== */
void test_asm_constraint_conflict(void) {
    int input1 = 12345;
    int input2 = 67890;
    int output1, output2, output3;
    
    /* Force reload by requiring specific register for output */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)        /* Must be in eax */
        : "mr"(input1 + 42)    /* Memory or register */
        : 
    );
    
    /* Early-clobber constraint forces reload */
    asm volatile (
        "addl %2, %0\n\t"
        "subl %1, %0\n\t"
        : "=&r"(output2)       /* Early clobber */
        : "r"(input1), "r"(input2)
        : 
    );
    
    /* Mixed register classes */
    asm volatile (
        "movl %1, %0\n\t"
        : "=r"(output3)
        : "q"(input2)          /* Must be in a, b, c, or d register */
        : 
    );
    
    g_checksum += output1 + output2 + output3;
}

/* ===== Test 2: Complex Built-in Function Operands ===== */
void test_builtin_complex_operand(void) {
    /* Force reload for builtin argument */
    int idx = compute_index();
    int cnt = __builtin_popcount(global_array[idx] + idx * 3);
    
    /* Complex address computation for atomic */
    long long *ptr = (long long*)&global_array[0];
    __atomic_fetch_add(ptr + idx, 1, __ATOMIC_RELAXED);
    
    /* Math builtin with complex argument */
    double x = __builtin_sqrt(global_double_array[idx] + sin(idx * 0.1));
    
    g_checksum += cnt + (int)x + (int)global_array[idx];
}

/* ===== Test 3: Register Variable Abuse ===== */
void test_register_variable_abuse(void) {
    /* Declare register variables */
    register int r1 asm("ebx");
    register int r2 asm("esi");
    register int r3 asm("edi");
    
    r1 = 100;
    r2 = 200;
    r3 = 300;
    
    int result;
    
    /* Force conflict: r1 is in ebx, but we need it in eax */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(result)        /* Must be in eax */
        : "r"(r1)             /* But r1 is tied to ebx */
        : 
    );
    
    /* Use register variable in memory context */
    int *ptr = &global_array[0];
    ptr[r2] = r3;  /* May force reload if r2 needs addressing mode */
    
    g_checksum += result + r1 + r2 + r3;
}

/* ===== Test 4: Architecture-Specific Secondary Reloads ===== */
void test_secondary_reload_trigger(void) {
    /* Test designed to trigger secondary reloads */
    
    #if defined(__arm__) || defined(__aarch64__)
    /* ARM-specific: System register access often needs secondary reload */
    uint32_t control_reg;
    asm volatile (
        "mrs %0, cpsr\n\t"
        : "=r"(control_reg)
        : 
        : 
    );
    
    /* NEON to ARM register move may need secondary reload */
    float32x4_t neon_vec = {1.0f, 2.0f, 3.0f, 4.0f};
    float result;
    asm volatile (
        "vmov.f32 %0, %1[0]\n\t"
        : "=r"(result)
        : "w"(neon_vec)
        : 
    );
    
    g_checksum += control_reg + (int)result;
    
    #elif defined(__x86_64__) || defined(__i386__)
    /* x86-specific: Control register access */
    uint32_t cr0;
    asm volatile (
        "mov %%cr0, %0\n\t"
        : "=r"(cr0)
        : 
        : 
    );
    
    /* x87 FPU register constraints */
    double x = 3.14159;
    double y;
    asm volatile (
        "fldl %1\n\t"
        "fstpl %0\n\t"
        : "=m"(y)
        : "m"(x)
        : "st", "st(1)"
    );
    
    g_checksum += cr0 + (int)y;
    
    #else
    /* Generic fallback: Use memory constraints with register pressure */
    int a = 1000, b = 2000, c = 3000;
    int r1, r2, r3, r4, r5, r6;
    
    /* Create register pressure */
    asm volatile (
        "movl %1, %0\n\t"
        : "=r"(r1) : "m"(a) : );
    asm volatile (
        "movl %1, %0\n\t"
        : "=r"(r2) : "m"(b) : );
    asm volatile (
        "movl %1, %0\n\t"
        : "=r"(r3) : "m"(c) : );
    
    /* Force spill/reload with memory constraint */
    asm volatile (
        "addl %2, %0\n\t"
        "addl %1, %0\n\t"
        : "=r"(r4)
        : "m"(global_array[10]), "m"(global_array[20])
        : 
    );
    
    g_checksum += r1 + r2 + r3 + r4;
    #endif
}

/* ===== Test 5: Mixed Operand Sizes and Addressing Modes ===== */
void test_mixed_operands(void) {
    char c = 'A';
    short s = 1234;
    int i = 56789;
    long long ll = 1234567890123LL;
    
    int result1, result2;
    
    /* Mixed sizes in same asm */
    asm volatile (
        "movsbl %1, %0\n\t"
        : "=r"(result1)
        : "m"(c)
        : 
    );
    
    /* Complex addressing mode */
    asm volatile (
        "movl %1, %0\n\t"
        : "=r"(result2)
        : "m"(global_array[compute_index() * 2 + 1])
        : 
    );
    
    /* Force 64-bit operand on 32-bit arch */
    #if defined(__i386__)
    long long ll_result;
    asm volatile (
        "movl %%eax, %0\n\t"
        "movl %%edx, %0+4\n\t"
        : "=m"(ll_result)
        : "A"(ll)  /* edx:eax pair */
        : 
    );
    g_checksum += (int)ll_result;
    #endif
    
    g_checksum += result1 + result2 + s + i;
}

/* ===== Test 6: Volatile Memory Accesses ===== */
void test_volatile_memory(void) {
    volatile int* volatile_ptr = &global_array[0];
    volatile float* volatile_float_ptr = &global_float_array[0];
    
    /* These volatile accesses create memory operands that may need reloads */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += volatile_ptr[i];
        volatile_float_ptr[i] = sum * 0.5f;
    }
    
    /* Force reload between volatile accesses */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m"(*volatile_ptr)
        : "m"(*volatile_ptr)
        : "eax", "memory"
    );
    
    g_checksum += sum;
}

/* ===== Main Test Driver ===== */
int main(void) {
    /* Initialize global arrays */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
        global_float_array[i] = i * 0.7f;
        global_double_array[i] = i * 1.3;
    }
    
    printf("Starting reload coverage tests...\n");
    
    /* Run tests multiple times to increase coverage probability */
    for (int iteration = 0; iteration < 1000; iteration++) {
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_mixed_operands();
        test_volatile_memory();
        
        /* Vary inputs slightly each iteration */
        global_array[iteration % 256] = iteration;
    }
    
    printf("Final checksum: %d\n", g_checksum);
    printf("Tests completed.\n");
    
    return 0;
}
