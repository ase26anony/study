/* test_reload_coverage.c - Comprehensive test to trigger reload.cc lines 1381-1399 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_result = 0;
volatile int g_checksum = 0;

/* Global variables to force memory operands */
int g_array[100] = {0};
float g_float_array[50] = {0.0f};
double g_double_array[25] = {0.0};

/* Function to create complex expressions */
int compute_index(int i) {
    return (i * 3 + 7) % 100;
}

float compute_float(int i) {
    return (float)(i * 2.5);
}

/* Test 1: Inline assembly with conflicting constraints */
void test_asm_constraint_conflict(int iterations) {
    int i;
    int input, output;
    
    for (i = 0; i < iterations; i++) {
        input = i * 2 + 1;
        
        /* Force reload by requiring specific register for output */
        /* "=a" requires eax/rax, but input may not be in eax */
        asm volatile (
            "movl %1, %0\n\t"
            : "=a"(output)          /* Must be in eax */
            : "mr"(input + g_array[compute_index(i)])  /* Memory or register */
            : /* No clobbers */
        );
        
        /* Early-clobber constraint forces separate register */
        int temp1 = i * 3;
        int temp2 = i * 5;
        asm volatile (
            "addl %2, %0\n\t"
            "subl %1, %0\n\t"
            : "=&r"(output)         /* Early clobber - can't overlap inputs */
            : "r"(temp1), "r"(temp2)
            : "cc"
        );
        
        /* Mixed register classes - integer vs floating point */
        double d_input = (double)input;
        double d_output;
        asm volatile (
            "movq %1, %0\n\t"
            : "=x"(d_output)        /* Must be in SSE register */
            : "fm"(d_input)         /* Floating point or memory */
            : /* No clobbers */
        );
        
        g_checksum += output + (int)d_output;
    }
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(int iterations) {
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* __builtin_popcount with complex expression */
        int idx = compute_index(i);
        int popcnt = __builtin_popcount(g_array[idx] + i);
        
        /* __builtin_ctz with function call in operand */
        int ctz = __builtin_ctz(g_array[compute_index(i + 1)] | 1);
        
        /* Math built-in with composite expression */
        double dval = __builtin_sqrt((double)(g_array[i % 100] + i * i));
        
        /* Atomic built-in with complex address */
        int atomic_val = __atomic_fetch_add(&g_array[compute_index(i)], 1, __ATOMIC_RELAXED);
        
        g_checksum += popcnt + ctz + (int)dval + atomic_val;
    }
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(int iterations) {
    int i;
    
    /* Declare register variables with specific registers */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    
    for (i = 0; i < iterations; i++) {
        r1 = i * 2;
        r2 = i * 3;
        
        /* Force conflict: use register variable in asm requiring different register */
        int result;
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r"(result)          /* Any general register */
            : "r"(r1), "r"(r2)      /* r1 is ebx, r2 is ecx */
            : "eax", "cc"
        );
        
        /* Try to take address (will generate warning but useful for reload) */
        int *ptr;
        asm volatile (
            "leal %1, %0\n\t"
            : "=r"(ptr)
            : "r"(r1)
        );
        
        g_checksum += result + (int)ptr;
    }
}

/* Test 4: Secondary reload triggers (architecture-specific) */
void test_secondary_reload_trigger(int iterations) {
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Different operand sizes forcing mode changes */
        char c = (char)(i & 0xFF);
        short s = (short)(i * 2);
        long long ll = (long long)i * 1000LL;
        
        /* Mixed-size operands in same asm */
        long long result;
        asm volatile (
            "movsbl %1, %%eax\n\t"
            "movswl %2, %%edx\n\t"
            "addl %%edx, %%eax\n\t"
            "cltq\n\t"
            "addq %3, %%rax\n\t"
            "movq %%rax, %0\n\t"
            : "=r"(result)
            : "r"(c), "r"(s), "r"(ll)
            : "rax", "rdx", "eax", "edx", "cc"
        );
        
        /* Memory constraint with complex addressing */
        int mem_result;
        asm volatile (
            "movl (%1, %2, 4), %0\n\t"
            : "=r"(mem_result)
            : "r"(g_array), "r"(i % 25)
            : "memory"
        );
        
        g_checksum += (int)result + mem_result;
    }
}

