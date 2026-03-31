/* test-omp-array-section.c
 * 
 * This program is designed to trigger the OMP_ARRAY_SECTION pretty-printing
 * logic in GCC's tree-pretty-print.cc, specifically lines 2736-2748.
 * It uses complex base expressions with operator precedence issues,
 * variable bounds, and multiple OpenMP contexts to ensure the pretty-printer
 * is invoked on array section nodes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper to prevent constant folding */
static volatile int g_volatile_size = 100;

/* Structure with array member for member access base expression */
struct WithArray {
    int data[200];
    int *ptr;
};

/* Function that returns different arrays based on condition */
int* select_array(int cond, int* a, int* b) {
    return cond ? a : b;
}

/* Function using array section in map clause - may trigger type warnings */
void process_section(int* section, int len) {
    /* Deliberate type context: passing what looks like pointer 
     * but is actually an array section tree node in OpenMP context */
    #pragma omp target if(0) map(to: section[0:len])
    {
        /* Empty target region - just for tree generation */
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to prevent constant propagation of bounds */
    int base_size = (argc > 1) ? atoi(argv[1]) : 50;
    if (base_size <= 0) base_size = 50;
    
    int n = base_size + g_volatile_size % 50;  /* Variable size */
    int offset = (argc > 2) ? atoi(argv[2]) : 10;
    int section_len = (argc > 3) ? atoi(argv[3]) : 20;
    
    /* Allocate and initialize arrays */
    int *arr1 = (int*)malloc(3 * n * sizeof(int));
    int *arr2 = (int*)malloc(3 * n * sizeof(int));
    struct WithArray s1;
    
    for (int i = 0; i < 3 * n; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        if (i < 200) s1.data[i] = i * 3;
    }
    
    s1.ptr = arr1;
    
    /* STRATEGY 1: Complex base expressions with operator precedence issues */
    
    /* Case 1: Conditional operator as base - will need parentheses 
     * because '?:' has lower precedence than array subscript */
    #pragma omp target data map(to: (select_array(offset > 5, arr1, arr2))[offset:section_len])
    {
        #pragma omp target map(alloc: (offset > 5 ? arr1 : arr2)[offset:section_len])
        {
            for (int i = 0; i < section_len; i++) {
                int *base = offset > 5 ? arr1 : arr2;
                base[offset + i] += 1;
            }
        }
    }
    
    /* Case 2: Pointer arithmetic as base - '+' has lower precedence than '[]' */
    #pragma omp target enter data map(to: (arr1 + offset)[0:section_len])
    
    /* Case 3: Structure member access - '.' has higher precedence, but mixed
     * with pointer arithmetic creates interesting trees */
    #pragma omp target map(tofrom: s1.data[offset:section_len])
    {
        for (int i = 0; i < section_len; i++) {
            s1.data[offset + i] *= 2;
        }
    }
    
    /* Case 4: Complex nested expression */
    int *ptr = arr2;
    #pragma omp target data map(tofrom: (ptr + (offset > 10 ? offset : 5))[0:section_len])
    {
        #pragma omp target 
        {
            int *base = ptr + (offset > 10 ? offset : 5);
            for (int i = 0; i < section_len; i++) {
                base[i] -= 1;
            }
        }
    }
    
    /* STRATEGY 2: Multiple OpenMP clauses with array sections */
    
    /* Different clauses to create various tree contexts */
    #pragma omp target data map(to: arr1[0:n]) map(from: arr2[0:n]) \
                            map(alloc: s1.data[0:section_len])
    {
        #pragma omp target 
        {
            for (int i = 0; i < n; i++) {
                arr1[i] = arr2[i] + s1.data[i % section_len];
            }
        }
    }
    
    /* STRATEGY 3: Task depend with array sections */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(out: arr1[0:section_len])
            {
                for (int i = 0; i < section_len; i++) {
                    arr1[i] = i;
                }
            }
            
            #pragma omp task depend(in: arr1[0:section_len]) \
                             depend(out: arr2[0:section_len])
            {
                for (int i = 0; i < section_len; i++) {
                    arr2[i] = arr1[i] * 2;
                }
            }
        }
    }
    
    /* STRATEGY 4: Potential type warning/error contexts */
    
    /* This might trigger diagnostics about array section usage */
    process_section(arr1, section_len);
    
    /* Array section in is_device_ptr clause with pointer arithmetic */
    #pragma omp target data map(to: arr1[0:n])
    {
        int *dev_ptr = arr1;
        #pragma omp target is_device_ptr(dev_ptr) \
                          map(tofrom: (dev_ptr + offset)[0:section_len])
        {
            for (int i = 0; i < section_len; i++) {
                dev_ptr[offset + i] += 3;
            }
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += arr1[i] + arr2[i];
        if (i < 200) checksum += s1.data[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    printf("Array section tests completed.\n");
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    
    return 0;
}
