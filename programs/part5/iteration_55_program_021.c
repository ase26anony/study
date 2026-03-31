/* reload_coverage.c - Test program to trigger various reload types in GCC */
#include <stdint.h>
#include <stdlib.h>

/* Force frame pointer usage and disable optimizations that reduce register pressure */
#pragma GCC optimize ("O0")
#pragma GCC push_options

/* Structure with many members to force complex addressing */
struct BigStruct {
    int a, b, c, d, e, f, g, h;
    int arr[8];
    struct BigStruct *next;
    volatile int volatile_member;
};

/* Global arrays to force address computations */
static int global_array[256];
static struct BigStruct struct_array[32];
static volatile int volatile_global;

/* Test 1: Complex array addressing with multiple index computations */
static __attribute__((noinline)) 
int test_complex_addressing(int a, int b, int c, int d, int e, int f) {
    /* Many local variables to consume registers */
    int i1 = a + 1, i2 = b + 2, i3 = c + 3, i4 = d + 4;
    int i5 = e + 5, i6 = f + 6, i7 = a * b, i8 = c * d;
    int i9 = e * f, i10 = a + b + c, i11 = d + e + f;
    int i12 = a * c * e, i13 = b * d * f;
    
    /* Complex multi-dimensional style addressing */
    int *ptr1 = &global_array[i1 * 4 + i2];
    int *ptr2 = &global_array[i3 * 8 + i4];
    int *ptr3 = &global_array[i5 * 12 + i6];
    
    /* Nested addressing - triggers RELOAD_FOR_INPUT_ADDRESS */
    int val1 = ptr1[i7 + i8];
    int val2 = ptr2[i9 + i10];
    int val3 = ptr3[i11 + i12];
    
    /* More complex addressing with multiple computations */
    int idx1 = (i1 * i2 + i3) * 2;
    int idx2 = (i4 * i5 + i6) * 3;
    int idx3 = (i7 * i8 + i9) * 4;
    
    /* This should trigger RELOAD_FOR_INPADDR_ADDRESS */
    int result = global_array[idx1] + 
                 global_array[idx2] + 
                 global_array[idx3] +
                 val1 + val2 + val3;
    
    /* Force all values to be used */
    asm volatile ("" : : "r"(result), "r"(i13));
    return result;
}

/* Test 2: Structure member access with pointer chasing */
static __attribute__((noinline))
int test_structure_access(int seed) {
    /* Many local struct pointers */
    struct BigStruct *s1 = &struct_array[seed % 16];
    struct BigStruct *s2 = &struct_array[(seed + 1) % 16];
    struct BigStruct *s3 = &struct_array[(seed + 2) % 16];
    struct BigStruct *s4 = &struct_array[(seed + 3) % 16];
    
    /* Complex structure member computations */
    int sum = 0;
    
    /* Manual loop unrolling to increase register pressure */
    /* Each of these accesses needs address computation */
    sum += s1->arr[0] + s1->arr[1] + s1->arr[2] + s1->arr[3];
    sum += s2->arr[4] + s2->arr[5] + s2->arr[6] + s2->arr[7];
    
    /* Nested structure access - triggers RELOAD_FOR_OPERAND_ADDRESS */
    if (s1->next) {
        sum += s1->next->a + s1->next->b;
    }
    
    /* Volatile access forces memory operations */
    s1->volatile_member = sum;
    s2->volatile_member = sum * 2;
    
    /* Complex expression with many intermediate values */
    int t1 = s3->a * s3->b;
    int t2 = s3->c * s3->d;
    int t3 = s3->e * s3->f;
    int t4 = s3->g * s3->h;
    
    /* This should trigger multiple reload types */
    sum += t1 + t2 + t3 + t4;
    
    /* Inline asm with memory output - triggers RELOAD_FOR_OUTPUT_ADDRESS */
    int output;
    asm volatile (
        "movl %1, %0\n\t"
        "addl $100, %0"
        : "=m"(output)  /* Memory output */
        : "r"(sum)      /* Input in register */
        : "cc"
    );
    
    return output;
}

