Here's a C++ program specifically designed to trigger the uncovered lines in double-int.cc:

```cpp
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <algorithm>

// Large constants that require both high and low parts
const __int128 LARGE_POS = ((__int128)1 << 80) + 12345;
const __int128 LARGE_NEG = -((__int128)1 << 80) - 54321;
const unsigned __int128 LARGE_UNSIGNED = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x123456789ABCDEF0ULL;
const __int128 MEDIUM_POS = ((__int128)1 << 60);
const __int128 MEDIUM_NEG = -((__int128)1 << 60);

// Comparator function that mimics the uncovered logic
int compare_int128(const void* a, const void* b) {
    __int128 val_a = *reinterpret_cast<const __int128*>(a);
    __int128 val_b = *reinterpret_cast<const __int128*>(b);
    
    // This should trigger the specific comparison path
    if (val_a < val_b) return -1;
    if (val_a > val_b) return 1;
    return 0;
}

// C++ comparator for std::sort
struct Int128Comparator {
    bool operator()(__int128 a, __int128 b) const {
        // Multiple comparison operations
        return a < b;
    }
};

// Function with mixed signed/unsigned comparisons
void mixed_comparisons(__int128 signed_val, unsigned __int128 unsigned_val) {
    // These comparisons should trigger the unsigned cast logic
    if ((unsigned __int128)signed_val < unsigned_val) {
        // This path might be taken
        volatile int dummy = 1;
        (void)dummy;
    }
    
    if (signed_val < (__int128)unsigned_val) {
        volatile int dummy = 2;
        (void)dummy;
    }
    
    // Compare with constants
    if (signed_val > LARGE_POS) {
        // Unreachable with our test data, but ensures code generation
        volatile int dummy = 999;
        (void)dummy;
    }
}

// Arithmetic operations with result comparisons
void arithmetic_comparisons(__int128 a, __int128 b) {
    __int128 sum = a + b;
    __int128 diff = a - b;
    __int128 shifted = a << 2;
    
    // Range check with wide integer bounds
    const __int128 MIN_VAL = -((__int128)1 << 100);
    const __int128 MAX_VAL = ((__int128)1 << 100);
    
    if (sum >= MIN_VAL && sum <= MAX_VAL) {
        volatile int dummy = 3;
        (void)dummy;
    }
    
    // Overflow-like check
    if (a > 0 && b > 0 && sum < a) {
        // Potential overflow
        volatile int dummy = 4;
        (void)dummy;
    }
    
    // Compare arithmetic results
    if (diff < shifted) {
        volatile int dummy = 5;
        (void)dummy;
    }
}

// Binary search that performs many comparisons
int binary_search(__int128 arr[], int size, __int128 target) {
    int left = 0;
    int right = size - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        // This comparison should trigger the target logic
        if (arr[mid] == target) {
            return mid;
        }
        
        // More comparisons
        if (arr[mid] < target) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    return -1;
}

// Simple bubble sort (not efficient but makes many comparisons)
void bubble_sort(__int128 arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            // Direct comparison - should trigger the uncovered code
            if (arr[j] > arr[j + 1]) {
                __int128 temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

// Function with control flow based on comparisons
int classify_value(__int128 val) {
    // Chain of comparisons
    if (val < LARGE_NEG) {
        return 1;
    } else if (val < MEDIUM_NEG) {
        return 2;
    } else if (val < 0) {
        return 3;
    } else if (val == 0) {
        return 4;
    } else if (val < MEDIUM_POS) {
        return 5;
    } else if (val < LARGE_POS) {
        return 6;
    } else {
        return 7;
    }
}

int main() {
    // Initialize array with mix of values
    const int ARRAY_SIZE = 20;
    __int128 values[ARRAY_SIZE];
    
    // Fill with various values including negatives and large numbers
    values[0] = 0;
    values[1] = -1;
    values[2] = 1;
    values[3] = LARGE_NEG;
    values[4] = LARGE_POS;
    values[5] = MEDIUM_NEG;
    values[6] = MEDIUM_POS;
    values[7] = ((__int128)1 << 70);
    values[8] = -((__int128)1 << 70);
    values[9] = 1234567890123456789LL;
    values[10] = -1234567890123456789LL;
    values[11] = ((__int128)1 << 90) + 555;
    values[12] = -((__int128)1 << 90) - 777;
    values[13] = 42;
    values[14] = -42;
    values[15] = ((__int128)0x7FFFFFFFFFFFFFFFLL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    values[16] = -((__int128)0x7FFFFFFFFFFFFFFFLL << 64);
    values[17] = 100;
    values[18] = -100;
    values[19] = ((__int128)1 << 63);
    
    // Test mixed signed/unsigned comparisons
    unsigned __int128 unsigned_val = LARGE_UNSIGNED;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        mixed_comparisons(values[i], unsigned_val);
    }
    
    // Test arithmetic comparisons
    for (int i = 0; i < ARRAY_SIZE - 1; i++) {
        arithmetic_comparisons(values[i], values[i + 1]);
    }
    
    // Test classification
    int class_counts[8] = {0};
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int cls = classify_value(values[i]);
        if (cls >= 1 && cls <= 7) {
            class_counts[cls]++;
        }
    }
    
    // Sort using qsort (C style)
    __int128 qsort_values[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        qsort_values[i] = values[i];
    }
    qsort(qsort_values, ARRAY_SIZE, sizeof(__int128), compare_int128);
    
    // Sort using bubble sort (makes many comparisons)
    __int128 bubble_values[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        bubble_values[i] = values[i];
    }
    bubble_sort(bubble_values, ARRAY_SIZE);
    
    // Sort using std::sort (C++ style)
    __int128 std_values[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        std_values[i] = values[i];
    }
    std::sort(std_values, std_values + ARRAY_SIZE, Int128Comparator());
    
    // Test binary search
    int found_index = binary_search(std_values, ARRAY_SIZE, MEDIUM_POS);
    
    // Calculate checksum to prevent optimization removal
    __int128 checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += qsort_values[i];
        checksum += bubble_values[i];
        checksum += std_values[i];
    }
    
    // Add unreachable code path with wide integer comparison
    // This ensures the comparison logic is generated even if not executed
    if (LARGE_POS > LARGE_UNSIGNED) {
        // Dead code - should never execute
        std::cout << "This should never print\n";
        checksum = 0;  // Try to affect checksum
    }
    
    // Output simple result to verify execution
    std::cout << "Checksum modulo 1000: " << (int)(checksum % 1000) << std::endl;
    std::cout << "Binary search result index: " << found_index << std::endl;
    
    // Output classification counts
    std::cout << "Classification counts: ";
    for (int i = 1; i <= 7; i++) {
        std::cout << class_counts[i] << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
```

This program includes:

1. **Wide Integer Comparisons**: Multiple comparison operations using `__int128` and `unsigned __int128` types in sorting algorithms (qsort, bubble sort, std::sort) and binary search.

2. **Mixed Signed/Unsigned Comparisons**: The `mixed_comparisons()` function explicitly compares signed and unsigned 128-bit integers, which should trigger the unsigned cast logic in the uncovered code.

3. **Constant Wide Integers**: Uses large constants like `LARGE_POS`, `LARGE_NEG`, and `LARGE_UNSIGNED` that require both high and low parts.

4. **Arithmetic Result Comparisons**: The `arithmetic_comparisons()` function performs arithmetic operations and compares results, including range checks with wide integer bounds.

5. **Control Flow Based on Comparisons**: The `classify_value()` function uses a chain of comparisons to direct program flow, and sorting algorithms use comparisons for control flow.

6. **Unreachable Code Path**: The final `if (LARGE_POS > LARGE_UNSIGNED)` comparison creates dead code to ensure the comparison logic is generated.

7. **Checksum Calculation**: Prevents optimization removal by calculating and outputting a checksum.

Compile with the suggested options to maximize coverage of the target comparison logic.
