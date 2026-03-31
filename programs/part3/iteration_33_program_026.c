/* Compile with: 
   g++ -O1 -fopenmp -fdump-tree-omplower -fdump-tree-ompexp -c tree-pretty-print-test.cc
   g++ -O2 -fopenmp -fopenmp-simd -fdump-tree-all -foffload=disable -c tree-pretty-print-test.cc
   g++ -O0 -fopenmp -foffload-abi=lp64 -fdump-tree-original -c tree-pretty-print-test.cc
*/

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Function to be used with declare target enter
#ifdef __cplusplus
extern "C" {
#endif

#pragma omp declare target enter(vec_add) to(array_a, array_b, array_c)
void vec_add(int n, double* a, double* b, double* c) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

#ifdef __cplusplus
}
#endif

// Another function with declare target
#pragma omp declare target enter(compute_reduction) 
double compute_reduction(int n, double* arr) {
    double sum = 0.0;
    volatile int bound = n; // Prevent optimization
    
    #pragma omp parallel for simd reduction(+:sum) collapse(2)
    for (int i = 0; i < bound; i++) {
        for (int j = 0; j < 2; j++) {
            sum += arr[i] * (j + 1);
        }
    }
    return sum;
}

int main(int argc, char** argv) {
    // Use argc for pseudo-random but reproducible sizes
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    // Data-dependent sizes to prevent optimization
    int size1 = 100 + (rand() % 100);
    int size2 = 50 + (rand() % 50);
    
    // Allocate arrays
    double* array_a = (double*)malloc(size1 * sizeof(double));
    double* array_b = (double*)malloc(size1 * sizeof(double));
    double* array_c = (double*)malloc(size1 * sizeof(double));
    double* scan_array = (double*)malloc(size2 * sizeof(double));
    
    // Initialize arrays
    for (int i = 0; i < size1; i++) {
        array_a[i] = i * 1.5;
        array_b[i] = i * 2.5;
    }
    
    for (int i = 0; i < size2; i++) {
        scan_array[i] = (i % 3) + 1.0;
    }
    
    // 1. Test with declare target enter - triggers OMP_CLAUSE_ENTER with TO
    #pragma omp target enter data map(to: array_a[0:size1], array_b[0:size1]) \
        map(alloc: array_c[0:size1])
    
    #pragma omp target
    {
        vec_add(size1, array_a, array_b, array_c);
    }
    
    #pragma omp target exit data map(from: array_c[0:size1])
    
    // 2. Test reduction with potential _reductemp_
    double total_sum = 0.0;
    volatile int dyn_bound = size2; // Prevent optimization
    
    #pragma omp parallel for simd reduction(+:total_sum)
    for (int i = 0; i < dyn_bound; i++) {
        // Complex reduction operation
        total_sum += scan_array[i] * (i % 5 + 1);
        if (omp_get_thread_num() % 2 == 0) {
            total_sum += 0.1; // Data-dependent addition
        }
    }
    
    // 3. Test scan directive - triggers _scantemp_
    double prefix_sum = 0.0;
    double scan_result = 0.0;
    
    #pragma omp parallel for simd reduction(+:prefix_sum) \
        scan(inscan: scan_result)
    for (int i = 0; i < size2; i++) {
        // prescan
        scan_array[i] = scan_array[i] + prefix_sum;
        
        #pragma omp scan inclusive(scan_result)
        scan_result = scan_array[i];
        
        // postscan
        prefix_sum = scan_result;
    }
    
    // 4. Test collapse with non-trivial bounds - may trigger _condtemp_
    int outer_bound = 10 + (rand() % 10);
    volatile int inner_bound = 5 + (rand() % 5);
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < outer_bound; i++) {
        for (int j = 0; j < inner_bound; j++) {
            int idx = i * inner_bound + j;
            if (idx < size2) {
                scan_array[idx] += (i + j) * 0.01;
            }
        }
    }
    
    // 5. Complex nested construct with multiple features
    double nested_sum = 0.0;
    double nested_prefix = 0.0;
    
    #pragma omp target teams distribute parallel for simd \
        reduction(+:nested_sum) map(tofrom: nested_sum) \
        num_teams(2) thread_limit(32)
    for (int i = 0; i < size1; i++) {
        nested_sum += array_c[i];
        
        // Conditional operation based on thread/team
        if (omp_get_team_num() % 2 == 0) {
            nested_sum *= 1.001;
        }
    }
    
    // 6. Additional declare target with link clause
    #pragma omp declare target link(array_a)
    
    // Compute checksum to prevent dead code elimination
    double checksum = 0.0;
    for (int i = 0; i < size1 && i < 10; i++) {
        checksum += array_c[i];
    }
    for (int i = 0; i < size2 && i < 10; i++) {
        checksum += scan_array[i];
    }
    
    checksum += total_sum + nested_sum + scan_result;
    
    printf("Checksum: %f\n", checksum);
    
    // Cleanup
    free(array_a);
    free(array_b);
    free(array_c);
    free(scan_array);
    
    return 0;
}

// Dummy function to hint compiler about OpenMP clause types
#ifdef DUMP_OMP
void __attribute__((used)) dump_omp_clauses() {
    // This function doesn't need to do anything
    // Its purpose is to ensure OpenMP constructs aren't optimized away
    asm volatile("" : : : "memory");
}
#endif
