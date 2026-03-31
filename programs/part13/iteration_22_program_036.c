/* test_reload_coverage.c - Comprehensive test to trigger reload.cc push_reload logic */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* Global volatile variables to prevent optimization */
volatile int g_volatile_result = 0;
volatile int g_checksum = 0;

/* Complex function to force register pressure */
static int compute_index(void) {
    static int counter = 0;
    return (counter++ * 37 + 123) & 0xFF;
}

/* Function to create complex addressing expressions */
static int* get_complex_address(int *base, int index) {
    return &base[index * 2 + 1];
}

/* ===== Test 1: Inline Assembly with Conflicting Constraints ===== */
void test_asm_constraint_conflict(void) {
    int input1 = 12345;
    int input2 = 67890;
    int output1, output2, output3;
    
    /* Force reload by requiring specific hard register for output */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)          /* Must be in eax */
        : "mr"(input1 + 42)      /* Memory or register, complex expression */
        : 
    );
    
    /* Early-clobber constraint forcing reload */
    asm volatile (
        "addl %1, %0\n\t"
        "subl %2, %0\n\t"
        : "=&r"(output2)         /* Early clobber */
        : "r"(input1), "r"(input2)
        : 
    );
    
    /* Multiple constraints that conflict */
    asm volatile (
        "imull %1, %0\n\t"
        : "=a"(output3)          /* Output in eax */
        : "r"(input2), "0"(output1)  /* Input0 same as output */
        : "edx"                  /* Clobbers edx for imul */
    );
    
    g_checksum += output1 + output2 + output3;
}

/* ===== Test 2: Built-in Functions with Complex Operands ===== */
void test_builtin_complex_operand(void) {
    static int array[256];
    int i, result;
    
    /* Initialize array */
    for (i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    /* Complex operand for builtin - forces temporary */
    result = __builtin_popcount(array[compute_index()] + g_checksum);
    g_checksum += result;
    
    /* Another complex builtin usage */
    result = __builtin_ctz(array[compute_index()] | 1); /* Ensure non-zero */
    g_checksum += result;
    
    /* Atomic builtin with complex address */
    int *ptr = get_complex_address(array, compute_index());
    __atomic_fetch_add(ptr, 1, __ATOMIC_RELAXED);
    g_checksum += *ptr;
}

/* ===== Test 3: Register Variable Abuse ===== */
void test_register_variable_abuse(void) {
    /* Declare register variables */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    
    r1 = 1000;
    r2 = 2000;
    
    /* Force conflict: use register variable in asm requiring different register */
    int temp;
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(temp)            /* Requires eax */
        : "r"(r1)               /* But r1 is in ebx */
        : 
    );
    
    /* Take address indirectly (GCC extension with warning) */
    int *ptr;
    asm volatile (
        "leal %1, %0\n\t"
        : "=r"(ptr)
        : "m"(r2)               /* Treat register variable as memory */
        : 
    );
    
    g_checksum += temp + (int)(intptr_t)ptr;
}

/* ===== Test 4: Secondary Reload Triggers ===== */
void test_secondary_reload_trigger(void) {
    double d1 = 3.14159;
    double d2 = 2.71828;
    double d_result;
    
    /* Floating point operations that might need secondary reloads */
    asm volatile (
        "addsd %1, %0\n\t"
        : "=x"(d_result)        /* SSE register */
        : "xm"(d1), "0"(d2)     /* Memory or SSE register */
        : 
    );
    
    /* Mix different sized operands */
    long long ll_val = 0x123456789ABCDEF0LL;
    int i_val;
    
    asm volatile (
        "movq %1, %%mm0\n\t"    /* MMX register */
        "movd %%mm0, %0\n\t"    /* Extract lower 32 bits */
        : "=r"(i_val)
        : "m"(ll_val)
        : "mm0"
    );
    
    g_checksum += (int)d_result + i_val;
}

