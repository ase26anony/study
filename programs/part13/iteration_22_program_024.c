/* Test program to trigger reload.cc push_reload logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile to prevent optimization */
volatile int global_checksum = 0;

/* Function to create complex expressions */
int compute_index(void) {
    static int counter = 0;
    return (counter++ * 37) & 0xFF;
}

/* Test 1: Inline assembly with conflicting constraints */
void test_asm_constraint_conflict(void) {
    int input1 = 12345;
    int input2 = 67890;
    int output1, output2, output3;
    
    /* Force reload by requiring specific register for output */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)      /* Must be in eax */
        : "mr"(input1 + 42)  /* Memory or register */
        : 
    );
    
    /* Early clobber to force reload */
    asm volatile (
        "addl %2, %0\n\t"
        "movl %0, %1\n\t"
        : "=&r"(output2), "=r"(output3)  /* Early clobber */
        : "r"(input2), "0"(output1)
        : 
    );
    
    /* Mix different sized operands */
    char char_var = 65;
    long long ll_var = 0x123456789ABCDEF0LL;
    long long ll_result;
    
    asm volatile (
        "movsbl %1, %%eax\n\t"
        "addq %%rax, %0\n\t"
        : "+r"(ll_result)
        : "r"(char_var)
        : "%rax"
    );
    
    global_checksum += output1 + output2 + output3 + (int)ll_result;
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    static int array[256] = {0};
    
    /* Initialize array */
    for (int i = 0; i < 256; i++) {
        array[i] = i * i;
    }
    
    /* Complex expression as builtin operand */
    int cnt = __builtin_popcount(array[compute_index()] + global_checksum);
    
    /* More complex builtin usage */
    double x = 2.0 + (double)global_checksum / 1000.0;
    double root = __builtin_sqrt(x * x + 1.0);
    
    /* Atomic operation with complex address */
    int index = compute_index();
    int old = __atomic_fetch_add(&array[index & 0xFF], cnt, __ATOMIC_SEQ_CST);
    
    global_checksum += cnt + (int)root + old;
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(void) {
    /* Declare register variables */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    
    r1 = 1000 + global_checksum;
    r2 = 2000 + global_checksum;
    
    /* Force conflict by requiring different register in asm */
    int result;
    asm volatile (
        "movl %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(result)
        : 
        : "%eax", "%ebx", "%ecx"
    );
    
    /* Try to take address (will generate warning but compile) */
    int *ptr;
    asm volatile (
        "leal %1, %0\n\t"
        : "=r"(ptr)
        : "r"(r1)
        : 
    );
    
    global_checksum += result + (int)(intptr_t)ptr;
}

/* Test 4: Secondary reload triggers */
void test_secondary_reload_trigger(void) {
    /* Different architectures need different approaches */
    
#if defined(__arm__) || defined(__aarch64__)
    /* ARM: System register access often needs secondary reload */
    unsigned int control_reg;
    asm volatile (
        "mrs %0, cpsr\n\t"
        : "=r"(control_reg)
        : 
        : 
    );
    
    /* NEON to ARM register move might need secondary reload */
    float f1 = 3.14159f;
    float f2;
    asm volatile (
        "vmov.f32 %0, %1\n\t"
        : "=r"(f2)
        : "w"(f1)  /* NEON register constraint */
        : 
    );
    
    global_checksum += control_reg + (int)f2;
    
#elif defined(__x86_64__) || defined(__i386__)
    /* x86: Control register access */
    unsigned int cr0;
    asm volatile (
        "mov %%cr0, %0\n\t"
        : "=r"(cr0)
        : 
        : 
    );
    
    /* x87 floating point with memory constraint */
    double d1 = 3.141592653589793;
    double d2;
    asm volatile (
        "fldl %1\n\t"
        "fstpl %0\n\t"
        : "=m"(d2)
        : "m"(d1)
        : "st", "st(1)"
    );
    
    /* MMX/SSE register constraints */
    int mmx_data[2] = {0x12345678, 0x9ABCDEF0};
    int mmx_result[2];
    
    asm volatile (
        "movq %1, %%mm0\n\t"
        "movq %%mm0, %0\n\t"
        : "=m"(mmx_result)
        : "m"(mmx_data)
        : "%mm0"
    );
    
    global_checksum += cr0 + (int)d2 + mmx_result[0];
    
#else
    /* Generic fallback: memory constraints with register pressure */
    int a = 100, b = 200, c = 300, d = 400;
    int results[4];
    
    /* Create register pressure then use memory constraints */
    asm volatile (
        "movl %1, %0\n\t"
        "addl %2, %0\n\t"
        "addl %3, %0\n\t"
        "addl %4, %0\n\t"
        : "=r"(results[0]), "=m"(results[1]), "=m"(results[2]), "=m"(results[3])
        : "r"(a), "r"(b), "r"(c), "r"(d), "0"(global_checksum)
        : 
    );
    
    global_checksum += results[0] + results[1] + results[2] + results[3];
#endif
}

/* Test 5: Mixed mode and addressing conflicts */
void test_mixed_mode_addressing(void) {
    struct {
        int a;
        char b[16];
        long long c;
    } data = {0};
    
    data.a = 1234;
    data.c = 0x1122334455667788LL;
    
    /* Different sized accesses to same location */
    char *char_ptr = data.b;
    int *int_ptr = (int *)data.b;
    
    /* Force different addressing modes */
    asm volatile (
        "movb $65, (%0)\n\t"
        "movl $123456, (%1)\n\t"
        : 
        : "r"(char_ptr + 3), "r"(int_ptr)
        : "memory"
    );
    
    /* Base+index addressing with specific constraint */
    int index = compute_index() & 7;
    int value;
    
    asm volatile (
        "movl (%1,%2,4), %0\n\t"
        : "=r"(value)
        : "r"(int_ptr), "r"(index)
        : "memory"
    );
    
    global_checksum += data.a + (int)data.c + value;
}

int main(void) {
    printf("Starting reload coverage test...\n");
    
    /* Run tests multiple times to increase coverage probability */
    for (int iteration = 0; iteration < 1000; iteration++) {
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_mixed_mode_addressing();
        
        /* Add some branching to affect register allocation */
        if (iteration % 100 == 0) {
            global_checksum += iteration * 7;
        }
    }
    
    printf("Final checksum: %d\n", global_checksum);
    printf("Test completed.\n");
    
    return 0;
}
