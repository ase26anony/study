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
struct Nested {
    int data[8];
    struct Nested *next;
    short offsets[4];
};

struct Container {
    struct Nested levels[3][2];
    int matrix[4][4];
    volatile long counter;
};

/* Test function 1: Complex array addressing with multiple index computations */
__attribute__((noinline))
static int test_complex_addressing(struct Container *cont, int idx1, int idx2, int idx3) {
    /* Many local variables to consume registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    int c1, c2, c3, c4, c5, c6, c7, c8, c9, c10;
    volatile int v1, v2, v3, v4;
    
    /* Force register pressure with complex expressions */
    a1 = idx1 * 3 + 1;
    a2 = idx2 * 5 + 2;
    a3 = idx3 * 7 + 3;
    a4 = a1 + a2 * a3;
    a5 = a2 + a3 * a1;
    a6 = a3 + a1 * a2;
    a7 = a4 * a5 - a6;
    a8 = a5 * a6 - a7;
    a9 = a6 * a7 - a8;
    a10 = a7 * a8 - a9;
    
    /* Multi-level array indexing - triggers RELOAD_FOR_INPUT_ADDRESS */
    b1 = cont->levels[0][1].data[a1 % 8];
    b2 = cont->levels[1][0].data[a2 % 8];
    b3 = cont->levels[2][1].data[a3 % 8];
    
    /* Nested addressing with multiple computations */
    b4 = cont->matrix[a1 % 4][a2 % 4] + cont->matrix[a2 % 4][a3 % 4];
    b5 = cont->matrix[a3 % 4][a1 % 4] * cont->matrix[a2 % 4][a2 % 4];
    
    /* Pointer arithmetic with multiple bases - triggers RELOAD_FOR_OPERAND_ADDRESS */
    struct Nested *ptr1 = &cont->levels[0][0];
    struct Nested *ptr2 = &cont->levels[1][1];
    struct Nested *ptr3 = &cont->levels[2][0];
    
    /* Complex address computations */
    b6 = ptr1->data[(a1 + a2) % 8] + ptr2->data[(a2 + a3) % 8];
    b7 = ptr3->data[(a3 + a1) % 8] * ptr1->offsets[a4 % 4];
    
    /* Volatile accesses force spills */
    v1 = vol_global;
    v2 = vol_array[a1 % 256];
    v3 = vol_array[a2 % 256];
    v4 = vol_array[a3 % 256];
    
    /* More computations to use all variables */
    c1 = a1 + b1 + v1;
    c2 = a2 + b2 + v2;
    c3 = a3 + b3 + v3;
    c4 = a4 + b4 + v4;
    c5 = a5 + b5 + v1;
    c6 = a6 + b6 + v2;
    c7 = a7 + b7 + v3;
    c8 = a8 + c1 + v4;
    c9 = a9 + c2 + v1;
    c10 = a10 + c3 + v2;
    
    /* Final complex expression */
    return c1 * c2 - c3 * c4 + c5 * c6 - c7 * c8 + c9 * c10;
}

/* Test function 2: Inline assembly with multiple outputs and clobbers */
__attribute__((noinline))
static int test_asm_reloads(int x, int y, int z) {
    int out1, out2, out3, out4;
    volatile int mem1, mem2, mem3;
    
    /* Inline asm with multiple outputs - triggers RELOAD_FOR_OUTPUT_ADDRESS */
    asm volatile (
        "movl %[x], %%eax\n\t"
        "movl %[y], %%ebx\n\t"
        "movl %[z], %%ecx\n\t"
        "addl %%ebx, %%eax\n\t"
        "imull %%ecx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "leal (%%ebx,%%ecx,2), %%edx\n\t"
        "movl %%edx, %[out2]\n\t"
        "movl %[mem1], %%esi\n\t"
        "addl %%esi, %%eax\n\t"
        "movl %%eax, %[out3]\n\t"
        : [out1] "=m" (out1),
          [out2] "=m" (out2),
          [out3] "=m" (out3)
        : [x] "rm" (x),
          [y] "rm" (y),
          [z] "rm" (z),
          [mem1] "m" (mem1)
        : "eax", "ebx", "ecx", "edx", "esi", "memory"
    );
    
    /* More complex asm with memory output - triggers RELOAD_FOR_OUTADDR_ADDRESS */
    int *ptr = &out4;
    asm volatile (
        "movl %[val], %%eax\n\t"
        "movl %%eax, %[ptr]\n\t"
        : [ptr] "=m" (*ptr)
        : [val] "ri" (x + y + z)
        : "eax", "memory"
    );
    
    /* Use all outputs */
    mem2 = out1 + out2;
    mem3 = out3 * out4;
    
    return mem2 + mem3;
}

