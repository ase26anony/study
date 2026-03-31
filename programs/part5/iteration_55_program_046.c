/* reload_coverage.c - Program to trigger various reload types in GCC reload1.cc */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Force specific reload types through different patterns */

/* Pattern 1: Complex array addressing - triggers RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS */
static inline __attribute__((always_inline)) 
int complex_array_access(int a, int b, int c, int d, int e, int f, int g, int h) {
    volatile int arr1[8][8][8];
    volatile int arr2[7][7][7];
    volatile int arr3[6][6][6];
    
    /* Multi-level indexing with many live variables */
    int idx1 = a + b;
    int idx2 = c + d;
    int idx3 = e + f;
    int idx4 = g + h;
    int idx5 = a * b;
    int idx6 = c * d;
    int idx7 = e * f;
    int idx8 = g * h;
    
    /* Complex addressing that needs address reloads */
    int val1 = arr1[idx1][idx2][idx3];
    int val2 = arr2[idx4][idx5][idx6];
    int val3 = arr3[idx7][idx8][idx1];
    
    /* More complex nested addressing */
    int val4 = arr1[arr2[idx1][idx2][idx3] & 7][arr3[idx4][idx5][idx6] & 7][idx7 & 7];
    
    return val1 + val2 + val3 + val4;
}

/* Pattern 2: Structure with pointer chasing - triggers RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
struct Node {
    volatile int data[4];
    struct Node* volatile next;
    volatile int more_data[3];
};

static __attribute__((noinline))
int pointer_chasing(struct Node* n1, struct Node* n2, struct Node* n3) {
    volatile int accum = 0;
    
    /* Pointer arithmetic with many intermediate values */
    struct Node* p1 = n1;
    struct Node* p2 = n2;
    struct Node* p3 = n3;
    
    /* Complex pointer dereferencing */
    accum += p1->data[0] + p1->data[1] + p1->data[2] + p1->data[3];
    accum += p2->data[0] + p2->data[1] + p2->data[2] + p2->data[3];
    accum += p3->data[0] + p3->data[1] + p3->data[2] + p3->data[3];
    
    /* Nested pointer access */
    accum += p1->next->data[0];
    accum += p2->next->data[1];
    accum += p3->next->data[2];
    
    /* Address of structure member computations */
    int* addr1 = &p1->data[1];
    int* addr2 = &p2->data[2];
    int* addr3 = &p3->data[3];
    
    accum += *addr1 + *addr2 + *addr3;
    
    return accum;
}

/* Pattern 3: Inline assembly with multiple outputs - triggers RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS */
static __attribute__((noinline))
int asm_reload_pattern(int a, int b, int c, int d, int e, int f, int g, int h) {
    volatile int out1, out2, out3, out4;
    volatile int mem1, mem2, mem3, mem4;
    
    /* Complex memory output addressing */
    int base = a + b;
    int index = c + d;
    int scale = e;
    int offset = f + g + h;
    
    /* Inline asm with memory output operand using complex addressing */
    asm volatile (
        "movl %[base], %%eax\n\t"
        "movl %[index], %%ebx\n\t"
        "movl %[scale], %%ecx\n\t"
        "movl %[offset], %%edx\n\t"
        "leal (%%eax,%%ebx,%%ecx), %%esi\n\t"
        "addl %%edx, %%esi\n\t"
        "movl %%esi, %[out1]\n\t"
        "movl %%eax, %[out2]\n\t"
        "movl %%ebx, %[out3]\n\t"
        "movl %%ecx, %[out4]"
        : [out1] "=m" (out1), [out2] "=m" (out2), [out3] "=m" (out3), [out4] "=m" (out4)
        : [base] "r" (base), [index] "r" (index), [scale] "r" (scale), [offset] "r" (offset)
        : "eax", "ebx", "ecx", "edx", "esi", "memory"
    );
    
    /* Another asm with different constraints */
    asm volatile (
        "addl %1, %0\n\t"
        "addl %2, %0\n\t"
        "addl %3, %0"
        : "+r" (mem1), "+r" (mem2), "+r" (mem3), "+r" (mem4)
        : 
        : "cc"
    );
    
    return out1 + out2 + out3 + out4 + mem1 + mem2 + mem3 + mem4;
}

