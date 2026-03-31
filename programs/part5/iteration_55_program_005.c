/* reload_coverage.c - Program to trigger various reload types in GCC reload1.cc */
#include <stdint.h>
#include <stdlib.h>

/* Force no optimization merging */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Complex structure to force address computations */
struct MultiLevel {
    int a[3][4][5];
    struct {
        int x;
        int y[2][3];
        volatile int z;
    } inner[2];
    int *ptr_array[8];
};

/* Global volatile to force memory accesses */
volatile int global_volatile = 42;

/* Test 1: Complex array addressing - triggers INPUT_ADDRESS and INPADDR_ADDRESS reloads */
NOINLINE static int test_array_addressing(int a1, int a2, int a3, int a4, int a5, 
                                          int a6, int a7, int a8, int a9, int a10) {
    /* Many local variables to consume registers */
    int v1 = a1 * 2, v2 = a2 + 3, v3 = a3 - 1, v4 = a4 / 2, v5 = a5 ^ 0xFF;
    int v6 = a6 << 2, v7 = a7 >> 1, v8 = a8 | 0x55, v9 = a9 & 0xAA, v10 = ~a10;
    
    /* Multi-dimensional arrays with complex indexing */
    int arr1[4][5][6];
    int arr2[7][8];
    volatile int arr3[9];
    
    /* Force register pressure with many computations */
    int sum = 0;
    
    /* Complex addressing that needs address reloads */
    #pragma GCC unroll 4
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            for (int k = 0; k < 6; k++) {
                /* Nested addressing: arr1[i][j][k] where indices are in registers */
                arr1[i][j][k] = v1 * i + v2 * j + v3 * k;
                
                /* More complex: arr2[arr1[i][j][k] % 7][(i+j+k) % 8] */
                int idx1 = arr1[i][j][k] % 7;
                int idx2 = (i + j + k) % 8;
                arr2[idx1][idx2] = v4 * idx1 + v5 * idx2;
                
                /* Volatile access forces spill */
                arr3[(idx1 + idx2) % 9] = global_volatile;
                
                /* Sum with many operands */
                sum += arr1[i][j][k] + arr2[idx1][idx2] + arr3[(idx1 + idx2) % 9]
                       + v6 * i + v7 * j + v8 * k + v9 * idx1 + v10 * idx2;
            }
        }
    }
    
    return sum;
}

/* Test 2: Structure addressing with inline assembly - triggers OUTPUT_ADDRESS and OUTADDR_ADDRESS */
NOINLINE static int test_struct_asm(struct MultiLevel *s, int n) {
    /* Many local copies of structure elements */
    int t1 = s->inner[0].x;
    int t2 = s->inner[1].x;
    volatile int t3 = s->inner[0].z;
    volatile int t4 = s->inner[1].z;
    
    int *p1 = s->ptr_array[0];
    int *p2 = s->ptr_array[1];
    int *p3 = s->ptr_array[2];
    int *p4 = s->ptr_array[3];
    
    /* Complex structure addressing */
    int sum = 0;
    
    #pragma GCC unroll 2
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 5; k++) {
                /* Multi-level array in structure */
                s->a[i][j][k] = t1 * i + t2 * j + t3 * k;
                
                /* Structure member with 2D array */
                s->inner[i%2].y[j%2][k%3] = t4 * (i + j + k);
                
                /* Inline assembly with memory output to complex address */
                int temp;
                asm volatile (
                    /* Output to memory address that needs computation */
                    "movl %[val], %[mem]\n\t"
                    : [mem] "=m" (s->a[i][j][k])  /* Complex address */
                    : [val] "r" (t1 + t2 + t3 + t4)
                    : "memory"
                );
                
                /* Another asm with multiple clobbers */
                asm volatile (
                    "addl %%eax, %%ebx\n\t"
                    "subl %%ecx, %%edx\n\t"
                    "imull %%esi, %%edi\n\t"
                    :
                    : "a" (i), "b" (j), "c" (k), "d" (t1), "S" (t2), "D" (t3)
                    : "cc"
                );
                
                sum += s->a[i][j][k] + s->inner[i%2].y[j%2][k%3];
            }
        }
    }
    
    /* Pointer chasing with complex addresses */
    int result = 0;
    asm volatile (
        /* Multiple output operands */
        "movl %[ptr1], %%eax\n\t"
        "movl (%%eax), %%ebx\n\t"
        "movl %[ptr2], %%ecx\n\t"
        "addl (%%ecx), %%ebx\n\t"
        "movl %%ebx, %[out1]\n\t"
        "movl %[ptr3], %%edx\n\t"
        "movl (%%edx), %%esi\n\t"
        "movl %[ptr4], %%edi\n\t"
        "addl (%%edi), %%esi\n\t"
        "movl %%esi, %[out2]\n\t"
        : [out1] "=r" (t1), [out2] "=r" (t2)
        : [ptr1] "m" (p1), [ptr2] "m" (p2), [ptr3] "m" (p3), [ptr4] "m" (p4)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    return sum + t1 + t2 + n;
}

