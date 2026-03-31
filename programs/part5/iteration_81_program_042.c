Here's a program specifically designed to trigger the uncovered lines in double-int.cc by performing wide integer comparisons with mixed signed/unsigned operations:

```cpp
#include <iostream>
#include <cstdlib>
#include <cstdint>

// Large constants that require both high and low parts
const __int128 LARGE_POS = ((__int128)1 << 80) + 12345;
const __int128 LARGE_NEG = -((__int128)1 << 80) - 54321;
const unsigned __int128 LARGE_UNSIGNED = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x123456789ABCDEF0ULL;
const __int128 MEDIUM_POS = ((__int128)1 << 60);
const __int128 MEDIUM_NEG = -((__int128)1 << 60);

// Comparator function that mimics the uncovered logic
int compare_int128(__int128 a, __int128 b) {
    // This should trigger the exact comparison pattern
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

// Comparator for qsort
extern "C" int qsort_compare_int128(const void* a, const void* b) {
    __int128 val_a = *reinterpret_cast<const __int128*>(a);
    __int128 val_b = *reinterpret_cast<const __int128*>(b);
    return compare_int128(val_a, val_b);
}

// Mixed signed/unsigned comparison function
int compare_mixed(__int128 signed_val, unsigned __int128 unsigned_val) {
    // This should trigger the unsigned cast comparisons
    if ((unsigned __int128)signed_val < unsigned_val) return -1;
    if ((unsigned __int128)signed_val > unsigned_val) return 1;
    return 0;
}

// Binary search on __int128 array
int binary_search(__int128 arr[], int size, __int128 target) {
    int left = 0;
    int right = size - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        // Multiple comparisons here
        if (arr[mid] == target) {
            return mid;
        } else if (arr[mid] < target) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return -1;
}

// Insertion sort with __int128 comparisons
void insertion_sort(__int128 arr[], int n) {
    for (int i = 1; i < n; i++) {
        __int128 key = arr[i];
        int j = i - 1;
        
        // Multiple comparisons in the inner loop
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

// Range check function with wide integer bounds
bool in_range(__int128 x, __int128 min_val, __int128 max_val) {
    return x >= min_val && x <= max_val;
}

// Arithmetic operations followed by comparisons
void arithmetic_comparisons(__int128 a, __int128 b) {
    __int128 sum = a + b;
    __int128 diff = a - b;
    __int128 shifted = a << 3;
    
    // Multiple comparisons of arithmetic results
    if (sum > diff) {
        // Do something
        volatile int dummy = 1;
        (void)dummy;
    }
    
    if (shifted < a) {
        // Do something else
        volatile int dummy = 2;
        (void)dummy;
    }
    
    // Overflow check
    __int128 product = a * 2;
    if (product < a) {
        // Potential overflow
        volatile int dummy = 3;
        (void)dummy;
    }
}

// Unreachable code path with wide integer comparisons
void unreachable_path() {
    const __int128 CONST_A = ((__int128)1 << 120);
    const __int128 CONST_B = ((__int128)1 << 121);
    
    // This comparison should always be false, creating dead code
    if (CONST_A > CONST_B) {
        // Dead code that should still generate comparison logic
        std::cout << "This should never print" << std::endl;
        
        // More comparisons in dead code
        __int128 temp = CONST_A - CONST_B;
        if (temp < 0) {
            volatile int dummy = 999;
            (void)dummy;
        }
    }
}

int main() {
    // Initialize array with mixed values
    const int ARRAY_SIZE = 20;
    __int128 values[ARRAY_SIZE];
    
    // Fill array with various values
    values[0] = LARGE_POS;
    values[1] = LARGE_NEG;
    values[2] = 0;
    values[3] = 100;
    values[4] = -100;
    values[5] = MEDIUM_POS;
    values[6] = MEDIUM_NEG;
    values[7] = ((__int128)1 << 70);
    values[8] = -((__int128)1 << 70);
    values[9] = 0x7FFFFFFFFFFFFFFFLL;  // Max int64_t
    values[10] = -0x8000000000000000LL; // Min int64_t
    values[11] = values[0] + values[1];
    values[12] = values[5] * 3;
    values[13] = values[6] / 2;
    values[14] = ((__int128)1 << 90) | 0x12345;
    values[15] = -((__int128)1 << 88);
    values[16] = 0;
    values[17] = 1;
    values[18] = -1;
    values[19] = ((__int128)1 << 100) - 1;
    
    // Perform various comparison operations
    
    // 1. Sort using insertion sort (many comparisons)
    __int128 sorted[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sorted[i] = values[i];
    }
    insertion_sort(sorted, ARRAY_SIZE);
    
    // 2. Sort using qsort (different comparison pattern)
    __int128 qsorted[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        qsorted[i] = values[i];
    }
    qsort(qsorted, ARRAY_SIZE, sizeof(__int128), qsort_compare_int128);
    
    // 3. Binary search for various values
    int found_indices[5];
    found_indices[0] = binary_search(sorted, ARRAY_SIZE, 0);
    found_indices[1] = binary_search(sorted, ARRAY_SIZE, LARGE_POS);
    found_indices[2] = binary_search(sorted, ARRAY_SIZE, LARGE_NEG);
    found_indices[3] = binary_search(sorted, ARRAY_SIZE, 100);
    found_indices[4] = binary_search(sorted, ARRAY_SIZE, -100);
    
    // 4. Mixed signed/unsigned comparisons
    unsigned __int128 unsigned_vals[3];
    unsigned_vals[0] = LARGE_UNSIGNED;
    unsigned_vals[1] = 0xFFFFFFFFFFFFFFFFULL;
    unsigned_vals[2] = ((unsigned __int128)1 << 80);
    
    int mixed_comparisons[6];
    mixed_comparisons[0] = compare_mixed(values[0], unsigned_vals[0]);
    mixed_comparisons[1] = compare_mixed(values[1], unsigned_vals[1]);
    mixed_comparisons[2] = compare_mixed(values[5], unsigned_vals[2]);
    mixed_comparisons[3] = compare_mixed(0, unsigned_vals[0]);
    mixed_comparisons[4] = compare_mixed(-1, unsigned_vals[1]);
    mixed_comparisons[5] = compare_mixed(values[19], unsigned_vals[2]);
    
    // 5. Range checks
    bool range_results[4];
    range_results[0] = in_range(values[0], LARGE_NEG, LARGE_POS);
    range_results[1] = in_range(values[1], LARGE_NEG, LARGE_POS);
    range_results[2] = in_range(0, MEDIUM_NEG, MEDIUM_POS);
    range_results[3] = in_range(values[19], 0, LARGE_POS);
    
    // 6. Arithmetic operations with comparisons
    for (int i = 0; i < ARRAY_SIZE - 1; i++) {
        arithmetic_comparisons(values[i], values[i + 1]);
    }
    
    // 7. Direct comparisons in control flow
    int comparison_results = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = i + 1; j < ARRAY_SIZE; j++) {
            if (values[i] < values[j]) {
                comparison_results++;
            } else if (values[i] > values[j]) {
                comparison_results--;
            }
            
            // More complex condition
            if ((values[i] > 0 && values[j] < 0) || 
                (values[i] < 0 && values[j] > 0)) {
                comparison_results += 2;
            }
        }
    }
    
    // 8. Call unreachable path function
    unreachable_path();
    
    // Calculate checksum to prevent optimization removal
    __int128 checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += sorted[i];
        checksum += qsorted[i];
    }
    
    // Add comparison results to checksum
    checksum += comparison_results;
    for (int i = 0; i < 6; i++) {
        checksum += mixed_comparisons[i];
    }
    
    // Output simple result
    std::cout << "Checksum modulo 1000: " << (int)(checksum % 1000) << std::endl;
    
    return 0;
}
```

This program specifically targets the uncovered comparison logic by:

1. **Wide Integer Comparisons**: Uses `__int128` and `unsigned __int128` throughout, with explicit comparisons in sorting algorithms, binary search, and direct comparisons.

2. **Mixed Signed/Unsigned Comparisons**: The `compare_mixed()` function explicitly casts signed values to unsigned for comparison, matching the pattern in the uncovered code.

3. **Constant Wide Integers**: Uses large constants like `((__int128)1 << 80)` and `((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64)` that require both high and low parts.

4. **Arithmetic Result Comparisons**: The `arithmetic_comparisons()` function performs arithmetic operations and then compares the results.

5. **Control Flow Based on Comparisons**: Uses comparison results in `if-else` chains, loop conditions (binary search, insertion sort), and sorting algorithms.

6. **Unreachable Code Path**: The `unreachable_path()` function contains comparisons that should never be true, ensuring the comparison logic is generated even if not executed.

7. **Checksum Calculation**: Prevents optimization removal by calculating and outputting a checksum based on all operations.

Compile with the recommended options to maximize coverage of the `double_int::cmp` logic.