/* Test 3: Inline assembly with multiple constraints */
static __attribute__((noinline))
int test_asm_reloads(int a, int b, int c, int d) {
    int r1, r2, r3, r4, r5, r6, r7, r8;
    
    /* Complex inline asm with many operands */
    /* This should trigger RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
    asm volatile (
        "imull %[in1], %[out1]\n\t"
        "imull %[in2], %[out2]\n\t"
        "addl %[out1], %[out3]\n\t"
        "addl %[out2], %[out4]\n\t"
        "leal (%[in3],%[in4],4), %[out5]\n\t"
        "movl %[out5], %[out6]\n\t"
        "addl $1, %[out6]"
        : [out1] "=&r"(r1), [out2] "=&r"(r2),
          [out3] "=&r"(r3), [out4] "=&r"(r4),
          [out5] "=&r"(r5), [out6] "=&r"(r6)
        : [in1] "r"(a), [in2] "r"(b),
          [in3] "r"(c), [in4] "r"(d)
        : "cc"
    );
    
    /* Another asm with memory operand and offset */
    int mem_var;
    asm volatile (
        "movl %%eax, %[mem]\n\t"
        : [mem] "=m"(mem_var)
        : "a"(r1 + r2)
        : "memory"
    );
    
    /* Complex pointer arithmetic */
    int *ptr = &global_array[a];
    ptr += b;
    ptr += c;
    ptr += d;
    
    /* Use the pointer with offset - triggers RELOAD_FOR_OPADDR_ADDR */
    int val = *(ptr + r1 + r2);
    
    return r3 + r4 + r5 + r6 + mem_var + val;
}

/* Test 4: Mixed addressing modes with volatile */
static __attribute__((noinline))
int test_mixed_addressing(int base) {
    /* Many local variables */
    volatile int v1 = base + 1;
    volatile int v2 = base + 2;
    volatile int v3 = base + 3;
    volatile int v4 = base + 4;
    int nv1 = base * 2;
    int nv2 = base * 3;
    int nv3 = base * 4;
    int nv4 = base * 5;
    
    /* Complex addressing with volatile base */
    int *addr1 = (int*)((uintptr_t)&global_array[0] + v1 * sizeof(int));
    int *addr2 = (int*)((uintptr_t)&global_array[16] + v2 * sizeof(int));
    int *addr3 = (int*)((int*)&global_array[32] + v3);
    
    /* Multiple dereferences with different addressing */
    int sum = *addr1 + *addr2 + *addr3;
    
    /* More complex: address of a structure member with offset */
    struct BigStruct *s = &struct_array[base % 8];
    int *member_addr = &s->arr[v4 % 8];
    
    /* Chain of computations */
    sum += member_addr[nv1] + member_addr[nv2];
    sum += *(member_addr + nv3) + *(member_addr + nv4);
    
    /* Force address computation into a variable */
    int *final_addr = member_addr + (nv1 * nv2) / 16;
    sum += *final_addr;
    
    return sum;
}

/* Test 5: Nested function calls with register pressure */
static __attribute__((noinline))
int inner_func(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Consume all registers */
    return a + b * 2 + c * 3 + d * 4 + e * 5 + f * 6 + g * 7 + h * 8;
}

static __attribute__((noinline))
int test_nested_calls(int start) {
    /* Call with many arguments - forces register/memory shuffling */
    int r1 = inner_func(start, start+1, start+2, start+3, 
                       start+4, start+5, start+6, start+7);
    int r2 = inner_func(start+8, start+9, start+10, start+11,
                       start+12, start+13, start+14, start+15);
    int r3 = inner_func(start+16, start+17, start+18, start+19,
                       start+20, start+21, start+22, start+23);
    
    /* Use results in complex addressing */
    int idx = (r1 + r2 + r3) % 256;
    int *ptr = &global_array[0];
    
    /* Triple nested addressing */
    int val = ptr[ptr[ptr[idx] % 256] % 256];
    
    return r1 + r2 + r3 + val;
}

/* Main driver that calls all tests */
int main(void) {
    /* Initialize data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    for (int i = 0; i < 32; i++) {
        struct_array[i].a = i;
        struct_array[i].b = i * 2;
        struct_array[i].c = i * 3;
        struct_array[i].d = i * 4;
        struct_array[i].e = i * 5;
        struct_array[i].f = i * 6;
        struct_array[i].g = i * 7;
        struct_array[i].h = i * 8;
        for (int j = 0; j < 8; j++) {
            struct_array[i].arr[j] = i * 10 + j;
        }
        struct_array[i].next = (i < 31) ? &struct_array[i + 1] : NULL;
        struct_array[i].volatile_member = 0;
    }
    
    volatile_global = 42;
    
    int result = 0;
    
    /* Call all test functions with different arguments */
    result += test_complex_addressing(1, 2, 3, 4, 5, 6);
    result += test_structure_access(7);
    result += test_asm_reloads(8, 9, 10, 11);
    result += test_mixed_addressing(12);
    result += test_nested_calls(13);
    
    /* Call them again with different values */
    result += test_complex_addressing(14, 15, 16, 17, 18, 19);
    result += test_structure_access(20);
    result += test_asm_reloads(21, 22, 23, 24);
    result += test_mixed_addressing(25);
    result += test_nested_calls(26);
    
    /* Use result to prevent dead code elimination */
    asm volatile ("" : : "r"(result));
    
    return result % 256;
}

#pragma GCC pop_options
