/* Test program to trigger reload.cc's push_reload function */
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
        : "mr"(input1 + 7)     /* Memory or register */
        : 
    );
    
    /* Early-clobber constraint forcing reload */
    asm volatile (
        "addl %1, %0\n\t"
        "subl %2, %0\n\t"
        : "=&r"(output2)       /* Early clobber */
        : "r"(input1), "r"(input2)
        : 
    );
    
    /* Multiple constraints with register pressure */
    int temp = input1 * input2;
    asm volatile (
        "imull %1, %0\n\t"
        : "+r"(temp)           /* Read-write operand */
        : "rm"(input2)         /* Register or memory */
        : "cc"
    );
    
    global_result ^= output1 + output2 + temp;
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    static int array[256] = {0};
    for (int i = 0; i < 256; i++) {
        array[i] = i * i;
    }
    
    /* Complex address computation requiring reload */
    int idx = compute_index(global_counter);
    int cnt = __builtin_popcount(array[idx & 0xFF]);
    
    /* Math built-in with complex argument */
    double x = (double)global_counter + 3.14159;
    double y = __builtin_sqrt(x * x + 1.0);
    
    /* Atomic operation with complex address */
    int atomic_var = 0;
    __atomic_fetch_add(&array[idx & 0x7F], cnt, __ATOMIC_RELAXED);
    
    global_result += cnt + (int)y;
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(void) {
    /* Declare register variables */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    
    r1 = global_counter * 2;
    r2 = global_counter * 3;
    
    /* Force conflict by using in asm with different constraints */
    int result;
    asm volatile (
        "addl %%ebx, %%ecx\n\t"
        "movl %%ecx, %0\n\t"
        : "=r"(result)
        : 
        : "ebx", "ecx"
    );
    
    /* Try to take address (will generate warning but test reload) */
    int *ptr;
    asm volatile (
        "leal %1, %0\n\t"
        : "=r"(ptr)
        : "m"(r1)
        : 
    );
    
    global_result ^= result + (int)ptr;
}

/* Test 4: Secondary reload triggers */
void test_secondary_reload_trigger(void) {
    /* Different architectures need different approaches */
    
#if defined(__arm__) || defined(__aarch64__)
    /* ARM: System register access often needs secondary reload */
    uint32_t control_reg;
    asm volatile (
        "mrs %0, cpsr\n\t"
        : "=r"(control_reg)
        : 
        : 
    );
    
    /* NEON to ARM register transfer may need secondary reload */
    float32x4_t neon_vec = {1.0f, 2.0f, 3.0f, 4.0f};
    float result;
    asm volatile (
        "fadds %0, %1, %1\n\t"
        : "=w"(result)        /* NEON/FP register */
        : "w"(neon_vec[0])    /* NEON/FP register */
        : 
    );
    
    global_result += control_reg + (int)(result * 100);
    
#elif defined(__x86_64__) || defined(__i386__)
    /* x86: Control register access */
    uint32_t cr0;
    asm volatile (
        "mov %%cr0, %0\n\t"
        : "=r"(cr0)
        : 
        : 
    );
    
    /* x87 FPU with memory constraint */
    double x = 3.14159 * global_counter;
    double y;
    asm volatile (
        "fldl %1\n\t"
        "fsqrt\n\t"
        "fstpl %0\n\t"
        : "=m"(y)            /* Memory output */
        : "m"(x)             /* Memory input */
        : "st", "st(1)"
    );
    
    /* MMX/SSE with mismatched constraints */
    int64_t mmx_var = 0x123456789ABCDEF0LL;
    asm volatile (
        "movq %1, %%mm0\n\t"
        "movq %%mm0, %0\n\t"
        : "=m"(mmx_var)
        : "m"(mmx_var)
        : "mm0"
    );
    
    global_result += cr0 + (int)y + (mmx_var & 0xFF);
#endif
}

/* Test 5: Mixed size operands and addressing modes */
void test_mixed_operands(void) {
    char c = 'A' + (global_counter % 26);
    short s = 1000 + global_counter;
    int i = 1000000 + global_counter;
    long long ll = 1000000000000LL + global_counter;
    
    /* Mixed sizes in same asm */
    long long result;
    asm volatile (
        "movsbl %1, %%eax\n\t"
        "movswl %2, %%edx\n\t"
        "addl %%edx, %%eax\n\t"
        "cltq\n\t"
        "addq %3, %%rax\n\t"
        "addq %4, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r"(result)
        : "m"(c), "m"(s), "r"(i), "r"(ll)
        : "rax", "rdx", "cc"
    );
    
    /* Complex addressing mode */
    struct {
        int a;
        int b[10];
        char c;
    } mystruct;
    
    mystruct.a = global_counter;
    for (int j = 0; j < 10; j++) {
        mystruct.b[j] = j * global_counter;
    }
    
    int array_idx = compute_index(global_counter) % 10;
    int struct_result;
    
    /* Force reload with complex address computation */
    asm volatile (
        "movl %1, %0\n\t"
        : "=r"(struct_result)
        : "m"(mystruct.b[array_idx])
        : 
    );
    
    global_result += (result & 0xFFFFFFFF) + struct_result;
}

/* Test 6: Register pressure and spilling */
void test_register_pressure(void) {
    /* Create many live values to force spilling */
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
    
    /* Use all in complex expression forcing many reloads */
    asm volatile (
        "addl %1, %0\n\t"
        "addl %2, %0\n\t"
        "addl %3, %0\n\t"
        "addl %4, %0\n\t"
        "addl %5, %0\n\t"
        "addl %6, %0\n\t"
        "addl %7, %0\n\t"
        "addl %8, %0\n\t"
        "addl %9, %0\n\t"
        "addl %10, %0\n\t"
        : "+r"(v1)
        : "r"(v2), "r"(v3), "r"(v4), "r"(v5),
          "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10)
        : "cc"
    );
    
    global_result += v1;
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
        test_register_pressure();
        
        /* Prevent optimization */
        if (global_result > 1000000) {
            global_result = 0;
        }
    }
    
    printf("Final result: %d\n", global_result);
    printf("Test completed.\n");
    
    return 0;
}
