/* Test program to trigger GCC reload pass push_reload logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile global to prevent optimization */
volatile int g_checksum = 0;

/* Function to create complex expressions */
int compute_index(void) {
    static int counter = 0;
    return counter++ & 0xF;
}

/* Force memory operations with complex addressing */
int global_array[256] = {0};
float global_floats[256] = {0.0f};

/* Test 1: Inline assembly with conflicting constraints */
void test_asm_constraint_conflict(void) {
    int input1 = 12345;
    int input2 = 67890;
    int output1, output2;
    
    /* Force reload by requiring specific register for output */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)          /* Must be in eax */
        : "mr"(input1 + 42)      /* Memory or register */
        : 
    );
    
    /* Early-clobber constraint forcing reload */
    asm volatile (
        "addl %2, %0\n\t"
        "movl %0, %1\n\t"
        : "=&r"(output1), "=r"(output2)  /* Early clobber */
        : "rm"(input2), "0"(input1)
        : 
    );
    
    /* Mixing register classes */
    float f1 = 3.14f;
    float f2 = 2.71f;
    float f_result;
    
    asm volatile (
        "fadds %1, %0\n\t"
        : "=t"(f_result)         /* Top of FPU stack */
        : "f"(f1), "0"(f2)
        : 
    );
    
    g_checksum += output1 + output2 + (int)f_result;
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    /* Force reload for builtin argument */
    int cnt = __builtin_popcount(global_array[compute_index()] + 42);
    
    /* Complex address computation for atomic */
    int index = compute_index();
    int old_val = __atomic_fetch_add(&global_array[index * 2], cnt, __ATOMIC_SEQ_CST);
    
    /* Math builtin with complex argument */
    double d = __builtin_sqrt((double)global_array[compute_index()] + 1.0);
    
    g_checksum += cnt + old_val + (int)d;
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(void) {
    /* Declare register variables */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    
    r1 = 1000;
    r2 = 2000;
    
    /* Force conflict by using in asm requiring different register */
    int result;
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(result)           /* Requires eax */
        : "r"(r1)                /* But r1 is in ebx */
        : 
    );
    
    /* Take address indirectly (GCC extension with warning) */
    int *ptr;
    asm volatile (
        "leal %1, %0\n\t"
        : "=r"(ptr)
        : "m"(r2)                /* Treat register as memory */
        : 
    );
    
    g_checksum += result + (int)(intptr_t)ptr;
}

/* Test 4: Secondary reload triggers */
void test_secondary_reload_trigger(void) {
    /* Different architectures need different approaches */
    
#if defined(__arm__) || defined(__aarch64__)
    /* ARM: Try to access system/coprocessor registers */
    uint32_t control_reg;
    
    asm volatile (
        "mrc p15, 0, %0, c1, c0, 0\n\t"
        : "=r"(control_reg)
        : 
        : 
    );
    
    g_checksum += control_reg;
    
#elif defined(__x86_64__) || defined(__i386__)
    /* x86: Force memory constraints with register pressure */
    int values[4] = {1, 2, 3, 4};
    int results[4];
    
    /* Create register pressure and memory constraints */
    for (int i = 0; i < 4; i++) {
        asm volatile (
            "movl (%1), %0\n\t"
            "addl $10, %0\n\t"
            : "=r"(results[i])
            : "r"(&values[i])
            : "memory"
        );
    }
    
    /* Mix operand sizes */
    char c = 65;
    long long ll = 0x123456789ABCDEF0LL;
    long long ll_result;
    
    asm volatile (
        "movsbl %1, %k0\n\t"
        "addq %2, %0\n\t"
        : "=r"(ll_result)
        : "r"(c), "r"(ll)
        : 
    );
    
    g_checksum += results[0] + results[1] + results[2] + results[3] + (int)ll_result;
    
#elif defined(__powerpc__) || defined(__ppc__)
    /* PowerPC: Use multiple register constraints */
    int r1, r2, r3;
    
    asm volatile (
        "mr %0, %1\n\t"
        "mr %2, %0\n\t"
        : "=r"(r1), "=r"(r2), "=r"(r3)
        : "i"(0x1234)
        : 
    );
    
    g_checksum += r1 + r2 + r3;
#endif
}

/* Test 5: Complex addressing modes */
void test_addressing_modes(void) {
    struct {
        int a;
        int b[10];
        char c;
    } s = {0};
    
    /* Force base+index addressing with reload */
    for (int i = 0; i < 10; i++) {
        int idx = compute_index() % 10;
        asm volatile (
            "movl %1, %0\n\t"
            : "=r"(s.b[i])
            : "m"(s.b[idx])
            : 
        );
    }
    
    /* Different sized accesses to same location */
    short *sp = (short *)&s.a;
    asm volatile (
        "movw %1, %0\n\t"
        : "=r"(*sp)
        : "i"(0x55AA)
        : 
    );
    
    g_checksum += s.a + s.b[0] + s.c;
}

/* Test 6: Register spilling due to constraint conflicts */
void test_register_spilling(void) {
    /* Create many live values to increase register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    int r1, r2, r3, r4, r5, r6, r7, r8;
    
    /* Force all to be live and used in asm with constraints */
    asm volatile (
        "addl %9, %1\n\t"
        "addl %10, %2\n\t"
        "addl %11, %3\n\t"
        "addl %12, %4\n\t"
        "addl %13, %5\n\t"
        "addl %14, %6\n\t"
        "addl %15, %7\n\t"
        "addl %16, %8\n\t"
        : "=r"(r1), "=r"(r2), "=r"(r3), "=r"(r4),
          "=r"(r5), "=r"(r6), "=r"(r7), "=r"(r8)
        : "0"(v1), "r"(v2), "r"(v3), "r"(v4),
          "r"(v5), "r"(v6), "r"(v7), "r"(v8)
        : 
    );
    
    g_checksum += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
}

int main(void) {
    printf("Starting reload coverage test...\n");
    
    /* Initialize global arrays */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
        global_floats[i] = i * 0.1f;
    }
    
    /* Run tests multiple times to increase coverage chance */
    for (int iteration = 0; iteration < 1000; iteration++) {
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_addressing_modes();
        test_register_spilling();
        
        /* Prevent loop unrolling from eliminating reloads */
        if (iteration % 100 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    printf("Final checksum: %d\n", g_checksum);
    printf("Test completed.\n");
    
    return 0;
}
