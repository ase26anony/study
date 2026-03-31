/* reload_coverage.c - Program to trigger various reload types in GCC reload1.cc */
#include <stdint.h>
#include <stdlib.h>

/* Force frame pointer usage and disable optimizations that might reduce reloads */
#pragma GCC optimize ("O0")
#pragma GCC push_options

/* Structure with many members to force complex addressing */
struct BigStruct {
    int a, b, c, d, e, f, g, h;
    int arr[8];
    struct BigStruct *next;
    volatile int volatile_member;
};

/* Global arrays to provide base addresses */
static int global_array[256];
static struct BigStruct global_structs[16];
static volatile int volatile_global;

/* Test 1: Complex array addressing with multiple index calculations */
static __attribute__((noinline)) 
int test_complex_addressing(int iter) {
    /* Many local variables to consume registers */
    int i1 = iter * 1, i2 = iter * 2, i3 = iter * 3, i4 = iter * 4;
    int i5 = iter * 5, i6 = iter * 6, i7 = iter * 7, i8 = iter * 8;
    int i9 = iter * 9, i10 = iter * 10, i11 = iter * 11, i12 = iter * 12;
    int i13 = iter * 13, i14 = iter * 14, i15 = iter * 15, i16 = iter * 16;
    
    /* Small arrays to force spilling */
    int local_arr1[4], local_arr2[4], local_arr3[4], local_arr4[4];
    
    /* Complex addressing patterns */
    int result = 0;
    
    /* RELOAD_FOR_INPUT: values used multiple times after clobbering */
    int base = global_array[i1 + i2];
    int index = global_array[i3 + i4];
    int scale = global_array[i5 + i6];
    int offset = global_array[i7 + i8];
    
    /* Force address computation into register (RELOAD_FOR_INPUT_ADDRESS) */
    result += global_array[base + index * scale + offset];
    
    /* More complex: address of address computation (RELOAD_FOR_INPADDR_ADDRESS) */
    int *addr_ptr = &global_array[base + index];
    result += *addr_ptr;
    
    /* Nested array access with volatile */
    volatile_global = i9;
    result += global_array[global_array[i10] + volatile_global];
    
    /* Pointer chasing with structure members */
    struct BigStruct *ptr = &global_structs[i11 % 16];
    for (int j = 0; j < 3; j++) {
        /* RELOAD_FOR_OPERAND_ADDRESS: ptr->next needs reloading */
        result += ptr->arr[j];
        ptr = ptr->next;
        if (!ptr) break;
    }
    
    /* Multiple outputs from inline asm to clobber registers */
    int out1, out2, out3, out4;
    asm volatile (
        "movl %5, %0\n\t"
        "movl %6, %1\n\t"
        "movl %7, %2\n\t"
        "movl %8, %3\n\t"
        : "=&r" (out1), "=&r" (out2), "=&r" (out3), "=&r" (out4)
        : "r" (i12), "r" (i13), "r" (i14), "r" (i15)
        : "eax", "ebx", "ecx", "edx", "esi", "edi"
    );
    
    result += out1 + out2 + out3 + out4;
    
    /* Use all local variables to prevent optimization */
    local_arr1[0] = i1; local_arr1[1] = i2; local_arr1[2] = i3; local_arr1[3] = i4;
    local_arr2[0] = i5; local_arr2[1] = i6; local_arr2[2] = i7; local_arr2[3] = i8;
    
    for (int k = 0; k < 4; k++) {
        result += local_arr1[k] + local_arr2[k];
    }
    
    return result;
}

/* Test 2: Structure member accesses with inline assembly outputs */
static __attribute__((noinline))
int test_structure_addressing(int seed) {
    /* Many scalar temporaries */
    int t1 = seed + 1, t2 = seed + 2, t3 = seed + 3, t4 = seed + 4;
    int t5 = seed + 5, t6 = seed + 6, t7 = seed + 7, t8 = seed + 8;
    int t9 = seed + 9, t10 = seed + 10, t11 = seed + 11, t12 = seed + 12;
    
    /* Local structure */
    struct BigStruct s1, s2, s3;
    
    /* Initialize structures */
    s1.a = t1; s1.b = t2; s1.c = t3; s1.d = t4;
    s2.a = t5; s2.b = t6; s2.c = t7; s2.d = t8;
    s3.a = t9; s3.b = t10; s3.c = t11; s3.d = t12;
    
    /* Chain structures */
    s1.next = &s2;
    s2.next = &s3;
    s3.next = &s1;
    
    int sum = 0;
    
    /* Complex structure member addressing */
    sum += s1.arr[s1.a % 4] + s2.arr[s2.b % 4];
    
    /* RELOAD_FOR_OUTPUT_ADDRESS: inline asm with memory output */
    int mem_out;
    asm volatile (
        "movl %1, %0\n\t"
        : "=m" (mem_out)
        : "r" (s1.c + s2.d)
        : "memory"
    );
    sum += mem_out;
    
    /* Multiple memory outputs with complex addresses */
    int out_arr[4];
    for (int i = 0; i < 4; i++) {
        /* RELOAD_FOR_OUTADDR_ADDRESS: address of out_arr[i] needs computation */
        asm volatile (
            "movl %1, %0\n\t"
            : "=m" (out_arr[i])
            : "r" (t1 + t2 + i)
            : "memory"
        );
        sum += out_arr[i];
    }
    
    /* Pointer arithmetic with multiple bases */
    int *ptr1 = &s1.a;
    int *ptr2 = &s2.b;
    int *ptr3 = &s3.c;
    
    /* RELOAD_FOR_OPADDR_ADDR: address of pointer needs reloading */
    sum += *(ptr1 + (t1 % 2));
    sum += *(ptr2 + (t2 % 2));
    sum += *(ptr3 + (t3 % 2));
    
    /* Volatile accesses force memory operations */
    s1.volatile_member = t4;
    s2.volatile_member = t5;
    sum += s1.volatile_member + s2.volatile_member;
    
    return sum;
}

