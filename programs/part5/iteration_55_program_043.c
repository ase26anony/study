/* test_reload_coverage.c
 * Designed to trigger multiple reload types in GCC's reload pass
 * Compile with: gcc -O0 -m32 -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-strict-aliasing test_reload_coverage.c -o test
 */

#include <stdint.h>
#include <stdio.h>

/* Force memory operations to prevent optimization */
#define VOLATILE_ACCESS(x) (*(volatile int*)(x))

/* Complex structure to force address computations */
struct nested {
    int data[4];
    struct nested *next;
    short offsets[8];
};

/* Global arrays to provide base addresses */
int global_array[256];
struct nested global_structs[16];
volatile int volatile_global;

/* Test function 1: Complex array addressing with multiple index computations */
static __attribute__((noinline)) 
int test_array_addressing(int a, int b, int c, int d, int e, int f) {
    /* Many local variables to consume registers */
    int i1 = a + 1;
    int i2 = b + 2;
    int i3 = c + 3;
    int i4 = d + 4;
    int i5 = e + 5;
    int i6 = f + 6;
    int i7 = a * b;
    int i8 = c * d;
    int i9 = e * f;
    int i10 = a + b + c;
    int i11 = d + e + f;
    int i12 = a * c * e;
    int i13 = b * d * f;
    int i14 = i1 + i2 + i3;
    int i15 = i4 + i5 + i6;
    
    /* Multi-dimensional array style access with complex addressing */
    /* This should trigger RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
    int sum = 0;
    sum += global_array[i1 + i2];                    /* Simple index */
    sum += global_array[i3 * 4 + i4];                /* Scaled index */
    sum += global_array[(i5 << 2) + (i6 >> 1)];      /* Shifted index */
    sum += global_array[i7 + i8 * 2 + i9];           /* Multiple index terms */
    
    /* Nested addressing with pointer arithmetic */
    /* Should trigger RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
    struct nested *ptr = &global_structs[0];
    for (int j = 0; j < 4; j++) {
        sum += ptr->data[(i10 + j) & 3];             /* Base + offset + index */
        sum += ptr->offsets[(i11 + j) & 7];          /* Different offset */
        /* Complex address computation */
        int *data_ptr = &ptr->data[0];
        sum += data_ptr[(i12 + j) & 3];              /* Pointer + index */
    }
    
    /* Volatile accesses force spills and reloads */
    VOLATILE_ACCESS(&volatile_global) = sum;
    sum += VOLATILE_ACCESS(&volatile_global);
    
    return sum + i14 + i15;
}

/* Test function 2: Inline assembly with multiple outputs and constraints */
static __attribute__((noinline))
int test_asm_reloads(int x, int y, int z) {
    int result1, result2, result3;
    int addr1, addr2, addr3;
    
    /* Many intermediate computations */
    int t1 = x * y;
    int t2 = y * z;
    int t3 = z * x;
    int t4 = x + y + z;
    int t5 = x - y - z;
    int t6 = t1 + t2 + t3;
    int t7 = t4 * t5;
    int t8 = t6 ^ t7;
    int t9 = ~t8;
    int t10 = t9 & 0xFF;
    
    /* Complex address computation for output */
    int *output_addr = &global_array[t10 & 0xF];
    
    /* Inline asm with multiple outputs and memory constraints */
    /* Should trigger RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    asm volatile (
        "movl %[in1], %[out1]\n\t"
        "addl %[in2], %[out1]\n\t"
        "movl %[out1], %[out2]\n\t"
        "imull %[in3], %[out2]\n\t"
        "leal (%[out2], %[out1], 2), %[out3]\n\t"
        : [out1] "=&r" (result1), 
          [out2] "=&r" (result2),
          [out3] "=&r" (result3),
          "=m" (*output_addr)      /* Memory output */
        : [in1] "r" (t1),
          [in2] "r" (t2),
          [in3] "r" (t3),
          "m" (*output_addr)       /* Memory input */
        : "cc"
    );
    
    /* More complex asm with alternative constraints */
    /* Should trigger RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS */
    asm volatile (
        "addl %%eax, %%ebx\n\t"
        "movl %%ebx, %%ecx\n\t"
        : "=b" (addr1), "=c" (addr2)
        : "a" (result1), "0" (result2), "1" (result3)
        : "cc"
    );
    
    /* Use the results in address computations */
    int *complex_addr = &global_array[(addr1 + addr2) & 0xFF];
    return result1 + result2 + result3 + *complex_addr + t10;
}

