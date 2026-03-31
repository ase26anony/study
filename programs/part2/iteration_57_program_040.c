/* reload_stress_test.c
 * Designed to trigger push_reload initialization (lines 1381-1399 in reload.cc)
 * Compile with: gcc -O3 -fno-omit-frame-pointer -fno-strict-aliasing -march=x86-64 -mno-sse -mno-avx reload_stress_test.c -o reload_test
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create dependencies */
volatile int global_int = 42;
volatile double global_double = 3.14159;
volatile char global_char = 'A';
int global_array[100] = {0};

/* Function to force evaluation into registers */
int compute_value(int x) {
    return x * 2 + 1;
}

double compute_double(double x) {
    return x * 1.5;
}

/* Test 1: Many operands with mixed constraints */
int test_many_operands(void) {
    int result = 0;
    
    /* Register variables with explicit registers */
    register int r1 asm ("r10") = 1;
    register int r2 asm ("r11") = 2;
    register int r3 asm ("r12") = 3;
    register int r4 asm ("r13") = 4;
    register int r5 asm ("r14") = 5;
    register int r6 asm ("r15") = 6;
    
    int out1, out2, out3, out4, out5, out6;
    int in1 = 100, in2 = 200, in3 = 300;
    double d1 = 1.5, d2 = 2.5;
    char c1 = 'X';
    
    /* Complex inline assembly with many operands and mixed constraints */
    __asm__ __volatile__ (
        /* Multiple outputs with different constraints */
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out1]\n\t"
        "mov %[r1], %[out2]\n\t"
        "imul %[r2], %[out2]\n\t"
        "lea (%[r3],%[r4],2), %[out3]\n\t"
        /* Force memory operations */
        "mov %[in3], (%[mem])\n\t"
        /* Use explicit register variables */
        "add %%r10, %%r11\n\t"
        "mov %%r11, %[out4]\n\t"
        /* Mixed size operations */
        "movsx %[c1], %[out5]\n\t"
        /* Create dependency chain */
        "mov %[out1], %[out6]\n\t"
        "add $1, %[out6]"
        
        : [out1] "=r" (out1), [out2] "=r" (out2), [out3] "=r" (out3),
          [out4] "=r" (out4), [out5] "=r" (out5), [out6] "=r" (out6)
        : [in1] "r" (in1), [in2] "r" (in2), [in3] "r" (in3),
          [r1] "r" (r1), [r2] "r" (r2), [r3] "r" (r3), [r4] "r" (r4),
          [c1] "r" ((int)c1), [mem] "r" (&global_array[0])
        : "memory", "cc", "r10", "r11", "r12", "r13", "r14", "r15"
    );
    
    result = out1 + out2 + out3 + out4 + out5 + out6;
    return result;
}

/* Test 2: Nested function calls in operands */
int test_nested_calls(void) {
    int result = 0;
    int out1, out2, out3;
    
    /* Function calls as operands - forces evaluation before assembly */
    __asm__ __volatile__ (
        "mov %[call1], %%eax\n\t"
        "add %[call2], %%eax\n\t"
        "add %[idx], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        /* Complex addressing mode */
        "mov (%[arr],%[idx],4), %[out2]\n\t"
        /* Another function call result */
        "add %[call3], %[out2]\n\t"
        /* Immediate constraint */
        "mov $0x%[imm], %[out3]"
        
        : [out1] "=r" (out1), [out2] "=r" (out2), [out3] "=r" (out3)
        : [call1] "r" (compute_value(global_int)),
          [call2] "r" (compute_value(global_int + 1)),
          [call3] "r" (compute_value(global_int + 2)),
          [arr] "r" (global_array),
          [idx] "r" (global_int % 50),
          [imm] "i" (0xDEADBEEF)
        : "memory", "cc", "eax"
    );
    
    result = out1 + out2 + out3;
    return result;
}

/* Test 3: Mixed data types and mode changes */
int test_mixed_types(void) {
    int result = 0;
    int out_int;
    double out_double;
    char out_char;
    short out_short;
    
    int in_int = 255;
    double in_double = 2.71828;
    char in_char = 'Z';
    short in_short = 32767;
    
    /* Mixed types in same assembly - forces mode conversions */
    __asm__ __volatile__ (
        /* Integer operations */
        "mov %[iin], %%eax\n\t"
        "and $0xFF, %%eax\n\t"
        "mov %%eax, %[oint]\n\t"
        /* Double to int conversion */
        "cvtsd2si %[idbl], %%ebx\n\t"
        "mov %%ebx, %[odbl]\n\t"
        /* Char with sign extension */
        "movsx %[ichr], %%ecx\n\t"
        "mov %%ecx, %[ochr]\n\t"
        /* Short operations */
        "mov %[ishrt], %%dx\n\t"
        "movswl %%dx, %%edx\n\t"
        "mov %%edx, %[oshrt]"
        
        : [oint] "=r" (out_int), [odbl] "=r" ((int)out_double),
          [ochr] "=r" (out_char), [oshrt] "=r" (out_short)
        : [iin] "r" (in_int), [idbl] "x" (in_double),
          [ichr] "r" ((int)in_char), [ishrt] "r" ((int)in_short)
        : "memory", "cc", "eax", "ebx", "ecx", "edx", "xmm0"
    );
    
    result = out_int + (int)out_double + out_char + out_short;
    return result;
}