/* Pattern 4: Mixed scalar operations with many live values - triggers RELOAD_FOR_INPUT, RELOAD_OTHER */
static __attribute__((noinline))
int scalar_pressure(int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8) {
    /* Create many scalar variables that must stay alive */
    int v1 = p1 * p2;
    int v2 = p3 * p4;
    int v3 = p5 * p6;
    int v4 = p7 * p8;
    int v5 = p1 + p2;
    int v6 = p3 + p4;
    int v7 = p5 + p6;
    int v8 = p7 + p8;
    int v9 = p1 - p2;
    int v10 = p3 - p4;
    int v11 = p5 - p6;
    int v12 = p7 - p8;
    int v13 = p1 ^ p2;
    int v14 = p3 ^ p4;
    int v15 = p5 ^ p6;
    int v16 = p7 ^ p8;
    
    /* Use all variables in complex expressions */
    int r1 = v1 + v2 + v3 + v4;
    int r2 = v5 * v6 * v7 * v8;
    int r3 = v9 & v10 & v11 & v12;
    int r4 = v13 | v14 | v15 | v16;
    
    /* Force spilling by using all in one expression */
    int result = r1 + r2 + r3 + r4;
    result += v1 * v5 + v2 * v6 + v3 * v7 + v4 * v8;
    result += v9 ^ v13 + v10 ^ v14 + v11 ^ v15 + v12 ^ v16;
    
    /* Use __builtin_expect to create data dependencies */
    if (__builtin_expect(result > 1000, 0)) {
        result = v1 + v2 + v3;
    } else {
        result = v4 + v5 + v6;
    }
    
    return result;
}

/* Pattern 5: Loop with unrolling - multiplies register pressure */
static __attribute__((noinline))
int unrolled_loop(int iterations) {
    volatile int array[16];
    int sum = 0;
    
    /* Manually unrolled loop with many array accesses */
    #pragma GCC unroll 4
    for (int i = 0; i < iterations; i++) {
        /* Many array accesses with complex indexing */
        sum += array[i & 0xF];
        sum += array[(i + 1) & 0xF];
        sum += array[(i + 2) & 0xF];
        sum += array[(i + 3) & 0xF];
        sum += array[(i + 4) & 0xF];
        sum += array[(i + 5) & 0xF];
        sum += array[(i + 6) & 0xF];
        sum += array[(i + 7) & 0xF];
    }
    
    return sum;
}

/* Pattern 6: Mixed addressing modes - triggers RELOAD_FOR_OTHER_ADDRESS */
static __attribute__((noinline))
int mixed_addressing(int a, int b, int c, int d, int e, int f) {
    volatile int matrix[8][8];
    volatile int* ptrs[8];
    int results[8];
    
    /* Initialize pointer array */
    for (int i = 0; i < 8; i++) {
        ptrs[i] = &matrix[i][0];
    }
    
    /* Mixed addressing: array + pointer + offset */
    results[0] = matrix[a][b];
    results[1] = *(ptrs[c] + d);
    results[2] = *(*(ptrs + e) + f);
    results[3] = matrix[b][c];
    results[4] = *(ptrs[d] + e);
    results[5] = *(*(ptrs + f) + a);
    
    /* Complex address computation */
    int* addr1 = &matrix[a & 7][b & 7];
    int* addr2 = ptrs[c & 7] + (d & 7);
    int** addr3 = &ptrs[e & 7];
    
    results[6] = *addr1 + *addr2 + **addr3;
    results[7] = matrix[f][a] + matrix[b][c];
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    
    return sum;
}

/* Main driver that calls all patterns */
int main(void) {
    int total = 0;
    
    /* Initialize test data */
    struct Node node1, node2, node3;
    for (int i = 0; i < 4; i++) {
        node1.data[i] = i;
        node2.data[i] = i + 4;
        node3.data[i] = i + 8;
    }
    node1.next = &node2;
    node2.next = &node3;
    node3.next = &node1;
    
    /* Call each pattern multiple times with different args */
    for (int i = 0; i < 10; i++) {
        total += complex_array_access(i, i+1, i+2, i+3, i+4, i+5, i+6, i+7);
        total += pointer_chasing(&node1, &node2, &node3);
        total += asm_reload_pattern(i, i*2, i*3, i*4, i*5, i*6, i*7, i*8);
        total += scalar_pressure(i, i+10, i+20, i+30, i+40, i+50, i+60, i+70);
        total += unrolled_loop(8);
        total += mixed_addressing(i, i+1, i+2, i+3, i+4, i+5);
    }
    
    printf("Total: %d\n", total);
    return total != 0 ? 0 : 1;
}
