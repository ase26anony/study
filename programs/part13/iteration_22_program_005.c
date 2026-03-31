/* reload_test.c - Test program to trigger GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Complex function to force register pressure */
int compute_index(int base) {
    return (base * 1103515245 + 12345) & 0x7fffffff;
}

/* Test 1: Inline assembly with conflicting constraints */
void test_asm_constraint_conflict(void) {
    int input1 = 42;
    int input2 = 100;
    int output1, output2, output3;
    
    /* Force reload by requiring specific registers */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)      /* Must be in eax */
        : "mr"(input1 + global_counter)  /* Memory or register */
        : 
    );
    
    /* Early clobber forces reload */
    asm volatile (
        "addl %2, %0\n\t"
        "movl %0, %1\n\t"
        : "=&r"(output2), "=r"(output3)  /* Early clobber on output2 */
        : "r"(input2), "0"(output1)
        : 
    );
    
    /* Mixed register classes */
    register int r1 asm("ebx") = output2;
    asm volatile (
        "xchgl %%ebx, %0\n\t"
        : "=r"(output1)
        : "0"(r1)
        : "ebx"
    );
    
    global_result ^= output1 + output2 + output3;
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    static int array[256];
    int i, j;
    
    /* Initialize array */
    for (i = 0; i < 256; i++) {
        array[i] = i * i;
    }
    
    /* Complex operand requiring reload */
    for (i = 0; i < 100; i++) {
        j = __builtin_popcount(array[compute_index(i) % 256] + global_counter);
        global_result += j;
    }
    
    /* Math built-in with complex argument */
    double x = 2.0 + (global_counter * 0.01);
    double y = __builtin_sqrt(x * x + 1.0);
    global_result += (int)(y * 100);
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(void) {
    /* Declare register variables */
    register int reg1 asm("esi");
    register int reg2 asm("edi");
    register int reg3;
    
    reg1 = global_counter + 1;
    reg2 = global_counter + 2;
    reg3 = global_counter + 3;
    
    /* Force conflicts with register constraints */
    int temp;
    asm volatile (
        "movl %%esi, %0\n\t"
        "addl %%edi, %0\n\t"
        : "=r"(temp)
        : 
        : "esi", "edi"
    );
    
    /* Use register variable in memory context */
    int *ptr = &reg3;  /* Warning but allowed in GCC */
    global_result += *ptr + temp;
    
    /* Mix sizes causing mode changes */
    char c = (char)reg1;
    long long ll = (long long)reg2 * 1000LL;
    asm volatile (
        "addb %1, %b0\n\t"
        : "+r"(ll)
        : "r"(c)
        : 
    );
    
    global_result += (int)ll;
}

/* Test 4: Atomic operations with complex addresses */
void test_atomic_complex_address(void) {
    struct {
        int data[16];
        int counter;
    } s;
    
    for (int i = 0; i < 16; i++) {
        s.data[i] = i * 10;
    }
    s.counter = 0;
    
    /* Atomic operation with complex address computation */
    for (int i = 0; i < 50; i++) {
        int idx = compute_index(i) % 16;
        int old = __atomic_fetch_add(&s.data[idx], 1, __ATOMIC_SEQ_CST);
        global_result += old;
    }
}

/* Test 5: Floating point constraints */
void test_float_constraints(void) {
    float f1 = 1.5f + global_counter * 0.1f;
    float f2 = 2.5f + global_counter * 0.2f;
    float f3, f4;
    
    /* Force float reloads */
    asm volatile (
        "fadds %1, %0\n\t"
        : "=t"(f3)          /* Must be top of FP stack on x87 */
        : "f"(f1), "0"(f2)
        : 
    );
    
    /* Mixed constraints */
    double d1 = (double)f3;
    asm volatile (
        "fldl %1\n\t"
        "fstpl %0\n\t"
        : "=m"(d1)
        : "f"(d1)
        : "st"
    );
    
    global_result += (int)(f3 * 100 + d1);
}

/* Test 6: Secondary reload triggers */
void test_secondary_reload_trigger(void) {
    int value = global_counter;
    int result;
    
    /* Try to trigger secondary reloads */
    
#if defined(__arm__)
    /* ARM-specific: System register access may need secondary reload */
    asm volatile (
        "mrs %0, cpsr\n\t"
        : "=r"(result)
        :
        :
    );
    
#elif defined(__aarch64__)
    /* AArch64: Special register */
    asm volatile (
        "mrs %0, nzcv\n\t"
        : "=r"(result)
        :
        :
    );
    
#elif defined(__x86_64__) || defined(__i386__)
    /* x86: Control registers */
    unsigned long cr0;
    asm volatile (
        "mov %%cr0, %0\n\t"
        : "=r"(cr0)
        :
        :
    );
    result = (int)cr0;
    
#else
    /* Generic: Memory constraint with register pressure */
    register int r1 asm("ebx") = value;
    register int r2 asm("ecx") = value * 2;
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m"(result)      /* Memory output */
        : "r"(r1), "r"(r2)
        : "eax", "memory"
    );
#endif
    
    global_result += result;
}

/* Test 7: Multiple output constraints */
void test_multiple_outputs(void) {
    int in1 = global_counter;
    int in2 = global_counter * 2;
    int out1, out2, out3;
    
    asm volatile (
        "movl %3, %0\n\t"
        "movl %4, %1\n\t"
        "addl %0, %1\n\t"
        "movl %1, %2\n\t"
        : "=&r"(out1), "=&r"(out2), "=r"(out3)  /* Multiple early clobber */
        : "r"(in1), "r"(in2)
        :
    );
    
    global_result += out1 + out2 + out3;
}

/* Test 8: Complex addressing modes */
void test_complex_addressing(void) {
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i * 3;
    }
    
    int sum = 0;
    for (int i = 0; i < 50; i++) {
        int idx1 = compute_index(i) % 100;
        int idx2 = compute_index(i + 100) % 100;
        
        /* Complex addressing in asm */
        asm volatile (
            "movl (%1, %2, 4), %%eax\n\t"
            "addl (%3), %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "+r"(sum)
            : "r"(array), "r"(idx1), "r"(&array[idx2])
            : "eax", "memory"
        );
    }
    
    global_result += sum;
}

int main(void) {
    printf("Starting reload pass coverage test...\n");
    
    /* Run tests multiple times to increase coverage probability */
    for (int iteration = 0; iteration < 1000; iteration++) {
        global_counter = iteration;
        
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_atomic_complex_address();
        test_float_constraints();
        test_secondary_reload_trigger();
        test_multiple_outputs();
        test_complex_addressing();
        
        /* Prevent loop optimization */
        if (iteration % 100 == 0) {
            printf("Iteration %d, result so far: %d\n", iteration, global_result);
        }
    }
    
    printf("Final result: %d\n", global_result);
    printf("Test completed.\n");
    
    return 0;
}
