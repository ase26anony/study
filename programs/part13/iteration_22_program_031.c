/* reload_test.c - Test program to trigger GCC reload pass uncovered lines */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Function to create complex expressions */
int compute_index(void) {
    static int counter = 0;
    return (counter++ * 7) % 100;
}

/* Complex function to force register pressure */
int complex_calculation(int a, int b, int c) {
    return (a * b) + (c << 2) - (a / (b + 1));
}

/* Test 1: Inline assembly with conflicting constraints */
void test_asm_constraint_conflict(void) {
    int input1 = 12345;
    int input2 = 67890;
    int output1, output2, output3;
    
    /* Force reload by requiring specific registers */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)      /* Must be in eax */
        : "mr"(input1 + global_counter)  /* Memory or register */
        : 
    );
    
    /* Early clobber to force reload */
    asm volatile (
        "addl %2, %0\n\t"
        "movl %0, %1\n\t"
        : "=&r"(output2), "=r"(output3)  /* Early clobber */
        : "r"(input2), "0"(output1)
        : 
    );
    
    /* Mixing register classes */
    register int r1 asm("ebx") = 42;
    int result;
    
    asm volatile (
        "xchgl %1, %0\n\t"
        : "=r"(result), "+r"(r1)
        : "0"(global_counter)
        : 
    );
    
    global_result ^= output1 + output2 + output3 + result;
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    static int array[256];
    int i;
    
    /* Initialize array */
    for (i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    /* Complex operand requiring temporary */
    int cnt = __builtin_popcount(array[compute_index()] + global_counter);
    
    /* Multiple complex arguments */
    int idx1 = compute_index();
    int idx2 = compute_index();
    int val = __builtin_add_overflow(array[idx1], array[idx2], &global_counter);
    
    /* Math built-in with complex argument */
    double x = (double)array[compute_index()] / 2.0;
    double root = __builtin_sqrt(x + global_counter);
    
    global_result += cnt + val + (int)root;
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(void) {
    /* Declare register variables */
    register int reg1 asm("esi");
    register int reg2 asm("edi");
    register int reg3 asm("ebx");
    
    reg1 = 1000 + global_counter;
    reg2 = 2000 + global_counter;
    reg3 = 3000 + global_counter;
    
    /* Force conflicts with specific register constraints */
    int temp;
    asm volatile (
        "movl %%esi, %0\n\t"
        "addl %%edi, %0\n\t"
        "addl %%ebx, %0\n\t"
        : "=r"(temp)
        : 
        : "esi", "edi", "ebx"
    );
    
    /* Try to take address (will generate warning but test reload) */
    int *ptr;
    asm volatile (
        "leal %1, %0\n\t"
        : "=r"(ptr)
        : "r"(reg1)
        : 
    );
    
    global_result ^= temp + (int)ptr;
}

/* Test 4: Secondary reload triggers */
void test_secondary_reload_trigger(void) {
    double d1 = 3.14159 + global_counter;
    double d2 = 2.71828 + global_counter;
    double d3, d4;
    
    /* Floating point constraints that might need secondary reloads */
    asm volatile (
        "addsd %1, %0\n\t"
        : "=x"(d3)          /* SSE register */
        : "xm"(d1), "0"(d2) /* Memory or SSE register */
        : 
    );
    
    /* Memory constraint with complex address */
    struct {
        double values[4];
    } data;
    
    data.values[0] = d1;
    data.values[1] = d2;
    data.values[2] = d3;
    
    asm volatile (
        "movsd %1, %0\n\t"
        : "=x"(d4)
        : "m"(data.values[compute_index() % 3])
        : 
    );
    
    /* Integer to/from float moves that might need reloads */
    int int_val = global_counter * 100;
    double float_val;
    
    asm volatile (
        "cvtsi2sd %1, %0\n\t"
        : "=x"(float_val)
        : "r"(int_val)
        : 
    );
    
    global_result += (int)(d3 + d4 + float_val);
}

/* Test 5: Mixed modes and sizes */
void test_mixed_modes(void) {
    char c1 = 'A' + (global_counter % 26);
    short s1 = 1000 + global_counter;
    int i1 = 1000000 + global_counter;
    long long ll1 = 1000000000LL + global_counter;
    
    /* Mixing different sized operands */
    long long result;
    
    asm volatile (
        "movsbl %1, %%eax\n\t"
        "movswl %2, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "addl %3, %%eax\n\t"
        "cltq\n\t"
        "addq %4, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r"(result)
        : "r"(c1), "r"(s1), "r"(i1), "r"(ll1)
        : "rax", "rbx", "eax", "ebx"
    );
    
    /* Pointer arithmetic with different types */
    int *ptr1 = (int*)malloc(100 * sizeof(int));
    char *ptr2 = (char*)ptr1;
    
    for (int i = 0; i < 100; i++) {
        ptr1[i] = i * global_counter;
    }
    
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += ptr2[i * sizeof(int)];
    }
    
    free(ptr1);
    global_result ^= (int)result + sum;
}

/* Test 6: Atomic operations with complex addresses */
void test_atomic_complex_address(void) {
    struct {
        int counter;
        int data[64];
        int padding[16];
    } atomic_struct;
    
    atomic_struct.counter = 0;
    for (int i = 0; i < 64; i++) {
        atomic_struct.data[i] = i * 7;
    }
    
    /* Atomic operation with complex address computation */
    int idx = compute_index() % 64;
    int old = __atomic_fetch_add(&atomic_struct.data[idx], 1, __ATOMIC_SEQ_CST);
    
    /* Compare-exchange with address computation */
    int expected = atomic_struct.data[idx];
    int desired = expected + 10;
    
    __atomic_compare_exchange(&atomic_struct.data[idx], &expected, &desired, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    global_result += old + expected;
}

/* Test 7: High register pressure */
void test_high_register_pressure(void) {
    /* Many live variables to force spilling and reloading */
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
    
    /* Complex expression using all variables */
    int result = complex_calculation(v1, v2, v3);
    result += complex_calculation(v4, v5, v6);
    result += complex_calculation(v7, v8, v9);
    result *= v10;
    
    /* Inline assembly using multiple variables */
    asm volatile (
        "imull %1, %0\n\t"
        "addl %2, %0\n\t"
        "subl %3, %0\n\t"
        "addl %4, %0\n\t"
        : "+r"(result)
        : "r"(v1), "r"(v2), "r"(v3), "r"(v4)
        : 
    );
    
    global_result ^= result;
}

int main(void) {
    printf("Starting reload pass coverage test...\n");
    
    /* Run tests multiple times to increase coverage probability */
    for (int iteration = 0; iteration < 1000; iteration++) {
        global_counter = iteration;
        
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_mixed_modes();
        test_atomic_complex_address();
        test_high_register_pressure();
        
        /* Prevent loop unrolling */
        if (iteration % 100 == 0) {
            printf("Iteration %d, result so far: %d\n", iteration, global_result);
        }
    }
    
    printf("Final result: %d\n", global_result);
    printf("Test completed.\n");
    
    return global_result != 0 ? 0 : 1;
}