/* Test 4: Secondary reload triggers */
int test_secondary_reloads(void) {
    int result = 0;
    int out1, out2, out3;
    
    /* Try to force specific register constraints */
    register int must_be_eax asm ("eax") = 0x12345678;
    register int must_be_ebx asm ("ebx") = 0x87654321;
    
    __asm__ __volatile__ (
        /* Force accumulator use */
        "xchg %%eax, %[val1]\n\t"
        "mov %%eax, %[out1]\n\t"
        /* Force base register */
        "xchg %%ebx, %[val2]\n\t"
        "mov %%ebx, %[out2]\n\t"
        /* Complex operation requiring intermediate */
        "mov %[val3], %%ecx\n\t"
        "ror $16, %%ecx\n\t"
        "mov %%ecx, %[out3]"
        
        : [out1] "=r" (out1), [out2] "=r" (out2), [out3] "=r" (out3)
        : [val1] "r" (must_be_eax), [val2] "r" (must_be_ebx),
          [val3] "r" (compute_value(global_int * 2))
        : "memory", "cc", "eax", "ebx", "ecx"
    );
    
    result = out1 + out2 + out3;
    return result;
}

/* Test 5: Memory clobber and volatile chains */
int test_memory_clobber(void) {
    int result = 0;
    int values[10];
    
    /* Initialize array */
    for (int i = 0; i < 10; i++) {
        values[i] = i * 10;
    }
    
    int temp1, temp2, temp3;
    
    /* Chain of volatile assembly with memory clobbers */
    __asm__ __volatile__ (
        "mov (%[ptr]), %%eax\n\t"
        "add $1, %%eax\n\t"
        "mov %%eax, %[t1]"
        : [t1] "=r" (temp1)
        : [ptr] "r" (&values[0])
        : "memory", "cc", "eax"
    );
    
    __asm__ __volatile__ (
        "mov %[t1], %%ebx\n\t"
        "imul $2, %%ebx\n\t"
        "mov %%ebx, (%[ptr2])\n\t"
        "mov (%[ptr2]), %[t2]"
        : [t2] "=r" (temp2)
        : [t1] "r" (temp1), [ptr2] "r" (&values[1])
        : "memory", "cc", "ebx"
    );
    
    __asm__ __volatile__ (
        "mov %[t2], %%ecx\n\t"
        "add (%[ptr3]), %%ecx\n\t"
        "mov %%ecx, %[t3]"
        : [t3] "=r" (temp3)
        : [t2] "r" (temp2), [ptr3] "r" (&values[2])
        : "memory", "cc", "ecx"
    );
    
    result = temp1 + temp2 + temp3;
    return result;
}

/* Test 6: Complex addressing with pointer arithmetic */
int test_complex_addressing(void) {
    int result = 0;
    int out1, out2;
    
    /* Complex pointer expressions */
    int *ptr1 = &global_array[global_int];
    int *ptr2 = &global_array[global_char];
    
    __asm__ __volatile__ (
        /* Complex addressing mode */
        "mov (%[p1],%[idx],4), %%eax\n\t"
        "add (%[p2],%[idx],2), %%eax\n\t"
        "mov %%eax, %[o1]\n\t"
        /* More complex: base + index*scale + displacement */
        "mov 16(%[p1],%[idx2],4), %%ebx\n\t"
        "mov %%ebx, %[o2]"
        
        : [o1] "=r" (out1), [o2] "=r" (out2)
        : [p1] "r" (global_array), [p2] "r" (ptr2),
          [idx] "r" (compute_value(5) % 20),
          [idx2] "r" (compute_value(10) % 20)
        : "memory", "cc", "eax", "ebx"
    );
    
    result = out1 + out2;
    return result;
}

int main(void) {
    int checksum = 0;
    
    printf("Starting reload stress test...\n");
    
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * i;
    }
    
    /* Run all tests to trigger various reload patterns */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_mixed_types();
    checksum += test_secondary_reloads();
    checksum += test_memory_clobber();
    checksum += test_complex_addressing();
    
    printf("Checksum: %d\n", checksum);
    
    /* Use checksum to prevent dead code elimination */
    __asm__ __volatile__ (
        ""
        : 
        : "r" (checksum)
        : "memory"
    );
    
    return checksum & 0xFF;  /* Return lower byte */
}
