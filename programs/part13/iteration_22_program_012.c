/* Test program to trigger reload.cc push_reload logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile int checksum = 0;

/* Complex function to force register pressure */
int compute_index(int base) {
    return (base * 1103515245 + 12345) & 0x7fffffff;
}

/* Global arrays to force memory operands */
int global_array[1000];
float global_float_array[1000];
double global_double_array[1000];

/* Test 1: Inline assembly with conflicting constraints */
void test_asm_constraint_conflict(void) {
    int input1 = 42;
    int input2 = 73;
    int output1, output2, output3;
    
    /* Force reload by requiring specific registers */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(output1)      /* Must be in eax */
        : "mr"(input1 + global_counter)  /* Memory or register */
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
    
    /* Mix different register classes */
    asm volatile (
        "movl %%ebx, %%eax\n\t"
        "addl %1, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(output1)
        : "r"(input2)
        : "%eax", "%ebx"
    );
    
    checksum += output1 + output2 + output3;
}

/* Test 2: Built-in functions with complex operands */
void test_builtin_complex_operand(void) {
    int i;
    
    for (i = 0; i < 100; i++) {
        /* Complex array indexing */
        int idx = compute_index(i);
        
        /* Builtin with complex memory operand */
        int cnt = __builtin_popcount(global_array[idx % 1000] + checksum);
        
        /* Another builtin with function call in operand */
        int leading_zeros = __builtin_clz(global_array[compute_index(cnt) % 1000]);
        
        /* Atomic operation with complex address */
        int* ptr = &global_array[(idx + leading_zeros) % 1000];
        __atomic_fetch_add(ptr, cnt, __ATOMIC_RELAXED);
        
        checksum += cnt + leading_zeros;
    }
}

/* Test 3: Register variable abuse */
void test_register_variable_abuse(void) {
    /* Declare register variables */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    register int r3 asm("edx");
    
    r1 = 100 + global_counter;
    r2 = 200 + global_counter;
    r3 = 300 + global_counter;
    
    int result1, result2;
    
    /* Force conflict: use register variable in asm requiring different register */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(result1)
        : "r"(r1), "r"(r2)  /* r1 is in ebx, but asm might need to move it */
        : "%eax"
    );
    
    /* Another conflict */
    asm volatile (
        "xchgl %0, %1\n\t"
        : "+r"(r2), "+r"(r3)
        :
        :
    );
    
    /* Try to take address (will force to memory) */
    int* ptr;
    asm volatile (
        "leal %1, %0\n\t"
        : "=r"(ptr)
        : "m"(r3)  /* Force r3 to memory location */
        :
    );
    
    checksum += result1 + r2 + r3 + (int)(intptr_t)ptr;
}

/* Test 4: Mixed data types and modes */
void test_mixed_types_modes(void) {
    char c1 = 'A' + (global_counter % 26);
    short s1 = 1000 + global_counter;
    int i1 = 1000000 + global_counter;
    long long ll1 = 1000000000LL + global_counter;
    float f1 = 3.14159f + global_counter;
    double d1 = 2.71828 + global_counter;
    
    /* Mix sizes in asm */
    int result_int;
    asm volatile (
        "movsbl %1, %%eax\n\t"
        "addw %2, %%ax\n\t"
        "addl %3, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(result_int)
        : "r"(c1), "r"(s1), "r"(i1)
        : "%eax"
    );
    
    /* Floating point with integer conversion */
    double result_double;
    asm volatile (
        "cvtsi2sd %1, %%xmm0\n\t"
        "addsd %2, %%xmm0\n\t"
        "movsd %%xmm0, %0\n\t"
        : "=x"(result_double)
        : "r"(i1), "x"(d1)
        : "%xmm0"
    );
    
    checksum += result_int + (int)result_double;
}

/* Test 5: Forcing secondary reloads */
void test_secondary_reload_trigger(void) {
    /* This test is architecture-specific */
    #if defined(__arm__)
    /* ARM: Access system registers often needs secondary reload */
    unsigned int control_reg;
    asm volatile (
        "mrc p15, 0, %0, c1, c0, 0\n\t"
        : "=r"(control_reg)
        :
        :
    );
    checksum += control_reg;
    
    #elif defined(__x86_64__)
    /* x86: Control registers need secondary reload */
    unsigned long long cr0;
    asm volatile (
        "mov %%cr0, %0\n\t"
        : "=r"(cr0)
        :
        :
    );
    
    /* x87 floating point stack manipulation */
    double x87_result;
    asm volatile (
        "fldl %1\n\t"
        "fsqrt\n\t"
        "fstpl %0\n\t"
        : "=m"(x87_result)
        : "m"(global_double_array[global_counter % 1000])
        :
    );
    checksum += (int)cr0 + (int)x87_result;
    
    #elif defined(__aarch64__)
    /* AArch64: System registers */
    unsigned long long nzcv;
    asm volatile (
        "mrs %0, nzcv\n\t"
        : "=r"(nzcv)
        :
        :
    );
    checksum += (int)nzcv;
    
    #else
    /* Generic: Use memory constraints with register-only instructions */
    int val1 = 12345;
    int val2 = 67890;
    int result;
    
    asm volatile (
        "movl (%1), %%eax\n\t"
        "addl (%2), %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(result)
        : "r"(&val1), "r"(&val2)
        : "%eax", "memory"
    );
    checksum += result;
    #endif
}

/* Test 6: High register pressure */
void test_high_register_pressure(void) {
    /* Many live variables to force spilling and reloading */
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
    
    /* Complex expression using all variables */
    int result = 
        (v1 * v2) + (v3 / (v4 + 1)) - (v5 % (v6 + 1)) +
        (v7 << 2) | (v8 >> 1) ^ (v9 & 0xFF) | (~v10);
    
    /* Use in inline asm to force specific registers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "imull %3, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(result)
        : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5)
        : "%eax"
    );
    
    checksum += result;
}

/* Test 7: Volatile memory accesses */
void test_volatile_memory_access(void) {
    volatile int* volatile_ptr = (volatile int*)global_array;
    
    /* Multiple volatile accesses force reloads */
    for (int i = 0; i < 50; i++) {
        int idx = (i * 7) % 1000;
        
        /* Read-modify-write pattern */
        int old_val = volatile_ptr[idx];
        int new_val = old_val + checksum + i;
        volatile_ptr[idx] = new_val;
        
        /* Use in asm with memory constraint */
        int temp;
        asm volatile (
            "movl (%1), %%eax\n\t"
            "addl %2, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r"(temp)
            : "r"(&volatile_ptr[idx]), "r"(i)
            : "%eax", "memory"
        );
        
        checksum += temp;
    }
}

int main(void) {
    /* Initialize global arrays */
    for (int i = 0; i < 1000; i++) {
        global_array[i] = i * 3;
        global_float_array[i] = i * 1.5f;
        global_double_array[i] = i * 2.5;
    }
    
    /* Run tests multiple times to increase coverage probability */
    for (int iteration = 0; iteration < 1000; iteration++) {
        global_counter = iteration;
        
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_mixed_types_modes();
        test_secondary_reload_trigger();
        test_high_register_pressure();
        test_volatile_memory_access();
        
        /* Prevent loop unrolling */
        if (iteration % 100 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
