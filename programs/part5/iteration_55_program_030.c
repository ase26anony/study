/* reload_coverage.c - Test program to trigger various reload types in GCC */
#include <stdint.h>
#include <stdlib.h>

/* Force frame pointer usage */
#pragma GCC optimize ("no-omit-frame-pointer")

/* Complex data structures to create addressing complexity */
struct Nested {
    int data[4];
    struct Nested *next;
    volatile int *volatile_ptr;
};

struct Matrix {
    int values[8][8];
    struct Nested *rows[8];
    volatile long counters[4];
};

/* Global arrays to prevent optimization */
volatile int global_array[256];
struct Matrix global_matrix;

/* Test 1: Complex array addressing with multiple index calculations */
static __attribute__((noinline)) 
int test_complex_addressing(int a, int b, int c, int d, int e, int f) {
    /* Many local variables to consume registers */
    int idx1 = a * b + c;
    int idx2 = d * e + f;
    int idx3 = (a + b) * (c + d);
    int idx4 = (e + f) * (a + d);
    int idx5 = b * c * d;
    int idx6 = e * f * a;
    int idx7 = (a << 2) + (b << 1);
    int idx8 = (c >> 1) + (d >> 2);
    
    /* Multi-level array access forcing address reloads */
    int sum = 0;
    sum += global_matrix.values[idx1 & 7][idx2 & 7];
    sum += global_matrix.values[idx3 & 7][idx4 & 7];
    sum += global_matrix.values[idx5 & 7][idx6 & 7];
    sum += global_matrix.values[idx7 & 7][idx8 & 7];
    
    /* Complex address computation for RELOAD_FOR_INPUT_ADDRESS */
    sum += *(int*)((char*)&global_matrix + 
                   idx1 * sizeof(int) + 
                   idx2 * sizeof(struct Nested*) + 
                   idx3);
    
    /* More address computations */
    sum += global_array[idx1 + idx2 * 16 + idx3];
    sum += global_array[idx4 + idx5 * 8 + idx6];
    
    return sum;
}

/* Test 2: Inline assembly with multiple outputs and clobbers */
static __attribute__((noinline))
int test_asm_reloads(int x, int y, int z) {
    int result1, result2, result3, result4;
    int addr1, addr2, addr3;
    
    /* Complex address computation before asm */
    volatile int* ptr1 = &global_array[x * y + z];
    volatile int* ptr2 = &global_array[y * z + x];
    volatile int* ptr3 = &global_array[z * x + y];
    
    /* Inline asm with multiple outputs and memory operands
       Forces RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    __asm__ volatile (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "imull %[in3], %%eax\n\t"
        "movl %%eax, %[out2]\n\t"
        "leal (%[ptr1], %[in1], 4), %%ebx\n\t"
        "movl (%%ebx), %[out3]\n\t"
        "leal (%[ptr2], %[in2], 2), %%ecx\n\t"
        "movl (%%ecx), %[out4]"
        : [out1] "=m" (result1),
          [out2] "=m" (result2),
          [out3] "=r" (result3),
          [out4] "=r" (result4)
        : [in1] "r" (x),
          [in2] "r" (y),
          [in3] "r" (z),
          [ptr1] "r" (ptr1),
          [ptr2] "r" (ptr2)
        : "eax", "ebx", "ecx", "memory"
    );
    
    /* More complex addressing for RELOAD_FOR_OPERAND_ADDRESS */
    int* volatile_ptr = (int*)((uintptr_t)ptr3 + (x << 2) + (y << 1));
    *volatile_ptr = result1 + result2;
    
    return result3 + result4 + *volatile_ptr;
}

/* Test 3: Structure access with pointer chasing */
static __attribute__((noinline))
int test_structure_reloads(struct Nested *start, int iterations) {
    struct Nested *current = start;
    int sum = 0;
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    
    /* Unrolled loop to increase register pressure */
    #pragma GCC unroll 4
    for (int i = 0; i < iterations && current != NULL; i++) {
        /* Multiple structure member accesses */
        temp1 = current->data[0];
        temp2 = current->data[1];
        temp3 = current->data[2];
        temp4 = current->data[3];
        
        /* Complex expression using all temporaries */
        sum += temp1 * temp2 + temp3 * temp4;
        
        /* Address computation for next pointer access */
        int offset = (temp1 + temp2 + temp3 + temp4) & 3;
        
        /* This creates RELOAD_FOR_INPADDR_ADDRESS */
        sum += *(int*)((char*)current + offset * sizeof(int) + 
                      ((uintptr_t)current >> 2));
        
        /* Pointer chasing with address computation */
        current = (struct Nested*)((char*)current->next + 
                                  (temp1 * sizeof(struct Nested)) % 64);
    }
    
    return sum;
}

