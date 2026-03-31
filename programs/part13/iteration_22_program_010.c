/* test_reload.c - Comprehensive test to trigger GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile globals to prevent optimization */
volatile int g_checksum = 0;
volatile int g_counter = 0;

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
        : 
    );
    
    /* Early-clobber constraint forces reload */
    asm volatile (
        "addl %2, %0\n\t"
        "movl %0, %1\n\t"
        : "=&r"(output2), "=r"(output3)  /* & = early clobber */
        : "r"(input2), "0"(output1)
        : 
    );
    
    /* Mixed register classes */
    register int r1 asm("ebx") = 100;
    asm volatile (
        "addl %%ebx, %0\n\t"
        : "+r"(output1)
        : 
        : "ebx"
    );
    
    g_checksum += output1 + output2 + output3 + r1;
}

/* ========== Test 2: Complex Built-in Function Operands ========== */
int compute_index(void) {
    return (g_counter++ % 8) * 2;
}

void test_builtin_complex_operand(void) {
    static int array[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    
    /* Complex addressing in builtin argument */
    int cnt1 = __builtin_popcount(array[compute_index()] + array[compute_index() + 1]);
    
    /* Nested function calls in builtin */
    double d = __builtin_sqrt((double)(compute_index() * 100 + 1));
    
    /* Atomic builtin with complex address */
    int val = 42;
    __atomic_fetch_add(&array[compute_index()], val, __ATOMIC_RELAXED);
    
    g_checksum += cnt1 + (int)d + array[0];
}

/* ========== Test 3: Register Variable Abuse ========== */
void test_register_variable_abuse(void) {
    /* Declare register variables bound to specific registers */
    register int r1 asm("esi") = 1000;
    register int r2 asm("edi") = 2000;
    register int r3 asm("ecx") = 3000;
    
    int result;
    
    /* Force conflict: use register variable in asm requiring different register */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(result)
        : "r"(r1), "r"(r2)
        : "eax"
    );
    
    /* Take address indirectly (GCC extension with warning) */
    int *ptr;
    asm volatile (
        "leal %1, %0\n\t"
        : "=r"(ptr)
        : "r"(r3)
    );
    
    g_checksum += result + *ptr;
}

/* ========== Test 4: Secondary Reload Triggers ========== */
void test_secondary_reload_trigger(void) {
    /* Architecture-specific secondary reload triggers */
    
#if defined(__arm__)
    /* ARM: System register access often requires secondary reload */
    unsigned int control_reg;
    asm volatile (
        "mrc p15, 0, %0, c1, c0, 0\n\t"
        : "=r"(control_reg)
    );
    
    /* NEON to ARM register transfer may need secondary reload */
    float32x4_t neon_vec = {1.0f, 2.0f, 3.0f, 4.0f};
    float result;
    asm volatile (
        "vmov.f32 %0, %1[0]\n\t"
        : "=r"(result)
        : "w"(neon_vec)
    );
    
    g_checksum += control_reg + (int)result;
    
#elif defined(__x86_64__)
    /* x86: Control register access requires secondary reload */
    unsigned long cr0;
    asm volatile (
        "mov %%cr0, %0\n\t"
        : "=r"(cr0)
    );
    
    /* x87 floating point with memory constraint */
    double x = 3.14159;
    double y;
    asm volatile (
        "fldl %1\n\t"
        "fstpl %0\n\t"
        : "=m"(y)
        : "m"(x)
    );
    
    g_checksum += (int)cr0 + (int)y;
    
#elif defined(__aarch64__)
    /* AArch64: System registers */
    unsigned long long tpidr;
    asm volatile (
        "mrs %0, tpidr_el0\n\t"
        : "=r"(tpidr)
    );
    
    g_checksum += (int)tpidr;
    
#else
    /* Generic: Memory constraints with register pressure */
    int a = 100, b = 200, c = 300;
    int r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
    
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
    
    /* Force spills and reloads */
    r4 = r1 + r2;
    r5 = r2 + r3;
    r6 = r3 + r1;
    r7 = r4 + r5;
    r8 = r5 + r6;
    r9 = r6 + r4;
    r10 = r7 + r8 + r9;
    
    g_checksum += r10;
#endif
}

/* ========== Test 5: Addressing Mode Conflicts ========== */
void test_addressing_mode_conflict(void) {
    struct {
        int a;
        int b[10];
        int c;
    } s = {0};
    
    int index = 5;
    int result;
    
    /* Complex addressing that may not satisfy simple constraints */
    asm volatile (
        "movl 4(,%1,4), %0\n\t"  /* Scale-index addressing */
        : "=r"(result)
        : "r"(index)
        : 
    );
    
    /* Mixed-size operands */
    char c1 = 100;
    long long ll1 = 999999999LL;
    long long ll_result;
    
    asm volatile (
        "movsbl %1, %%eax\n\t"
        "addq %%rax, %0\n\t"
        : "+r"(ll_result)
        : "r"(c1)
        : "rax"
    );
    
    g_checksum += result + (int)ll_result;
}

/* ========== Main Driver ========== */
int main(void) {
    printf("Starting reload pass coverage test...\n");
    
    /* Run tests multiple times to increase coverage probability */
    for (int i = 0; i < 1000; i++) {
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_addressing_mode_conflict();
        
        /* Prevent loop unrolling from simplifying reloads */
        if (i % 100 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    printf("Final checksum: %d\n", g_checksum);
    printf("Test completed. Compile with:\n");
    printf("  gcc -O1 -fno-omit-frame-pointer test_reload.c -o test_reload\n");
    printf("  gcc -O2 -fdump-rtl-reload test_reload.c 2>&1 | grep -i push_reload\n");
    
    return g_checksum != 0 ? 0 : 1;
}
