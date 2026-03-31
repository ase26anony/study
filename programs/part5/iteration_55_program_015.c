/* reload_coverage.c - Test program to trigger various reload types in GCC */
#include <stdint.h>
#include <stdlib.h>

/* Force frame pointer usage and prevent optimizations */
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

/* Test 1: Complex array addressing with multiple index calculations */
static __attribute__((noinline)) 
int test_complex_addressing(int iter) {
    /* Many local variables to consume registers */
    register int r0 asm("eax") = iter;
    int r1 = iter * 2;
    int r2 = iter * 3;
    int r3 = iter * 4;
    int r4 = iter * 5;
    int r5 = iter * 6;
    int r6 = iter * 7;
    int r7 = iter * 8;
    int r8 = iter * 9;
    int r9 = iter * 10;
    int r10 = iter * 11;
    int r11 = iter * 12;
    int r12 = iter * 13;
    int r13 = iter * 14;
    int r14 = iter * 15;
    int r15 = iter * 16;
    
    /* Volatile to force memory accesses */
    volatile int v1 = r0;
    volatile int v2 = r1;
    
    /* Complex 3D array-like addressing - forces address reloads */
    int *ptr1 = &global_array[r0 + r1 * 4 + r2 * 16];
    int *ptr2 = &global_array[r3 + r4 * 4 + r5 * 16];
    int *ptr3 = &global_array[r6 + r7 * 4 + r8 * 16];
    int *ptr4 = &global_array[r9 + r10 * 4 + r11 * 16];
    
    /* Nested addressing: address of address computation */
    int **ptr_to_ptr = &ptr1;
    
    /* Multiple uses of same value after clobbering - forces input reloads */
    int sum = *ptr1 + *ptr2;
    asm volatile("" : "+r"(r0), "+r"(r1), "+r"(r2), "+r"(r3),
                     "+r"(r4), "+r"(r5), "+r"(r6), "+r"(r7) : : "memory");
    
    /* Now r0-r7 are clobbered, need reloads for remaining uses */
    sum += *ptr3 + *ptr4 + r8 + r9 + r10 + r11;
    
    /* More complex addressing with pointer arithmetic */
    struct BigStruct *sptr = &global_structs[r12 & 0xF];
    int *arr_ptr = &sptr->arr[r13 & 0x7];
    
    /* Force output address reload with inline asm */
    int output;
    asm volatile("movl %1, %0\n\t"
                 : "=m"(*(int (*)[4])arr_ptr)  /* Complex output address */
                 : "r"(sum)
                 : "memory");
    
    return sum + *arr_ptr + r14 + r15;
}