/* ARM-specific secondary reload tests */
#ifdef __arm__
void test_arm_secondary_reload(int iterations) {
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Attempt to use system/coprocessor registers */
        unsigned int control_reg;
        
        /* MRC/MCR instructions often need secondary reloads */
        asm volatile (
            "mrc p15, 0, %0, c1, c0, 0\n\t"
            : "=r"(control_reg)
        );
        
        /* NEON/VFP to general purpose register moves */
        float f = (float)i;
        int int_result;
        asm volatile (
            "vmov.f32 s0, %1\n\t"
            "vcvt.s32.f32 s0, s0\n\t"
            "vmov %0, s0\n\t"
            : "=r"(int_result)
            : "t"(f)
            : "s0"
        );
        
        g_checksum += control_reg + int_result;
    }
}
#endif

/* x86-64 specific tests for more reload scenarios */
#ifdef __x86_64__
void test_x86_64_specific(int iterations) {
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Force use of specific segment register */
        unsigned short fs_val;
        asm volatile (
            "mov %%fs:0, %0\n\t"
            : "=r"(fs_val)
        );
        
        /* MMX/SSE to general purpose register */
        __m128i vec = _mm_set1_epi32(i);
        int extracted;
        asm volatile (
            "pextrd $0, %1, %0\n\t"
            : "=r"(extracted)
            : "x"(vec)
        );
        
        /* String instructions with explicit registers */
        char src[16] = "test";
        char dst[16];
        asm volatile (
            "mov %1, %%rsi\n\t"
            "mov %2, %%rdi\n\t"
            "mov $5, %%rcx\n\t"
            "rep movsb\n\t"
            : "=S"(src), "=D"(dst)
            : "r"(src), "r"(dst)
            : "rcx", "memory"
        );
        
        g_checksum += fs_val + extracted + dst[0];
    }
}
#endif

/* Main test driver */
int main(int argc, char **argv) {
    int iterations = 1000;
    
    printf("Starting reload coverage tests...\n");
    
    /* Initialize global arrays */
    for (int i = 0; i < 100; i++) {
        g_array[i] = i * 3;
    }
    for (int i = 0; i < 50; i++) {
        g_float_array[i] = (float)i * 1.5f;
    }
    for (int i = 0; i < 25; i++) {
        g_double_array[i] = (double)i * 2.5;
    }
    
    /* Run all tests */
    test_asm_constraint_conflict(iterations);
    printf("Test 1 complete, checksum: %d\n", g_checksum);
    
    test_builtin_complex_operand(iterations);
    printf("Test 2 complete, checksum: %d\n", g_checksum);
    
    test_register_variable_abuse(iterations);
    printf("Test 3 complete, checksum: %d\n", g_checksum);
    
    test_secondary_reload_trigger(iterations);
    printf("Test 4 complete, checksum: %d\n", g_checksum);
    
    /* Architecture-specific tests */
#ifdef __arm__
    test_arm_secondary_reload(iterations / 10);
    printf("ARM-specific test complete, checksum: %d\n", g_checksum);
#endif
    
#ifdef __x86_64__
    #include <xmmintrin.h>
    #include <emmintrin.h>
    test_x86_64_specific(iterations / 10);
    printf("x86-64 specific test complete, checksum: %d\n", g_checksum);
#endif
    
    /* Final result to prevent optimization */
    g_volatile_result = g_checksum;
    
    printf("All tests complete. Final checksum: %d\n", g_checksum);
    printf("Expected to trigger push_reload() with full field initialization.\n");
    
    return g_checksum == 0 ? 1 : 0;
}
