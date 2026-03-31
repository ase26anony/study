/* reload_coverage.c - Program to trigger various reload types in GCC reload1.cc */
#include <stdint.h>
#include <stdlib.h>

/* Force specific architecture for register scarcity */
#ifdef __x86_64__
#error "Compile with -m32 flag for 32-bit target"
#endif

/* Volatile variables to force memory accesses */
static volatile int vol_global = 42;
static volatile int vol_array[100];

/* Complex structure for address computations */
struct Nested {
    int data[4][4];
    struct Nested *next;
    int offsets[8];
};

/* Test function 1: Complex array addressing with multiple index computations */
__attribute__((noinline))
static int test_complex_addressing(int a, int b, int c, int d, int e, int f) {
    /* Many local variables to consume registers */
    int i1 = a + 1, i2 = b + 2, i3 = c + 3, i4 = d + 4, i5 = e + 5, i6 = f + 6;
    int j1 = a * 2, j2 = b * 3, j3 = c * 4, j4 = d * 5, j5 = e * 6, j6 = f * 7;
    int k1 = a ^ b, k2 = b ^ c, k3 = c ^ d, k4 = d ^ e, k5 = e ^ f, k6 = f ^ a;
    
    /* Multi-dimensional arrays with volatile accesses */
    volatile int arr3d[3][3][3];
    volatile int arr2d[8][8];
    
    /* Complex address computations - should trigger RELOAD_FOR_INPUT_ADDRESS */
    int sum = 0;
    
    /* Manual loop unrolling for register pressure */
    /* Each iteration uses different addressing modes */
    sum += arr3d[i1][j1][k1];  /* Base + 3 indices */
    sum += arr3d[i2][j2][k2];
    sum += arr3d[i3][j3][k3];
    sum += arr3d[i4][j4][k4];
    
    /* Nested addressing with pointer arithmetic */
    sum += *(*(arr2d + i5) + j5);  /* RELOAD_FOR_INPADDR_ADDRESS */
    sum += *(*(arr2d + i6) + j6);
    
    /* More complex: address of address computation */
    int *ptr1 = &arr2d[i1][j1];
    int *ptr2 = &arr2d[i2][j2];
    sum += *ptr1 + *ptr2;  /* RELOAD_FOR_OPERAND_ADDRESS */
    
    /* Force spills with many live values */
    vol_array[0] = i1 + i2 + i3 + i4 + i5 + i6;
    vol_array[1] = j1 + j2 + j3 + j4 + j5 + j6;
    vol_array[2] = k1 + k2 + k3 + k4 + k5 + k6;
    
    return sum;
}

/* Test function 2: Structure access with pointer chasing */
__attribute__((noinline))
static int test_structure_access(struct Nested *n1, struct Nested *n2, 
                                 struct Nested *n3, struct Nested *n4) {
    /* Many structure pointers to consume registers */
    struct Nested *p1 = n1, *p2 = n2, *p3 = n3, *p4 = n4;
    volatile int sum = 0;
    
    /* Complex structure member access with offset computations */
    /* This should trigger RELOAD_FOR_OUTPUT_ADDRESS and others */
    for (int i = 0; i < 4; i++) {
        #pragma GCC unroll 4
        for (int j = 0; j < 4; j++) {
            /* Multiple base registers needed for different structures */
            sum += p1->data[i][j];
            sum += p2->data[j][i];  /* Swapped indices */
            sum += p3->data[i][i];
            sum += p4->data[j][j];
            
            /* Address of structure member with offset */
            int *addr1 = &p1->data[i][j];
            int *addr2 = &p2->data[j][i];
            sum += *addr1 * *addr2;  /* RELOAD_FOR_OPADDR_ADDR */
        }
    }
    
    /* Pointer chasing with address reloads */
    p1 = p1->next;
    p2 = p2->next;
    sum += p1->offsets[0] + p2->offsets[1];
    
    return sum;
}

