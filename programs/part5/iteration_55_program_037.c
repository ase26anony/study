/* reload_coverage.c - Test program to trigger various reload types in GCC reload1.cc */

#include <stdint.h>
#include <stdlib.h>

/* Force frame pointer usage and prevent optimizations */
#pragma GCC optimize ("O0")
#pragma GCC push_options

/* Structure to create complex addressing modes */
struct NestedData {
    int values[8];
    struct NestedData *next;
    volatile int volatile_field;
    int padding[3];
};

/* Global arrays to create register pressure through complex addressing */
static int global_array_3d[4][5][6];
static volatile int volatile_globals[100];
static struct NestedData data_chain[10];

/* Initialize data to prevent dead code elimination */
__attribute__((constructor)) 
static void init_data(void) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 5; j++)
            for (int k = 0; k < 6; k++)
                global_array_3d[i][j][k] = i * 100 + j * 10 + k;
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 8; j++)
            data_chain[i].values[j] = i * 10 + j;
        data_chain[i].next = (i < 9) ? &data_chain[i+1] : NULL;
        data_chain[i].volatile_field = i * 2;
    }
}

/* Test 1: Complex array addressing - triggers RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS */
__attribute__((noinline))
static int test_complex_addressing(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Many local variables to consume registers */
    int local1 = a + b;
    int local2 = c * d;
    int local3 = e - f;
    int local4 = g / (h + 1);
    int local5 = a * c;
    int local6 = b * d;
    int local7 = e * g;
    int local8 = f * h;
    int local9 = local1 + local2;
    int local10 = local3 + local4;
    int local11 = local5 + local6;
    int local12 = local7 + local8;
    
    /* Complex 3D array addressing - forces address computation reloads */
    int sum = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 4; k++) {
                /* Nested addressing: global_array_3d[i+local1][j+local2][k+local3] */
                sum += global_array_3d[i + local1 % 2][j + local2 % 3][k + local3 % 4];
                /* More complex: *(base + index1*stride1 + index2*stride2) */
                sum += *(*(global_array_3d[i] + j) + k);
            }
        }
    }
    
    /* Volatile accesses force memory operations */
    volatile_globals[local1 % 50] = sum;
    volatile_globals[local2 % 50] = sum * 2;
    
    return sum + local9 + local10 + local11 + local12;
}

/* Test 2: Structure pointer chasing with inline asm - triggers RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS */
__attribute__((noinline))
static int test_structure_asm(int seed) {
    struct NestedData *ptr = &data_chain[0];
    int results[8];
    
    /* Many local variables for register pressure */
    int temp1 = seed * 2;
    int temp2 = seed + 17;
    int temp3 = seed - 5;
    int temp4 = seed * 3;
    int temp5 = seed / 2;
    int temp6 = seed + 23;
    int temp7 = seed * 7;
    int temp8 = seed - 11;
    
    /* Inline asm with multiple outputs and complex addressing */
    for (int i = 0; i < 8; i++) {
        int idx = (temp1 + i) % 8;
        int offset = temp2 * i;
        
        /* Inline assembly with memory output operand using complex address */
        __asm__ volatile (
            "movl %[val], (%[addr]) \n\t"
            "addl $1, %[val] \n\t"
            : [val] "+r" (temp1), "=m" (*(int*)((char*)ptr + offset))
            : [addr] "r" ((char*)ptr + offset)
            : "memory"
        );
        
        /* Structure member access with pointer arithmetic */
        results[i] = ptr->values[idx] + ptr->volatile_field;
        
        /* Pointer chasing - creates address reloads */
        if (ptr->next && (i % 3 == 0)) {
            ptr = ptr->next;
        }
    }
    
    /* Complex expression using all temporaries */
    return temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + temp7 + temp8 + results[0];
}

