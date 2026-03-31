/* test_reload_coverage.c - Comprehensive test to trigger reload.cc lines 1381-1399 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_result = 0;
volatile int g_checksum = 0;

/* Global variables for memory operands */
int g_array[100] = {0};
float g_float_array[50] = {0.0f};
double g_double_array[25] = {0.0};

/* Function to create complex expressions */
int compute_index(void) {
    static int counter = 0;
    return (counter++ * 37) % 100;
}

float compute_float(void) {
    static float f = 1.0f;
    f = f * 1.1f;
    return f;
}

/* Test 1: Inline assembly with conflicting constraints */
void test_asm_constraint_conflict(void) {
    int input1 = 42;
    int input2 = 73;
    int output1, output2, output3;
    
    /* Force reload by requiring specific register for output */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)        /* Must be in eax */
        : "mr"(input1 + 17)    /* Memory or register, complex expression */
        : 
    );
    
    /* Early-clobber constraint forces reload */
    asm volatile (
        "addl %1, %0\n\t"
        "subl %2, %0\n\t"
        : "=&r"(output2)       /* Early clobber */
        : "r"(input1), "r"(input2)
        : 
    );
    
    /* Multiple constraints with register pressure */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    asm volatile (
        "imull %1, %0\n\t"
        "addl %2, %0\n\t"
        : "+r"(a), "+r"(b), "+r"(c)
        : "r"(d), "r"(e)
        : 
    );
    
    g_volatile_result += output1 + output2 + a + b + c;
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    /* Complex expression as built-in argument */
    int popcnt = __builtin_popcount(g_array[compute_index()] + 255);
    
    /* Multiple built-ins with register pressure */
    int ctz1 = __builtin_ctz(g_array[compute_index()] | 1);
    int ctz2 = __builtin_ctz(g_array[compute_index() + 1] | 1);
    int ctz3 = __builtin_ctz(g_array[compute_index() + 2] | 1);
    
    /* Atomic operations with complex addresses */
    int index = compute_index();
    __atomic_fetch_add(&g_array[index], 1, __ATOMIC_RELAXED);
    __atomic_fetch_add(&g_array[index + 1], popcnt, __ATOMIC_RELAXED);
    
    g_volatile_result += popcnt + ctz1 + ctz2 + ctz3;
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(void) {
    /* Register variables with specific register constraints */
    register int r1 asm("ebx");
    register int r2 asm("esi");
    register int r3 asm("edi");
    
    r1 = 100;
    r2 = 200;
    r3 = 300;
    
    /* Force conflicts by using in asm with different constraints */
    int temp;
    asm volatile (
        "movl %1, %0\n\t"
        : "=r"(temp)
        : "r"(r1)
        : 
    );
    
    /* Try to take address (will generate warning but useful for reload) */
    int *ptr;
    asm volatile (
        "leal %1, %0\n\t"
        : "=r"(ptr)
        : "r"(r2)
        : 
    );
    
    /* Use register variable in memory context */
    asm volatile (
        "addl $1, %0\n\t"
        : "+m"(*(int*)&r3)
        : 
        : 
    );
    
    g_volatile_result += r1 + r2 + r3 + temp;
}

/* Test 4: Architecture-specific secondary reload triggers */
void test_secondary_reload_trigger(void) {
    /* x86 specific: Control register access (requires secondary reload) */
    #if defined(__i386__) || defined(__x86_64__)
    unsigned long cr0;
    asm volatile (
        "mov %%cr0, %0\n\t"
        : "=r"(cr0)
        : 
        : 
    );
    
    /* x87 FPU stack manipulation */
    double d1 = 3.14159;
    double d2 = 2.71828;
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "faddp\n\t"
        "fstpl %0\n\t"
        : "=m"(d1)
        : "m"(d1), "m"(d2)
        : 
    );
    #endif
    
    /* ARM specific: CPSR access */
    #if defined(__arm__)
    unsigned long cpsr;
    asm volatile (
        "mrs %0, cpsr\n\t"
        : "=r"(cpsr)
        : 
        : 
    );
    #endif
    
    /* PowerPC specific: SPR access */
    #if defined(__powerpc__) || defined(__ppc__)
    unsigned long spr;
    asm volatile (
        "mfspr %0, 0x10F\n\t"  /* Random SPR */
        : "=r"(spr)
        : 
        : 
    );
    #endif
    
    g_volatile_result += cr0;
}

/* Test 5: Mixed modes and addressing */
void test_mixed_modes_addressing(void) {
    char c = 'A';
    short s = 1234;
    int i = 56789;
    long long ll = 1234567890123LL;
    
    /* Mixed size operands in asm */
    asm volatile (
        "addb %1, %b0\n\t"    /* byte operation */
        "addw %2, %w0\n\t"    /* word operation */
        "addl %3, %k0\n\t"    /* dword operation */
        "addq %4, %0\n\t"     /* qword operation */
        : "+r"(ll)
        : "r"(c), "r"(s), "r"(i), "r"(ll)
        : 
    );
    
    /* Complex addressing modes */
    struct {
        int a;
        int b[10];
        int c;
    } s1 = {0};
    
    int idx = compute_index() % 10;
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0\n\t"
        : "+m"(s1.b[idx])
        : "m"(s1.a)
        : "%eax"
    );
    
    g_volatile_result += ll + s1.b[idx];
}

/* Test 6: Floating point constraints */
void test_floating_point_constraints(void) {
    float f1 = compute_float();
    float f2 = f1 * 2.0f;
    double d1 = f1 * 3.0;
    double d2 = d1 * 1.5;
    
    /* Force xmm register constraints on x86_64 */
    #if defined(__x86_64__)
    asm volatile (
        "addss %1, %0\n\t"
        : "+x"(f1)
        : "x"(f2)
        : 
    );
    
    asm volatile (
        "addsd %1, %0\n\t"
        : "+x"(d1)
        : "x"(d2)
        : 
    );
    #endif
    
    /* General floating point constraints */
    asm volatile (
        "" 
        : "+f"(f1), "+f"(f2)
        : 
        : 
    );
    
    g_volatile_result += (int)f1 + (int)d1;
}

/* Main test driver */
int main(void) {
    printf("Starting reload coverage test...\n");
    
    /* Initialize global arrays */
    for (int i = 0; i < 100; i++) {
        g_array[i] = i * 3;
    }
    for (int i = 0; i < 50; i++) {
        g_float_array[i] = i * 1.5f;
    }
    
    /* Run tests multiple times to increase coverage probability */
    for (int iteration = 0; iteration < 1000; iteration++) {
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_mixed_modes_addressing();
        test_floating_point_constraints();
        
        /* Update checksum to prevent elimination */
        g_checksum ^= g_volatile_result;
        g_volatile_result = iteration;
    }
    
    printf("Final checksum: %d\n", g_checksum);
    printf("Test completed.\n");
    
    return g_checksum != 0 ? 0 : 1;
}
