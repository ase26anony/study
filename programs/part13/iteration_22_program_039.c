/* Test program to trigger reload.cc push_reload uncovered lines */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_checksum = 0;

/* Global arrays to create memory operands */
int global_array[100] = {0};
float global_float_array[100] = {0.0f};

/* Function to create complex expressions */
int compute_index(void) {
    static int counter = 0;
    return (counter++ * 7 + 3) % 100;
}

/* Force register pressure with many live variables */
void test_asm_constraint_conflict(void) {
    int i;
    int input1 = 12345;
    int input2 = 67890;
    int output1, output2, output3;
    
    /* Multiple asm statements with conflicting constraints */
    for (i = 0; i < 1000; i++) {
        /* Output requires specific register, input is complex expression */
        asm volatile (
            "movl %1, %0\n\t"
            : "=a"(output1)          /* Must be in eax */
            : "mr"(input1 + i * 2)   /* Memory or register */
            : /* No clobbers */
        );
        
        /* Early clobber forces separate register */
        asm volatile (
            "addl %2, %0\n\t"
            "movl %0, %1\n\t"
            : "=&r"(output2), "=r"(output3)  /* & = early clobber */
            : "r"(input2), "0"(output1)      /* input2 in any reg, output1 in same as output2 */
            : "cc"
        );
        
        /* Mix different sized operands */
        short s_input = i & 0xFFFF;
        long long ll_output;
        asm volatile (
            "movswl %1, %%eax\n\t"
            "cltq\n\t"
            "movq %%rax, %0\n\t"
            : "=r"(ll_output)
            : "r"(s_input)
            : "rax"
        );
        
        /* Update checksum to prevent elimination */
        global_checksum ^= output1 ^ output2 ^ output3 ^ (int)ll_output;
    }
}

/* Complex expressions as builtin operands */
void test_builtin_complex_operand(void) {
    int i;
    unsigned int val;
    
    for (i = 0; i < 1000; i++) {
        /* Builtin with function call in operand */
        val = __builtin_popcount(global_array[compute_index()] + i);
        
        /* Builtin with memory access and computation */
        int cnt = __builtin_ctz(val | 1);  /* Avoid undefined behavior for 0 */
        
        /* Atomic operation with complex address */
        int index = (i * 13) % 100;
        __atomic_fetch_add(&global_array[index], cnt, __ATOMIC_RELAXED);
        
        /* Math builtin with float expression */
        float fval = global_float_array[i % 100] + (float)i * 0.1f;
        int isqrt = (int)__builtin_sqrt(fval);
        
        global_checksum += val + cnt + isqrt;
    }
}

/* Register variable abuse */
void test_register_variable_abuse(void) {
    int i;
    
    /* Try to use register variables in conflicting ways */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    
    r1 = 1000;
    r2 = 2000;
    
    for (i = 0; i < 1000; i++) {
        int temp;
        
        /* Force r1 into eax through asm */
        asm volatile (
            "xchgl %%ebx, %%eax\n\t"
            "addl $1, %%eax\n\t"
            "xchgl %%eax, %%ebx\n\t"
            : "=b"(r1), "=a"(temp)
            : "0"(r1), "1"(i)
            : "cc"
        );
        
        /* Use r2 in memory context (taking address through inline asm trick) */
        asm volatile (
            "movl %1, %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            : "=m"(global_array[i % 10])
            : "r"(r2)
            : "ecx", "memory"
        );
        
        /* Mix register variables with complex constraints */
        asm volatile (
            "imull %2, %1\n\t"
            "addl %1, %0\n\t"
            : "+r"(r1), "=&r"(r2)
            : "rm"(i + 1)
            : "cc"
        );
        
        global_checksum += r1 + r2;
    }
}

/* Architecture-specific secondary reload triggers */
void test_secondary_reload_trigger(void) {
    int i;
    
    /* These patterns are designed to trigger secondary reloads */
    for (i = 0; i < 1000; i++) {
        double d1 = i * 1.5;
        double d2;
        long double ld1 = i * 2.5L;
        long double ld2;
        
        /* Memory constraints with register-only operations */
        asm volatile (
            "fldl %1\n\t"
            "fstpl %0\n\t"
            : "=m"(d2)
            : "m"(d1)
            : "memory", "st", "st(1)"
        );
        
        /* Different sized floating point operations */
        asm volatile (
            "fldt %1\n\t"
            "fstpt %0\n\t"
            : "=m"(ld2)
            : "m"(ld1)
            : "memory", "st", "st(1)"
        );
        
        /* Force spill and reload with 'm' constraint */
        int arr[4] = {i, i+1, i+2, i+3};
        int sum;
        asm volatile (
            "movl (%1), %%eax\n\t"
            "addl 4(%1), %%eax\n\t"
            "addl 8(%1), %%eax\n\t"
            "addl 12(%1), %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r"(sum)
            : "r"(arr)
            : "rax", "memory"
        );
        
        global_checksum += (int)d2 + (int)ld2 + sum;
    }
}

/* Main test driver */
int main(void) {
    int i;
    
    /* Initialize global arrays */
    for (i = 0; i < 100; i++) {
        global_array[i] = i * 3;
        global_float_array[i] = i * 0.7f;
    }
    
    printf("Starting reload tests...\n");
    
    /* Run all tests multiple times */
    for (i = 0; i < 10; i++) {
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        
        global_counter++;
    }
    
    printf("Final checksum: %d\n", global_checksum);
    printf("Tests completed. Global counter: %d\n", global_counter);
    
    return global_checksum != 0 ? 0 : 1;
}
