/* test_reload_coverage.c - Comprehensive test to trigger push_reload logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile global to prevent dead code elimination */
volatile int global_checksum = 0;

/* Complex function to force expression evaluation */
static int compute_index(void) {
    static int counter = 0;
    return (counter++ * 37) & 0xFF;
}

/* Function returning non-trivial value */
static int get_value(void) {
    return 0xABCD1234;
}

/* Global arrays to force memory operands */
int global_array[256] = {0};
float global_floats[256] = {0.0f};
double global_doubles[256] = {0.0};

/* ========== Test 1: Inline Assembly with Conflicting Constraints ========== */
void test_asm_constraint_conflict(void) {
    int input1 = 42;
    int input2 = 73;
    int output1, output2, output3;
    long long large_input = 0x123456789ABCDEF0LL;
    long long large_output;
    
    /* Force reload by requiring specific register for output */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)          /* Must be in eax */
        : "mr"(input1 + 17)      /* Memory or register expression */
        : 
    );
    
    /* Early-clobber constraint forcing reload */
    asm volatile (
        "addl %2, %0\n\t"
        "movl %0, %1\n\t"
        : "=&r"(output2), "=r"(output3)  /* Early clobber on first output */
        : "r"(input2), "0"(get_value())  /* Complex input expression */
        : 
    );
    
    /* Mixed-size operands requiring mode changes */
    asm volatile (
        "movq %1, %0\n\t"
        : "=r"(large_output)
        : "mr"(large_input + 0x1000)
        : 
    );
    
    /* Multiple constraints forcing choice */
    asm volatile (
        "imull %1, %0\n\t"
        : "+r"(output1)
        : "rm"(global_array[compute_index()])  /* Memory with index computation */
        : "cc"
    );
    
    /* Update checksum */
    global_checksum += output1 + output2 + output3 + (int)large_output;
}

/* ========== Test 2: Built-in Functions with Complex Operands ========== */
void test_builtin_complex_operand(void) {
    unsigned int val1, val2, val3;
    double dval;
    
    /* Builtin with function call as argument */
    val1 = __builtin_popcount(get_value() + compute_index());
    
    /* Builtin with memory access and computation */
    val2 = __builtin_ctz(global_array[compute_index()] | 1);
    
    /* Math builtin with complex expression */
    dval = __builtin_sqrt(global_doubles[compute_index()] + 3.14159);
    
    /* Atomic builtin with complex address */
    int index = compute_index();
    __atomic_fetch_add(&global_array[index], 1, __ATOMIC_RELAXED);
    
    /* Multiple builtins in expression */
    val3 = __builtin_ffs(val1) + __builtin_parity(val2);
    
    global_checksum += val1 + val2 + val3 + (int)dval;
}

/* ========== Test 3: Register Variable Abuse ========== */
void test_register_variable_abuse(void) {
    /* Register variables with specific register requests */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    register int r3 asm("edx");
    
    r1 = 100;
    r2 = 200;
    r3 = 300;
    
    int result1, result2;
    
    /* Force conflict: use register variable in asm requiring different register */
    asm volatile (
        "movl %%ebx, %%eax\n\t"
        "addl $50, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(result1)
        : 
        : "eax", "ebx", "cc"
    );
    
    /* Take address indirectly (GCC extension with warning) */
    int *ptr;
    asm volatile (
        "leal %1, %0\n\t"
        : "=r"(ptr)
        : "r"(r2)
        : 
    );
    
    /* Use register variable in context requiring memory */
    asm volatile (
        "addl $100, %0\n\t"
        : "+m"(*ptr)  /* Memory constraint on register variable's value */
        : 
        : "cc"
    );
    
    /* Mix register variables with complex expressions */
    asm volatile (
        "imull %2, %1\n\t"
        "addl %1, %0\n\t"
        : "+r"(r1), "+r"(r2)
        : "r"(global_array[compute_index()])
        : "cc"
    );
    
    global_checksum += r1 + r2 + r3 + result1;
}

