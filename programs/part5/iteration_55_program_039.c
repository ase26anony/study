/* reload_coverage.c - Program to trigger various reload types in GCC reload1.cc */

#include <stdint.h>
#include <stdlib.h>

/* Force frame pointer usage and prevent optimizations */
#pragma GCC optimize ("O0")
#pragma GCC push_options

/* Structure with many members to force complex addressing */
struct BigStruct {
    int a, b, c, d, e, f, g, h;
    int arr[8][8];
    struct BigStruct *next;
    volatile int volatile_member;
};

/* Global arrays to provide base addresses */
static int global_array[256][256];
static struct BigStruct global_structs[32];

/* Test 1: Complex array addressing with multiple index computations */
static __attribute__((noinline)) 
int test_complex_addressing(int x, int y, int z) {
    /* Many local variables to consume registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    volatile int v1, v2, v3;
    
    /* Complex array indexing - forces address reloads */
    a1 = global_array[x][y] + global_array[y][z];
    a2 = global_array[z][x] * global_array[x+y][z-y];
    
    /* Multi-level array access with computed indices */
    a3 = global_array[global_array[x][y] & 255][global_array[y][z] & 255];
    
    /* Volatile accesses force memory operations */
    v1 = a1;
    v2 = a2;
    v3 = a3;
    
    /* More complex expressions with many temporaries */
    a4 = (a1 * a2) + (a3 << 2) - (x * y * z);
    a5 = (a2 / (a1 + 1)) | (a3 & 0xFF);
    
    /* Nested array accesses in expressions */
    a6 = global_array[a4 & 255][a5 & 255] + 
         global_array[a5 & 255][a4 & 255];
    
    /* Manual unrolling to increase register pressure */
    a7 = a1 + a2; a8 = a3 + a4; a9 = a5 + a6; a10 = a7 + a8;
    b1 = a9 * a10; b2 = a10 / (a9 + 1); b3 = b1 ^ b2;
    b4 = b3 << 3; b5 = b4 >> 1; b6 = b5 + b3;
    b7 = b6 * a1; b8 = b7 - a2; b9 = b8 | a3;
    b10 = b9 & 0xFFFF;
    
    return a1 + a2 + a3 + a4 + a5 + a6 + b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10;
}

/* Test 2: Structure member accesses with pointer chasing */
static __attribute__((noinline))
int test_structure_accesses(int idx) {
    struct BigStruct local_struct;
    struct BigStruct *ptr1, *ptr2, *ptr3;
    int sum = 0;
    volatile int vol;
    
    /* Initialize pointers with complex expressions */
    ptr1 = &global_structs[idx];
    ptr2 = &global_structs[(idx * 7) % 32];
    ptr3 = &local_struct;
    
    /* Complex structure member accesses */
    sum += ptr1->arr[ptr1->a & 7][ptr1->b & 7];
    sum += ptr2->arr[ptr2->c & 7][ptr2->d & 7];
    
    /* Pointer arithmetic with structure offsets */
    sum += ((int*)ptr1)[ptr1->e & 3] + ((int*)ptr2)[ptr2->f & 3];
    
    /* Volatile member access forces reload */
    vol = ptr1->volatile_member;
    sum += vol;
    
    /* Chain pointer accesses */
    ptr3->next = ptr1;
    sum += ptr3->next->arr[0][0];
    
    /* More complex addressing */
    sum += ptr1->arr[ptr1->g & 7][ptr1->h & 7] * 
           ptr2->arr[ptr2->a & 7][ptr2->b & 7];
    
    return sum;
}

/* Test 3: Inline assembly with multiple outputs and clobbers */
static __attribute__((noinline))
int test_inline_asm(int a, int b, int c, int d) {
    int out1, out2, out3, out4;
    volatile int mem1, mem2;
    
    /* Inline asm with memory output - forces output address reloads */
    asm volatile (
        "movl %[input1], %[output1]\n\t"
        "addl %[input2], %[output1]\n\t"
        "movl %[output1], %[mem1]\n\t"
        "imull %[input3], %[output1]\n\t"
        : [output1] "=r" (out1), [mem1] "=m" (mem1)
        : [input1] "r" (a), [input2] "r" (b), [input3] "r" (c)
        : "cc"
    );
    
    /* Another asm with different constraints */
    asm volatile (
        "leal (%[in1], %[in2], 4), %[out1]\n\t"
        "movl %[out1], %[mem2]\n\t"
        : [out1] "=r" (out2), [mem2] "=m" (mem2)
        : [in1] "r" (a), [in2] "r" (d)
        : "cc"
    );
    
    /* Asm with multiple outputs */
    asm volatile (
        "movl %1, %0\n\t"
        "addl %2, %0\n\t"
        "movl %0, %3\n\t"
        : "=r" (out3), "=r" (out4)
        : "r" (b), "m" (mem1)
        : "cc"
    );
    
    return out1 + out2 + out3 + out4 + mem1 + mem2;
}

