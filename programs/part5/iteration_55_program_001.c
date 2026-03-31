/* reload_coverage.c - Program to trigger various reload types in GCC reload1.cc */
#include <stdint.h>
#include <stdlib.h>

/* Force specific architecture for register scarcity */
#ifdef __x86_64__
#error "Compile with -m32 for 32-bit target to increase register pressure"
#endif

/* Prevent unwanted optimizations */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Complex data structures to force address computations */
struct Nested {
    int data[4];
    struct Nested *next;
    volatile int counter;
};

struct MultiDim {
    int arr[3][3][3];
    volatile long offsets[8];
    struct Nested *ptr;
};

/* Global arrays to prevent complete optimization */
static volatile int global_array[256];
static struct MultiDim global_structs[4];

/* Test 1: Complex array addressing with multiple index computations */
NOINLINE static int test_complex_addressing(int a, int b, int c, int d, int e, int f) {
    /* Many local variables to consume registers */
    volatile int v1 = a * b;
    volatile int v2 = c + d;
    volatile int v3 = e ^ f;
    volatile int v4 = a | c;
    volatile int v5 = b & d;
    volatile int v6 = e << 2;
    volatile int v7 = f >> 1;
    volatile int v8 = v1 + v2;
    volatile int v9 = v3 * v4;
    volatile int v10 = v5 - v6;
    
    /* Multi-dimensional array with complex indexing */
    int local_arr[4][4][4];
    
    /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                /* Complex address computation requiring multiple registers */
                local_arr[i + v1 % 2][j + v2 % 2][k + v3 % 2] = 
                    global_structs[i].arr[j][k][0] + 
                    *(int*)((char*)&global_structs[i] + 
                           v4 * sizeof(int) + 
                           v5 * j * sizeof(int) +
                           v6 * k);
            }
        }
    }
    
    /* More register pressure */
    volatile int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += local_arr[i][i % 3][i % 2] + v7 + v8 - v9 + v10;
    }
    
    return sum;
}

/* Test 2: Inline assembly with multiple outputs and complex constraints */
NOINLINE static int test_asm_reloads(int x, int y, int z) {
    int result1, result2, result3;
    volatile int addr_var = 0x1000;
    volatile int offset_var = 0x20;
    
    /* Force RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    /* Complex memory output operand */
    asm volatile (
        "movl %[addr], %%eax\n\t"
        "addl %[offset], %%eax\n\t"
        "movl %[x], (%%eax)\n\t"
        "movl %[y], 4(%%eax)\n\t"
        "movl %[z], 8(%%eax)\n\t"
        "movl (%%eax), %[out1]\n\t"
        "movl 4(%%eax), %[out2]\n\t"
        "movl 8(%%eax), %[out3]\n\t"
        : [out1] "=r" (result1), [out2] "=r" (result2), [out3] "=r" (result3)
        : [x] "r" (x), [y] "r" (y), [z] "r" (z),
          [addr] "m" (addr_var), [offset] "m" (offset_var)
        : "eax", "memory", "cc"
    );
    
    /* Force more register pressure */
    volatile int a = result1 * result2;
    volatile int b = result2 + result3;
    volatile int c = result3 ^ result1;
    volatile int d = a << b;
    volatile int e = c >> 2;
    volatile int f = d | e;
    volatile int g = f & a;
    volatile int h = g + b;
    volatile int i = h - c;
    volatile int j = i * d;
    
    return result1 + result2 + result3 + a + b + c + d + e + f + g + h + i + j;
}

