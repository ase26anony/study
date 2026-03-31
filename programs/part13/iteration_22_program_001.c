/* test_reload.c - Comprehensive test to trigger GCC's reload pass */
/* Compile with: gcc -O1 -fno-omit-frame-pointer test_reload.c -o test_reload */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global volatile to prevent optimization */
volatile int global_volatile = 0;

/* Complex function to force expression evaluation */
static int compute_index(void) {
    static int counter = 0;
    return (counter++ * 7) & 0xF;
}

/* Array for memory operand testing */
static int global_array[32] = {0};

/* ========== Test 1: Inline Assembly with Conflicting Constraints ========== */
void test_asm_constraint_conflict(void) {
    int input1 = 12345;
    int input2 = 67890;
    int output1, output2, output3;
    
    /* Force reload by requiring specific register for output */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)        /* Must be in eax */
        : "mr"(input1 + 42)    /* Memory or register */
        : 
    );
    
    /* Early-clobber constraint forces reload */
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
        : "=r"(output3)
        : "0"(output1), "r"(input2)  /* Output tied to input 0 */
        : 
    );
    
    global_volatile += output1 + output2 + output3;
}

/* ========== Test 2: Built-in Functions with Complex Operands ========== */
void test_builtin_complex_operand(void) {
    unsigned int x = 0xDEADBEEF;
    unsigned int y = 0xCAFEBABE;
    int result;
    
    /* Builtin with function call as argument */
    result = __builtin_popcount(x ^ compute_index());
    global_volatile += result;
    
    /* Builtin with memory access */
    result = __builtin_ctz(global_array[compute_index()] | 1);
    global_volatile += result;
    
    /* Multiple builtins with interdependent results */
    int a = __builtin_ffs(x);
    int b = __builtin_clz(y);
    result = __builtin_popcount(a * b);
    global_volatile += result;
}

/* ========== Test 3: Register Variable Abuse ========== */
void test_register_variable_abuse(void) {
    /* Register variables with specific register constraints */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    register int r3 asm("edx");
    
    r1 = 100;
    r2 = 200;
    r3 = 300;
    
    int temp;
    
    /* Force conflict: use register variable in asm requiring different register */
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
        : "m"(r2)              /* Memory operand for register variable */
        : 
    );
    
    global_volatile += temp + (int)(intptr_t)ptr + r3;
}

/* ========== Test 4: Secondary Reload Triggers ========== */
void test_secondary_reload_trigger(void) {
    double d1 = 3.14159;
    double d2 = 2.71828;
    double result;
    
    /* Floating point operations often need secondary reloads */
    asm volatile (
        "addsd %1, %0\n\t"
        : "=x"(result)         /* XMM register */
        : "xm"(d1), "0"(d2)    /* Memory or XMM register */
        : 
    );
    
    /* Atomic operation with complex address */
    int atomic_var = 42;
    __atomic_fetch_add(&atomic_var, global_array[compute_index()], __ATOMIC_SEQ_CST);
    
    global_volatile += (int)result + atomic_var;
}

/* ========== Test 5: Mixed Size Operands ========== */
void test_mixed_size_operands(void) {
    char c = 'A';
    short s = 1234;
    int i = 56789;
    long long ll = 9876543210LL;
    
    int result1, result2;
    
    /* Mixed sizes in same asm statement */
    asm volatile (
        "movsbl %1, %0\n\t"
        "addw %2, %w0\n\t"
        "addl %3, %0\n\t"
        : "=r"(result1)
        : "r"(c), "r"(s), "r"(i)
        : 
    );
    
    /* 64-bit operand on 32-bit target (if compiled as 32-bit) */
    asm volatile (
        "movl %%eax, %%eax\n\t"  /* Dummy to use the value */
        : "=a"(result2)
        : "A"(ll)               /* 64-bit in eax:edx */
        : "edx"
    );
    
    global_volatile += result1 + result2;
}

/* ========== Test 6: High Register Pressure ========== */
void test_high_register_pressure(void) {
    /* Many live variables to increase register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    
    /* Complex expression using all variables */
    int sum = v1 + v2 + v3 + v4 + v5 + 
              v6 + v7 + v8 + v9 + v10 +
              v11 + v12 + v13 + v14 + v15;
    
    /* Inline asm that uses many different registers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "addl %3, %%eax\n\t"
        "addl %4, %%eax\n\t"
        "addl %5, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(sum)
        : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5)
        : "eax"
    );
    
    global_volatile += sum;
}

/* ========== Main Test Driver ========== */
int main(void) {
    int i;
    
    /* Initialize global array */
    for (i = 0; i < 32; i++) {
        global_array[i] = i * 3;
    }
    
    /* Run tests multiple times to increase coverage probability */
    for (i = 0; i < 1000; i++) {
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_mixed_size_operands();
        test_high_register_pressure();
        
        /* Prevent loop unrolling from simplifying reloads */
        if (i % 100 == 0) {
            global_volatile += compute_index();
        }
    }
    
    printf("Final checksum: %d\n", global_volatile);
    return 0;
}
