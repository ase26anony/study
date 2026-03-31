Here's a program specifically designed to trigger the uncovered comparison logic in double-int.cc:

```cpp
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <algorithm>

// Large constants that require both high and low parts
const __int128 LARGE_POS = ((__int128)1 << 80) + 12345;
const __int128 LARGE_NEG = -((__int128)1 << 80) - 67890;
const unsigned __int128 LARGE_UNSIGNED = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x123456789ABCDEFULL;
const __int128 VERY_LARGE = ((__int128)1 << 100) - 1;
const __int128 VERY_LARGE_NEG = -((__int128)1 << 100) + 1;

// Comparator function that mimics the uncovered logic
int compare_int128(__int128 a, __int128 b) {
    // This should trigger the compiler to use double_int::cmp logic
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

// Comparator for qsort
extern "C" int qsort_compare_int128(const void* a, const void* b) {
    __int128 val_a = *reinterpret_cast<const __int128*>(a);
    __int128 val_b = *reinterpret_cast<const __int128*>(b);
    
    // Mixed signed/unsigned comparisons to trigger casting logic
    if ((unsigned __int128)val_a < (unsigned __int128)val_b) return -1;
    if ((unsigned __int128)val_a > (unsigned __int128)val_b) return 1;
    
    // Additional signed comparison
    if (val_a < val_b) return -1;
    if (val_a > val_b) return 1;
    return 0;
}

// Binary search that performs many comparisons
int binary_search_int128(__int128 arr[], int size, __int128 target) {
    int left = 0;
    int right = size - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        // Multiple comparison operations
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

// Range check function with wide integer bounds
bool in_range(__int128 x, __int128 min_val, __int128 max_val) {
    // Multiple comparisons in conditional
    return (x >= min_val) && (x <= max_val);
}

// Function that performs arithmetic and then compares
void arithmetic_and_compare(__int128 a, __int128 b) {
    __int128 sum = a + b;
    __int128 diff = a - b;
    __int128 shifted = a << 2;
    
    // Comparisons of arithmetic results
    if (sum > diff) {
        // Do something
        volatile int dummy = 0;
        (void)dummy;
    }
    
    if (shifted < b) {
        // Do something else
        volatile int dummy2 = 0;
        (void)dummy2;
    }
    
    // Overflow check simulation
    __int128 product;
    bool overflow = false;
    
    // Simulate multiplication with overflow check
    if (a > 0 && b > 0) {
        if (a > (__INT128_MAX__ / b)) {
            overflow = true;
        }
    } else if (a < 0 && b < 0) {
        if (a < (__INT128_MAX__ / b)) {
            overflow = true;
        }
    }
    
    if (!overflow) {
        product = a * b;
        // Compare product with original values
        if (product > a && product > b) {
            volatile int dummy3 = 0;
            (void)dummy3;
        }
    }
}

// Sorting function using comparison-based algorithm
void bubble_sort_int128(__int128 arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            // Direct comparison that should trigger cmp logic
            if (arr[j] > arr[j + 1]) {
                __int128 temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

// Unreachable code path with wide integer comparisons
void unreachable_comparisons() {
    // These comparisons should generate code but never execute
    if (LARGE_POS > VERY_LARGE) {
        // Dead code
        std::cout << "This should never print\n";
    }
    
    if ((unsigned __int128)LARGE_NEG < LARGE_UNSIGNED) {
        // More dead code
        volatile int dead = 1;
        (void)dead;
    }
}

int main() {
    // Initialize array with mix of positive, negative, and large values
    const int ARRAY_SIZE = 20;
    __int128 values[ARRAY_SIZE];
    
    // Fill array with various values
    values[0] = 0;
    values[1] = 1;
    values[2] = -1;
    values[3] = LARGE_POS;
    values[4] = LARGE_NEG;
    values[5] = __INT128_MAX__;
    values[6] = __INT128_MIN__;
    values[7] = LARGE_POS / 2;
    values[8] = LARGE_NEG / 2;
    values[9] = ((__int128)1 << 64) - 1;
    values[10] = -((__int128)1 << 64);
    values[11] = VERY_LARGE;
    values[12] = VERY_LARGE_NEG;
    values[13] = 100;
    values[14] = -100;
    values[15] = ((__int128)1 << 90);
    values[16] = -((__int128)1 << 90);
    values[17] = 0x7FFFFFFFFFFFFFFF;  // Max int64_t
    values[18] = -0x8000000000000000; // Min int64_t
    values[19] = (__int128)LARGE_UNSIGNED;
    
    // Perform various comparison operations
    
    // 1. Sort using bubble sort (many comparisons)
    bubble_sort_int128(values, ARRAY_SIZE);
    
    // 2. Also sort using qsort with comparator
    __int128 values2[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        values2[i] = values[ARRAY_SIZE - 1 - i];  // Reverse order
    }
    qsort(values2, ARRAY_SIZE, sizeof(__int128), qsort_compare_int128);
    
    // 3. Binary search for various values
    int found1 = binary_search_int128(values, ARRAY_SIZE, LARGE_POS);
    int found2 = binary_search_int128(values, ARRAY_SIZE, -1);
    int found3 = binary_search_int128(values, ARRAY_SIZE, 0);
    
    // 4. Range checks
    bool in_range1 = in_range(LARGE_POS, LARGE_NEG, VERY_LARGE);
    bool in_range2 = in_range(0, LARGE_NEG, LARGE_POS);
    bool in_range3 = in_range(VERY_LARGE_NEG, VERY_LARGE_NEG, VERY_LARGE);
    
    // 5. Arithmetic and comparisons
    for (int i = 0; i < ARRAY_SIZE - 1; i++) {
        arithmetic_and_compare(values[i], values[i + 1]);
        
        // Direct comparisons in loop
        if (values[i] == values[i + 1]) {
            // Shouldn't happen with our data
            volatile int eq = 1;
            (void)eq;
        }
        
        // Mixed signed/unsigned comparison
        if ((unsigned __int128)values[i] < (unsigned __int128)values[i + 1]) {
            volatile int lt = 1;
            (void)lt;
        }
    }
    
    // 6. Call unreachable function (should generate comparison code)
    unreachable_comparisons();
    
    // 7. Complex conditional with wide integers
    __int128 checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        // Conditional based on comparison
        if (values[i] > 0) {
            checksum += values[i] % 1000;  // Reduce size
        } else if (values[i] < 0) {
            checksum -= (-values[i]) % 1000;
        }
        
        // Nested comparisons
        if (i > 0) {
            if (values[i] > values[i-1]) {
                checksum += i;
            } else if (values[i] < values[i-1]) {
                checksum -= i;
            }
        }
    }
    
    // 8. Switch-like logic using comparisons
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (values[i] == 0) {
            checksum += 1;
        } else if (values[i] > 0 && values[i] < 100) {
            checksum += 2;
        } else if (values[i] >= 100 && values[i] < 10000) {
            checksum += 3;
        } else if (values[i] >= 10000) {
            checksum += 4;
        } else if (values[i] < 0 && values[i] > -100) {
            checksum += 5;
        } else {
            checksum += 6;
        }
    }
    
    // Output checksum to prevent optimization removal
    std::cout << "Checksum: " << (long long)(checksum % 1000000) << std::endl;
    
    // Additional output to show we found items
    std::cout << "Found LARGE_POS at index: " << found1 << std::endl;
    std::cout << "Found -1 at index: " << found2 << std::endl;
    std::cout << "Found 0 at index: " << found3 << std::endl;
    
    return 0;
}
```

This program is designed to maximize coverage of the specific comparison logic:

1. **Wide Integer Comparisons**: Uses `__int128` throughout with explicit comparisons in sorting, searching, and conditional logic.

2. **Mixed Signed/Unsigned Comparisons**: Includes explicit casts to `unsigned __int128` in comparisons to trigger the casting logic in the uncovered code.

3. **Constant Wide Integers**: Defines and uses large constants like `LARGE_POS`, `LARGE_NEG`, and `LARGE_UNSIGNED` that require both high and low parts.

4. **Arithmetic Result Comparisons**: The `arithmetic_and_compare` function performs arithmetic operations and then compares results.

5. **Control Flow Based on Comparisons**: Uses comparison results in `if-else` chains, loop conditions, and sorting algorithms.

6. **Unreachable Code Path**: The `unreachable_comparisons` function contains comparisons that should generate code but never execute.

7. **Prevents Optimization**: The checksum calculation and output ensure the compiler can't optimize away the comparison logic.

Compile with the suggested options to maximize coverage of the `double_int::cmp` method logic.
