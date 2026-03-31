/* reload_stress_test.c
 * Designed to stress GCC's reload mechanism and trigger push_reload initialization.
 * Compile with: gcc -O3 -fno-omit-frame-pointer -fno-strict-aliasing -march=x86-64 -mno-sse -mno-avx reload_stress_test.c -o reload_test
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create dependencies and prevent optimization */
volatile int global_counter = 0;
volatile double global_double = 3.14159;
volatile char* global_ptr = (char*)0x1000;
int global_array[256] = {0};

/* Helper functions that return values needing computation */
int compute_value(int x) {
    return x * 2 + 1;
}

double compute_double(int x) {
    return (double)x / 3.0;
}

int* get_pointer(int index) {
    return &global_array[index & 255];
}

/* Test 1: Many operands exhausting registers */
int test_many_operands(void) {
    register int r0 asm ("r12") = global_counter++;
    register int r1 asm ("r13") = global_counter++;
    register int r2 asm ("r14") = global_counter++;
    register int r3 asm ("r15") = global_counter++;
    
    int out0, out1, out2, out3, out4, out5, out6, out7;
    int in0 = compute_value(1);
    int in1 = compute_value(2);
    int in2 = compute_value(3);
    int in3 = compute_value(4);
    int in4 = compute_value(5);
    int in5 = compute_value(6);
    int in6 = compute_value(7);
    int in7 = compute_value(8);
    
    /* Complex inline asm with many operands forcing register spills */
    __asm__ __volatile__ (
        "mov %[in0], %[out0]\n\t"
        "add %[in1], %[out0]\n\t"
        "mov %[in2], %[out1]\n\t"
        "imul %[in3], %[out1]\n\t"
        "lea (%[in4],%[in5],2), %[out2]\n\t"
        "mov %[r0], %[out3]\n\t"
        "add %[r1], %[out3]\n\t"
        "mov %[r2], %[out4]\n\t"
        "sub %[r3], %[out4]\n\t"
        "mov %[in6], %[out5]\n\t"
        "and %[in7], %[out5]\n\t"
        : [out0] "=r" (out0), [out1] "=r" (out1), [out2] "=r" (out2),
          [out3] "=r" (out3), [out4] "=r" (out4), [out5] "=r" (out5),
          [out6] "=r" (out6), [out7] "=r" (out7)
        : [in0] "r" (in0), [in1] "r" (in1), [in2] "r" (in2), [in3] "r" (in3),
          [in4] "r" (in4), [in5] "r" (in5), [in6] "r" (in6), [in7] "r" (in7),
          [r0] "r" (r0), [r1] "r" (r1), [r2] "r" (r2), [r3] "r" (r3)
        : "memory", "cc"
    );
    
    return out0 + out1 + out2 + out3 + out4 + out5 + out6 + out7;
}

/* Test 2: Mixed data types and mode changes */
int test_mixed_types(void) {
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    int i1 = 100000, i2 = 200000;
    long l1 = 3000000000L, l2 = 4000000000L;
    double d1 = compute_double(10);
    double d2 = compute_double(20);
    
    int out_int;
    short out_short;
    char out_char;
    long out_long;
    
    /* Force mode changes by using different types in same asm */
    __asm__ __volatile__ (
        "movsx %[c1], %[out_int]\n\t"
        "add %[i1], %[out_int]\n\t"
        "mov %[s1], %[out_short]\n\t"
        "addw %[s2], %[out_short]\n\t"
        "mov %[c2], %[out_char]\n\t"
        "mov %[l1], %[out_long]\n\t"
        "add %[l2], %[out_long]\n\t"
        /* Force double to int conversion through memory */
        "fldl %[d1]\n\t"
        "fistpl %[out_int]\n\t"
        : [out_int] "=r" (out_int), [out_short] "=r" (out_short),
          [out_char] "=r" (out_char), [out_long] "=r" (out_long)
        : [c1] "r" ((int)c1), [c2] "r" ((int)c2),
          [s1] "r" ((int)s1), [s2] "r" ((int)s2),
          [i1] "r" (i1), [i2] "r" (i2),
          [l1] "r" (l1), [l2] "r" (l2),
          [d1] "m" (d1), [d2] "m" (d2)
        : "memory", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)", "cc"
    );
    
    return out_int + out_short + out_char + out_long;
}

/* Test 3: Complex addressing modes with function calls in operands */
int test_complex_addressing(void) {
    int index = global_counter++ & 255;
    int offset = compute_value(index);
    int* ptr = get_pointer(index);
    
    int result1, result2, result3;
    
    /* Function calls in asm operands force evaluation before asm */
    __asm__ __volatile__ (
        "mov (%[ptr],%[offset],4), %[result1]\n\t"
        "addl $1, (%[ptr],%[offset],4)\n\t"
        "mov %[result1], %[result2]\n\t"
        "imul %[offset], %[result2]\n\t"
        "lea (%[result1],%[result2],2), %[result3]\n\t"
        : [result1] "=r" (result1), [result2] "=r" (result2), [result3] "=r" (result3)
        : [ptr] "r" (ptr), [offset] "r" (offset),
          "m" (*ptr)  /* Memory input to prevent optimization */
        : "memory", "cc"
    );
    
    /* Another asm with pointer arithmetic */
    int* ptr2 = ptr + compute_value(offset);
    int result4;
    
    __asm__ __volatile__ (
        "mov (%[ptr2]), %[result4]\n\t"
        "add %[index], %[result4]\n\t"
        : [result4] "=r" (result4)
        : [ptr2] "r" (ptr2), [index] "r" (index),
          "m" (*ptr2)
        : "memory", "cc"
    );
    
    return result1 + result2 + result3 + result4;
}

