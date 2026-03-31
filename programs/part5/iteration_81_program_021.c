```cpp
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <algorithm>

// Large constants with non-zero high and low parts
const __int128 LARGE_POS = ((__int128)1 << 80) + 12345;
const __int128 LARGE_NEG = -((__int128)1 << 80) - 67890;
const unsigned __int128 LARGE_UNSIGNED = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x8888888888888888ULL;
const __int128 MEDIUM_POS = ((__int128)1 << 60);
const __int128 MEDIUM_NEG = -((__int128)1 << 60);

// Comparator function that mimics the uncovered logic
int compare_signed(__int128 a, __int128 b) {
    // This should trigger the high/low comparison logic
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

int compare_unsigned(unsigned __int128 a, unsigned __int128 b) {
    // Force unsigned comparison
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

// Mixed signed/unsigned comparison
int compare_mixed(__int128 a, unsigned __int128 b) {
    // This may trigger the specific casting behavior
    if (a < (__int128)b) return -1;
    if (a > (__int128)b) return 1;
    return 0;
}

// Sorting function that performs many comparisons
void bubble_sort(__int128 arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (compare_signed(arr[j], arr[j + 1]) > 0) {
                __int128 temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

// Binary search with wide integer comparisons
int binary_search(__int128 arr[], int n, __int128 target) {
    int left = 0, right = n - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        int cmp = compare_signed(arr[mid], target);
        
        if (cmp == 0) return mid;
        if (cmp < 0) left = mid + 1;
        else right = mid - 1;
    }
    return -1;
}

// Range check with wide integer bounds
bool in_range(__int128 x, __int128 min_val, __int128 max_val) {
    return compare_signed(x, min_val) >= 0 && compare_signed(x, max_val) <= 0;
}

// Arithmetic operations followed by comparisons
void arithmetic_checks(__int128 a, __int128 b) {
    __int128 sum = a + b;
    __int128 diff = a - b;
    __int128 prod = a * (b >> 60); // Scale down to avoid overflow
    
    // Compare arithmetic results
    if (compare_signed(sum, diff) > 0) {
        // Do something
        volatile int marker = 1;
    }
    
    if (compare_signed(prod, a) < 0) {
        // Do something else
        volatile int marker = 2;
    }
    
    // Overflow-like check
    __int128 shifted = a << 2;
    if (compare_signed(shifted, a) < 0) {
        // Possible overflow
        volatile int marker = 3;
    }
}

// Unreachable code path with wide integer comparison
void dead_code_path() {
    // These comparisons should still generate code
    if (LARGE_POS > LARGE_NEG) {
        // This is always true, but compiler might not know
        volatile int never_reached = 999;
    }
    
    if (((unsigned __int128)LARGE_POS) < LARGE_UNSIGNED) {
        volatile int also_never_reached = 888;
    }
}

int main() {
    // Initialize array with mixed values
    const int ARRAY_SIZE = 10;
    __int128 values[ARRAY_SIZE];
    
    // Assign values with different high word patterns
    values[0] = LARGE_POS;
    values[1] = LARGE_NEG;
    values[2] = MEDIUM_POS;
    values[3] = MEDIUM_NEG;
    values[4] = 0;
    values[5] = 1;
    values[6] = -1;
    values[7] = ((__int128)1 << 70) - 1;
    values[8] = -((__int128)1 << 70);
    values[9] = (__int128)0x7FFFFFFFFFFFFFFFLL << 32;
    
    // Perform many comparisons through sorting
    bubble_sort(values, ARRAY_SIZE);
    
    // Perform binary searches
    int idx1 = binary_search(values, ARRAY_SIZE, MEDIUM_POS);
    int idx2 = binary_search(values, ARRAY_SIZE, LARGE_NEG);
    
    // Test range checks
    bool in_range1 = in_range(values[2], MEDIUM_NEG, MEDIUM_POS);
    bool in_range2 = in_range(LARGE_POS, MEDIUM_NEG, MEDIUM_POS);
    
    // Test arithmetic operations with comparisons
    arithmetic_checks(values[0], values[1]);
    arithmetic_checks(values[2], values[3]);
    
    // Test unsigned comparisons
    unsigned __int128 uvals[3];
    uvals[0] = LARGE_UNSIGNED;
    uvals[1] = LARGE_UNSIGNED >> 1;
    uvals[2] = 0x123456789ABCDEF0ULL;
    
    // Sort unsigned values
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2 - i; j++) {
            if (compare_unsigned(uvals[j], uvals[j + 1]) > 0) {
                unsigned __int128 temp = uvals[j];
                uvals[j] = uvals[j + 1];
                uvals[j + 1] = temp;
            }
        }
    }
    
    // Test mixed comparisons
    compare_mixed(values[0], uvals[0]);
    compare_mixed(values[1], uvals[1]);
    
    // Generate dead code path (should still produce comparison code)
    dead_code_path();
    
    // Compute checksum to prevent optimization removal
    __int128 checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += values[i];
    }
    
    // Also add unsigned values
    for (int i = 0; i < 3; i++) {
        checksum += (__int128)uvals[i];
    }
    
    // Output simple result
    printf("Checksum modulo 1000: %lld\n", (long long)(checksum % 1000));
    printf("Found MEDIUM_POS at index: %d\n", idx1);
    printf("Found LARGE_NEG at index: %d\n", idx2);
    printf("Range test 1: %d, Range test 2: %d\n", in_range1, in_range2);
    
    return 0;
}
```

This program specifically targets the uncovered lines in `double_int::cmp` by:

1. **Wide Integer Comparisons**: Uses `__int128` and `unsigned __int128` throughout, with explicit comparisons in sorting and searching algorithms.

2. **Mixed Signed/Unsigned Comparisons**: The `compare_mixed()` function and various operations with both signed and unsigned types should trigger the casting behavior in the uncovered code.

3. **Constant Wide Integers**: Uses large constants like `((__int128)1 << 80)` that have non-zero high and low parts.

4. **Arithmetic Result Comparisons**: The `arithmetic_checks()` function performs arithmetic operations and then compares the results.

5. **Control Flow Based on Comparisons**: Uses comparison results in `if` statements, loop conditions, and sorting algorithms.

6. **Unreachable Code Path**: The `dead_code_path()` function contains comparisons that should generate code even though the path is unreachable.

The program performs numerous comparisons through:
- Bubble sort on signed `__int128` values
- Binary search with comparisons
- Range checks with wide integer bounds
- Unsigned value sorting
- Mixed signed/unsigned comparisons

The checksum output ensures the compiler cannot optimize away the computations, and the various comparison patterns should trigger the specific high/low comparison logic in the target code.
