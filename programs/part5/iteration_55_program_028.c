/* reload_coverage.c - Test program to trigger various reload types in GCC */
#include <stdint.h>
#include <stdlib.h>

/* Force specific register usage and prevent optimizations */
#define NOOPT __attribute__((optimize("O0")))
#define NOINLINE __attribute__((noinline))
#define VOLATILE_MEM volatile

/* Complex structure to force address computations */
struct MultiLevel {
    int data[8];
    struct MultiLevel *next;
    short offsets[4];
    long double fp_val;
};

/* Global arrays to create complex addressing patterns */
static int global_array[256][16];
static struct MultiLevel struct_pool[64];
static VOLATILE_MEM int volatile_buffer[1024];

/* Function 1: Complex array indexing with multiple address computations */
NOINLINE NOOPT
static int test_complex_addressing(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Many local variables to exhaust registers */
    int i1 = a + b;
    int i2 = c * d;
    int i3 = e ^ f;
    int i4 = g & h;
    int i5 = a * c;
    int i6 = b + d;
    int i7 = e | f;
    int i8 = g ^ h;
    int i9 = a + c + e;
    int i10 = b * d * f;
    int i11 = g - h;
    int i12 = a << 2;
    int i13 = b >> 1;
    int i14 = c & 0xFF;
    int i15 = d | 0x0F;
    int i16 = e ^ 0x55;
    
    /* Complex array addressing - will need RELOAD_FOR_INPUT_ADDRESS */
    int sum = 0;
    
    /* Multi-level array indexing with multiple computations */
    sum += global_array[i1 + i2][i3 & 0xF];
    sum += global_array[i4 * 2][i5 % 16];
    sum += global_array[i6 - i7][i8 ^ 0x7];
    sum += global_array[i9 >> 1][i10 & 0xF];
    
    /* Nested addressing with pointer arithmetic */
    struct MultiLevel *ptr = &struct_pool[0];
    for (int i = 0; i < 4; i++) {
        /* This creates RELOAD_FOR_INPADDR_ADDRESS */
        sum += ptr->data[ptr->offsets[i] + i];
        ptr = ptr->next;
    }
    
    /* More complex expressions with volatile */
    VOLATILE_MEM int v1 = volatile_buffer[i11];
    VOLATILE_MEM int v2 = volatile_buffer[i12];
    sum += v1 * v2;
    
    return sum + i13 + i14 + i15 + i16;
}

/* Function 2: Inline assembly with multiple outputs and constraints */
NOINLINE NOOPT
static int test_asm_reloads(int x, int y, int z) {
    int result1, result2, result3, result4;
    int addr1, addr2, addr3;
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    
    /* Many temporaries to create register pressure */
    temp1 = x * y;
    temp2 = y * z;
    temp3 = z * x;
    temp4 = x + y + z;
    temp5 = x ^ y ^ z;
    temp6 = (x << 3) | (y << 2) | (z << 1);
    temp7 = ~x & ~y & ~z;
    temp8 = temp1 + temp2 + temp3;
    
    /* Complex address computation for output */
    addr1 = (int)(&global_array[temp1 % 16][temp2 % 8]);
    addr2 = (int)(&struct_pool[temp3 % 32]);
    addr3 = (int)(&volatile_buffer[temp4 % 256]);
    
    /* Inline asm with multiple outputs and memory constraints
       This can trigger RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    __asm__ volatile (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movl %[addr1], %%ebx\n\t"
        "movl %%eax, (%%ebx)\n\t"
        "movl %[in3], %%ecx\n\t"
        "imull %%ecx, %%eax\n\t"
        "movl %%eax, %[out2]\n\t"
        "movl %[addr2], %%edx\n\t"
        "addl $4, %%edx\n\t"
        "movl %%eax, (%%edx)"
        : [out1] "=m" (*(int*)addr1),  /* Memory output - needs address reload */
          [out2] "=r" (result2)        /* Register output */
        : [in1] "r" (temp5),
          [in2] "r" (temp6),
          [in3] "r" (temp7),
          [addr1] "r" (addr1),         /* Address in register */
          [addr2] "r" (addr2)          /* Another address */
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    /* More asm with operand address reloads */
    int *ptr1 = (int*)addr3;
    int *ptr2 = &global_array[0][0];
    
    __asm__ volatile (
        "movl (%[ptr1]), %%eax\n\t"
        "addl (%[ptr2], %[idx], 4), %%eax\n\t"
        "movl %%eax, %[res]"
        : [res] "=r" (result3)
        : [ptr1] "r" (ptr1),           /* Pointer operand - may need RELOAD_FOR_OPERAND_ADDRESS */
          [ptr2] "r" (ptr2),
          [idx] "r" (temp8 & 0xF)
        : "eax"
    );
    
    return result2 + result3 + temp8;
}

