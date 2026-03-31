/* reload_stress_test.c
 * Designed to stress GCC's reload mechanism and trigger push_reload initialization
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to create dependencies */
int global_int = 42;
double global_double = 3.14159;
char global_char_array[256];
int* global_ptr = &global_int;

/* Complex function that returns values requiring computation */
int compute_value(int base) {
    return base * 2 + 1;
}

double compute_double(int base) {
    return (double)base * 1.5;
}

char* get_string_offset(int offset) {
    return &global_char_array[offset];
}

/* Test 1: Many operands to exhaust registers */
int test_many_operands(void) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    double da = 1.1, db = 2.2, dc = 3.3, dd = 4.4;
    char ca = 'A', cb = 'B', cc = 'C';
    
    /* Force register pressure with many input/output operands */
    __asm__ __volatile__ (
        "/* Many operand test */\n\t"
        "addl %1, %0\n\t"
        "addl %2, %0\n\t"
        "addl %3, %0\n\t"
        "addl %4, %0\n\t"
        "addl %5, %0\n\t"
        "addl %6, %0\n\t"
        "addl %7, %0\n\t"
        "addl %8, %0"
        : "+r"(a), "+r"(b), "+r"(c)
        : "r"(d), "r"(e), "r"(f), "r"(g), "r"(h), 
          "r"(i), "r"(j), "r"(k), "r"(l)
        : "memory", "cc"
    );
    
    /* Mixed types in same asm */
    __asm__ __volatile__ (
        "/* Mixed types */\n\t"
        "mov %1, %%eax\n\t"
        "cvtsi2sd %%eax, %%xmm0\n\t"
        "addsd %2, %%xmm0\n\t"
        "movsd %%xmm0, %0"
        : "=m"(da)
        : "r"(a), "m"(db)
        : "eax", "xmm0", "memory"
    );
    
    return a + b + c + (int)da;
}

/* Test 2: Nested function calls in operands */
int test_nested_calls(void) {
    int result = 0;
    int temp1, temp2, temp3;
    
    /* Function calls that must be evaluated into registers */
    __asm__ __volatile__ (
        "/* Nested calls */\n\t"
        "addl %%ebx, %%eax\n\t"
        "imull %%ecx, %%eax"
        : "=a"(result)
        : "a"(compute_value(10)), 
          "b"(compute_value(20)), 
          "c"(compute_value(30))
        : "memory"
    );
    
    /* Complex addressing with function calls */
    char* ptr;
    __asm__ __volatile__ (
        "/* Complex addressing */\n\t"
        "mov %1, %0\n\t"
        "add %2, %0"
        : "=r"(ptr)
        : "r"(get_string_offset(compute_value(5))),
          "i"(sizeof(int) * compute_value(2))
        : "memory"
    );
    
    /* Multiple volatile asm blocks with dependencies */
    temp1 = result;
    __asm__ __volatile__ (
        "addl $100, %0"
        : "+r"(temp1)
        :: "memory"
    );
    
    temp2 = temp1;
    __asm__ __volatile__ (
        "shrl $2, %0"
        : "+r"(temp2)
        :: "memory", "cc"
    );
    
    return result + temp1 + temp2 + (int)(*ptr);
}

/* Test 3: Explicit register variables and constraints */
int test_explicit_registers(void) {
    /* Explicit register variables */
    register int r1 asm ("r12") = 100;
    register int r2 asm ("r13") = 200;
    register int r3 asm ("r14") = 300;
    register double dr1 asm ("xmm8") = 10.5;
    
    int out1, out2;
    double out3;
    
    /* Force moves between explicit registers */
    __asm__ __volatile__ (
        "/* Explicit register moves */\n\t"
        "mov %1, %%eax\n\t"
        "add %2, %%eax\n\t"
        "mov %%eax, %0\n\t"
        "mov %3, %%xmm0\n\t"
        "addsd %4, %%xmm0\n\t"
        "movsd %%xmm0, %5"
        : "=r"(out1), "=r"(out2), "=x"(out3)
        : "r"(r1), "r"(r2), "x"(dr1), "m"(global_double)
        : "eax", "xmm0", "memory"
    );
    
    /* Specific register constraints that may need secondary reloads */
    int accumulator;
    __asm__ __volatile__ (
        "/* Specific register constraint */\n\t"
        "movl %1, %%eax\n\t"
        "imull %%eax, %%eax\n\t"
        "movl %%eax, %0"
        : "=a"(accumulator)
        : "r"(r3)
        : "memory"
    );
    
    return out1 + out2 + (int)out3 + accumulator;
}