/* Test 3: Multiple output operands and operand address reloads - triggers RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
__attribute__((noinline))
static int test_multi_output(int x, int y, int z) {
    /* Create many intermediate values */
    int a = x * y;
    int b = y * z;
    int c = z * x;
    int d = a + b;
    int e = b + c;
    int f = c + a;
    int g = d * e;
    int h = e * f;
    int i = f * d;
    int j = g + h;
    int k = h + i;
    int l = i + g;
    
    /* Array of pointers for operand address reloads */
    int* ptr_array[6];
    ptr_array[0] = &a;
    ptr_array[1] = &b;
    ptr_array[2] = &c;
    ptr_array[3] = &d;
    ptr_array[4] = &e;
    ptr_array[5] = &f;
    
    /* Complex inline asm with multiple memory operands */
    int result = 0;
    for (int idx = 0; idx < 6; idx++) {
        int* current_ptr = ptr_array[idx];
        int offset = idx * 4;
        
        /* Inline asm that uses the pointer as both input and output address */
        __asm__ volatile (
            "movl (%[in]), %%eax \n\t"
            "imull %[mul], %%eax \n\t"
            "movl %%eax, (%[out]) \n\t"
            : "=m" (*current_ptr)
            : [in] "r" (current_ptr), [mul] "r" (idx + 1), [out] "r" (current_ptr)
            : "%eax", "memory"
        );
        
        /* Use the modified value */
        result += *current_ptr + offset;
    }
    
    /* More register pressure */
    volatile int* volatile_ptr = &volatile_globals[0];
    for (int m = 0; m < 4; m++) {
        *volatile_ptr = result + m;
        volatile_ptr += 5;  /* Skip pattern to prevent optimization */
    }
    
    return result + j + k + l;
}

/* Test 4: Mixed addressing modes and other reload types - triggers RELOAD_OTHER, RELOAD_FOR_OTHER_ADDRESS */
__attribute__((noinline))
static int test_mixed_reloads(int base) {
    /* Extreme local variable count */
    int v1 = base + 1, v2 = base + 2, v3 = base + 3, v4 = base + 4;
    int v5 = base * 2, v6 = base * 3, v7 = base * 4, v8 = base * 5;
    int v9 = v1 + v5, v10 = v2 + v6, v11 = v3 + v7, v12 = v4 + v8;
    int v13 = v9 * 2, v14 = v10 * 3, v15 = v11 * 4, v16 = v12 * 5;
    int v17 = v13 - v1, v18 = v14 - v2, v19 = v15 - v3, v20 = v16 - v4;
    
    /* Multi-dimensional local array */
    int local_array[3][4][2];
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 4; j++)
            for (int k = 0; k < 2; k++)
                local_array[i][j][k] = i * 100 + j * 10 + k + base;
    
    /* Complex addressing with multiple levels */
    int sum = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            /* Address computation that needs reloading */
            int* addr1 = &local_array[i][j][0] + v1;
            int* addr2 = &local_array[i][j][1] + v2;
            
            /* Memory operations with computed addresses */
            sum += *addr1 + *addr2;
            
            /* Pointer arithmetic in loop */
            int* complex_addr = addr1 + (v3 * i + v4 * j);
            sum += *complex_addr;
        }
    }
    
    /* Use __builtin_expect to create data dependencies */
    if (__builtin_expect(sum > 1000, 0)) {
        /* Force spill/reload patterns */
        v1 = v2 + v3;
        v2 = v3 + v4;
        v3 = v4 + v5;
        v4 = v5 + v6;
    }
    
    /* Final complex expression using all variables */
    return sum + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + 
           v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16 +
           v17 + v18 + v19 + v20;
}

/* Main driver that calls all tests */
int main(void) {
    int total = 0;
    
    /* Call each test multiple times with different arguments */
    for (int i = 0; i < 3; i++) {
        total += test_complex_addressing(i, i+1, i+2, i+3, i+4, i+5, i+6, i+7);
        total += test_structure_asm(i * 10);
        total += test_multi_output(i * 5, i * 7, i * 11);
        total += test_mixed_reloads(i * 20);
    }
    
    /* Use result to prevent dead code elimination */
    volatile int final_result = total;
    
    /* Additional volatile writes to force more reloads */
    for (int i = 0; i < 10; i++) {
        volatile_globals[i] = total + i;
    }
    
    return final_result % 256;  /* Return non-zero to indicate execution */
}

#pragma GCC pop_options
