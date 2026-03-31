/* reload_coverage.c - Test program to trigger various reload types in GCC */
#include <stdint.h>
#include <stdlib.h>

/* Force specific register usage patterns */
#define FORCE_REGISTER_PRESSURE __attribute__((noinline, optimize("O0")))

/* Complex structure to force address computations */
struct NestedData {
    int values[8];
    struct NestedData *next;
    volatile int flags;
    double coords[3];
};

/* Global arrays to prevent optimization */
volatile int global_array[256];
volatile double global_doubles[128];
volatile struct NestedData *global_list;

/* Test 1: Complex array addressing with multiple index computations */
FORCE_REGISTER_PRESSURE
static int test_complex_addressing(int a, int b, int c, int d, int e, int f) {
    /* Many local variables to consume registers */
    volatile int v1 = a * b;
    volatile int v2 = c + d;
    volatile int v3 = e ^ f;
    volatile int v4 = a | c;
    volatile int v5 = b & d;
    volatile int v6 = e << 2;
    volatile int v7 = f >> 1;
    volatile int v8 = a + b + c;
    volatile int v9 = d * e * f;
    volatile int v10 = ~a;
    
    /* Multi-dimensional array access with complex addressing */
    int arr3d[4][4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                /* Complex address computation - may need RELOAD_FOR_INPUT_ADDRESS */
                arr3d[i][j][k] = global_array[
                    (i * v1 + j * v2 + k * v3 + 
                     v4 * v5 + v6 * v7 + v8 * v9) & 0xFF
                ];
            }
        }
    }
    
    /* Pointer chasing with address reloads */
    volatile struct NestedData *ptr = global_list;
    int sum = 0;
    while (ptr) {
        /* Nested array access - may need RELOAD_FOR_INPADDR_ADDRESS */
        sum += ptr->values[
            (ptr->values[0] * v1 + ptr->values[1] * v2) & 0x7
        ];
        
        /* Complex structure member access */
        double *coord_ptr = &ptr->coords[0];
        sum += (int)(coord_ptr[0] + coord_ptr[1] + coord_ptr[2]);
        
        ptr = ptr->next;
    }
    
    return sum + arr3d[0][0][0] + v1 + v2 + v3;
}

/* Test 2: Inline assembly with multiple outputs and clobbers */
FORCE_REGISTER_PRESSURE
static int test_asm_reloads(int x, int y, int z) {
    int result1, result2, result3;
    volatile int mem1, mem2, mem3;
    
    /* Many local variables to increase register pressure */
    int a = x + 1, b = y + 2, c = z + 3;
    int d = x * y, e = y * z, f = z * x;
    int g = x ^ y, h = y ^ z, i = z ^ x;
    int j = x | y, k = y | z, l = z | x;
    int m = x & y, n = y & z, o = z & x;
    
    /* Complex memory address computation for output */
    int *out_addr1 = &mem1 + (a * b) / 256;
    int *out_addr2 = &mem2 + (c * d) / 256;
    int *out_addr3 = &mem3 + (e * f) / 256;
    
    /* Inline asm with multiple outputs to memory addresses */
    /* May trigger RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    __asm__ volatile (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "imull %[in3], %%eax\n\t"
        "movl %%eax, %[out2]\n\t"
        "subl %[in4], %%eax\n\t"
        "movl %%eax, %[out3]"
        : [out1] "=m" (*out_addr1),
          [out2] "=m" (*out_addr2),
          [out3] "=m" (*out_addr3)
        : [in1] "r" (a), [in2] "r" (b), 
          [in3] "r" (c), [in4] "r" (d)
        : "eax", "memory", "cc"
    );
    
    /* Another asm with operand address reloads */
    int *ptr1 = &a + g;
    int *ptr2 = &b + h;
    int *ptr3 = &c + i;
    
    __asm__ volatile (
        "movl (%[ptr1]), %%ebx\n\t"
        "addl (%[ptr2]), %%ebx\n\t"
        "movl %%ebx, (%[ptr3])"
        : 
        : [ptr1] "r" (ptr1), [ptr2] "r" (ptr2), [ptr3] "r" (ptr3)
        : "ebx", "memory"
    );
    
    return mem1 + mem2 + mem3 + a + b + c;
}

