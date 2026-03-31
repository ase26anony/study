/* Test program to trigger specific reload.cc lines 1381-1399 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_checksum = 0;
volatile int g_counter = 0;

/* Global variables for memory operands */
int global_array[100] = {0};
float global_floats[50] = {0.0f};
double global_doubles[50] = {0.0};

/* Function to create complex expressions */
int compute_index(void) {
    static int counter = 0;
    return (counter++ * 7) % 50;
}

/* Test 1: Inline assembly with conflicting constraints */
void test_asm_constraint_conflict(void) {
    int input1 = 42;
    int input2 = 73;
    int output1, output2;
    
    /* Force reload by requiring specific register for output */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)        /* Must be in eax */
        : "mr"(input1 + g_counter)  /* Memory or register */
        : 
    );
    
    /* Early-clobber constraint forcing reload */
    asm volatile (
        "addl %2, %0\n\t"
        "movl %0, %1\n\t"
        : "=&r"(output1), "=r"(output2)  /* Early clobber */
        : "r"(input2), "0"(input1)
        : 
    );
    
    /* Mixing different sized operands */
    char char_val = 127;
    long long ll_val = 0x123456789ABCDEFLL;
    long long ll_result;
    
    asm volatile (
        "movsbl %1, %%eax\n\t"
        "addq %%rax, %0\n\t"
        : "+r"(ll_result)
        : "r"(char_val)
        : "%rax"
    );
    
    g_checksum += output1 + output2 + (int)ll_result;
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    /* Complex expression as built-in argument */
    int popcnt = __builtin_popcount(global_array[compute_index()] + g_counter);
    
    /* Function call in built-in argument */
    int ctz = __builtin_ctz(compute_index() | 1);  /* Avoid zero argument */
    
    /* Atomic operation with complex address */
    int index = compute_index();
    __atomic_fetch_add(&global_array[index * 2], 1, __ATOMIC_RELAXED);
    
    /* Math built-in with memory operand */
    double d = __builtin_sqrt(global_doubles[compute_index()] + 1.0);
    
    g_checksum += popcnt + ctz + (int)d;
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(void) {
    /* Register variables with specific register constraints */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    
    r1 = 100 + g_counter;
    r2 = 200 + g_counter;
    
    /* Try to force conflict by using in asm with different constraint */
    int result;
    asm volatile (
        "movl %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        : "=a"(result)        /* Requires eax, not ebx/ecx */
        : 
        : "%ebx", "%ecx"
    );
    
    /* Attempt to take address (will generate warning but may trigger reloads) */
    int *ptr;
    asm volatile (
        "leal %1, %0\n\t"
        : "=r"(ptr)
        : "m"(r1)            /* Memory constraint on register variable */
        : 
    );
    
    g_checksum += result + (int)(intptr_t)ptr;
}

/* Test 4: Secondary reload triggers */
void test_secondary_reload_trigger(void) {
#ifdef __x86_64__
    /* x86-64 specific: Control register access (requires secondary reload) */
    uint64_t cr0;
    asm volatile (
        "mov %%cr0, %0\n\t"
        : "=r"(cr0)
        : 
    );
    
    /* x87 floating point stack manipulation */
    double x = 3.14159 + g_counter;
    double y;
    asm volatile (
        "fldl %1\n\t"
        "fstpl %0\n\t"
        : "=m"(y)
        : "m"(x)
        : "st"
    );
    
    g_checksum += (int)cr0 + (int)y;
    
#elif defined(__arm__) || defined(__aarch64__)
    /* ARM specific: System register access */
    uint32_t cpsr;
    asm volatile (
        "mrs %0, cpsr\n\t"
        : "=r"(cpsr)
        : 
    );
    
    /* NEON register constraints */
    float32x4_t v1, v2, v3;
    v1 = vld1q_f32(global_floats);
    v2 = vld1q_f32(global_floats + 4);
    
    asm volatile (
        "vadd.f32 %0, %1, %2\n\t"
        : "=w"(v3)        /* NEON register constraint */
        : "w"(v1), "w"(v2)
        : 
    );
    
    g_checksum += cpsr + (int)vgetq_lane_f32(v3, 0);
    
#else
    /* Generic memory constraint forcing spill/reload */
    int a = 1000 + g_counter;
    int b = 2000 + g_counter;
    int c;
    
    asm volatile (
        "imull %1, %0\n\t"
        : "+r"(a)          /* Input and output in register */
        : "m"(b)           /* But b must be in memory */
        : 
    );
    
    g_checksum += a;
#endif
}

/* Test 5: Addressing mode conflicts */
void test_addressing_mode_conflicts(void) {
    int array[10] = {0};
    int index = g_counter % 10;
    
    /* Force base+index addressing conflict */
    asm volatile (
        "addl $1, %0\n\t"
        : "+m"(array[index])  /* Complex addressing */
        : 
        : 
    );
    
    /* Register constraint that conflicts with addressing mode */
    int result;
    asm volatile (
        "leal (%1,%2,4), %0\n\t"
        : "=r"(result)
        : "r"(array), "r"(index)  /* Both in registers */
        : 
    );
    
    /* Try to force a specific segment register (x86) */
#ifdef __i386__
    int far_ptr;
    asm volatile (
        "mov %%gs:0, %0\n\t"
        : "=r"(far_ptr)
        : 
        : 
    );
    g_checksum += far_ptr;
#endif
    
    g_checksum += result + array[index];
}

/* Test 6: Mixed data types and modes */
void test_mixed_types_modes(void) {
    /* Mix integer and floating point in same asm */
    int i = g_counter;
    float f = (float)g_counter;
    double d;
    
    asm volatile (
        "cvtsi2ssl %1, %%xmm0\n\t"
        "cvtss2sd %%xmm0, %%xmm1\n\t"
        "movsd %%xmm1, %0\n\t"
        : "=m"(d)
        : "r"(i)
        : "%xmm0", "%xmm1"
    );
    
    /* Different sized memory accesses */
    short s = 32767;
    long long ll = 0;
    
    asm volatile (
        "movswl %1, %%eax\n\t"
        "cltq\n\t"
        "movq %%rax, %0\n\t"
        : "=m"(ll)
        : "m"(s)
        : "%rax"
    );
    
    g_checksum += (int)d + (int)ll;
}

int main(void) {
    /* Initialize globals */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 3;
        if (i < 50) {
            global_floats[i] = i * 1.5f;
            global_doubles[i] = i * 2.5;
        }
    }
    
    /* Run tests multiple times to increase reload opportunities */
    for (int iteration = 0; iteration < 1000; iteration++) {
        g_counter = iteration;
        
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_addressing_mode_conflicts();
        test_mixed_types_modes();
        
        /* Prevent loop unrolling */
        if (iteration % 100 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    printf("Final checksum: %d\n", g_checksum);
    return 0;
}
