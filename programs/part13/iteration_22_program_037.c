/* reload_test.c - Test program to trigger specific reload.cc code paths */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to create memory operands and prevent optimizations */
volatile int global_counter = 0;
volatile int global_array[100] = {0};
volatile double global_double = 3.14159;
volatile float global_float = 2.71828f;

/* Function to create complex expressions */
int compute_index(void) {
    static int counter = 0;
    return (counter++ * 7) % 100;
}

/* Test 1: Inline assembly with conflicting constraints */
void test_asm_constraint_conflict(void) {
    int input1 = 42;
    int input2 = 73;
    int output1, output2, output3;
    
    /* Force reload by requiring specific registers for output */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)        /* Must be in eax */
        : "mr"(input1 + global_counter)  /* Memory or register */
        : 
    );
    
    /* Early-clobber constraint forcing reload */
    asm volatile (
        "addl %2, %0\n\t"
        "movl %0, %1\n\t"
        : "=&r"(output2), "=r"(output3)  /* Early clobber on output2 */
        : "r"(input2), "0"(output1)      /* input2 in register, output1 in same as output2 */
        : 
    );
    
    /* Mixed register classes causing conflicts */
    register int r1 asm("ebx") = 100;
    register int r2 asm("ecx") = 200;
    
    asm volatile (
        "xchgl %0, %1\n\t"
        : "+r"(r1), "+r"(r2)
        :
        : 
    );
    
    /* Use results to prevent elimination */
    global_counter += output1 + output2 + output3 + r1 + r2;
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    unsigned int x = 0xDEADBEEF;
    unsigned long long y = 0x123456789ABCDEF0ULL;
    
    /* Builtin with function call as argument */
    int popcnt = __builtin_popcount(global_array[compute_index()] + x);
    
    /* Builtin with memory access and computation */
    int ctz = __builtin_ctz(y | (1ULL << compute_index()));
    
    /* Math builtin with complex expression */
    double sqrt_val = __builtin_sqrt(global_double * global_counter + 1.0);
    
    /* Atomic builtin with complex address */
    int atomic_val = __atomic_fetch_add(
        (int*)&global_array[compute_index() % 50], 
        popcnt, 
        __ATOMIC_SEQ_CST
    );
    
    global_counter += popcnt + ctz + (int)sqrt_val + atomic_val;
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(void) {
    /* Declare register variables bound to specific registers */
    register int reg_a asm("eax");
    register int reg_b asm("ebx");
    register int reg_c asm("ecx");
    
    reg_a = 1000 + global_counter;
    reg_b = 2000 + global_counter;
    reg_c = 3000 + global_counter;
    
    /* Force conflicts by using in asm with different constraints */
    int result1, result2;
    
    /* Output requires specific register different from input */
    asm volatile (
        "movl %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        : "=a"(result1)        /* Result must be in eax */
        : "b"(reg_b), "c"(reg_c) /* Inputs in ebx, ecx */
        : 
    );
    
    /* Try to take address (will generate warning but test reload) */
    int *ptr;
    asm volatile (
        "leal %1, %0\n\t"
        : "=r"(ptr)
        : "m"(reg_a)           /* Force reg_a into memory operand */
        : 
    );
    
    /* Use pointer value */
    result2 = (int)(intptr_t)ptr;
    
    global_counter += result1 + result2;
}

/* Test 4: Secondary reload triggers */
void test_secondary_reload_trigger(void) {
    int value = 0x12345678;
    double dvalue = 3.14159 * global_counter;
    float fvalue = 2.71828f * global_counter;
    
    /* Different architectures have different secondary reload requirements */
    
#if defined(__arm__) || defined(__aarch64__)
    /* ARM: System register access often requires secondary reloads */
    uint32_t control_reg;
    asm volatile (
        "mrs %0, cpsr\n\t"
        : "=r"(control_reg)
        :
        : 
    );
    
    /* NEON to ARM register moves may need secondary reloads */
    asm volatile (
        "vmov.f32 s0, %1\n\t"
        "vcvt.s32.f32 s0, s0\n\t"
        "vmov %0, s0\n\t"
        : "=r"(value)
        : "r"(fvalue)
        : "s0"
    );
    
#elif defined(__x86_64__) || defined(__i386__)
    /* x86: Control register access requires secondary reloads */
    uint32_t cr0;
    asm volatile (
        "mov %%cr0, %0\n\t"
        : "=r"(cr0)
        :
        : 
    );
    
    /* x87 floating point with memory constraints */
    double result;
    asm volatile (
        "fldl %1\n\t"
        "fsqrt\n\t"
        "fstpl %0\n\t"
        : "=m"(result)
        : "m"(dvalue)
        : "st", "st(1)"
    );
    
    /* SSE/AVX with specific register constraints */
    asm volatile (
        "movsd %1, %%xmm0\n\t"
        "sqrtsd %%xmm0, %%xmm0\n\t"
        "movsd %%xmm0, %0\n\t"
        : "=m"(result)
        : "m"(dvalue)
        : "xmm0"
    );
    
    value = (int)result + cr0;
#endif
    
    /* Mixed size operands causing mode changes */
    char c = 'A' + (global_counter % 26);
    long long ll = 0xFFFFFFFF00000000ULL + global_counter;
    
    asm volatile (
        "movsbl %1, %0\n\t"
        : "=r"(value)
        : "r"(c)
        : 
    );
    
    asm volatile (
        "addq %1, %0\n\t"
        : "+r"(ll)
        : "r"((long long)value * 1000)
        : 
    );
    
    global_counter += value + (int)ll;
}

/* Test 5: Complex addressing modes */
void test_complex_addressing(void) {
    struct {
        int a;
        int b[10];
        double c;
    } s = {0};
    
    int index = compute_index() % 10;
    
    /* Force base+index addressing with reloads */
    asm volatile (
        "movl %1, %0\n\t"
        : "=r"(global_array[0])
        : "m"(s.b[index])      /* Complex addressing */
        : 
    );
    
    /* Force displacement-only addressing */
    asm volatile (
        "movl (%%eax), %%ebx\n\t"
        : "=b"(global_array[1])
        : "a"(&s.b[5])         /* Base in eax */
        : 
    );
    
    /* Multiple memory operands causing register pressure */
    asm volatile (
        "movl (%1), %%eax\n\t"
        "addl (%2), %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m"(global_array[2])
        : "r"(&global_array[3]), "r"(&global_array[4])
        : "eax"
    );
}

/* Main test driver */
int main(void) {
    int i;
    
    printf("Starting reload tests...\n");
    
    /* Run tests multiple times to increase coverage probability */
    for (i = 0; i < 1000; i++) {
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_complex_addressing();
        
        /* Alternate global values to create different patterns */
        global_double += 0.1;
        global_float -= 0.05f;
        
        if (i % 100 == 0) {
            printf("Iteration %d, counter = %d\n", i, global_counter);
        }
    }
    
    printf("Final counter value: %d\n", global_counter);
    printf("Tests completed.\n");
    
    return global_counter != 0 ? 0 : 1;
}
