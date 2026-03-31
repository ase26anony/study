/* test_depend_clauses.c - Exhaustive test for OMP_CLAUSE_DEPEND_* pretty-printing */
/* Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple test_depend_clauses.c */

#include <stdlib.h>

int main(void) {
    /* Declare test variables of various types */
    int x = 0, y = 0, z = 0;
    int arr[10] = {0};
    int *ptr = &x;
    void *depobj_var = NULL;
    
    /* Prevent runtime execution while ensuring compilation */
    if (0) {
        /* Block 1: Basic depend clause types in task construct */
        /* Should trigger: OMP_CLAUSE_DEPEND_IN, OMP_CLAUSE_DEPEND_OUT, OMP_CLAUSE_DEPEND_INOUT */
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z)
        {
            y = x + 1;
            z = y + z;
        }
        
        /* Block 2: Set-based depend types in nested task */
        /* Should trigger: OMP_CLAUSE_DEPEND_MUTEXINOUTSET, OMP_CLAUSE_DEPEND_INOUTSET */
        #pragma omp parallel
        {
            #pragma omp single
            {
                #pragma omp task depend(mutexinoutset: arr[0]) 
                {
                    arr[0] = 1;
                }
                
                #pragma omp task depend(inoutset: arr[1])
                {
                    arr[1] = 2;
                }
            }
        }
        
        /* Block 3: depobj modifier with pointer */
        /* Should trigger: OMP_CLAUSE_DEPEND_DEPOBJ */
        #pragma omp task depend(depobj: depobj_var)
        {
            depobj_var = malloc(sizeof(int));
        }
        
        /* Block 4: Multiple items in single clause with array section */
        /* Should trigger iteration through multiple depend clauses */
        #pragma omp task depend(in: arr[2], arr[3]) depend(out: *ptr, arr[4])
        {
            *ptr = arr[2] + arr[3];
            arr[4] = *ptr;
        }
        
        /* Block 5: Target region with depend clause */
        /* Tests different construct context */
        #pragma omp target depend(in: arr[5]) map(tofrom: arr[5])
        {
            arr[5] *= 2;
        }
        
        /* Block 6: Combined construct with iterator modifier */
        /* Tests iterator syntax (C/C++ specific) */
        int len = 5;
        #pragma omp task depend(in: arr[6:len])
        {
            for (int i = 6; i < 6 + len; i++) {
                arr[i] = i;
            }
        }
        
        /* Block 7: Complex nested structure to stress pretty-printer */
        /* Multiple clauses on single construct to potentially trigger LAST iteration */
        #pragma omp parallel
        {
            #pragma omp single
            {
                #pragma omp task depend(in: x) depend(out: y) \
                                 depend(inout: z) depend(mutexinoutset: arr[7]) \
                                 depend(inoutset: arr[8]) depend(depobj: depobj_var)
                {
                    /* Complex operation using all dependencies */
                    y = x;
                    z = y + z;
                    arr[7] = z;
                    arr[8] = arr[7] + 1;
                }
            }
        }
        
        /* Block 8: Taskwait with depend clause */
        /* Another context for depend clause printing */
        #pragma omp task depend(inout: x)
        {
            x = 100;
        }
        
        #pragma omp taskwait depend(inoutset: x)
        
        /* Block 9: Taskloop with depend clause */
        /* Tests taskloop construct */
        #pragma omp taskloop depend(in: arr[0]) grainsize(1)
        for (int i = 0; i < 10; i++) {
            arr[i] = i * 2;
        }
    }
    
    /* Ensure variables are used to prevent optimization */
    return x + y + z + arr[0] + (ptr != NULL) + (depobj_var != NULL);
}

/* Additional test functions to create more compilation units with depend clauses */
#ifdef TEST_MORE

void test_depobj_only(void) {
    void *d = NULL;
    #pragma omp task depend(depobj: d)
    {
        d = malloc(1);
    }
    free(d);
}

void test_inoutset_complex(void) {
    int set1[5], set2[5];
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(inoutset: set1[0], set2[0])
            {
                set1[0] = 1;
                set2[0] = 2;
            }
            
            #pragma omp task depend(inoutset: set1[0])
            {
                set1[0] += 1;
            }
        }
    }
}

void test_mutexinoutset_nested(void) {
    int counter = 0;
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            #pragma omp task depend(mutexinoutset: counter)
            {
                counter++;
            }
        }
        
        #pragma omp section
        {
            #pragma omp task depend(mutexinoutset: counter)
            {
                counter--;
            }
        }
    }
}

#endif /* TEST_MORE */
