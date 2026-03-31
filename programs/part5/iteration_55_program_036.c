/* reload_coverage.c - Test program to trigger various reload types in GCC */
#include <stdint.h>
#include <stdlib.h>

/* Force no optimization on specific functions */
#define NOOPT __attribute__((optimize("O0")))
#define NOINLINE __attribute__((noinline))

/* Volatile variables to force memory accesses */
static volatile int vol_global = 42;

/* Complex structure for address computations */
struct Nested {
    int data[8];
    struct Nested *next;
    short offsets[4];
};

struct Container {
    struct Nested arrays[3][2];
    int matrix[4][4];
    long counters[6];
};

/* Test 1: Complex array addressing with multiple index computations */
NOINLINE NOOPT
static int test_complex_addressing(struct Container *cont, int idx1, int idx2, int idx3) {
    /* Many local variables to consume registers */
    int a = idx1 * 2;
    int b = idx2 + vol_global;
    int c = idx3 & 0xFF;
    int d = a + b;
    int e = b - c;
    int f = c * d;
    int g = d / (e + 1);
    int h = e ^ f;
    int i = f | g;
    int j = g & h;
    int k = h << 2;
    int l = i >> 1;
    int m = j + k;
    int n = l - m;
    int o = m * n;
    int p = n / (o + 1);
    int q = o ^ p;
    
    /* Complex array addressing - will need address reloads */
    int result = 0;
    
    /* RELOAD_FOR_INPUT_ADDRESS: address computation for cont->arrays */
    result += cont->arrays[a % 3][b % 2].data[c % 8];
    
    /* RELOAD_FOR_INPADDR_ADDRESS: nested addressing */
    result += cont->arrays[(d % 3)][(e % 2)].offsets[f % 4];
    
    /* More complex addressing with multiple computations */
    result += cont->matrix[g % 4][h % 4] * cont->matrix[i % 4][j % 4];
    
    /* Pointer chasing with address computations */
    struct Nested *ptr = &cont->arrays[k % 3][l % 2];
    for (int x = 0; x < 2; x++) {
        /* RELOAD_FOR_OPERAND_ADDRESS: ptr itself needs reloading */
        result += ptr->data[m % 8];
        ptr = ptr->next;
        if (!ptr) break;
    }
    
    /* Volatile writes to force spills */
    vol_global = result;
    
    return result + q;
}

/* Test 2: Inline assembly with multiple outputs and complex constraints */
NOINLINE NOOPT
static int test_asm_reloads(int *mem1, int *mem2, int *mem3) {
    int out1, out2, out3, out4;
    int addr1, addr2, addr3;
    
    /* Compute complex addresses */
    addr1 = (int)(mem1 + vol_global);
    addr2 = (int)(mem2 + (vol_global >> 2));
    addr3 = (int)(mem3 + (vol_global << 1));
    
    /* Inline asm with multiple outputs and memory constraints */
    /* This should trigger RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    asm volatile (
        "movl %[addr1], %%eax\n\t"
        "movl %[addr2], %%ebx\n\t"
        "movl %[addr3], %%ecx\n\t"
        "movl (%%eax), %%edx\n\t"
        "addl (%%ebx), %%edx\n\t"
        "subl (%%ecx), %%edx\n\t"
        "movl %%edx, %[out1]\n\t"
        "imull %%edx, %%edx\n\t"
        "movl %%edx, %[out2]\n\t"
        "leal (%%eax, %%ebx), %%esi\n\t"
        "movl %%esi, %[out3]\n\t"
        "leal (%%ecx, %%edx, 2), %%edi\n\t"
        "movl %%edi, %[out4]"
        : [out1] "=r" (out1), [out2] "=r" (out2), 
          [out3] "=r" (out3), [out4] "=r" (out4)
        : [addr1] "mr" (addr1), [addr2] "mr" (addr2), [addr3] "mr" (addr3)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* Use all outputs in complex expression */
    return out1 + out2 * 3 - out3 / 2 + out4;
}

