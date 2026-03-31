/* reload_stress_test.c
 * Designed to stress GCC's reload mechanism and trigger push_reload initialization
 * Compile with: gcc -O3 -fno-omit-frame-pointer -fno-strict-aliasing -march=x86-64 -mno-sse -mno-avx -S reload_stress_test.c
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create register pressure */
int global_int1 = 123;
int global_int2 = 456;
int global_int3 = 789;
double global_double = 3.14159;
char global_char_array[256];
int global_int_array[100];

/* Function to force evaluation into register */
int get_value(int x) { return x * 2 + 1; }
double get_double(double x) { return x * 2.0; }
int* get_pointer(int* p) { return p + 1; }

/* Test 1: Many operands with mixed constraints */
int test_many_operands(void) {
    int out1, out2, out3, out4;
    int in1 = 100, in2 = 200, in3 = 300, in4 = 400;
    double d1 = 1.5, d2 = 2.5;
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    
    /* Complex inline asm with many operands and mixed constraints */
    __asm__ __volatile__ (
        /* Outputs with different constraints */
        "movl %[in1], %[out1]\n\t"
        "addl %[in2], %[out1]\n\t"
        "movl %[in3], %[out2]\n\t"
        "imull %[in4], %[out2]\n\t"
        /* Force memory operations */
        "movb %[c1], (%[mem1])\n\t"
        "movw %[s1], (%[mem2])\n\t"
        /* Mixed size operations */
        "movzbl %[c2], %[out3]\n\t"
        "movzwl %[s2], %[out4]"
        : [out1] "=r" (out1), [out2] "=r" (out2), 
          [out3] "=r" (out3), [out4] "=r" (out4),
          [mem1] "+m" (global_char_array[0]),
          [mem2] "+m" (global_int_array[0])
        : [in1] "r" (in1), [in2] "r" (in2),
          [in3] "r" (in3), [in4] "r" (in4),
          [c1] "r" (c1), [c2] "r" (c2),
          [s1] "r" (s1), [s2] "r" (s2),
          "m" (global_double)  /* Memory input */
        : "cc", "memory"
    );
    
    return out1 + out2 + out3 + out4;
}

/* Test 2: Nested function calls in operands */
int test_nested_calls(void) {
    int result1, result2;
    int* ptr_result;
    
    /* Function calls directly in asm operands */
    __asm__ __volatile__ (
        "movl %%eax, %[res1]\n\t"
        "leal (%%ebx, %%ecx, 2), %[res2]\n\t"
        "movl %%edx, %[ptr]"
        : [res1] "=r" (result1), 
          [res2] "=r" (result2),
          [ptr] "=r" (ptr_result)
        : "a" (get_value(global_int1)),      /* Function call in eax */
          "b" (get_value(global_int2)),      /* Function call in ebx */
          "c" (get_value(global_int3)),      /* Function call in ecx */
          "d" (get_pointer(global_int_array)) /* Function call in edx */
        : "memory"
    );
    
    /* Use pointer arithmetic in another asm */
    int index = 10;
    __asm__ __volatile__ (
        "movl (%[ptr], %[idx], 4), %[res1]"
        : [res1] "=r" (result1)
        : [ptr] "r" (ptr_result),
          [idx] "r" (get_value(index))  /* Nested call for index */
        : "memory"
    );
    
    return result1 + (int)(ptr_result - global_int_array);
}

/* Test 3: Explicit register variables forcing moves */
int test_explicit_registers(void) {
    register int r1 asm ("r12") = 111;
    register int r2 asm ("r13") = 222;
    register int r3 asm ("r14") = 333;
    register double dr1 asm ("xmm0") = 1.0;  /* Will need conversion */
    
    int out1, out2, out3;
    
    /* Mix explicit registers with regular constraints */
    __asm__ __volatile__ (
        "movl %[reg1], %[out1]\n\t"
        "addl %[reg2], %[out1]\n\t"
        "movl %[reg3], %[out2]\n\t"
        /* Force double to int conversion - mode change */
        "cvttsd2si %[dreg], %[out3]"
        : [out1] "=r" (out1),
          [out2] "=r" (out2),
          [out3] "=r" (out3)
        : [reg1] "r" (r1),
          [reg2] "r" (r2),
          [reg3] "r" (r3),
          [dreg] "x" (dr1)  /* xmm register constraint */
        : "cc"
    );
    
    /* Chain of asm statements with interdependent operands */
    __asm__ __volatile__ (
        "addl $100, %0"
        : "+r" (out1)
        :
        : "cc"
    );
    
    __asm__ __volatile__ (
        "subl %1, %0"
        : "+r" (out2)
        : "r" (out1)
        : "cc"
    );
    
    return out1 + out2 + out3;
}

