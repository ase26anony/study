/* test_reload.c - Comprehensive test for GCC reload pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;
volatile int global_array[100] = {0};

/* Function to compute index with side effects */
int compute_index(void) {
    static int idx = 0;
    return (idx++ % 50) + 25;
}

/* Test 1: Inline assembly with conflicting constraints */
void test_asm_constraint_conflict(void) {
    int input1 = 12345;
    int input2 = 67890;
    int output1, output2, output3;
    
    /* Force reload by requiring specific registers */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)        /* Must be in eax */
        : "mr"(input1 + global_counter)  /* Memory or register */
        : 
    );
    
    /* Early-clobber constraint forces reload */
    asm volatile (
        "addl %2, %0\n\t"
        "movl %0, %1\n\t"
        : "=&r"(output2), "=r"(output3)  /* Early clobber on first output */
        : "r"(input2), "0"(output1)
        : 
    );
    
    /* Mixed register classes */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(output1)
        : "m"(global_array[10]), "i"(100)
        : "%eax"
    );
    
    global_result ^= output1 ^ output2 ^ output3;
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    unsigned int x = 0xDEADBEEF;
    unsigned long long y = 0x123456789ABCDEF0ULL;
    double d = 3.141592653589793;
    
    /* Complex expression as built-in argument */
    int popcnt = __builtin_popcount(x + global_counter);
    
    /* Function call in built-in argument */
    int ctz = __builtin_ctz(global_array[compute_index()] | 1);
    
    /* Math built-in with composite expression */
    double sqrt_val = __builtin_sqrt(d * d + (double)global_counter);
    
    /* Atomic built-in with complex address */
    int old_val = __atomic_fetch_add(&global_array[compute_index() + 5], 
                                     popcnt + ctz, __ATOMIC_SEQ_CST);
    
    global_result += popcnt + ctz + (int)sqrt_val + old_val;
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(void) {
    /* Declare register variables (GCC extension) */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    
    r1 = 1000 + global_counter;
    r2 = 2000 - global_counter;
    
    /* Force conflict: use register variable in asm requiring different register */
    int result;
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(result)
        : "r"(r1), "r"(r2)  /* r1/r2 in ebx/ecx, but may need reload to eax */
        : "%eax"
    );
    
    /* Attempt to take address (will generate warning but compile) */
    int *ptr;
    asm volatile (
        "leal %1, %0\n\t"
        : "=r"(ptr)
        : "r"(r1)
        : 
    );
    
    global_result ^= result + (int)(intptr_t)ptr;
}

/* Test 4: Architecture-specific secondary reload triggers */
void test_secondary_reload_trigger(void) {
    /* This test varies by architecture */
    
#if defined(__arm__) || defined(__aarch64__)
    /* ARM: System register access often requires secondary reloads */
    unsigned int control_reg;
    asm volatile (
        "mrs %0, cpsr\n\t"
        : "=r"(control_reg)
        : 
        : 
    );
    
    /* NEON register constraints */
    float32x4_t neon_vec;
    asm volatile (
        "vadd.f32 %0, %0, %0\n\t"
        : "+w"(neon_vec)  /* NEON register constraint */
        : 
        : 
    );
    
    global_result += control_reg;
    
#elif defined(__x86_64__) || defined(__i386__)
    /* x86: Control register access */
    unsigned long cr0;
    asm volatile (
        "mov %%cr0, %0\n\t"
        : "=r"(cr0)
        : 
        : 
    );
    
    /* x87 floating point stack */
    double x = 3.14159;
    double y;
    asm volatile (
        "fldl %1\n\t"
        "fsqrt\n\t"
        "fstpl %0\n\t"
        : "=m"(y)
        : "m"(x)
        : "st", "st(1)"
    );
    
    /* MMX/SSE register constraints with memory operands */
    __m128i vec;
    asm volatile (
        "paddd %1, %0\n\t"
        : "+x"(vec)  /* SSE register constraint */
        : "xm"(global_array[20])  /* Memory allowed but may need reload */
        : 
    );
    
    global_result += (int)cr0 + (int)y;
    
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    /* PowerPC: SPR access */
    unsigned int spr;
    asm volatile (
        "mfspr %0, 1\n\t"  /* XER register */
        : "=r"(spr)
        : 
        : 
    );
    
    global_result += spr;
#endif
}

/* Test 5: Mixed modes and addressing */
void test_mixed_modes_addressing(void) {
    char c = 'A';
    short s = 1234;
    int i = 56789;
    long long ll = 0x123456789ABCDEF0ULL;
    
    /* Mixed sizes in asm */
    long long result;
    asm volatile (
        "movsbl %1, %%eax\n\t"
        "movswl %2, %%edx\n\t"
        "addl %%edx, %%eax\n\t"
        "cltq\n\t"
        "addq %3, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r"(result)
        : "m"(c), "m"(s), "r"(ll)
        : "%eax", "%edx", "%rax"
    );
    
    /* Complex addressing modes */
    int index = compute_index();
    asm volatile (
        "movl (%1, %2, 4), %0\n\t"
        : "=r"(result)
        : "r"(global_array), "r"(index)
        : 
    );
    
    global_result += (int)result;
}

/* Test 6: High register pressure to force spills and reloads */
void test_high_register_pressure(void) {
    /* Many live variables to increase register pressure */
    int v1 = global_counter + 1;
    int v2 = global_counter + 2;
    int v3 = global_counter + 3;
    int v4 = global_counter + 4;
    int v5 = global_counter + 5;
    int v6 = global_counter + 6;
    int v7 = global_counter + 7;
    int v8 = global_counter + 8;
    int v9 = global_counter + 9;
    int v10 = global_counter + 10;
    
    /* Chain of operations forcing multiple reloads */
    asm volatile (
        "addl %1, %0\n\t"
        "addl %2, %0\n\t"
        "addl %3, %0\n\t"
        "addl %4, %0\n\t"
        "addl %5, %0\n\t"
        : "+r"(v1)
        : "r"(v2), "r"(v3), "r"(v4), "r"(v5), "r"(v6)
        : 
    );
    
    asm volatile (
        "imull %1, %0\n\t"
        "imull %2, %0\n\t"
        : "+r"(v7)
        : "r"(v8), "r"(v9)
        : 
    );
    
    /* Force spill by using all variables */
    global_result += v1 + v7 + v10;
}

int main(void) {
    int i;
    
    /* Initialize global array */
    for (i = 0; i < 100; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    /* Run tests multiple times to increase coverage probability */
    for (i = 0; i < 1000; i++) {
        global_counter = i;
        
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_mixed_modes_addressing();
        test_high_register_pressure();
        
        /* Prevent optimization */
        if (global_result > 1000000) {
            global_result = 0;
        }
    }
    
    printf("Final result: %d\n", global_result);
    return 0;
}
