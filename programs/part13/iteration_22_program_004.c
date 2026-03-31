/* test_reload.c - Comprehensive test to trigger GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile global to prevent optimization */
volatile int g_checksum = 0;

/* ========== Test 1: Inline Assembly with Conflicting Constraints ========== */
void test_asm_constraint_conflict(int iterations) {
    int i;
    for (i = 0; i < iterations; i++) {
        int input1 = i * 3 + 1;
        int input2 = i * 7 - 2;
        int output1, output2;
        
        /* Force reload: output requires specific register (eax), 
           input is complex expression that likely won't be in eax */
        asm volatile (
            "movl %1, %0\n\t"
            : "=a"(output1)        /* Must be in eax */
            : "mr"(input1 + input2 * 2)  /* Complex expression */
            : 
        );
        
        /* Early-clobber constraint forces reload */
        asm volatile (
            "addl %2, %0\n\t"
            "movl %0, %1\n\t"
            : "=&r"(output1), "=r"(output2)  /* & = early clobber */
            : "r"(input1), "0"(output1)
            : 
        );
        
        /* Mix different register classes */
        register int r1 asm("ebx") = input1;
        asm volatile (
            "xchgl %%ebx, %0\n\t"
            : "=r"(output1)
            : "0"(r1)
            : "ebx"
        );
        
        g_checksum += output1 + output2;
    }
}

/* ========== Test 2: Complex Built-in Function Operands ========== */
int compute_index(int i) {
    return (i * 13) % 17;
}

void test_builtin_complex_operand(int iterations) {
    int i;
    static int global_array[32] = {0};
    
    /* Initialize array */
    for (i = 0; i < 32; i++) {
        global_array[i] = i * i;
    }
    
    for (i = 0; i < iterations; i++) {
        /* Complex operand: array access with function call */
        int cnt = __builtin_popcount(global_array[compute_index(i)]);
        
        /* Another complex built-in with composite expression */
        double x = (double)i + 0.5;
        double root = __builtin_sqrt(x * x + 1.0);
        
        /* Atomic built-in with complex address */
        int* ptr = &global_array[i % 32];
        int old = __atomic_fetch_add(ptr, cnt, __ATOMIC_SEQ_CST);
        
        g_checksum += cnt + (int)root + old;
    }
}

/* ========== Test 3: Register Variable Abuse ========== */
void test_register_variable_abuse(int iterations) {
    int i;
    for (i = 0; i < iterations; i++) {
        /* Declare register variables */
        register int r1 asm("esi") = i * 2;
        register int r2 asm("edi") = i * 3;
        
        /* Force conflict: use register variable in asm requiring different reg */
        int result;
        asm volatile (
            "movl %%esi, %%eax\n\t"
            "addl %%edi, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r"(result)
            : 
            : "eax", "esi", "edi"
        );
        
        /* Take address indirectly (GCC extension with warning) */
        int* ptr;
        asm volatile (
            "leal %1, %0\n\t"
            : "=r"(ptr)
            : "r"(r1)
            : 
        );
        
        g_checksum += result + *ptr;
    }
}

/* ========== Test 4: Secondary Reload Triggers ========== */
void test_secondary_reload_trigger(int iterations) {
    int i;
    
    /* Architecture-specific secondary reload triggers */
    #if defined(__arm__) || defined(__aarch64__)
    /* ARM: System register access often requires secondary reloads */
    for (i = 0; i < iterations; i++) {
        unsigned int cpsr;
        asm volatile (
            "mrs %0, cpsr\n\t"
            : "=r"(cpsr)
            : 
            : 
        );
        g_checksum += cpsr & 0xFF;
    }
    #elif defined(__x86_64__) || defined(__i386__)
    /* x86: Control register access */
    for (i = 0; i < iterations; i++) {
        unsigned int cr0;
        asm volatile (
            "mov %%cr0, %0\n\t"
            : "=r"(cr0)
            : 
            : 
        );
        g_checksum += cr0 & 0xFF;
        
        /* x87 FPU stack manipulation - often needs reloads */
        double x = i * 1.5;
        double y;
        asm volatile (
            "fldl %1\n\t"
            "fsqrt\n\t"
            "fstpl %0\n\t"
            : "=m"(y)
            : "m"(x)
            : "st", "st(1)"
        );
        g_checksum += (int)y;
    }
    #endif
    
    /* Memory constraints with register-only operations */
    for (i = 0; i < iterations; i++) {
        int arr[4] = {i, i+1, i+2, i+3};
        int sum;
        
        /* Force memory operand into instruction that might prefer register */
        asm volatile (
            "movl (%1), %0\n\t"
            "addl 4(%1), %0\n\t"
            "addl 8(%1), %0\n\t"
            "addl 12(%1), %0\n\t"
            : "=r"(sum)
            : "r"(arr)
            : "memory"
        );
        
        g_checksum += sum;
    }
}

/* ========== Test 5: Mixed Modes and Addressing ========== */
void test_mixed_modes_addressing(int iterations) {
    int i;
    for (i = 0; i < iterations; i++) {
        /* Mixed sizes in same asm */
        char c = 'A' + (i % 26);
        long long ll = (long long)i * 1000000;
        int result;
        
        asm volatile (
            "movsbl %1, %%eax\n\t"
            "addl %%eax, %k2\n\t"
            "movl %k2, %0\n\t"
            : "=r"(result)
            : "r"(c), "r"((int)ll)
            : "eax"
        );
        
        /* Complex addressing mode */
        struct {
            int a;
            int b[10];
        } s;
        s.a = i;
        for (int j = 0; j < 10; j++) {
            s.b[j] = i + j;
        }
        
        int idx = i % 10;
        int val;
        asm volatile (
            "movl %c2(%1), %0\n\t"
            : "=r"(val)
            : "r"(&s.b[0]), "n"(idx * sizeof(int))
            : "memory"
        );
        
        g_checksum += result + val;
    }
}

/* ========== Main Driver ========== */
int main(int argc, char** argv) {
    int iterations = 1000;
    
    printf("Starting reload pass coverage test...\n");
    
    test_asm_constraint_conflict(iterations);
    printf("Test 1 complete, checksum = %d\n", g_checksum);
    
    test_builtin_complex_operand(iterations);
    printf("Test 2 complete, checksum = %d\n", g_checksum);
    
    test_register_variable_abuse(iterations);
    printf("Test 3 complete, checksum = %d\n", g_checksum);
    
    test_secondary_reload_trigger(iterations);
    printf("Test 4 complete, checksum = %d\n", g_checksum);
    
    test_mixed_modes_addressing(iterations);
    printf("Test 5 complete, checksum = %d\n", g_checksum);
    
    printf("All tests complete. Final checksum = %d\n", g_checksum);
    
    return 0;
}
