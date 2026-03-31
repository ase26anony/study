/* test_reload.c - Comprehensive test to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* Volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile int checksum = 0;

/* Complex function to force register pressure */
int compute_index(int base) {
    return (base * 1103515245 + 12345) & 0x7fffffff;
}

/* Global arrays for memory operand testing */
int global_array[1024];
float float_array[1024];
double double_array[1024];

/* ============================================
   Test 1: Inline Assembly with Constraint Conflicts
   ============================================ */
void test_asm_constraint_conflict(void) {
    int input1 = 42;
    int input2 = 73;
    int output1, output2, output3;
    
    /* Force reload by requiring specific hard registers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (output1)      /* Output in any register */
        : "mr" (input1 + global_counter),  /* Memory or register */
          "mr" (input2)
        : "%eax"              /* EAX is clobbered */
    );
    
    checksum += output1;
    
    /* Early-clobber constraint forces reload */
    int temp = 100;
    asm volatile (
        "movl %1, %0\n\t"
        "addl $5, %0"
        : "=&r" (output2)     /* Early-clobber - can't share with inputs */
        : "r" (temp),
          "m" (global_array[10])
        : 
    );
    
    checksum += output2;
    
    /* Mixed register classes causing conflicts */
    register int r1 asm("ebx") = 50;
    register int r2 asm("ecx") = 60;
    
    asm volatile (
        "xchgl %%ebx, %%ecx\n\t"
        "addl %%ecx, %%ebx"
        : "+&r" (r1), "+&r" (r2)
        : 
        : 
    );
    
    checksum += r1 + r2;
    
    /* Floating point to integer conversion requiring reload */
    float fval = 3.14159f;
    int ival;
    
    asm volatile (
        "cvttss2si %1, %0"
        : "=r" (ival)
        : "x" (fval)          /* SSE register constraint */
        : 
    );
    
    checksum += ival;
}

/* ============================================
   Test 2: Built-in Functions with Complex Operands
   ============================================ */
void test_builtin_complex_operand(void) {
    /* Complex expression as built-in argument */
    int popcnt = __builtin_popcount(
        global_array[compute_index(global_counter) % 1024] + 
        (global_counter * 7)
    );
    checksum += popcnt;
    
    /* Built-in with function call argument */
    int ctz = __builtin_ctz(
        compute_index(global_counter) | 1  /* Ensure non-zero */
    );
    checksum += ctz;
    
    /* Atomic built-in with complex address */
    int idx = global_counter % 1024;
    __atomic_fetch_add(&global_array[idx], 1, __ATOMIC_RELAXED);
    
    /* Math built-in with memory operand */
    double dval = __builtin_sqrt(double_array[idx] + 1.0);
    checksum += (int)dval;
    
    /* Built-in requiring specific register */
    unsigned long long large = 0x123456789ABCDEF0ULL;
    int parity = __builtin_parityll(large >> (global_counter % 64));
    checksum += parity;
}

/* ============================================
   Test 3: Register Variable Abuse
   ============================================ */
void test_register_variable_abuse(void) {
    /* Register variables with conflicting usage */
    register int reg1 asm("esi") = 100 + global_counter;
    register int reg2 asm("edi") = 200 + global_counter;
    
    /* Force them into contexts requiring different registers */
    int result;
    asm volatile (
        "leal (%1, %2), %0"
        : "=r" (result)
        : "r" (reg1), "r" (reg2)
        : 
    );
    checksum += result;
    
    /* Use register variable in memory context */
    int *ptr = &global_array[0];
    ptr[0] = reg1;  /* Store register variable to memory */
    
    /* Inline asm that conflicts with register binding */
    asm volatile (
        "movl %1, %%eax\n\t"
        "imull %%eax, %0"
        : "+r" (reg2)
        : "r" (reg1)
        : "%eax", "%edx"
    );
    checksum += reg2;
}

/* ============================================
   Test 4: Architecture-Specific Secondary Reloads
   ============================================ */
