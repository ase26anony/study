/* test_reload.c - Comprehensive test to trigger reload.cc push_reload logic */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_checksum = 0;

/* Complex function to force register pressure */
int compute_index(int base) {
    return (base * 1103515245 + 12345) & 0x7fffffff;
}

/* Test 1: Inline assembly with conflicting constraints */
void test_asm_constraint_conflict(void) {
    int input1 = 42;
    int input2 = 73;
    int output1, output2, output3;
    
    /* Force reload by requiring specific registers for output */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)          /* Must be in eax */
        : "mr"(input1 + 17)      /* Memory or register, complex expression */
        : 
    );
    
    /* Early-clobber constraint forcing reload */
    asm volatile (
        "addl %2, %0\n\t"
        "subl %1, %0\n\t"
        : "=&r"(output2)         /* Early clobber - can't overlap inputs */
        : "r"(input1), "r"(input2)
        : 
    );
    
    /* Multiple constraints with register pressure */
    int temp = input1 * input2;
    asm volatile (
        "imull %1, %0\n\t"
        : "+r"(temp)             /* Read-write operand */
        : "rm"(input2)           /* Register or memory */
        : "cc"                   /* Clobbers condition codes */
    );
    
    /* Floating point to integer with constraints */
    double fp_input = 3.14159;
    long long int_result;
    asm volatile (
        "cvtsd2si %1, %0\n\t"
        : "=r"(int_result)       /* Integer register */
        : "x"(fp_input)          /* SSE register */
        : 
    );
    
    global_checksum += output1 + output2 + temp + (int)int_result;
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    static int array[256];
    int i;
    
    /* Initialize array */
    for (i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    /* Built-in with complex address computation */
    int index = compute_index(global_counter);
    int popcnt = __builtin_popcount(array[index & 0xFF] + global_counter);
    
    /* Built-in with function call in operand */
    int ctz = __builtin_ctz(compute_index(popcnt) | 1);
    
    /* Math built-in with complex expression */
    double x = (double)global_counter / 100.0;
    double sqrt_val = __builtin_sqrt(x + 1.0);
    
    /* Atomic built-in with complex address */
    int atomic_var = 0;
    __atomic_fetch_add(&atomic_var, array[ctz & 0xFF], __ATOMIC_SEQ_CST);
    
    global_checksum += popcnt + ctz + (int)sqrt_val + atomic_var;
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(void) {
    /* Declare register variables */
    register int r1 asm("ebx");
    register int r2 asm("esi");
    register int r3 asm("edi");
    
    r1 = global_counter + 1;
    r2 = global_counter + 2;
    r3 = global_counter + 3;
    
    int result1, result2;
    
    /* Force conflict: use register variable in asm requiring different register */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(result1)
        : "r"(r1), "r"(r2)       /* r1 is ebx, r2 is esi, but we use eax in asm */
        : "eax"
    );
    
    /* Mix register variables with memory constraints */
    asm volatile (
        "addl %%ebx, %0\n\t"
        : "+m"(global_checksum)   /* Memory destination */
        : "r"(r3)                 /* Register source */
        : "ebx"
    );
    
    /* Try to take address (will generate warning but may trigger reloads) */
    int *ptr;
    asm volatile (
        "leal %1, %0\n\t"
        : "=r"(ptr)
        : "r"(r1)
        : 
    );
    
    global_checksum += result1 + (int)ptr;
}

/* Test 4: Architecture-specific secondary reload triggers */
void test_secondary_reload_trigger(void) {
    /* This test is architecture-specific */
    
#ifdef __x86_64__
    /* On x86_64, use xmm/ymm/zmm registers with constraints */
    double xmm_in = 1.5;
    double xmm_out;
    
    asm volatile (
        "vmovsd %1, %0\n\t"
        : "=x"(xmm_out)          /* XMM register */
        : "xm"(xmm_in)           /* XMM or memory */
        : 
    );
    
    /* Use 64-bit registers with 32-bit constraints */
    uint64_t big_val = 0x123456789ABCDEF0ULL;
    uint32_t low_part;
    
    asm volatile (
        "movl %k1, %0\n\t"       /* %k1 means 32-bit version of 64-bit reg */
        : "=r"(low_part)
        : "r"(big_val)
        : 
    );
    
    global_checksum += (int)xmm_out + low_part;
    
#elif defined(__arm__) || defined(__aarch64__)
    /* On ARM, try to access system/coprocessor registers */
    unsigned int cpsr;
    
    /* Reading CPSR often requires special handling */
    asm volatile (
        "mrs %0, cpsr\n\t"
        : "=r"(cpsr)
        : 
        : 
    );
    
    /* NEON register constraints */
    float32x4_t neon_in = {1.0f, 2.0f, 3.0f, 4.0f};
    float32x4_t neon_out;
    
    asm volatile (
        "vadd.f32 %0, %1, %1\n\t"
        : "=w"(neon_out)         /* NEON register */
        : "w"(neon_in)           /* NEON register */
        : 
    );
    
    global_checksum += cpsr + (int)neon_out[0];
    
#elif defined(__powerpc__) || defined(__PPC__)
    /* On PowerPC, use condition register */
    unsigned int cr;
    
    asm volatile (
        "mfcr %0\n\t"
        : "=r"(cr)
        : 
        : 
    );
    
    global_checksum += cr;
#endif
}

/* Test 5: Mixed modes and addressing */
void test_mixed_modes_addressing(void) {
    char char_var = 'A';
    short short_var = 1000;
    int int_var = 1000000;
    long long ll_var = 10000000000LL;
    
    /* Mix different sized operands */
    long long result;
    asm volatile (
        "movsbl %1, %%eax\n\t"
        "movswl %2, %%edx\n\t"
        "addl %%edx, %%eax\n\t"
        "cltq\n\t"
        "addq %3, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r"(result)
        : "r"(char_var), "r"(short_var), "r"(int_var)
        : "rax", "rdx", "cc"
    );
    
    /* Complex addressing mode */
    struct {
        int a;
        int b[10];
        int c;
    } s;
    
    s.a = 1;
    s.c = 2;
    for (int i = 0; i < 10; i++) {
        s.b[i] = i * 2;
    }
    
    int addr_result;
    asm volatile (
        "movl 4 + %1, %0\n\t"    /* Access s.b[0] with offset */
        : "=r"(addr_result)
        : "r"(&s)
        : "memory"
    );
    
    global_checksum += (int)result + addr_result;
}

/* Main test driver */
int main(void) {
    printf("Starting reload coverage test...\n");
    
    /* Run tests multiple times to increase coverage probability */
    for (int iteration = 0; iteration < 1000; iteration++) {
        global_counter = iteration;
        
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_mixed_modes_addressing();
        
        /* Prevent loop optimization */
        asm volatile ("" : : : "memory");
    }
    
    printf("Final checksum: %d\n", global_checksum);
    printf("Test completed.\n");
    
    return 0;
}