/* ===== Test 5: Architecture-Specific Secondary Reloads ===== */
#ifdef __arm__
void test_arm_secondary_reload(void) {
    register unsigned long cpsr_val asm("cpsr");
    unsigned long temp;
    
    /* Access to system registers often needs secondary reload */
    asm volatile (
        "mrs %0, cpsr\n\t"
        : "=r"(temp)
        :
        : 
    );
    
    /* NEON/VFP to general purpose register moves */
    float f1 = 1.5f;
    int i1;
    
    asm volatile (
        "vmov.f32 s0, %1\n\t"
        "vmov %0, s0\n\t"
        : "=r"(i1)
        : "t"(f1)               /* VFP register constraint */
        : "s0"
    );
    
    g_checksum += temp + i1;
}
#endif

#ifdef __x86_64__
void test_x86_64_complex_reload(void) {
    /* 64-bit specific constraints */
    uint64_t big_val = 0xFEDCBA9876543210ULL;
    uint32_t part1, part2;
    
    /* Split 64-bit value with conflicting constraints */
    asm volatile (
        "movq %2, %%rax\n\t"
        "movl %%eax, %0\n\t"
        "shrq $32, %%rax\n\t"
        "movl %%eax, %1\n\t"
        : "=r"(part1), "=r"(part2)
        : "m"(big_val)
        : "rax"
    );
    
    /* String operations with explicit registers */
    char buffer[64] = "Test string for reload coverage";
    char buffer2[64];
    
    asm volatile (
        "lea %1, %%rsi\n\t"
        "lea %0, %%rdi\n\t"
        "movl $32, %%ecx\n\t"
        "rep movsb\n\t"
        : 
        : "r"(buffer2), "r"(buffer)
        : "rsi", "rdi", "ecx", "memory"
    );
    
    g_checksum += part1 + part2 + buffer2[0];
}
#endif

/* ===== Test 6: Mixed Mode Operations ===== */
void test_mixed_mode_operations(void) {
    /* Operands of different sizes */
    char c1 = 'A';
    short s1 = 1234;
    int i1 = 56789;
    long long ll1 = 9876543210LL;
    
    /* Force conversions and reloads */
    asm volatile (
        "movsbl %1, %%eax\n\t"
        "addw %2, %%ax\n\t"
        "addl %3, %%eax\n\t"
        "movq %%rax, %0\n\t"    /* Zero-extend to 64-bit */
        : "=r"(ll1)
        : "r"(c1), "r"(s1), "r"(i1)
        : "rax"
    );
    
    /* Memory operand with complex addressing */
    struct {
        int a;
        int b[10];
        int c;
    } s;
    
    s.a = 100;
    s.c = 200;
    for (int i = 0; i < 10; i++) {
        s.b[i] = i * 10;
    }
    
    int idx = compute_index() % 10;
    int result;
    
    asm volatile (
        "movl %1, %0\n\t"
        : "=r"(result)
        : "m"(s.b[idx * 2])     /* Complex addressing */
        : 
    );
    
    g_checksum += (int)ll1 + result + s.a + s.c;
}

/* ===== Test 7: High Register Pressure ===== */
void test_high_register_pressure(void) {
    /* Many live variables to force spills and reloads */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    
    /* Chain of operations keeping all variables live */
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
    
    /* Use all variables in final computation */
    g_checksum += v1 + v7 + v10 + v11 + v12 + v13 + v14 + v15;
}

/* ===== Main Test Driver ===== */
int main(void) {
    int i;
    
    printf("Starting reload coverage test...\n");
    
    /* Run tests multiple times to increase coverage probability */
    for (i = 0; i < 1000; i++) {
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_mixed_mode_operations();
        test_high_register_pressure();
        
        /* Architecture-specific tests */
        #ifdef __arm__
        test_arm_secondary_reload();
        #endif
        
        #ifdef __x86_64__
        test_x86_64_complex_reload();
        #endif
        
        /* Prevent loop elimination */
        if (i % 100 == 0) {
            g_volatile_result = g_checksum;
        }
    }
    
    printf("Final checksum: %d\n", g_checksum);
    printf("Test completed.\n");
    
    return g_checksum != 0 ? 0 : 1;
}
