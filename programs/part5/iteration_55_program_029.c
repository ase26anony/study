/* reload_coverage.c - Test program to trigger various reload types in GCC */
#include <stdint.h>
#include <stdio.h>

/* Force specific register usage and prevent optimizations */
#define NOOPT __attribute__((optimize("O0")))
#define NOINLINE __attribute__((noinline))
#define VOLATILE_MEM volatile

/* Complex structure to force address computations */
struct NestedData {
    int values[8];
    struct NestedData *next;
    int matrix[3][3];
    volatile int sync;
};

/* Global arrays to create complex addressing patterns */
static int global_array[256];
static struct NestedData data_pool[16];
static volatile int memory_barrier;

/* Function 1: Complex array addressing with multiple index computations */
NOINLINE NOOPT
static int test_complex_addressing(int a, int b, int c, int d, int e, int f) {
    /* Many local variables to create register pressure */
    int i1 = a + 1, i2 = b + 2, i3 = c + 3, i4 = d + 4, i5 = e + 5, i6 = f + 6;
    int j1 = a * 2, j2 = b * 3, j3 = c * 4, j4 = d * 5, j5 = e * 6, j6 = f * 7;
    int k1 = a ^ b, k2 = b ^ c, k3 = c ^ d, k4 = d ^ e, k5 = e ^ f, k6 = f ^ a;
    
    /* Small arrays to force spills */
    int arr1[8], arr2[8], arr3[8];
    
    /* Complex array indexing - forces RELOAD_FOR_INPUT_ADDRESS */
    for (int i = 0; i < 8; i++) {
        /* Multi-level computation for array indices */
        arr1[i] = global_array[(i1 + i2 * i + i3) & 0xFF];
        arr2[i] = global_array[(j1 + j2 * i + j3) & 0xFF];
        arr3[i] = global_array[(k1 + k2 * i + k3) & 0xFF];
    }
    
    /* Nested array access with complex addressing - forces RELOAD_FOR_INPADDR_ADDRESS */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            /* Complex address computation that may need reloading */
            sum += arr1[(i * j + i1) & 7] * arr2[(i + j * j2) & 7];
            sum += arr3[(i1 * i + j2 * j) & 7];
        }
    }
    
    /* Volatile access to force memory operations */
    memory_barrier = sum;
    
    return sum + i1 + i2 + i3 + i4 + i5 + i6;
}

/* Function 2: Structure access with pointer chasing and inline asm */
NOINLINE NOOPT
static int test_structure_reloads(int seed) {
    struct NestedData local_data[4];
    struct NestedData *ptr_array[8];
    int temp_values[12];
    
    /* Initialize structure pointers with complex computations */
    for (int i = 0; i < 4; i++) {
        local_data[i].next = &local_data[(i + 1) & 3];
        for (int j = 0; j < 8; j++) {
            local_data[i].values[j] = seed + i * 8 + j;
        }
        for (int x = 0; x < 3; x++) {
            for (int y = 0; y < 3; y++) {
                local_data[i].matrix[x][y] = seed * (x + 1) + (y + 1) * i;
            }
        }
        local_data[i].sync = i;
    }
    
    /* Pointer chasing with complex addressing - forces RELOAD_FOR_OPERAND_ADDRESS */
    int result = 0;
    struct NestedData *current = &local_data[0];
    for (int i = 0; i < 16; i++) {
        /* Access through pointer with offset - may need address reload */
        result += current->values[i & 7];
        result += current->matrix[(i >> 1) & 1][(i >> 2) & 1];
        
        /* Complex pointer arithmetic */
        current = (struct NestedData *)((char *)current->next + 
                 (current->values[i & 7] & 0x3));
    }
    
    /* Inline assembly with multiple outputs and memory constraints */
    /* This can trigger RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    int out1, out2, out3;
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "imull %[in3], %%eax\n\t"
        "movl %%eax, %[out2]\n\t"
        "leal (%[in4], %[in5], 2), %%ecx\n\t"
        "addl %%ecx, %%eax\n\t"
        "movl %%eax, %[out3]\n\t"
        : [out1] "=m" (temp_values[0]),  /* Memory output - may need address reload */
          [out2] "=m" (temp_values[1]),
          [out3] "=m" (temp_values[2])
        : [in1] "r" (result),
          [in2] "r" (seed),
          [in3] "r" (result ^ seed),
          [in4] "r" (result >> 4),
          [in5] "r" (seed << 2)
        : "eax", "ecx", "memory"
    );
    
    return result + temp_values[0] + temp_values[1] + temp_values[2];
}

