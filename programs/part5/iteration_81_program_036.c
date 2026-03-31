Here's a C++ program specifically designed to trigger the uncovered comparison logic in `double_int::cmp`:

```cpp
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <algorithm>

// Large constants that require both high and low parts
const __int128 LARGE_POS = ((__int128)1 << 80) + 12345;
const __int128 LARGE_NEG = -((__int128)1 << 80) - 67890;
const unsigned __int128 LARGE_UNSIGNED = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) + 0x123456789ABCDEFULL;
const __int128 MIXED_HIGH = ((__int128)0x8000000000000000ULL << 64) + 0x12345678; // High bit set

// Comparator function that mimics the uncovered logic
int compare_int128(__int128 a, __int128 b) {
    // This should trigger the exact comparison pattern
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

// Comparator for unsigned __int128
int compare_uint128(unsigned __int128 a, unsigned __int128 b) {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

// Mixed signed/unsigned comparison
int compare_mixed(__int128 a, unsigned __int128 b) {
    // This should trigger the unsigned cast of high parts
    if ((unsigned __int128)a < b) return -1;
    if ((unsigned __int128)a > b) return 1;
    return 0;
}

// Sorting function that performs many comparisons
void bubble_sort_int128(__int128 arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            // Multiple comparison operations
            if (arr[j] > arr[j + 1]) {
                __int128 temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

// Binary search with wide integer comparisons
int binary_search_int128(__int128 arr[], int n, __int128 target) {
    int left = 0, right = n - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        // Comparison in loop condition
        if (arr[mid] == target) return mid;
        if (arr[mid] < target) left = mid + 1;
        else right = mid - 1;
    }
    return -1;
}

// Function with arithmetic and comparisons
void process_with_arithmetic(__int128 a, __int128 b) {
    __int128 sum = a + b;
    __int128 diff = a - b;
    __int128 shifted = a << 3;
    
    // Range check with large constants
    if (sum >= LARGE_NEG && sum <= LARGE_POS) {
        // Multiple comparisons
        if (diff < 0) {
            if (shifted > sum) {
                // Nested comparison logic
            }
        }
    }
    
    // Overflow check
    __int128 product = a * 2;
    if (product < a && a > 0) {
        // Potential overflow
    }
}

// Unreachable code path with comparisons
void unreachable_comparisons() {
    // These comparisons should still generate code
    if (LARGE_POS > LARGE_UNSIGNED) {
        std::cout << "This should never print\n";
    }
    
    if ((__int128)LARGE_UNSIGNED < LARGE_NEG) {
        std::cout << "This should also never print\n";
    }
}

// qsort comparator for C-style sorting
extern "C" int qsort_comparator(const void* a, const void* b) {
    __int128 val_a = *((__int128*)a);
    __int128 val_b = *((__int128*)b);
    
    // Direct comparison that should trigger the uncovered logic
    if (val_a < val_b) return -1;
    if (val_a > val_b) return 1;
    return 0;
}

int main() {
    // Initialize array with mixed values
    const int ARRAY_SIZE = 10;
    __int128 values[ARRAY_SIZE];
    
    // Mix of positive, negative, and large values
    values[0] = 0;
    values[1] = -1;
    values[2] = 1;
    values[3] = LARGE_POS;
    values[4] = LARGE_NEG;
    values[5] = (__int128)LARGE_UNSIGNED;
    values[6] = MIXED_HIGH;
    values[7] = ((__int128)1 << 64) - 1;
    values[8] = -((__int128)1 << 64);
    values[9] = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) + 0xFFFFFFFFFFFFFFFFULL;
    
    // Perform multiple comparison operations
    
    // 1. Direct comparisons
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = 0; j < ARRAY_SIZE; j++) {
            compare_int128(values[i], values[j]);
        }
    }
    
    // 2. Mixed signed/unsigned comparisons
    unsigned __int128 unsigned_vals[3] = {
        LARGE_UNSIGNED,
        LARGE_UNSIGNED / 2,
        (unsigned __int128)0x123456789ABCDEFULL
    };
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = 0; j < 3; j++) {
            compare_mixed(values[i], unsigned_vals[j]);
            compare_uint128((unsigned __int128)values[i], unsigned_vals[j]);
        }
    }
    
    // 3. Sort using bubble sort (many comparisons)
    __int128 bubble_sort_array[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        bubble_sort_array[i] = values[i];
    }
    bubble_sort_int128(bubble_sort_array, ARRAY_SIZE);
    
    // 4. Sort using qsort
    __int128 qsort_array[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        qsort_array[i] = values[i];
    }
    qsort(qsort_array, ARRAY_SIZE, sizeof(__int128), qsort_comparator);
    
    // 5. Binary search with comparisons
    for (int i = 0; i < ARRAY_SIZE; i++) {
        binary_search_int128(bubble_sort_array, ARRAY_SIZE, values[i]);
    }
    
    // 6. Arithmetic operations with comparisons
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = 0; j < ARRAY_SIZE; j++) {
            process_with_arithmetic(values[i], values[j]);
        }
    }
    
    // 7. Call unreachable function (should still generate comparison code)
    unreachable_comparisons();
    
    // Calculate checksum to prevent optimization removal
    __int128 checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += bubble_sort_array[i];
        checksum += qsort_array[i];
    }
    
    // Also include comparisons in checksum calculation
    for (int i = 1; i < ARRAY_SIZE; i++) {
        if (bubble_sort_array[i] > bubble_sort_array[i-1]) {
            checksum += 1;
        }
    }
    
    // Output simple result
    std::cout << "Checksum (mod 1000): " << (int)(checksum % 1000) << std::endl;
    
    // Additional control flow based on comparisons
    __int128 test_val = ((__int128)1 << 90) + 123;
    if (test_val < LARGE_POS) {
        if (test_val > LARGE_NEG) {
            // Nested comparisons
            __int128 mid = (LARGE_POS + LARGE_NEG) / 2;
            if (test_val == mid) {
                std::cout << "Exact middle found\n";
            } else if (test_val < mid) {
                std::cout << "Below middle\n";
            } else {
                std::cout << "Above middle\n";
            }
        }
    }
    
    // Switch-like behavior using comparisons
    __int128 switch_val = values[3];
    if (switch_val == 0) {
        std::cout << "Zero\n";
    } else if (switch_val > 0 && switch_val < 100) {
        std::cout << "Small positive\n";
    } else if (switch_val >= 100 && switch_val < ((__int128)1 << 50)) {
        std::cout << "Medium positive\n";
    } else if (switch_val >= ((__int128)1 << 50)) {
        std::cout << "Large positive\n";
    } else if (switch_val < 0 && switch_val > -100) {
        std::cout << "Small negative\n";
    } else {
        std::cout << "Large negative\n";
    }
    
    return 0;
}
```

This program specifically targets the uncovered comparison logic by:

1. **Wide Integer Comparisons**: Uses `__int128` and `unsigned __int128` throughout with explicit comparison operations in sorting algorithms, binary search, and direct comparisons.

2. **Mixed Signed/Unsigned Comparisons**: The `compare_mixed()` function and mixed comparisons in main() ensure the unsigned cast of high parts is exercised.

3. **Constant Wide Integers**: Defines large constants like `LARGE_POS`, `LARGE_NEG`, and `LARGE_UNSIGNED` that require both high and low parts.

4. **Arithmetic Result Comparisons**: The `process_with_arithmetic()` function performs arithmetic operations and then compares results, including overflow checks.

5. **Control Flow Based on Comparisons**: Uses comparison results in `if-else` chains, loop conditions, and sorting algorithms. The unreachable code ensures comparison logic is generated even if not executed.

6. **Multiple Comparison Patterns**: Includes bubble sort, qsort, binary search, and nested comparisons to maximize coverage of the comparison logic.

Compile with the suggested options to target the specific `double_int::cmp` logic.