/* Test 3: Pointer chasing with structure accesses */
NOINLINE static int test_pointer_chasing(struct Nested *start, int iterations) {
    struct Nested *current = start;
    volatile int sum = 0;
    volatile int temp1, temp2, temp3, temp4, temp5;
    
    /* Force RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
    for (int i = 0; i < iterations; i++) {
        if (!current) break;
        
        /* Complex pointer arithmetic forcing address reloads */
        int *data_ptr = (int*)((char*)current + 
                              (i % 4) * sizeof(int) + 
                              (current->counter % 8));
        
        /* Multiple uses of the same computed address */
        temp1 = *data_ptr;
        temp2 = *(data_ptr + 1);
        temp3 = *(data_ptr + 2);
        
        /* Force RELOAD_FOR_OTHER_ADDRESS */
        asm volatile (
            "movl %[ptr], %%ebx\n\t"
            "addl $16, %%ebx\n\t"
            "movl (%%ebx), %%ecx\n\t"
            : : [ptr] "r" (current) : "ebx", "ecx", "memory"
        );
        
        /* More register pressure */
        temp4 = temp1 * temp2 + temp3;
        temp5 = temp2 - temp3 * temp1;
        
        sum += temp4 + temp5 + current->counter;
        
        /* Complex address computation for next pointer */
        current = (struct Nested*)((char**)current + 
                                  (temp1 % 2) * sizeof(struct Nested*) +
                                  (temp2 % 4));
    }
    
    return sum;
}

/* Test 4: Mixed reload types with volatile and builtins */
NOINLINE static int test_mixed_reloads(void) {
    /* Create many local variables for register pressure */
    volatile int var1 = 1, var2 = 2, var3 = 3, var4 = 4, var5 = 5;
    volatile int var6 = 6, var7 = 7, var8 = 8, var9 = 9, var10 = 10;
    volatile int var11 = 11, var12 = 12, var13 = 13, var14 = 14, var15 = 15;
    volatile int var16 = 16, var17 = 17, var18 = 18, var19 = 19, var20 = 20;
    
    /* Complex expression forcing many temporaries */
    int result = 
        var1 * var2 + var3 - var4 / (var5 + 1) +
        var6 * var7 - var8 + var9 / (var10 + 1) +
        var11 * var12 + var13 - var14 / (var15 + 1) +
        var16 * var17 - var18 + var19 / (var20 + 1);
    
    /* Force RELOAD_OTHER through obscure operand usage */
    /* Use __builtin_expect to create data dependencies */
    if (__builtin_expect(result > 100, 0)) {
        /* Complex addressing mode */
        int *complex_ptr = &global_array[
            (var1 * var2 + var3 * var4) % 256 +
            (var5 * var6 - var7 * var8) % 256 +
            (var9 * var10 + var11 * var12) % 256
        ];
        
        /* Multiple memory accesses with same base but different offsets */
        volatile int val1 = *complex_ptr;
        volatile int val2 = *(complex_ptr + var13 % 64);
        volatile int val3 = *(complex_ptr + var14 % 64 + var15 % 32);
        
        result += val1 + val2 + val3;
    }
    
    /* More register pressure with inline assembly */
    asm volatile (
        "movl %[a], %%eax\n\t"
        "movl %[b], %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %[res]\n\t"
        : [res] "=m" (result)
        : [a] "r" (result), [b] "r" (var20)
        : "eax", "ebx", "cc"
    );
    
    return result;
}

/* Helper to initialize test data */
static void init_test_data(void) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                for (int l = 0; l < 3; l++) {
                    global_structs[i].arr[j][k][l] = i * 1000 + j * 100 + k * 10 + l;
                }
            }
        }
        global_structs[i].ptr = (struct Nested*)malloc(sizeof(struct Nested));
        if (global_structs[i].ptr) {
            for (int j = 0; j < 4; j++) {
                global_structs[i].ptr->data[j] = i * 10 + j;
            }
            global_structs[i].ptr->counter = i;
            global_structs[i].ptr->next = NULL;
        }
    }
    
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
}

/* Main driver that calls all tests */
int main(void) {
    init_test_data();
    
    int total = 0;
    
    /* Call test functions multiple times with different arguments */
    for (int i = 0; i < 3; i++) {
        total += test_complex_addressing(i, i+1, i+2, i+3, i+4, i+5);
        total += test_asm_reloads(i*10, i*20, i*30);
        if (global_structs[i % 4].ptr) {
            total += test_pointer_chasing(global_structs[i % 4].ptr, 5);
        }
        total += test_mixed_reloads();
    }
    
    /* Use the result to prevent dead code elimination */
    volatile int final_result = total;
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        if (global_structs[i].ptr) {
            free(global_structs[i].ptr);
        }
    }
    
    return final_result % 256;
}
