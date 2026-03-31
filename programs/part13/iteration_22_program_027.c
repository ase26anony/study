/* Test program to trigger reload.cc push_reload logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile to prevent optimization */
volatile int global_counter = 0;
volatile int checksum = 0;

/* Complex function to force evaluation */
int compute_index(void) {
    static int counter = 0;
    return (counter++ * 37) & 0xFF;
}

/* Function with side effects */
int complex_expression(int x) {
    return (x * 3 + 7) ^ 0x55AA;
}

/* Test 1: Inline assembly with conflicting constraints */
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
    
    /* Early clobber forces reload */
    asm volatile (
        "addl %2, %0\n\t"
        "movl %0, %1\n\t"
        : "=&r"(output2), "=r"(output3)  /* Early clobber on first output */
        : "r"(input2), "0"(output1)
        : 
    );
    
    /* Mixing register classes */
    register int r1 asm("ebx") = 100;
    asm volatile (
        "xchgl %0, %1\n\t"
        : "+r"(r1), "+m"(input1)
        :
        : "memory"
    );
    
    checksum += output1 + output2 + output3 + r1;
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    static int array[256] = {0};
    
    /* Initialize array */
    for (int i = 0; i < 256; i++) {
        array[i] = i * i;
    }
    
    /* Complex operand requiring temporary */
    int cnt = __builtin_popcount(array[compute_index()] + global_counter);
    
    /* More complex built-in usage */
    unsigned long val = (unsigned long)&array[128];
    int leading_zeros = __builtin_clz(val);
    int trailing_zeros = __builtin_ctz(val | 1);
    
    /* Atomic operation with complex address */
    int index = compute_index();
    __atomic_fetch_add(&array[index], cnt, __ATOMIC_RELAXED);
    
    checksum += cnt + leading_zeros + trailing_zeros + array[index];
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(void) {
    /* Declare register variables */
    register int reg1 asm("esi") = 1000;
    register int reg2 asm("edi") = 2000;
    register int reg3 asm("ebx") = 3000;
    
    int result1, result2;
    
    /* Force conflict between register variable and asm constraint */
    asm volatile (
        "movl %%esi, %0\n\t"
        "addl %%edi, %0\n\t"
        : "=r"(result1)   /* General reg, not necessarily esi/edi */
        : 
        : "%esi", "%edi"
    );
    
    /* Use register variable in memory context */
    int *ptr = &reg3;  /* Taking address - may force spill */
    result2 = *ptr + reg1;
    
    /* More conflicts */
    asm volatile (
        "imull %1, %0\n\t"
        : "+r"(reg2)
        : "r"(result1)
        : "%edx"
    );
    
    checksum += result1 + result2 + reg2;
}

/* Test 4: Secondary reload triggers */
void test_secondary_reload_trigger(void) {
    double d1 = 3.14159;
    double d2 = 2.71828;
    double d3, d4;
    
    /* Floating point constraints that might need secondary reloads */
    asm volatile (
        "addsd %1, %0\n\t"
        : "=x"(d3)        /* SSE register */
        : "xm"(d1)        /* SSE register or memory */
        : 
    );
    
    /* Memory constraint with complex address */
    struct {
        double values[4];
        int count;
    } data = {{1.1, 2.2, 3.3, 4.4}, 4};
    
    asm volatile (
        "movsd %1, %0\n\t"
        : "=x"(d4)
        : "m"(data.values[compute_index() % 4])
        : 
    );
    
    /* Integer to/from float moves that might need reloads */
    int int_val = 42;
    double d5;
    
    asm volatile (
        "cvtsi2sd %1, %0\n\t"
        : "=x"(d5)
        : "r"(int_val)
        : 
    );
    
    checksum += (int)(d3 + d4 + d5);
}

/* Test 5: Mixed size operands and addressing modes */
void test_mixed_operands(void) {
    char c1 = 'A';
    short s1 = 1234;
    int i1 = 56789;
    long long ll1 = 1234567890123LL;
    
    int result;
    long long llresult;
    
    /* Mixed sizes in asm */
    asm volatile (
        "movsbl %1, %0\n\t"
        "addw %2, %0\n\t"
        "addl %3, %0\n\t"
        : "=r"(result)
        : "r"(c1), "r"(s1), "r"(i1)
        : 
    );
    
    /* 64-bit operations on 32-bit arch might need special handling */
    asm volatile (
        "addq %1, %0\n\t"
        : "+r"(ll1)
        : "r"(123456789LL)
        : 
    );
    
    /* Complex addressing mode */
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i * 2;
    }
    
    asm volatile (
        "movl (%1, %2, 4), %0\n\t"
        : "=r"(result)
        : "r"(array), "r"(compute_index() % 25)
        : "memory"
    );
    
    checksum += result + (int)(ll1 & 0xFFFFFFFF);
}

/* Test 6: High register pressure to force spills and reloads */
void test_high_register_pressure(void) {
    /* Many live variables to increase register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    
    /* Chain of operations forcing register shuffling */
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
        : "%edx"
    );
    
    /* Use all variables to keep them live */
    checksum += v1 + v7 + v10 + v11 + v12 + v13 + v14 + v15;
}

int main(void) {
    printf("Starting reload coverage test...\n");
    
    /* Run tests multiple times to increase coverage chance */
    for (int iteration = 0; iteration < 1000; iteration++) {
        global_counter = iteration;
        
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_mixed_operands();
        test_high_register_pressure();
        
        /* Alternate between different patterns */
        if (iteration % 3 == 0) {
            /* Extra pressure */
            for (int i = 0; i < 10; i++) {
                test_asm_constraint_conflict();
            }
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
