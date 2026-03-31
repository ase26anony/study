/* reload_coverage.c - Test program to trigger various reload types in GCC reload1.cc */

#include <stdint.h>
#include <stdlib.h>

/* Force specific architecture for register scarcity */
#ifdef __x86_64__
#error "Compile with -m32 flag for 32-bit target"
#endif

/* Volatile variables to force memory accesses */
static volatile int g_volatile_counter = 0;
static volatile int* g_volatile_ptr = NULL;

/* Complex data structures to create addressing modes */
struct NestedStruct {
    int data[4];
    struct NestedStruct* next;
    short offsets[8];
};

struct ComplexData {
    int matrix[3][3][3];
    struct NestedStruct nodes[5];
    long long big_array[10];
    volatile int sync;
};

/* Test function 1: Complex array addressing with multiple index calculations */
__attribute__((noinline))
static int test_complex_addressing(struct ComplexData* cd, int idx1, int idx2, int idx3) {
    /* Many local variables to consume registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    volatile int v1, v2, v3;
    int* ptr1, *ptr2, *ptr3;
    
    /* Force register pressure with many live variables */
    a1 = idx1 * 2;
    a2 = idx2 * 3;
    a3 = idx3 * 4;
    a4 = a1 + a2;
    a5 = a2 + a3;
    a6 = a3 + a1;
    a7 = a4 * a5;
    a8 = a5 * a6;
    a9 = a6 * a4;
    a10 = a7 + a8 + a9;
    
    /* Complex array addressing - triggers RELOAD_FOR_INPUT_ADDRESS */
    /* Multi-level indexing with computations */
    b1 = cd->matrix[idx1][idx2][idx3];
    b2 = cd->matrix[idx3][idx1][idx2];
    b3 = cd->matrix[idx2][idx3][idx1];
    
    /* More complex addressing with scaled indices */
    b4 = cd->matrix[a1 % 3][a2 % 3][a3 % 3];
    b5 = cd->matrix[(a1 + a2) % 3][(a2 + a3) % 3][(a3 + a1) % 3];
    
    /* Structure member addressing with offset */
    b6 = cd->nodes[idx1 % 5].data[idx2 % 4];
    b7 = cd->nodes[idx2 % 5].data[idx3 % 4];
    
    /* Pointer arithmetic creating address reload needs */
    ptr1 = &cd->nodes[0].data[0];
    ptr2 = &cd->nodes[1].data[0];
    ptr3 = &cd->nodes[2].data[0];
    
    /* Volatile accesses force spills */
    v1 = g_volatile_counter;
    v2 = cd->sync;
    v3 = *g_volatile_ptr;
    
    /* Complex expression using all variables */
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
           b1 + b2 + b3 + b4 + b5 + b6 + b7 + v1 + v2 + v3 +
           *ptr1 + *ptr2 + *ptr3;
}