/* Test 4: Mixed operand types with volatile accesses */
static __attribute__((noinline))
int test_mixed_operands(int base) {
    /* Many local variables of different types */
    volatile int v1 = base + 1;
    volatile int v2 = base + 2;
    volatile int v3 = base + 3;
    volatile int v4 = base + 4;
    volatile int v5 = base + 5;
    volatile int v6 = base + 6;
    volatile int v7 = base + 7;
    volatile int v8 = base + 8;
    
    int *ptr1, *ptr2, *ptr3, *ptr4;
    int arr1[4], arr2[4], arr3[4], arr4[4];
    
    /* Complex pointer arithmetic */
    ptr1 = &arr1[0] + (v1 & 3);
    ptr2 = &arr2[0] + (v2 & 3);
    ptr3 = &arr3[0] + (v3 & 3);
    ptr4 = &arr4[0] + (v4 & 3);
    
    /* Multiple memory writes forcing output address reloads */
    *ptr1 = v1 * v2;
    *ptr2 = v3 * v4;
    *ptr3 = v5 * v6;
    *ptr4 = v7 * v8;
    
    /* Nested addressing for RELOAD_FOR_OTHER_ADDRESS */
    int **pptr = &ptr1;
    ***((int***)((char*)&pptr + (v1 & 1))) = v1 + v2 + v3;
    
    /* Complex expression using all variables */
    return *ptr1 + *ptr2 + *ptr3 + *ptr4 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
}

/* Test 5: Extreme register pressure with many live ranges */
static __attribute__((noinline))
int test_extreme_pressure(int seed) {
    /* Declare many variables to exhaust registers */
    int a = seed * 1, b = seed * 2, c = seed * 3, d = seed * 4;
    int e = seed * 5, f = seed * 6, g = seed * 7, h = seed * 8;
    int i = seed * 9, j = seed * 10, k = seed * 11, l = seed * 12;
    int m = seed * 13, n = seed * 14, o = seed * 15, p = seed * 16;
    int q = seed * 17, r = seed * 18, s = seed * 19, t = seed * 20;
    int u = seed * 21, v = seed * 22, w = seed * 23, x = seed * 24;
    
    /* Complex expressions keeping all variables live */
    int sum = 0;
    sum += a * b + c * d;
    sum += e * f + g * h;
    sum += i * j + k * l;
    sum += m * n + o * p;
    sum += q * r + s * t;
    sum += u * v + w * x;
    
    /* Address computations interspersed */
    int *ptr_arr[8];
    ptr_arr[0] = &a; ptr_arr[1] = &b; ptr_arr[2] = &c; ptr_arr[3] = &d;
    ptr_arr[4] = &e; ptr_arr[5] = &f; ptr_arr[6] = &g; ptr_arr[7] = &h;
    
    /* Complex addressing pattern */
    for (int idx = 0; idx < 8; idx++) {
        sum += *(ptr_arr[idx] + (idx & 3));
    }
    
    /* More computations */
    sum += (a + b) * (c + d) * (e + f);
    sum += (g + h) * (i + j) * (k + l);
    sum += (m + n) * (o + p) * (q + r);
    
    return sum;
}

/* Initialize global data */
void init_globals(void) {
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            global_matrix.values[i][j] = i * 8 + j;
        }
        global_matrix.rows[i] = NULL;
    }
    
    for (int i = 0; i < 4; i++) {
        global_matrix.counters[i] = 0;
    }
}

/* Main driver function */
int main(void) {
    init_globals();
    
    int total = 0;
    
    /* Call all test functions with different arguments */
    total += test_complex_addressing(1, 2, 3, 4, 5, 6);
    total += test_asm_reloads(7, 8, 9);
    
    /* Create linked structure for test 3 */
    struct Nested nodes[4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            nodes[i].data[j] = i * 4 + j;
        }
        nodes[i].next = (i < 3) ? &nodes[i + 1] : NULL;
        nodes[i].volatile_ptr = &global_array[i * 16];
    }
    total += test_structure_reloads(&nodes[0], 4);
    
    total += test_mixed_operands(100);
    total += test_extreme_pressure(42);
    
    /* Use the result to prevent dead code elimination */
    volatile int result = total;
    
    return result > 0 ? 0 : 1;
}