/* Test 2: Inline assembly with multiple constraints */
static __attribute__((noinline))
int test_asm_reloads(int a, int b, int c, int d) {
    int result1, result2, result3, result4;
    volatile int mem1, mem2, mem3, mem4;
    
    /* Multiple output operands with memory constraints */
    asm volatile(
        "addl %5, %0\n\t"
        "subl %6, %1\n\t"
        "imull %7, %2\n\t"
        "andl %8, %3\n\t"
        : "=r"(result1), "=r"(result2), "=r"(result3), "=r"(result4),
          "=m"(mem1), "=m"(mem2)
        : "r"(a), "r"(b), "r"(c), "r"(d),
          "m"(mem3), "m"(mem4)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* Operand address reload: use address of result as input */
    int *ptr_array[] = {&result1, &result2, &result3, &result4};
    
    /* Complex addressing mode */
    int idx = (a + b) & 3;
    int **ptr_to_result = &ptr_array[idx];
    
    /* Force operand address reload */
    asm volatile("" : : "r"(ptr_to_result), "m"(*ptr_array[0]) : "memory");
    
    return result1 + result2 + result3 + result4 + *ptr_array[idx];
}

/* Test 3: Structure member access with offset calculations */
static __attribute__((noinline))
int test_struct_addressing(int base) {
    struct BigStruct local_struct;
    struct BigStruct *structs[8];
    volatile int offsets[8];
    
    /* Initialize pointers with complex expressions */
    for (int i = 0; i < 8; i++) {
        offsets[i] = (base + i * 17) & 0xFF;
        structs[i] = &global_structs[offsets[i] & 0xF];
    }
    
    /* Multiple structure member accesses with different offsets */
    int sum = 0;
    
    /* Manual loop unrolling for register pressure */
    sum += structs[0]->a + structs[0]->arr[offsets[0] & 0x7];
    sum += structs[1]->b + structs[1]->arr[offsets[1] & 0x7];
    sum += structs[2]->c + structs[2]->arr[offsets[2] & 0x7];
    sum += structs[3]->d + structs[3]->arr[offsets[3] & 0x7];
    sum += structs[4]->e + structs[4]->arr[offsets[4] & 0x7];
    sum += structs[5]->f + structs[5]->arr[offsets[5] & 0x7];
    sum += structs[6]->g + structs[6]->arr[offsets[6] & 0x7];
    sum += structs[7]->h + structs[7]->arr[offsets[7] & 0x7];
    
    /* Pointer chasing with address reloads */
    struct BigStruct *current = &local_struct;
    for (int i = 0; i < 4; i++) {
        current->volatile_member = sum + i;
        /* Force address computation reload */
        int *member_ptr = &current->arr[i];
        asm volatile("" : "+r"(member_ptr) : : "memory");
        sum += *member_ptr;
    }
    
    return sum;
}

/* Test 4: Mixed reload types with unrolled loops */
static __attribute__((noinline))
int test_mixed_reloads(int seed) {
    int data[16];
    int *ptr_array[16];
    volatile int temp;
    
    /* Initialize with complex patterns */
    for (int i = 0; i < 16; i++) {
        data[i] = seed + i * 3;
        ptr_array[i] = &data[(i * 5) & 0xF];
    }
    
    int result = 0;
    
    /* Unrolled computation with many intermediate values */
    #pragma GCC unroll 4
    for (int i = 0; i < 16; i++) {
        /* Input reload: value used multiple times after asm clobber */
        int val = data[i] + *ptr_array[i];
        
        /* Clobber many registers */
        asm volatile("" : : : 
            "eax", "ebx", "ecx", "edx", "esi", "edi");
        
        /* Now need reload of 'val' */
        result += val + ptr_array[i][1];
        
        /* Output address reload with complex expression */
        int *output_addr = &data[(i + 1) & 0xF];
        temp = result;
        asm volatile("movl %1, %0\n\t"
                     : "=m"(*output_addr)
                     : "r"(temp)
                     : "memory");
    }
    
    /* Operand address reload for pointer array */
    int **ptr_to_ptr_array = &ptr_array[0];
    asm volatile("" : : "r"(ptr_to_ptr_array), "m"(ptr_array[0]) : "memory");
    
    return result;
}

/* Main driver that calls all tests */
int main(void) {
    /* Initialize globals */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 2;
    }
    
    for (int i = 0; i < 16; i++) {
        global_structs[i].a = i;
        global_structs[i].b = i * 2;
        global_structs[i].c = i * 3;
        global_structs[i].d = i * 4;
        global_structs[i].e = i * 5;
        global_structs[i].f = i * 6;
        global_structs[i].g = i * 7;
        global_structs[i].h = i * 8;
        for (int j = 0; j < 8; j++) {
            global_structs[i].arr[j] = i + j;
        }
    }
    
    int total = 0;
    
    /* Call tests multiple times with different arguments */
    for (int i = 0; i < 100; i++) {
        total += test_complex_addressing(i);
        total += test_asm_reloads(i, i+1, i+2, i+3);
        total += test_struct_addressing(i * 7);
        total += test_mixed_reloads(i * 11);
    }
    
    /* Use result to prevent dead code elimination */
    volatile int final_result = total;
    return final_result % 256;
}

#pragma GCC pop_options