/* Test function 3: Inline assembly with multiple outputs and clobbers */
__attribute__((noinline))
static int test_inline_asm(int a, int b, int c, int d, int e, int f, 
                           int g, int h, int i, int j) {
    /* Many input values in registers */
    int out1, out2, out3, out4, out5;
    volatile int mem1 = a, mem2 = b, mem3 = c, mem4 = d;
    
    /* Complex inline assembly that clobbers many registers */
    /* This should trigger RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS */
    asm volatile (
        /* Multiple output operands */
        "movl %[in1], %[out1]\n\t"
        "addl %[in2], %[out1]\n\t"
        "movl %[in3], %[out2]\n\t"
        "imull %[in4], %[out2]\n\t"
        /* Memory operand with complex address */
        "movl %[mem1], %%eax\n\t"
        "addl (%%eax, %[in5], 4), %[out3]\n\t"  /* RELOAD_FOR_INPUT_ADDRESS */
        /* Another with different addressing */
        "movl %[mem2], %%ebx\n\t"
        "leal (%%ebx, %[in6], 2), %%ecx\n\t"
        "movl (%%ecx), %[out4]\n\t"  /* RELOAD_FOR_INPADDR_ADDRESS */
        /* Output to memory address */
        "movl %[out5], (%[mem3], %[in7], 1)\n\t"  /* RELOAD_FOR_OUTPUT_ADDRESS */
        /* Clobber many registers to force reloads */
        :
        [out1] "=r" (out1),
        [out2] "=r" (out2),
        [out3] "=r" (out3),
        [out4] "=r" (out4),
        [out5] "=r" (out5)
        :
        [in1] "r" (a),
        [in2] "r" (b),
        [in3] "r" (c),
        [in4] "r" (d),
        [in5] "r" (e),
        [in6] "r" (f),
        [in7] "r" (g),
        [mem1] "m" (mem1),
        [mem2] "m" (mem2),
        [mem3] "m" (mem3)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* Use all outputs to prevent dead code elimination */
    return out1 + out2 + out3 + out4 + out5 + mem4;
}

/* Test function 4: Mixed operand types with __builtin_expect */
__attribute__((noinline))
static int test_mixed_operands(int *base, int index1, int index2, 
                               int offset1, int offset2, int offset3) {
    /* Many intermediate values */
    int t1 = __builtin_expect(base[index1], 0);
    int t2 = __builtin_expect(base[index2], 1);
    int t3 = offset1 * 2;
    int t4 = offset2 * 3;
    int t5 = offset3 * 4;
    int t6 = t1 + t2;
    int t7 = t3 + t4;
    int t8 = t5 + t6;
    int t9 = t7 + t8;
    int t10 = t9 * 2;
    
    /* Complex addressing with multiple base registers */
    int *ptr1 = base + offset1;
    int *ptr2 = base + offset2;
    int *ptr3 = base + offset3;
    
    /* Chain of address computations */
    int val1 = *(ptr1 + t1);
    int val2 = *(ptr2 + t2);
    int val3 = *(ptr3 + t3);
    
    /* Address of address computation */
    int **ptr_to_ptr = &ptr1;
    int val4 = **ptr_to_ptr;  /* RELOAD_FOR_OPERAND_ADDRESS */
    
    /* More complex: computed address stored, then used */
    int *computed_addr = &base[t4 + t5];
    int val5 = *computed_addr;
    
    /* Force all values to be used */
    vol_global = t10;
    return val1 + val2 + val3 + val4 + val5;
}

/* Test function 5: Extreme register pressure with all reload types */
__attribute__((noinline))
static int test_extreme_pressure(void) {
    /* Maximum local variables */
    int v01 = 1, v02 = 2, v03 = 3, v04 = 4, v05 = 5, v06 = 6, v07 = 7, v08 = 8;
    int v09 = 9, v10 = 10, v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    volatile int arr[20];
    volatile int *parr = arr;
    
    /* Use all variables in complex expressions */
    /* This should trigger all reload types */
    
    /* RELOAD_FOR_INPUT: values used multiple times after clobbering */
    int sum = v01 + v02;
    sum += v01 * v03;  /* v01 used again */
    
    /* Array with complex indexing - multiple address reloads */
    sum += arr[v01 + v02];
    sum += arr[v03 + v04];
    sum += arr[v05 + v06];
    sum += arr[v07 + v08];
    sum += arr[v09 + v10];
    
    /* Pointer arithmetic with multiple bases */
    sum += *(parr + v11);
    sum += *(parr + v12 + v13);
    
    /* Nested addressing */
    int idx1 = v14 + v15;
    int idx2 = v16 + v17;
    sum += *(parr + idx1 * 2 + idx2);
    
    /* Address of array element stored, then used */
    int *addr1 = &arr[v18];
    int *addr2 = &arr[v19];
    sum += *addr1 + *addr2;  /* RELOAD_FOR_OPERAND_ADDRESS */
    
    /* Complex inline asm snippet */
    int out;
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out]\n\t"
        : [out] "=r" (out)
        : [in1] "r" (v20), [in2] "r" (sum)
        : "eax"
    );
    
    return out;
}

/* Main driver function */
int main(void) {
    int total = 0;
    
    /* Initialize test data */
    struct Nested n1, n2, n3, n4;
    int base_array[100];
    
    /* Simple initialization */
    for (int i = 0; i < 100; i++) {
        base_array[i] = i;
        vol_array[i] = i * 2;
    }
    
    n1.next = &n2;
    n2.next = &n3;
    n3.next = &n4;
    n4.next = &n1;
    
    /* Call all test functions with many arguments */
    total += test_complex_addressing(1, 2, 3, 4, 5, 6);
    total += test_structure_access(&n1, &n2, &n3, &n4);
    total += test_inline_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    total += test_mixed_operands(base_array, 10, 20, 5, 15, 25);
    total += test_extreme_pressure();
    
    /* Use result to prevent optimization */
    vol_global = total;
    
    return total > 0 ? 0 : 1;
}
