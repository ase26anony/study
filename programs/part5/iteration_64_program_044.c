/* test-omp-depend-coverage.c
 * 
 * This test program is designed to trigger GCC's tree pretty-printer
 * for all OMP_CLAUSE_DEPEND_* enumeration values, specifically targeting
 * the uncovered block in tree-pretty-print.cc lines 824-846.
 *
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple -c test-omp-depend-coverage.c
 * Additional flags for more dumps: -fdump-tree-original -fdump-tree-optimized
 */

#include <stdlib.h>

int main(void) {
    /* Declare variables for various dependency types */
    int x = 0, y = 0, z = 0;
    int arr[10] = {0};
    int *ptr = &x;
    int depobj_var = 0;
    
    /* 1. OMP_CLAUSE_DEPEND_IN - simple in dependency */
    if (0) {
        #pragma omp task depend(in: x)
        {
            x = 1;
        }
    }
    
    /* 2. OMP_CLAUSE_DEPEND_OUT - simple out dependency */
    if (0) {
        #pragma omp task depend(out: y)
        {
            y = 2;
        }
    }
    
    /* 3. OMP_CLAUSE_DEPEND_INOUT - inout dependency */
    if (0) {
        #pragma omp task depend(inout: z)
        {
            z++;
        }
    }
    
    /* 4. OMP_CLAUSE_DEPEND_INOUTSET - inoutset dependency */
    if (0) {
        #pragma omp task depend(inoutset: arr[0])
        {
            arr[0] = 10;
        }
    }
    
    /* 5. OMP_CLAUSE_DEPEND_MUTEXINOUTSET - mutexinoutset dependency */
    if (0) {
        #pragma omp task depend(mutexinoutset: arr[1])
        {
            arr[1] = 20;
        }
    }
    
    /* 6. OMP_CLAUSE_DEPEND_DEPOBJ - depobj dependency */
    if (0) {
        #pragma omp task depend(depobj: depobj_var)
        {
            depobj_var = 30;
        }
    }
    
    /* 7. Complex dependency expressions with multiple items */
    if (0) {
        int a = 0, b = 0, c = 0;
        #pragma omp task depend(in: a, b) depend(out: c)
        {
            c = a + b;
        }
    }
    
    /* 8. Pointer dereference in dependency */
    if (0) {
        #pragma omp task depend(out: *ptr)
        {
            *ptr = 100;
        }
    }
    
    /* 9. Array section with iterator (C/C++ syntax) */
    if (0) {
        int len = 5;
        #pragma omp task depend(in: arr[0:len])
        {
            int sum = 0;
            for (int i = 0; i < len; i++) {
                sum += arr[i];
            }
        }
    }
    
    /* 10. Target region with dependencies */
    if (0) {
        #pragma omp target depend(in: x) depend(out: y) map(tofrom: x, y)
        {
            y = x * 2;
        }
    }
    
    /* 11. Combined construct with dependency */
    if (0) {
        #pragma omp target parallel for depend(inout: z)
        for (int i = 0; i < 10; i++) {
            arr[i] = z + i;
        }
    }
    
    /* 12. Nested tasks inside parallel region - creates multiple depend clauses
     * This helps trigger iteration through clause lists, potentially reaching
     * OMP_CLAUSE_DEPEND_LAST during pretty-printing */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                #pragma omp task depend(in: x)
                { /* Task A */ }
                
                #pragma omp task depend(out: y) depend(inoutset: z)
                { /* Task B */ }
                
                #pragma omp task depend(mutexinoutset: arr[2]) depend(depobj: depobj_var)
                { /* Task C */ }
            }
        }
    }
    
    /* 13. Taskwait with depend clause */
    if (0) {
        #pragma omp task depend(out: x)
        { x = 1; }
        
        #pragma omp task depend(in: x)
        { y = x + 1; }
        
        #pragma omp taskwait depend(inoutset: z)
    }
    
    /* 14. Taskloop with dependency */
    if (0) {
        #pragma omp taskloop depend(inout: x)
        for (int i = 0; i < 10; i++) {
            arr[i] = x + i;
        }
    }
    
    /* 15. Taskgroup with task dependency */
    if (0) {
        #pragma omp taskgroup
        {
            #pragma omp task depend(in: y)
            { z = y * 2; }
        }
    }
    
    /* Force a barrier to ensure all tasks complete if executed */
    #pragma omp barrier
    
    return 0;
}

/* Additional test to ensure all depend kinds are covered in one construct
 * This creates a task with multiple depend clauses of different types */
#ifdef TEST_ALL_IN_ONE
void test_all_depend_kinds(void) {
    int var_in = 0, var_out = 0, var_inout = 0;
    int var_inoutset = 0, var_mutexinoutset = 0, var_depobj = 0;
    
    /* This single task with multiple depend clauses will cause the
     * pretty-printer to iterate through all clause types */
    #pragma omp task \
        depend(in: var_in) \
        depend(out: var_out) \
        depend(inout: var_inout) \
        depend(inoutset: var_inoutset) \
        depend(mutexinoutset: var_mutexinoutset) \
        depend(depobj: var_depobj)
    {
        var_out = var_in + var_inout + var_inoutset + var_mutexinoutset + var_depobj;
    }
}
#endif
