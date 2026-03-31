/* test_reload.c - Comprehensive test for GCC reload pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_result = 0;
volatile int g_checksum = 0;

/* Global arrays to force memory operands */
int g_array[100] = {0};
float g_float_array[100] = {0.0f};
double g_double_array[100] = {0.0};

/* Function to compute index with side effects */
int compute_index(void) {
    static int counter = 0;
    return (counter++ % 50) + 25;
}

/* Complex expression function */
int complex_expr(int a, int b) {
    return (a * b) + (a / (b ? b : 1)) - (a % (b ? b : 1));
}

/* ============================================
   Test 1: Inline assembly with constraint conflicts
   ============================================ */
void test_asm_constraint_conflict(void) {
    int input1 = 12345;
    int input2 = 67890;
    int output1, output2, output3;
    int temp;
    
    /* Force reload by requiring specific register for output */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)        /* Must be in eax */
        : "mr"(input1 + 42)    /* Memory or register, complex expr */
        : 
    );
    
    /* Early-clobber constraint forces reload */
    asm volatile (
        "addl %2, %0\n\t"
        "movl %0, %1\n\t"
        : "=&r"(temp), "=r"(output2)  /* Early clobber on temp */
        : "r"(input2), "0"(input1)
        : 
    );
    
    /* Multiple constraints that conflict */
    asm volatile (
        "imull %1, %0\n\t"
        : "=a"(output3)        /* Output in eax */
        : "r"(input2), "0"(input1)    /* Input in same reg as output */
        : "%edx"               /* Clobbers edx for imul */
    );
    
    /* Mixed size constraints */
    short s_input = 1000;
    long long ll_output;
    asm volatile (
        "movswl %1, %k0\n\t"
        "cltq\n\t"
        : "=r"(ll_output)      /* 64-bit output */
        : "r"(s_input)         /* 16-bit input */
        : 
    );
    
    /* Update checksum to prevent elimination */
    g_checksum += output1 + output2 + output3 + (int)ll_output;
}

/* ============================================
   Test 2: Built-in functions with complex operands
   ============================================ */
void test_builtin_complex_operand(void) {
    unsigned int x = 0x12345678;
    unsigned int y = 0x9ABCDEF0;
    int result;
    
    /* Builtin with function call as argument */
    result = __builtin_popcount(x + compute_index());
    g_checksum += result;
    
    /* Builtin with memory access and computation */
    result = __builtin_ctz(g_array[compute_index()] | 1);
    g_checksum += result;
    
    /* Builtin with complex expression */
    result = __builtin_clz(complex_expr(x, y));
    g_checksum += result;
    
    /* Atomic builtin with complex address */
    int index = compute_index();
    __atomic_fetch_add(&g_array[index * 2], 1, __ATOMIC_RELAXED);
    
    /* Math builtin with float expression */
    float f = g_float_array[compute_index()] + 3.14159f;
    double d = __builtin_sqrt((double)f * 2.0);
    g_checksum += (int)d;
}

/* ============================================
   Test 3: Register variable abuse
   ============================================ */
void test_register_variable_abuse(void) {
    /* Register variables with specific register constraints */
    register int r1 asm("ebx");
    register int r2 asm("esi");
    register int r3 asm("edi");
    
    r1 = 100;
    r2 = 200;
    r3 = 300;
    
    int output;
    
    /* Try to force r1 into a different specific register */
    asm volatile (
        "movl %%ebx, %%eax\n\t"
        "addl $42, %%eax\n\t"
        : "=a"(output)        /* Requires eax, not ebx */
        : "b"(r1)             /* Input in ebx */
        : 
    );
    
    g_checksum += output;
    
    /* Use register variable in memory context */
    int *ptr = &g_array[0];
    asm volatile (
        "addl %%esi, (%0)\n\t"
        : 
        : "r"(ptr), "r"(r2)
        : "memory"
    );
    
    /* Multiple register variables in one asm */
    asm volatile (
        "addl %%edi, %%ebx\n\t"
        "movl %%ebx, %%esi\n\t"
        : 
        : "b"(r1), "D"(r3)
        : 
    );
}

/* ============================================
   Test 4: Secondary reload triggers
   ============================================ */
