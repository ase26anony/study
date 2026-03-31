/* Test program to trigger reload.cc push_reload logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_checksum = 0;
volatile int g_counter = 0;

/* Global variables for memory operands */
int global_array[100] = {0};
float global_floats[50] = {0.0f};
double global_doubles[25] = {0.0};

/* Function to compute index - forces evaluation */
int compute_index(void) {
    static int idx = 0;
    return (idx++ % 10) + 1;
}

/* Complex expression function */
int complex_expr(int a, int b) {
    return (a * b) + (a / (b ? b : 1)) - (a % (b ? b : 1));
}

/* ===== Test 1: Inline Assembly with Conflicting Constraints ===== */
void test_asm_constraint_conflict(void) {
    int input1 = 12345;
    int input2 = 67890;
    int output1, output2, output3;
    
    /* Force reload by requiring specific hard registers */
    for (int i = 0; i < 100; i++) {
        /* Output must be in eax, input can be memory/register */
        asm volatile (
            "movl %1, %0\n\t"
            : "=a"(output1)        /* Must be in eax */
            : "mr"(input1 + i)     /* Memory or register */
            : /* No clobbers */
        );
        
        /* Early-clobber constraint forces separate register */
        asm volatile (
            "addl %1, %0\n\t"
            : "=&r"(output2)       /* Early clobber */
            : "r"(input2), "0"(output1)
            : "cc"
        );
        
        /* Mix different register classes */
        asm volatile (
            "imull %1, %0\n\t"
            : "+r"(output2)        /* General register */
            : "r"(input1)
            : "cc"
        );
        
        g_checksum += output1 + output2;
    }
    
    /* Force memory reload with specific register constraint */
    int *ptr = &global_array[10];
    asm volatile (
        "movl (%1), %0\n\t"
        : "=r"(output3)
        : "r"(ptr)
        : "memory"
    );
    
    g_checksum += output3;
}

/* ===== Test 2: Built-in Functions with Complex Operands ===== */
void test_builtin_complex_operand(void) {
    unsigned int val = 0x12345678;
    
    for (int i = 0; i < 500; i++) {
        /* Built-in with function call as argument */
        int popcnt = __builtin_popcount(complex_expr(val, i));
        
        /* Built-in with array access using computed index */
        int ctz = __builtin_ctz(global_array[compute_index()] + i);
        
        /* Atomic operation with complex address */
        int index = compute_index();
        __atomic_fetch_add(&global_array[index], popcnt + ctz, __ATOMIC_RELAXED);
        
        /* Math built-in with composite expression */
        double d = __builtin_sqrt(global_doubles[i % 25] + (double)i);
        
        g_checksum += popcnt + ctz + (int)d;
    }
}

/* ===== Test 3: Register Variable Abuse ===== */
void test_register_variable_abuse(void) {
    /* Declare register variables */
    register int r1 asm("ebx");
    register int r2 asm("esi");
    register int r3 asm("edi");
    
    r1 = 100;
    r2 = 200;
    r3 = 300;
    
    int temp;
    
    /* Force conflict: use register variable in asm requiring different register */
    for (int i = 0; i < 100; i++) {
        /* r1 is in ebx, but we need result in eax */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r"(temp)        /* Any general register */
            : "r"(r1 + i)       /* Comes from ebx */
            : "eax", "cc"
        );
        
        /* Use the register variable in memory context */
        asm volatile (
            "movl %1, %0\n\t"
            : "=r"(temp)
            : "m"(r2)           /* Treat register as memory - forces reload */
            : /* No clobbers */
        );
        
        g_checksum += temp + r3;
        
        /* Modify register variables */
        r1 += 1;
        r2 += 2;
        r3 += 3;
    }
}

