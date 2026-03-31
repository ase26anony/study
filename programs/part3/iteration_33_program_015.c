/* Compile with: g++ -O1 -fopenmp -fdump-tree-omplower -fdump-tree-ompexp -fdump-tree-all -o test_omp test_omp.cc */
/* Also try: g++ -O2 -fopenmp -fopenmp-simd -fdump-tree-all -foffload=disable -o test_omp test_omp.cc */
/* And: g++ -O0 -fopenmp -foffload-abi=lp64 -fdump-tree-original -o test_omp test_omp.cc */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* For _SCANTEMP_ and _REDUCTEMP_ */
void omp_scan_reduction(int n, double* arr, double* prefix) {
    volatile int vn = n; /* Prevent optimization */
    double sum = 0.0;
    
    /* This should generate _scantemp_ and _reductemp_ clauses */
    #pragma omp parallel for simd reduction(+:sum) \
            simdlen(4) safelen(8) \
            scan(inscan:prefix_sum:sum)
    for (int i = 0; i < vn; i++) {
        double val = arr[i];
        
        #pragma omp scan inclusive(prefix_sum)
        sum += val;
        prefix[i] = sum;
        
        /* Data-dependent control flow for condition temporaries */
        if (i % (omp_get_thread_num() + 1) == 0) {
            prefix[i] *= 1.01;
        }
    }
}

/* For _CONDTEMP_ with collapse */
void nested_collapse(int m, int n, double* matrix) {
    volatile int vm = m, vn = n;
    
    /* Complex loop bound may generate _condtemp_ */
    #pragma omp parallel for collapse(2) \
            schedule(dynamic, 4) \
            num_threads(omp_get_max_threads() > 2 ? omp_get_max_threads() : 2)
    for (int i = 0; i < vm + (vm % 3); i++) {
        for (int j = 0; j < vn * (i % 2 + 1); j++) {
            int idx = i * vn + j;
            if (idx < m * n) {
                matrix[idx] = (i + j) * 0.5;
                
                /* More complex condition for temporaries */
                if (i > j && (i * j) % 7 == omp_get_thread_num()) {
                    matrix[idx] += 1.0;
                }
            }
        }
    }
}

/* For ENTER clause with to() mapper */
#pragma omp declare target enter(vec_add) to(p1) to(p2)
void vec_add(double* a, double* b, double* c, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Another function with enter and link */
#pragma omp declare target enter(scale_array) \
        to(scale_factor) link(scale_array)
static double scale_factor = 2.5;
void scale_array(double* arr, int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        arr[i] *= scale_factor;
    }
}

int main(int argc, char** argv) {
    /* Use argc for pseudo-random but reproducible sizes */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Variable sizes prevent optimization */
    int n = 100 + (rand() % 100);
    int m = 50 + (rand() % 50);
    
    /* Allocate arrays */
    double* arr = (double*)malloc(n * sizeof(double));
    double* prefix = (double*)malloc(n * sizeof(double));
    double* matrix = (double*)malloc(m * n * sizeof(double));
    double* a = (double*)malloc(n * sizeof(double));
    double* b = (double*)malloc(n * sizeof(double));
    double* c = (double*)malloc(n * sizeof(double));
    
    if (!arr || !prefix || !matrix || !a || !b || !c) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        arr[i] = i * 0.5;
        a[i] = i * 1.0;
        b[i] = i * 2.0;
    }
    
    /* 1. Trigger _scantemp_ and _reductemp_ */
    printf("Running scan with reduction...\n");
    omp_scan_reduction(n, arr, prefix);
    
    /* 2. Trigger _condtemp_ with collapse */
    printf("Running nested collapse...\n");
    nested_collapse(m, n, matrix);
    
    /* 3. Trigger ENTER clause with to() */
    printf("Running target with enter...\n");
    
    /* Map data for target region */
    double *p1 = a, *p2 = b, *p3 = c;
    #pragma omp target enter data map(to: p1[0:n], p2[0:n]) map(alloc: p3[0:n])
    
    #pragma omp target teams distribute parallel for \
            map(tofrom: p3[0:n]) \
            reduction(+:seed)
    for (int i = 0; i < n; i++) {
        p3[i] = p1[i] + p2[i] + (seed % 10) * 0.1;
        seed += i; /* Use seed in computation */
    }
    
    #pragma omp target exit data map(from: p3[0:n]) map(release: p1[0:n], p2[0:n])
    
    /* 4. Combined construct with multiple clauses */
    printf("Running combined construct...\n");
    double total = 0.0;
    volatile int chunk = 16;
    
    #pragma omp target teams distribute parallel for simd \
            map(tofrom: total) map(to: matrix[0:m*n]) \
            reduction(+:total) collapse(2) \
            num_teams(2) thread_limit(64) \
            private(seed) lastprivate(chunk)
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            int idx = i * n + j;
            total += matrix[idx];
            
            /* Complex condition for temporaries */
            if ((i * j) % 13 == (omp_get_team_num() + omp_get_thread_num()) % 13) {
                total += 0.1;
            }
            
            if (i == m - 1 && j == n - 1) {
                chunk = idx;
            }
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < n; i += 10) {
        checksum += prefix[i] + c[i];
    }
    for (int i = 0; i < m * n; i += 20) {
        checksum += matrix[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Total: %f\n", total);
    printf("Final chunk: %d\n", chunk);
    
    /* Cleanup */
    free(arr);
    free(prefix);
    free(matrix);
    free(a);
    free(b);
    free(c);
    
    return 0;
}

/* Dummy function to hint compiler about OpenMP clauses */
#ifdef DUMP_OMP
void __attribute__((used)) dump_omp_clauses() {
    /* These declarations might help keep OpenMP structures */
    asm volatile ("# OMP Clause references" : : 
                  "r"((long)OMP_CLAUSE__REDUCTEMP_),
                  "r"((long)OMP_CLAUSE__CONDTEMP_),
                  "r"((long)OMP_CLAUSE__SCANTEMP_),
                  "r"((long)OMP_CLAUSE_ENTER));
}
#endif
