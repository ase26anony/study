/* Compile with: gcc -O3 -fno-inline -fomit-frame-pointer -march=x86-64 -mno-sse -fno-optimize-sibling-calls reload_test.c -o reload_test */

#include <stdio.h>
#include <stdint.h>

/* Pattern 1: Complex addressing modes with volatile indices */
volatile int v_idx1 = 3, v_idx2 = 7, v_idx3 = 11;

/* Pattern 2: Vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Pattern 3: Structure for value passing */
struct DataPair {
    int a;
    int b;
    int c;
    int d;
};

/* Pattern 4: Large arrays to force spilling */
#define LARGE_SIZE 512
int large_array1[LARGE_SIZE][LARGE_SIZE];
int large_array2[LARGE_SIZE][LARGE_SIZE];

/* Pattern 5: Function passing/returning structures by value */
struct DataPair process_pair(struct DataPair p1, struct DataPair p2) {
    struct DataPair result;
    /* Complex operations to prevent optimization */
    result.a = p1.a + p2.b + v_idx1;
    result.b = p1.b - p2.c * v_idx2;
    result.c = p1.c ^ p2.d;
    result.d = p1.d | p2.a;
    
    /* Inline asm to force specific register usage */
    asm volatile ("# Middle asm block"
                  : "+r" (result.a), "+r" (result.b)
                  : "r" (result.c), "r" (result.d)
                  : "cc", "memory");
    
    return result;
}

struct DataPair chain_process(struct DataPair p) {
    struct DataPair temp1, temp2;
    temp1.a = p.b;
    temp1.b = p.c;
    temp1.c = p.d;
    temp1.d = p.a;
    
    temp2 = process_pair(p, temp1);
    
    /* More complex addressing */
    large_array1[v_idx1][temp2.a % LARGE_SIZE] = temp2.b;
    large_array2[temp2.c % LARGE_SIZE][v_idx2] = temp2.d;
    
    return temp2;
}

/* Pattern 6: Control flow with goto to split live ranges */
int complex_control_flow(int x) {
    int result = 0;
    volatile int control = x;
    
    if (control > 100) {
        goto block1;
    } else if (control > 50) {
        goto block2;
    } else {
        goto block3;
    }
    
block1:
    {
        int temp1 = x * 3;
        int temp2 = x + v_idx1;
        /* Force address reloads with array access */
        result = large_array1[temp1 % LARGE_SIZE][temp2 % LARGE_SIZE];
        goto join;
    }
    
block2:
    {
        int temp1 = x / 2;
        int temp2 = x - v_idx2;
        /* Different addressing pattern */
        result = large_array2[temp2 % LARGE_SIZE][temp1 % LARGE_SIZE];
        goto join;
    }
    
block3:
    {
        int temp1 = x << 2;
        int temp2 = x >> 1;
        /* Yet another addressing pattern */
        result = large_array1[temp2 % LARGE_SIZE][temp1 % LARGE_SIZE];
        goto join;
    }
    
join:
    /* Use inline asm with multiple constraints */
    asm volatile ("# Control flow asm"
                  : "+r" (result), "=m" (large_array1[0][0])
                  : "r" (v_idx3), "m" (large_array2[0][0])
                  : "cc");
    
    return result;
}

/* Pattern 7: Vector operations */
v4si vector_operations(v4si a, v4si b) {
    v4si result;
    
    /* Shuffle operation that may need decomposition */
    result = __builtin_shuffle(a, b, (v4si){3, 2, 1, 0});
    
    /* Complex vector arithmetic */
    result = result * a + b - (a >> 1);
    
    /* Store to memory with complex addressing */
    int *ptr = (int*)&result;
    for (int i = 0; i < 4; i++) {
        large_array1[v_idx1 + i][v_idx2] = ptr[i];
    }
    
    return result;
}

int main() {
    int checksum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < LARGE_SIZE; i++) {
        for (int j = 0; j < LARGE_SIZE; j++) {
            large_array1[i][j] = i * j;
            large_array2[i][j] = i + j;
        }
    }
    
    /* Pattern 1: Complex array addressing with volatile indices */
    for (int i = 0; i < 100; i++) {
        /* This should trigger RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
        large_array1[v_idx1 + i][v_idx2 * 2] = 
            large_array2[v_idx3][v_idx1 + i * 3] + 
            large_array1[i][v_idx2 + v_idx3];
    }
    
    /* Pattern 2: Structure passing chain */
    struct DataPair dp1 = {1, 2, 3, 4};
    struct DataPair dp2 = {5, 6, 7, 8};
    
    for (int i = 0; i < 50; i++) {
        dp1 = chain_process(dp1);
        dp2 = process_pair(dp2, dp1);
        
        /* Complex addressing in structure field access */
        checksum += large_array1[dp1.a % LARGE_SIZE][dp1.b % LARGE_SIZE];
        checksum -= large_array2[dp2.c % LARGE_SIZE][dp2.d % LARGE_SIZE];
    }
    
    /* Pattern 3: Inline assembly with multiple constraints */
    int asm_var1 = 0, asm_var2 = 0, asm_var3 = 0;
    
    /* Series of asm blocks creating dependency chains */
    asm volatile ("# First asm block\n\t"
                  "movl %2, %0\n\t"
                  "addl %3, %0"
                  : "=r" (asm_var1), "=m" (large_array1[0][v_idx1])
                  : "r" (v_idx1), "r" (v_idx2)
                  : "cc");
    
    asm volatile ("# Second asm block\n\t"
                  "imull %2, %0\n\t"
                  "movl %0, %1"
                  : "+r" (asm_var1), "=m" (large_array2[v_idx2][0])
                  : "r" (asm_var1)
                  : "cc", "memory");
    
    asm volatile ("# Third asm block with multiple outputs\n\t"
                  "leal (%1, %2, 4), %0\n\t"
                  "movl %0, %3"
                  : "=&r" (asm_var2), "=r" (asm_var3)
                  : "r" (asm_var1), "m" (large_array1[v_idx3][v_idx1]),
                    "1" (v_idx3)
                  : "cc");
    
    checksum += asm_var1 + asm_var2 + asm_var3;
    
    /* Pattern 4: Vector operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    for (int i = 0; i < 20; i++) {
        vec1 = vector_operations(vec1, vec2);
        vec2 = vector_operations(vec2, vec1);
        
        /* Extract elements with complex addressing */
        int *vptr = (int*)&vec1;
        for (int j = 0; j < 4; j++) {
            checksum += vptr[j] * large_array2[j][i % LARGE_SIZE];
        }
    }
    
    /* Pattern 5: Complex control flow */
    for (int i = 0; i < 200; i++) {
        checksum += complex_control_flow(i);
    }
    
    /* Pattern 6: Pointer arithmetic with non-constant offsets */
    int *ptr1 = &large_array1[0][0];
    int *ptr2 = &large_array2[0][0];
    
    for (int i = 0; i < 1000; i++) {
        /* Complex pointer arithmetic that can't be folded */
        int offset = (v_idx1 * i + v_idx2) % (LARGE_SIZE * LARGE_SIZE);
        ptr1[offset] = ptr2[offset * 2 % (LARGE_SIZE * LARGE_SIZE)] + i;
        checksum += ptr1[offset];
    }
    
    /* Final checksum computation with volatile to prevent optimization */
    volatile int final_checksum = checksum;
    printf("Result: %d\n", final_checksum);
    
    return 0;
}
