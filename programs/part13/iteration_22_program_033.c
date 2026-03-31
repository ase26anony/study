/* Test program to trigger reload.cc:1381-1399 coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Complex function to force temporary evaluation */
int compute_index(int base) {
    return (base * 3 + 7) % 13;
}

/* Function with side effects */
int side_effect(int x) {
    global_counter++;
    return x ^ 0x55AA55AA;
}

/* ===== Test 1: Inline Assembly with Conflicting Constraints ===== */
void test_asm_constraint_conflict(void) {
    int input1 = 12345;
    int input2 = 67890;
    int output1, output2, output3;
    
    /* Force reload by requiring specific hard registers */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)        /* Must be in eax */
        : "mr"(input1 + global_counter)  /* Memory or register */
        : 
    );
    
    /* Early-clobber constraint forces reload */
    asm volatile (
        "addl %2, %0\n\t"
        "subl %1, %0\n\t"
        : "=&r"(output2)       /* Early clobber */
        : "r"(input1), "mr"(input2 + side_effect(1))
        : "cc"
    );
    
    /* Mixed register classes */
    asm volatile (
        "imull %1, %0\n\t"
        : "=r"(output3)
        : "r"(output1), "0"(output2)  /* Input/output same register */
        : "cc"
    );
    
    global_result ^= output1 + output2 + output3;
}

/* ===== Test 2: Built-in Functions with Complex Operands ===== */
void test_builtin_complex_operand(void) {
    static int array[100] = {0};
    int i, j;
    
    /* Initialize array with pattern */
    for (i = 0; i < 100; i++) {
        array[i] = i * 3 + 7;
    }
    
    /* Built-in with complex address computation */
    for (i = 0; i < 50; i++) {
        j = compute_index(i);
        int cnt = __builtin_popcount(array[j] + side_effect(i));
        global_result += cnt;
    }
    
    /* Multiple built-ins in sequence */
    for (i = 0; i < 20; i++) {
        unsigned int x = array[i] ^ array[i+10];
        int leading = __builtin_clz(x);
        int trailing = __builtin_ctz(x | 1);  /* Avoid zero */
        global_result ^= leading + trailing;
    }
}

/* ===== Test 3: Register Variable Abuse ===== */
void test_register_variable_abuse(void) {
    /* Declare register variables */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    register int r3 asm("edx");
    
    r1 = 1000 + global_counter;
    r2 = 2000 + global_counter;
    r3 = 3000 + global_counter;
    
    int temp;
    
    /* Force conflict: use register variable in asm requiring different reg */
    asm volatile (
        "movl %%ebx, %0\n\t"
        "addl %%ecx, %0\n\t"
        "addl %%edx, %0\n\t"
        : "=r"(temp)
        : /* No inputs, using fixed registers directly */
        : "ebx", "ecx", "edx"
    );
    
    /* Now use the register variables in conflicting ways */
    asm volatile (
        "movl %1, %%eax\n\t"
        "imull %2, %%eax\n\t"
        "addl %3, %%eax\n\t"
        : "=a"(temp)
        : "r"(r1), "r"(r2), "mr"(r3 + global_counter)
        : "cc"
    );
    
    global_result += temp;
}

/* ===== Test 4: Secondary Reload Triggers ===== */
void test_secondary_reload_trigger(void) {
    double d1 = 3.14159 + global_counter;
    double d2 = 2.71828 + global_counter;
    double d3, d4;
    
    /* Floating point operations that might need secondary reloads */
    asm volatile (
        "addsd %1, %0\n\t"
        : "=x"(d3)
        : "xm"(d1), "0"(d2)
    );
    
    /* Memory constraint with complex address */
    struct {
        double x;
        double y;
    } point = {1.234, 5.678};
    
    asm volatile (
        "movsd %1, %0\n\t"
        : "=x"(d4)
        : "m"(point.x + global_counter)  /* Complex memory operand */
        : 
    );
    
    /* Integer to/from float moves */
    int i1 = 42 + global_counter;
    double d5;
    
    asm volatile (
        "cvtsi2sd %1, %0\n\t"
        : "=x"(d5)
        : "rm"(i1)
    );
    
    global_result += (int)(d3 + d4 + d5);
}

/* ===== Test 5: Mixed Size Operands ===== */
void test_mixed_size_operands(void) {
    char c1 = 'A' + (global_counter % 26);
    short s1 = 1000 + global_counter;
    int i1 = 1000000 + global_counter;
    long long ll1 = 1000000000LL + global_counter;
    
    int result;
    
    /* Mixed sizes in same asm */
    asm volatile (
        "movsbl %1, %%eax\n\t"
        "addw %2, %%ax\n\t"
        "addl %3, %%eax\n\t"
        "addq %4, %%rax\n\t"
        : "=a"(result)
        : "r"(c1), "r"(s1), "r"(i1), "r"(ll1)
        : "cc"
    );
    
    /* Atomic operation with complex address */
    static long atomic_var = 0;
    long old_val = __atomic_fetch_add(&atomic_var, result, __ATOMIC_SEQ_CST);
    
    global_result += old_val;
}

/* ===== Test 6: High Register Pressure ===== */
void test_high_register_pressure(void) {
    /* Many live variables to increase register pressure */
    int v1 = global_counter * 1;
    int v2 = global_counter * 2;
    int v3 = global_counter * 3;
    int v4 = global_counter * 4;
    int v5 = global_counter * 5;
    int v6 = global_counter * 6;
    int v7 = global_counter * 7;
    int v8 = global_counter * 8;
    int v9 = global_counter * 9;
    int v10 = global_counter * 10;
    
    /* Chain of operations forcing spills and reloads */
    asm volatile (
        "addl %1, %0\n\t"
        "addl %2, %0\n\t"
        "addl %3, %0\n\t"
        "addl %4, %0\n\t"
        "addl %5, %0\n\t"
        : "+r"(v1)
        : "r"(v2), "r"(v3), "r"(v4), "r"(v5), "r"(v6)
        : "cc"
    );
    
    asm volatile (
        "imull %1, %0\n\t"
        "imull %2, %0\n\t"
        : "+r"(v7)
        : "r"(v8), "rm"(v9 + v10)
        : "cc"
    );
    
    global_result += v1 + v7;
}

/* ===== Test 7: Memory Constraints ===== */
void test_memory_constraints(void) {
    struct LargeStruct {
        int data[20];
    };
    
    static struct LargeStruct ls1, ls2;
    int i, sum = 0;
    
    /* Initialize */
    for (i = 0; i < 20; i++) {
        ls1.data[i] = i * 2 + global_counter;
        ls2.data[i] = i * 3 + global_counter;
    }
    
    /* Force memory operands with complex addressing */
    for (i = 0; i < 10; i++) {
        int idx = compute_index(i);
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            : "=a"(sum)
            : "m"(ls1.data[idx]), "m"(ls2.data[idx + 5])
            : "cc"
        );
        global_result += sum;
    }
}

int main(void) {
    int i;
    
    printf("Starting reload coverage test...\n");
    
    /* Run tests multiple times to increase coverage chance */
    for (i = 0; i < 1000; i++) {
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_mixed_size_operands();
        test_high_register_pressure();
        test_memory_constraints();
        
        /* Update global counter to vary behavior */
        global_counter = (global_counter * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    printf("Final result: %d\n", global_result);
    printf("Global counter: %d\n", global_counter);
    
    return global_result != 0 ? 0 : 1;
}
