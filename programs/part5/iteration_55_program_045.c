/* reload_coverage.c - Test program to trigger various reload types in GCC */
#include <stdint.h>
#include <stdlib.h>

/* Force frame pointer usage and prevent optimizations */
#pragma GCC optimize ("O0")
#pragma GCC push_options

/* Structure with many members to force complex addressing */
struct ComplexData {
    int a, b, c, d, e, f, g, h;
    int arr[8][8];
    struct ComplexData *next;
    volatile int sync;
};

/* Global arrays to create addressing complexity */
static int global_matrix[16][16];
static volatile int volatile_buffer[256];
static struct ComplexData data_pool[4];

/* Test 1: Complex array addressing with multiple index calculations */
__attribute__((noinline))
static int test_complex_addressing(int seed) {
    /* Many local variables to exhaust registers */
    register int r0 asm("eax") = seed;
    int r1 = seed * 2;
    int r2 = seed * 3;
    int r3 = seed * 4;
    int r4 = seed * 5;
    int r5 = seed * 6;
    int r6 = seed * 7;
    int r7 = seed * 8;
    int r8 = seed * 9;
    int r9 = seed * 10;
    int r10 = seed * 11;
    int r11 = seed * 12;
    int r13 = seed * 13;
    int r14 = seed * 14;
    int r15 = seed * 15;
    
    /* Complex array indexing - forces RELOAD_FOR_INPUT_ADDRESS */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        /* Manual loop unrolling for more register pressure */
        sum += global_matrix[r0 + i][r1 + i] + 
               global_matrix[r2 + i][r3 + i] +
               global_matrix[r4 + i][r5 + i] +
               global_matrix[r6 + i][r7 + i];
    }
    
    /* Nested addressing with pointer arithmetic - forces RELOAD_FOR_INPADDR_ADDRESS */
    int *ptr1 = &global_matrix[r8][r9];
    int *ptr2 = &global_matrix[r10][r11];
    int *ptr3 = &global_matrix[r13][r14];
    
    /* Multiple volatile accesses force spills */
    volatile_buffer[r0] = r1;
    volatile_buffer[r2] = r3;
    volatile_buffer[r4] = r5;
    
    /* Complex expression with many operands */
    return sum + *ptr1 + *ptr2 + *ptr3 + r15;
}

/* Test 2: Structure member accesses with inline assembly */
__attribute__((noinline))
static int test_structure_asm(int idx) {
    struct ComplexData local_data[4];
    int results[8];
    
    /* Initialize structure with complex patterns */
    for (int i = 0; i < 4; i++) {
        local_data[i].a = idx + i;
        local_data[i].b = idx * i;
        local_data[i].c = idx - i;
        local_data[i].d = idx / (i + 1);
        local_data[i].e = idx % (i + 2);
        local_data[i].f = idx ^ i;
        local_data[i].g = idx | i;
        local_data[i].h = idx & i;
        
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                local_data[i].arr[j][k] = idx + j * 8 + k;
            }
        }
    }
    
    /* Inline assembly with multiple outputs - forces various reload types */
    int out1, out2, out3, out4;
    asm volatile (
        /* Complex addressing for outputs - forces RELOAD_FOR_OUTPUT_ADDRESS */
        "movl %[val1], %[res1]\n\t"
        "movl %[val2], %[res2]\n\t"
        "addl %[val3], %[res3]\n\t"
        "subl %[val4], %[res4]\n\t"
        : [res1] "=m" (results[0]),   /* Memory output */
          [res2] "=m" (results[1]),   /* Another memory output */
          [res3] "=r" (out3),         /* Register output */
          [res4] "=r" (out4)          /* Register output */
        : [val1] "r" (local_data[0].a),
          [val2] "r" (local_data[1].b),
          [val3] "r" (local_data[2].c),
          [val4] "r" (local_data[3].d)
        : "memory", "cc", "eax", "ebx", "ecx", "edx"
    );
    
    /* More complex structure addressing */
    int total = 0;
    for (int i = 0; i < 4; i++) {
        /* Multi-level structure access */
        total += local_data[i].arr[local_data[i].a & 7][local_data[i].b & 7];
        total += local_data[i].arr[local_data[i].c & 7][local_data[i].d & 7];
        total += local_data[i].arr[local_data[i].e & 7][local_data[i].f & 7];
        total += local_data[i].arr[local_data[i].g & 7][local_data[i].h & 7];
    }
    
    return total + out3 + out4 + results[0] + results[1];
}