/* Test 4: Secondary reload triggers with specific register constraints */
int test_secondary_reloads(void) {
    int value = compute_value(global_counter++);
    int accumulator_result;
    int memory_result;
    
    /* Force value into accumulator register */
    __asm__ __volatile__ (
        "mov %[value], %%eax\n\t"
        "add $100, %%eax\n\t"
        "mov %%eax, %[acc_result]\n\t"
        : [acc_result] "=r" (accumulator_result)
        : [value] "r" (value)
        : "eax", "memory", "cc"
    );
    
    /* Force memory operand with complex addressing */
    int array_index = compute_value(value) & 255;
    __asm__ __volatile__ (
        "movl $42, %[mem_result]\n\t"
        "addl %%eax, %[mem_result]\n\t"
        : [mem_result] "=m" (global_array[array_index])
        : "a" (accumulator_result)
        : "memory", "cc"
    );
    
    /* Mixed constraints that may need secondary reloads */
    int in1 = 100, in2 = 200, out1;
    __asm__ __volatile__ (
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out1]\n\t"
        : [out1] "=a" (out1)  /* Output must be in eax */
        : [in1] "ri" (in1),   /* Register or immediate */
          [in2] "rm" (in2)    /* Register or memory */
        : "cc"
    );
    
    return accumulator_result + global_array[array_index] + out1;
}

/* Test 5: Chain of volatile asm blocks with interdependent operands */
int test_asm_chains(void) {
    int val1 = 1, val2 = 2, val3 = 3, val4 = 4;
    int tmp1, tmp2, tmp3, tmp4, final_result;
    
    /* Chain 1: val1 -> tmp1 -> tmp2 */
    __asm__ __volatile__ (
        "mov %[v1], %[t1]\n\t"
        "add $10, %[t1]\n\t"
        : [t1] "=r" (tmp1)
        : [v1] "r" (val1)
        : "cc"
    );
    
    __asm__ __volatile__ (
        "mov %[t1], %[t2]\n\t"
        "imul %[v2], %[t2]\n\t"
        : [t2] "=r" (tmp2)
        : [t1] "r" (tmp1), [v2] "r" (val2)
        : "cc"
    );
    
    /* Chain 2: val3 -> tmp3 -> tmp4 with memory clobber */
    __asm__ __volatile__ (
        "mov %[v3], %[t3]\n\t"
        "lea (%[t3],%[t3],2), %[t3]\n\t"
        : [t3] "=r" (tmp3)
        : [v3] "r" (val3)
        : "memory", "cc"
    );
    
    __asm__ __volatile__ (
        "mov %[t3], %[t4]\n\t"
        "sub %[v4], %[t4]\n\t"
        : [t4] "=r" (tmp4)
        : [t3] "r" (tmp3), [v4] "r" (val4)
        : "cc"
    );
    
    /* Final combination */
    __asm__ __volatile__ (
        "mov %[t2], %[final]\n\t"
        "add %[t4], %[final]\n\t"
        "add $1000, %[final]\n\t"
        : [final] "=r" (final_result)
        : [t2] "r" (tmp2), [t4] "r" (tmp4)
        : "cc"
    );
    
    return final_result;
}

/* Test 6: Explicit register variables with spills */
int test_explicit_registers(void) {
    /* Declare many register variables to exhaust specific registers */
    register int a asm ("r8") = compute_value(1);
    register int b asm ("r9") = compute_value(2);
    register int c asm ("r10") = compute_value(3);
    register int d asm ("r11") = compute_value(4);
    register int e asm ("r12") = compute_value(5);
    register int f asm ("r13") = compute_value(6);
    register int g asm ("r14") = compute_value(7);
    register int h asm ("r15") = compute_value(8);
    
    int result1, result2, result3, result4;
    
    /* Use all register variables in complex asm */
    __asm__ __volatile__ (
        "mov %[ra], %[r1]\n\t"
        "add %[rb], %[r1]\n\t"
        "mov %[rc], %[r2]\n\t"
        "imul %[rd], %[r2]\n\t"
        "mov %[re], %[r3]\n\t"
        "sub %[rf], %[r3]\n\t"
        "mov %[rg], %[r4]\n\t"
        "xor %[rh], %[r4]\n\t"
        : [r1] "=r" (result1), [r2] "=r" (result2),
          [r3] "=r" (result3), [r4] "=r" (result4)
        : [ra] "r" (a), [rb] "r" (b), [rc] "r" (c), [rd] "r" (d),
          [re] "r" (e), [rf] "r" (f), [rg] "r" (g), [rh] "r" (h)
        : "memory", "cc"
    );
    
    /* Force some spills by using the results in another asm */
    int final;
    __asm__ __volatile__ (
        "mov %[r1], %[final]\n\t"
        "add %[r2], %[final]\n\t"
        "add %[r3], %[final]\n\t"
        "add %[r4], %[final]\n\t"
        : [final] "=r" (final)
        : [r1] "r" (result1), [r2] "r" (result2),
          [r3] "r" (result3), [r4] "r" (result4)
        : "cc"
    );
    
    return final + a + b + c + d + e + f + g + h;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    /* Run all tests to trigger various reload scenarios */
    checksum += test_many_operands();
    checksum += test_mixed_types();
    checksum += test_complex_addressing();
    checksum += test_secondary_reloads();
    checksum += test_asm_chains();
    checksum += test_explicit_registers();
    
    /* Final volatile asm to ensure all values are used */
    __asm__ __volatile__ (
        "addl $0, %0\n\t"
        : "+r" (checksum)
        :
        : "cc"
    );
    
    printf("Final checksum: %d\n", checksum);
    return checksum & 0xFF;  /* Return lower byte to avoid large values */
}
