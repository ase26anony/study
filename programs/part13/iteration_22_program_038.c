/* Test program to trigger reload.cc push_reload uncovered lines */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Complex function to force register pressure */
int compute_index(int base) {
    return (base * 1103515245 + 12345) & 0x7fffffff;
}

/* Test 1: Inline assembly with conflicting constraints */
void test_asm_constraint_conflict(void) {
    int input1 = 42;
    int input2 = 100;
    int output1, output2, output3;
    
    /* Force reload by requiring specific registers */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)        /* Must be in eax */
        : "mr"(input1 + 123)   /* Memory or register */
        : 
    );
    
    /* Early clobber forces reload */
    asm volatile (
        "addl %2, %0\n\t"
        "movl %0, %1\n\t"
        : "=&r"(output2), "=r"(output3)  /* Early clobber on output2 */
        : "r"(input2), "0"(output1)
        : 
    );
    
    /* Mixed register classes */
    register int r1 asm("ebx") = 777;
    asm volatile (
        "xchgl %0, %1\n\t"
        : "+r"(r1), "+m"(input1)
        : 
        : 
    );
    
    global_result ^= output1 ^ output2 ^ output3 ^ r1 ^ input1;
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    static int array[256] = {0};
    int idx;
    
    /* Initialize array */
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    /* Complex operand for builtin */
    idx = compute_index(global_counter);
    int cnt = __builtin_popcount(array[idx % 256] + global_counter);
    
    /* Another builtin with complex address */
    long long value = 0x123456789ABCDEF0LL;
    int trailing = __builtin_ctzll(value + global_counter);
    
    /* Atomic builtin with complex address */
    int atomic_var = 0;
    __atomic_fetch_add(&atomic_var, array[trailing % 256], __ATOMIC_SEQ_CST);
    
    global_result += cnt + trailing + atomic_var;
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(void) {
    /* Declare register variables */
    register int reg1 asm("esi") = 111;
    register int reg2 asm("edi") = 222;
    register double reg_fpu asm("st(0)") = 3.14159;
    
    int normal_var = 333;
    
    /* Force conflict between register variable and asm constraint */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0\n\t"
        : "+r"(reg1)           /* reg1 is in esi, but we use eax */
        : "r"(reg2)
        : "%eax"               /* Clobber eax */
    );
    
    /* Try to take address (will generate warning but compiles) */
    int *ptr;
    asm volatile (
        "leal %1, %0\n\t"
        : "=r"(ptr)
        : "r"(reg1)
        : 
    );
    
    /* Use in memory context */
    asm volatile (
        "movl %1, (%0)\n\t"
        : 
        : "r"(ptr), "r"(normal_var)
        : "memory"
    );
    
    global_result ^= reg1 ^ reg2 ^ normal_var;
}

/* Test 4: Secondary reload triggers */
void test_secondary_reload_trigger(void) {
    int x = 1234;
    int y = 5678;
    int z;
    
    /* Different operand sizes forcing mode changes */
    char c = 'A';
    long long ll = 0xFFFFFFFFLL;
    
    /* Mixed size operands in asm */
    asm volatile (
        "movsbl %1, %%eax\n\t"
        "addl %%eax, %0\n\t"
        : "+r"(x)
        : "m"(c)               /* Memory operand of different size */
        : "%eax"
    );
    
    /* Large immediate that might need reload */
    asm volatile (
        "movl $0x12345678, %%eax\n\t"
        "addl %%eax, %0\n\t"
        : "+r"(y)
        : 
        : "%eax"
    );
    
    /* Force memory operand with complex addressing */
    struct {
        int a;
        int b[10];
    } s = {0};
    
    asm volatile (
        "movl $999, %0\n\t"
        : "=m"(s.b[x % 10])    /* Complex memory address */
        : 
        : 
    );
    
    /* Floating point to integer move that might need reload */
    double d = 2.71828;
    int id;
    asm volatile (
        "movq %1, %%xmm0\n\t"
        "cvttsd2si %%xmm0, %0\n\t"
        : "=r"(id)
        : "xm"(d)              /* SSE memory/register */
        : "%xmm0"
    );
    
    global_result += x + y + z + id + s.a;
}

/* Test 5: Multiple constraints and operand matching */
void test_multiple_constraints(void) {
    int a = 1, b = 2, c = 3, d = 4;
    int r1, r2, r3;
    
    /* Multiple alternative constraints */
    asm volatile (
        "imull %2, %0\n\t"
        : "=r"(r1), "=r"(r2)
        : "0"(a), "r"(b), "1"(c), "r"(d)
        : 
    );
    
    /* Tie input to output in non-trivial way */
    asm volatile (
        "movl %1, %0\n\t"
        "addl $100, %0\n\t"
        : "=r"(r3)
        : "0"(r1)              /* Tied to output */
        : 
    );
    
    /* Volatile asm with many operands */
    asm volatile (
        "addl %2, %0\n\t"
        "subl %3, %1\n\t"
        "xorl %0, %1\n\t"
        : "+r"(r1), "+r"(r2)
        : "r"(r3), "r"(a)
        : 
    );
    
    global_result ^= r1 ^ r2 ^ r3;
}

/* Main test driver */
int main(void) {
    printf("Starting reload coverage test...\n");
    
    /* Run tests multiple times to increase coverage chance */
    for (int i = 0; i < 1000; i++) {
        global_counter = i;
        
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_multiple_constraints();
        
        /* Prevent loop optimization */
        if (global_result > 1000000) {
            global_result = 0;
        }
    }
    
    printf("Final result: %d\n", global_result);
    printf("Test completed.\n");
    
    return 0;
}