/* Test function 3: Loop unrolling with address computations */
__attribute__((noinline))
static int test_loop_unrolling(int *arr, int n) {
    int sum = 0;
    int i;
    
    /* Manually unrolled loop - each iteration uses many registers */
    #pragma GCC unroll 4
    for (i = 0; i < n; i++) {
        /* Complex addressing within loop - triggers RELOAD_FOR_INPADDR_ADDRESS */
        int idx1 = i * 3;
        int idx2 = i * 5;
        int idx3 = i * 7;
        
        /* Multiple array accesses with different index computations */
        int val1 = arr[idx1 % n];
        int val2 = arr[idx2 % n];
        int val3 = arr[idx3 % n];
        int val4 = arr[(idx1 + idx2) % n];
        int val5 = arr[(idx2 + idx3) % n];
        int val6 = arr[(idx3 + idx1) % n];
        
        /* Address of element computation - triggers RELOAD_FOR_OTHER_ADDRESS */
        int *ptr1 = &arr[(val1 + i) % n];
        int *ptr2 = &arr[(val2 + i) % n];
        int *ptr3 = &arr[(val3 + i) % n];
        
        /* Use computed addresses */
        int comp1 = *ptr1 + val4;
        int comp2 = *ptr2 + val5;
        int comp3 = *ptr3 + val6;
        
        /* Complex expression consuming many temporaries */
        sum += val1 * comp1 - val2 * comp2 + val3 * comp3;
        
        /* Volatile write to force spill */
        vol_array[i % 256] = sum;
    }
    
    return sum;
}

/* Test function 4: Structure member accesses with offsets */
__attribute__((noinline))
static int test_struct_offsets(struct Container *cont) {
    /* Access structure members with complex offset computations */
    int total = 0;
    
    /* Multiple structure accesses - each needs address computation */
    total += cont->levels[0][0].data[0];
    total += cont->levels[0][1].data[1];
    total += cont->levels[1][0].data[2];
    total += cont->levels[1][1].data[3];
    total += cont->levels[2][0].data[4];
    total += cont->levels[2][1].data[5];
    
    /* Matrix accesses with computed indices */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            /* Complex index computation - triggers various reload types */
            int idx = (i * 3 + j * 5) % 4;
            total += cont->matrix[i][j] * cont->matrix[j][idx];
            
            /* Address computation for next element */
            if (j < 3) {
                volatile int *addr = &cont->matrix[i][j+1];
                total += *addr;
            }
        }
    }
    
    /* Pointer chasing with offset computations */
    struct Nested *current = &cont->levels[0][0];
    for (int i = 0; i < 3; i++) {
        /* Access with offset - triggers RELOAD_FOR_OPADDR_ADDR */
        total += current->data[i] + current->offsets[i % 4];
        
        /* Compute next pointer with offset */
        if (i < 2) {
            current = &cont->levels[i+1][0];
        }
    }
    
    return total;
}

/* Test function 5: Mixed operations with __builtin_expect */
__attribute__((noinline))
static int test_builtin_expect(int a, int b, int c, int d, int e, int f) {
    /* Use __builtin_expect to create data dependencies */
    int x = __builtin_expect(a > 0, 1) ? b : c;
    int y = __builtin_expect(b > 0, 0) ? c : d;
    int z = __builtin_expect(c > 0, 1) ? d : e;
    int w = __builtin_expect(d > 0, 0) ? e : f;
    
    /* Complex expression with many intermediates */
    int t1 = x * y + z * w;
    int t2 = y * z + w * x;
    int t3 = z * w + x * y;
    int t4 = w * x + y * z;
    
    /* Address computations intermixed */
    volatile int *ptr1 = &vol_array[x % 256];
    volatile int *ptr2 = &vol_array[y % 256];
    volatile int *ptr3 = &vol_array[z % 256];
    volatile int *ptr4 = &vol_array[w % 256];
    
    /* Use the pointers */
    int v1 = *ptr1 + t1;
    int v2 = *ptr2 + t2;
    int v3 = *ptr3 + t3;
    int v4 = *ptr4 + t4;
    
    /* Final computation */
    return v1 * v2 - v3 * v4 + t1 * t2 - t3 * t4;
}

/* Main driver function */
int main(void) {
    int result = 0;
    
    /* Initialize data */
    struct Container container;
    int array[100];
    
    for (int i = 0; i < 100; i++) {
        array[i] = i * 3;
    }
    
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 8; k++) {
                container.levels[i][j].data[k] = i * 100 + j * 10 + k;
            }
            for (int k = 0; k < 4; k++) {
                container.levels[i][j].offsets[k] = (i + j + k) * 2;
            }
        }
    }
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            container.matrix[i][j] = i * 4 + j;
        }
    }
    
    container.counter = 0;
    
    /* Call all test functions to trigger different reload patterns */
    result += test_complex_addressing(&container, 1, 2, 3);
    result += test_asm_reloads(10, 20, 30);
    result += test_loop_unrolling(array, 100);
    result += test_struct_offsets(&container);
    result += test_builtin_expect(1, 2, 3, 4, 5, 6);
    
    /* Use result to prevent dead code elimination */
    vol_global = result;
    
    return result > 0 ? 0 : 1;
}