void test_secondary_reload_trigger(void) {
    /* Different architectures need different approaches */
    
#if defined(__arm__) || defined(__aarch64__)
    /* ARM: System register access often needs secondary reloads */
    unsigned int control_reg;
    asm volatile (
        "mrs %0, cpsr\n\t"
        : "=r"(control_reg)
        : 
        : 
    );
    g_checksum += control_reg & 0xFF;
    
#elif defined(__x86_64__) || defined(__i386__)
    /* x86: Control register access */
    unsigned long long cr0;
    asm volatile (
        "mov %%cr0, %0\n\t"
        : "=r"(cr0)
        : 
        : 
    );
    g_checksum += cr0 & 0xFF;
    
    /* x87 floating point stack manipulation */
    double d1 = 3.14159;
    double d2 = 2.71828;
    double result;
    
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "faddp\n\t"
        "fstpl %0\n\t"
        : "=m"(result)
        : "m"(d1), "m"(d2)
        : "st", "st(1)"
    );
    g_checksum += (int)result;
    
#elif defined(__powerpc__) || defined(__ppc__)
    /* PowerPC: Special purpose register access */
    unsigned int msr;
    asm volatile (
        "mfmsr %0\n\t"
        : "=r"(msr)
        : 
        : 
    );
    g_checksum += msr & 0xFF;
#endif
    
    /* Memory constraints with register-only operations */
    long long large_value = 0x123456789ABCDEF0LL;
    long long shifted;
    
    asm volatile (
        "movq %1, %%rax\n\t"
        "shrq $32, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=m"(shifted)        /* Memory output */
        : "m"(large_value)     /* Memory input */
        : "rax"
    );
    g_checksum += (int)shifted;
}

/* ============================================
   Test 5: Addressing mode conflicts
   ============================================ */
void test_addressing_mode_conflicts(void) {
    int array[100];
    int *ptr = array;
    int index = compute_index();
    int result;
    
    /* Base register constraint with complex address */
    asm volatile (
        "movl (%1), %0\n\t"
        : "=r"(result)
        : "b"(ptr + index)    /* Complex address computation */
        : "memory"
    );
    g_checksum += result;
    
    /* Displacement-only addressing forced */
    asm volatile (
        "movl %1, %0\n\t"
        : "=r"(result)
        : "m"(g_array[50])    /* Fixed displacement */
        : 
    );
    g_checksum += result;
    
    /* Indexed addressing */
    asm volatile (
        "movl g_array(,%1,4), %0\n\t"
        : "=r"(result)
        : "r"(index)
        : "memory"
    );
    g_checksum += result;
}

/* ============================================
   Test 6: Mixed data types and modes
   ============================================ */
void test_mixed_types_modes(void) {
    /* Char to long long with sign extension */
    char c = -100;
    long long ll;
    
    asm volatile (
        "movsbl %1, %k0\n\t"
        "cltq\n\t"
        : "=r"(ll)
        : "r"(c)
        : 
    );
    g_checksum += (int)ll;
    
    /* Float to double conversion */
    float f = 3.14159f;
    double d;
    
    asm volatile (
        "cvtss2sd %1, %0\n\t"
        : "=x"(d)
        : "x"(f)
        : 
    );
    g_checksum += (int)d;
    
    /* Mixed integer/floating point */
    int i = 255;
    float f2;
    
    asm volatile (
        "cvtsi2ssl %1, %0\n\t"
        : "=x"(f2)
        : "r"(i)
        : 
    );
    g_checksum += (int)f2;
}

/* ============================================
   Main test driver
   ============================================ */
int main(void) {
    int i;
    
    /* Initialize global arrays */
    for (i = 0; i < 100; i++) {
        g_array[i] = i * 3;
        g_float_array[i] = i * 1.5f;
        g_double_array[i] = i * 2.5;
    }
    
    printf("Starting reload coverage tests...\n");
    
    /* Run tests multiple times to increase coverage probability */
    for (i = 0; i < 1000; i++) {
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_addressing_mode_conflicts();
        test_mixed_types_modes();
        
        /* Prevent loop unrolling from eliminating reloads */
        if (i % 100 == 0) {
            g_volatile_result = g_checksum;
        }
    }
    
    printf("Final checksum: %d\n", g_checksum);
    printf("Test completed.\n");
    
    return g_checksum != 0 ? 0 : 1;
}
