/* Test program to exercise GCC reload pass switch cases */
#include <stdio.h>
#include <stdint.h>

/* Volatile indices to prevent constant folding */
static volatile int idx1 = 1;
static volatile int idx2 = 2;
static volatile int idx3 = 3;

/* Structure for passing by value */
struct SmallStruct {
    int a;
    int b;
    int c;
    int d;
};

/* Large array to force spills */
int large_array[1000][100];

/* Vector type to stress register allocation */
typedef int v4si __attribute__((vector_size(16)));

/* Function passing/returning structure by value */
struct SmallStruct process_struct(struct SmallStruct s1, struct SmallStruct s2) {
    struct SmallStruct result;
    result.a = s1.a + s2.b;
    result.b = s1.b - s2.a;
    result.c = s1.c * s2.c;
    result.d = s1.d / (s2.d ? s2.d : 1);
    return result;
}

/* Function with complex addressing */
void complex_addressing(int n) {
    int i, j, k;
    
    /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
    for (i = 0; i < n; i++) {
        for (j = 0; j < 10; j++) {
            /* Complex addressing with volatile indices */
            large_array[idx1 + i][j * idx2] = 
                large_array[idx3 * 2][i + idx1] + 
                large_array[j][idx2 * i];
            
            /* More complex addressing */
            large_array[(i * idx1 + j * idx2) % 100][(idx3 + i) % 1000] +=
                large_array[(j << 2) % 100][(i >> 1) % 1000];
        }
    }
}

/* Function with inline assembly chains */
void asm_chains(void) {
    int a, b, c, d, e, f;
    volatile int mem1, mem2, mem3;
    
    /* Chain 1: Output used as input in next asm */
    asm volatile (
        "movl %1, %0\n\t"
        "addl $1, %0"
        : "=r" (a)
        : "r" (idx1)
        : "cc"
    );
    
    asm volatile (
        "imull %1, %0\n\t"
        "addl %2, %0"
        : "+r" (a)
        : "r" (idx2), "r" (a)
        : "cc"
    );
    
    /* Force memory operand reloads */
    mem1 = a;
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %%eax, %0"
        : "=m" (mem2)
        : "r" (mem1)
        : "%eax"
    );
    
    /* Multiple output operands */
    asm volatile (
        "leal (%1,%2,2), %0\n\t"
        "movl %0, %3"
        : "=r" (b), "=m" (mem3)
        : "r" (a), "r" (idx3)
        : "cc"
    );
}

/* Function with vector operations */
void vector_ops(void) {
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3, v4;
    volatile v4si vmem;
    
    /* Vector operations that may need decomposition */
    v3 = v1 + v2;
    v4 = v1 * v2;
    
    /* Shuffle operation */
    v4 = __builtin_shuffle(v3, v4, (v4si){3, 2, 1, 0});
    
    /* Store to volatile to prevent elimination */
    vmem = v3 + v4;
    
    /* Use in addressing calculation */
    int* ptr = (int*)&vmem;
    large_array[ptr[0] % 100][ptr[1] % 1000] = ptr[2];
}

/* Function with goto to split live ranges */
void split_live_ranges(int n) {
    int a = idx1, b = idx2, c = idx3;
    int i = 0;
    
    if (n <= 0) goto cleanup;
    
loop_start:
    /* Complex computation creating register pressure */
    a = a * b + c;
    b = b - a * i;
    c = c + (a ^ b);
    
    /* Store intermediate results to memory */
    large_array[i % 100][0] = a;
    large_array[i % 100][1] = b;
    large_array[i % 100][2] = c;
    
    i++;
    if (i < n) goto loop_start;
    
    /* Force RELOAD_FOR_OTHER_ADDRESS for spill code */
    asm volatile ("" : : "r" (a), "r" (b), "r" (c) : "memory");
    
cleanup:
    /* Use values after control flow merge */
    large_array[0][99] = a + b + c;
}

/* Main orchestrator */
int main(void) {
    int i, checksum = 0;
    struct SmallStruct s1 = {100, 200, 300, 400};
    struct SmallStruct s2 = {500, 600, 700, 800};
    struct SmallStruct result;
    
    /* Initialize array with pattern */
    for (i = 0; i < 1000; i++) {
        large_array[i][0] = i;
        large_array[i][1] = i * 2;
        large_array[i][2] = i * 3;
    }
    
    /* Exercise structure passing (RELOAD_FOR_INPADDR_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS) */
    for (i = 0; i < 10; i++) {
        result = process_struct(s1, s2);
        s1.a = result.b;
        s1.b = result.c;
        s2.c = result.d;
        s2.d = result.a;
    }
    
    /* Complex addressing patterns */
    complex_addressing(50);
    
    /* Inline assembly chains */
    for (i = 0; i < 5; i++) {
        asm_chains();
    }
    
    /* Vector operations */
    vector_ops();
    
    /* Split live ranges with gotos */
    split_live_ranges(100);
    
    /* Compute checksum to prevent optimization */
    for (i = 0; i < 100; i++) {
        checksum += large_array[i][0];
        checksum += large_array[i][1];
        checksum += large_array[i][2];
        checksum += large_array[i][99];
    }
    
    checksum += result.a + result.b + result.c + result.d;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
