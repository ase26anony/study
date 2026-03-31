/* test_reload.c - Comprehensive test to trigger reload.cc uncovered lines */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* Volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_checksum = 0;

/* Global arrays to create memory operands */
int global_array[100] = {0};
float global_float_array[100] = {0.0f};
double global_double_array[100] = {0.0};

/* Function to create complex expressions */
int compute_index(void) {
    static int counter = 0;
    return (counter++ * 37) % 100;
}

float compute_float(void) {
    static float f = 0.5f;
    f = f * 1.1f + 0.3f;
    return f;
}

/* Test 1: Inline assembly with conflicting constraints */
void test_asm_constraint_conflict(void) {
    int input1 = 12345;
    int input2 = 67890;
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
        : "r"(input2), "0"(output1)
        : 
    );
    
    /* Mixing different sized operands */
    short s_input = 100;
    long long ll_output;
    asm volatile (
        "movswl %1, %%eax\n\t"
        "cltq\n\t"
        "movq %%rax, %0\n\t"
        : "=r"(ll_output)
        : "r"(s_input)
        : "%rax"
    );
    
    /* Update checksum to prevent elimination */
    global_checksum += output1 + output2 + output3 + (int)(ll_output & 0xFFFFFFFF);
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    /* Complex expression as builtin argument */
    int popcnt = __builtin_popcount(global_array[compute_index()] + global_counter);
    
    /* Builtin with function call argument */
    int ctz = __builtin_ctz(compute_index() | 1);  /* Avoid zero argument */
    
    /* Math builtin with complex argument */
    double d = __builtin_sqrt(fabs(compute_float() * 2.0 + 1.0));
    
    /* Atomic builtin with complex address */
    int index = compute_index();
    int old_val = __atomic_fetch_add(&global_array[index], 1, __ATOMIC_RELAXED);
    
    global_checksum += popcnt + ctz + (int)d + old_val;
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
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(result)
        : "r"(r1), "r"(r2)  /* r1 is in ebx, r2 in ecx, but we use eax */
        : "%eax"
    );
    
    /* Try to take address (will generate warning but useful for reload) */
    int *ptr;
    asm volatile (
        "leal %1, %0\n\t"
        : "=r"(ptr)
        : "r"(r1)
        : 
    );
    
    global_checksum += result + (int)(intptr_t)ptr;
}

/* Test 4: Architecture-specific secondary reload triggers */
void test_secondary_reload_trigger(void) {
    /* This test is architecture-specific */
    
#if defined(__arm__) || defined(__aarch64__)
    /* ARM: Try to access system/coprocessor registers */
    uint32_t control_reg;
    asm volatile (
        "mrc p15, 0, %0, c1, c0, 0\n\t"
        : "=r"(control_reg)
        : 
        : 
    );
    
    /* NEON register constraints */
    float32x4_t neon_vec;
    asm volatile (
        "vld1.32 {%0}, [%1]\n\t"
        : "=w"(neon_vec)      /* NEON register constraint */
        : "r"(&global_float_array[0])
        : 
    );
    
    global_checksum += control_reg + (int)neon_vec[0];
    
#elif defined(__x86_64__) || defined(__i386__)
    /* x86: Control register access */
    uint32_t cr0;
    asm volatile (
        "mov %%cr0, %0\n\t"
        : "=r"(cr0)
        : 
        : 
    );
    
    /* x87 floating point with memory constraint */
    double x87_result;
    asm volatile (
        "fldl %1\n\t"
        "fsqrt\n\t"
        "fstpl %0\n\t"
        : "=m"(x87_result)    /* Memory output constraint */
        : "m"(global_double_array[compute_index()])
        : 
    );
    
    /* MMX/SSE register constraints */
    __m128i sse_vec;
    asm volatile (
        "movdqu %1, %0\n\t"
        : "=x"(sse_vec)       /* SSE register constraint */
        : "m"(global_array[0])
        : 
    );
    
    global_checksum += cr0 + (int)x87_result + _mm_extract_epi32(sse_vec, 0);
    
#elif defined(__powerpc__) || defined(__PPC__)
    /* PowerPC: SPR access */
    uint32_t spr;
    asm volatile (
        "mfspr %0, 0x10F\n\t"  /* Random SPR */
        : "=r"(spr)
        : 
        : 
    );
    
    global_checksum += spr;
#endif
}

/* Test 5: Mixed mode and addressing conflicts */
void test_mixed_mode_addressing(void) {
    int base = 1000;
    int index = compute_index();
    int scale = 4;
    int displacement = 100;
    
    /* Complex addressing mode that might not be directly supported */
    int result;
    asm volatile (
        "movl (%1, %2, %c3), %0\n\t"
        : "=r"(result)
        : "r"(base), "r"(index * scale), "i"(sizeof(int))
        : 
    );
    
    /* Mix 8-bit and 32-bit operands */
    char char_val = 65;
    int int_val;
    asm volatile (
        "movsbl %1, %0\n\t"
        : "=r"(int_val)
        : "r"(char_val)
        : 
    );
    
    /* Force memory operand with register constraint */
    struct {
        int a;
        int b[10];
    } s = {0};
    
    int member_result;
    asm volatile (
        "movl %1, %0\n\t"
        : "=r"(member_result)
        : "m"(s.b[index])     /* Complex memory address */
        : 
    );
    
    global_checksum += result + int_val + member_result;
}

/* Test 6: High register pressure to force spills and reloads */
void test_high_register_pressure(void) {
    /* Use many variables to increase register pressure */
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
    
    /* Chain of operations forcing intermediate reloads */
    asm volatile (
        "addl %1, %0\n\t"
        "addl %2, %0\n\t"
        "addl %3, %0\n\t"
        "addl %4, %0\n\t"
        "addl %5, %0\n\t"
        : "+r"(v1)
        : "r"(v2), "r"(v3), "r"(v4), "r"(v5), "r"(v6)
        : 
    );
    
    asm volatile (
        "imull %1, %0\n\t"
        "imull %2, %0\n\t"
        : "+r"(v7)
        : "r"(v8), "r"(v9)
        : 
    );
    
    /* Force spill by using all variables */
    global_checksum += v1 + v7 + v10;
}

int main(void) {
    printf("Starting reload coverage test...\n");
    
    /* Initialize global arrays */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 3;
        global_float_array[i] = i * 1.5f;
        global_double_array[i] = i * 2.5;
    }
    
    /* Run tests multiple times to increase coverage chance */
    for (int iteration = 0; iteration < 1000; iteration++) {
        global_counter = iteration;
        
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_mixed_mode_addressing();
        test_high_register_pressure();
        
        /* Alternate between different patterns */
        if (iteration % 2 == 0) {
            /* Force different code paths */
            volatile int temp = global_array[iteration % 50];
            asm volatile ("" : : "r"(temp) : );
        }
    }
    
    printf("Final checksum: %d\n", global_checksum);
    printf("Test completed.\n");
    
    return 0;
}
