Here's a C++ program designed to trigger the uncovered lines in expr.cc:

```cpp
// Compile with: g++ -O3 -ftree-slp-vectorize -march=native -std=c++11 expr_test.cc -o expr_test

// External function to prevent dead code elimination
extern "C" void __attribute__((noinline, noclone)) 
escape(void* p1, void* p2, void* p3, void* p4, void* p5) {
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4), "r"(p5) : "memory");
}

// GCC vector extensions
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

// Struct with multiple contiguous int members
struct ContiguousStruct {
    int a;
    int b;
    int c;
    int d;
    int e;
    int f;
};

// Union for testing
union TestUnion {
    int arr[6];
    struct {
        int x1, x2, x3, x4, x5, x6;
    };
};

int main() {
    // 1. Simple integer array with constant bounds
    int arr1[10] = {0};
    
    // const_bounds_p = true, count = 1 (MEM_P true, count <= 2)
    constexpr int lo1 = 2;
    constexpr int hi1 = 2;
    arr1[lo1] = 42;  // Single element assignment
    
    // const_bounds_p = true, count = 2 (MEM_P true, count <= 2)
    constexpr int lo2 = 3;
    constexpr int hi2 = 4;
    // Assign to slice [3..4]
    for (int i = lo2; i <= hi2; i++) {
        arr1[i] = i * 10;
    }
    
    // const_bounds_p = true, count = 5 > 2 (MEM_P true, count > 2)
    constexpr int lo3 = 5;
    constexpr int hi3 = 9;
    // TYPE_SIZE(int) * count = 4 * 5 = 20 (fits in uhwi)
    for (int i = lo3; i <= hi3; i++) {
        arr1[i] = i * 100;
    }
    
    // 2. Volatile array - affects MEM_P handling
    volatile int volatile_arr[8] = {0};
    constexpr int vlo = 1;
    constexpr int vhi = 3;  // count = 3 > 2
    for (int i = vlo; i <= vhi; i++) {
        volatile_arr[i] = i * 50;
    }
    
    // 3. Atomic array
    _Atomic int atomic_arr[6] = {0};
    constexpr int alo = 0;
    constexpr int ahi = 2;  // count = 3 > 2
    for (int i = alo; i <= ahi; i++) {
        atomic_arr[i] = i * 25;
    }
    
    // 4. Struct assignment - treating struct as array-like target
    ContiguousStruct s = {0};
    // Assign to contiguous members b, c, d (count = 3 > 2)
    // Using compound literal with designated initializers
    struct { int b; int c; int d; } tmp1 = {101, 102, 103};
    s.b = tmp1.b;
    s.c = tmp1.c;
    s.d = tmp1.d;
    
    // 5. Union assignment
    TestUnion u = {0};
    constexpr int ulo = 1;
    constexpr int uhi = 4;  // count = 4 > 2
    for (int i = ulo; i <= uhi; i++) {
        u.arr[i] = i * 30;
    }
    
    // 6. GCC vector extensions (likely not MEM_P when in registers)
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {10, 20, 30, 40};
    
    // Vector slice assignment with constant bounds
    // This may trigger the !MEM_P path
    constexpr int vec_lo = 1;
    constexpr int vec_hi = 2;  // count = 2
    for (int i = vec_lo; i <= vec_hi; i++) {
        vec1[i] = vec2[i] * 2;
    }
    
    // Another vector with count = 3 > 2
    v8hi vec3 = {1, 2, 3, 4, 5, 6, 7, 8};
    constexpr int vec3_lo = 2;
    constexpr int vec3_hi = 4;  // count = 3
    for (int i = vec3_lo; i <= vec3_hi; i++) {
        vec3[i] = i * 5;
    }
    
    // 7. Multi-dimensional array section assignment
    int md_arr[3][6] = {{0}};
    // Assign to md_arr[1][2..5] - count = 4 > 2
    constexpr int md_lo = 2;
    constexpr int md_hi = 5;
    for (int i = md_lo; i <= md_hi; i++) {
        md_arr[1][i] = i * 15;
    }
    
    // 8. Mixed: volatile vector in memory
    volatile v4si volatile_vec = {0};
    constexpr int vv_lo = 0;
    constexpr int vv_hi = 2;  // count = 3 > 2
    for (int i = vv_lo; i <= vv_hi; i++) {
        volatile_vec[i] = i * 8;
    }
    
    // 9. Test with different element sizes
    short short_arr[20] = {0};
    constexpr int sh_lo = 5;
    constexpr int sh_hi = 15;  // count = 11 > 2, TYPE_SIZE(short) * count = 2 * 11 = 22
    for (int i = sh_lo; i <= sh_hi; i++) {
        short_arr[i] = i * 3;
    }
    
    long long ll_arr[10] = {0};
    constexpr int ll_lo = 0;
    constexpr int ll_hi = 3;  // count = 4 > 2, TYPE_SIZE(long long) * count = 8 * 4 = 32
    for (int i = ll_lo; i <= ll_hi; i++) {
        ll_arr[i] = i * 100LL;
    }
    
    // Prevent dead code elimination
    escape(&arr1, &volatile_arr, &atomic_arr, &s, &u);
    escape(&vec1, &vec2, &vec3, &md_arr, &volatile_vec);
    escape(&short_arr, &ll_arr, 0, 0, 0);
    
    // Compute checksum to ensure all assignments executed
    int checksum = 0;
    
    // Sum array elements
    for (int i = 0; i < 10; i++) checksum += arr1[i];
    for (int i = 0; i < 8; i++) checksum += volatile_arr[i];
    for (int i = 0; i < 6; i++) checksum += atomic_arr[i];
    
    // Struct and union
    checksum += s.b + s.c + s.d;
    for (int i = 0; i < 6; i++) checksum += u.arr[i];
    
    // Vectors
    for (int i = 0; i < 4; i++) checksum += vec1[i];
    for (int i = 0; i < 8; i++) checksum += vec3[i];
    
    // Multi-dimensional array
    for (int i = 0; i < 6; i++) checksum += md_arr[1][i];
    
    // Volatile vector
    for (int i = 0; i < 4; i++) checksum += volatile_vec[i];
    
    // Different element sizes
    for (int i = 0; i < 20; i++) checksum += short_arr[i];
    for (int i = 0; i < 10; i++) checksum += (int)ll_arr[i];
    
    return checksum & 0xFF;  // Return lower 8 bits as exit code
}
```

This program systematically tests the uncovered code paths:

1. **Constant-Bounded Array Assignments**: Uses `constexpr` variables for bounds, creating cases with `count = 1`, `2`, and `> 2`.

2. **Mixed Target Types**: Tests both memory locations (arrays) and register vectors, with various element counts.

3. **Struct and Union Assignment**: Assigns to multiple contiguous struct members, treating them as array-like targets.

4. **Vector Extension Slicing**: Uses GCC vector extensions with compile-time index ranges.

5. **Volatile and Atomic Qualifiers**: Applies `volatile` and `_Atomic` to test their effect on optimization decisions.

6. **Multi-dimensional Array Section**: Assigns to a contiguous sub-array with constant higher-dimension index.

The program uses `constexpr` variables to ensure bounds are compile-time constants, and the `escape()` function prevents dead code elimination. The checksum calculation ensures all assignments execute.

Compile with different optimization levels to trigger different paths:
- `-O1 -fno-tree-vectorize` for basic constant propagation
- `-O3 -ftree-slp-vectorize -march=native` for aggressive vectorization
- `-O0 -fno-omit-frame-pointer` for maximum RTL expansion paths
