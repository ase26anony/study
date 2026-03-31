```cpp
#include <cstdint>
#include <cstdlib>
#include <iostream>

// Comparator function that mimics the uncovered double_int::cmp logic
// Compares two __int128 values by splitting into high/low parts
int cmp_int128(__int128 a, __int128 b) {
    // Split into high/low parts (assuming little-endian)
    uint64_t a_low = static_cast<uint64_t>(a & 0xFFFFFFFFFFFFFFFFULL);
    uint64_t b_low = static_cast<uint64_t>(b & 0xFFFFFFFFFFFFFFFFULL);
    uint64_t a_high = static_cast<uint64_t>(a >> 64);
    uint64_t b_high = static_cast<uint64_t>(b >> 64);
    
    // This directly mirrors the uncovered code block
    if ((uint64_t)a_high < (uint64_t)b_high)
        return -1;
    if ((uint64_t)a_high > (uint64_t)b_high)
        return 1;
    if (a_low < b_low)
        return -1;
    if (a_low > b_low)
        return 1;
    return 0;
}

// Alternative comparator for unsigned __int128
int cmp_uint128(unsigned __int128 a, unsigned __int128 b) {
    uint64_t a_low = static_cast<uint64_t>(a & 0xFFFFFFFFFFFFFFFFULL);
    uint64_t b_low = static_cast<uint64_t>(b & 0xFFFFFFFFFFFFFFFFULL);
    uint64_t a_high = static_cast<uint64_t>(a >> 64);
    uint64_t b_high = static_cast<uint64_t>(b >> 64);
    
    if (a_high < b_high)
        return -1;
    if (a_high > b_high)
        return 1;
    if (a_low < b_low)
        return -1;
    if (a_low > b_low)
        return 1;
    return 0;
}

// Sorting function that performs many comparisons
void sort_int128_array(__int128 arr[], size_t n) {
    // Simple bubble sort to ensure many comparisons
    for (size_t i = 0; i < n - 1; i++) {
        for (size_t j = 0; j < n - i - 1; j++) {
            if (cmp_int128(arr[j], arr[j + 1]) > 0) {
                __int128 temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

// Binary search that performs comparisons
int binary_search(__int128 arr[], size_t n, __int128 target) {
    int low = 0, high = n - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        int cmp = cmp_int128(arr[mid], target);
        if (cmp == 0)
            return mid;
        else if (cmp < 0)
            low = mid + 1;
        else
            high = mid - 1;
    }
    return -1;
}

// Function with mixed signed/unsigned comparisons
void mixed_comparisons(__int128 s1, __int128 s2, unsigned __int128 u1, unsigned __int128 u2) {
    // Compare signed values where high word might be negative
    if (s1 < s2) {
        volatile int dummy = 1; // Prevent optimization
    }
    
    // Compare unsigned values
    if (u1 > u2) {
        volatile int dummy = 2;
    }
    
    // Mixed signed/unsigned comparison
    if ((unsigned __int128)s1 < u2) {
        volatile int dummy = 3;
    }
    
    // Another mixed comparison
    if (s1 < (__int128)u2) {
        volatile int dummy = 4;
    }
}

// Range checking with wide integer bounds
bool in_range(__int128 x, __int128 min_val, __int128 max_val) {
    return cmp_int128(x, min_val) >= 0 && cmp_int128(x, max_val) <= 0;
}

// Arithmetic operations with result comparisons
void arithmetic_comparisons(__int128 a, __int128 b) {
    __int128 sum = a + b;
    __int128 diff = a - b;
    __int128 prod = a * (b >> 64); // Partial multiplication to avoid overflow
    
    // Compare arithmetic results
    if (sum > diff) {
        volatile int dummy = 5;
    }
    
    if (prod < a) {
        volatile int dummy = 6;
    }
    
    // Overflow check
    __int128 max_val = ((__int128)1 << 120) - 1;
    if (sum > max_val) {
        volatile int dummy = 7;
    }
}

// Unreachable code path with wide integer comparisons
void unreachable_path() {
    // Large constants with non-zero high and low parts
    const __int128 CONST_A = ((__int128)1 << 80) + 12345;
    const __int128 CONST_B = ((__int128)1 << 79) - 54321;
    const unsigned __int128 CONST_C = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64);
    const unsigned __int128 CONST_D = CONST_C + 0x123456789ABCDEFULL;
    
    // This comparison should always be true, making the else branch unreachable
    if (CONST_A > CONST_B) {
        // This branch is always taken
        volatile int dummy = 8;
    } else {
        // Unreachable code - but the comparison logic should still be generated
        if (CONST_C < CONST_D) {
            volatile int dummy = 9;
        }
    }
}

int main() {
    // Initialize array with mix of positive, negative, and large numbers
    const size_t ARRAY_SIZE = 10;
    __int128 values[ARRAY_SIZE];
    
    // Large constants with non-zero high parts
    const __int128 LARGE_POS = ((__int128)1 << 80) + 12345;
    const __int128 LARGE_NEG = -((__int128)1 << 80) - 67890;
    const unsigned __int128 LARGE_UNSIGNED = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64);
    
    // Initialize array values
    values[0] = 0;
    values[1] = -1;
    values[2] = 100;
    values[3] = -100;
    values[4] = LARGE_POS;
    values[5] = LARGE_NEG;
    values[6] = (__int128)LARGE_UNSIGNED;
    values[7] = -((__int128)LARGE_UNSIGNED >> 2);
    values[8] = ((__int128)1 << 64) + 1;
    values[9] = -((__int128)1 << 64) - 1;
    
    // Perform sorting (many comparisons)
    sort_int128_array(values, ARRAY_SIZE);
    
    // Perform binary searches (more comparisons)
    int idx1 = binary_search(values, ARRAY_SIZE, LARGE_POS);
    int idx2 = binary_search(values, ARRAY_SIZE, 0);
    
    // Mixed signed/unsigned comparisons
    mixed_comparisons(values[0], values[1], 
                     (unsigned __int128)values[4], 
                     (unsigned __int128)values[5]);
    
    // Range checking
    __int128 range_min = -((__int128)1 << 70);
    __int128 range_max = ((__int128)1 << 70);
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        if (in_range(values[i], range_min, range_max)) {
            volatile int dummy = 10 + i;
        }
    }
    
    // Arithmetic operations and comparisons
    arithmetic_comparisons(values[2], values[3]);
    arithmetic_comparisons(LARGE_POS, LARGE_NEG);
    
    // Call unreachable path function
    unreachable_path();
    
    // Compute checksum to verify execution and prevent optimization removal
    unsigned __int128 checksum = 0;
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        checksum += (unsigned __int128)values[i];
    }
    
    // Output simple result
    std::cout << "Sorted array checksum (mod 1000): " 
              << (uint64_t)(checksum % 1000) << std::endl;
    std::cout << "Found LARGE_POS at index: " << idx1 << std::endl;
    std::cout << "Found 0 at index: " << idx2 << std::endl;
    
    return 0;
}
```

This program is designed to trigger the specific uncovered lines in `double-int.cc` by:

1. **Wide Integer Comparisons**: Uses `__int128` and `unsigned __int128` throughout, with explicit comparisons in sorting, binary search, and conditional statements.

2. **Mixed Signed/Unsigned Comparisons**: The `mixed_comparisons()` function performs comparisons between signed and unsigned wide integers, which should trigger the casting logic in the uncovered code.

3. **Constant Wide Integers**: Defines large constants like `LARGE_POS`, `LARGE_NEG`, and `LARGE_UNSIGNED` that have non-zero high parts.

4. **Arithmetic Result Comparisons**: The `arithmetic_comparisons()` function performs arithmetic operations and compares the results.

5. **Control Flow Based on Comparisons**: Uses comparison results in `if` statements, loop conditions, and sorting algorithms.

6. **Unreachable Code Path**: The `unreachable_path()` function contains a comparison that's always true, making the else branch unreachable but ensuring the comparison logic is generated.

The program outputs a checksum to verify execution and prevent compiler optimization from removing the comparison logic.