/* Test function 3: Structure pointer chasing with complex expressions */
static __attribute__((noinline))
int test_structure_chasing(int base) {
    /* Create many local variables */
    struct nested locals[8];
    int indices[8];
    int *pointers[8];
    
    /* Initialize with complex expressions */
    for (int i = 0; i < 8; i++) {
        indices[i] = (base + i * 3) & 0xF;
        locals[i].next = (i < 7) ? &locals[i + 1] : NULL;
        for (int j = 0; j < 4; j++) {
            locals[i].data[j] = base + i * 10 + j;
        }
        for (int j = 0; j < 8; j++) {
            locals[i].offsets[j] = (base + i + j) & 0x7FFF;
        }
        pointers[i] = &locals[i].data[0];
    }
    
    /* Pointer chasing with address computations */
    /* Should trigger various address reload types */
    int sum = 0;
    struct nested *current = &locals[0];
    
    #pragma GCC unroll 4
    for (int i = 0; i < 8; i++) {
        if (current) {
            /* Complex addressing modes */
            sum += current->data[indices[i] & 3];                /* RELOAD_FOR_INPUT */
            sum += current->offsets[indices[(i + 1) & 7] & 7];   /* Different index */
            
            /* Address of a structure field used in computation */
            int *data_addr = &current->data[0];                  /* RELOAD_FOR_OPERAND_ADDRESS */
            sum += data_addr[(indices[i] + 1) & 3];              /* Pointer + computed index */
            
            /* Chain following */
            current = current->next;
            
            /* More computations to use registers */
            sum += indices[i] * indices[(i + 1) & 7];
            sum += (int)pointers[i] & 0xFF;
        }
    }
    
    /* Final complex expression */
    return sum + base + (int)(locals[0].next != NULL);
}

/* Test function 4: Mixed operations with volatile and asm */
static __attribute__((noinline))
int test_mixed_reloads(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Exhaust registers with many variables */
    int v1 = a + b;
    int v2 = c + d;
    int v3 = e + f;
    int v4 = g + h;
    int v5 = a * c;
    int v6 = b * d;
    int v7 = e * g;
    int v8 = f * h;
    int v9 = v1 + v2;
    int v10 = v3 + v4;
    int v11 = v5 + v6;
    int v12 = v7 + v8;
    int v13 = v9 * v10;
    int v14 = v11 * v12;
    int v15 = v13 + v14;
    int v16 = v15 ^ 0xABCD;
    
    /* Complex array indexing with multiple dimensions simulated */
    /* Should trigger RELOAD_FOR_INPUT_ADDRESS */
    int idx1 = (v1 + v2) & 0xF;
    int idx2 = (v3 + v4) & 0xF;
    int idx3 = (v5 + v6) & 0xF;
    int idx4 = (v7 + v8) & 0xF;
    
    int sum = 0;
    sum += global_array[idx1 * 16 + idx2];      /* Scaled index */
    sum += global_array[idx3 * 8 + idx4];       /* Different scale */
    
    /* Nested addressing with structure */
    struct nested *sptr = &global_structs[idx1 & 7];
    sum += sptr->data[idx2 & 3];
    sum += sptr->offsets[idx3 & 7];
    
    /* Inline asm that clobbers many registers */
    /* Should trigger RELOAD_FOR_OUTPUT_ADDRESS */
    int asm_out1, asm_out2;
    int *mem_out = &global_array[v16 & 0xFF];
    
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "movl %[in2], %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movl %[in3], %%ecx\n\t"
        "imull %%ecx, %%eax\n\t"
        "movl %%eax, %[out2]\n\t"
        "movl %%eax, %[mem]\n\t"
        : [out1] "=r" (asm_out1),
          [out2] "=r" (asm_out2),
          [mem] "=m" (*mem_out)
        : [in1] "r" (v9),
          [in2] "r" (v10),
          [in3] "r" (v11)
        : "eax", "ebx", "ecx", "cc", "memory"
    );
    
    /* Use all variables to prevent optimization */
    return sum + asm_out1 + asm_out2 + v16;
}

/* Main driver that calls all test functions */
int main(void) {
    /* Initialize globals */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 4; j++) {
            global_structs[i].data[j] = i * 10 + j;
        }
        for (int j = 0; j < 8; j++) {
            global_structs[i].offsets[j] = (i + j) * 2;
        }
        global_structs[i].next = (i < 15) ? &global_structs[i + 1] : NULL;
    }
    
    volatile_global = 42;
    
    /* Call test functions with different arguments to trigger various patterns */
    int result = 0;
    
    /* Test 1: Array addressing */
    result += test_array_addressing(1, 2, 3, 4, 5, 6);
    result += test_array_addressing(7, 8, 9, 10, 11, 12);
    
    /* Test 2: Assembly reloads */
    result += test_asm_reloads(13, 14, 15);
    result += test_asm_reloads(16, 17, 18);
    
    /* Test 3: Structure chasing */
    result += test_structure_chasing(100);
    result += test_structure_chasing(200);
    
    /* Test 4: Mixed operations */
    result += test_mixed_reloads(19, 20, 21, 22, 23, 24, 25, 26);
    result += test_mixed_reloads(27, 28, 29, 30, 31, 32, 33, 34);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
