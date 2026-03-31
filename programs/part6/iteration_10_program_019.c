Here's a comprehensive C++ program designed to trigger the specific DWARF attributes in the uncovered lines:

```cpp
// Compile with: g++ -std=c++17 -g -O0 -fno-inline -fno-eliminate-unused-debug-types -pthread -o dwarf_test dwarf_test.cpp
// Also try: g++ -std=c++17 -g -O2 -fvar-tracking-assignments -fno-omit-frame-pointer -pthread -o dwarf_test dwarf_test.cpp

#include <iostream>
#include <optional>
#include <thread>
#include <vector>
#include <atomic>
#include <cstring>
#include <cstdint>

// ==================== 1. C++11 explicit constructors and conversion operators ====================
class ExplicitInt {
    int value;
public:
    explicit ExplicitInt(int v) : value(v) {}
    explicit operator bool() const { return value != 0; }
    explicit operator int() const { return value; }
};

class ExplicitString {
    const char* str;
public:
    explicit ExplicitString(const char* s) : str(s) {}
    explicit operator const char*() const { return str; }
};

class MultiExplicit {
    double d;
    int i;
public:
    explicit MultiExplicit(double d_val, int i_val = 0) : d(d_val), i(i_val) {}
    explicit operator float() const { return static_cast<float>(d); }
};

// ==================== 2. maybe_unused and optional attributes ====================
[[maybe_unused]] static int global_unused = 42;

std::optional<int> get_optional_value(bool flag) {
    if (flag) return 42;
    return std::nullopt;
}

void process_data([[maybe_unused]] int input, std::optional<double> opt_val) {
    [[maybe_unused]] double local_unused = 3.14;
    if (opt_val) {
        std::cout << "Optional has value: " << *opt_val << std::endl;
    }
}

// ==================== 3. Complex types with mutable and bit-fields ====================
struct MutableStruct {
    mutable int counter;
    int normal;
    mutable double cached_value;
    
    // Bit-fields
    unsigned int length : 8;
    unsigned int : 4; // padding
    mutable unsigned int flags : 4;
    
    MutableStruct() : counter(0), normal(0), cached_value(0.0), length(0), flags(0) {}
    
    void increment() const {
        counter++; // mutable member modified in const context
        flags = (flags + 1) & 0xF;
    }
};

union ComplexUnion {
    MutableStruct ms;
    struct {
        mutable int union_mutable;
        int data[4];
    } inner;
    
    ComplexUnion() : ms() {}
};

// ==================== 4. Fortran-like array descriptors ====================
template<typename T, int DIMS>
struct FortranArray {
    T* data;
    int lower_bounds[DIMS];
    int upper_bounds[DIMS];
    int strides[DIMS];
    int segment_id; // For DW_AT_segment simulation
    
    FortranArray() : data(nullptr), segment_id(0) {
        // Initialize with non-zero lower bounds
        for (int i = 0; i < DIMS; ++i) {
            lower_bounds[i] = 1; // Fortran-style 1-based indexing
            upper_bounds[i] = 10;
            strides[i] = 1;
        }
    }
    
    // Column-major access (Fortran ordering)
    T& at(int i, int j) {
        int idx = (i - lower_bounds[0]) + 
                  (j - lower_bounds[1]) * (upper_bounds[0] - lower_bounds[0] + 1);
        return data[idx];
    }
};

// ==================== 5. String types with explicit length and picture strings ====================
class ExplicitLengthString {
    char* buffer;
    size_t length; // Explicit length, not null-terminated
    size_t capacity;
    
public:
    ExplicitLengthString(const char* str) {
        length = strlen(str);
        capacity = length + 1;
        buffer = new char[capacity];
        memcpy(buffer, str, length);
        buffer[length] = '\0';
    }
    
    ~ExplicitLengthString() { delete[] buffer; }
    
    size_t get_length() const { return length; }
    size_t get_byte_size() const { return length; }
    size_t get_bit_size() const { return length * 8; }
};

class PictureString {
    char* picture;
    double value;
    
public:
    PictureString(const char* pic, double val = 0.0) : value(val) {
        picture = new char[strlen(pic) + 1];
        strcpy(picture, pic);
    }
    
    ~PictureString() { delete[] picture; }
    
    const char* get_picture() const { return picture; }
};

// ==================== 6. Function prototypes ====================
// Full prototypes
int add_numbers(int a, int b) __attribute__((prototype));
int add_numbers(int a, int b) {
    return a + b;
}

void process_array(int* arr, size_t size) __attribute__((prototype));
void process_array(int* arr, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        arr[i] *= 2;
    }
}

// ==================== 7. Thread-local storage with scaling ====================
thread_local int thread_specific = 0;
thread_local double thread_scaled_array[100];
thread_local std::vector<int> thread_local_vec __attribute__((init_priority(1000)));

void thread_worker(int id, std::atomic<int>& result) {
    // Initialize thread-local with dynamic value
    thread_specific = id * 100;
    
    // Use scaled indexing
    for (int i = 0; i < 100; ++i) {
        thread_scaled_array[i] = (id * 1000.0) + (i * 10.0);
    }
    
    // Perform calculation with scaled values
    double local_sum = 0;
    for (int i = 0; i < 100; ++i) {
        local_sum += thread_scaled_array[i] * (i + 1);
    }
    
    // Store result atomically
    result.fetch_add(static_cast<int>(local_sum) + thread_specific);
}

// ==================== Main test driver ====================
int main() {
    uint64_t hash = 0;
    
    // 1. Test explicit constructors and conversions
    ExplicitInt ei(42);
    ExplicitString es("test");
    MultiExplicit me(3.14, 2);
    
    // Explicit conversions required
    bool b = static_cast<bool>(ei);
    const char* s = static_cast<const char*>(es);
    float f = static_cast<float>(me);
    
    hash += static_cast<int>(b);
    hash += strlen(s);
    hash += static_cast<int>(f * 100);
    
    // 2. Test optional and maybe_unused
    std::optional<int> opt1 = get_optional_value(true);
    std::optional<double> opt2 = std::nullopt;
    
    [[maybe_unused]] auto [unused1, unused2] = std::make_pair(1, 2.0);
    
    if (opt1) {
        hash += *opt1;
    }
    
    process_data(10, 3.14);
    
    // 3. Test mutable structs and bit-fields
    MutableStruct ms;
    ms.increment(); // Modifies mutable members in const context
    ms.length = 255;
    
    ComplexUnion cu;
    cu.ms.increment();
    cu.inner.union_mutable = 42;
    
    hash += ms.counter;
    hash += ms.length;
    hash += cu.inner.union_mutable;
    
    // 4. Test Fortran-like arrays
    FortranArray<double, 2> farray;
    farray.data = new double[100];
    farray.lower_bounds[0] = -5;
    farray.lower_bounds[1] = 1;
    farray.segment_id = 1;
    
    // Initialize array
    for (int i = farray.lower_bounds[0]; i <= 5; ++i) {
        for (int j = farray.lower_bounds[1]; j <= 10; ++j) {
            farray.at(i, j) = i * 10 + j;
        }
    }
    
    // Access with non-zero lower bounds
    double array_sum = 0;
    for (int i = farray.lower_bounds[0]; i <= 5; ++i) {
        for (int j = farray.lower_bounds[1]; j <= 10; ++j) {
            array_sum += farray.at(i, j);
        }
    }
    
    hash += static_cast<int>(array_sum);
    delete[] farray.data;
    
    // 5. Test string types
    ExplicitLengthString els("Hello, World!");
    PictureString ps("999V.99", 123.45);
    
    hash += els.get_length();
    hash += els.get_byte_size();
    hash += strlen(ps.get_picture());
    
    // 6. Test function prototypes
    int sum = add_numbers(10, 20);
    int arr[5] = {1, 2, 3, 4, 5};
    process_array(arr, 5);
    
    for (int i = 0; i < 5; ++i) {
        hash += arr[i];
    }
    hash += sum;
    
    // 7. Test thread-local storage with scaling
    std::atomic<int> thread_result(0);
    std::vector<std::thread> threads;
    
    // Initialize thread-local vector
    thread_local_vec.push_back(100);
    
    // Launch threads
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(thread_worker, i + 1, std::ref(thread_result));
    }
    
    // Wait for threads
    for (auto& t : threads) {
        t.join();
    }
    
    hash += thread_result.load();
    
    // Print final hash to ensure execution
    std::cout << "Final hash: " << hash << std::endl;
    std::cout << "Test completed successfully!" << std::endl;
    
    return 0;
}
```

This program specifically targets each of the required DWARF attributes:

1. **DW_AT_explicit**: Multiple classes with `explicit` constructors and conversion operators
2. **DW_AT_is_optional**: `[[maybe_unused]]` attributes on variables, parameters, and structured bindings, plus `std::optional` usage
3. **DW_AT_mutable**: Structs with `mutable` members and bit-fields
4. **DW_AT_lower_bound/ordering/segment**: Fortran-style array descriptors with non-zero lower bounds and column-major ordering
5. **DW_AT_string_length/picture_string**: Custom string classes with explicit length storage and picture string simulation
6. **DW_AT_prototyped**: Functions declared with `__attribute__((prototype))` and full prototypes
7. **DW_AT_threads_scaled**: `thread_local` variables with dynamic initialization and scaled array access

The program performs actual computations and prints a final hash to verify execution, while ensuring all the language constructs are used in ways that should trigger the corresponding DWARF attribute generation.
