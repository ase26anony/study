/* test_reload.c - Comprehensive test to trigger reload.cc push_reload logic */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_checksum = 0;
volatile int g_counter = 0;

/* Global arrays to force memory operands */
int global_array[100] = {0};
float global_floats[100] = {0.0f};

/* Function to compute index - forces evaluation */
int compute_index(void) {
    static int idx = 0;
    return (idx++ % 50) + 25;
}

/* Test 1: Inline assembly with conflicting constraints */
void test_asm_constraint_conflict(void) {
    int input1 = 12345;
    int input2 = 67890;
    int output1, output2, output3;
    
    /* Force reload by requiring specific registers for output */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)          /* Must be in eax */
        : "mr"(input1 + 42)      /* Memory or register, complex expression */
        : 
    );
    
    /* Early-clobber constraint forces reload */
    asm volatile (
        "addl %2, %0\n\t"
        "movl %0, %1\n\t"
        : "=&r"(output2), "=r"(output3)  /* Early clobber on first output */
        : "r"(input2), "0"(input1)       /* Input constraints */
        : 
    );
    
    /* Mixing register classes */
    asm volatile (
        "movl %1, %%ebx\n\t"
        "movl %%ebx, %0\n\t"
        : "=r"(output1)
        : "m"(global_array[10])  /* Memory operand */
        : "%ebx"                 /* Clobbers ebx */
    );
    
    g_checksum += output1 + output2 + output3;
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    unsigned int x = 0xDEADBEEF;
    unsigned int y = 0xCAFEBABE;
    
    /* Builtin with function call as argument */
    int cnt1 = __builtin_popcount(x + compute_index());
    
    /* Builtin with memory access */
    int cnt2 = __builtin_ctz(global_array[compute_index()] | 1);
    
    /* Multiple builtins in expression */
    int result = __builtin_popcount(x) + 
                 __builtin_clz(y) + 
                 __builtin_parity(x ^ y);
    
    /* Atomic builtin with complex address */
    int old_val = __atomic_fetch_add(&global_array[compute_index()], 5, __ATOMIC_SEQ_CST);
    
    g_checksum += cnt1 + cnt2 + result + old_val;
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(void) {
    /* Register variables with specific register constraints */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    register int r3 asm("edx");
    
    r1 = 100;
    r2 = 200;
    r3 = 300;
    
    /* Force conflict by using in asm with different register requirement */
    int temp;
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0\n\t"
        : "=r"(temp)
        : "r"(r1)        /* r1 is in ebx, but we're moving to eax */
        : "%eax"
    );
    
    /* Take address indirectly (GCC extension with warning) */
    int *ptr;
    asm volatile (
        "leal %1, %0\n\t"
        : "=r"(ptr)
        : "r"(r2)
        : 
    );
    
    /* Use in memory context */
    asm volatile (
        "movl %1, (%0)\n\t"
        : 
        : "r"(&global_array[20]), "r"(r3)
        : "memory"
    );
    
    g_checksum += temp + *ptr;
}

/* Test 4: Architecture-specific secondary reload triggers */
void test_secondary_reload_trigger(void) {
#ifdef __x86_64__
    /* x86_64 specific: Force reloads with 64-bit constraints */
    uint64_t big_val = 0x123456789ABCDEF0ULL;
    uint64_t result;
    
    asm volatile (
        "movq %1, %%rax\n\t"
        "rorq $32, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r"(result)
        : "m"(big_val)      /* Memory operand needing reload */
        : "%rax"
    );
    
    g_checksum += (int)(result >> 32);
    
#elif defined(__arm__)
    /* ARM specific: System register access often needs secondary reloads */
    uint32_t control_reg;
    
    asm volatile (
        "mrc p15, 0, %0, c1, c0, 0\n\t"
        : "=r"(control_reg)
        : 
        : 
    );
    
    /* NEON to ARM register transfer might need secondary reload */
    float32x4_t neon_vec = {1.0f, 2.0f, 3.0f, 4.0f};
    float result_f;
    
    asm volatile (
        "vmov.f32 %0, %1[0]\n\t"
        : "=r"(result_f)
        : "w"(neon_vec)
        : 
    );
    
    g_checksum += control_reg + (int)result_f;
    
#elif defined(__aarch64__)
    /* AArch64: Special register constraints */
    uint64_t tpidr;
    
    asm volatile (
        "mrs %0, tpidr_el0\n\t"
        : "=r"(tpidr)
        : 
        : 
    );
    
    g_checksum += (int)tpidr;
#endif
}

/* Test 5: Mixed modes and sizes */
void test_mixed_modes(void) {
    char c = 'A';
    short s = 1234;
    int i = 56789;
    long long ll = 0x1234567890ABCDEFLL;
    float f = 3.14159f;
    double d = 2.718281828459045;
    
    /* Mixing sizes in asm */
    int result1;
    asm volatile (
        "movsx %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(result1)
        : "r"(c), "r"(i)    /* char and int mixed */
        : "%eax"
    );
    
    /* Float to int conversion might need reload */
    int result2;
    asm volatile (
        "cvttss2si %1, %0\n\t"
        : "=r"(result2)
        : "x"(f)            /* SSE register constraint */
        : 
    );
    
    /* 64-bit value in 32-bit context */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %2, %%edx\n\t"
        "addl $1, %%eax\n\t"
        "adcl $0, %%edx\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(result1)
        : "r"((int)ll), "r"((int)(ll >> 32))
        : "%eax", "%edx"
    );
    
    g_checksum += result1 + result2;
}

/* Test 6: High register pressure */
void test_high_register_pressure(void) {
    /* Many live variables to force spilling and reloading */
    int v1 = g_counter++;
    int v2 = g_counter++;
    int v3 = g_counter++;
    int v4 = g_counter++;
    int v5 = g_counter++;
    int v6 = g_counter++;
    int v7 = g_counter++;
    int v8 = g_counter++;
    int v9 = g_counter++;
    int v10 = g_counter++;
    
    /* Complex expression using all variables */
    int result = (((v1 * v2) + (v3 << v4)) | (v5 & ~v6)) ^ 
                 ((v7 - v8) * (v9 / (v10 ? v10 : 1)));
    
    /* Force all into asm with many operands */
    asm volatile (
        "imull %1, %0\n\t"
        "addl %2, %0\n\t"
        "orl %3, %0\n\t"
        "xorl %4, %0\n\t"
        : "+r"(result)
        : "r"(v1), "r"(v2), "r"(v3), "r"(v4)
        : 
    );
    
    g_checksum += result;
}

/* Main test driver */
int main(void) {
    printf("Starting reload coverage test...\n");
    
    /* Initialize global arrays */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 3;
        global_floats[i] = i * 0.5f;
    }
    
    /* Run tests multiple times to increase coverage chance */
    for (int iteration = 0; iteration < 1000; iteration++) {
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_mixed_modes();
        test_high_register_pressure();
        
        /* Prevent loop unrolling from optimizing away reloads */
        asm volatile ("" : : : "memory");
    }
    
    printf("Final checksum: %d\n", g_checksum);
    printf("Test completed.\n");
    
    return 0;
}