/* Function 3: Pointer chasing with complex address computations */
NOINLINE NOOPT
static long test_pointer_chasing(int seed) {
    /* Create a chain of structures */
    for (int i = 0; i < 63; i++) {
        struct_pool[i].next = &struct_pool[i + 1];
        for (int j = 0; j < 4; j++) {
            struct_pool[i].offsets[j] = (i * j) & 3;
        }
    }
    struct_pool[63].next = &struct_pool[0];
    
    /* Many local variables for register pressure */
    int idx1 = seed * 3;
    int idx2 = seed * 5;
    int idx3 = seed * 7;
    int idx4 = seed * 11;
    int idx5 = seed * 13;
    int idx6 = seed * 17;
    int idx7 = seed * 19;
    int idx8 = seed * 23;
    int idx9 = seed * 29;
    int idx10 = seed * 31;
    
    /* Complex pointer arithmetic - triggers various address reloads */
    struct MultiLevel *current = &struct_pool[seed % 64];
    long total = 0;
    
    /* Manual loop unrolling to increase register pressure */
    /* Each iteration uses different addressing modes */
    
    /* Iteration 1: Direct structure access */
    total += current->data[current->offsets[0]];
    
    /* Iteration 2: Pointer offset computation */
    struct MultiLevel *next = current->next;
    total += next->data[(idx1 + idx2) & 7];
    
    /* Iteration 3: More complex address */
    total += next->next->data[(idx3 * idx4) & 7];
    
    /* Iteration 4: With volatile */
    VOLATILE_MEM struct MultiLevel *vptr = next->next->next;
    total += vptr->data[idx5 & 7];
    
    /* Iteration 5: Array of pointers */
    struct MultiLevel *ptr_array[4];
    ptr_array[0] = current;
    ptr_array[1] = next;
    ptr_array[2] = next->next;
    ptr_array[3] = vptr;
    
    for (int i = 0; i < 4; i++) {
        /* This addressing may need RELOAD_FOR_OPADDR_ADDR */
        total += ptr_array[i]->data[ptr_array[i]->offsets[i]];
    }
    
    /* Additional computations to use all local variables */
    total += global_array[idx6 & 0xFF][idx7 & 0xF];
    total += global_array[idx8 & 0xFF][idx9 & 0xF];
    total += global_array[idx10 & 0xFF][(idx1 + idx2) & 0xF];
    
    return total;
}

/* Function 4: Mixed operand types with builtins */
NOINLINE NOOPT
static int test_mixed_operands(double d1, double d2, float f1, float f2) {
    /* Integer locals */
    int i1 = (int)d1;
    int i2 = (int)d2;
    int i3 = (int)f1;
    int i4 = (int)f2;
    int i5 = i1 * i2;
    int i6 = i3 + i4;
    int i7 = i1 ^ i2;
    int i8 = i3 | i4;
    int i9 = i5 << 2;
    int i10 = i6 >> 1;
    
    /* Use __builtin_expect to create data dependencies */
    if (__builtin_expect(i1 > i2, 0)) {
        i9 = i3 * i4;
    }
    
    if (__builtin_expect(i3 < i4, 1)) {
        i10 = i1 + i2;
    }
    
    /* Complex expression mixing everything */
    int result = 0;
    
    /* Multiple memory accesses with different address computations */
    for (int i = 0; i < 8; i++) {
        /* Varying addressing modes */
        switch (i & 3) {
            case 0:
                /* Simple indexing */
                result += global_array[i1 + i][i2 & 0xF];
                break;
            case 1:
                /* Pointer with offset */
                result += *(int*)((char*)&struct_pool[0] + i * 16 + i3);
                break;
            case 2:
                /* Volatile access */
                result += volatile_buffer[i4 + i];
                break;
            case 3:
                /* Complex computed address */
                int *addr = &global_array[(i5 + i) & 0xFF][(i6 + i) & 0xF];
                result += *addr;
                break;
        }
    }
    
    return result + i7 + i8 + i9 + i10;
}

/* Main driver function */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 16; j++) {
            global_array[i][j] = i * j;
        }
    }
    
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 8; j++) {
            struct_pool[i].data[j] = i + j;
        }
        struct_pool[i].fp_val = i * 1.5;
    }
    
    for (int i = 0; i < 1024; i++) {
        volatile_buffer[i] = i & 0xFF;
    }
    
    /* Call all test functions with different arguments
       to trigger different reload patterns */
    
    /* Test 1: Complex addressing */
    result += test_complex_addressing(
        argc, argc*2, argc*3, argc*4,
        argc*5, argc*6, argc*7, argc*8
    );
    
    /* Test 2: Assembly reloads */
    result += test_asm_reloads(argc, argc+1, argc+2);
    
    /* Test 3: Pointer chasing */
    result += (int)test_pointer_chasing(argc);
    
    /* Test 4: Mixed operands */
    result += test_mixed_operands(
        argc * 1.5, argc * 2.5,
        argc * 0.5f, argc * 1.5f
    );
    
    /* Prevent dead code elimination */
    volatile int final_result = result;
    
    return final_result % 256;
}
