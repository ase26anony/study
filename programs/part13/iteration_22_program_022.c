/* test_reload.c - Comprehensive test to trigger reload.cc push_reload logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_checksum = 0;
volatile int g_counter = 0;

/* Global arrays to create memory operands */
int global_array[100] = {0};
float global_floats[100] = {0.0f};

/* Function to create complex expressions */
int compute_index(void) {
    static int idx = 0;
    return (idx++ * 7 + 3) % 50;
}

float compute_float(void) {
    static float f = 0.5f;
    f = f * 1.1f + 0.3f;
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
        : "mr"(input1 + g_counter)  /* Memory or register */
        : 
    );
    
    /* Early-clobber constraint forces reload */
    asm volatile (
        "addl %2, %0\n\t"
        "movl %0, %1\n\t"
        : "=&r"(output2), "=r"(output3)  /* Early clobber on first output */
        : "r"(input2), "0"(output1)
        : 
    );
    
    /* Mixed register classes */
    asm volatile (
        "movl %1, %%ebx\n\t"
        "addl %%ebx, %0\n\t"
        : "=r"(output1)
        : "r"(input2), "0"(output1)
        : "%ebx"  /* Clobber ebx */
    );
    
    g_checksum += output1 + output2 + output3;
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    int idx = compute_index();
    
    /* Builtin with function call as argument */
    int popcnt = __builtin_popcount(global_array[idx] + g_counter);
    
    /* Builtin with memory access and computation */
    int ctz = __builtin_ctz(global_array[compute_index()] | 1);
    
    /* Atomic builtin with complex address */
    int *ptr = &global_array[idx * 2 % 50];
    __atomic_fetch_add(ptr, popcnt, __ATOMIC_RELAXED);
    
    /* Math builtin with computed argument */
    float f = __builtin_sqrtf(global_floats[compute_index()] + 1.0f);
    
    g_checksum += popcnt + ctz + (int)f;
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(void) {
    /* Declare register variables */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    
    r1 = 100 + g_counter;
    r2 = 200 - g_counter;
    
    /* Force conflict: use register variable in asm requiring different register */
    int result;
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0\n\t"
        : "=r"(result)
        : "r"(r1)  /* r1 is tied to ebx, but we need value in eax for add */
        : "%eax"
    );
    
    /* Try to take address (will generate warning but may trigger reloads) */
    int *ptr;
    asm volatile (
        "leal %1, %0\n\t"
        : "=r"(ptr)
        : "r"(r2)
        : 
    );
    
    g_checksum += result + (int)(intptr_t)ptr;
}

/* Test 4: Secondary reload triggers (architecture specific) */
void test_secondary_reload_trigger(void) {
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
    float32x4_t neon_vec;
    asm volatile (
        "vld1.32 {%0}, [%1]\n\t"
        : "=w"(neon_vec)
        : "r"(&global_floats[0])
        : "memory"
    );
    
    g_checksum += control_reg;
    
#elif defined(__x86_64__) || defined(__i386__)
    /* x86: Control register access requires secondary reloads */
    uint32_t cr0;
    asm volatile (
        "mov %%cr0, %0\n\t"
        : "=r"(cr0)
        : 
        : 
    );
    
    /* x87 to SSE moves may need reloads */
    double x = 3.14159;
    double y;
    asm volatile (
        "fldl %1\n\t"
        "fstpl %0\n\t"
        : "=m"(y)
        : "m"(x)
        : "st", "st(1)"
    );
    
    /* MMX/SSE mixing */
    __m64 mmx_var;
    __m128 sse_var;
    asm volatile (
        "movq %1, %0\n\t"
        : "=y"(mmx_var)
        : "x"(sse_var)
        : 
    );
    
    g_checksum += cr0 + (int)y;
    
#endif
}

/* Test 5: Mixed operand sizes and addressing modes */
void test_mixed_operands(void) {
    char c = 'A' + g_counter;
    short s = 1000 + g_counter;
    int i = 1000000 + g_counter;
    long long ll = 1000000000LL + g_counter;
    
    /* Mixed sizes in same asm statement */
    long long result;
    asm volatile (
        "movsxl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "cltq\n\t"
        "addq %3, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r"(result)
        : "r"(c), "r"(s), "r"(i)
        : "%rax", "%eax"
    );
    
    /* Complex addressing modes */
    struct {
        int a;
        int b[10];
        char c;
    } mystruct;
    
    mystruct.a = 42;
    mystruct.b[5] = 99;
    mystruct.c = 'X';
    
    int offset = compute_index() % 10;
    int struct_result;
    
    /* Force base+index*scale addressing that might need reload */
    asm volatile (
        "movl (%1,%2,4), %0\n\t"
        : "=r"(struct_result)
        : "r"(&mystruct.b[0]), "r"(offset)
        : "memory"
    );
    
    g_checksum += (int)result + struct_result;
}

/* Test 6: High register pressure to force spills and reloads */
void test_register_pressure(void) {
    /* Use many variables to create register pressure */
    int v1 = g_counter + 1;
    int v2 = g_counter + 2;
    int v3 = g_counter + 3;
    int v4 = g_counter + 4;
    int v5 = g_counter + 5;
    int v6 = g_counter + 6;
    int v7 = g_counter + 7;
    int v8 = g_counter + 8;
    int v9 = g_counter + 9;
    int v10 = g_counter + 10;
    
    /* Chain of operations forcing multiple reloads */
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
    
    /* Force memory operand reload */
    asm volatile (
        "addl (%1), %0\n\t"
        : "+r"(v10)
        : "r"(&global_array[10])
        : "memory"
    );
    
    g_checksum += v1 + v7 + v10;
}

int main(void) {
    printf("Starting reload coverage test...\n");
    
    /* Initialize global arrays */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 3 + 7;
        global_floats[i] = i * 0.1f;
    }
    
    /* Run tests multiple times to increase coverage probability */
    for (int iter = 0; iter < 1000; iter++) {
        g_counter = iter;
        
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_mixed_operands();
        test_register_pressure();
        
        /* Prevent loop unrolling from eliminating reloads */
        asm volatile ("" : : : "memory");
    }
    
    printf("Final checksum: %d\n", g_checksum);
    printf("Test completed.\n");
    
    return 0;
}
