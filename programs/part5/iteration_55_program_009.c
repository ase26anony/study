/* reload_coverage.c - Program to trigger various reload types in GCC reload1.cc */
#include <stdint.h>
#include <stdlib.h>

/* Force specific architecture for register scarcity */
#ifdef __x86_64__
#error "Compile with -m32 for 32-bit target to increase register pressure"
#endif

/* Volatile variables to force memory accesses */
static volatile int g_volatile_counter = 0;
static volatile int* g_volatile_ptr = NULL;

/* Complex structure for address computations */
struct NestedData {
    int values[8];
    struct NestedData* next;
    int matrix[3][3];
};

/* Global arrays to work with */
static int g_array1[256];
static int g_array2[256];
static struct NestedData g_structs[16];

/* ========== TEST FUNCTION 1: Complex Array Addressing ========== */
/* Targets: RELOAD_FOR_INPUT, RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS */
static int __attribute__((noinline)) 
test_complex_addressing(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Many local variables to consume registers */
    int i1 = a + 1, i2 = b + 2, i3 = c + 3, i4 = d + 4;
    int i5 = e + 5, i6 = f + 6, i7 = g + 7, i8 = h + 8;
    int i9 = a * b, i10 = c * d, i11 = e * f, i12 = g * h;
    volatile int v1, v2, v3, v4;
    
    /* Complex array indexing - forces address reloads */
    int* ptr1 = &g_array1[i1 * i2 + i3];
    int* ptr2 = &g_array2[i4 * i5 + i6];
    
    /* Multi-level array access with volatile */
    v1 = g_array1[((i1 * i3) + (i2 * i4)) % 256];
    v2 = g_array2[((i5 * i7) + (i6 * i8)) % 256];
    
    /* Nested addressing requiring multiple base registers */
    int result = 0;
    for (int i = 0; i < 4; i++) {
        /* Unroll manually to increase pressure */
        result += g_array1[ptr1[i] % 256] * g_array2[ptr2[i] % 256];
        result += g_array1[(ptr1[i] + i1) % 256] * g_array2[(ptr2[i] + i2) % 256];
        result += g_array1[(ptr1[i] * i3) % 256] * g_array2[(ptr2[i] * i4) % 256];
        result += g_array1[(ptr1[i] / (i5 + 1)) % 256] * g_array2[(ptr2[i] / (i6 + 1)) % 256];
    }
    
    /* Address of address computation */
    int** addr_of_ptr = &ptr1;
    v3 = **addr_of_ptr;
    
    return result + v1 + v2 + v3 + i9 + i10 + i11 + i12;
}

/* ========== TEST FUNCTION 2: Structure Member Access ========== */
/* Targets: RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS, RELOAD_FOR_OPERAND_ADDRESS */
static int __attribute__((noinline))
test_structure_access(int idx) {
    /* Many local struct pointers */
    struct NestedData* s1 = &g_structs[idx % 16];
    struct NestedData* s2 = &g_structs[(idx + 1) % 16];
    struct NestedData* s3 = &g_structs[(idx + 2) % 16];
    struct NestedData* s4 = &g_structs[(idx + 3) % 16];
    
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    volatile int v;
    
    /* Complex structure member addressing */
    temp1 = s1->values[0] + s1->values[1] * s1->values[2];
    temp2 = s2->values[3] + s2->values[4] * s2->values[5];
    temp3 = s3->matrix[0][0] + s3->matrix[1][1] + s3->matrix[2][2];
    temp4 = s4->matrix[0][2] + s4->matrix[2][0] + s4->matrix[1][1];
    
    /* Pointer chasing with address computations */
    struct NestedData* cur = s1;
    for (int i = 0; i < 4; i++) {
        temp5 += cur->values[i % 8];
        temp6 += cur->matrix[i % 3][(i + 1) % 3];
        /* Simulate linked list traversal */
        cur = (struct NestedData*)((char*)cur + sizeof(int) * 2);
    }
    
    /* Inline assembly with memory output operand */
    /* This can trigger output address reloads */
    int output1, output2;
    asm volatile (
        "movl %[in1], %[out1]\n\t"
        "addl %[in2], %[out1]\n\t"
        "movl %[out1], %[out2]"
        : [out1] "=r" (output1), [out2] "=m" (g_array1[idx])
        : [in1] "r" (temp1), [in2] "r" (temp2)
        : "cc"
    );
    
    /* More complex asm with multiple constraints */
    int* ptr_out = &g_array2[idx];
    asm volatile (
        "imull %%eax, %%ecx\n\t"
        "addl %%ecx, %[result]"
        : [result] "+m" (*ptr_out)
        : "a" (temp3), "c" (temp4)
        : "cc"
    );
    
    v = g_volatile_counter;
    return temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + output1 + v;
}

