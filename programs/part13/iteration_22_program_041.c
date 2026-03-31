/* test_reload.c - Comprehensive test to trigger reload.cc push_reload logic */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* Volatile variables to prevent optimization */
volatile int g_checksum = 0;
volatile int g_counter = 0;

/* Global variables for memory operands */
int global_array[100] = {0};
float global_floats[50] = {0.0f};
double global_doubles[50] = {0.0};

/* Function to compute index with side effects */
int compute_index(void) {
    g_counter++;
    return (g_counter * 7) % 50;
}

/* Function with complex expression */
int complex_expr(int a, int b) {
    return (a * b + a / (b ? b : 1)) ^ (a - b);
}

/* Test 1: Inline assembly with conflicting constraints */
void test_asm_constraint_conflict(void) {
    int input1 = 12345;
    int input2 = 67890;
    int output1, output2, output3;
    
    /* Force reload by requiring specific registers */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)        /* Must be in eax */
        : "mr"(input1 + 42)    /* Memory or register, but complex expr */
        : 
    );
    
    /* Early clobber forces reload */
    asm volatile (
        "addl %2, %0\n\t"
        "subl %1, %0\n\t"
        : "=&r"(output2)       /* Early clobber */
        : "r"(input1), "r"(input2)
        : 
    );
    
    /* Multiple constraints that conflict */
    asm volatile (
        "imull %1, %0\n\t"
        : "=a,a"(output3)      /* Output in eax */
        : "r,m"(complex_expr(input1, input2))  /* Complex expression */
        : "cc"
    );
    
    g_checksum += output1 + output2 + output3;
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    unsigned int x = 0xDEADBEEF;
    unsigned int y = 0xCAFEBABE;
    int count1, count2, count3;
    
    /* Builtin with function call as operand */
    count1 = __builtin_popcount(global_array[compute_index()]);
    
    /* Builtin with complex expression */
    count2 = __builtin_ctz(x ^ y | (x >> 16) | (y << 8));
    
    /* Builtin with memory indirect */
    count3 = __builtin_ffs(global_array[compute_index()] + 
                          global_array[compute_index() + 1]);
    
    /* Atomic builtin with complex address */
    int index = compute_index();
    __atomic_fetch_add(&global_array[index * 2 % 100], 1, __ATOMIC_RELAXED);
    
    g_checksum += count1 + count2 + count3 + global_array[0];
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(void) {
    /* Register variables with specific registers */
    register int r1 asm("ebx");
    register int r2 asm("esi");
    register int r3 asm("edi");
    
    r1 = 100;
    r2 = 200;
    r3 = 300;
    
    int temp;
    
    /* Force conflict: use register variable in asm requiring different reg */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(temp)           /* Requires eax */
        : "r"(r1)              /* But r1 is in ebx */
        : 
    );
    
    /* Take address indirectly (GCC extension with warning) */
    int *ptr;
    asm volatile (
        "leal %1, %0\n\t"
        : "=r"(ptr)
        : "m"(r2)              /* Treat register variable as memory */
        : 
    );
    
    /* Use in memory constraint context */
    asm volatile (
        "addl $1, %0\n\t"
        : "+m"(r3)             /* Force register variable to memory */
        : 
        : "cc"
    );
    
    g_checksum += temp + (int)(intptr_t)ptr + r3;
}

/* Test 4: Mixed data types and modes */
void test_mixed_types_modes(void) {
    char c = 'A';
    short s = 1234;
    int i = 56789;
    long long ll = 0x123456789ABCDEFLL;
    float f = 3.14159f;
    double d = 2.718281828459045;
    
    int result_int;
    long long result_ll;
    double result_double;
    
    /* Mixed sizes in same asm */
    asm volatile (
        "movsbl %1, %0\n\t"
        : "=r"(result_int)
        : "m"(c)               /* char to int requires mode change */
        : 
    );
    
    /* 64-bit operation on 32-bit target might need reloads */
    asm volatile (
        "addq %1, %0\n\t"
        : "=r"(result_ll)
        : "r"(ll), "0"(0x1000LL)
        : 
    );
    
    /* Float to int conversion */
    asm volatile (
        "cvttss2si %1, %0\n\t"
        : "=r"(result_int)
        : "x"(f)               /* SSE register constraint */
        : 
    );
    
    /* Memory constraint with complex address */
    asm volatile (
        "addsd %1, %0\n\t"
        : "=x"(result_double)
        : "m"(global_doubles[compute_index() * 2 % 50])
        : 
    );
    
    g_checksum += result_int + (int)result_ll + (int)result_double;
}

