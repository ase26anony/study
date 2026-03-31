```cpp
#include <cstdio>
#include <cstdlib>
#include <omp.h>

// Helper function to prevent constant folding
volatile int get_seed() {
    static volatile int seed = 42;
    return seed;
}

int main() {
    const int N = 100;
    int arr1[N], arr2[N], arr3[N];
    volatile int seed = get_seed();
    
    // Initialize arrays with non-constant values
    for (int i = 0; i < N; ++i) {
        arr1[i] = i + seed;
        arr2[i] = i * 2 + seed;
        arr3[i] = i * 3 + seed;
    }
    
    int sum = 0;
    
    // Use single-threaded parallel region to ensure pretty-printer sees all constructs
    #pragma omp parallel num_threads(1)
    {
        #pragma omp single
        {
            // Task 1: depend(in: arr1)
            #pragma omp task depend(in: arr1[0:N])
            {
                int local_sum = 0;
                for (int i = 0; i < N; ++i) {
                    local_sum += arr1[i];  // Read from arr1
                }
                #pragma omp atomic
                sum += local_sum;
            }
            
            // Task 2: depend(out: arr2)
            #pragma omp task depend(out: arr2[0:N])
            {
                for (int i = 0; i < N; ++i) {
                    arr2[i] = i + seed + 1;  // Write to arr2
                }
            }
            
            // Task 3: depend(inout: arr3)
            #pragma omp task depend(inout: arr3[0:N])
            {
                for (int i = 0; i < N; ++i) {
                    arr3[i] += i;  // Read and write arr3
                }
            }
            
            // Task 4: depend(mutexinoutset: arr1)
            #pragma omp task depend(mutexinoutset: arr1[0:N])
            {
                for (int i = 0; i < N; ++i) {
                    arr1[i] *= 2;  // Update arr1
                }
            }
            
            // Task 5: depend(inoutset: arr2)
            #pragma omp task depend(inoutset: arr2[0:N])
            {
                for (int i = 0; i < N; ++i) {
                    arr2[i] -= i;  // Update arr2
                }
            }
            
            // Taskwait with destroy dependency
            #pragma omp taskwait depend(destroy: arr3)
            
            // Additional task to ensure arr3 is used after destroy
            #pragma omp task
            {
                int local_sum = 0;
                for (int i = 0; i < N; ++i) {
                    local_sum += arr3[i];
                }
                #pragma omp atomic
                sum += local_sum;
            }
            
            #pragma omp taskwait
        }
    }
    
    // Final checksum computation to prevent dead code elimination
    int final_checksum = sum;
    for (int i = 0; i < N; ++i) {
        final_checksum += arr1[i] + arr2[i] + arr3[i];
    }
    
    printf("Checksum: %d\n", final_checksum);
    
    return 0;
}
```

**Key Design Rationale:**

1. **Complete Coverage of All `depend` Types:**
   - `depend(in: arr1)` - Task 1
   - `depend(out: arr2)` - Task 2
   - `depend(inout: arr3)` - Task 3
   - `depend(mutexinoutset: arr1)` - Task 4
   - `depend(inoutset: arr2)` - Task 5
   - `depend(destroy: arr3)` - `taskwait` construct

2. **Preventing Optimizations:**
   - `volatile` seed variable prevents constant folding
   - Atomic operations on shared `sum` variable
   - Final checksum computation and printf ensure all tasks execute
   - Array operations are non-trivial but valid

3. **Valid OpenMP Semantics:**
   - All tasks are created within an `omp single` region
   - Proper array section syntax `arr[0:N]` for depend clauses
   - Data dependencies create valid execution ordering
   - `taskwait` ensures synchronization

4. **Compilation Recommendations:**
   - Compile with: `g++ -O1 -fopenmp -fdump-tree-omplower -o test test.cpp`
   - For pretty-printer activation: `g++ -O2 -fopenmp -g -fdump-tree-original test.cpp`
   - For diagnostics: `g++ -O0 -fopenmp -fsanitize=address test.cpp`

This program should trigger the pretty-printer logic for all uncovered lines in `cxx-pretty-print.cc` when compiled with OpenMP support and appropriate debugging flags.