/* ========== TEST FUNCTION 3: Inline Assembly with Many Clobbers ========== */
/* Targets: RELOAD_OTHER, RELOAD_FOR_OTHER_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
static int __attribute__((noinline))
test_asm_clobbers(int a, int b, int c, int d, int e, int f) {
    int r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
    volatile int v;
    
    /* Use many register variables to increase pressure */
    register int reg1 asm("eax") = a;
    register int reg2 asm("ebx") = b;
    register int reg3 asm("ecx") = c;
    register int reg4 asm("edx") = d;
    register int reg5 asm("esi") = e;
    register int reg6 asm("edi") = f;
    
    /* Complex inline assembly that clobbers many registers */
    /* This forces spills and reloads of various types */
    asm volatile (
        "movl %[r1], %%eax\n\t"
        "addl %[r2], %%eax\n\t"
        "movl %%eax, %[r3]\n\t"
        "movl %[r4], %%ebx\n\t"
        "imull %%eax, %%ebx\n\t"
        "movl %%ebx, %[r5]\n\t"
        "leal (%%eax, %%ebx, 4), %%ecx\n\t"
        "movl %%ecx, %[r7]\n\t"
        "movl %[ptr], %%edx\n\t"
        "movl (%%edx), %%esi\n\t"
        "addl %%esi, %%ecx\n\t"
        "movl %%ecx, %[r9]"
        : [r3] "=m" (r3), [r5] "=m" (r5), [r7] "=m" (r7), [r9] "=m" (r9)
        : [r1] "r" (reg1), [r2] "r" (reg2), [r4] "r" (reg4), 
          [ptr] "r" (&g_volatile_counter)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "cc", "memory"
    );
    
    /* Another asm with alternative constraints */
    int* addr_array[4] = {&g_array1[a], &g_array1[b], &g_array1[c], &g_array1[d]};
    int result = 0;
    
    for (int i = 0; i < 4; i++) {
        int temp;
        /* asm with memory input and register output */
        asm volatile (
            "movl (%[addr]), %[out]\n\t"
            "addl %%eax, %[out]"
            : [out] "=r" (temp)
            : [addr] "r" (addr_array[i]), "a" (i)
            : "memory"
        );
        result += temp;
    }
    
    v = g_volatile_counter;
    return r3 + r5 + r7 + r9 + result + v + reg1 + reg2 + reg3 + reg4 + reg5 + reg6;
}

/* ========== TEST FUNCTION 4: Mixed Operand Types ========== */
/* Targets all remaining reload types through diverse patterns */
static int __attribute__((noinline))
test_mixed_operands(int seed) {
    /* Create many local variables with different lifetimes */
    int locals[20];
    int* ptrs[10];
    volatile int vol[5];
    
    for (int i = 0; i < 20; i++) {
        locals[i] = seed + i * 3;
    }
    
    for (int i = 0; i < 10; i++) {
        ptrs[i] = &locals[i * 2];
    }
    
    /* Complex expressions mixing different operand types */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        /* Manual unrolling */
        int idx1 = (locals[i] + locals[i+1]) % 20;
        int idx2 = (locals[i+2] * locals[i+3]) % 20;
        int idx3 = (locals[i+4] - locals[i+5]) % 20;
        
        /* Multiple memory accesses with address computations */
        sum += ptrs[idx1 % 10][0];
        sum += *(ptrs[idx2 % 10] + (idx3 % 5));
        sum += g_array1[ptrs[idx1 % 10][0] % 256];
        
        /* Volatile access forces memory barrier */
        vol[i % 5] = sum;
        
        /* Pointer arithmetic creating new addresses */
        int* new_ptr = ptrs[idx1 % 10] + (idx2 % 3);
        sum += *new_ptr;
        
        /* Nested pointer dereference */
        int** ptr_to_ptr = &ptrs[idx3 % 10];
        sum += **ptr_to_ptr;
    }
    
    /* Use __builtin_expect to create data dependencies */
    if (__builtin_expect(sum > 1000, 0)) {
        for (int i = 0; i < 5; i++) {
            sum += __builtin_popcount(locals[i]);
        }
    }
    
    return sum;
}

/* ========== MAIN DRIVER ========== */
int main(int argc, char** argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        g_array1[i] = i * 3 + 1;
        g_array2[i] = i * 5 - 2;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            g_structs[i].values[j] = i * 8 + j;
        }
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                g_structs[i].matrix[j][k] = (i * 9) + (j * 3) + k;
            }
        }
        g_structs[i].next = &g_structs[(i + 1) % 16];
    }
    
    g_volatile_ptr = &g_volatile_counter;
    
    /* Call test functions with different arguments to trigger various reloads */
    int result = 0;
    
    /* Test 1: Complex addressing patterns */
    result += test_complex_addressing(1, 2, 3, 4, 5, 6, 7, 8);
    result += test_complex_addressing(8, 7, 6, 5, 4, 3, 2, 1);
    
    /* Test 2: Structure accesses */
    for (int i = 0; i < 4; i++) {
        result += test_structure_access(i * 4);
    }
    
    /* Test 3: Assembly with clobbers */
    result += test_asm_clobbers(10, 20, 30, 40, 50, 60);
    result += test_asm_clobbers(60, 50, 40, 30, 20, 10);
    
    /* Test 4: Mixed operand types */
    for (int i = 0; i < 8; i++) {
        result += test_mixed_operands(i * 100);
    }
    
    /* Use result to prevent dead code elimination */
    g_volatile_counter = result % 1000;
    
    return result > 0 ? 0 : 1;
}