void test_secondary_reload_trigger(void) {
#if defined(__arm__) || defined(__aarch64__)
    /* ARM-specific: System register access often needs secondary reload */
    unsigned int control_reg;
    asm volatile (
        "mrs %0, cpsr"
        : "=r" (control_reg)
        : 
        : 
    );
    checksum += control_reg & 0xFF;
    
#elif defined(__x86_64__) || defined(__i386__)
    /* x86: Control register access */
    unsigned int cr0;
    asm volatile (
        "mov %%cr0, %0"
        : "=r" (cr0)
        : 
        : 
    );
    checksum += cr0 & 0xFF;
    
    /* x87 floating point stack manipulation */
    double x = 3.14159;
    double y;
    asm volatile (
        "fldl %1\n\t"
        "fstpl %0"
        : "=m" (y)
        : "m" (x)
        : "st", "st(1)"
    );
    checksum += (int)y;
    
    /* MMX/SSE register constraints */
    int mmx_var[2] = {1, 2};
    asm volatile (
        "movq %1, %%mm0\n\t"
        "movq %%mm0, %0"
        : "=m" (mmx_var)
        : "m" (mmx_var)
        : "%mm0"
    );
    checksum += mmx_var[0];
#endif
    
    /* Memory constraint forcing spill/reload */
    int large_array[100];
    for (int i = 0; i < 100; i++) {
        large_array[i] = i + global_counter;
    }
    
    int sum = 0;
    asm volatile (
        "movl $0, %%eax\n\t"
        "movl $0, %%ecx\n"
        "1:\n\t"
        "addl (%1, %%ecx, 4), %%eax\n\t"
        "incl %%ecx\n\t"
        "cmpl $100, %%ecx\n\t"
        "jl 1b\n\t"
        "movl %%eax, %0"
        : "=r" (sum)
        : "r" (large_array)
        : "%eax", "%ecx", "memory"
    );
    checksum += sum;
}

/* ============================================
   Test 5: Mixed Mode and Size Operands
   ============================================ */
void test_mixed_mode_operands(void) {
    /* Different sized operands in same asm */
    char c = 'A' + (global_counter % 26);
    short s = 1000 + global_counter;
    int i = 1000000 + global_counter;
    long long ll = 1000000000LL + global_counter;
    
    long long result;
    asm volatile (
        "movsbl %1, %%eax\n\t"
        "movswl %2, %%ecx\n\t"
        "addl %%ecx, %%eax\n\t"
        "cltq\n\t"
        "addq %3, %%rax\n\t"
        "addq %4, %%rax\n\t"
        "movq %%rax, %0"
        : "=r" (result)
        : "r" (c), "r" (s), "r" (i), "r" (ll)
        : "%rax", "%rcx", "%eax", "%ecx"
    );
    checksum += (int)result;
    
    /* Mixed floating point precision */
    float f = 1.234f + global_counter;
    double d = 9.876 + global_counter;
    long double ld = 3.14159265358979L + global_counter;
    
    double dresult;
    asm volatile (
        "cvtss2sd %1, %%xmm0\n\t"
        "addsd %2, %%xmm0\n\t"
        "cvtsd2ss %%xmm0, %%xmm1\n\t"
        "cvtss2sd %%xmm1, %0"
        : "=x" (dresult)
        : "x" (f), "x" (d)
        : "%xmm0", "%xmm1"
    );
    checksum += (int)dresult;
}

/* ============================================
   Test 6: High Register Pressure Scenario
   ============================================ */
void test_high_register_pressure(void) {
    /* Many live variables to force spills and reloads */
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
    int result = (((v1 * v2) + (v3 * v4)) - 
                  ((v5 * v6) + (v7 * v8))) * 
                  (v9 - v10);
    
    /* Force all variables to be used in asm to keep them live */
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
        "addl %10, %0"
        : "+r" (result)
        : "r" (v1), "r" (v2), "r" (v3), "r" (v4),
          "r" (v5), "r" (v6), "r" (v7), "r" (v8),
          "r" (v9), "r" (v10)
        : 
    );
    
    checksum += result;
}

/* ============================================
   Main Test Driver
   ============================================ */
int main(void) {
    /* Initialize global arrays */
    for (int i = 0; i < 1024; i++) {
        global_array[i] = i;
        float_array[i] = i * 0.1f;
        double_array[i] = i * 0.01;
    }
    
    printf("Starting reload pass coverage test...\n");
    
    /* Run tests multiple times to increase coverage probability */
    for (int iteration = 0; iteration < 1000; iteration++) {
        global_counter = iteration;
        
        test_asm_constraint_conflict();
        test_builtin_complex_operand();
        test_register_variable_abuse();
        test_secondary_reload_trigger();
        test_mixed_mode_operands();
        test_high_register_pressure();
        
        /* Prevent loop unrolling from eliminating reloads */
        if (iteration % 100 == 0) {
            printf("Iteration %d, checksum = %d\n", iteration, checksum);
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return checksum != 0 ? 0 : 1;
}