/* Function 3: Mixed operations with manual loop unrolling */
NOINLINE NOOPT
static int test_mixed_reload_types(int base) {
    /* Many scalar variables to consume registers */
    int v1 = base, v2 = base * 2, v3 = base * 3, v4 = base * 4;
    int v5 = base * 5, v6 = base * 6, v7 = base * 7, v8 = base * 8;
    int v9 = base * 9, v10 = base * 10, v11 = base * 11, v12 = base * 12;
    int v13 = base * 13, v14 = base * 14, v15 = base * 15, v16 = base * 16;
    
    /* Array with complex indexing patterns */
    int buffer[32];
    
    /* Manual loop unrolling to increase register pressure */
    /* Each iteration uses different addressing modes */
    
    /* Pattern 1: Direct array access with complex index */
    buffer[(v1 + v2) & 31] = v1 + v3;
    buffer[(v2 + v3) & 31] = v2 + v4;
    buffer[(v3 + v4) & 31] = v3 + v5;
    buffer[(v4 + v5) & 31] = v4 + v6;
    
    /* Pattern 2: Pointer arithmetic with multiple bases */
    int *ptr1 = &buffer[(v5 + v6) & 31];
    int *ptr2 = &buffer[(v6 + v7) & 31];
    int *ptr3 = &buffer[(v7 + v8) & 31];
    
    *ptr1 = v5 * v6;
    *ptr2 = v6 * v7;
    *ptr3 = v7 * v8;
    
    /* Pattern 3: Nested address computation */
    /* This should trigger RELOAD_FOR_OTHER_ADDRESS */
    int idx1 = (v8 + v9) & 31;
    int idx2 = (v9 + v10) & 31;
    int idx3 = (v10 + v11) & 31;
    
    buffer[buffer[idx1] & 31] = v8 + v9;
    buffer[buffer[idx2] & 31] = v9 + v10;
    buffer[buffer[idx3] & 31] = v10 + v11;
    
    /* Complex expression with many operands - forces RELOAD_FOR_INPUT */
    int complex_result = 
        (v1 * v2) + (v3 * v4) - (v5 * v6) + (v7 * v8) -
        (v9 * v10) + (v11 * v12) - (v13 * v14) + (v15 * v16);
    
    /* Mix with array accesses */
    for (int i = 0; i < 8; i++) {
        complex_result += buffer[(complex_result + i) & 31];
        complex_result -= buffer[(complex_result - i) & 31];
    }
    
    /* Another inline asm with multiple memory inputs/outputs */
    /* Can trigger RELOAD_FOR_OPADDR_ADDR */
    int final_result;
    asm volatile (
        "movl %[addr1], %%esi\n\t"
        "movl (%%esi), %%eax\n\t"
        "addl %[val1], %%eax\n\t"
        "movl %[addr2], %%edi\n\t"
        "addl (%%edi), %%eax\n\t"
        "movl %%eax, %[result]\n\t"
        : [result] "=m" (final_result)
        : [addr1] "r" (&buffer[0]),
          [val1] "r" (complex_result),
          [addr2] "r" (&buffer[16])
        : "eax", "esi", "edi", "memory"
    );
    
    return final_result;
}

/* Function 4: Extreme register pressure with all reload types */
NOINLINE NOOPT
static int test_extreme_pressure(int p1, int p2, int p3, int p4, int p5, int p6) {
    /* Maximum number of local variables */
    int r0 = p1, r1 = p2, r2 = p3, r3 = p4, r4 = p5, r5 = p6;
    int r6 = p1 * p2, r7 = p2 * p3, r8 = p3 * p4, r9 = p4 * p5, r10 = p5 * p6;
    int r11 = p1 + p6, r12 = p2 + p5, r13 = p3 + p4, r14 = p4 + p3, r15 = p5 + p2;
    int s0 = r0 ^ r1, s1 = r1 ^ r2, s2 = r2 ^ r3, s3 = r3 ^ r4, s4 = r4 ^ r5;
    int s5 = r5 ^ r6, s6 = r6 ^ r7, s7 = r7 ^ r8, s8 = r8 ^ r9, s9 = r9 ^ r10;
    
    /* Arrays for additional pressure */
    VOLATILE_MEM int mem1[8], mem2[8], mem3[8];
    
    /* Complex expressions using all variables */
    for (int i = 0; i < 8; i++) {
        /* Each array access uses different complex addressing */
        mem1[i] = r0 + r1 * i + r2 * (i * i);
        mem2[i] = r3 + r4 * i + r5 * (i * i);
        mem3[i] = r6 + r7 * i + r8 * (i * i);
    }
    
    /* Mixed operations designed to use all reload types */
    int total = 0;
    
    /* RELOAD_FOR_INPUT patterns */
    total += (r0 * r1) + (r2 * r3) - (r4 * r5) + (r6 * r7);
    
    /* RELOAD_FOR_INPUT_ADDRESS patterns */
    for (int i = 0; i < 4; i++) {
        total += mem1[mem2[i & 7] & 7] * mem2[mem3[i & 7] & 7];
    }
    
    /* RELOAD_FOR_OPERAND_ADDRESS patterns */
    int *ptr_array[] = {&mem1[0], &mem2[0], &mem3[0], &total};
    for (int i = 0; i < 4; i++) {
        *ptr_array[i] += ptr_array[(i + 1) & 3] - ptr_array[i];
    }
    
    /* Final complex computation */
    total = total + s0 - s1 + s2 - s3 + s4 - s5 + s6 - s7 + s8 - s9;
    
    /* Force all values to be used */
    memory_barrier = total;
    
    return total + r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
}

/* Main driver function */
int main(void) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    for (int i = 0; i < 16; i++) {
        data_pool[i].sync = i * 2;
        data_pool[i].next = &data_pool[(i + 1) & 15];
    }
    
    int result = 0;
    
    /* Call all test functions with different arguments */
    result += test_complex_addressing(1, 2, 3, 4, 5, 6);
    result += test_structure_reloads(42);
    result += test_mixed_reload_types(100);
    result += test_extreme_pressure(7, 8, 9, 10, 11, 12);
    
    /* Add more calls with different patterns */
    result += test_complex_addressing(10, 20, 30, 40, 50, 60);
    result += test_structure_reloads(123);
    result += test_mixed_reload_types(200);
    result += test_extreme_pressure(13, 14, 15, 16, 17, 18);
    
    printf("Final result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
