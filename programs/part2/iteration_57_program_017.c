/* reload_stress_test.c
 * Designed to stress GCC's reload mechanism and trigger push_reload initialization
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to create dependencies */
int global_int = 42;
double global_double = 3.14159;
char global_array[256];
int *global_ptr = &global_int;

/* Function that returns values needing computation */
int compute_value(int x) {
    return x * 2 + 1;
}

double compute_double(int x) {
    return (double)x / 7.0;
}

int* compute_pointer(int *base, int offset) {
    return base + offset;
}

/* Test 1: Many operands exhausting registers */
int test_many_operands(void) {
    register int r0 asm ("r12") = 1;
    register int r1 asm ("r13") = 2;
    register int r2 asm ("r14") = 3;
    register int r3 asm ("r15") = 4;
    
    int a = 10, b = 20, c = 30, d = 40, e = 50, f = 60, g = 70, h = 80;
    int i = 90, j = 100, k = 110, l = 120, m = 130, n = 140, o = 150, p = 160;
    double d1 = 1.1, d2 = 2.2, d3 = 3.3, d4 = 4.4;
    char ch1 = 'A', ch2 = 'B';
    short s1 = 1000, s2 = 2000;
    
    /* Complex inline assembly with many operands of different types */
    __asm__ __volatile__ (
        "/* Many operand test */\n\t"
        "addl %[a], %[b]\n\t"
        "addl %[c], %[d]\n\t"
        "imull %[e], %[f]\n\t"
        "addl %%r12d, %[g]\n\t"
        "addl %%r13d, %[h]\n\t"
        "movl %[i], %%eax\n\t"
        "addl %[j], %%eax\n\t"
        "movl %%eax, %[k]\n\t"
        "movw %w[s1], %%cx\n\t"
        "addw %w[s2], %%cx\n\t"
        "movw %%cx, %w[l]\n\t"
        "movb %b[ch1], %%dl\n\t"
        "addb %b[ch2], %%dl\n\t"
        "movb %%dl, %b[m]\n\t"
        : [b] "+r" (b), [d] "+r" (d), [f] "+r" (f), [g] "+r" (g),
          [h] "+r" (h), [k] "=r" (k), [l] "=r" (l), [m] "=r" (m)
        : [a] "r" (a), [c] "r" (c), [e] "r" (e), [i] "r" (i),
          [j] "r" (j), [s1] "r" (s1), [s2] "r" (s2), 
          [ch1] "r" (ch1), [ch2] "r" (ch2),
          "r" (r0), "r" (r1)  /* Explicit register vars */
        : "rax", "rcx", "rdx", "memory", "cc"
    );
    
    return a + b + c + d + e + f + g + h + i + j + k + l + m;
}

/* Test 2: Nested function calls in operands */
int test_nested_calls(void) {
    int x = 5, y = 10, z = 15;
    double dx, dy;
    int *ptr1, *ptr2;
    
    /* Function calls as operands - forces evaluation into registers */
    __asm__ __volatile__ (
        "/* Nested calls test */\n\t"
        "movl %[call1], %%eax\n\t"
        "addl %[call2], %%eax\n\t"
        "movl %%eax, %[result]\n\t"
        : [result] "=r" (x)
        : [call1] "r" (compute_value(y)), 
          [call2] "r" (compute_value(z + global_int))
        : "rax", "memory", "cc"
    );
    
    /* Mixed types with function calls */
    __asm__ __volatile__ (
        "/* Mixed type calls */\n\t"
        "movq %[dbl_call], %%xmm0\n\t"
        "cvtsd2si %%xmm0, %%eax\n\t"
        "addl %[int_call], %%eax\n\t"
        "movl %%eax, %[out]\n\t"
        : [out] "=r" (y)
        : [dbl_call] "r" (compute_double(x)),
          [int_call] "r" (compute_value(x * 2))
        : "rax", "xmm0", "memory", "cc"
    );
    
    /* Pointer arithmetic with function calls */
    __asm__ __volatile__ (
        "/* Pointer arithmetic */\n\t"
        "movq %[ptr_expr], %%rax\n\t"
        "movl (%%rax), %%ebx\n\t"
        "addl %[offset], %%ebx\n\t"
        "movl %%ebx, %[ptr_result]\n\t"
        : [ptr_result] "=r" (z)
        : [ptr_expr] "r" (compute_pointer(&global_int, y)),
          [offset] "r" (compute_value(x))
        : "rax", "rbx", "memory", "cc"
    );
    
    return x + y + z;
}

/* Test 3: Complex addressing modes and memory clobbers */
int test_addressing_modes(void) {
    int array[100];
    int *ptr = array;
    int i, j, k, sum = 0;
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        array[i] = i * 2;
    }
    
    /* Complex addressing with non-constant offsets */
    for (i = 0; i < 10; i++) {
        int offset = compute_value(i);
        
        __asm__ __volatile__ (
            "/* Complex addressing */\n\t"
            "movl %[base], %%eax\n\t"
            "movl %[idx], %%ebx\n\t"
            "movl (%%rax, %%rbx, 4), %%ecx\n\t"
            "addl %%ecx, %[sum]\n\t"
            : [sum] "+r" (sum)
            : [base] "r" (ptr), 
              [idx] "r" (offset)  /* Non-constant index */
            : "rax", "rbx", "rcx", "memory", "cc"
        );
    }
    
    /* Multiple memory clobbers in sequence */
    __asm__ __volatile__ (
        "/* Memory barrier 1 */\n\t"
        "mfence\n\t"
        :
        :
        : "memory"
    );
    
    /* Interdependent volatile blocks */
    j = 0;
    k = 0;
    __asm__ __volatile__ (
        "/* Block A */\n\t"
        "movl %[val], %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        : [out1] "=r" (j)
        : [val] "r" (sum)
        : "rax", "memory", "cc"
    );
    
    __asm__ __volatile__ (
        "/* Block B */\n\t"
        "movl %[val], %%ebx\n\t"
        "imull $3, %%ebx\n\t"
        "movl %%ebx, %[out2]\n\t"
        : [out2] "=r" (k)
        : [val] "r" (j)  /* Depends on previous block */
        : "rbx", "memory", "cc"
    );
    
    return sum + j + k;
}

