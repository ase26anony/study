/* reload_coverage.c - Program to trigger various reload types in GCC reload1.cc */
#include <stdint.h>
#include <stdlib.h>

/* Force specific architecture for register scarcity */
#ifdef __x86_64__
#undef __x86_64__
#endif

/* Disable optimizations that might reduce reloads */
#pragma GCC optimize ("O0")
#pragma GCC push_options

/* Volatile variables to force memory accesses */
static volatile int global_counter = 0;
static volatile int global_array[256];

/* Complex structure to force address computations */
struct NestedData {
    int values[8];
    struct NestedData *next;
    short offsets[4];
    volatile long *ptr_array[3];
};

/* Function 1: Focus on RELOAD_FOR_INPUT and RELOAD_FOR_INPUT_ADDRESS */
static __attribute__((noinline)) 
int test_input_reloads(struct NestedData *data, int idx1, int idx2, int idx3) {
    /* Many local variables to exhaust registers */
    int a = idx1 * 2;
    int b = idx2 + global_counter;
    int c = idx3 ^ 0x55AA;
    int d = a + b;
    int e = c - d;
    int f = global_array[a & 0xFF];
    int g = b * c;
    int h = d ^ e;
    int i = f + g;
    int j = h - i;
    int k = a * b * c;
    int l = d + e + f;
    int m = g ^ h ^ i;
    int n = j + k + l;
    int o = m - n;
    int p = o * 2;
    int q = p + 1;
    int r = q ^ 0xFF;
    int s = r << 2;
    int t = s >> 1;
    
    /* Complex array addressing - forces input address reloads */
    int result = data->values[(a + b) & 7] + 
                 data->values[(c + d) & 7] * 
                 data->values[(e + f) & 7] -
                 data->values[(g + h) & 7] /
                 (data->values[(i + j) & 7] + 1);
    
    /* More computations using all variables */
    result += (a * b) + (c * d) - (e * f) + (g * h) - (i * j) + 
              (k * l) - (m * n) + (o * p) - (q * r) + (s * t);
    
    /* Volatile access forces spill/reload */
    global_counter = result;
    
    return result;
}

/* Function 2: Focus on RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
static __attribute__((noinline))
void test_output_address_reloads(int *out1, int *out2, int *out3, 
                                 int *out4, int *out5, int *out6) {
    /* Complex output addressing with inline assembly */
    int temp1, temp2, temp3, temp4, temp5, temp6;
    
    /* Inline asm with multiple outputs and memory constraints */
    asm volatile (
        "movl $1, %0\n\t"
        "movl $2, %1\n\t"
        "movl $3, %2\n\t"
        "movl $4, %3\n\t"
        "movl $5, %4\n\t"
        "movl $6, %5\n\t"
        : "=r"(temp1), "=r"(temp2), "=r"(temp3), 
          "=r"(temp4), "=r"(temp5), "=r"(temp6)
        :
        : "eax", "ebx", "ecx", "edx", "esi", "edi"
    );
    
    /* Complex address computations for outputs */
    *(out1 + temp1) = temp1 * 2;
    *(out2 + (temp2 & 0xF)) = temp2 + global_counter;
    *(out3 + (temp3 >> 2)) = temp3 ^ 0x1234;
    *(out4 + (temp4 * 3)) = temp4 - temp1;
    *(out5 + (temp5 / 2)) = temp5 | temp2;
    *(out6 + (temp6 % 16)) = temp6 & temp3;
    
    /* Nested pointer access */
    int **ptr_ptr = &out1;
    ***((int ***)&ptr_ptr + (temp1 & 1)) = temp1 + temp2;
}

/* Function 3: Focus on RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
static __attribute__((noinline))
int test_operand_address_reloads(struct NestedData **data_array, 
                                 int count, int offset) {
    /* Many pointer variables to force address reloads */
    struct NestedData *ptr1 = data_array[0];
    struct NestedData *ptr2 = data_array[1];
    struct NestedData *ptr3 = data_array[2];
    struct NestedData *ptr4 = data_array[3];
    struct NestedData *ptr5 = data_array[4];
    struct NestedData *ptr6 = data_array[5];
    struct NestedData *ptr7 = data_array[6];
    struct NestedData *ptr8 = data_array[7];
    
    int sum = 0;
    
    /* Manual loop unrolling for maximum register pressure */
    /* Each iteration uses different addressing modes */
    
    /* Iteration 1: Direct structure access */
    sum += ptr1->values[(offset + 0) & 7];
    sum += ptr1->offsets[(offset + 1) & 3];
    
    /* Iteration 2: Pointer arithmetic */
    sum += (ptr2 + (offset & 1))->values[(offset + 2) & 7];
    
    /* Iteration 3: Double indirection */
    sum += *(ptr3->ptr_array[0] + (offset & 3));
    
    /* Iteration 4: Triple computation */
    sum += ptr4->values[0] + ptr4->values[1] - ptr4->values[2];
    
    /* Iteration 5: Complex expression */
    sum += (ptr5->next ? ptr5->next->values[0] : 0) * 
           (ptr6->next ? ptr6->next->values[1] : 1);
    
    /* Iteration 6: Volatile pointer access */
    volatile int *volatile_ptr = (volatile int *)&ptr7->values[0];
    sum += *volatile_ptr + *(volatile_ptr + 1);
    
    /* Iteration 7: Address of address computation */
    int *addr1 = &ptr8->values[0];
    int *addr2 = &ptr8->values[4];
    sum += *addr1 * *addr2;
    
    /* Iteration 8: More complex addressing */
    sum += *(int *)((char *)ptr1 + offset * sizeof(int));
    
    return sum;
}

