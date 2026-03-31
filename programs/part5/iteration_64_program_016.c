/* test_depend_clauses.c - Exhaustive test for OMP_CLAUSE_DEPEND_* pretty-printing */

#include <stdlib.h>

/* Global variables for various dependency types */
int global_var = 0;
int global_arr[100];
int *global_ptr = &global_var;
omp_depend_t depobj_var;

void test_depend_clauses() {
    int x = 0, y = 0, z = 0;
    int arr[50];
    int *ptr = arr;
    omp_depend_t depobj_local;
    
    /* 1. OMP_CLAUSE_DEPEND_DEPOBJ - with depobj modifier */
    if (0) {
        #pragma omp task depend(depobj: depobj_local)
        {
            global_var++;
        }
        
        #pragma omp task depend(depobj: depobj_var)
        {
            global_arr[0] = 1;
        }
    }
    
    /* 2. OMP_CLAUSE_DEPEND_IN - input dependency */
    if (0) {
        #pragma omp task depend(in: x) shared(x)
        {
            int temp = x;  /* Read x */
            (void)temp;
        }
        
        /* Multiple items in single clause */
        #pragma omp task depend(in: x, y, arr[10]) shared(x, y, arr)
        {
            int sum = x + y + arr[10];
            (void)sum;
        }
        
        /* With iterator modifier (C/C++ syntax) */
        #pragma omp task depend(in: arr[5:10])
        {
            for (int i = 5; i < 15; i++) arr[i] = i;
        }
    }
    
    /* 3. OMP_CLAUSE_DEPEND_OUT - output dependency */
    if (0) {
        #pragma omp task depend(out: y)
        {
            y = 42;  /* Write y */
        }
        
        /* Pointer dereference */
        #pragma omp task depend(out: *ptr)
        {
            *ptr = 100;
        }
        
        /* Array element */
        #pragma omp task depend(out: arr[25])
        {
            arr[25] = 999;
        }
    }
    
    /* 4. OMP_CLAUSE_DEPEND_INOUT - input/output dependency */
    if (0) {
        #pragma omp task depend(inout: z)
        {
            z = z * 2 + 1;  /* Read and write z */
        }
        
        /* Multiple inout dependencies */
        #pragma omp task depend(inout: x, y) shared(x, y)
        {
            x++;
            y--;
        }
    }
    
    /* 5. OMP_CLAUSE_DEPEND_MUTEXINOUTSET - mutex set dependency */
    if (0) {
        #pragma omp task depend(mutexinoutset: global_var)
        {
            global_var = global_var ^ 0xFF;
        }
        
        /* In parallel region with multiple tasks */
        #pragma omp parallel
        {
            #pragma omp task depend(mutexinoutset: arr[0])
            {
                arr[0] = 1;
            }
            
            #pragma omp task depend(mutexinoutset: arr[0])
            {
                arr[0] = 2;
            }
        }
    }
    
    /* 6. OMP_CLAUSE_DEPEND_INOUTSET - set dependency */
    if (0) {
        #pragma omp task depend(inoutset: global_arr[10])
        {
            global_arr[10] += 5;
        }
        
        /* Combined with other dependencies */
        #pragma omp task depend(in: x) depend(inoutset: y)
        {
            y = x + y;
        }
    }
    
    /* 7. Trigger OMP_CLAUSE_DEPEND_LAST through multiple clauses */
    /* Multiple depend clauses on single construct to ensure full iteration */
    if (0) {
        #pragma omp task depend(in: x) depend(out: y) depend(inout: z) \
                         depend(mutexinoutset: arr[0]) depend(inoutset: arr[1])
        {
            /* Complex operation using all dependencies */
            z = x + y + arr[0] + arr[1];
            arr[0] = z;
            arr[1] = z * 2;
        }
    }
    
    /* 8. Target regions with dependencies */
    if (0) {
        #pragma omp target depend(in: x) depend(out: arr[0:10]) map(tofrom: x, arr)
        {
            for (int i = 0; i < 10; i++) {
                arr[i] = x + i;
            }
        }
        
        #pragma omp target teams distribute parallel for \
                   depend(inout: global_var) map(tofrom: global_var)
        for (int i = 0; i < 100; i++) {
            global_arr[i] = global_var + i;
        }
    }
    
    /* 9. Nested tasks with complex dependency graph */
    if (0) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                #pragma omp task depend(out: x)
                { x = 1; }
                
                #pragma omp task depend(in: x) depend(out: y)
                { y = x + 2; }
                
                #pragma omp task depend(in: x) depend(in: y) depend(out: z)
                { z = x + y; }
                
                #pragma omp task depend(in: z) depend(depobj: depobj_local)
                { global_var = z; }
            }
        }
    }
    
    /* 10. Taskloop with dependencies */
    if (0) {
        #pragma omp taskloop depend(inout: arr[0:50]) shared(arr)
        for (int i = 0; i < 50; i++) {
            arr[i] = i * 2;
        }
    }
}

/* Main function to organize test blocks */
int main() {
    /* Initialize dependency object */
    depobj_var = omp_depend_t();
    
    /* Call test function - multiple times to ensure all paths are processed */
    test_depend_clauses();
    
    /* Additional test with different scope */
    if (0) {
        int local_var = 0;
        #pragma omp task depend(inout: local_var)
        {
            local_var = 100;
        }
        
        /* Force task execution for completeness */
        #pragma omp taskwait
    }
    
    return 0;
}
