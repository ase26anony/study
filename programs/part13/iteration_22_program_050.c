/* test_reload.c - Comprehensive test to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile global to prevent dead code elimination */
volatile int g_checksum = 0;

/* Complex function to force evaluation before builtins */
static int compute_index(void) {
    static int counter = 0;
    return (counter++ * 37) & 0xFF;
}

/* Global array to force memory accesses */
int global_array[256] = {0};

/* ========== Test 1: Inline Assembly with Conflicting Constraints ========== */
void test_asm_constraint_conflict(void) {
    int input1 = 12345;
    int input2 = 67890;
    int output1, output2, output3;
    
    /* Force reload by requiring specific hard register for output */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)          /* Must be in eax */
        : "mr"(input1 + 42)      /* Memory or register, but not necessarily eax */
        : /* No clobbers */
    );
    
    /* Early-clobber constraint forces separate register allocation */
    asm volatile (
        "addl %2, %0\n\t"
        "subl %1, %0\n\t"
        : "=&r"(output2)         /* Early clobber - can't share with inputs */
        : "r"(input1), "r"(input2)
        : "cc"
    );
    
    /* Mixed register classes (float vs integer) */
    double finput = 3.14159;
    long long lloutput;
    asm volatile (
        "cvtsd2si %1, %0\n\t"
        : "=r"(lloutput)         /* Integer register */
        : "x"(finput)            /* SSE register */
        : /* No clobbers */
    );
    
    /* Update checksum to prevent elimination */
    g_checksum += output1 + output2 + (int)lloutput;
}

/* ========== Test 2: Built-in Functions with Complex Operands ========== */
void test_builtin_complex_operand(void) {
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    /* Complex expression as builtin argument */
    int cnt1 = __builtin_popcount(
        global_array[compute_index()] + 
        global_array[compute_index() * 2]
    );
    
    /* Function call in builtin argument */
    int cnt2 = __builtin_ctz(
        (unsigned)global_array[compute_index()] | 1
    );
    
    /* Atomic builtin with complex address */
    int index = compute_index();
    int old_val = __atomic_fetch_add(
        &global_array[index & 0xFF], 
        5, 
        __ATOMIC_SEQ_CST
    );
    
    /* Math builtin with composite expression */
    double x = 2.0 + (double)compute_index() / 100.0;
    double root = __builtin_sqrt(x * x + 1.0);
    
    g_checksum += cnt1 + cnt2 + old_val + (int)root;
}

/* ========== Test 3: Register Variable Abuse ========== */
void test_register_variable_abuse(void) {
    /* Declare register variables tied to specific registers */
    register int r1 asm("ebx");
    register int r2 asm("esi");
    
    r1 = 1000;
    r2 = 2000;
    
    /* Force conflict: use register variable in asm requiring different register */
    int result;
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(result)
        : "r"(r1), "r"(r2)      /* r1/r2 are in ebx/esi, but we need eax for computation */
        : "eax", "cc"
    );
    
    /* Attempt to take address (GCC extension with warning) */
    int *ptr;
    asm volatile (
        "leal %1, %0\n\t"
        : "=r"(ptr)
        : "r"(r1)
        : /* No clobbers */
    );
    
    g_checksum += result + (int)(intptr_t)ptr;
}

/* ========== Test 4: Secondary Reload Triggers ========== */
void test_secondary_reload_trigger(void) {
    /* Architecture-specific secondary reload triggers */
    
#if defined(__arm__) || defined(__aarch64__)
    /* ARM: System register access often requires secondary reloads */
    unsigned int control_reg;
    asm volatile (
        "mrs %0, cpsr\n\t"
        : "=r"(control_reg)
        : /* No inputs */
        : /* No clobbers */
    );
    
    /* NEON to general purpose register move */
    float32x4_t vec = {1.0f, 2.0f, 3.0f, 4.0f};
    int32_t lane0;
    asm volatile (
        "vmov.s32 %0, %1[0]\n\t"
        : "=r"(lane0)
        : "w"(vec)
        : /* No clobbers */
    );
    
    g_checksum += control_reg + lane0;
    
#elif defined(__x86_64__) || defined(__i386__)
    /* x86: Control register access */
    unsigned long cr0;
    asm volatile (
        "mov %%cr0, %0\n\t"
        : "=r"(cr0)
        : /* No inputs */
        : /* No clobbers */
    );
    
    /* x87 FPU register constraints */
    double x = 3.14159;
    double y = 2.71828;
    double fpu_result;
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "faddp\n\t"
        "fstpl %0\n\t"
        : "=m"(fpu_result)
        : "m"(x), "m"(y)
        : "st", "st(1)"
    );
    
    g_checksum += (int)cr0 + (int)fpu_result;
    
#else
    /* Generic: Memory constraints with register-only operations */
    int mem_var = 42;
    int tmp;
    asm volatile (
        "movl %1, %%eax\n\t"
        "imull %%eax, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m"(tmp)              /* Output to memory */
        : "m"(mem_var)           /* Input from memory */
        : "eax", "cc"
    );
    
    g_checksum += tmp;
#endif
}

/* ========== Test 5: Addressing Mode Conflicts ========== */
void test_addressing_mode_conflicts(void) {
    struct {
        int a;
        int b[10];
        char c;
    } s = {0};
    
    int index = compute_index() % 10;
    
    /* Complex addressing in asm constraint */
    int result1;
    asm volatile (
        "movl %1, %0\n\t"
        : "=r"(result1)
        : "m"(s.b[index * 2])    /* Complex address calculation */
        : /* No clobbers */
    );
    
    /* Mixed-size operands */
    char c = 'A';
    long long ll = 0x123456789ABCDEF0LL;
    long long result2;
    asm volatile (
        "movsbl %1, %%eax\n\t"
        "addq %%rax, %2\n\t"
        "movq %2, %0\n\t"
        : "=r"(result2)
        : "r"(c), "r"(ll)
        : "rax", "cc"
    );
    
    g_checksum += result1 + (int)result2;
}

/* ========== Main Test Driver ========== */
int main(void) {
    printf("Starting reload pass coverage test...\n");
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    /* Run tests multiple times to increase reload pressure */
    for (int iteration = 0; iteration < 1000; iteration++) {
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_addressing_mode_conflicts();
        
        /* Add some branching to affect register allocation */
        if (iteration % 100 == 0) {
            g_checksum += iteration;
        }
    }
    
    printf("Final checksum: %d\n", g_checksum);
    printf("Test completed. If compiled with -O1 or -O2, this should\n");
    printf("trigger multiple calls to push_reload in GCC's reload pass.\n");
    
    return 0;
}
