/* reload_test.c - Designed to trigger push_reload initialization block */
#include <stdio.h>
#include <stdint.h>

/* Global variables to create register pressure */
int global_int1 = 123;
int global_int2 = 456;
int global_int3 = 789;
double global_double1 = 3.14159;
double global_double2 = 2.71828;
char global_char_array[256];
int global_int_array[100];

/* Function declarations for nested calls */
int compute_value(int a, int b);
double compute_double(double x);
int* get_pointer(int index);

/* Test functions targeting different reload scenarios */

/* Test 1: Many operands exhausting registers */
int test_many_operands(void) {
    int out1, out2, out3, out4, out5;
    int in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    int in6 = 6, in7 = 7, in8 = 8, in9 = 9, in10 = 10;
    double din1 = 1.1, din2 = 2.2, dout1, dout2;
    
    /* Complex assembly with many operands - forces register spilling */
    __asm__ __volatile__ (
        "mov %[i1], %%eax\n\t"
        "add %[i2], %%eax\n\t"
        "imul %[i3], %%eax\n\t"
        "mov %%eax, %[o1]\n\t"
        "mov %[i4], %%ebx\n\t"
        "sub %[i5], %%ebx\n\t"
        "mov %%ebx, %[o2]\n\t"
        "mov %[i6], %%ecx\n\t"
        "xor %[i7], %%ecx\n\t"
        "mov %%ecx, %[o3]\n\t"
        "mov %[i8], %%edx\n\t"
        "or %[i9], %%edx\n\t"
        "mov %%edx, %[o4]\n\t"
        "mov %[i10], %%esi\n\t"
        "and $0xFF, %%esi\n\t"
        "mov %%esi, %[o5]\n\t"
        : [o1] "=r" (out1), [o2] "=r" (out2), [o3] "=r" (out3),
          [o4] "=r" (out4), [o5] "=r" (out5)
        : [i1] "r" (in1), [i2] "r" (in2), [i3] "r" (in3),
          [i4] "r" (in4), [i5] "r" (in5), [i6] "r" (in6),
          [i7] "r" (in7), [i8] "r" (in8), [i9] "r" (in9),
          [i10] "r" (in10)
        : "eax", "ebx", "ecx", "edx", "esi", "memory"
    );
    
    /* Mixed types in same assembly */
    __asm__ __volatile__ (
        "fldl %[din1]\n\t"
        "faddl %[din2]\n\t"
        "fstpl %[dout1]\n\t"
        "mov %[oi1], %%eax\n\t"
        "cvtsi2sd %%eax, %%xmm0\n\t"
        "movsd %%xmm0, %[dout2]\n\t"
        : [dout1] "=m" (dout1), [dout2] "=m" (dout2)
        : [din1] "m" (din1), [din2] "m" (din2), [oi1] "r" (out1)
        : "eax", "xmm0", "st", "st(1)", "memory"
    );
    
    return out1 + out2 + out3 + out4 + out5 + (int)dout1 + (int)dout2;
}

/* Test 2: Nested function calls in operands */
int test_nested_calls(void) {
    int result1, result2;
    int* ptr_result;
    
    /* Function calls as operands - forces evaluation before assembly */
    __asm__ __volatile__ (
        "mov %[call1], %%eax\n\t"
        "add %[call2], %%eax\n\t"
        "mov %%eax, %[out1]\n\t"
        "lea (%[call3],%[call4],4), %%ebx\n\t"
        "mov %%ebx, %[out2]\n\t"
        : [out1] "=r" (result1), [out2] "=r" (result2)
        : [call1] "r" (compute_value(10, 20)),
          [call2] "r" (compute_value(30, 40)),
          [call3] "r" (get_pointer(5)),
          [call4] "r" (compute_value(2, 3))
        : "eax", "ebx", "memory"
    );
    
    /* Complex addressing mode with array indexing */
    __asm__ __volatile__ (
        "mov %[idx], %%eax\n\t"
        "mov %[arr](,%%eax,4), %%ebx\n\t"
        "mov %%ebx, %[ptrout]\n\t"
        : [ptrout] "=r" (ptr_result)
        : [idx] "r" (compute_value(1, 2)),
          [arr] "r" (global_int_array)
        : "eax", "ebx", "memory"
    );
    
    return result1 + result2 + (*ptr_result);
}

