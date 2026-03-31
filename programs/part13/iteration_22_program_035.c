/* test_reload.c - Comprehensive test to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;
int global_array[100] = {0};

/* Function to create complex expressions */
int compute_index(void) {
    static int counter = 0;
    return (counter++ * 37) % 100;
}

/* Test 1: Inline assembly with conflicting constraints */
void test_asm_constraint_conflict(void) {
    int input1 = 12345;
    int input2 = 67890;
    int output1, output2, output3;
    
    /* Force reload by requiring specific register for output */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)          /* Must be in eax */
        : "mr"(input1 + 42)      /* Memory or register, but complex expr */
        : 
    );
    
    /* Early clobber forces reload */
    asm volatile (
        "addl %2, %0\n\t"
        "movl %0, %1\n\t"
        : "=&r"(output2), "=r"(output3)  /* Early clobber on first output */
        : "r"(input2), "0"(output1)
        : 
    );
    
    /* Mixing register classes */
    double dinput = 3.14159;
    double doutput;
    asm volatile (
        "movsd %1, %0\n\t"
        : "=x"(doutput)          /* Must be in SSE register */
        : "m"(dinput)            /* Memory operand */
        : 
    );
    
    global_result += output1 + output2 + output3 + (int)doutput;
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    unsigned int x = 0xDEADBEEF;
    unsigned int y = 0xCAFEBABE;
    
    /* Complex expression as builtin argument */
    int popcnt = __builtin_popcount(x ^ y + global_array[compute_index()]);
    
    /* Builtin with function call as argument */
    int ctz = __builtin_ctz(compute_index() | 1);
    
    /* Atomic builtin with complex address */
    int index = compute_index();
    __atomic_fetch_add(&global_array[index], popcnt, __ATOMIC_RELAXED);
    
    global_result += popcnt + ctz;
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(void) {
    /* Declare register variables */
    register int r1 asm("ebx");
    register int r2 asm("esi");
    
    r1 = 100;
    r2 = 200;
    
    /* Use in conflicting context */
    int temp;
    asm volatile (
        "movl %1, %0\n\t"
        : "=r"(temp)            /* General register */
        : "r"(r1)               /* But r1 is tied to ebx */
        : 
    );
    
    /* Force address-taking (GCC extension with warning) */
    int *ptr;
    asm volatile (
        "leal %1, %0\n\t"
        : "=r"(ptr)
        : "r"(&r2)              /* Taking address of register variable */
        : 
    );
    
    global_result += temp + *ptr;
}

/* Test 4: Secondary reload triggers */
void test_secondary_reload_trigger(void) {
    /* Different architectures need different approaches */
    
#if defined(__arm__) || defined(__aarch64__)
    /* ARM: System register access often needs secondary reload */
    unsigned int control_reg;
    asm volatile (
        "mrs %0, cpsr\n\t"
        : "=r"(control_reg)
        : 
        : 
    );
    
    /* NEON to ARM register move might need secondary reload */
    float32x4_t neon_vec = {1.0f, 2.0f, 3.0f, 4.0f};
    float arm_float;
    asm volatile (
        "vmov.f32 %0, %1[0]\n\t"
        : "=r"(arm_float)
        : "w"(neon_vec)
        : 
    );
    
    global_result += control_reg + (int)arm_float;
    
#elif defined(__x86_64__) || defined(__i386__)
    /* x86: Control register access */
    unsigned long cr0;
    asm volatile (
        "mov %%cr0, %0\n\t"
        : "=r"(cr0)
        : 
        : 
    );
    
    /* MMX/SSE to general register */
    __m128i sse_val = _mm_set1_epi32(42);
    int general_reg;
    asm volatile (
        "movd %1, %0\n\t"
        : "=r"(general_reg)
        : "x"(sse_val)
        : 
    );
    
    global_result += (int)cr0 + general_reg;
    
#else
    /* Generic: Use memory constraints with register pressure */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    
    /* Create register pressure */
    asm volatile (
        "imull %1, %0\n\t"
        "addl %2, %0\n\t"
        "subl %3, %0\n\t"
        "andl %4, %0\n\t"
        "orl  %5, %0\n\t"
        : "+r"(a)
        : "r"(b), "r"(c), "r"(d), "r"(e), "r"(f)
        : 
    );
    
    global_result += a;
#endif
}

/* Test 5: Addressing mode conflicts */
void test_addressing_mode_conflicts(void) {
    int array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int index = 5;
    int result;
    
    /* Force base+index addressing conflict */
    asm volatile (
        "movl (%1, %2, 4), %0\n\t"
        : "=r"(result)
        : "r"(array), "r"(index)  /* Both in registers */
        : "memory"
    );
    
    /* Different sized operands */
    char char_val = 127;
    long long ll_val;
    asm volatile (
        "movsbl %1, %0\n\t"
        : "=r"(ll_val)           /* 64-bit output */
        : "r"(char_val)          /* 8-bit input */
        : 
    );
    
    global_result += result + (int)ll_val;
}

/* Test 6: Multiple reloads in sequence */
void test_multiple_reload_sequence(void) {
    int a = 10, b = 20, c = 30, d = 40;
    int r1, r2, r3, r4;
    
    /* Chain of operations forcing multiple reloads */
    asm volatile (
        "movl %1, %0\n\t"
        : "=r"(r1)
        : "mr"(a + b)
        : 
    );
    
    asm volatile (
        "addl %2, %1\n\t"
        "movl %1, %0\n\t"
        : "=r"(r2), "=&r"(r3)
        : "r"(c), "1"(r1)
        : 
    );
    
    asm volatile (
        "imull %1, %0\n\t"
        : "+r"(r3)
        : "rm"(d)
        : 
    );
    
    asm volatile (
        "leal (%1, %2, 2), %0\n\t"
        : "=r"(r4)
        : "r"(r2), "r"(r3)
        : 
    );
    
    global_result += r4;
}

int main(void) {
    int i;
    
    /* Initialize global array */
    for (i = 0; i < 100; i++) {
        global_array[i] = i * 3;
    }
    
    /* Run tests multiple times to increase reload opportunities */
    for (i = 0; i < 1000; i++) {
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_addressing_mode_conflicts();
        test_multiple_reload_sequence();
        
        global_counter++;
    }
    
    printf("Final result: %d\n", global_result);
    printf("Counter: %d\n", global_counter);
    
    return 0;
}
