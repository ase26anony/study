/* reload_stress.c - Stress test for GCC reload.cc push_reload logic */

#include <stdint.h>
#include <stdlib.h>

/* Global variables to create dependencies */
int global_int = 42;
double global_double = 3.14159;
int global_array[100];
volatile int volatile_var = 7;

/* Functions to use in assembly operands */
int func_return_int(void) { return global_int * 2; }
double func_return_double(void) { return global_double * 2.0; }
int* func_return_ptr(void) { return &global_int; }

/* Complex addressing helper */
int array_index(int idx) { return global_array[idx % 100]; }

/* Test 1: Many operands exhausting registers */
int test_many_operands(void) {
    register int r0 asm ("r12") = 1;
    register int r1 asm ("r13") = 2;
    register int r2 asm ("r14") = 3;
    register int r3 asm ("r15") = 4;
    
    int out0, out1, out2, out3, out4, out5, out6, out7;
    int in0 = 10, in1 = 20, in2 = 30, in3 = 40;
    int in4 = 50, in5 = 60, in6 = 70, in7 = 80;
    double d0 = 1.1, d1 = 2.2;
    char c0 = 'a', c1 = 'b';
    short s0 = 100, s1 = 200;
    
    /* Force many reloads with mixed constraints */
    __asm__ __volatile__ (
        "/* Many operand test */\n\t"
        "mov %[in0], %[out0]\n\t"
        "add %[in1], %[out1]\n\t"
        "sub %[in2], %[out2]\n\t"
        "imul %[in3], %[out3]\n\t"
        : [out0] "=r" (out0), [out1] "=r" (out1),
          [out2] "=r" (out2), [out3] "=r" (out3),
          [out4] "=m" (out4), [out5] "=m" (out5)
        : [in0] "r" (in0), [in1] "r" (in1 + r0),
          [in2] "r" (in2 + r1), [in3] "r" (in3 + r2),
          [in4] "m" (in4), [in5] "m" (in5),
          "r" (r3), "r" (global_int)
        : "memory", "cc"
    );
    
    /* Mixed types forcing mode changes */
    __asm__ __volatile__ (
        "/* Mixed type test */\n\t"
        "movzx %w[c0], %[out6]\n\t"
        "movsx %w[s0], %[out7]\n\t"
        : [out6] "=r" (out6), [out7] "=r" (out7)
        : [c0] "r" ((int)c0), [s0] "r" ((int)s0),
          "r" ((long)d0), "r" ((int)d1)  /* Casts force mode changes */
        : "cc"
    );
    
    return out0 + out1 + out2 + out3 + out6 + out7;
}

/* Test 2: Nested function calls in operands */
int test_nested_calls(void) {
    int result1, result2, result3;
    double dresult;
    int* ptr_result;
    
    /* Function calls as operands - must be evaluated into registers */
    __asm__ __volatile__ (
        "/* Nested calls test */\n\t"
        "mov %[call1], %[res1]\n\t"
        "add %[call2], %[res2]\n\t"
        : [res1] "=r" (result1), [res2] "=r" (result2)
        : [call1] "r" (func_return_int()),
          [call2] "r" (array_index(func_return_int() % 50)),
          "r" (volatile_var)  /* Volatile forces reload */
        : "memory", "cc"
    );
    
    /* Pointer arithmetic forcing address reloads */
    int idx = global_int;
    __asm__ __volatile__ (
        "lea (%[base], %[idx], 4), %[ptr]\n\t"
        : [ptr] "=r" (ptr_result)
        : [base] "r" (global_array), [idx] "r" (idx + func_return_int() % 10)
        : "cc"
    );
    
    /* Double with integer operation - mode mixing */
    __asm__ __volatile__ (
        "movq %[dbl], %[ires]\n\t"
        "shr $32, %[ires]\n\t"
        : [ires] "=r" (result3)
        : [dbl] "r" ((long)func_return_double())  /* Cast forces reload */
        : "cc"
    );
    
    return result1 + result2 + result3 + (int)(ptr_result - global_array);
}

