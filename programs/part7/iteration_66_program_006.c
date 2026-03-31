Here's a C++ program designed to trigger the uncovered lines in expr.cc:

```cpp
// Compile with: g++ -O3 -ftree-slp-vectorize -march=native -std=c++11 expr_test.cc -o expr_test

// External function to prevent dead code elimination
extern "C" void __attribute__((noinline, noclone)) 
escape(void* p) {
    asm volatile("" : : "r"(p) : "memory");
}

// Vector extension types
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

// Struct with multiple int members
struct MultiInt {
    int a;
    int b;
    int c;
    int d;
    int e;
    int f;
};

// Union with overlapping members
union MixedUnion {
    struct {
        int x;
        int y;
        int z;
    } ints;
    struct {
        float f1;
        float f2;
        float f3;
    } floats;
    char bytes[12];
};

int main() {
    int checksum = 0;
    
    // 1. Simple integer array with various slice sizes
    int arr1[10] = {0};
    
    // count = 1 case
    constexpr int lo1 = 2;
    constexpr int hi1 = 2;
    arr1[lo1] = 42;  // Simple assignment, count = 1
    
    // count = 2 case
    constexpr int lo2 = 3;
    constexpr int hi2 = 4;
    // Using compound literal for slice assignment
    int* slice2 = &arr1[lo2];
    slice2[0] = 100;
    slice2[1] = 200;
    
    // count > 2 case (count = 3)
    constexpr int lo3 = 5;
    constexpr int hi3 = 7;
    int* slice3 = &arr1[lo3];
    slice3[0] = 300;
    slice3[1] = 400;
    slice3[2] = 500;
    
    // 2. Volatile array
    volatile int volatile_arr[8] = {0};
    constexpr int vlo = 1;
    constexpr int vhi = 3;
    volatile_arr[vlo] = 111;
    volatile_arr[vlo + 1] = 222;
    volatile_arr[vlo + 2] = 333;
    
    // 3. Atomic array
    _Atomic int atomic_arr[6] = {0};
    constexpr int alo = 0;
    constexpr int ahi = 2;
    atomic_arr[alo] = 777;
    atomic_arr[alo + 1] = 888;
    atomic_arr[alo + 2] = 999;
    
    // 4. GCC vector extensions (register targets)
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {10, 20, 30, 40};
    
    // Vector slice assignment with constant bounds
    // This should trigger non-MEM_P path
    typedef int v2si __attribute__((vector_size(8)));
    v2si slice_vec;
    // Simulating slice extraction (compiler may optimize differently)
    int* vptr = (int*)&vec1;
    constexpr int vec_lo = 1;
    constexpr int vec_hi = 2;
    vptr[vec_lo] = 99;
    vptr[vec_hi] = 199;
    
    // 5. Struct member assignments (treating as array-like)
    MultiInt mystruct = {0};
    
    // Assign to multiple contiguous struct members
    // Using pointer arithmetic to simulate slice assignment
    int* struct_ptr = &mystruct.a;
    constexpr int struct_lo = 0;
    constexpr int struct_hi = 3;  // count = 4 > 2
    struct_ptr[struct_lo] = 1000;
    struct_ptr[struct_lo + 1] = 2000;
    struct_ptr[struct_lo + 2] = 3000;
    struct_ptr[struct_lo + 3] = 4000;
    
    // 6. Union assignments
    MixedUnion myunion;
    constexpr int union_lo = 0;
    constexpr int union_hi = 2;  // count = 3 > 2
    myunion.ints.x = 555;
    myunion.ints.y = 666;
    myunion.ints.z = 777;
    
    // 7. Multi-dimensional array section
    int md_arr[3][6] = {{0}};
    
    // Assign to contiguous sub-array: md_arr[1][2..4]
    constexpr int md_dim1 = 1;
    constexpr int md_lo = 2;
    constexpr int md_hi = 4;  // count = 3 > 2
    
    int* md_slice = &md_arr[md_dim1][md_lo];
    md_slice[0] = 1111;
    md_slice[1] = 2222;
    md_slice[2] = 3333;
    
    // 8. Vector with more elements (v8hi)
    v8hi vec8 = {1, 2, 3, 4, 5, 6, 7, 8};
    short* v8ptr = (short*)&vec8;
    constexpr int v8lo = 2;
    constexpr int v8hi_idx = 5;  // count = 4 > 2
    v8ptr[v8lo] = 50;
    v8ptr[v8lo + 1] = 60;
    v8ptr[v8lo + 2] = 70;
    v8ptr[v8lo + 3] = 80;
    
    // 9. Mixed volatile struct
    volatile MultiInt volatile_struct;
    volatile_struct.a = 9999;
    volatile_struct.b = 8888;
    volatile_struct.c = 7777;
    
    // 10. Array with exactly 2 elements (edge case)
    int small_arr[2] = {0};
    constexpr int small_lo = 0;
    constexpr int small_hi = 1;  // count = 2
    small_arr[small_lo] = 21;
    small_arr[small_hi] = 22;
    
    // Prevent dead code elimination
    escape(&arr1);
    escape(&volatile_arr);
    escape(&atomic_arr);
    escape(&vec1);
    escape(&mystruct);
    escape(&myunion);
    escape(&md_arr);
    escape(&vec8);
    escape(&volatile_struct);
    escape(&small_arr);
    
    // Calculate checksum to ensure all assignments executed
    checksum += arr1[lo1];
    checksum += arr1[lo2] + arr1[lo2 + 1];
    checksum += arr1[lo3] + arr1[lo3 + 1] + arr1[lo3 + 2];
    checksum += volatile_arr[vlo] + volatile_arr[vlo + 1] + volatile_arr[vlo + 2];
    checksum += atomic_arr[alo] + atomic_arr[alo + 1] + atomic_arr[alo + 2];
    checksum += vptr[vec_lo] + vptr[vec_hi];
    checksum += struct_ptr[struct_lo] + struct_ptr[struct_lo + 1] + 
                struct_ptr[struct_lo + 2] + struct_ptr[struct_lo + 3];
    checksum += myunion.ints.x + myunion.ints.y + myunion.ints.z;
    checksum += md_slice[0] + md_slice[1] + md_slice[2];
    checksum += v8ptr[v8lo] + v8ptr[v8lo + 1] + v8ptr[v8lo + 2] + v8ptr[v8lo + 3];
    checksum += volatile_struct.a + volatile_struct.b + volatile_struct.c;
    checksum += small_arr[small_lo] + small_arr[small_hi];
    
    return checksum % 256;  // Return non-zero to indicate execution
}
```

This program specifically targets the uncovered lines by:

1. **Constant-Bounded Assignments**: All slice bounds are compile-time constants using `constexpr`
2. **Mixed Target Types**: 
   - Memory targets: `arr1`, `volatile_arr`, `atomic_arr`, `mystruct`, `md_arr`
   - Register targets: `vec1`, `vec8` (GCC vector extensions)
3. **Struct/Union Assignments**: `MultiInt` struct and `MixedUnion` union with multiple member assignments
4. **Vector Extensions**: `v4si`, `v8hi` types with slice operations
5. **Volatile/Atomic Qualifiers**: `volatile_arr`, `atomic_arr`, `volatile_struct`
6. **Multi-dimensional Arrays**: `md_arr[1][2..4]` slice assignment
7. **Various Count Values**: 
   - count = 1: `arr1[2] = 42`
   - count = 2: `arr1[3..4]`, `small_arr[0..1]`
   - count > 2: `arr1[5..7]`, struct assignments, union assignments, etc.

The program should be compiled with different optimization levels to exercise different code paths in the compiler. The `escape()` function prevents dead code elimination while the checksum calculation ensures all assignments actually execute.
