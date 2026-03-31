/* test-omp-array-section.c
 * 
 * This program is designed to trigger the pretty-printing logic for
 * OMP_ARRAY_SECTION nodes in GCC's tree-pretty-print.cc, specifically
 * lines 2736-2748. It uses OpenMP array sections with complex base
 * expressions (to exercise operator precedence checks) and variable
 * bounds (to prevent constant folding). The code will cause the compiler
 * to invoke the pretty-printer when using dump flags (-fdump-tree-*)
 * or when emitting diagnostics for OpenMP mapping.
 *
 * Compilation examples for coverage:
 *   gcc -O1 -fopenmp -fdump-tree-original -fdump-tree-omplower -c test-omp-array-section.c
 *   gcc -O0 -fopenmp -Wall -Werror=openmp-mapping -c test-omp-array-section.c
 *   gcc -O2 -fopenmp -foffload=disable -fdump-tree-optimized -c test-omp-array-section.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to introduce variability and prevent constant propagation */
static int get_bound(int base, int argc, const char **argv) {
    volatile int v = base;
    if (argc > 1) v += atoi(argv[1]);
    return v;
}

/* Function that uses array sections in a way that may provoke type warnings */
static void process_section(int *section, int len) {
    for (int i = 0; i < len; i++) {
        section[i] += 1;
    }
}

int main(int argc, char *argv[]) {
    int size = 100;
    int offset = 10;
    int section_len = 20;
    
    /* Use command-line arguments to make bounds non-constant */
    volatile int dynamic_offset = get_bound(offset, argc, (const char**)argv);
    int dynamic_len = get_bound(section_len, argc, (const char**)argv);
    
    /* Allocate and initialize arrays */
    int *arr1 = (int*)malloc(size * sizeof(int));
    int *arr2 = (int*)malloc(size * sizeof(int));
    int *ptr = arr1;
    
    for (int i = 0; i < size; i++) {
        arr1[i] = i;
        arr2[i] = size - i;
    }
    
    /* Structure with array member to create complex base expressions */
    struct {
        int *member_array;
        int id;
    } struct_var;
    struct_var.member_array = arr2;
    struct_var.id = 1;
    
    /* 
     * TARGET DATA REGION with multiple array sections in map clauses.
     * Using complex base expressions to trigger op_prio checks:
     * 1. Pointer arithmetic: (ptr + offset)[0:len]
     * 2. Conditional operator: (dynamic_offset > 5 ? arr1 : arr2)[start:len]
     * 3. Structure member access: struct_var.member_array[0:len]
     */
    #pragma omp target data \
        map(to: (ptr + dynamic_offset)[0:dynamic_len]) \
        map(tofrom: (dynamic_offset > 5 ? arr1 : arr2)[dynamic_offset:dynamic_len]) \
        map(alloc: struct_var.member_array[5:dynamic_len])
    {
        /* 
         * TARGET REGION inside the data region.
         * Additional array section usage with different base expressions.
         */
        int start = dynamic_offset;
        int len = dynamic_len;
        
        #pragma omp target \
            map(always, tofrom: (ptr + start)[0:len]) \
            map(to: arr2[start:len])
        {
            for (int i = 0; i < len; i++) {
                (ptr + start)[i] *= 2;
                arr2[start + i] += (ptr + start)[i];
            }
        }
        
        /* 
         * Another usage: array section in is_device_ptr clause 
         * (pointer + offset as base)
         */
        int *dev_ptr = ptr + start;
        #pragma omp target is_device_ptr(dev_ptr[0:len])
        {
            for (int i = 0; i < len; i++) {
                dev_ptr[i] += i;
            }
        }
    }
    
    /* 
     * TASK with depend clause using array section.
     * This creates additional OMP_ARRAY_SECTION nodes.
     */
    #pragma omp task depend(inout: arr1[dynamic_offset:dynamic_len])
    {
        for (int i = dynamic_offset; i < dynamic_offset + dynamic_len; i++) {
            arr1[i] -= 1;
        }
    }
    
    #pragma omp taskwait
    
    /* 
     * Deliberate type-checking issue: passing array section to function
     * expecting pointer. This may trigger a warning/error and cause the
     * compiler to pretty-print the array section node in diagnostics.
     * (Commented out by default; uncomment for diagnostic testing)
     */
    /* process_section(arr1[dynamic_offset:dynamic_len], dynamic_len); */
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += arr1[i] + arr2[i];
    }
    printf("Checksum: %ld\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    
    return 0;
}