/* Test 3: Pointer arithmetic and operand addresses - triggers OPERAND_ADDRESS and OPADDR_ADDR */
NOINLINE static int test_pointer_arithmetic(int *base, int count) {
    /* Many pointer variables */
    int *p1 = base + 1;
    int *p2 = base + 2;
    int *p3 = base + 3;
    int *p4 = base + 4;
    int *p5 = base + 5;
    int *p6 = base + 6;
    int *p7 = base + 7;
    int *p8 = base + 8;
    
    /* Scalar temporaries */
    int v1 = *base;
    int v2 = *p1;
    int v3 = *p2;
    int v4 = *p3;
    int v5 = *p4;
    int v6 = *p5;
    int v7 = *p6;
    int v8 = *p7;
    
    /* Complex pointer expressions */
    int sum = 0;
    
    #pragma GCC unroll 8
    for (int i = 0; i < count; i++) {
        /* Each of these requires address computation before dereference */
        int val1 = *(p1 + i * 2);
        int val2 = *(p2 + i * 3);
        int val3 = *(p3 + i * 4);
        int val4 = *(p4 + i * 5);
        int val5 = *(p5 + i * 6);
        int val6 = *(p6 + i * 7);
        int val7 = *(p7 + i * 8);
        int val8 = *(p8 + i * 9);
        
        /* More complex: pointer to pointer */
        int **pp1 = &p1;
        int **pp2 = &p2;
        int **pp3 = &p3;
        
        /* Dereference through multiple levels */
        int d1 = **pp1;
        int d2 = **pp2;
        int d3 = **pp3;
        
        /* Address of array element with index computation */
        int *addr1 = &((int*)base)[i * 2 + v1];
        int *addr2 = &((int*)base)[i * 3 + v2];
        int *addr3 = &((int*)base)[i * 4 + v3];
        
        /* Use all values in complex expression */
        sum += val1 * val2 + val3 * val4 - val5 * val6 + val7 * val8
               + d1 * d2 * d3 + *addr1 + *addr2 + *addr3
               + v1 * i + v2 * (i+1) + v3 * (i+2) + v4 * (i+3)
               + v5 * (i+4) + v6 * (i+5) + v7 * (i+6) + v8 * (i+7);
               
        /* Volatile write to force spills */
        volatile int *volatile_ptr = (volatile int*)base;
        volatile_ptr[i] = sum;
    }
    
    return sum;
}

/* Test 4: Mixed addressing modes for OTHER and OTHER_ADDRESS reloads */
NOINLINE static int test_mixed_addressing(void) {
    /* Diverse local variables */
    register int r1 asm ("ebx") = 1;
    register int r2 asm ("esi") = 2;
    register int r3 asm ("edi") = 3;
    
    int stack_vars[16];
    volatile int volatile_vars[8];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        stack_vars[i] = i * i;
    }
    
    int result = 0;
    
    /* Mixed addressing patterns */
    for (int i = 0; i < 8; i++) {
        /* Direct register usage */
        int temp = r1 + r2 + r3;
        
        /* Complex array addressing with multiple base registers */
        int idx1 = (temp + i) % 16;
        int idx2 = (temp * i) % 16;
        int idx3 = (temp - i) % 16;
        
        /* Multiple memory accesses with different addressing */
        int val1 = stack_vars[idx1];
        int val2 = stack_vars[idx2];
        int val3 = stack_vars[idx3];
        
        /* Volatile access */
        volatile_vars[i % 8] = val1 + val2 + val3;
        
        /* Inline asm with multiple alternative constraints */
        asm volatile (
            "addl %[in1], %[in2]\n\t"
            "movl %[in2], %[out]\n\t"
            : [out] "=r,m" (temp)  /* Alternative constraints */
            : [in1] "r,i" (val1), [in2] "0,r" (val2)
            : "cc"
        );
        
        /* More complex: address of volatile variable */
        int *volatile_addr = (int*)&volatile_vars[i % 8];
        
        /* Use __builtin_expect to create data dependencies */
        if (__builtin_expect(temp > 100, 0)) {
            *volatile_addr = temp;
        } else {
            *volatile_addr = temp * 2;
        }
        
        result += temp + *volatile_addr;
        
        /* Rotate registers */
        int rot = r1;
        r1 = r2;
        r2 = r3;
        r3 = rot + i;
    }
    
    return result;
}

/* Main driver */
int main(void) {
    int total = 0;
    
    /* Initialize data */
    struct MultiLevel s = {0};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 5; k++) {
                s.a[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    s.inner[0].x = 1;
    s.inner[1].x = 2;
    s.inner[0].z = 3;
    s.inner[1].z = 4;
    
    int array_data[100];
    for (int i = 0; i < 100; i++) {
        array_data[i] = i * 3;
    }
    
    /* Call all test functions to trigger different reload patterns */
    total += test_array_addressing(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    total += test_struct_asm(&s, 42);
    total += test_pointer_arithmetic(array_data, 10);
    total += test_mixed_addressing();
    
    /* Use result to prevent dead code elimination */
    volatile int dummy = total;
    
    return total > 0 ? 0 : 1;
}
