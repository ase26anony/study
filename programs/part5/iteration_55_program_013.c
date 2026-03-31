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

/* Global arrays to force address computations */
static int global_array_1[256];
static int global_array_2[256];
static int global_array_3[256];
static struct BigStruct global_structs[16];

/* Test 1: Complex array addressing with multiple index computations */
static __attribute__((noinline)) 
int test_complex_addressing(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Many local variables to consume registers */
    volatile int v1 = a * b;
    volatile int v2 = c * d;
    volatile int v3 = e * f;
    volatile int v4 = g * h;
    int i1 = a + b;
    int i2 = c + d;
    int i3 = e + f;
    int i4 = g + h;
    int i5 = a * c;
    int i6 = b * d;
    int i7 = e * g;
    int i8 = f * h;
    
    /* Complex 3D array-style addressing - forces RELOAD_FOR_INPUT_ADDRESS */
    /* The address computation itself needs registers */
    int result = 0;
    
    /* Manual loop unrolling to increase register pressure */
    /* Each access has different index computation */
    result += global_array_1[(i1 + i2) & 0xFF];
    result += global_array_2[(i3 + i4) & 0xFF];
    result += global_array_3[(i5 + i6) & 0xFF];
    result += global_array_1[(i7 + i8) & 0xFF];
    
    /* Nested addressing with multiple levels */
    result += global_array_2[global_array_1[i1 & 0xFF] & 0xFF];
    result += global_array_3[global_array_2[i2 & 0xFF] & 0xFF];
    
    /* Pointer chasing with address computation */
    struct BigStruct *ptr = &global_structs[0];
    for (int i = 0; i < 4; i++) {
        /* Each access needs address reloads */
        result += ptr->arr[(i1 + i) & 0x7];
        result += ptr->arr[(i2 + i) & 0x7];
        ptr = ptr->next;
        if (!ptr) break;
    }
    
    return result + v1 + v2 + v3 + v4;
}

/* Test 2: Inline assembly with multiple outputs and complex constraints */
static __attribute__((noinline))
int test_asm_reloads(int x, int y, int z) {
    int out1, out2, out3, out4;
    volatile int mem1, mem2, mem3;
    
    /* Force values to memory */
    mem1 = x;
    mem2 = y;
    mem3 = z;
    
    /* Complex inline assembly that clobbers many registers */
    /* This can trigger RELOAD_FOR_OUTPUT_ADDRESS and others */
    asm volatile (
        /* Multiple output operands with memory constraints */
        "movl %[in1], %[out1]\n\t"
        "addl %[in2], %[out1]\n\t"
        "movl %[out1], %[out2]\n\t"
        "imull %[in3], %[out2]\n\t"
        /* Complex address computation in output */
        "leal (%[out1], %[out2], 4), %[out3]\n\t"
        /* Force spill/reload by using all registers */
        "movl %%eax, %[out4]\n\t"
        : [out1] "=&r" (out1), 
          [out2] "=&r" (out2),
          [out3] "=&r" (out3),
          [out4] "=m" (global_array_1[x & 0xFF])  /* Memory output with address */
        : [in1] "rm" (mem1),   /* Register or memory */
          [in2] "rm" (mem2),
          [in3] "rm" (mem3)
        : "eax", "ebx", "ecx", "edx", "memory", "cc"
    );
    
    /* More complex expressions to use results */
    int sum = out1 + out2 + out3;
    
    /* Nested pointer access - forces operand address reloads */
    int *ptr1 = &global_array_1[0];
    int *ptr2 = &global_array_2[0];
    int *ptr3 = &global_array_3[0];
    
    /* Chain of dependent computations */
    sum += *(ptr1 + out1);
    sum += *(ptr2 + out2);
    sum += *(ptr3 + out3);
    
    /* Address of address computation */
    sum += *(&global_array_1[0] + (out1 & 0xFF));
    
    return sum;
}

/* Test 3: Structure member accesses with offset computations */
static __attribute__((noinline))
int test_struct_addressing(int idx) {
    /* Many local structs to consume registers */
    struct BigStruct s1, s2, s3, s4, s5, s6, s7, s8;
    
    /* Initialize with volatile to force stores */
    s1.volatile_member = idx;
    s2.volatile_member = idx + 1;
    s3.volatile_member = idx + 2;
    s4.volatile_member = idx + 3;
    s5.volatile_member = idx + 4;
    s6.volatile_member = idx + 5;
    s7.volatile_member = idx + 6;
    s8.volatile_member = idx + 7;
    
    /* Chain structures */
    s1.next = &s2;
    s2.next = &s3;
    s3.next = &s4;
    s4.next = &s5;
    s5.next = &s6;
    s6.next = &s7;
    s7.next = &s8;
    s8.next = &s1;
    
    int result = 0;
    struct BigStruct *current = &s1;
    
    /* Loop with complex addressing - each iteration needs reloads */
    for (int i = 0; i < 8; i++) {
        /* Multiple structure member accesses in one expression */
        result += current->a + current->b + current->c + current->d;
        result += current->arr[i & 0x7];
        result += current->volatile_member;
        
        /* Address computation for next pointer */
        current = current->next;
        
        /* Additional computation using the address */
        if (current) {
            result += current->arr[(result + i) & 0x7];
        }
    }
    
    /* Array of pointers to force address reloads */
    struct BigStruct *ptr_array[8] = {&s1, &s2, &s3, &s4, &s5, &s6, &s7, &s8};
    
    /* Access through pointer array with index computation */
    for (int i = 0; i < 8; i++) {
        int idx2 = (result + i) & 0x7;
        result += ptr_array[idx2]->arr[i];
    }
    
    return result;
}

