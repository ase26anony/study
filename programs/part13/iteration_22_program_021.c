/* Test program to trigger reload.cc push_reload uncovered lines */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int checksum = 0;

/* Complex functions to force temporary values */
int compute_index(void) {
    static int counter = 0;
    return (counter++ * 7 + 3) % 100;
}

int complex_expression(int a, int b) {
    return (a * b + (a >> 3) - (b << 2)) ^ 0x55AA55AA;
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
    
    /* Early-clobber constraint forces reload */
    asm volatile (
        "addl %2, %0\n\t"
        "movl %0, %1\n\t"
        : "=&r"(output2), "=r"(output3)  /* Early clobber on output2 */
        : "r"(input2), "0"(input1)
        : 
    );
    
    /* Mix different register classes */
    register int r1 asm("ebx") = 100;
    asm volatile (
        "addl %%ebx, %0\n\t"
        : "+r"(output1)
        : 
        : "%ebx"
    );
    
    checksum += output1 + output2 + output3 + r1;
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    unsigned int values[100];
    for (int i = 0; i < 100; i++) {
        values[i] = i * 3 + 7;
    }
    
    /* Complex addressing in builtin operand */
    int cnt1 = __builtin_popcount(values[compute_index()] + global_counter);
    
    /* Nested function calls as operand */
    int cnt2 = __builtin_ctz(complex_expression(global_counter, 42) | 1);
    
    /* Atomic operation with complex address */
    int index = compute_index();
    __atomic_fetch_add(&values[index], cnt1 + cnt2, __ATOMIC_RELAXED);
    
    checksum += cnt1 + cnt2 + values[index % 100];
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(void) {
    /* Declare register variables tied to specific registers */
    register int reg_a asm("eax");
    register int reg_b asm("ebx");
    register int reg_c asm("ecx");
    
    reg_a = 1000 + global_counter;
    reg_b = 2000 + global_counter;
    reg_c = 3000 + global_counter;
    
    int result1, result2;
    
    /* Force conflict: output requires different register than input */
    asm volatile (
        "movl %1, %0\n\t"
        "addl %%ecx, %0\n\t"
        : "=r"(result1)
        : "r"(reg_a), "r"(reg_b)
        : "%ecx"
    );
    
    /* Use register variable in memory context */
    int *ptr = &result1;  /* Take address - forces spill if in register */
    result2 = *ptr + reg_c;
    
    /* Mix with floating point to force different register classes */
    double dbl = (double)reg_a * 1.5;
    asm volatile ("" : "+r"(reg_b) : "r"(reg_a), "rm"(dbl) : );
    
    checksum += result1 + result2 + reg_a + reg_b + reg_c + (int)dbl;
}

/* Test 4: Secondary reload triggers */
void test_secondary_reload_trigger(void) {
    /* Different architectures require different approaches */
    
#if defined(__arm__) || defined(__aarch64__)
    /* ARM: System register access often needs secondary reload */
    unsigned int control_reg;
    asm volatile (
        "mrs %0, cpsr\n\t"
        : "=r"(control_reg)
        :
        :
    );
    
    /* NEON to ARM register moves might need secondary reload */
    float32x4_t neon_vec = {1.0f, 2.0f, 3.0f, 4.0f};
    float result;
    asm volatile (
        "vmov.f32 %0, %1[0]\n\t"
        : "=r"(result)
        : "w"(neon_vec)
        :
    );
    
    checksum += control_reg + (int)(result * 100);
    
#elif defined(__x86_64__) || defined(__i386__)
    /* x86: Control register access */
    unsigned int cr0;
    asm volatile (
        "mov %%cr0, %0\n\t"
        : "=r"(cr0)
        :
        :
    );
    
    /* x87 floating point with memory constraints */
    double x = 3.14159;
    double y;
    asm volatile (
        "fldl %1\n\t"
        "fstpl %0\n\t"
        : "=m"(y)
        : "m"(x)
        : "st", "st(1)"
    );
    
    /* MMX/SSE register constraints */
    __m128i vec = _mm_set_epi32(4, 3, 2, 1);
    int elem;
    asm volatile (
        "movd %1, %0\n\t"
        : "=r"(elem)
        : "x"(vec)
        :
    );
    
    checksum += cr0 + (int)(y * 100) + elem;
#endif
}

/* Test 5: Mixed modes and addressing */
void test_mixed_modes_addressing(void) {
    char char_array[256];
    long long big_array[50];
    
    for (int i = 0; i < 256; i++) {
        char_array[i] = i;
    }
    for (int i = 0; i < 50; i++) {
        big_array[i] = i * 1000000LL;
    }
    
    /* Mix operand sizes */
    long long result1;
    asm volatile (
        "movsbl %1, %k0\n\t"
        "cltq\n\t"
        : "=a"(result1)
        : "m"(char_array[compute_index() % 256])
        :
    );
    
    /* Complex addressing mode */
    int idx = compute_index() % 50;
    long long result2;
    asm volatile (
        "movq %1, %0\n\t"
        : "=r"(result2)
        : "m"(big_array[idx])
        :
    );
    
    /* Force base register constraint if supported */
    register long long *base_ptr asm("rbx") = big_array;
    long long result3;
    asm volatile (
        "movq (%1), %0\n\t"
        : "=r"(result3)
        : "b"(base_ptr)
        : "memory"
    );
    
    checksum += (int)(result1 + result2 + result3);
}

/* Test 6: High register pressure */
void test_high_register_pressure(void) {
    /* Many live variables to increase register pressure */
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
    
    /* Complex sequence of operations forcing spills and reloads */
    v1 = complex_expression(v1, v2);
    v2 = v3 * v4 - v5;
    v3 = (v6 << 3) | (v7 >> 2);
    v4 = __builtin_popcount(v8 ^ v9);
    v5 = v10 + compute_index();
    
    /* Inline asm using many variables */
    asm volatile (
        "imull %2, %1\n\t"
        "addl %3, %0\n\t"
        "subl %4, %1\n\t"
        : "+r"(v1), "+r"(v2)
        : "r"(v3), "r"(v4), "r"(v5)
        :
    );
    
    checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

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
        test_high_register_pressure();
        
        /* Prevent loop optimization */
        if (iteration % 100 == 0) {
            printf("Iteration %d, checksum so far: %d\n", iteration, checksum);
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return checksum != 0 ? 0 : 1;
}
