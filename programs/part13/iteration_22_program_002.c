/* test_reload.c - Comprehensive test for GCC reload pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile global to prevent dead code elimination */
volatile int g_checksum = 0;

/* Complex function to force expression evaluation */
int compute_index(void) {
    static int counter = 0;
    return (counter++ * 37) & 0xFF;
}

/* Function returning address with complex computation */
int* get_complex_address(int *base, int offset) {
    return base + (offset * 3) / 2;
}

/* ========== Test 1: Inline Assembly with Conflicting Constraints ========== */
void test_asm_constraint_conflict(void) {
    int input1 = 12345;
    int input2 = 67890;
    int output1, output2, output3;
    
    /* Force reload by requiring specific register for output */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)        /* Must be in eax */
        : "mr"(input1 + 42)    /* Memory or register, but complex expr */
        : 
    );
    
    /* Early-clobber constraint forcing reload */
    asm volatile (
        "addl %2, %0\n\t"
        "movl %0, %1\n\t"
        : "=&r"(output2), "=r"(output3)  /* & = early clobber */
        : "r"(input2), "0"(input1)       /* input1 tied to output2 */
        : 
    );
    
    /* Mixed size constraints causing mode changes */
    char char_var = 65;
    long long ll_var = 0x123456789ABCDEF0LL;
    long long ll_result;
    
    asm volatile (
        "movsx %%al, %%rax\n\t"
        "addq %1, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r"(ll_result)
        : "r"(ll_var), "a"(char_var)     /* char_var must be in al/rax */
        : "rax"
    );
    
    g_checksum += output1 + output2 + output3 + (int)ll_result;
}

/* ========== Test 2: Built-in Functions with Complex Operands ========== */
void test_builtin_complex_operand(void) {
    static int global_array[256];
    int i;
    
    /* Initialize array */
    for (i = 0; i < 256; i++) {
        global_array[i] = i * i;
    }
    
    /* Builtin with function call as argument - forces temporary */
    int popcnt = __builtin_popcount(global_array[compute_index()]);
    
    /* Builtin with complex address computation */
    int* ptr = get_complex_address(global_array, compute_index());
    int ctz = __builtin_ctz(*ptr | 1);  /* Avoid undefined behavior for 0 */
    
    /* Math builtin with complex expression */
    double x = 2.0 + (double)compute_index() / 100.0;
    double sqrt_val = __builtin_sqrt(x * x + 1.0);
    
    /* Atomic builtin with complex address */
    int atomic_var = 0;
    __atomic_fetch_add(&atomic_var, popcnt + ctz, __ATOMIC_SEQ_CST);
    
    g_checksum += popcnt + ctz + (int)sqrt_val + atomic_var;
}

/* ========== Test 3: Register Variable Abuse ========== */
void test_register_variable_abuse(void) {
    /* Register variables with specific register constraints */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    register int r3 asm("edx");
    
    r1 = 100;
    r2 = 200;
    r3 = 300;
    
    int result1, result2;
    
    /* Force conflict: use register variable in asm requiring different register */
    asm volatile (
        "movl %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(result1)
        : /* No inputs, using fixed registers directly */
        : "eax", "ebx", "ecx"
    );
    
    /* Try to take address (GCC extension with warning) */
    int* ptr;
    asm volatile (
        "leal %1, %0\n\t"
        : "=r"(ptr)
        : "m"(r3)      /* 'm' constraint may force reload */
        :
    );
    
    /* Use in memory context */
    asm volatile (
        "addl %1, %0\n\t"
        : "+m"(*ptr)   /* Memory constraint on register variable's pseudo-address */
        : "ri"(r1)     /* Register or immediate */
        :
    );
    
    g_checksum += result1 + r1 + r2 + r3;
}

/* ========== Test 4: Architecture-Specific Secondary Reload Triggers ========== */
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
    
    /* Modify and write back - may require GPR as intermediate */
    control_reg |= 0x1C0;  /* Set some bits */
    
    asm volatile (
        "msr cpsr_f, %0\n\t"  /* Only update flags field */
        :
        : "r"(control_reg & 0xFF000000)  /* Requires reload to specific format */
        :
    );
    
    g_checksum += control_reg;
    
#elif defined(__x86_64__) || defined(__i386__)
    /* x86: Control register access requires secondary reloads */
    unsigned long long cr0;
    
    asm volatile (
        "mov %%cr0, %0\n\t"
        : "=r"(cr0)
        :
        :
    );
    
    /* FPU/MMX/SSE register moves might need temporaries */
    double x = 3.14159;
    double y;
    
    asm volatile (
        "movsd %1, %%xmm0\n\t"
        "addsd %%xmm0, %%xmm0\n\t"
        "movsd %%xmm0, %0\n\t"
        : "=m"(y)           /* Memory output */
        : "m"(x)            /* Memory input - both may need reloads */
        : "xmm0"
    );
    
    /* String operations with specific registers */
    char src[16] = "HelloReload!";
    char dst[16];
    
    asm volatile (
        "cld\n\t"
        "mov %1, %%rsi\n\t"
        "mov %0, %%rdi\n\t"
        "mov $16, %%rcx\n\t"
        "rep movsb\n\t"
        : 
        : "r"(dst), "r"(src)
        : "rsi", "rdi", "rcx", "memory"
    );
    
    g_checksum += (int)cr0 + (int)y + dst[0];
    
#elif defined(__powerpc__) || defined(__PPC__)
    /* PowerPC: Special purpose register access */
    unsigned int msr;
    
    asm volatile (
        "mfmsr %0\n\t"
        : "=r"(msr)
        :
        :
    );
    
    g_checksum += msr;
#endif
}

/* ========== Test 5: Mixed Mode and Complex Addressing ========== */
void test_mixed_mode_addressing(void) {
    struct Complex {
        int a;
        double b;
        char c[32];
    } data;
    
    data.a = 42;
    data.b = 2.71828;
    
    int index = compute_index() % 32;
    
    /* Complex addressing in asm constraint */
    asm volatile (
        "addl $1, %0\n\t"
        : "+m"(data.c[index])  /* Complex memory address */
        :
        :
    );
    
    /* Mixed integer/floating point constraints */
    double fp_result;
    int int_input = 100;
    
    asm volatile (
        "cvtsi2sd %1, %%xmm0\n\t"
        "sqrtsd %%xmm0, %%xmm0\n\t"
        "movsd %%xmm0, %0\n\t"
        : "=m"(fp_result)
        : "r"(int_input)
        : "xmm0"
    );
    
    g_checksum += data.a + (int)data.b + data.c[0] + (int)fp_result;
}

/* ========== Main Test Driver ========== */
int main(void) {
    int i;
    
    printf("Starting reload pass coverage test...\n");
    
    /* Run tests multiple times to increase coverage probability */
    for (i = 0; i < 1000; i++) {
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_mixed_mode_addressing();
        
        /* Add some branching to affect register allocation */
        if (i % 7 == 0) {
            g_checksum += i * 3;
        } else if (i % 13 == 0) {
            g_checksum -= i * 2;
        }
    }
    
    printf("Final checksum: %d\n", g_checksum);
    printf("Test completed.\n");
    
    return 0;
}