/* Test 3: Nested loops with unrolling for register pressure */
static __attribute__((noinline))
int test_loop_unrolling(int limit) {
    /* Many induction variables */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    
    int accum = 0;
    
    /* Manually unrolled loop */
    #pragma GCC unroll 4
    for (int iter = 0; iter < limit; iter++) {
        /* Complex expressions consuming many registers */
        int temp1 = a * b + c * d - e * f + g * h;
        int temp2 = i * j + k * l - m * n + o * p;
        int temp3 = temp1 * temp2 - a * c * e * g;
        int temp4 = b * d * f * h * j * l * n * p;
        
        /* Array indexing with multiple computations */
        accum += global_array[(temp1 + iter) % 256];
        accum += global_array[(temp2 + iter * 2) % 256];
        accum += global_array[(temp3 + iter * 3) % 256];
        accum += global_array[(temp4 + iter * 4) % 256];
        
        /* Modify many variables to create dependencies */
        a += b; b += c; c += d; d += e;
        e += f; f += g; g += h; h += i;
        i += j; j += k; k += l; l += m;
        m += n; n += o; o += p; p += a;
        
        /* Inline asm to clobber registers periodically */
        if (iter % 4 == 0) {
            asm volatile (
                "/* Clobber many registers */\n\t"
                :
                :
                : "eax", "ebx", "ecx", "edx", "esi", "edi", 
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
            );
        }
    }
    
    /* RELOAD_FOR_OTHER_ADDRESS: obscure addressing mode */
    int (*func_ptr)(int) = test_complex_addressing;
    accum += func_ptr(accum % 8);
    
    return accum;
}

/* Test 4: Mixed operand types with extended asm constraints */
static __attribute__((noinline))
int test_mixed_operands(int x, int y, int z) {
    /* Many local variables with different types */
    int i1 = x, i2 = y, i3 = z;
    volatile int vi1 = x * 2, vi2 = y * 3, vi3 = z * 4;
    int *p1 = &i1, *p2 = &i2, *p3 = &i3;
    int **pp1 = &p1, **pp2 = &p2, **pp3 = &p3;
    
    int result = 0;
    
    /* Extended asm with multiple alternative constraints */
    asm volatile (
        "addl %1, %0\n\t"
        "subl %2, %0\n\t"
        "imull %3, %0\n\t"
        : "+r" (result)
        : "r" (i1), "r" (i2), "r" (i3)
        : "cc"
    );
    
    /* Memory operand with complex address */
    result += *(int *)((char *)pp1 + (x % 4));
    
    /* Inline asm with memory input and register output */
    int asm_out;
    asm volatile (
        "movl (%1), %0\n\t"
        "addl (%2), %0\n\t"
        : "=r" (asm_out)
        : "r" (p1), "r" (p2)
        : "memory"
    );
    result += asm_out;
    
    /* Force RELOAD_OTHER through obscure operand */
    register int r1 asm("ebx") = x;
    register int r2 asm("esi") = y;
    asm volatile (
        "movl %%ebx, %%eax\n\t"
        "addl %%esi, %%eax\n\t"
        : "=a" (result)
        : "b" (r1), "S" (r2)
        : "cc"
    );
    
    /* Use volatile variables */
    vi1 = result;
    vi2 = vi1 + x;
    result = vi2;
    
    return result;
}

/* Main driver that calls all tests */
int main(int argc, char *argv[]) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    for (int i = 0; i < 16; i++) {
        global_structs[i].a = i;
        global_structs[i].b = i * 2;
        global_structs[i].c = i * 3;
        global_structs[i].d = i * 4;
        for (int j = 0; j < 8; j++) {
            global_structs[i].arr[j] = i + j;
        }
        global_structs[i].next = &global_structs[(i + 1) % 16];
        global_structs[i].volatile_member = 0;
    }
    
    volatile_global = 42;
    
    int total = 0;
    
    /* Call each test multiple times with different arguments */
    for (int i = 0; i < 10; i++) {
        total += test_complex_addressing(i);
        total += test_structure_addressing(i * 7);
        total += test_loop_unrolling(8 + (i % 4));
        total += test_mixed_operands(i, i * 2, i * 3);
    }
    
    /* Use result to prevent dead code elimination */
    if (total > 1000000) {
        return 0;
    }
    
    return total & 0xFF;
}

#pragma GCC pop_options
