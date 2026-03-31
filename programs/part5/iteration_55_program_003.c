/* reload_coverage.c - Program to trigger various reload types in GCC reload1.cc */
#include <stdint.h>
#include <stdlib.h>

/* Force specific architecture for register scarcity */
#ifdef __x86_64__
#error "Compile with -m32 for 32-bit x86 to increase register pressure"
#endif

/* Volatile variables to force memory accesses */
static volatile int vol_global = 42;
static volatile int vol_array[256];

/* Complex structure for address computations */
struct nested {
    int a;
    int b[4];
    struct nested *next;
    int c[3][2];
};

/* Test function 1: Focus on INPUT and INPUT_ADDRESS reloads */
__attribute__((noinline))
static int test_input_reloads(int param1, int param2, int param3, 
                              int param4, int param5, int param6) {
    /* Many local variables to exhaust registers */
    int v1 = param1 + vol_global;
    int v2 = param2 * param3;
    int v3 = param4 ^ param5;
    int v4 = param6 & 0xFF;
    int v5 = v1 + v2;
    int v6 = v3 - v4;
    int v7 = v5 * v6;
    int v8 = v2 / (v4 ? v4 : 1);
    int v9 = v7 ^ v8;
    int v10 = v9 + param1;
    
    /* Multi-dimensional array with complex addressing */
    int arr3d[3][4][5];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 5; k++) {
                /* Complex address computation forcing input address reloads */
                arr3d[i][j][k] = 
                    v1 * i + v2 * j + v3 * k + 
                    v4 * (i * j) + v5 * (j * k) + v6 * (k * i);
            }
        }
    }
    
    /* Nested addressing: address itself needs computation */
    int *ptr_arr[10];
    for (int i = 0; i < 10; i++) {
        ptr_arr[i] = &arr3d[i % 3][(i + 1) % 4][(i + 2) % 5];
    }
    
    /* Force RELOAD_FOR_INPUT_ADDRESS with complex base+index*scale */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        /* Complex addressing: *(base + index*scale + offset) */
        sum += *(ptr_arr[i] + (v1 % 3) * 20 + (v2 % 4) * 5 + (v3 % 5));
    }
    
    /* Use all variables again to prevent optimization */
    return sum + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Test function 2: Focus on OUTPUT_ADDRESS and OPERAND_ADDRESS reloads */
__attribute__((noinline))
static int test_output_address_reloads(struct nested *s, int count) {
    volatile int results[100];
    int temp[50];
    
    /* Complex pointer chasing with structure accesses */
    struct nested *current = s;
    int idx = 0;
    
    while (current && idx < 50) {
        /* Multiple structure member accesses */
        temp[idx] = current->a + current->b[idx % 4];
        
        /* Nested array in structure */
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 2; j++) {
                temp[idx] += current->c[i][j] * (i + j * 2);
            }
        }
        
        /* Inline assembly with output to memory address */
        int output_addr = (int)(&results[idx]);
        int input_val = temp[idx];
        
        /* Force RELOAD_FOR_OUTPUT_ADDRESS */
        __asm__ volatile (
            "movl %1, %%eax\n\t"
            "addl $100, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=m" (*(volatile int*)output_addr)  /* Complex output address */
            : "r" (input_val)                     /* Input in register */
            : "%eax", "memory"
        );
        
        current = current->next;
        idx++;
    }
    
    /* Sum results to prevent dead code elimination */
    int total = 0;
    for (int i = 0; i < idx && i < 100; i++) {
        total += results[i];
    }
    return total;
}

