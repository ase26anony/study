```cpp
#include <cstdio>
#include <cstdlib>
#include <omp.h>

// Helper function to prevent constant folding
volatile int get_seed() {
    return 42;
}

int main() {
    const int N = 100;
    int arr1[N], arr2[N], arr3[N];
    int sum = 0;
    
    // Initialize arrays with non-constant values
    volatile int seed = get_seed();
    for (int i = 0; i < N; ++i) {
        arr1[i] = (i + seed) % 100;
        arr2[i] = (i * 2 + seed) % 100;
        arr3[i] = (i * 3 + seed) % 100;
    }
    
    // Use single-threaded parallel region to ensure pretty-printer sees all constructs
    #pragma omp parallel num_threads(1)
    {
        #pragma omp single nowait
        {
            // Task 1: depend(in) - read-only dependency
            #pragma omp task depend(in: arr1[0:N]) shared(arr1, sum)
            {
                int local_sum = 0;
                for (int i = 0; i < N; ++i) {
                    local_sum += arr1[i];  // Read from arr1
                }
                #pragma omp atomic
                sum += local_sum;
            }
            
            // Task 2: depend(out) - write-only dependency
            #pragma omp task depend(out: arr2[0:N]) shared(arr2, sum)
            {
                for (int i = 0; i < N; ++i) {
                    arr2[i] = i * 3;  // Write to arr2
                }
                int local_sum = 0;
                for (int i = 0; i < N; ++i) {
                    local_sum += arr2[i];
                }
                #pragma omp atomic
                sum += local_sum;
            }
            
            // Task 3: depend(inout) - read-write dependency
            #pragma omp task depend(inout: arr3[0:N]) shared(arr3, sum)
            {
                for (int i = 0; i < N; ++i) {
                    arr3[i] += 1;  // Read and write arr3
                }
                int local_sum = 0;
                for (int i = 0; i < N; ++i) {
                    local_sum += arr3[i];
                }
                #pragma omp atomic
                sum += local_sum;
            }
            
            // Task 4: depend(mutexinoutset) - mutex dependency
            #pragma omp task depend(mutexinoutset: arr1[0:N]) shared(arr1, sum)
            {
                for (int i = 0; i < N; ++i) {
                    arr1[i] *= 2;  // Update arr1
                }
                int local_sum = 0;
                for (int i = 0; i < N; ++i) {
                    local_sum += arr1[i];
                }
                #pragma omp atomic
                sum += local_sum;
            }
            
            // Task 5: depend(inoutset) - set-based dependency
            #pragma omp task depend(inoutset: arr2[0:N]) shared(arr2, sum)
            {
                for (int i = 0; i < N; ++i) {
                    arr2[i] -= 5;  // Update arr2
                }
                int local_sum = 0;
                for (int i = 0; i < N; ++i) {
                    local_sum += arr2[i];
                }
                #pragma omp atomic
                sum += local_sum;
            }
            
            // Taskwait with destroy dependency
            #pragma omp taskwait depend(destroy: arr3[0:N])
            
            // Additional computation after taskwait
            #pragma omp task shared(arr1, arr2, arr3, sum)
            {
                int local_sum = 0;
                for (int i = 0; i < N; ++i) {
                    local_sum += arr1[i] + arr2[i] + arr3[i];
                }
                #pragma omp atomic
                sum += local_sum;
            }
        }
    }
    
    // Final checksum computation and output
    int final_sum = sum;
    for (int i = 0; i < N; ++i) {
        final_sum += arr1[i] + arr2[i] + arr3[i];
    }
    
    printf("Checksum: %d\n", final_sum);
    return 0;
}
```

**Key Features:**

1. **All Required Depend Types**: The code includes:
   - `depend(in: arr1)` (Task 1)
   - `depend(out: arr2)` (Task 2)
   - `depend(inout: arr3)` (Task 3)
   - `depend(mutexinoutset: arr1)` (Task 4)
   - `depend(inoutset: arr2)` (Task 5)
   - `depend(destroy: arr3)` (taskwait)

2. **Valid OpenMP Semantics**:
   - Uses array sections `arr[0:N]` for proper dependency specification
   - All tasks perform actual computations with their dependent arrays
   - Proper use of `shared` and `atomic` for data sharing

3. **Optimization Prevention**:
   - `volatile` seed variable
   - Complex array initialization using runtime value
   - Final checksum computation and output

4. **Compilation Recommendations**:
   - Use `g++ -O1 -fopenmp -fdump-tree-omplower` to see OpenMP lowering
   - Use `g++ -O2 -fopenmp -g -fdump-tree-original` to trigger pretty-printer
   - Use `g++ -O0 -fopenmp -fsanitize=address` for debugging

The code should trigger all the uncovered pretty-printer cases while maintaining valid OpenMP semantics and preventing compiler optimizations from eliminating the tasks.
