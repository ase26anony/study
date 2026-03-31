/* test-omp-array-section.c
 * 
 * This program is designed to trigger the pretty-printing logic for
 * OMP_ARRAY_SECTION nodes in GCC's tree-pretty-print.cc, specifically
 * lines 2736-2748, which handle the formatting of OpenMP array sections.
 *
 * The code uses complex base expressions with operator precedence issues
 * (requiring parentheses in the printed output) and variable bounds to
 * ensure the uncovered lines are executed during compiler dumps or
 * diagnostics.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to prevent constant folding */
static int use_arg(int argc, char **argv) {
    return (argc > 1) ? atoi(argv[1]) : 5;
}

/* Function that takes a pointer and size for a target region */
void process_section(int *base, int start, int length) {
    #pragma omp target map(tofrom: base[start:length])
    for (int i = 0; i < length; i++) {
        base[start + i] += i;
    }
}

/* Another function using array section with complex base */
void process_with_conditional(int *arr1, int *arr2, int cond, int start, int len) {
    /* This creates a base expression: (cond ? arr1 : arr2)
     * The ternary operator has lower precedence than array subscripting,
     * so parentheses may be needed when pretty-printing the array section.
     */
    #pragma omp target map(tofrom: (cond ? arr1 : arr2)[start:len])
    for (int i = 0; i < len; i++) {
        (cond ? arr1 : arr2)[start + i] *= 2;
    }
}

/* Function using pointer arithmetic as base */
void process_with_pointer_arithmetic(int *ptr, int offset, int count) {
    /* Base expression: (ptr + offset) 
     * Addition has lower precedence than array subscripting.
     */
    #pragma omp target map(tofrom: (ptr + offset)[0:count])
    for (int i = 0; i < count; i++) {
        (ptr + offset)[i] -= 1;
    }
}

/* Struct with array member to create member access base */
struct WithArray {
    int header;
    int data[100];
};

void process_struct_member(struct WithArray *s, int idx, int len) {
    /* Base expression: s->data (or s[0].data)
     * Member access has higher precedence than array subscripting,
     * but we can create a more complex base with indexing.
     */
    #pragma omp target map(tofrom: s[0].data[idx:len])
    for (int i = 0; i < len; i++) {
        s[0].data[idx + i] += s[0].header;
    }
}

/* Task dependency with array section */
void create_tasks(int *arr, int n, int chunk) {
    #pragma omp task depend(out: arr[0:chunk])
    {
        #pragma omp target map(tofrom: arr[0:chunk])
        for (int i = 0; i < chunk; i++) arr[i] = i;
    }
    
    #pragma omp task depend(in: arr[0:chunk]) depend(out: arr[chunk:n-chunk])
    {
        #pragma omp target map(tofrom: arr[chunk:n-chunk])
        for (int i = chunk; i < n; i++) arr[i] = arr[i-chunk] + 1;
    }
    
    #pragma omp task depend(in: arr[chunk:n-chunk])
    {
        int sum = 0;
        for (int i = 0; i < n; i++) sum += arr[i];
        printf("Task computed sum: %d\n", sum);
    }
}

int main(int argc, char **argv) {
    /* Use command-line arguments to prevent constant propagation */
    volatile int base_size = use_arg(argc, argv);
    volatile int section_start = (argc > 2) ? atoi(argv[2]) : 2;
    volatile int section_len = (argc > 3) ? atoi(argv[3]) : 10;
    
    int total_size = base_size * 4;
    int *array1 = (int *)malloc(total_size * sizeof(int));
    int *array2 = (int *)malloc(total_size * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < total_size; i++) {
        array1[i] = i % 100;
        array2[i] = (i + 50) % 100;
    }
    
    /* 1. Simple array section with variable bounds */
    printf("Test 1: Simple array section\n");
    #pragma omp target data map(tofrom: array1[section_start:section_len])
    {
        #pragma omp target map(tofrom: array1[section_start:section_len])
        for (int i = 0; i < section_len; i++) {
            array1[section_start + i] += 1;
        }
    }
    
    /* 2. Array section with conditional base (triggers op_prio check) */
    printf("Test 2: Array section with conditional base\n");
    int cond = (argc > 1);
    process_with_conditional(array1, array2, cond, section_start, section_len);
    
    /* 3. Array section with pointer arithmetic base */
    printf("Test 3: Array section with pointer arithmetic base\n");
    int offset = (argc > 1) ? 3 : 1;
    process_with_pointer_arithmetic(array1, offset, section_len);
    
    /* 4. Array section with struct member access */
    printf("Test 4: Array section with struct member access\n");
    struct WithArray s;
    s.header = 10;
    for (int i = 0; i < 100; i++) s.data[i] = i;
    process_struct_member(&s, section_start, section_len);
    
    /* 5. Array sections in task dependencies */
    printf("Test 5: Array sections in task dependencies\n");
    #pragma omp parallel
    #pragma omp single
    create_tasks(array1, total_size, total_size/2);
    
    /* 6. Deliberate type warning/error potential */
    /* This may trigger diagnostics when pretty-printing array sections */
    printf("Test 6: Mixed contexts for diagnostics\n");
    {
        int *ptr = array1;
        /* Array section in non-OpenMP context - may trigger warnings */
        /* The following line is deliberately questionable for diagnostics */
        int (*func_ptr)(int *) = NULL;
        /* func_ptr(array1[0:5]); */ /* This would be invalid C but helps trigger diagnostics */
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < total_size; i++) {
        checksum += array1[i] + array2[i] + s.data[i % 100];
    }
    printf("Final checksum: %d\n", checksum);
    
    free(array1);
    free(array2);
    
    return 0;
}