/* Test 4: Mixed data types and mode changes */
int test_mixed_types(void) {
    char c1 = 'X', c2 = 'Y';
    short s1 = 100, s2 = 200;
    int i1 = 1000, i2 = 2000;
    long l1 = 10000, l2 = 20000;
    float f1 = 1.5f, f2 = 2.5f;
    double d1 = 3.14159, d2 = 2.71828;
    
    int result_int = 0;
    double result_double = 0.0;
    
    /* Mixed integer types requiring mode changes */
    __asm__ __volatile__ (
        "/* Mixed integer types */\n\t"
        "movsbl %b[c1], %%eax\n\t"      /* char -> int */
        "movswl %w[s1], %%ebx\n\t"      /* short -> int */
        "addl %%ebx, %%eax\n\t"
        "addl %[i1], %%eax\n\t"
        "addq %[l1], %%rax\n\t"         /* int -> long */
        "movl %%eax, %[out]\n\t"
        : [out] "=r" (result_int)
        : [c1] "r" (c1), [s1] "r" (s1),
          [i1] "r" (i1), [l1] "r" (l1)
        : "rax", "rbx", "memory", "cc"
    );
    
    /* Floating point through integer registers (with -mno-sse) */
    __asm__ __volatile__ (
        "/* FP through integer regs */\n\t"
        "movq %[d1], %%rax\n\t"         /* double in integer reg */
        "movq %%rax, %[temp]\n\t"
        "fldl %[temp]\n\t"              /* load to FPU */
        "movq %[d2], %%rbx\n\t"
        "movq %%rbx, %[temp2]\n\t"
        "faddl %[temp2]\n\t"
        "fstpl %[temp3]\n\t"
        "movq %[temp3], %%rcx\n\t"
        "movq %%rcx, %[dout]\n\t"
        : [dout] "=r" (result_double),
          [temp] "=m" (*(double*)global_array),
          [temp2] "=m" (*(double*)(global_array + 8)),
          [temp3] "=m" (*(double*)(global_array + 16))
        : [d1] "r" (d1), [d2] "r" (d2)
        : "rax", "rbx", "rcx", "st", "st(1)", "st(2)", "st(3)",
          "st(4)", "st(5)", "st(6)", "st(7)", "memory", "cc"
    );
    
    /* Casting between types */
    __asm__ __volatile__ (
        "/* Type casting */\n\t"
        "movl %[float_val], %%eax\n\t"  /* Treat float bits as int */
        "addl $0x3F800000, %%eax\n\t"   /* Add 1.0f in hex */
        "movl %%eax, %[cast_out]\n\t"
        : [cast_out] "=r" (i2)
        : [float_val] "r" (*(int*)&f1)  /* Reinterpret cast */
        : "rax", "memory", "cc"
    );
    
    return result_int + (int)result_double + i2;
}

/* Test 5: Secondary reload triggers */
int test_secondary_reloads(void) {
    register int acc asm ("eax") = 100;
    register int counter asm ("ecx") = 200;
    int result = 0;
    
    /* Force use of specific registers with conflicting constraints */
    __asm__ __volatile__ (
        "/* Specific register constraints */\n\t"
        "movl %%eax, %%ebx\n\t"         /* eax -> ebx */
        "addl %%ecx, %%ebx\n\t"         /* ecx -> ebx */
        "movl $0x1, %%edx\n\t"
        "testl %%ebx, %%edx\n\t"        /* test sets flags */
        "setz %%al\n\t"                 /* result in al */
        "movzbl %%al, %[out]\n\t"
        : [out] "=r" (result)
        : "a" (acc), "c" (counter)      /* Fixed registers */
        : "rbx", "rdx", "memory", "cc"
    );
    
    /* Memory operand that needs intermediate register */
    int mem_value = 500;
    __asm__ __volatile__ (
        "/* Memory with offset */\n\t"
        "leaq %[mem], %%rax\n\t"
        "movl (%%rax), %%ebx\n\t"
        "addl $100, %%ebx\n\t"
        "movl %%ebx, %[result]\n\t"
        : [result] "=r" (result)
        : [mem] "m" (*(struct {int a; char b; int c;}*)&mem_value)
        : "rax", "rbx", "memory", "cc"
    );
    
    /* Immediate that might need register */
    __asm__ __volatile__ (
        "/* Large immediate */\n\t"
        "movl $0x12345678, %%eax\n\t"
        "addl %[input], %%eax\n\t"
        "movl %%eax, %[out]\n\t"
        : [out] "=r" (result)
        : [input] "r" (result)
        : "rax", "memory", "cc"
    );
    
    return result;
}

/* Main function orchestrating all tests */
int main(void) {
    int checksum = 0;
    
    /* Initialize global array */
    memset(global_array, 0, sizeof(global_array));
    for (int i = 0; i < 256; i++) {
        global_array[i] = (char)(i % 256);
    }
    
    printf("Starting reload stress tests...\n");
    
    /* Run all tests and accumulate checksum */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_addressing_modes();
    checksum += test_mixed_types();
    checksum += test_secondary_reloads();
    
    printf("Final checksum: %d\n", checksum);
    
    /* Use checksum in return to prevent optimization */
    return checksum == 0 ? 1 : 0;
}
