/* test_reload_coverage.c
 * Comprehensive test to trigger push_reload logic in GCC's reload pass
 * Compile with: gcc -O1 -fno-omit-frame-pointer test_reload_coverage.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* Global volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_checksum = 0;
volatile float global_float = 3.14159f;
volatile double global_double = 2.71828;

/* Complex global array to force memory addressing */
int global_array[1024];

/* Function to compute index with side effects */
int compute_index(void) {
    static int counter = 0;
    return (counter++ * 37) & 1023;
}

/* Test 1: Inline assembly with conflicting constraints */
void test_asm_constraint_conflict(void) {
    int input1 = 42;
    int input2 = 100;
    int output1, output2, output3;
    
    /* Force reload by requiring specific register for output */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)      /* Must be in eax */
        : "mr"(input1 + global_counter)  /* Memory or register */
        : 
    );
    
    /* Early-clobber constraint forces reload */
    asm volatile (
        "addl %2, %0\n\t"
        "movl %0, %1\n\t"
        : "=&r"(output2), "=r"(output3)  /* Early clobber on first output */
        : "r"(input2), "0"(output1)
        : 
    );
    
    /* Mixed size constraints */
    short s_input = 32767;
    long long ll_output;
    asm volatile (
        "movswl %1, %%eax\n\t"
        "cltq\n\t"
        "movq %%rax, %0\n\t"
        : "=r"(ll_output)
        : "r"(s_input)
        : "%rax"
    );
    
    global_checksum += output1 + output2 + output3 + (int)ll_output;
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    /* Force reload for builtin argument */
    int popcnt = __builtin_popcount(global_array[compute_index()] + global_counter);
    
    /* Complex expression requiring temporary */
    int ctz = __builtin_ctz((unsigned int)(global_float * 1000) | 1);
    
    /* Atomic operation with complex address */
    int atomic_val = 0;
    __atomic_fetch_add(&global_array[compute_index()], popcnt, __ATOMIC_RELAXED);
    
    /* Math builtin with function call argument */
    double sqrt_val = __builtin_sqrt(fabs(global_double * compute_index()));
    
    global_checksum += popcnt + ctz + atomic_val + (int)sqrt_val;
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(void) {
    /* Declare register variables */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    
    r1 = global_counter + 1;
    r2 = global_counter + 2;
    
    /* Force conflict: use register variable in asm requiring different register */
    int result;
    asm volatile (
        "movl %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(result)
        : 
        : "%eax", "%ebx", "%ecx"
    );
    
    /* Try to take address (will generate warning but may force reload) */
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
    /* Different architectures require different approaches */
    
#if defined(__x86_64__) || defined(__i386__)
    /* x86: Use x87 FPU constraints that might need secondary reloads */
    double x = global_double;
    double y;
    
    asm volatile (
        "fldl %1\n\t"
        "fstpl %0\n\t"
        : "=m"(y)
        : "m"(x)
        : "st", "st(1)"
    );
    
    /* MMX/SSE constraints */
    int mmx_var __attribute__((vector_size(8)));
    asm volatile (
        "movq %1, %0\n\t"
        : "=x"(mmx_var)
        : "x"(mmx_var)
        : 
    );
    
    global_checksum += (int)y + mmx_var[0];
    
#elif defined(__arm__)
    /* ARM: System register access often needs secondary reload */
    unsigned int control_reg;
    
    asm volatile (
        "mrc p15, 0, %0, c1, c0, 0\n\t"
        : "=r"(control_reg)
        : 
        : 
    );
    
    /* NEON to VFP transfer might need secondary reload */
    float32x2_t neon_vec = {1.0f, 2.0f};
    float vfp_reg;
    
    asm volatile (
        "vmov.f32 %0, %1[0]\n\t"
        : "=t"(vfp_reg)
        : "w"(neon_vec)
        : 
    );
    
    global_checksum += control_reg + (int)vfp_reg;
    
#elif defined(__aarch64__)
    /* AArch64: Special register constraints */
    unsigned long long tpidr;
    
    asm volatile (
        "mrs %0, tpidr_el0\n\t"
        : "=r"(tpidr)
        : 
        : 
    );
    
    global_checksum += (int)tpidr;
#endif
}

/* Test 5: Complex addressing modes and mixed types */
void test_addressing_modes(void) {
    struct {
        int a;
        int b[10];
        double c;
    } complex_struct;
    
    /* Force complex address computation */
    int index = compute_index() % 10;
    
    /* Constraint requiring base register only */
    int result;
    asm volatile (
        "movl (%1, %2, 4), %0\n\t"
        : "=r"(result)
        : "r"(&complex_struct.b), "r"(index)
        : 
    );
    
    /* Mixed integer sizes in same asm */
    char c = 'A';
    short s = 12345;
    int i = 67890;
    long long ll = 0;
    
    asm volatile (
        "movsbl %1, %%eax\n\t"
        "movswl %2, %%ebx\n\t"
        "addl %3, %%ebx\n\t"
        "addl %%eax, %%ebx\n\t"
        "movslq %%ebx, %0\n\t"
        : "=r"(ll)
        : "r"(c), "r"(s), "r"(i)
        : "%rax", "%rbx"
    );
    
    global_checksum += result + (int)ll;
}

/* Test 6: High register pressure to force spills and reloads */
void test_high_register_pressure(void) {
    /* Many live variables to increase register pressure */
    int v1 = global_counter + 1;
    int v2 = global_counter + 2;
    int v3 = global_counter + 3;
    int v4 = global_counter + 4;
    int v5 = global_counter + 5;
    int v6 = global_counter + 6;
    int v7 = global_counter + 7;
    int v8 = global_counter + 8;
    int v9 = global_counter + 9;
    int v10 = global_counter + 10;
    
    /* Chain of operations forcing temporary reloads */
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
    
    /* Another chain */
    asm volatile (
        "imull %1, %0\n\t"
        "imull %2, %0\n\t"
        : "+r"(v7)
        : "r"(v8), "r"(v9)
        : 
    );
    
    global_checksum += v1 + v7 + v10;
}

/* Main test driver */
int main(void) {
    /* Initialize global array */
    for (int i = 0; i < 1024; i++) {
        global_array[i] = i * 3;
    }
    
    printf("Starting reload coverage test...\n");
    
    /* Run tests multiple times to increase coverage probability */
    for (int iteration = 0; iteration < 1000; iteration++) {
        global_counter = iteration;
        
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_addressing_modes();
        test_high_register_pressure();
        
        /* Alternate between different global values */
        global_float = sinf(iteration * 0.01f);
        global_double = cos(iteration * 0.01);
    }
    
    printf("Final checksum: %d\n", global_checksum);
    printf("Test completed.\n");
    
    return 0;
}