/* Test 3: Mixed operand types with pointer arithmetic */
NOINLINE NOOPT
static int test_mixed_operands(struct Container *cont, int *base_ptr) {
    /* Many local pointers to increase register pressure */
    int *p1 = base_ptr + 1;
    int *p2 = base_ptr + 2;
    int *p3 = base_ptr + 3;
    int *p4 = base_ptr + 4;
    int *p5 = base_ptr + 5;
    int *p6 = base_ptr + 6;
    int *p7 = base_ptr + 7;
    int *p8 = base_ptr + 8;
    
    /* Complex pointer expressions */
    int **pp1 = &p1;
    int **pp2 = &p2;
    int **pp3 = &p3;
    
    /* RELOAD_FOR_OPADDR_ADDR: address of pointer needs reload */
    int val1 = **pp1 + **pp2;
    
    /* More complex indirection */
    int val2 = *(*(pp3) + 2) + *(p4 + *p5);
    
    /* Structure member access with computed offset */
    int offset = vol_global % 6;
    int val3 = cont->counters[offset] + cont->counters[offset + 1];
    
    /* Inline asm with memory output */
    int output;
    asm volatile (
        "movl %[ptr], %%eax\n\t"
        "movl %[val], (%%eax)\n\t"
        "movl (%%eax), %[out]"
        : [out] "=r" (output)
        : [ptr] "r" (p6), [val] "r" (val1 + val2)
        : "eax", "memory"
    );
    
    /* Use all values */
    return val1 + val2 * 2 - val3 + output + *p7 - *p8;
}

/* Test 4: Loop with unrolling causing register pressure */
NOINLINE
static int test_loop_unroll(int *array, int size) {
    int sum = 0;
    
    /* Manual unrolling to increase register pressure */
    #pragma GCC unroll 4
    for (int i = 0; i < size; i += 4) {
        /* Many temporaries in loop body */
        int t1 = array[i];
        int t2 = array[i + 1];
        int t3 = array[i + 2];
        int t4 = array[i + 3];
        
        /* Complex expressions using all temporaries */
        int u1 = t1 * t2 + vol_global;
        int u2 = t3 - t4 * 2;
        int u3 = (t1 + t3) / (t2 + 1);
        int u4 = t4 ^ t1 | t2;
        
        /* Nested array-like computation */
        sum += u1 + u2 * u3 - u4;
        
        /* Address computation inside loop */
        int *addr = &array[i] + (u1 % 4);
        sum += *addr;
    }
    
    return sum;
}

/* Test 5: Function with RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS patterns */
NOINLINE NOOPT
static int test_other_reloads(void) {
    /* Large stack array to force spills */
    int big_array[32];
    int *pointers[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 32; i++) {
        big_array[i] = i * 3 + vol_global;
    }
    
    for (int i = 0; i < 16; i++) {
        pointers[i] = &big_array[i * 2];
    }
    
    /* Complex expression using many array elements */
    int result = 0;
    
    /* Multiple dependent computations */
    result = big_array[0] * big_array[1];
    result += big_array[2] - big_array[3];
    result *= big_array[4] / (big_array[5] + 1);
    result ^= big_array[6] | big_array[7];
    
    /* Pointer arithmetic chain */
    int *chain = pointers[0];
    for (int i = 1; i < 8; i++) {
        chain = chain + (int)(pointers[i] - pointers[i-1]);
        result += *chain;
    }
    
    /* Inline asm with multiple alternative constraints */
    int final;
    asm volatile (
        "movl %[input], %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %[output]"
        : [output] "=r,m" (final)  /* Alternative constraints */
        : [input] "r,m" (result)   /* Can be register or memory */
        : "eax"
    );
    
    return final;
}

/* Main driver function */
int main(void) {
    struct Container container;
    int data_array[64];
    int *heap_ptr = (int*)malloc(256 * sizeof(int));
    
    /* Initialize data */
    for (int i = 0; i < 64; i++) {
        data_array[i] = i;
    }
    
    if (heap_ptr) {
        for (int i = 0; i < 256; i++) {
            heap_ptr[i] = i * 2;
        }
    }
    
    /* Initialize container */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 8; k++) {
                container.arrays[i][j].data[k] = i * 100 + j * 10 + k;
            }
            for (int k = 0; k < 4; k++) {
                container.arrays[i][j].offsets[k] = k * 2;
            }
            container.arrays[i][j].next = (j < 1) ? &container.arrays[i][j+1] : NULL;
        }
    }
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            container.matrix[i][j] = i * 4 + j;
        }
    }
    
    for (int i = 0; i < 6; i++) {
        container.counters[i] = i * 10;
    }
    
    /* Call all test functions to trigger different reload patterns */
    int total = 0;
    
    total += test_complex_addressing(&container, 1, 2, 3);
    total += test_complex_addressing(&container, 4, 5, 6);
    
    total += test_asm_reloads(data_array, data_array + 16, data_array + 32);
    
    total += test_mixed_operands(&container, data_array);
    
    total += test_loop_unroll(data_array, 64);
    
    total += test_other_reloads();
    
    if (heap_ptr) {
        total += test_loop_unroll(heap_ptr, 128);
        free(heap_ptr);
    }
    
    /* Use result to prevent dead code elimination */
    vol_global = total % 1000;
    
    return total > 0 ? 0 : 1;
}