/* Test 4: Mixed data types and mode changes */
int test_mixed_types(void) {
    char c = 'X';
    short s = 1234;
    int i = 56789;
    long long ll = 9876543210LL;
    float f = 2.71828f;
    double d = 1.41421;
    
    int out_int;
    short out_short;
    char out_char;
    double out_double;
    
    /* Mixed type constraints in single asm */
    __asm__ __volatile__ (
        "movsbl %[cin], %%eax\n\t"
        "movswl %[sin], %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "addl %[iin], %%eax\n\t"
        "movl %%eax, %[iout]\n\t"
        /* Force float->int conversion */
        "cvttss2si %[fin], %%ecx\n\t"
        "movl %%ecx, %[iout2]"
        : [iout] "=r" (out_int),
          [iout2] "=r" (out_int)  /* Reuse output */
        : [cin] "r" (c),
          [sin] "r" (s),
          [iin] "r" (i),
          [fin] "x" (f)  /* xmm register for float */
        : "eax", "ebx", "ecx", "cc"
    );
    
    /* Double with memory constraint */
    __asm__ __volatile__ (
        "movsd %[din], %[dout]"
        : [dout] "=m" (out_double)
        : [din] "x" (d)
        :
    );
    
    /* Cast double to int - forces mode change */
    int double_as_int = (int)d;
    __asm__ __volatile__ (
        "addl %[dcast], %[out]"
        : [out] "+r" (out_int)
        : [dcast] "r" (double_as_int)
        : "cc"
    );
    
    return out_int + (int)out_double;
}

/* Test 5: Complex addressing modes with array indexing */
int test_complex_addressing(void) {
    int results[10];
    int* ptrs[5];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 10; i++) {
        results[i] = i * 100;
    }
    for (int i = 0; i < 5; i++) {
        ptrs[i] = &results[i * 2];
    }
    
    /* Complex addressing in asm operands */
    for (int i = 0; i < 5; i++) {
        int idx = i * 2;
        int offset = get_value(i);  /* Function call */
        
        __asm__ __volatile__ (
            "movl (%[base], %[off], 4), %%eax\n\t"
            "addl %%eax, %[sum]"
            : [sum] "+r" (sum)
            : [base] "r" (results),
              [off] "r" (offset)  /* Non-constant offset */
            : "eax", "cc", "memory"
        );
    }
    
    /* Pointer array indexing */
    int* dynamic_ptr = ptrs[get_value(1) % 5];  /* Nested call */
    __asm__ __volatile__ (
        "movl (%[ptr]), %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, (%[ptr])"
        :
        : [ptr] "r" (dynamic_ptr)
        : "eax", "memory"
    );
    
    return sum + *dynamic_ptr;
}

/* Test 6: Secondary reload triggers */
int test_secondary_reloads(void) {
    int value = 0x12345678;
    int result;
    
    /* Try to force accumulator-specific operations */
    __asm__ __volatile__ (
        "movl %[val], %%eax\n\t"
        "bswap %%eax\n\t"
        "movl %%eax, %[res]"
        : [res] "=r" (result)
        : [val] "r" (value)
        : "eax", "cc"
    );
    
    /* Force use of specific register classes */
    register int acc asm ("eax") = result;
    register int cnt asm ("ecx") = 10;
    
    __asm__ __volatile__ (
        "shrl %%cl, %%eax"
        : "+a" (acc)
        : "c" (cnt)
        : "cc"
    );
    
    /* Memory barrier that forces reloads */
    __asm__ __volatile__ (
        "mfence"
        :
        :
        : "memory"
    );
    
    /* Another asm that uses the result */
    int final;
    __asm__ __volatile__ (
        "movl %%eax, %[out]\n\t"
        "andl $0xFF, %[out]"
        : [out] "=r" (final)
        : "a" (acc)
        : "cc"
    );
    
    return final;
}

/* Main function that runs all tests */
int main(void) {
    int checksum = 0;
    
    /* Initialize global arrays */
    for (int i = 0; i < 256; i++) {
        global_char_array[i] = (char)(i % 128);
    }
    for (int i = 0; i < 100; i++) {
        global_int_array[i] = i * 10;
    }
    
    /* Run all tests to stress reload mechanism */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_explicit_registers();
    checksum += test_mixed_types();
    checksum += test_complex_addressing();
    checksum += test_secondary_reloads();
    
    /* Final volatile asm to prevent optimization */
    __asm__ __volatile__ (
        ""
        :
        :
        : "memory"
    );
    
    /* Return deterministic checksum */
    return checksum & 0xFF;  /* Return lower byte for exit code */
}
