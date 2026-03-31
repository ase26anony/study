/* reload_stress.c - Extreme register pressure test for GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define NUM_LOCALS 40
#define ITERATIONS 1000

/* Opaque noinline functions to prevent optimization */
__attribute__((noinline)) int helper1(int a, int b, int c, int d, int e, int f) {
    volatile int sink = a + b - c + d - e + f;
    return sink;
}

__attribute__((noinline)) double helper2(double a, double b, double c, 
                                        double d, double e, double f) {
    volatile double sink = a * b + c * d - e * f;
    return sink;
}

__attribute__((noinline)) long helper3(long a, long b, long c, long d,
                                      void* addr1, void* addr2) {
    volatile long sink = a + b + c + d + (long)addr1 + (long)addr2;
    return sink;
}

__attribute__((noinline)) float helper4(float a, float b, float c, float d,
                                       int* ptr1, double* ptr2) {
    volatile float sink = a * b + c / d + *ptr1 + (float)*ptr2;
    return sink;
}

__attribute__((noinline)) void* helper5(void* base, int offset1, int offset2,
                                       int offset3, int offset4) {
    volatile char* sink = (char*)base + offset1 + offset2 + offset3 + offset4;
    return (void*)sink;
}

/* Main stress function with extreme register pressure */
__attribute__((noinline, noipa))
int stress_reload(int* arr_int, double* arr_dbl, long* arr_long, float* arr_flt) {
    /* Declare many local variables to exhaust registers */
    /* Integer variables */
    volatile int v1 = arr_int[0];
    volatile int v2 = arr_int[1];
    volatile int v3 = arr_int[2];
    volatile int v4 = arr_int[3];
    volatile int v5 = arr_int[4];
    volatile int v6 = arr_int[5];
    volatile int v7 = arr_int[6];
    volatile int v8 = arr_int[7];
    volatile int v9 = arr_int[8];
    volatile int v10 = arr_int[9];
    
    /* More integer variables */
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    /* Floating point variables */
    volatile double d1 = arr_dbl[0];
    volatile double d2 = arr_dbl[1];
    volatile double d3 = arr_dbl[2];
    volatile double d4 = arr_dbl[3];
    volatile double d5 = arr_dbl[4];
    volatile double d6 = arr_dbl[5];
    
    /* Long variables */
    volatile long l1 = arr_long[0];
    volatile long l2 = arr_long[1];
    volatile long l3 = arr_long[2];
    volatile long l4 = arr_long[3];
    volatile long l5 = arr_long[4];
    
    /* Float variables */
    volatile float f1 = arr_flt[0];
    volatile float f2 = arr_flt[1];
    volatile float f3 = arr_flt[2];
    volatile float f4 = arr_flt[3];
    volatile float f5 = arr_flt[4];
    
    /* Pointer variables - taking addresses of locals */
    int* p1 = &v1;
    int* p2 = &v2;
    double* p3 = &d1;
    double* p4 = &d2;
    long* p5 = &l1;
    float* p6 = &f1;
    
    /* Additional volatile pointers to prevent optimization */
    volatile int* volatile pv1 = &v3;
    volatile double* volatile pv2 = &d3;
    volatile long* volatile pv3 = &l2;
    volatile float* volatile pv4 = &f2;
    
    int checksum = 0;
    
    /* Complex loop with extreme register pressure */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (i * 7 + v1 * 3 + v2 * 5) % ARRAY_SIZE;
        int idx2 = (i * 11 + v3 * 13 + v4 * 17) % ARRAY_SIZE;
        int idx3 = (i * 19 + v5 * 23 + v6 * 29) % ARRAY_SIZE;
        int idx4 = (i * 31 + v7 * 37 + v8 * 41) % ARRAY_SIZE;
        int idx5 = (i * 43 + v9 * 47 + v10 * 53) % ARRAY_SIZE;
        
        /* More complex indices for different arrays */
        int idx_dbl = (idx1 * 2 + idx2 * 3 + idx3 * 5) % ARRAY_SIZE;
        int idx_long = (idx2 * 7 + idx3 * 11 + idx4 * 13) % ARRAY_SIZE;
        int idx_flt = (idx3 * 17 + idx4 * 19 + idx5 * 23) % ARRAY_SIZE;
        
        /* Control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* Branch 0: Complex address computations */
                int* addr1 = &arr_int[idx1];
                int* addr2 = &arr_int[idx2];
                double* addr3 = &arr_dbl[idx_dbl];
                long* addr4 = &arr_long[idx_long];
                
                /* Inline assembly with conflicting constraints */
                int temp1, temp2;
                asm volatile (
                    "mov %[val1], %[tmp1]\n\t"
                    "mov %[val2], %[tmp2]\n\t"
                    "add %[tmp2], %[tmp1]\n\t"
                    : [tmp1] "=r" (temp1), [tmp2] "=r" (temp2)
                    : [val1] "m" (*addr1), [val2] "m" (*addr2)
                    : "cc"
                );
                
                v11 = temp1 + temp2;
                
                /* Use computed addresses in function calls */
                checksum += helper1(v1, v2, v3, *addr1, *addr2, v11);
                checksum += (int)helper2(d1, d2, d3, *addr3, d4, d5);
                
                /* Force address reloads by using same value as data and address */
                int* volatile addr5 = (int*)((uintptr_t)addr1 + *addr1);
                checksum += *addr5;
                break;
            }
            
            case 1: {
                /* Branch 1: Different addressing patterns */
                double* addr_dbl1 = &arr_dbl[(idx1 + idx2 * 2) % ARRAY_SIZE];
                double* addr_dbl2 = &arr_dbl[(idx3 + idx4 * 3) % ARRAY_SIZE];
                float* addr_flt1 = &arr_flt[(idx2 + idx3 * 5) % ARRAY_SIZE];
                
                /* Inline assembly with memory constraints */
                double result1, result2;
                asm volatile (
                    "movsd (%[addr1]), %[res1]\n\t"
                    "movsd (%[addr2]), %[res2]\n\t"
                    "addsd %[res2], %[res1]\n\t"
                    : [res1] "=x" (result1), [res2] "=x" (result2)
                    : [addr1] "r" (addr_dbl1), [addr2] "r" (addr_dbl2)
                    : "memory"
                );
                
                d6 = result1 + result2;
                
                checksum += (int)helper4(f1, f2, f3, f4, (int*)addr_dbl1, addr_dbl2);
                break;
            }
            
            case 2: {
                /* Branch 2: Pointer arithmetic with multiple offsets */
                char* base_ptr = (char*)arr_int;
                int offset1 = v1 * 4;
                int offset2 = v2 * 8;
                int offset3 = v3 * 12;
                int offset4 = v4 * 16;
                
                void* complex_addr = helper5(base_ptr, offset1, offset2, offset3, offset4);
                checksum += *(int*)complex_addr;
                
                /* More complex addressing */
                long* addr_long1 = &arr_long[(offset1 + offset2) % ARRAY_SIZE];
                long* addr_long2 = &arr_long[(offset3 + offset4) % ARRAY_SIZE];
                
                checksum += helper3(l1, l2, l3, l4, addr_long1, addr_long2);
                break;
            }
            
            case 3:
            case 4:
            case 5: {
                /* Multiple cases sharing some code but with variations */
                int* addr_arr[4];
                addr_arr[0] = &arr_int[idx1];
                addr_arr[1] = &arr_int[idx2];
                addr_arr[2] = &arr_int[idx3];
                addr_arr[3] = &arr_int[idx4];
                
                /* Chain of dependent address computations */
                int* chain_addr = addr_arr[i % 4];
                for (int j = 0; j < 3; j++) {
                    chain_addr = &arr_int[(*chain_addr + j) % ARRAY_SIZE];
                }
                
                checksum += *chain_addr;
                
                /* Mixed type operations */
                if (i % 3 == 0) {
                    checksum += (int)(d1 * d2 + f1 * f2);
                } else {
                    checksum += (int)(d3 * d4 - f3 * f4);
                }
                break;
            }
            
            default: {
                /* Default branch with even more complex patterns */
                /* Nested addressing */
                int** ptr_to_ptr = &p1;
                int*** ptr_to_ptr_to_ptr = &ptr_to_ptr;
                
                /* Force multiple levels of indirection */
                checksum += ***ptr_to_ptr_to_ptr;
                
                /* Array of pointers with complex indexing */
                void* ptr_array[8];
                ptr_array[0] = &arr_int[(i * 59) % ARRAY_SIZE];
                ptr_array[1] = &arr_dbl[(i * 61) % ARRAY_SIZE];
                ptr_array[2] = &arr_long[(i * 67) % ARRAY_SIZE];
                ptr_array[3] = &arr_flt[(i * 71) % ARRAY_SIZE];
                ptr_array[4] = &v1;
                ptr_array[5] = &d1;
                ptr_array[6] = &l1;
                ptr_array[7] = &f1;
                
                /* Use computed goto to create non-trivial control flow */
                static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
                goto *labels[i % 4];
                
                label0:
                    checksum += *(int*)ptr_array[0];
                    goto end_labels;
                label1:
                    checksum += (int)*(double*)ptr_array[1];
                    goto end_labels;
                label2:
                    checksum += *(long*)ptr_array[2];
                    goto end_labels;
                label3:
                    checksum += (int)*(float*)ptr_array[3];
                    goto end_labels;
                end_labels:
                break;
            }
        }
        
        /* Update most local variables to keep them live */
        v1 = v1 + arr_int[idx1];
        v2 = v2 + arr_int[idx2];
        v3 = v3 + arr_int[idx3];
        v4 = v4 + arr_int[idx4];
        v5 = v5 + arr_int[idx5];
        
        d1 = d1 + arr_dbl[idx_dbl];
        d2 = d2 + arr_dbl[(idx_dbl + 1) % ARRAY_SIZE];
        
        l1 = l1 + arr_long[idx_long];
        l2 = l2 + arr_long[(idx_long + 1) % ARRAY_SIZE];
        
        f1 = f1 + arr_flt[idx_flt];
        f2 = f2 + arr_flt[(idx_flt + 1) % ARRAY_SIZE];
        
        /* More variable updates */
        v11 = v1 + v2;
        v12 = v3 + v4;
        v13 = v5 + v6;
        v14 = v7 + v8;
        v15 = v9 + v10;
        
        v16 = v11 * v12;
        v17 = v13 * v14;
        v18 = v15 * v16;
        v19 = v17 * v18;
        v20 = v19 % 1000;
        
        /* Force spilling around function calls */
        checksum += helper1(v11, v12, v13, v14, v15, v16);
        checksum += helper1(v17, v18, v19, v20, v1, v2);
        
        /* Use all pointer variables */
        *p1 = v1;
        *p2 = v2;
        *p3 = d1;
        *p4 = d2;
        *p5 = l1;
        *p6 = f1;
        
        *pv1 = v3;
        *pv2 = d3;
        *pv3 = l2;
        *pv4 = f2;
    }
    
    return checksum;
}

int main() {
    /* Allocate and initialize arrays with pattern data */
    int* arr_int = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* arr_dbl = (double*)malloc(ARRAY_SIZE * sizeof(double));
    long* arr_long = (long*)malloc(ARRAY_SIZE * sizeof(long));
    float* arr_flt = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    if (!arr_int || !arr_dbl || !arr_long || !arr_flt) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr_int[i] = i * 3 + 1;
        arr_dbl[i] = i * 1.5 + 0.5;
        arr_long[i] = i * 7L + 3L;
        arr_flt[i] = i * 0.7f + 0.3f;
    }
    
    printf("Starting reload stress test...\n");
    
    /* Call the stress function */
    int result = stress_reload(arr_int, arr_dbl, arr_long, arr_flt);
    
    printf("Checksum result: %d\n", result);
    printf("Test completed.\n");
    
    /* Cleanup */
    free(arr_int);
    free(arr_dbl);
    free(arr_long);
    free(arr_flt);
    
    return 0;
}