/* Test 4: Mixed addressing modes and operand types */
static __attribute__((noinline))
int test_mixed_addressing(int base) {
    int *ptr_array[16];
    int values[16];
    int sum = 0;
    int i, j;
    
    /* Initialize pointers with complex addresses */
    for (i = 0; i < 16; i++) {
        ptr_array[i] = &values[(base + i * 3) % 16];
        values[i] = (base + i * 7) & 0xFF;
    }
    
    /* Complex pointer dereferencing with index computations */
    for (i = 0; i < 8; i++) {
        /* Force RELOAD_FOR_OPERAND_ADDRESS */
        sum += *(ptr_array[i] + (ptr_array[i+1] - ptr_array[i]));
        
        /* Force RELOAD_FOR_INPUT_ADDRESS */
        sum += ptr_array[i][values[i] & 7];
        
        /* Nested addressing */
        sum += *(int*)((char*)ptr_array[i] + values[(i+1) & 0xF] * sizeof(int));
    }
    
    /* Manual unrolling for more pressure */
    sum += ptr_array[0][0] + ptr_array[1][1] + ptr_array[2][2] + ptr_array[3][3];
    sum += ptr_array[4][4] + ptr_array[5][5] + ptr_array[6][6] + ptr_array[7][7];
    
    return sum;
}

/* Test 5: Extreme register pressure with many live ranges */
static __attribute__((noinline))
int test_extreme_pressure(int seed) {
    /* Declare many variables to consume all registers */
    int v01, v02, v03, v04, v05, v06, v07, v08, v09, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    volatile int mem[8];
    
    /* Initialize with complex expressions */
    v01 = seed * 1;  v02 = seed * 2;  v03 = seed * 3;  v04 = seed * 4;
    v05 = seed * 5;  v06 = seed * 6;  v07 = seed * 7;  v08 = seed * 8;
    v09 = seed * 9;  v10 = seed * 10; v11 = seed * 11; v12 = seed * 12;
    v13 = seed * 13; v14 = seed * 14; v15 = seed * 15; v16 = seed * 16;
    v17 = seed * 17; v18 = seed * 18; v19 = seed * 19; v20 = seed * 20;
    v21 = seed * 21; v22 = seed * 22; v23 = seed * 23; v24 = seed * 24;
    v25 = seed * 25; v26 = seed * 26; v27 = seed * 27; v28 = seed * 28;
    v29 = seed * 29; v30 = seed * 30;
    
    /* Create complex data flow graph */
    v01 = v01 + v02; v03 = v03 + v04; v05 = v05 + v06;
    v07 = v07 + v08; v09 = v09 + v10; v11 = v11 + v12;
    v13 = v13 + v14; v15 = v15 + v16; v17 = v17 + v18;
    v19 = v19 + v20; v21 = v21 + v22; v23 = v23 + v24;
    v25 = v25 + v26; v27 = v27 + v28; v29 = v29 + v30;
    
    /* More operations keeping many values live */
    v02 = v01 * v03; v04 = v05 * v07; v06 = v09 * v11;
    v08 = v13 * v15; v10 = v17 * v19; v12 = v21 * v23;
    v14 = v25 * v27; v16 = v29 * v01;
    
    /* Volatile stores force spills */
    mem[0] = v01; mem[1] = v02; mem[2] = v03; mem[3] = v04;
    mem[4] = v05; mem[5] = v06; mem[6] = v07; mem[7] = v08;
    
    /* Use all variables in final computation */
    return v01 + v02 + v03 + v04 + v05 + v06 + v07 + v08 +
           v09 + v10 + v11 + v12 + v13 + v14 + v15 + v16 +
           v17 + v18 + v19 + v20 + v21 + v22 + v23 + v24 +
           v25 + v26 + v27 + v28 + v29 + v30;
}

/* Main driver that calls all tests */
int main(int argc, char **argv) {
    int result = 0;
    int i;
    
    /* Initialize global data */
    for (i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_array[i][j] = (i * 31 + j * 17) & 0xFF;
        }
    }
    
    for (i = 0; i < 32; i++) {
        global_structs[i].a = i * 1;
        global_structs[i].b = i * 2;
        global_structs[i].c = i * 3;
        global_structs[i].d = i * 4;
        global_structs[i].e = i * 5;
        global_structs[i].f = i * 6;
        global_structs[i].g = i * 7;
        global_structs[i].h = i * 8;
        global_structs[i].volatile_member = i * 9;
        global_structs[i].next = &global_structs[(i + 1) % 32];
    }
    
    /* Run all tests multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        result += test_complex_addressing(i, i+1, i+2);
        result += test_structure_accesses(i);
        result += test_inline_asm(i, i+1, i+2, i+3);
        result += test_mixed_addressing(i);
        result += test_extreme_pressure(i);
    }
    
    /* Use result to prevent dead code elimination */
    if (argc > 1) {
        return result % 256;
    }
    return 0;
}

#pragma GCC pop_options
