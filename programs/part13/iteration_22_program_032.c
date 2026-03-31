/* Test program to trigger reload.cc:1381-1399 coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to create register pressure and prevent optimization */
volatile int global_counter = 0;
volatile int global_checksum = 0;

/* Function to create complex expressions */
int compute_index(int base) {
    return (base * 3 + 7) & 0xFF;
}

/* Test 1: Inline assembly with conflicting constraints */
void test_asm_constraint_conflict(void) {
    int input1 = 12345;
    int input2 = 67890;
    int output1, output2, output3;
    
    /* Force reload by requiring specific register for output */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)        /* Must be in eax */
        : "mr"(input1 + 42)    /* Memory or register */
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
    
    /* Mix different sized operands */
    short s_input = 1000;
    long long ll_output;
    asm volatile (
        "movswl %1, %0\n\t"
        : "=r"(ll_output)
        : "r"(s_input)
        : 
    );
    
    global_checksum += output1 + output2 + output3 + (int)ll_output;
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    static int array[256] = {0};
    for (int i = 0; i < 256; i++) {
        array[i] = i;
    }
    
    /* Complex expression as builtin argument */
    int idx = compute_index(global_counter);
    int popcnt = __builtin_popcount(array[idx] + global_counter);
    
    /* Multiple complex arguments */
    int ctz = __builtin_ctz(array[compute_index(popcnt)] | 1);
    
    /* Address computation requiring reload */
    int *ptr = &array[compute_index(ctz)];
    int fetch = __atomic_fetch_add(ptr, 1, __ATOMIC_RELAXED);
    
    global_checksum += popcnt + ctz + fetch;
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(void) {
    /* Declare register variables */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    
    r1 = 1000 + global_counter;
    r2 = 2000 + global_counter;
    
    /* Force conflict by requiring different register in asm */
    int result;
    asm volatile (
        "movl %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(result)
        : 
        : "eax", "ebx", "ecx"
    );
    
    /* Try to take address (will generate warning but compile) */
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
    double d1 = 3.14159 + global_counter;
    double d2 = 2.71828 + global_counter;
    double d_result;
    
    /* Floating point operations that may require secondary reloads */
    asm volatile (
        "addsd %1, %0\n\t"
        : "=x"(d_result)
        : "xm"(d1), "0"(d2)
        : 
    );
    
    /* Memory constraint with complex address */
    struct {
        int a;
        double b[4];
        int c;
    } s = {0};
    
    s.b[2] = d_result;
    
    asm volatile (
        "movsd %1, %0\n\t"
        : "=x"(d_result)
        : "m"(s.b[compute_index(2) % 4])
        : 
    );
    
    /* Integer to/from floating point moves */
    int int_val = 42;
    double double_val;
    
    asm volatile (
        "cvtsi2sd %1, %0\n\t"
        : "=x"(double_val)
        : "r"(int_val)
        : 
    );
    
    global_checksum += (int)d_result + (int)double_val;
}

/* Test 5: Mixed mode operations */
void test_mixed_mode_operations(void) {
    char c_array[100];
    long long ll_array[50];
    
    for (int i = 0; i < 100; i++) {
        c_array[i] = (char)(i + global_counter);
    }
    
    for (int i = 0; i < 50; i++) {
        ll_array[i] = (long long)i * 1000 + global_counter;
    }
    
    /* Mixed sizes in same operation */
    long long sum = 0;
    for (int i = 0; i < 50; i++) {
        int idx = compute_index(i);
        sum += (long long)c_array[idx] + ll_array[i % 50];
    }
    
    /* Pointer arithmetic with different types */
    char *c_ptr = c_array;
    long long *ll_ptr = ll_array;
    
    asm volatile (
        "movsbl (%1), %%eax\n\t"
        "addq (%2), %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r"(sum)
        : "r"(c_ptr + 10), "r"(ll_ptr + 5)
        : "rax", "eax"
    );
    
    global_checksum += (int)(sum & 0xFFFFFFFF);
}

/* Test 6: High register pressure */
void test_high_register_pressure(void) {
    /* Many live variables to force spilling */
    int v1 = global_counter * 1;
    int v2 = global_counter * 2;
    int v3 = global_counter * 3;
    int v4 = global_counter * 4;
    int v5 = global_counter * 5;
    int v6 = global_counter * 6;
    int v7 = global_counter * 7;
    int v8 = global_counter * 8;
    int v9 = global_counter * 9;
    int v10 = global_counter * 10;
    
    /* Complex expression using all variables */
    int result = 
        (v1 * v2) + (v3 / (v4 | 1)) - (v5 & v6) + 
        (v7 ^ v8) | (v9 << 2) + (v10 >> 1);
    
    /* Inline asm that uses many registers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "subl %3, %%eax\n\t"
        "imull %4, %%eax\n\t"
        "addl %5, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(result)
        : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5)
        : "eax", "cc"
    );
    
    global_checksum += result + v6 + v7 + v8 + v9 + v10;
}

int main(void) {
    printf("Starting reload coverage test...\n");
    
    /* Run tests multiple times to increase coverage probability */
    for (int i = 0; i < 1000; i++) {
        global_counter = i;
        
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_mixed_mode_operations();
        test_high_register_pressure();
        
        /* Prevent loop optimization */
        asm volatile ("" : : : "memory");
    }
    
    printf("Final checksum: %d\n", global_checksum);
    printf("Test completed.\n");
    
    return 0;
}
