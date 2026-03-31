/* reload_test.c - Comprehensive test to trigger push_reload logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_checksum = 0;

/* Function to create complex expressions */
int compute_index(void) {
    static int counter = 0;
    return (counter++ * 37) & 0xFF;
}

/* Test 1: Inline assembly with conflicting constraints */
void test_asm_constraint_conflict(void) {
    int input1 = 12345;
    int input2 = 67890;
    int output1, output2, output3;
    
    /* Force reload by requiring specific register for output */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)      /* Must be in eax */
        : "mr"(input1 + 42)  /* Memory or register */
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
    
    /* Mixing different sized operands */
    short s_input = 1000;
    long long ll_output;
    asm volatile (
        "movswl %1, %%eax\n\t"
        "cltq\n\t"
        "movq %%rax, %0\n\t"
        : "=r"(ll_output)
        : "r"(s_input)
        : "%rax"
    );
    
    global_checksum += output1 + output2 + output3 + (int)ll_output;
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    static int array[256] = {0};
    for (int i = 0; i < 256; i++) {
        array[i] = i * i;
    }
    
    /* Complex address computation for builtin */
    int cnt = __builtin_popcount(array[compute_index()] + global_counter);
    
    /* Builtin with function call as argument */
    double x = 2.0 + global_counter * 0.01;
    double root = __builtin_sqrt(x * x + 1.0);
    
    /* Atomic operation with complex address */
    int index = compute_index();
    int old = __atomic_fetch_add(&array[index], cnt, __ATOMIC_SEQ_CST);
    
    global_checksum += cnt + (int)root + old;
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(void) {
    /* Declare register variables */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    
    r1 = 1000 + global_counter;
    r2 = 2000 + global_counter;
    
    /* Force conflict: use register variable in asm requiring different register */
    int result;
    asm volatile (
        "movl %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(result)
        : 
        : "%eax", "%ebx", "%ecx"
    );
    
    /* Try to take address (will generate warning but may force reloads) */
    int *ptr;
    asm volatile (
        "leal %1, %0\n\t"
        : "=r"(ptr)
        : "r"(r1)
        : 
    );
    
    global_checksum += result + (int)(intptr_t)ptr;
}

/* Test 4: Secondary reload triggers */
void test_secondary_reload_trigger(void) {
    /* Different architectures require different approaches */
    
#if defined(__arm__) || defined(__aarch64__)
    /* ARM: System register access often requires secondary reloads */
    unsigned int control_reg;
    asm volatile (
        "mrs %0, cpsr\n\t"
        : "=r"(control_reg)
        : 
        : 
    );
    
    /* NEON to ARM register transfer may need secondary reload */
    float f1 = 1.5f, f2 = 2.5f;
    float f_result;
    asm volatile (
        "vadd.f32 %0, %1, %2\n\t"
        : "=w"(f_result)      /* NEON register */
        : "w"(f1), "w"(f2)    /* NEON registers */
        : 
    );
    
    global_checksum += control_reg + (int)f_result;
    
#elif defined(__x86_64__) || defined(__i386__)
    /* x86: Control register access requires secondary reloads */
    unsigned long cr0;
    asm volatile (
        "mov %%cr0, %0\n\t"
        : "=r"(cr0)
        : 
        : 
    );
    
    /* x87 FPU stack operations */
    double d1 = 3.14159;
    double d2 = 2.71828;
    double d_result;
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "faddp\n\t"
        "fstpl %0\n\t"
        : "=m"(d_result)
        : "m"(d1), "m"(d2)
        : "st", "st(1)"
    );
    
    global_checksum += (int)cr0 + (int)d_result;
    
#else
    /* Generic: Memory constraints with register pressure */
    int vals[4] = {1, 2, 3, 4};
    int sum;
    
    /* Force many registers to be live */
    asm volatile (
        "movl (%1), %%eax\n\t"
        "addl 4(%1), %%eax\n\t"
        "addl 8(%1), %%eax\n\t"
        "addl 12(%1), %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(sum)
        : "r"(vals)
        : "%eax", "memory"
    );
    
    global_checksum += sum;
#endif
}

/* Test 5: Addressing mode conflicts */
void test_addressing_mode_conflicts(void) {
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i * 3;
    }
    
    /* Force base+index addressing with complex computation */
    int idx1 = compute_index() % 50;
    int idx2 = compute_index() % 50;
    int result;
    
    /* Multiple memory accesses with different addressing requirements */
    asm volatile (
        "movl (%1, %2, 4), %%eax\n\t"   /* base + index*4 */
        "addl (%3), %%eax\n\t"          /* direct memory */
        "movl %%eax, %0\n\t"
        : "=r"(result)
        : "r"(array), "r"(idx1), "r"(&array[idx2])
        : "%eax", "memory"
    );
    
    /* Byte vs word operations causing mode changes */
    char char_array[100];
    for (int i = 0; i < 100; i++) {
        char_array[i] = i;
    }
    
    int char_sum;
    asm volatile (
        "movsbl (%1), %%eax\n\t"
        "addsbl 50(%1), %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(char_sum)
        : "r"(char_array)
        : "%eax", "memory"
    );
    
    global_checksum += result + char_sum;
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
        test_addressing_mode_conflicts();
        
        /* Create register pressure by using many variables */
        int a = iteration * 1;
        int b = iteration * 2;
        int c = iteration * 3;
        int d = iteration * 4;
        int e = iteration * 5;
        
        /* Force all to be live across asm */
        asm volatile (
            "addl %1, %0\n\t"
            "addl %2, %0\n\t"
            "addl %3, %0\n\t"
            "addl %4, %0\n\t"
            : "+r"(a)
            : "r"(b), "r"(c), "r"(d), "r"(e)
            : 
        );
        
        global_checksum += a;
    }
    
    printf("Final checksum: %d\n", global_checksum);
    printf("Test completed.\n");
    
    return 0;
}