/* Test function 2: Inline assembly with multiple outputs and constraints */
__attribute__((noinline))
static int test_asm_reloads(int x, int y, int z) {
    int result1, result2, result3, result4;
    int addr1, addr2, addr3;
    volatile int mem1, mem2;
    
    /* Reserve memory for asm outputs */
    int output_mem[4] = {0};
    
    /* Complex inline asm with multiple outputs and memory constraints */
    /* This should trigger RELOAD_FOR_OUTPUT_ADDRESS and others */
    __asm__ volatile (
        /* Multiple output operands clobbering many registers */
        "movl %[x], %%eax\n\t"
        "movl %[y], %%ebx\n\t"
        "movl %[z], %%ecx\n\t"
        "addl %%ebx, %%eax\n\t"
        "imull %%ecx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        
        /* Memory output with complex address */
        "leal (%[x], %[y], 2), %%edx\n\t"
        "movl %%edx, %[out2]\n\t"
        
        /* Another computation */
        "movl %[x], %%esi\n\t"
        "subl %[y], %%esi\n\t"
        "movl %%esi, %[out3]\n\t"
        
        /* Memory operation with computed address */
        "movl %[mem_ptr], %%edi\n\t"
        "movl (%%edi), %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, (%%edi)\n\t"
        
        : [out1] "=m" (output_mem[0]),
          [out2] "=m" (output_mem[1]),
          [out3] "=m" (output_mem[2]),
          "+m" (mem1)
        : [x] "r" (x),
          [y] "r" (y),
          [z] "r" (z),
          [mem_ptr] "r" (&mem1)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* More complex asm with input/output addressing */
    int temp_addr = (int)&output_mem[3];
    __asm__ volatile (
        /* Operand address reload test */
        "movl (%[addr]), %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, (%[addr])\n\t"
        : "+m" (*(int*)temp_addr)
        : [addr] "r" (&temp_addr)
        : "eax", "memory"
    );
    
    result1 = output_mem[0];
    result2 = output_mem[1];
    result3 = output_mem[2];
    result4 = output_mem[3];
    
    /* Complex expression to keep variables live */
    addr1 = (int)&result1;
    addr2 = (int)&result2;
    addr3 = (int)&result3;
    
    mem2 = g_volatile_counter;
    
    return result1 + result2 + result3 + result4 + 
           (addr1 % 256) + (addr2 % 256) + (addr3 % 256) + mem2;
}

/* Test function 3: Pointer chasing and nested addressing */
__attribute__((noinline))
static int test_pointer_chasing(struct NestedStruct* start, int iterations) {
    struct NestedStruct* current = start;
    int sum = 0;
    volatile int* volatile_ptr = &g_volatile_counter;
    int temp_array[8];
    int i, j, k;
    
    /* Initialize temp array */
    for (i = 0; i < 8; i++) {
        temp_array[i] = i * iterations;
    }
    
    /* Manual loop unrolling for register pressure */
    /* This creates many similar patterns that need reloads */
    
    /* Unrolled iteration 1 */
    if (current) {
        /* Complex addressing with multiple base registers */
        sum += current->data[0];
        sum += current->data[1];
        sum += current->data[2];
        sum += current->data[3];
        
        /* Pointer arithmetic for next */
        current = current->next;
    }
    
    /* Unrolled iteration 2 */
    if (current) {
        /* Different addressing pattern */
        sum += current->offsets[0] + temp_array[0];
        sum += current->offsets[1] + temp_array[1];
        sum += current->offsets[2] + temp_array[2];
        sum += current->offsets[3] + temp_array[3];
        
        current = current->next;
    }
    
    /* Unrolled iteration 3 */
    if (current) {
        /* More complex expression */
        int idx1 = *volatile_ptr % 4;
        int idx2 = (*volatile_ptr + 1) % 4;
        
        sum += current->data[idx1] * temp_array[idx2];
        sum += current->data[idx2] * temp_array[idx1];
        
        /* Address computation that needs reloading */
        int* data_ptr = &current->data[0];
        for (j = 0; j < 2; j++) {
            sum += *(data_ptr + j) + temp_array[j + 4];
        }
        
        current = current->next;
    }
    
    /* Additional computations to use more registers */
    int t1 = sum * 2;
    int t2 = sum / 3;
    int t3 = sum % 7;
    int t4 = t1 + t2;
    int t5 = t2 + t3;
    int t6 = t3 + t1;
    
    /* Complex final expression */
    return sum + t1 + t2 + t3 + t4 + t5 + t6 + *volatile_ptr;
}

/* Test function 4: Mixed operand types and constraints */
__attribute__((noinline))
static int test_mixed_operands(int base, int offset1, int offset2) {
    /* Local arrays for address computations */
    int array1[10];
    int array2[10];
    int array3[10];
    
    /* Many scalar variables */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int w1, w2, w3, w4, w5, w6, w7, w8, w9, w10;
    volatile int sync1, sync2;
    
    /* Initialize arrays */
    for (int i = 0; i < 10; i++) {
        array1[i] = base + i;
        array2[i] = offset1 * i;
        array3[i] = offset2 + i * 2;
    }
    
    /* Complex expressions with array addressing */
    v1 = array1[base % 10];
    v2 = array2[offset1 % 10];
    v3 = array3[offset2 % 10];
    
    /* Nested array addressing */
    v4 = array1[array2[offset1 % 10] % 10];
    v5 = array2[array3[offset2 % 10] % 10];
    v6 = array3[array1[base % 10] % 10];
    
    /* More computations */
    v7 = v1 * v2 + v3;
    v8 = v2 * v3 + v1;
    v9 = v3 * v1 + v2;
    v10 = v7 + v8 + v9;
    
    /* Pointer-based addressing */
    int* p1 = &array1[0];
    int* p2 = &array2[0];
    int* p3 = &array3[0];
    
    /* Complex pointer arithmetic */
    w1 = *(p1 + (v1 % 10));
    w2 = *(p2 + (v2 % 10));
    w3 = *(p3 + (v3 % 10));
    w4 = *(p1 + (v4 % 10));
    w5 = *(p2 + (v5 % 10));
    w6 = *(p3 + (v6 % 10));
    
    /* Volatile accesses */
    sync1 = g_volatile_counter;
    sync2 = *g_volatile_ptr;
    
    /* Use __builtin_expect to create data dependencies */
    if (__builtin_expect(sync1 > 0, 0)) {
        w7 = w1 * 2;
    } else {
        w7 = w1 / 2;
    }
    
    if (__builtin_expect(sync2 > 0, 1)) {
        w8 = w2 + 100;
    } else {
        w8 = w2 - 100;
    }
    
    w9 = w3 ^ w4;
    w10 = w5 | w6;
    
    /* Final complex expression using all variables */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8 + w9 + w10 +
           sync1 + sync2;
}

/* Main driver function */
int main(int argc, char** argv) {
    struct ComplexData cd;
    struct NestedStruct nodes[10];
    int i, j, k;
    int total = 0;
    
    /* Initialize global volatile */
    g_volatile_counter = 1;
    g_volatile_ptr = &g_volatile_counter;
    
    /* Initialize complex data structure */
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            for (k = 0; k < 3; k++) {
                cd.matrix[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 4; j++) {
            cd.nodes[i].data[j] = i * 10 + j;
        }
        for (j = 0; j < 8; j++) {
            cd.nodes[i].offsets[j] = (short)(i * 5 + j);
        }
        cd.nodes[i].next = (i < 4) ? &cd.nodes[i + 1] : NULL;
    }
    
    /* Initialize linked list for pointer chasing */
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 4; j++) {
            nodes[i].data[j] = i * 20 + j;
        }
        for (j = 0; j < 8; j++) {
            nodes[i].offsets[j] = (short)(i * 3 + j);
        }
        nodes[i].next = (i < 9) ? &nodes[i + 1] : NULL;
    }
    
    /* Call all test functions multiple times with different arguments */
    /* This increases the chance of triggering different reload patterns */
    
    /* Test 1: Complex addressing */
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            for (k = 0; k < 3; k++) {
                total += test_complex_addressing(&cd, i, j, k);
                g_volatile_counter++; /* Force memory writes */
            }
        }
    }
    
    /* Test 2: Assembly reloads */
    for (i = 0; i < 10; i++) {
        total += test_asm_reloads(i, i * 2, i * 3);
        total += test_asm_reloads(i * 4, i * 5, i * 6);
    }
    
    /* Test 3: Pointer chasing */
    for (i = 0; i < 5; i++) {
        total += test_pointer_chasing(&nodes[0], i);
        total += test_pointer_chasing(&nodes[5], i * 2);
    }
    
    /* Test 4: Mixed operands */
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            total += test_mixed_operands(i, j, i + j);
            total += test_mixed_operands(i * 2, j * 3, i * j);
        }
    }
    
    /* Use the result to prevent dead code elimination */
    if (total > 0) {
        return total % 256; /* Return non-zero result */
    }
    
    return 0;
}
