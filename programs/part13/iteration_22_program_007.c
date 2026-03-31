/* test_reload.c - Comprehensive test to trigger push_reload logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile int checksum = 0;

/* Function to create complex expressions */
int compute_index(void) {
    static int counter = 0;
    return (counter++ * 7 + 3) % 100;
}

/* Test 1: Inline assembly with conflicting constraints */
void test_asm_constraint_conflict(void) {
    int input1 = 12345;
    int input2 = 67890;
    int output1, output2, output3;
    
    /* Force reload by requiring specific register for output */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)          /* Must be in eax */
        : "mr"(input1 + 42)      /* Memory or register, but not necessarily eax */
        : 
    );
    
    /* Early-clobber constraint forces reload */
    asm volatile (
        "addl %2, %0\n\t"
        "movl %0, %1\n\t"
        : "=&r"(output2), "=r"(output3)  /* Early clobber on output2 */
        : "r"(input2), "0"(input1)
        : 
    );
    
    /* Mixing register classes */
    double dinput = 3.14159;
    long long lloutput;
    
    asm volatile (
        "cvtsd2si %1, %0\n\t"
        : "=r"(lloutput)         /* General purpose register */
        : "x"(dinput)            /* Must be in SSE register */
        : 
    );
    
    checksum += output1 + output2 + output3 + (int)lloutput;
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    unsigned int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i * 3;
    }
    
    /* Complex expression as builtin argument */
    int cnt1 = __builtin_popcount(array[compute_index()] + global_counter);
    
    /* Nested function calls in builtin */
    int cnt2 = __builtin_ctz(__builtin_ffs(global_counter + 1) * 2);
    
    /* Atomic operation with complex address */
    int idx = compute_index();
    int old = __atomic_fetch_add(&array[idx], 5, __ATOMIC_SEQ_CST);
    
    checksum += cnt1 + cnt2 + old;
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(void) {
    /* Declare register variables with specific registers */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    
    r1 = 1000;
    r2 = 2000;
    
    /* Force conflict: use register variable in asm requiring different register */
    int result;
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(result)
        : "r"(r1), "r"(r2)   /* r1 is tied to ebx, but we use eax in asm */
        : "%eax"
    );
    
    /* Take address indirectly (GCC extension with warning) */
    int *ptr;
    asm volatile (
        "leal %1, %0\n\t"
        : "=r"(ptr)
        : "m"(r1)            /* 'm' constraint may force reload */
        : 
    );
    
    checksum += result + (int)(intptr_t)ptr;
}

/* Test 4: Secondary reload triggers */
void test_secondary_reload_trigger(void) {
    /* Different architectures require different approaches */
    
#if defined(__arm__) || defined(__aarch64__)
    /* ARM: System register access often requires secondary reloads */
    unsigned int control_reg;
    asm volatile (
        "mrs %0, cpsr\n\t"
        : "=r"(control_reg)
        : 
        : 
    );
    
    /* NEON to general purpose register moves */
    float32x4_t neon_vec = {1.0f, 2.0f, 3.0f, 4.0f};
    float result;
    asm volatile (
        "faddp %0, %1.4s\n\t"
        : "=w"(result)       /* NEON scalar register */
        : "w"(neon_vec)      /* NEON vector register */
        : 
    );
    
    checksum += control_reg + (int)result;
    
#elif defined(__x86_64__) || defined(__i386__)
    /* x86: Control register access */
    unsigned int cr0;
    asm volatile (
        "mov %%cr0, %0\n\t"
        : "=r"(cr0)
        : 
        : 
    );
    
    /* MMX/SSE to general purpose moves */
    __m128i vec = _mm_set1_epi32(0x12345678);
    int elem;
    asm volatile (
        "movd %1, %0\n\t"
        : "=r"(elem)
        : "x"(vec)           /* SSE register */
        : 
    );
    
    checksum += cr0 + elem;
    
#else
    /* Generic: Memory constraints with register operands */
    int value = 42;
    int result;
    
    asm volatile (
        "movl %1, %0\n\t"
        : "=r"(result)
        : "m"(value)         /* 'm' constraint on register variable */
        : 
    );
    
    checksum += result;
#endif
}

/* Test 5: Addressing mode conflicts */
void test_addressing_mode_conflict(void) {
    struct {
        int a;
        int b[10];
        int c;
    } s = {0};
    
    int index = compute_index() % 10;
    
    /* Complex addressing in asm */
    int result1, result2;
    
    asm volatile (
        "movl %1, %0\n\t"
        : "=r"(result1)
        : "m"(s.b[index])    /* Complex memory address */
        : 
    );
    
    /* Force base register constraint if available */
#if defined(__i386__) || defined(__x86_64__)
    asm volatile (
        "movl (%%ebx), %0\n\t"
        : "=r"(result2)
        : "b"(&s)            /* Force ebx as base register */
        : 
    );
#else
    asm volatile (
        "movl (%1), %0\n\t"
        : "=r"(result2)
        : "r"(&s)
        : 
    );
#endif
    
    checksum += result1 + result2;
}

/* Test 6: Mode size mismatches */
void test_mode_mismatch(void) {
    char c = 'A';
    short s = 1234;
    int i = 56789;
    long long ll = 1234567890123LL;
    
    /* Mixing sizes in asm */
    long long result;
    
    asm volatile (
        "movsbl %1, %%eax\n\t"
        "movswl %2, %%edx\n\t"
        "addl %%edx, %%eax\n\t"
        "cltq\n\t"
        "addq %3, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r"(result)
        : "r"(c), "r"(s), "r"(ll)
        : "%rax", "%rdx"
    );
    
    /* Floating point to integer conversion */
    double d = 3.14159;
    int int_result;
    
    asm volatile (
        "cvttsd2si %1, %0\n\t"
        : "=r"(int_result)
        : "x"(d)
        : 
    );
    
    checksum += (int)result + int_result;
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
        test_secondary_reload_trigger();
        test_addressing_mode_conflict();
        test_mode_mismatch();
        
        /* Prevent loop optimization */
        if (iteration % 100 == 0) {
            printf("Iteration %d, checksum = %d\n", iteration, checksum);
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
