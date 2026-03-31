/* test_reload.c - Comprehensive test to trigger GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_checksum = 0;
volatile int g_counter = 0;

/* Global variables to force memory operands */
int global_array[100] = {0};
float global_floats[50] = {0.0f};
double global_doubles[25] = {0.0};

/* Function to create complex expressions */
int compute_index(void) {
    static int idx = 0;
    return (idx++ * 7 + 3) % 50;
}

float compute_float(void) {
    static float f = 0.5f;
    f = f * 1.1f + 0.3f;
    return f;
}

/* Test 1: Inline assembly with conflicting constraints */
void test_asm_constraint_conflict(void) {
    int input1 = 42;
    int input2 = 73;
    int output1, output2;
    int temp;
    
    /* Force reload by requiring specific register for output */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)      /* Must be in eax */
        : "mr"(input1 + g_counter)  /* Memory or register */
        : 
    );
    
    /* Early-clobber constraint forcing reload */
    asm volatile (
        "addl %2, %0\n\t"
        "movl %0, %1\n\t"
        : "=&r"(temp), "=r"(output2)  /* & = early clobber */
        : "r"(input2), "0"(input1)
        : 
    );
    
    /* Mixed size constraints */
    short s_input = 100;
    long long ll_output;
    asm volatile (
        "movswl %1, %%eax\n\t"
        "cltq\n\t"
        "movq %%rax, %0\n\t"
        : "=r"(ll_output)
        : "r"(s_input)
        : "%rax"
    );
    
    g_checksum += output1 + output2 + (int)ll_output;
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    /* Complex expression as builtin argument */
    int popcnt = __builtin_popcount(
        global_array[compute_index()] + g_counter
    );
    
    /* Multiple complex arguments */
    int ctz = __builtin_ctz(
        (unsigned int)(popcnt * 17 + 1)
    );
    
    /* Atomic builtin with complex address */
    int old_val = __atomic_fetch_add(
        &global_array[compute_index() % 20],
        5,
        __ATOMIC_SEQ_CST
    );
    
    /* Math builtin with function call */
    double d = __builtin_sqrt(
        global_doubles[compute_index() % 10] + 1.0
    );
    
    g_checksum += popcnt + ctz + old_val + (int)d;
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(void) {
    /* Declare register variables */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    register int r3 asm("edx");
    
    r1 = 100 + g_counter;
    r2 = 200 + g_counter;
    r3 = 300 + g_counter;
    
    int result1, result2;
    
    /* Force conflict: require specific register different from variable's */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(result1)
        : "r"(r1), "r"(r2)
        : "%eax"
    );
    
    /* Use register variable in memory context */
    asm volatile (
        "movl %1, (%2)\n\t"
        : 
        : "r"(r3), "r"(&global_array[10])
        : "memory"
    );
    
    /* Try to take address (GCC extension with warning) */
    int *ptr;
    asm volatile (
        "leal %1, %0\n\t"
        : "=r"(ptr)
        : "r"(r1)
    );
    
    g_checksum += result1 + global_array[10] + (int)(intptr_t)ptr;
}

