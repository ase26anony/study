/* reload_stress_test.c
 * Designed to trigger push_reload initialization block (lines 1381-1399)
 * Compile with: gcc -O3 -fno-omit-frame-pointer -fno-strict-aliasing -funroll-loops -fno-optimize-sibling-calls -march=x86-64 -mno-sse -mno-avx reload_stress_test.c -o reload_test
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create dependencies */
volatile int global_int = 42;
volatile double global_double = 3.14159;
volatile char global_char = 'A';
int global_array[100] = {0};
double global_darray[50] = {0.0};

/* Function prototypes for nested calls */
int func_return_int(int x);
double func_return_double(double x);
void* func_return_ptr(void* p);
int compute_index(void);

/* Register variables to force specific register allocation */
register int reg_var1 asm ("r12");
register int reg_var2 asm ("r13");
register double reg_double asm ("xmm0");  /* Will be forced to integer regs with -mno-sse */

/* Test 1: Many operands with mixed types to exhaust registers */
int test_many_operands(void) {
    int out1, out2, out3, out4;
    char out_char;
    short out_short;
    double out_dbl;
    void* out_ptr;
    
    int in1 = global_int + 1;
    int in2 = global_int * 2;
    double in_dbl = global_double * 2.0;
    char in_char = global_char + 1;
    short in_short = 500;
    void* in_ptr = &global_int;
    
    /* Complex inline asm with many operands and mixed constraints */
    __asm__ __volatile__ (
        /* Multiple outputs with different constraints */
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out1]\n\t"
        "mov %[in_char], %b[out_char]\n\t"
        "mov %[in_short], %w[out_short]\n\t"
        /* Force memory operand */
        "mov %[mem_in], %%rax\n\t"
        "mov (%%rax), %%rax\n\t"
        "mov %%rax, %[out_ptr]\n\t"
        /* Use immediate */
        "mov $0xDEADBEEF, %[out2]\n\t"
        /* Mixed size operations */
        "cvtsi2sd %[in1], %[out_dbl]\n\t"  /* Integer to double conversion */
        : [out1] "=r" (out1),
          [out2] "=r" (out2),
          [out_char] "=r" (out_char),
          [out_short] "=r" (out_short),
          [out_dbl] "=r" (out_dbl),  /* Will be integer reg with -mno-sse */
          [out_ptr] "=r" (out_ptr)
        : [in1] "r" (in1),
          [in2] "r" (in2),
          [in_char] "r" ((int)in_char),
          [in_short] "r" ((int)in_short),
          [mem_in] "m" (&global_int),
          "i" (0xDEADBEEF)
        : "rax", "memory", "cc"
    );
    
    return out1 + out2 + out_char + out_short + (int)out_dbl + (int)(intptr_t)out_ptr;
}

/* Test 2: Nested function calls in asm operands */
int test_nested_calls(void) {
    int result1, result2, result3;
    double dbl_result;
    
    /* Function calls that must be evaluated before assembly */
    __asm__ __volatile__ (
        "mov %[call1], %%eax\n\t"
        "add %[call2], %%eax\n\t"
        "mov %%eax, %[res1]\n\t"
        "cvtsi2sd %[call3], %[dblres]\n\t"
        : [res1] "=r" (result1),
          [res2] "=r" (result2),
          [dblres] "=r" (dbl_result)
        : [call1] "r" (func_return_int(global_int)),
          [call2] "r" (func_return_int(global_int * 2)),
          [call3] "r" (compute_index())
        : "rax", "memory", "cc"
    );
    
    /* Second asm block using results from first */
    __asm__ __volatile__ (
        "add %[prev], %%eax\n\t"
        "mov %%eax, %[final]\n\t"
        : [final] "=r" (result3)
        : [prev] "r" (result1),
          "a" (result2)  /* Specific register constraint */
        : "cc"
    );
    
    return result1 + result2 + result3 + (int)dbl_result;
}

/* Test 3: Complex addressing modes with array indexing */
int test_complex_addressing(void) {
    int results[10] = {0};
    int index1, index2, index3;
    
    /* Non-constant array indexing */
    index1 = compute_index() % 50;
    index2 = (compute_index() + global_int) % 50;
    index3 = compute_index() % 10;
    
    __asm__ __volatile__ (
        /* Complex memory addressing */
        "mov %[arr], %%rbx\n\t"
        "mov %[idx1], %%rcx\n\t"
        "mov (%[arr],%%rcx,4), %%eax\n\t"  /* arr[idx1] */
        "mov %%eax, %[out1]\n\t"
        
        /* Pointer arithmetic in addressing */
        "lea (%[arr],%[idx2],4), %%rdx\n\t"
        "mov (%%rdx), %%eax\n\t"
        "add $4, %%eax\n\t"
        "mov %%eax, %[out2]\n\t"
        
        /* Multiple memory accesses */
        "mov %[darr], %%rsi\n\t"
        "mov %[idx3], %%rcx\n\t"
        "mov (%%rsi,%%rcx,8), %%rax\n\t"  /* darr[idx3] */
        "mov %%rax, %[out3]\n\t"
        : [out1] "=r" (results[0]),
          [out2] "=r" (results[1]),
          [out3] "=r" (results[2])
        : [arr] "r" (global_array),
          [darr] "r" (global_darray),
          [idx1] "r" (index1),
          [idx2] "r" (index2),
          [idx3] "r" (index3)
        : "rax", "rbx", "rcx", "rdx", "rsi", "memory", "cc"
    );
    
    return results[0] + results[1] + results[2];
}