/* Test function 3: Focus on OTHER and OTHER_ADDRESS reloads */
__attribute__((noinline))
static int test_other_reloads(int *base_ptr, int offset) {
    /* Many scalar variables */
    int a1 = base_ptr[0] + offset;
    int a2 = base_ptr[1] * offset;
    int a3 = base_ptr[2] ^ offset;
    int a4 = base_ptr[3] & offset;
    int a5 = a1 + a2;
    int a6 = a3 - a4;
    int a7 = a5 * a6;
    int a8 = a2 / (a4 ? a4 : 1);
    int a9 = a7 ^ a8;
    int a10 = a9 + offset;
    int a11 = a10 * 2;
    int a12 = a11 - 1;
    int a13 = a12 & 0xFF;
    int a14 = a13 | 0x55;
    int a15 = a14 << 2;
    int a16 = a15 >> 1;
    int a17 = a16 + a1;
    int a18 = a17 * a2;
    int a19 = a18 ^ a3;
    int a20 = a19 & a4;
    
    /* Complex inline assembly with multiple alternatives */
    int result1, result2;
    
    /* Force RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
    __asm__ volatile (
        "movl (%[ptr1]), %%eax\n\t"
        "addl (%[ptr2]), %%eax\n\t"
        "movl %%eax, %[res1]\n\t"
        "leal (%[ptr3], %[idx], 4), %%ebx\n\t"
        "movl (%%ebx), %%ecx\n\t"
        "movl %%ecx, %[res2]\n\t"
        : [res1] "=r" (result1), [res2] "=r" (result2)
        : [ptr1] "r" (&a1), [ptr2] "r" (&a2), 
          [ptr3] "r" (&a3), [idx] "r" (a4)
        : "%eax", "%ebx", "%ecx", "memory"
    );
    
    /* More complex addressing modes */
    int *ptr1 = &a5 + (a6 % 10);
    int *ptr2 = &a7 + (a8 % 10);
    int *ptr3 = &a9 + (a10 % 10);
    
    /* Force RELOAD_FOR_OTHER_ADDRESS */
    int final = 0;
    for (int i = 0; i < 20; i++) {
        /* Complex address computation */
        int *addr = ptr1 + (ptr2 - ptr1) * (i % 3) + (ptr3 - ptr2) * (i % 2);
        final += *addr * i;
    }
    
    return result1 + result2 + final + a20;
}

/* Test function 4: Mixed reload types with loop unrolling */
#pragma GCC unroll 4
__attribute__((noinline))
static int test_mixed_reloads_unrolled(int seed) {
    /* Large number of local arrays */
    int arr1[8], arr2[8], arr3[8], arr4[8];
    int *ptr_arr[8];
    
    /* Initialize with complex patterns */
    for (int i = 0; i < 8; i++) {
        arr1[i] = seed + i * 3;
        arr2[i] = seed - i * 2;
        arr3[i] = seed ^ (i * 5);
        arr4[i] = seed & (i * 7);
        ptr_arr[i] = &arr1[i] + (arr2[i] % 4);
    }
    
    /* Unrolled computations with complex addressing */
    int sum = 0;
    
    /* Manually unrolled loop */
    sum += *(ptr_arr[0] + arr1[0] % 4) * arr2[0];
    sum += *(ptr_arr[1] + arr1[1] % 4) * arr2[1];
    sum += *(ptr_arr[2] + arr1[2] % 4) * arr2[2];
    sum += *(ptr_arr[3] + arr1[3] % 4) * arr2[3];
    sum += *(ptr_arr[4] + arr1[4] % 4) * arr2[4];
    sum += *(ptr_arr[5] + arr1[5] % 4) * arr2[5];
    sum += *(ptr_arr[6] + arr1[6] % 4) * arr2[6];
    sum += *(ptr_arr[7] + arr1[7] % 4) * arr2[7];
    
    /* More complex nested addressing */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int *base = ptr_arr[i];
            int index = arr3[j];
            int scale = arr4[(i + j) % 8];
            /* Force multiple address reload types */
            sum += *(base + index * (scale % 4 + 1));
        }
    }
    
    return sum;
}

/* Main driver function */
int main(void) {
    int total = 0;
    
    /* Initialize volatile array */
    for (int i = 0; i < 256; i++) {
        vol_array[i] = i * 3;
    }
    
    /* Initialize nested structure */
    struct nested nodes[5];
    for (int i = 0; i < 5; i++) {
        nodes[i].a = i * 10;
        for (int j = 0; j < 4; j++) {
            nodes[i].b[j] = i * 20 + j;
        }
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 2; k++) {
                nodes[i].c[j][k] = i * 30 + j * 10 + k;
            }
        }
        nodes[i].next = (i < 4) ? &nodes[i + 1] : NULL;
    }
    
    /* Initialize data array */
    int data[100];
    for (int i = 0; i < 100; i++) {
        data[i] = i * 7;
    }
    
    /* Call all test functions to trigger different reload types */
    total += test_input_reloads(1, 2, 3, 4, 5, 6);
    total += test_output_address_reloads(&nodes[0], 5);
    total += test_other_reloads(data, 42);
    total += test_mixed_reloads_unrolled(123);
    
    /* Use result to prevent dead code elimination */
    vol_global = total % 1000;
    
    return vol_global;
}