/* Function 4: Focus on RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS */
static __attribute__((noinline))
int test_other_reload_types(int param1, int param2, int param3, 
                           int param4, int param5, int param6,
                           int param7, int param8, int param9,
                           int param10, int param11, int param12) {
    /* Use all parameters in complex ways */
    int array[16];
    
    /* Initialize array with parameter combinations */
    for (int i = 0; i < 16; i++) {
        array[i] = i + param1 + param2 - param3 * param4;
    }
    
    /* Complex expression with many temporaries */
    int t1 = param1 * param2;
    int t2 = param3 + param4;
    int t3 = param5 ^ param6;
    int t4 = param7 & param8;
    int t5 = param9 | param10;
    int t6 = param11 << param12;
    int t7 = t1 + t2;
    int t8 = t3 - t4;
    int t9 = t5 * t6;
    int t10 = t7 ^ t8;
    int t11 = t9 + t10;
    int t12 = t11 >> 2;
    int t13 = t12 * 3;
    int t14 = t13 & 0xFF;
    int t15 = t14 | 0x80;
    int t16 = t15 ^ 0x55;
    
    /* Mixed array and scalar computations */
    int result = 0;
    result += array[t1 & 0xF] * array[t2 & 0xF];
    result -= array[t3 & 0xF] / (array[t4 & 0xF] + 1);
    result += array[t5 & 0xF] ^ array[t6 & 0xF];
    result |= array[t7 & 0xF] & array[t8 & 0xF];
    
    /* Inline asm with multiple alternative constraints */
    asm volatile (
        "addl %1, %0\n\t"
        "subl %2, %0\n\t"
        "imull %3, %0\n\t"
        : "+r"(result)
        : "r"(t9), "r"(t10), "r"(t11)
        : "cc"
    );
    
    /* More complex operations */
    result = (result * t12) + (t13 / t14) - (t15 ^ t16);
    
    /* Force spill with volatile */
    volatile int vol = result;
    result = vol + param1;
    
    return result;
}

/* Function 5: Mixed reload types with loop unrolling */
#pragma GCC unroll 4
static __attribute__((noinline))
int test_mixed_reloads_in_loop(int iterations, struct NestedData *data) {
    int sum = 0;
    
    /* Manually unrolled loop with different patterns each iteration */
    for (int i = 0; i < iterations; i++) {
        /* Pattern 1: Input reloads */
        int a = i * 2;
        int b = i + 1;
        int c = a * b;
        sum += data->values[a & 7] + data->values[b & 7] * c;
        
        /* Pattern 2: Input address reloads */
        sum += *(data->ptr_array[0] + (a & 3)) - 
               *(data->ptr_array[1] + (b & 3));
        
        /* Pattern 3: Output address reloads */
        int *ptr = &data->values[c & 7];
        *ptr = sum;
        
        /* Pattern 4: Operand address reloads */
        int **ptr_to_ptr = &ptr;
        sum += **ptr_to_ptr;
        
        /* Pattern 5: Other address reloads */
        volatile int *vol_ptr = (volatile int *)ptr;
        sum += *vol_ptr;
    }
    
    return sum;
}

/* Main driver function */
int main(void) {
    /* Initialize test data */
    struct NestedData data_array[8];
    struct NestedData *ptr_array[8];
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            data_array[i].values[j] = i * 10 + j;
        }
        for (int j = 0; j < 4; j++) {
            data_array[i].offsets[j] = (short)(i * 5 + j);
        }
        for (int j = 0; j < 3; j++) {
            data_array[i].ptr_array[j] = (volatile long *)&data_array[i].values[j];
        }
        data_array[i].next = (i < 7) ? &data_array[i + 1] : NULL;
        ptr_array[i] = &data_array[i];
    }
    
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    int output_array[64];
    int result = 0;
    
    /* Call all test functions to trigger different reload types */
    result += test_input_reloads(&data_array[0], 1, 2, 3);
    result += test_input_reloads(&data_array[1], 4, 5, 6);
    
    test_output_address_reloads(&output_array[0], &output_array[8], 
                               &output_array[16], &output_array[24],
                               &output_array[32], &output_array[40]);
    
    result += test_operand_address_reloads(ptr_array, 8, 2);
    result += test_operand_address_reloads(ptr_array, 8, 4);
    
    result += test_other_reload_types(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
    result += test_other_reload_types(13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24);
    
    result += test_mixed_reloads_in_loop(4, &data_array[2]);
    result += test_mixed_reloads_in_loop(8, &data_array[3]);
    
    /* Use results to prevent dead code elimination */
    for (int i = 0; i < 64; i++) {
        result += output_array[i];
    }
    
    return result & 0xFF; /* Return non-zero to indicate execution */
}

#pragma GCC pop_options