/* Test 3: Secondary reload triggers */
int test_secondary_reloads(void) {
    int out1, out2, out3;
    double din = global_double;
    int sin = global_int;
    
    /* Force secondary reloads by using specific register constraints */
    __asm__ __volatile__ (
        "/* Secondary reload test 1 */\n\t"
        "mov %[in1], %%eax\n\t"
        "add $1, %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        : [out1] "=r" (out1)
        : [in1] "r" (sin + func_return_int()),
          "a" (0)  /* Explicit 'a' constraint with different value */
        : "eax", "cc"
    );
    
    /* Memory constraint with complex address */
    __asm__ __volatile__ (
        "/* Secondary reload test 2 */\n\t"
        "mov (%[addr], %[idx], 4), %[out2]\n\t"
        : [out2] "=r" (out2)
        : [addr] "r" (global_array),
          [idx] "r" ((func_return_int() + volatile_var) % 50)
        : "memory", "cc"
    );
    
    /* Immediate constraint mixed with registers */
    __asm__ __volatile__ (
        "/* Mixed constraints */\n\t"
        "imul $0x1234, %[in3], %[out3]\n\t"
        : [out3] "=r" (out3)
        : [in3] "r" (global_int),
          "i" (0x1234)  /* Immediate constraint */
        : "cc"
    );
    
    return out1 + out2 + out3;
}

/* Test 4: Complex operand chains */
int test_operand_chains(void) {
    int values[10];
    int results[10];
    
    /* Initialize array with complex indices */
    for (int i = 0; i < 10; i++) {
        values[i] = (func_return_int() + i * volatile_var) % 100;
    }
    
    /* Chain of assembly blocks with interdependent operands */
    int chain_val = values[0];
    for (int i = 0; i < 5; i++) {
        int next_val;
        __asm__ __volatile__ (
            "add %[inc], %[val]\n\t"
            "mov %[val], %[next]\n\t"
            : [next] "=r" (next_val), [val] "+r" (chain_val)
            : [inc] "r" (values[i + 1]),
              "r" (array_index(chain_val % 100))  /* Extra dependency */
            : "cc"
        );
        results[i] = next_val;
        chain_val = next_val;
    }
    
    /* Mixed size operations in same statement */
    char c1 = 'x', c2 = 'y';
    short s1 = 1000, s2 = 2000;
    int final_result;
    
    __asm__ __volatile__ (
        "/* Mixed sizes */\n\t"
        "movsbl %[c1], %[res]\n\t"
        "movswl %[s1], %%eax\n\t"
        "add %%eax, %[res]\n\t"
        : [res] "=r" (final_result)
        : [c1] "r" ((int)c1), [s1] "r" ((int)s1),
          [c2] "m" (c2), [s2] "m" (s2)  /* Some memory operands */
        : "eax", "cc", "memory"
    );
    
    int sum = final_result;
    for (int i = 0; i < 5; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 5: Floating point with integer registers */
int test_mixed_fp_int(void) {
    double d1 = global_double;
    double d2 = func_return_double();
    float f1 = (float)d1;
    float f2 = (float)d2;
    int iresult1, iresult2, iresult3;
    
    /* Double through integer register - forces mode change */
    __asm__ __volatile__ (
        "movq %[dbl], %[out]\n\t"
        "and $0xFFFFFFFF, %[out]\n\t"
        : [out] "=r" (iresult1)
        : [dbl] "r" ((long)d1)  /* Cast to long forces reload */
        : "cc"
    );
    
    /* Float with integer operations */
    __asm__ __volatile__ (
        "movd %[flt], %[out]\n\t"
        "shl $1, %[out]\n\t"
        : [out] "=r" (iresult2)
        : [flt] "r" ((int)f1)  /* Cast forces reload */
        : "cc"
    );
    
    /* Mixed in same asm */
    __asm__ __volatile__ (
        "/* Mixed fp/int */\n\t"
        "mov %[ival], %[out]\n\t"
        "add %[fval], %[out]\n\t"
        : [out] "=r" (iresult3)
        : [ival] "r" (global_int),
          [fval] "r" ((int)(f2 * 10.0f))  /* Complex expression */
        : "cc"
    );
    
    return iresult1 + iresult2 + iresult3;
}

int main(void) {
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 3;
    }
    
    int checksum = 0;
    
    /* Run all tests to stress reload mechanism */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_secondary_reloads();
    checksum += test_operand_chains();
    checksum += test_mixed_fp_int();
    
    /* Final assembly barrier to prevent optimization */
    __asm__ __volatile__ (
        "/* Final barrier */\n\t"
        :
        :
        : "memory"
    );
    
    return checksum & 0xFF;  /* Return non-zero deterministic result */
}
