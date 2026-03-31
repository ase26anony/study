/* test_reload.c - Comprehensive test to trigger GCC reload pass logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_checksum = 0;

/* Function to create complex expressions */
int compute_index(void) {
    static int counter = 0;
    return (counter++ * 7 + 3) & 0xF;
}

int complex_expr(int a, int b) {
    return (a * b + (a >> 3) - (b << 2)) & 0xFF;
}

/* Test 1: Inline assembly with conflicting constraints */
void test_asm_constraint_conflict(void) {
    int input1 = 12345;
    int input2 = 67890;
    int output1, output2, output3;
    
    /* Force reload by requiring specific registers for output */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)        /* Must be in eax */
        : "mr"(input1 + 42)    /* Memory or register, complex expr */
        : 
    );
    
    /* Early-clobber constraint forcing reload */
    asm volatile (
        "addl %2, %0\n\t"
        "subl %1, %0\n\t"
        : "=&r"(output2)       /* Early clobber */
        : "r"(input1), "r"(input2)
        : 
    );
    
    /* Multiple constraints with register pressure */
    int temp = input1 * 2;
    asm volatile (
        "imull %2, %1\n\t"
        "addl %1, %0\n\t"
        : "+r"(output3), "=&a"(temp)
        : "rm"(input2)
        : "cc"
    );
    
    global_checksum += output1 + output2 + output3 + temp;
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    unsigned int values[16];
    for (int i = 0; i < 16; i++) {
        values[i] = (i * 137) & 0xFFFF;
    }
    
    /* Complex addressing in builtin operand */
    int cnt1 = __builtin_popcount(values[compute_index()]);
    
    /* Nested function call in builtin */
    int cnt2 = __builtin_ctz(complex_expr(255, 127) + 1);
    
    /* Atomic operation with complex address */
    int index = compute_index();
    __atomic_fetch_add(&values[index], cnt1 + cnt2, __ATOMIC_RELAXED);
    
    global_checksum += cnt1 + cnt2 + values[index];
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(void) {
    /* Declare register variables */
    register int r1 asm("ebx");
    register int r2 asm("esi");
    register int r3 asm("edi");
    
    r1 = 1000 + global_counter;
    r2 = 2000 + global_counter;
    r3 = 3000 + global_counter;
    
    int result;
    
    /* Force conflict: use register variable in asm requiring different reg */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(result)
        : "r"(r1), "r"(r2)  /* r1 is ebx, but constraint is general reg */
        : "eax"
    );
    
    /* Try to take address (will generate warning but test reload) */
    int *ptr;
    asm volatile (
        "leal %1, %0\n\t"
        : "=r"(ptr)
        : "r"(r3)
    );
    
    global_checksum += result + (int)(intptr_t)ptr;
}

/* Test 4: Architecture-specific secondary reload triggers */
void test_secondary_reload_trigger(void) {
    double d1 = 3.14159 + global_counter;
    double d2 = 2.71828 + global_counter;
    double result;
    
    /* Floating point operations that might need secondary reloads */
    asm volatile (
        "addsd %1, %0\n\t"
        : "=x"(result)
        : "xm"(d1), "0"(d2)
    );
    
    /* Memory constraint with offset that might need reload */
    struct {
        int a;
        double b;
        int c;
    } s = {1, d1, 2};
    
    double d3;
    asm volatile (
        "movsd %1, %0\n\t"
        : "=x"(d3)
        : "m"(s.b)
    );
    
    /* Mixed size operands */
    long long ll1 = 0x123456789ABCDEF0LL + global_counter;
    int i1;
    asm volatile (
        "movl %%eax, %0\n\t"
        : "=r"(i1)
        : "A"(ll1)  /* edx:eax pair */
        : "eax", "edx"
    );
    
    global_checksum += (int)result + (int)d3 + i1;
}

/* Test 5: Multiple reloads in loops */
void test_multiple_reloads_in_loop(void) {
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i * 3;
    }
    
    int sum = 0;
    
    /* Loop with inline asm causing register pressure */
    for (int i = 0; i < 50; i++) {
        int idx1 = compute_index();
        int idx2 = (i * 7) % 100;
        int temp;
        
        asm volatile (
            "movl (%1), %%eax\n\t"
            "addl (%2), %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r"(temp)
            : "r"(&array[idx1]), "r"(&array[idx2])
            : "eax", "memory"
        );
        
        sum += temp;
        
        /* Another asm with different constraints */
        asm volatile (
            "imull %1, %0\n\t"
            : "+r"(sum)
            : "rm"(i + 1)
            : "cc"
        );
    }
    
    global_checksum += sum;
}

/* Test 6: Complex expression chains */
void test_complex_expression_chains(void) {
    volatile int a = 10, b = 20, c = 30;
    int r1, r2, r3, r4;
    
    /* Chain of operations requiring temporaries */
    r1 = (a + b) * (c - a);
    r2 = (r1 >> 3) | (b << 2);
    
    /* Inline asm in the middle of expression */
    asm volatile (
        "bsrl %1, %0\n\t"
        : "=r"(r3)
        : "r"(r2)
        : "cc"
    );
    
    r4 = complex_expr(r1, r3) + __builtin_popcount(r2);
    
    /* Force memory operand with complex addressing */
    struct {
        int x[4];
        int y[4];
    } data;
    
    for (int i = 0; i < 4; i++) {
        data.x[i] = r1 + i;
        data.y[i] = r3 - i;
    }
    
    int final;
    asm volatile (
        "movl (%1, %2, 4), %%eax\n\t"
        "addl 16(%1, %2, 4), %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(final)
        : "r"(&data.x[0]), "r"(compute_index() & 3)
        : "eax"
    );
    
    global_checksum += r4 + final;
}

int main(void) {
    printf("Starting reload coverage test...\n");
    
    /* Run tests multiple times to increase coverage chance */
    for (int iteration = 0; iteration < 100; iteration++) {
        global_counter = iteration;
        
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        
        /* x86-specific register variable test */
#if defined(__i386__) || defined(__x86_64__)
        test_register_variable_abuse();
#endif
        
        test_secondary_reload_trigger();
        test_multiple_reloads_in_loop();
        test_complex_expression_chains();
        
        /* Prevent loop unrolling from optimizing away reloads */
        asm volatile ("" : : : "memory");
    }
    
    printf("Final checksum: %d\n", global_checksum);
    printf("Test completed.\n");
    
    return 0;
}