/* Test 4: Mixed data types and mode changes */
int test_mixed_types(void) {
    char c1 = 'X';
    short s1 = 1000;
    int i1 = 50000;
    long long ll1 = 1234567890LL;
    float f1 = 2.71828f;
    double d1 = 3.14159;
    
    int result_int;
    double result_double;
    
    /* Mixing types in same asm - forces mode changes */
    __asm__ __volatile__ (
        "/* Mixed type conversions */\n\t"
        "movsbl %1, %%eax\n\t"
        "movswl %2, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "addl %3, %%eax\n\t"
        "cvtsi2sdl %%eax, %%xmm0\n\t"
        "addsd %4, %%xmm0\n\t"
        "cvtsd2sil %%xmm0, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(result_int)
        : "r"(c1), "r"(s1), "r"(i1), "m"(d1)
        : "eax", "ebx", "xmm0", "memory"
    );
    
    /* Casting between types as operands */
    __asm__ __volatile__ (
        "/* Type casting */\n\t"
        "cvtsi2sdl %1, %%xmm0\n\t"
        "addsd %2, %%xmm0\n\t"
        "movsd %%xmm0, %0"
        : "=m"(result_double)
        : "r"((int)f1), "m"(d1)
        : "xmm0", "memory"
    );
    
    /* Using long long which requires multiple registers on 32-bit */
    long long ll_result;
    __asm__ __volatile__ (
        "/* 64-bit operations */\n\t"
        "movq %1, %%xmm0\n\t"
        "movq %2, %%xmm1\n\t"
        "paddq %%xmm1, %%xmm0\n\t"
        "movq %%xmm0, %0"
        : "=m"(ll_result)
        : "m"(ll1), "m"(1234567890LL)
        : "xmm0", "xmm1", "memory"
    );
    
    return result_int + (int)result_double + (int)ll_result;
}

/* Test 5: Complex memory addressing modes */
int test_complex_addressing(void) {
    int array[100];
    int* ptr_array[50];
    int result = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        array[i] = i * 2;
    }
    for (int i = 0; i < 50; i++) {
        ptr_array[i] = &array[i * 2];
    }
    
    /* Complex addressing with non-constant offsets */
    int index = compute_value(10);
    int offset = compute_value(5);
    
    __asm__ __volatile__ (
        "/* Complex addressing mode */\n\t"
        "mov %1, %%eax\n\t"
        "mov %2, %%ebx\n\t"
        "leal (%%eax, %%ebx, 4), %%ecx\n\t"
        "mov (%%ecx), %%edx\n\t"
        "addl %%edx, %0"
        : "+r"(result)
        : "r"(array), "r"(index)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    /* Pointer chasing */
    int** dptr = &global_ptr;
    __asm__ __volatile__ (
        "/* Pointer indirection */\n\t"
        "mov %1, %%rax\n\t"
        "mov (%%rax), %%rax\n\t"
        "mov (%%rax), %%eax\n\t"
        "addl %%eax, %0"
        : "+r"(result)
        : "r"(dptr)
        : "rax", "memory"
    );
    
    /* Array indexing with computation */
    __asm__ __volatile__ (
        "/* Array with computed index */\n\t"
        "mov %1, %%rax\n\t"
        "mov %2, %%rbx\n\t"
        "shl $2, %%rbx\n\t"
        "add %%rbx, %%rax\n\t"
        "mov (%%rax), %%eax\n\t"
        "addl %%eax, %0"
        : "+r"(result)
        : "r"(ptr_array), "r"(offset)
        : "rax", "rbx", "memory"
    );
    
    return result;
}

/* Test 6: Secondary reload triggers */
int test_secondary_reloads(void) {
    int value = 12345;
    double dvalue = 678.901;
    int result = 0;
    
    /* Try to force secondary reloads with specific constraints */
    __asm__ __volatile__ (
        "/* Potential secondary reload */\n\t"
        "mov %1, %%edi\n\t"
        "test %%edi, %%edi\n\t"
        "setg %%al\n\t"
        "movzx %%al, %%eax\n\t"
        "addl %%eax, %0"
        : "+r"(result)
        : "r"(value)
        : "eax", "edi", "memory", "cc"
    );
    
    /* Using flag-setting instructions */
    __asm__ __volatile__ (
        "/* Flag register pressure */\n\t"
        "cmp $1000, %1\n\t"
        "jg 1f\n\t"
        "addl $10, %0\n\t"
        "jmp 2f\n\t"
        "1:\n\t"
        "addl $20, %0\n\t"
        "2:"
        : "+r"(result)
        : "r"(value)
        : "memory", "cc"
    );
    
    /* Memory constraint that might need temporary register */
    __asm__ __volatile__ (
        "/* Memory constraint */\n\t"
        "fldl %1\n\t"
        "fistpl %0"
        : "=m"(result)
        : "m"(dvalue)
        : "memory", "st"
    );
    
    return result;
}

/* Main function that runs all tests */
int main(void) {
    int checksum = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_char_array[i] = (char)(i % 256);
    }
    
    printf("Starting reload stress tests...\n");
    
    /* Run all tests */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_explicit_registers();
    checksum += test_mixed_types();
    checksum += test_complex_addressing();
    checksum += test_secondary_reloads();
    
    printf("Final checksum: %d\n", checksum);
    
    /* Use checksum in a way that prevents dead code elimination */
    __asm__ __volatile__ (
        "/* Final barrier */\n\t"
        : 
        : "r"(checksum)
        : "memory"
    );
    
    return checksum % 256;
}