/* Test 5: Architecture-specific secondary reload triggers */
void test_secondary_reload_trigger(void) {
#ifdef __arm__
    /* ARM-specific: System register access often needs secondary reload */
    unsigned int control_reg;
    asm volatile (
        "mrc p15, 0, %0, c1, c0, 0\n\t"
        : "=r"(control_reg)
        : 
        : 
    );
    
    /* NEON to ARM register transfer */
    float32x2_t neon_vec = {1.0f, 2.0f};
    int arm_reg;
    asm volatile (
        "vmov.f32 %0, %1[0]\n\t"
        : "=r"(arm_reg)
        : "w"(neon_vec)
        : 
    );
    
    g_checksum += control_reg + arm_reg;
    
#elif defined(__x86_64__)
    /* x86-64: Control register access */
    unsigned long cr0;
    asm volatile (
        "mov %%cr0, %0\n\t"
        : "=r"(cr0)
        : 
        : 
    );
    
    /* x87 floating point with specific constraints */
    double x87_result;
    asm volatile (
        "fldl %1\n\t"
        "fsqrt\n\t"
        "fstpl %0\n\t"
        : "=m"(x87_result)
        : "m"(global_doubles[10])
        : "st", "st(1)"
    );
    
    g_checksum += (int)cr0 + (int)x87_result;
    
#elif defined(__aarch64__)
    /* AArch64: System register */
    unsigned long long tpidr;
    asm volatile (
        "mrs %0, tpidr_el0\n\t"
        : "=r"(tpidr)
        : 
        : 
    );
    
    g_checksum += (int)tpidr;
#endif
}

/* Test 6: High register pressure to force spills and reloads */
void test_high_register_pressure(void) {
    /* Many live variables to increase register pressure */
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
    
    /* Complex expressions using all variables */
    int result = 0;
    
    asm volatile (
        "addl %1, %0\n\t"
        "addl %2, %0\n\t"
        "addl %3, %0\n\t"
        "addl %4, %0\n\t"
        "addl %5, %0\n\t"
        "addl %6, %0\n\t"
        "addl %7, %0\n\t"
        "addl %8, %0\n\t"
        "addl %9, %0\n\t"
        "addl %10, %0\n\t"
        : "+r"(result)
        : "r"(v1), "r"(v2), "r"(v3), "r"(v4),
          "r"(v5), "r"(v6), "r"(v7), "r"(v8),
          "r"(v9), "r"(v10)
        : "cc"
    );
    
    g_checksum += result;
}

/* Test 7: Memory addressing mode conflicts */
void test_addressing_mode_conflicts(void) {
    struct Point {
        int x;
        int y;
        int z;
    } points[10];
    
    int base_index = compute_index() % 5;
    
    /* Complex addressing that might not match constraints */
    int result1, result2;
    
    /* Base + index * scale + displacement */
    asm volatile (
        "movl %1, %0\n\t"
        : "=r"(result1)
        : "m"(points[base_index].z)
        : 
    );
    
    /* Force memory operand with register indirect */
    asm volatile (
        "movl (%1), %0\n\t"
        : "=r"(result2)
        : "r"(&global_array[base_index * 3])
        : "memory"
    );
    
    g_checksum += result1 + result2;
}

int main(void) {
    printf("Starting reload stress test...\n");
    
    /* Initialize globals */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 3;
    }
    for (int i = 0; i < 50; i++) {
        global_floats[i] = i * 1.5f;
        global_doubles[i] = i * 2.5;
    }
    
    /* Run tests multiple times to increase coverage probability */
    for (int iteration = 0; iteration < 1000; iteration++) {
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_mixed_types_modes();
        test_secondary_reload_trigger();
        test_high_register_pressure();
        test_addressing_mode_conflicts();
        
        /* Prevent loop unrolling from eliminating reloads */
        if (iteration % 100 == 0) {
            g_counter = iteration;
        }
    }
    
    printf("Final checksum: %d\n", g_checksum);
    printf("Test completed.\n");
    
    return 0;
}