/* Test 3: Mixed operand types and addressing modes */
FORCE_REGISTER_PRESSURE  
static int test_mixed_operands(int base) {
    /* Create many scalar variables */
    volatile int v[20];
    for (int i = 0; i < 20; i++) {
        v[i] = base + i * 3;
    }
    
    /* Complex expressions with multiple uses of same value */
    int *addr_array[10];
    for (int i = 0; i < 10; i++) {
        /* Address computation that may need reloading */
        addr_array[i] = &v[i] + (v[i+5] & 0xF);
    }
    
    int sum = 0;
    
    /* Manual loop unrolling to increase register pressure */
    /* Each iteration uses different addressing patterns */
    
    /* Iteration 1: Direct array access */
    sum += v[0] * v[1];
    sum += v[2] / (v[3] + 1);
    
    /* Iteration 2: Pointer dereference */
    int *p1 = addr_array[0];
    int *p2 = addr_array[1];
    sum += *p1 + *p2;
    
    /* Iteration 3: Complex pointer arithmetic */
    sum += *(p1 + (v[4] & 0x3)) + *(p2 + (v[5] & 0x3));
    
    /* Iteration 4: More complex addressing */
    sum += addr_array[2][v[6] & 0x3] + addr_array[3][v[7] & 0x3];
    
    /* Use __builtin_expect to create data dependencies */
    if (__builtin_expect(sum > 1000, 0)) {
        for (int i = 0; i < 5; i++) {
            sum += v[10 + i] * v[15 + i];
        }
    }
    
    /* Volatile accesses to force memory operations */
    volatile int *volatile vptr = &v[0];
    for (int i = 0; i < 5; i++) {
        sum += *(vptr + i);
    }
    
    return sum;
}

/* Test 4: Structure access with nested addressing */
FORCE_REGISTER_PRESSURE
static int test_structure_addressing(void) {
    struct ComplexStruct {
        int data[8];
        struct {
            int x;
            int y;
            int z;
        } points[4];
        volatile int status;
        double matrix[2][2];
    };
    
    /* Local structure with many members */
    volatile struct ComplexStruct s1, s2, s3;
    
    /* Initialize with complex expressions */
    for (int i = 0; i < 8; i++) {
        s1.data[i] = i * 2 + 1;
        s2.data[i] = i * 3 + 2;
        s3.data[i] = i * 5 + 3;
    }
    
    int result = 0;
    
    /* Nested structure member access */
    for (int i = 0; i < 4; i++) {
        /* Complex addressing: s.points[i].x where i is in register */
        result += s1.points[i].x + s2.points[i].y + s3.points[i].z;
        
        /* Even more complex: array index based on previous result */
        int idx = result & 0x3;
        result += s1.data[idx] * s2.data[idx] - s3.data[idx];
    }
    
    /* Matrix operations with double types */
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            /* Mix float and int operations */
            result += (int)(s1.matrix[i][j] * 100.0);
            result += (int)(s2.matrix[j][i] * 50.0);
        }
    }
    
    /* Pointer to member with offset */
    volatile int *status_ptr = &s1.status;
    for (int i = 0; i < 4; i++) {
        result += *(status_ptr + i);
    }
    
    return result;
}

/* Test 5: Function with inline asm using multiple alternative constraints */
FORCE_REGISTER_PRESSURE
static int test_asm_constraints(int a, int b, int c, int d) {
    int out1, out2, out3;
    volatile int mem_out;
    
    /* Inline asm with multiple alternative constraints */
    /* May trigger RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
    __asm__ volatile (
        "# Complex asm with alternatives\n\t"
        "movl %[in1], %[out1]\n\t"
        "leal (%[in2], %[in3], 2), %[out2]\n\t"
        "addl %[out2], %[out1]\n\t"
        "movl %[out1], %[mem]"
        : [out1] "=&r" (out1), [out2] "=&r" (out2), [mem] "=m" (mem_out)
        : [in1] "rm" (a),  /* register or memory */
          [in2] "r" (b), 
          [in3] "rm" (c)   /* register or memory */
        : "cc"
    );
    
    /* Another asm with output to computed address */
    int *dest = &mem_out + (d & 0xF);
    __asm__ volatile (
        "movl %%eax, %[dest]"
        : [dest] "=m" (*dest)
        : "a" (out1 + out2)
        : "memory"
    );
    
    return out1 + out2 + mem_out;
}

/* Main driver that calls all tests */
int main(void) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    /* Create a linked list for testing */
    struct NestedData nodes[5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 8; j++) {
            nodes[i].values[j] = i * 8 + j;
        }
        nodes[i].next = (i < 4) ? &nodes[i+1] : NULL;
        nodes[i].flags = i * 100;
        for (int j = 0; j < 3; j++) {
            nodes[i].coords[j] = (i + j) * 1.5;
        }
    }
    global_list = &nodes[0];
    
    int result = 0;
    
    /* Call each test multiple times with different arguments */
    for (int i = 0; i < 3; i++) {
        result += test_complex_addressing(i, i+1, i+2, i+3, i+4, i+5);
        result += test_asm_reloads(i*10, i*20, i*30);
        result += test_mixed_operands(i*100);
        result += test_structure_addressing();
        result += test_asm_constraints(i, i*2, i*3, i*4);
    }
    
    /* Use result to prevent dead code elimination */
    volatile int final_result = result;
    
    return final_result > 0 ? 0 : 1;
}