/* Test 4: Architecture-specific secondary reload triggers */
void test_secondary_reload_trigger(void) {
#ifdef __x86_64__
    /* x86-64 specific: MMX/SSE constraints */
    double d1 = 3.14159 + g_counter;
    double d2 = 2.71828 + g_counter;
    double d3;
    
    asm volatile (
        "movsd %1, %%xmm0\n\t"
        "addsd %2, %%xmm0\n\t"
        "movsd %%xmm0, %0\n\t"
        : "=x"(d3)          /* SSE register constraint */
        : "xm"(d1), "xm"(d2) /* Memory or SSE register */
        : "%xmm0"
    );
    
    /* x87 floating point stack */
    long double ld1 = 1.23456789L;
    long double ld2;
    asm volatile (
        "fldt %1\n\t"
        "fstpt %0\n\t"
        : "=m"(ld2)
        : "m"(ld1)
        : "st", "st(1)"
    );
    
    g_checksum += (int)d3 + (int)ld2;
    
#elif defined(__arm__)
    /* ARM specific: NEON and system registers */
    int arm_val = 0x12345678;
    int arm_result;
    
    /* System register access (may require secondary reload) */
    asm volatile (
        "mrs %0, cpsr\n\t"
        "orr %0, %0, %1\n\t"
        "msr cpsr_f, %0\n\t"
        : "=r"(arm_result)
        : "r"(arm_val)
        : "cc"
    );
    
    g_checksum += arm_result;
    
#elif defined(__aarch64__)
    /* AArch64 specific */
    uint64_t a64_val = 0xABCDEF0123456789ULL;
    uint64_t a64_result;
    
    asm volatile (
        "mov %0, %1\n\t"
        "rev %0, %0\n\t"
        : "=r"(a64_result)
        : "r"(a64_val)
    );
    
    g_checksum += (int)a64_result;
#endif
}

/* Test 5: Mixed mode and addressing conflicts */
void test_mixed_mode_addressing(void) {
    char c_array[100];
    long long ll_array[50];
    
    /* Different sized operands in same asm */
    char c_val = 'A' + g_counter;
    long long ll_val = 0x123456789ABCDEF0LL;
    long long ll_result;
    
    asm volatile (
        "movsbl %1, %%eax\n\t"
        "addq %2, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r"(ll_result)
        : "r"(c_val), "r"(ll_val)
        : "%rax"
    );
    
    /* Complex addressing modes */
    int idx1 = compute_index() % 50;
    int idx2 = compute_index() % 25;
    
    asm volatile (
        "movl (%1, %2, 4), %%eax\n\t"
        "addl (%3, %4, 8), %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(g_counter)
        : "r"(global_array), "r"(idx1),
          "r"(ll_array), "r"(idx2)
        : "%eax", "memory"
    );
    
    g_checksum += (int)ll_result + g_counter;
}

/* Test 6: High register pressure to force spills */
void test_high_register_pressure(void) {
    /* Many live variables to increase register pressure */
    int v1 = g_counter + 1;
    int v2 = g_counter + 2;
    int v3 = g_counter + 3;
    int v4 = g_counter + 4;
    int v5 = g_counter + 5;
    int v6 = g_counter + 6;
    int v7 = g_counter + 7;
    int v8 = g_counter + 8;
    int v9 = g_counter + 9;
    int v10 = g_counter + 10;
    
    /* Chain of operations forcing temporary reloads */
    asm volatile (
        "addl %1, %0\n\t"
        "addl %2, %0\n\t"
        "addl %3, %0\n\t"
        "addl %4, %0\n\t"
        "addl %5, %0\n\t"
        : "+r"(v1)
        : "r"(v2), "r"(v3), "r"(v4), "r"(v5), "r"(v6)
    );
    
    asm volatile (
        "imull %1, %0\n\t"
        "imull %2, %0\n\t"
        : "+r"(v7)
        : "r"(v8), "r"(v9)
    );
    
    /* Force all variables to be used */
    g_checksum += v1 + v7 + v10;
}

int main(void) {
    printf("Starting reload coverage test...\n");
    
    /* Initialize global arrays */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 3 + 1;
    }
    for (int i = 0; i < 50; i++) {
        global_floats[i] = i * 0.1f;
    }
    for (int i = 0; i < 25; i++) {
        global_doubles[i] = i * 0.2;
    }
    
    /* Run tests multiple times to increase coverage chance */
    for (int iteration = 0; iteration < 1000; iteration++) {
        g_counter = iteration;
        
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_mixed_mode_addressing();
        test_high_register_pressure();
        
        /* Prevent loop unrolling from eliminating reloads */
        if (iteration % 100 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    printf("Final checksum: %d\n", g_checksum);
    printf("Test completed.\n");
    
    return g_checksum != 0 ? 0 : 1;
}