/* ===== Test 4: Architecture-Specific Secondary Reload Triggers ===== */
void test_secondary_reload_trigger(void) {
    /* This test is architecture-specific */
    
#if defined(__arm__) || defined(__aarch64__)
    /* ARM: System register access often requires secondary reloads */
    uint32_t control_reg;
    
    asm volatile (
        "mrs %0, cpsr\n\t"
        : "=r"(control_reg)
        : /* No inputs */
        : /* No clobbers */
    );
    
    g_checksum += control_reg & 0xFF;
    
#elif defined(__x86_64__) || defined(__i386__)
    /* x86: Control register access */
    uint32_t cr0;
    
    asm volatile (
        "mov %%cr0, %0\n\t"
        : "=r"(cr0)
        : /* No inputs */
        : /* No clobbers */
    );
    
    g_checksum += cr0 & 0xFF;
    
    /* x87 floating point stack manipulation */
    double x = 3.14159;
    double y;
    
    asm volatile (
        "fldl %1\n\t"
        "fsqrt\n\t"
        "fstpl %0\n\t"
        : "=m"(y)
        : "m"(x)
        : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
    );
    
    g_checksum += (int)y;
    
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    /* PowerPC: Special purpose register access */
    uint32_t msr;
    
    asm volatile (
        "mfmsr %0\n\t"
        : "=r"(msr)
        : /* No inputs */
        : /* No clobbers */
    );
    
    g_checksum += msr & 0xFF;
#endif
    
    /* Generic test: mixing operand sizes */
    char c = 'A';
    short s = 1234;
    int i = 56789;
    long long ll = 123456789012LL;
    
    long long result;
    
    asm volatile (
        "movsxl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movsxl %%eax, %0\n\t"
        "addq %3, %0\n\t"
        : "=r"(result)
        : "r"(c), "r"(s), "r"(ll)
        : "eax", "cc"
    );
    
    g_checksum += (int)(result & 0xFFFFFFFF);
}

/* ===== Test 5: Addressing Mode Conflicts ===== */
void test_addressing_mode_conflicts(void) {
    struct {
        int a;
        int b[10];
        int c;
    } s = {0};
    
    int *ptr_array[5];
    for (int i = 0; i < 5; i++) {
        ptr_array[i] = &global_array[i * 10];
    }
    
    for (int i = 0; i < 100; i++) {
        int idx = i % 5;
        int offset = i % 10;
        int result;
        
        /* Complex addressing that may need reload */
        asm volatile (
            "movl (%1, %2, 4), %0\n\t"  /* Base + index*4 */
            : "=r"(result)
            : "r"(ptr_array[idx]), "r"(offset)
            : "memory"
        );
        
        /* Structure member access with index */
        asm volatile (
            "movl %1, %0\n\t"
            : "=r"(result)
            : "m"(s.b[offset])  /* Memory operand with displacement */
            : /* No clobbers */
        );
        
        g_checksum += result;
        
        /* Update structure to prevent optimization */
        s.b[offset] = i;
        s.a = complex_expr(s.a, i);
    }
}

/* ===== Test 6: Register Pressure and Spilling ===== */
void test_register_pressure(void) {
    /* Use many variables to increase register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    int v9 = 9, v10 = 10, v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    
    /* Force all variables to be live across asm statements */
    for (int i = 0; i < 50; i++) {
        int temp;
        
        /* Use many different variables in asm constraints */
        asm volatile (
            "addl %1, %0\n\t"
            "addl %2, %0\n\t"
            "addl %3, %0\n\t"
            : "=r"(temp)
            : "r"(v1 + i), "r"(v2 + i), "r"(v3 + i)
            : "cc"
        );
        
        asm volatile (
            "imull %1, %0\n\t"
            : "+r"(temp)
            : "r"(v4)
            : "cc"
        );
        
        /* Chain dependencies to prevent reordering */
        v1 = v2 + v3;
        v2 = v4 + v5;
        v3 = v6 + v7;
        v4 = v8 + v9;
        v5 = v10 + v11;
        v6 = v12 + v13;
        v7 = v14 + v15;
        
        /* Use results to prevent dead code elimination */
        g_checksum += temp + v1 + v2 + v3 + v4 + v5 + v6 + v7;
    }
}

/* ===== Main Function ===== */
int main(void) {
    printf("Starting reload coverage test...\n");
    
    /* Initialize global arrays */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 3;
    }
    for (int i = 0; i < 50; i++) {
        global_floats[i] = i * 1.5f;
    }
    for (int i = 0; i < 25; i++) {
        global_doubles[i] = i * 2.7;
    }
    
    /* Run all tests multiple times */
    for (int iteration = 0; iteration < 10; iteration++) {
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_addressing_mode_conflicts();
        test_register_pressure();
        
        g_counter++;
    }
    
    printf("Final checksum: %d\n", g_checksum);
    printf("Test completed.\n");
    
    return g_checksum != 0 ? 0 : 1;
}