/* Test 4: Mixed addressing modes and pointer arithmetic */
static __attribute__((noinline))
int test_mixed_addressing(int base) {
    /* Declare many local arrays to consume stack space and registers */
    int arr1[16], arr2[16], arr3[16], arr4[16];
    int arr5[16], arr6[16], arr7[16], arr8[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        arr1[i] = base + i;
        arr2[i] = base + i * 2;
        arr3[i] = base + i * 3;
        arr4[i] = base + i * 4;
        arr5[i] = base + i * 5;
        arr6[i] = base + i * 6;
        arr7[i] = base + i * 7;
        arr8[i] = base + i * 8;
    }
    
    int sum = 0;
    
    /* Complex expression with multiple array accesses */
    /* Each index computation is different and complex */
    #pragma GCC unroll 4
    for (int i = 0; i < 8; i++) {
        /* Different addressing modes in one expression */
        sum += arr1[(i + base) & 0xF];
        sum += arr2[(i * 2 + base) & 0xF];
        sum += arr3[(i * 3 + base) & 0xF];
        sum += arr4[(i * 4 + base) & 0xF];
        sum += arr5[(i * 5 + base) & 0xF];
        sum += arr6[(i * 6 + base) & 0xF];
        sum += arr7[(i * 7 + base) & 0xF];
        sum += arr8[(i * 8 + base) & 0xF];
        
        /* Pointer arithmetic with multiple bases */
        int *p1 = &arr1[0] + i;
        int *p2 = &arr2[0] + (i * 2);
        int *p3 = &arr3[0] + (i * 3);
        
        sum += *p1 + *p2 + *p3;
        
        /* Address of a pointer computation */
        sum += *(*(&p1 + (i & 1)) + (i & 0xF));
    }
    
    /* Nested loop with address computations */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            /* 2D addressing simulated with 1D arrays */
            int idx = (i * 4 + j) & 0xF;
            sum += arr1[idx] + arr2[idx] + arr3[idx] + arr4[idx];
            
            /* More complex: address depends on previous computation */
            int *ptr = &arr5[0] + ((sum + idx) & 0xF);
            sum += *ptr;
        }
    }
    
    return sum;
}

/* Test 5: Extreme register pressure with all operand types */
static __attribute__((noinline))
int test_extreme_pressure(int seed) {
    /* Maximum number of local variables */
    int v01 = seed + 1, v02 = seed + 2, v03 = seed + 3, v04 = seed + 4;
    int v05 = seed + 5, v06 = seed + 6, v07 = seed + 7, v08 = seed + 8;
    int v09 = seed + 9, v10 = seed + 10, v11 = seed + 11, v12 = seed + 12;
    int v13 = seed + 13, v14 = seed + 14, v15 = seed + 15, v16 = seed + 16;
    volatile int v17 = seed + 17, v18 = seed + 18;
    
    /* Complex expression using all variables - forces many reloads */
    int result = 
        v01 * v02 + v03 * v04 + v05 * v06 + v07 * v08 +
        v09 * v10 + v11 * v12 + v13 * v14 + v15 * v16 +
        v17 * v18;
    
    /* Use all variables in address computations */
    result += global_array_1[(v01 + v02) & 0xFF];
    result += global_array_2[(v03 + v04) & 0xFF];
    result += global_array_3[(v05 + v06) & 0xFF];
    result += global_array_1[(v07 + v08) & 0xFF];
    result += global_array_2[(v09 + v10) & 0xFF];
    result += global_array_3[(v11 + v12) & 0xFF];
    result += global_array_1[(v13 + v14) & 0xFF];
    result += global_array_2[(v15 + v16) & 0xFF];
    
    /* Pointer chain */
    int *p1 = &v01;
    int *p2 = &v02;
    int *p3 = &v03;
    int *p4 = &v04;
    int *p5 = &v05;
    int *p6 = &v06;
    int *p7 = &v07;
    int *p8 = &v08;
    
    /* Dereference all pointers */
    result += *p1 + *p2 + *p3 + *p4 + *p5 + *p6 + *p7 + *p8;
    
    /* Address of address */
    result += **(&p1 + 0);
    
    /* Inline asm to force specific reload patterns */
    asm volatile (
        "movl %[a], %%eax\n\t"
        "addl %[b], %%eax\n\t"
        "movl %%eax, %[r]\n\t"
        : [r] "=m" (global_array_1[result & 0xFF])
        : [a] "rm" (v01),
          [b] "rm" (v02)
        : "eax", "cc"
    );
    
    return result;
}

/* Main driver that calls all tests */
int main(void) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array_1[i] = i;
        global_array_2[i] = i * 2;
        global_array_3[i] = i * 3;
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
        global_structs[i].next = &global_structs[(i + 1) & 0xF];
        global_structs[i].volatile_member = i;
    }
    
    int total = 0;
    
    /* Call all test functions with different arguments */
    total += test_complex_addressing(1, 2, 3, 4, 5, 6, 7, 8);
    total += test_asm_reloads(100, 200, 300);
    total += test_struct_addressing(50);
    total += test_mixed_addressing(1000);
    total += test_extreme_pressure(42);
    
    /* Call multiple times with different args */
    total += test_complex_addressing(8, 7, 6, 5, 4, 3, 2, 1);
    total += test_asm_reloads(400, 500, 600);
    total += test_struct_addressing(100);
    total += test_mixed_addressing(2000);
    total += test_extreme_pressure(123);
    
    /* Use result to prevent dead code elimination */
    volatile int final_result = total;
    
    return final_result > 0 ? 0 : 1;
}

#pragma GCC pop_options