/* Test 3: Pointer chasing with complex address computations */
__attribute__((noinline))
static int test_pointer_chasing(int start) {
    /* Initialize linked structure */
    for (int i = 0; i < 3; i++) {
        data_pool[i].next = &data_pool[i + 1];
        data_pool[i].sync = i * 100;
    }
    data_pool[3].next = &data_pool[0];
    
    /* Many pointer variables */
    struct ComplexData *ptr1 = &data_pool[start & 3];
    struct ComplexData *ptr2 = ptr1->next;
    struct ComplexData *ptr3 = ptr2->next;
    struct ComplexData *ptr4 = ptr3->next;
    
    /* Complex pointer arithmetic - forces RELOAD_FOR_OPERAND_ADDRESS */
    int *addr1 = &ptr1->arr[ptr1->a & 7][ptr1->b & 7];
    int *addr2 = &ptr2->arr[ptr2->c & 7][ptr2->d & 7];
    int *addr3 = &ptr3->arr[ptr3->e & 7][ptr3->f & 7];
    int *addr4 = &ptr4->arr[ptr4->g & 7][ptr4->h & 7];
    
    /* Chain of dependent computations */
    int val1 = *addr1 + ptr1->sync;
    int val2 = *addr2 + ptr2->sync + val1;
    int val3 = *addr3 + ptr3->sync + val2;
    int val4 = *addr4 + ptr4->sync + val3;
    
    /* Force address reloads through volatile */
    volatile int *volatile vptr = (volatile int*)addr1;
    int volatile_read = *vptr;
    
    /* Inline asm with memory operand constraints */
    int final_result;
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "addl %[in3], %%eax\n\t"
        "addl %[in4], %%eax\n\t"
        "movl %%eax, %[out]\n\t"
        : [out] "=m" (final_result)
        : [in1] "m" (val1),
          [in2] "m" (val2),
          [in3] "m" (val3),
          [in4] "m" (val4)
        : "eax", "memory", "cc"
    );
    
    return final_result + volatile_read;
}

/* Test 4: Mixed addressing modes with builtins */
__attribute__((noinline))
static int test_mixed_addressing(int base) {
    /* Create many local arrays */
    int arr1[16], arr2[16], arr3[16], arr4[16];
    int *ptr_arr[8];
    
    /* Initialize with complex patterns */
    for (int i = 0; i < 16; i++) {
        arr1[i] = base + i;
        arr2[i] = base * i;
        arr3[i] = base - i;
        arr4[i] = base ^ i;
    }
    
    /* Setup pointer array with different bases */
    ptr_arr[0] = &arr1[base & 15];
    ptr_arr[1] = &arr2[(base + 1) & 15];
    ptr_arr[2] = &arr3[(base + 2) & 15];
    ptr_arr[3] = &arr4[(base + 3) & 15];
    ptr_arr[4] = &global_matrix[base & 15][0];
    ptr_arr[5] = &global_matrix[0][base & 15];
    
    /* Complex pointer dereferencing chain */
    int sum = 0;
    for (int i = 0; i < 6; i++) {
        /* Use __builtin_expect to inhibit optimization */
        if (__builtin_expect(i & 1, 0)) {
            sum += *ptr_arr[i] + *(ptr_arr[i] + 1);
        } else {
            sum += *ptr_arr[i] - *(ptr_arr[i] + 1);
        }
        
        /* Additional address computation */
        sum += *(ptr_arr[i] + (base & 3));
    }
    
    /* Force RELOAD_FOR_OTHER_ADDRESS through obscure pattern */
    int (*complex_addr)[16] = &global_matrix[base & 15];
    int (*another_addr)[16] = &global_matrix[(base + 8) & 15];
    
    /* Mix array and pointer access */
    sum += (*complex_addr)[base & 15];
    sum += (*another_addr)[(base + 7) & 15];
    
    /* Memory barrier to force ordering */
    asm volatile("" ::: "memory");
    
    return sum;
}

/* Main driver that calls all tests */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Initialize global data */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            global_matrix[i][j] = i * 16 + j;
        }
    }
    
    /* Run all tests multiple times with different seeds */
    for (int iter = 0; iter < 3; iter++) {
        int seed = argc > 1 ? atoi(argv[1]) + iter : 42 + iter;
        
        result ^= test_complex_addressing(seed);
        result ^= test_structure_asm(seed * 2);
        result ^= test_pointer_chasing(seed * 3);
        result ^= test_mixed_addressing(seed * 5);
        
        /* Force side effects */
        volatile_buffer[iter] = result;
    }
    
    /* Use result to prevent dead code elimination */
    return result & 0xFF;
}

#pragma GCC pop_options