/* Test 3: Explicit register variables and mode changes */
int test_explicit_registers(void) {
    /* Explicit register variables */
    register int reg_var1 asm ("r12") = 100;
    register int reg_var2 asm ("r13") = 200;
    register double reg_double asm ("xmm15") = 5.5;
    int out_int;
    double out_double;
    
    /* Moving between explicit registers - may need reloads */
    __asm__ __volatile__ (
        "mov %[reg1], %%eax\n\t"
        "add %[reg2], %%eax\n\t"
        "cvtsi2sd %%eax, %%xmm0\n\t"
        "addsd %[regd], %%xmm0\n\t"
        "movsd %%xmm0, %[dout]\n\t"
        "mov %%eax, %[iout]\n\t"
        : [iout] "=r" (out_int), [dout] "=m" (out_double)
        : [reg1] "r" (reg_var1), [reg2] "r" (reg_var2),
          [regd] "x" (reg_double)
        : "eax", "xmm0", "memory"
    );
    
    /* Mode change: double to int via memory */
    __asm__ __volatile__ (
        "cvttsd2si %[din], %%eax\n\t"
        "mov %%eax, %[out]\n\t"
        : [out] "=r" (out_int)
        : [din] "m" (out_double)
        : "eax", "memory"
    );
    
    return out_int + (int)out_double + reg_var1 + reg_var2;
}

/* Test 4: Secondary reload triggers */
int test_secondary_reloads(void) {
    int result;
    double dbl_temp;
    
    /* Using specific register constraints that may need secondary reloads */
    __asm__ __volatile__ (
        "pushf\n\t"  /* Flags register - may need secondary reload */
        "mov %[in1], %%eax\n\t"
        "test %%eax, %%eax\n\t"
        "popf\n\t"
        "setz %%al\n\t"
        "movzx %%al, %[out]\n\t"
        : [out] "=r" (result)
        : [in1] "r" (global_int1)
        : "eax", "cc", "memory"
    );
    
    /* Memory constraint with complex address computation */
    __asm__ __volatile__ (
        "fldl %[mem1]\n\t"
        "fsin\n\t"
        "fstpl %[mem2]\n\t"
        : [mem2] "=m" (dbl_temp)
        : [mem1] "m" (global_double1)
        : "st", "st(1)", "memory"
    );
    
    /* Immediate constraint mixed with register constraints */
    __asm__ __volatile__ (
        "mov $0x12345678, %%eax\n\t"
        "add %[in2], %%eax\n\t"
        "mov %%eax, %[out2]\n\t"
        : [out2] "=r" (result)
        : [in2] "r" (global_int2),
          "i" (0x1000)  /* Immediate constraint */
        : "eax", "memory"
    );
    
    return result + (int)dbl_temp;
}

/* Test 5: Volatile chains with interdependent operands */
int test_volatile_chains(void) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int x, y, z;
    
    /* Chain of volatile assembly blocks */
    __asm__ __volatile__ (
        "mov %[a], %%eax\n\t"
        "add %[b], %%eax\n\t"
        "mov %%eax, %[x]\n\t"
        : [x] "=r" (x)
        : [a] "r" (a), [b] "r" (b)
        : "eax", "memory"
    );
    
    __asm__ __volatile__ (
        "mov %[x], %%ebx\n\t"
        "imul %[c], %%ebx\n\t"
        "mov %%ebx, %[y]\n\t"
        : [y] "=r" (y)
        : [x] "r" (x), [c] "r" (c)
        : "ebx", "memory"
    );
    
    __asm__ __volatile__ (
        "mov %[y], %%ecx\n\t"
        "sub %[d], %%ecx\n\t"
        "add %[e], %%ecx\n\t"
        "mov %%ecx, %[z]\n\t"
        : [z] "=r" (z)
        : [y] "r" (y), [d] "r" (d), [e] "r" (e)
        : "ecx", "memory"
    );
    
    /* Use all results in final computation */
    __asm__ __volatile__ (
        "mov %[x], %%eax\n\t"
        "add %[y], %%eax\n\t"
        "add %[z], %%eax\n\t"
        "mov %%eax, %[final]\n\t"
        : [final] "=r" (a)
        : [x] "r" (x), [y] "r" (y), [z] "r" (z)
        : "eax", "memory"
    );
    
    return a;
}

/* Helper functions for nested calls */
int compute_value(int a, int b) {
    return a * b + a - b;
}

double compute_double(double x) {
    return x * x + 2.0 * x + 1.0;
}

int* get_pointer(int index) {
    return &global_int_array[index % 100];
}

/* Main function orchestrating all tests */
int main(void) {
    int checksum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        global_char_array[i] = (char)(i % 128);
    }
    for (int i = 0; i < 100; i++) {
        global_int_array[i] = i * i;
    }
    
    /* Run all tests to trigger various reload scenarios */
    checksum += test_many_operands();
    checksum += test_nested_calls();
    checksum += test_explicit_registers();
    checksum += test_secondary_reloads();
    checksum += test_volatile_chains();
    
    /* Final assembly to ensure all values are used */
    __asm__ __volatile__ (
        "addl $0x1, %0\n\t"
        : "+r" (checksum)
        :
        : "cc", "memory"
    );
    
    printf("Final checksum: %d\n", checksum);
    return checksum;
}