/* Test 4: Explicit register variables and mode changes */
int test_register_vars(void) {
    int result = 0;
    double dbl_temp;
    
    /* Initialize register variables */
    reg_var1 = global_int * 2;
    reg_var2 = func_return_int(global_int);
    reg_double = global_double;  /* Will be problematic with -mno-sse */
    
    /* Force moves between register variables and regular vars */
    __asm__ __volatile__ (
        /* Move between explicit register var and regular var */
        "mov %[reg1], %%eax\n\t"
        "add %[reg2], %%eax\n\t"
        /* Mode change: use double as integer */
        "mov %[regdbl], %%rbx\n\t"
        "shr $32, %%rbx\n\t"
        "add %%ebx, %%eax\n\t"
        "mov %%eax, %[result]\n\t"
        : [result] "=r" (result)
        : [reg1] "r" (reg_var1),
          [reg2] "r" (reg_var2),
          [regdbl] "r" ((int)reg_double)  /* Cast forces mode change */
        : "rax", "rbx", "memory", "cc"
    );
    
    /* Second asm with different mode */
    __asm__ __volatile__ (
        "cvtsi2sd %[res], %[dblout]\n\t"
        : [dblout] "=r" (dbl_temp)
        : [res] "r" (result)
        : "memory"
    );
    
    return result + (int)dbl_temp;
}

/* Test 5: Secondary reload triggers with specific constraints */
int test_secondary_reloads(void) {
    int out1, out2;
    int in1 = global_int;
    double in_dbl = global_double;
    
    /* Try to force secondary reloads by using difficult constraints */
    __asm__ __volatile__ (
        /* Specific register constraints that may require moves */
        "mov %[in1], %%eax\n\t"
        "add $1, %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        
        /* Memory constraint with complex address */
        "mov %[mem], %%rbx\n\t"
        "mov (%%rbx), %%ecx\n\t"
        "add %%ecx, %[out1]\n\t"
        
        /* Immediate with register constraint */
        "mov $0x12345678, %[out2]\n\t"
        : [out1] "=a" (out1),  /* Specific output in eax */
          [out2] "=r" (out2)
        : [in1] "r" (in1),     /* General input, may need move to eax */
          [mem] "m" (&global_array[compute_index() % 50]),  /* Complex memory address */
          "i" (0x12345678)
        : "rbx", "rcx", "memory", "cc"
    );
    
    /* Chain of asm statements with interdependent operands */
    int temp;
    __asm__ __volatile__ (
        "mov %[prev], %%ecx\n\t"
        "imul $3, %%ecx\n\t"
        "mov %%ecx, %[temp]\n\t"
        : [temp] "=r" (temp)
        : [prev] "r" (out1)
        : "rcx", "cc"
    );
    
    return out1 + out2 + temp;
}

/* Test 6: Mixed data types in single asm statement */
int test_mixed_types(void) {
    char c_out;
    short s_out;
    int i_out;
    long long ll_out;
    float f_out;  /* Will use integer regs with -mno-sse */
    double d_out; /* Will use integer regs with -mno-sse */
    
    char c_in = global_char;
    short s_in = 1000;
    int i_in = global_int;
    long long ll_in = 0x123456789ABCDEF0LL;
    float f_in = 2.71828f;
    double d_in = global_double;
    
    __asm__ __volatile__ (
        /* Operations on different sized data */
        "mov %[cin], %b[cout]\n\t"
        "mov %[sin], %w[sout]\n\t"
        "mov %[iin], %[iout]\n\t"
        "mov %[llin], %[llout]\n\t"
        
        /* Float/double using integer registers (with -mno-sse) */
        "mov %[fin], %[fout]\n\t"
        "mov %[din], %[dout]\n\t"
        
        /* Mixed operations */
        "add %w[sin], %[iout]\n\t"
        "shl $8, %[llout]\n\t"
        : [cout] "=r" (c_out),
          [sout] "=r" (s_out),
          [iout] "=r" (i_out),
          [llout] "=r" (ll_out),
          [fout] "=r" (*(int*)&f_out),  /* Treat float as int for register */
          [dout] "=r" (*(long long*)&d_out)  /* Treat double as long long */
        : [cin] "r" ((int)c_in),
          [sin] "r" ((int)s_in),
          [iin] "r" (i_in),
          [llin] "r" (ll_in),
          [fin] "r" (*(int*)&f_in),
          [din] "r" (*(long long*)&d_in)
        : "memory"
    );
    
    return c_out + s_out + i_out + (int)ll_out + (int)f_out + (int)d_out;
}

/* Helper functions for nested calls */
int func_return_int(int x) {
    return x * 3 + 7;
}

double func_return_double(double x) {
    return x * 1.5;
}

void* func_return_ptr(void* p) {
    return (char*)p + 8;
}

int compute_index(void) {
    static int counter = 0;
    return (counter++ * 13 + 7) % 100;
}

/* Main function that runs all tests */
int main(void) {
    int checksum = 0;
    
    /* Initialize global arrays */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 3;
    }
    for (int i = 0; i < 50; i++) {
        global_darray[i] = i * 1.5;
    }
    
    /* Run all tests to trigger various reload scenarios */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_complex_addressing();
    checksum += test_register_vars();
    checksum += test_secondary_reloads();
    checksum += test_mixed_types();
    
    /* Final volatile asm to ensure all values are used */
    __asm__ __volatile__ (
        "add $1, %0\n\t"
        : "+r" (checksum)
        :
        : "cc"
    );
    
    /* Return deterministic checksum */
    return checksum & 0xFF;  /* Return lower byte for exit code */
}