/* ========== Test 4: Architecture-Specific Secondary Reloads ========== */
void test_secondary_reload_trigger(void) {
#if defined(__arm__) || defined(__aarch64__)
    /* ARM-specific: System register access often needs secondary reloads */
    unsigned int control_reg;
    
    asm volatile (
        "mrs %0, cpsr\n\t"
        : "=r"(control_reg)
        : 
        : 
    );
    
    /* NEON to ARM register transfer may need secondary reload */
    float32x4_t neon_vec = {1.0f, 2.0f, 3.0f, 4.0f};
    float arm_float;
    
    asm volatile (
        "vmov.f32 %0, %1[0]\n\t"
        : "=r"(arm_float)
        : "w"(neon_vec)
        : 
    );
    
    global_checksum += control_reg + (int)arm_float;
    
#elif defined(__x86_64__) || defined(__i386__)
    /* x86-specific: Control register access */
    unsigned long long cr0;
    
    asm volatile (
        "mov %%cr0, %0\n\t"
        : "=r"(cr0)
        : 
        : 
    );
    
    /* x87 FPU stack manipulation */
    double x = 3.14159;
    double y;
    
    asm volatile (
        "fldl %1\n\t"
        "fsqrt\n\t"
        "fstpl %0\n\t"
        : "=m"(y)
        : "m"(x)
        : "st", "st(1)"
    );
    
    /* MMX/SSE register constraints */
    __m128i vec1, vec2;
    int mask;
    
    vec1 = _mm_set_epi32(1, 2, 3, 4);
    vec2 = _mm_set_epi32(5, 6, 7, 8);
    
    asm volatile (
        "pcmpeqd %1, %0\n\t"
        "pmovmskb %0, %k2\n\t"
        : "+x"(vec1)
        : "x"(vec2), "r"(mask)
        : "cc"
    );
    
    global_checksum += (int)cr0 + (int)y + mask;
#endif
}

/* ========== Test 5: Mixed Mode and Addressing Conflicts ========== */
void test_mixed_mode_addressing(void) {
    char c1 = 'A';
    short s1 = 1234;
    int i1 = 56789;
    long long ll1 = 0x1122334455667788LL;
    
    int result_int;
    long long result_ll;
    
    /* Mixed size operands in same asm */
    asm volatile (
        "movsbl %1, %0\n\t"
        : "=r"(result_int)
        : "m"(c1)
        : 
    );
    
    /* Complex addressing mode requirements */
    struct {
        int a;
        int b[10];
        char c;
    } mystruct;
    
    mystruct.a = 100;
    mystruct.b[5] = 500;
    mystruct.c = 'Z';
    
    int struct_result;
    
    /* Force base+index*scale addressing with reload */
    asm volatile (
        "movl %1, %0\n\t"
        : "=r"(struct_result)
        : "m"(mystruct.b[compute_index() % 10])
        : 
    );
    
    /* String operations with explicit register constraints */
    char src[50] = "Hello, World!";
    char dst[50];
    
    asm volatile (
        "movl $50, %%ecx\n\t"
        "movl %1, %%esi\n\t"
        "movl %0, %%edi\n\t"
        "rep movsb\n\t"
        : 
        : "r"(dst), "r"(src)
        : "ecx", "esi", "edi", "memory"
    );
    
    global_checksum += result_int + struct_result + dst[0];
}

/* ========== Test 6: High Register Pressure ========== */
void test_high_register_pressure(void) {
    /* Many live variables to increase register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    int v9 = 9, v10 = 10, v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15, v16 = 16;
    
    /* Force many reloads through complex expressions */
    v1 = v1 * v2 + v3 - v4 / (v5 | 1);
    v2 = v6 + v7 * v8 - v9;
    v3 = v10 & v11 | v12 ^ v13;
    v4 = v14 << v15 >> v16;
    
    /* Intermix with asm statements */
    asm volatile (
        "addl %2, %0\n\t"
        "subl %3, %1\n\t"
        : "+r"(v1), "+r"(v2)
        : "r"(v3), "r"(v4)
        : "cc"
    );
    
    asm volatile (
        "imull %1, %0\n\t"
        : "+r"(v5)
        : "r"(v6)
        : "cc"
    );
    
    /* Use all variables in final computation */
    global_checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 +
                      v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16;
}

/* ========== Main Test Driver ========== */
int main(void) {
    int i;
    
    /* Initialize global arrays */
    for (i = 0; i < 256; i++) {
        global_array[i] = i * 3;
        global_floats[i] = i * 1.5f;
        global_doubles[i] = i * 2.5;
    }
    
    printf("Starting reload coverage test...\n");
    
    /* Run tests multiple times to increase coverage probability */
    for (i = 0; i < 1000; i++) {
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_mixed_mode_addressing();
        test_high_register_pressure();
        
        /* Prevent loop optimization */
        asm volatile ("" : : : "memory");
    }
    
    printf("Final checksum: %d\n", global_checksum);
    printf("Test completed.\n");
    
    return 0;
}
