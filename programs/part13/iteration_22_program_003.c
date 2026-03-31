/* test_reload.c - Comprehensive test to trigger reload.cc push_reload logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_checksum = 0;

/* Function to create complex expressions */
int compute_index(void) {
    static int counter = 0;
    return (counter++ * 37 + 123) & 0xFF;
}

/* Test 1: Inline assembly with conflicting constraints */
void test_asm_constraint_conflict(void) {
    int input1 = 42;
    int input2 = 73;
    int output1, output2, output3;
    
    /* Force reload by requiring specific register for output */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)        /* Must be in eax */
        : "mr"(input1 + 17)    /* Memory or register expression */
        : 
    );
    
    /* Early-clobber constraint forces reload */
    asm volatile (
        "addl %2, %0\n\t"
        "movl %0, %1\n\t"
        : "=&r"(output2), "=r"(output3)  /* Early clobber on output2 */
        : "r"(input2), "0"(output1)
        : 
    );
    
    /* Mixing register classes */
    double dinput = 3.14159;
    double doutput;
    asm volatile (
        "movsd %1, %0\n\t"
        : "=x"(doutput)        /* XMM register */
        : "m"(dinput)          /* Memory operand */
        : 
    );
    
    global_checksum += output1 + output2 + output3 + (int)doutput;
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    static int array[256] = {0};
    for (int i = 0; i < 256; i++) {
        array[i] = i * i;
    }
    
    /* Complex expression as builtin argument */
    int cnt = __builtin_popcount(array[compute_index()] + global_counter);
    
    /* Multiple complex arguments */
    unsigned long val = (unsigned long)&array[128];
    int trailing = __builtin_ctz(val | 1);  /* Force non-zero */
    
    /* Math builtin with complex argument */
    double x = (double)cnt * 1.5;
    double root = __builtin_sqrt(x + 3.0);
    
    global_checksum += cnt + trailing + (int)root;
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(void) {
    /* Declare register variables */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    
    r1 = 100;
    r2 = 200;
    
    /* Force conflict by using in asm requiring different register */
    int result;
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(result)
        : "r"(r1), "r"(r2)
        : "%eax"  /* Clobber eax */
    );
    
    /* Try to take address (GCC extension with warning) */
    int *ptr;
    asm volatile (
        "leal %1, %0\n\t"
        : "=r"(ptr)
        : "r"(r1)
        : 
    );
    
    global_checksum += result + (int)ptr;
}

/* Test 4: Atomic operations with complex addresses */
void test_atomic_complex_address(void) {
    struct {
        int data[64];
        int counter;
    } mystruct;
    
    for (int i = 0; i < 64; i++) {
        mystruct.data[i] = i * 3;
    }
    mystruct.counter = 0;
    
    /* Atomic operation with complex address computation */
    int idx = compute_index() & 63;
    int old = __atomic_fetch_add(&mystruct.data[idx], 5, __ATOMIC_SEQ_CST);
    
    /* Another with offset */
    __atomic_add_fetch(&mystruct.counter, old, __ATOMIC_SEQ_CST);
    
    global_checksum += mystruct.counter;
}

/* Test 5: Architecture-specific secondary reload triggers */
void test_secondary_reload_trigger(void) {
#ifdef __arm__
    /* ARM-specific: System register access often needs secondary reload */
    unsigned int control_reg;
    asm volatile (
        "mrc p15, 0, %0, c1, c0, 0\n\t"
        : "=r"(control_reg)
        : 
        : 
    );
    
    /* NEON to ARM register move may need secondary reload */
    float fval = 1.5f;
    int ival;
    asm volatile (
        "vmov.f32 s0, %1\n\t"
        "vmov %0, s0\n\t"
        : "=r"(ival)
        : "t"(fval)  /* 't' = VFP register */
        : "s0"
    );
    
    global_checksum += control_reg + ival;
    
#elif defined(__x86_64__)
    /* x86-64: Control register access */
    unsigned long cr0;
    asm volatile (
        "mov %%cr0, %0\n\t"
        : "=r"(cr0)
        : 
        : 
    );
    
    /* Mix x87 and SSE registers */
    double d1 = 2.71828;
    double d2;
    asm volatile (
        "fldl %1\n\t"
        "fstpl %0\n\t"
        : "=m"(d2)
        : "m"(d1)
        : "st", "st(1)"
    );
    
    global_checksum += (int)cr0 + (int)d2;
    
#else
    /* Generic: Force memory constraints with register pressure */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    int result;
    
    /* Many register operands to create pressure */
    asm volatile (
        "imull %1, %2\n\t"
        "addl %3, %2\n\t"
        "subl %4, %2\n\t"
        "addl %5, %2\n\t"
        "movl %2, %0\n\t"
        : "=m"(result)  /* Memory output constraint */
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "0"(f)
        : 
    );
    
    global_checksum += result;
#endif
}

/* Test 6: Mixed-size operands and addressing modes */
void test_mixed_operands(void) {
    char c1 = 'A';
    short s1 = 1000;
    int i1 = 100000;
    long long ll1 = 10000000000LL;
    
    /* Mixed sizes in same asm */
    long long result;
    asm volatile (
        "movsbl %1, %%eax\n\t"
        "movswl %2, %%edx\n\t"
        "addl %%edx, %%eax\n\t"
        "cltq\n\t"
        "addq %3, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r"(result)
        : "m"(c1), "m"(s1), "r"(ll1)
        : "%eax", "%edx", "%rax"
    );
    
    /* Complex addressing mode */
    int array[100];
    for (int i = 0; i < 100; i++) array[i] = i;
    
    int idx1 = compute_index() % 100;
    int idx2 = (idx1 + 37) % 100;
    int sum;
    
    asm volatile (
        "movl (%1, %2, 4), %%eax\n\t"
        "addl (%3, %4, 4), %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(sum)
        : "r"(array), "r"(idx1), "r"(array), "r"(idx2)
        : "%eax"
    );
    
    global_checksum += (int)result + sum;
}

/* Main test driver */
int main(void) {
    printf("Starting reload coverage test...\n");
    
    /* Run tests multiple times to increase coverage probability */
    for (int iteration = 0; iteration < 1000; iteration++) {
        global_counter = iteration;
        
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_atomic_complex_address();
        test_secondary_reload_trigger();
        test_mixed_operands();
        
        /* Prevent loop optimization */
        if (iteration % 100 == 0) {
            printf("Progress: %d iterations, checksum = %d\n", 
                   iteration, global_checksum);
        }
    }
    
    printf("Final checksum: %d\n", global_checksum);
    printf("Test completed.\n");
    
    return 0;
}
