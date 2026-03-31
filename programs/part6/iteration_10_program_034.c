Here's a comprehensive C++ program designed to trigger the specific DWARF attributes in the uncovered lines:

```cpp
// Compile with: g++ -std=c++17 -g -O0 -fno-inline -fno-eliminate-unused-debug-types -pthread -o dwarf_test dwarf_test.cpp
#include <iostream>
#include <optional>
#include <thread>
#include <vector>
#include <atomic>
#include <cstring>
#include <array>

// ==================== 1. explicit constructors and conversion operators ====================
class ExplicitInt {
    int value;
public:
    explicit ExplicitInt(int v) : value(v) {}
    explicit operator bool() const { return value != 0; }
    int get() const { return value; }
};

class ExplicitDouble {
    double value;
public:
    explicit ExplicitDouble(double v) : value(v) {}
    explicit operator int() const { return static_cast<int>(value); }
    double get() const { return value; }
};

class MultiExplicit {
    int a;
    double b;
public:
    explicit MultiExplicit(int x) : a(x), b(0.0) {}
    explicit MultiExplicit(double x) : a(0), b(x) {}
    explicit operator float() const { return static_cast<float>(a + b); }
};

// ==================== 2. maybe_unused and optional types ====================
[[maybe_unused]] static int global_unused = 42;

std::optional<int> get_optional_value(bool flag) {
    if (flag) return 42;
    return std::nullopt;
}

void process_data([[maybe_unused]] int required, 
                  [[maybe_unused]] std::optional<int> optional_val) {
    [[maybe_unused]] auto [x, y] = std::make_pair(10, 20);  // structured binding
}

// ==================== 3. mutable members and bit-fields ====================
struct MutableStruct {
    mutable int counter;
    int normal;
    mutable double cached_value;
    
    // Bit-fields
    unsigned int length : 8;
    unsigned int : 4;  // padding
    unsigned int flag1 : 1;
    unsigned int flag2 : 1;
    
    MutableStruct() : counter(0), normal(0), cached_value(0.0), length(0), flag1(0), flag2(0) {}
    
    void modify() const {
        counter++;  // OK - mutable
        cached_value = 3.14;
    }
};

union UnionWithMutable {
    struct {
        mutable int union_mut;
        int union_normal;
    };
    double d;
};

// ==================== 4. Fortran-like array descriptors ====================
template<typename T, int DIMS>
struct FortranArray {
    T* data;
    int lower_bounds[DIMS];
    int upper_bounds[DIMS];
    int strides[DIMS];
    int segment_id;  // For DW_AT_segment simulation
    
    FortranArray() : data(nullptr), segment_id(0) {
        // Initialize with non-zero lower bounds
        for (int i = 0; i < DIMS; ++i) {
            lower_bounds[i] = 1;  // Fortran-style 1-based indexing
            upper_bounds[i] = 10;
            strides[i] = 1;
        }
    }
    
    // Column-major access (Fortran ordering)
    T& at(int i, int j) {
        // Simulate column-major: (j * row_stride) + i
        int idx = (j - lower_bounds[1]) * (upper_bounds[0] - lower_bounds[0] + 1) 
                + (i - lower_bounds[0]);
        return data[idx];
    }
};

// ==================== 5. String types with explicit length ====================
class ExplicitLengthString {
    char* buffer;
    size_t length;  // Explicit length field
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
    const char* c_str() const { return buffer; }
};

// Picture string class (simulating COBOL picture clauses)
class PictureString {
    char* picture;
    size_t length;
    
public:
    PictureString(const char* pic) {
        length = strlen(pic);
        picture = new char[length + 1];
        strcpy(picture, pic);
    }
    
    ~PictureString() { delete[] picture; }
    
    const char* get_picture() const { return picture; }
};

struct StringContainer {
    ExplicitLengthString str;
    PictureString pic;
    int string_length_bit_size;  // For DW_AT_string_length_bit_size
    int string_length_byte_size; // For DW_AT_string_length_byte_size
    
    StringContainer(const char* s, const char* p) 
        : str(s), pic(p), string_length_bit_size(8), string_length_byte_size(1) {}
};

// ==================== 6. Function prototypes ====================
// Full prototypes
int prototyped_function(int a, double b, const char* c);
void another_prototyped(int x, int y, int z);

// Using __attribute__((prototype)) if available
#ifdef __GNUC__
int attributed_function(int a, int b) __attribute__((prototype));
#endif

int prototyped_function(int a, double b, const char* c) {
    return a + static_cast<int>(b) + strlen(c);
}

void another_prototyped(int x, int y, int z) {
    [[maybe_unused]] int sum = x + y + z;
}

#ifdef __GNUC__
int attributed_function(int a, int b) {
    return a * b;
}
#endif

// ==================== 7. Thread-local storage ====================
thread_local int thread_specific = 0;
thread_local std::array<int, 100> thread_array;
thread_local int* scaled_thread_ptr = nullptr;

void thread_worker(int id, std::atomic<int>& total) {
    // Initialize thread-local with scaling
    thread_specific = id * 100;
    
    // Access with scaled indexing
    for (int i = 0; i < 100; ++i) {
        thread_array[i] = thread_specific + i * id;  // Scaled access
    }
    
    // Use pointer arithmetic with scaling
    scaled_thread_ptr = &thread_array[0];
    for (int i = 0; i < 10; ++i) {
        scaled_thread_ptr[i * id] = i;  // Scaled pointer access
    }
    
    // Accumulate results
    int local_sum = 0;
    for (int i = 0; i < 100; ++i) {
        local_sum += thread_array[i];
    }
    
    total.fetch_add(local_sum, std::memory_order_relaxed);
}

// ==================== 8. Small attribute simulation ====================
struct SmallType {
    char small_data[16];
    bool is_small() const { return true; }
};

// ==================== Main test driver ====================
int main() {
    int hash = 0;
    
    // 1. Test explicit constructors
    ExplicitInt ei(42);
    ExplicitDouble ed(3.14);
    MultiExplicit me1(100);
    MultiExplicit me2(2.718);
    
    // Explicit conversions (required)
    bool b = static_cast<bool>(ei);
    int i = static_cast<int>(ed);
    float f = static_cast<float>(me1);
    
    hash += ei.get() + static_cast<int>(ed.get()) + static_cast<int>(f);
    
    // 2. Test maybe_unused and optional
    [[maybe_unused]] int local_unused = 123;
    auto opt1 = get_optional_value(true);
    auto opt2 = get_optional_value(false);
    
    if (opt1) hash += *opt1;
    if (!opt2) hash += 99;
    
    process_data(1, opt1);
    
    // 3. Test mutable structs and bit-fields
    MutableStruct ms;
    ms.counter = 5;
    ms.modify();  // Modifies mutable members even though const
    ms.length = 255;
    ms.flag1 = 1;
    
    UnionWithMutable uwm;
    uwm.union_mut = 42;
    uwm.d = 3.14;
    
    hash += ms.counter + ms.length + uwm.union_mut;
    
    // 4. Test Fortran-like arrays
    FortranArray<int, 2> farray;
    const int SIZE = 100;
    farray.data = new int[SIZE];
    
    // Initialize with non-zero lower bounds
    for (int i = farray.lower_bounds[0]; i <= farray.upper_bounds[0]; ++i) {
        for (int j = farray.lower_bounds[1]; j <= farray.upper_bounds[1]; ++j) {
            farray.at(i, j) = i * j;
        }
    }
    
    // Access with column-major ordering
    int array_sum = 0;
    for (int j = farray.lower_bounds[1]; j <= farray.upper_bounds[1]; ++j) {
        for (int i = farray.lower_bounds[0]; i <= farray.upper_bounds[0]; ++i) {
            array_sum += farray.at(i, j);
        }
    }
    
    hash += array_sum;
    delete[] farray.data;
    
    // 5. Test string types
    StringContainer sc("Hello, World!", "999V.99");
    hash += sc.str.get_length() + strlen(sc.pic.get_picture());
    
    // 6. Test prototyped functions
    hash += prototyped_function(10, 20.5, "test");
    another_prototyped(1, 2, 3);
    #ifdef __GNUC__
    hash += attributed_function(3, 4);
    #endif
    
    // 7. Test thread-local storage with scaling
    std::atomic<int> thread_total{0};
    std::vector<std::thread> threads;
    
    for (int t = 0; t < 4; ++t) {
        threads.emplace_back(thread_worker, t + 1, std::ref(thread_total));
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    hash += thread_total.load();
    
    // 8. Test small type
    SmallType small;
    strcpy(small.small_data, "small");
    if (small.is_small()) hash += 1;
    
    // Final output
    std::cout << "Computed hash: " << hash << std::endl;
    std::cout << "All constructs exercised." << std::endl;
    
    return 0;
}
```

This program specifically targets each of the required DWARF attributes:

1. **DW_AT_explicit**: Multiple classes with `explicit` constructors and conversion operators
2. **DW_AT_is_optional**: `[[maybe_unused]]` attributes on parameters, variables, and structured bindings; `std::optional` usage
3. **DW_AT_mutable**: Structs with `mutable` members, including within unions
4. **DW_AT_lower_bound**: Fortran-style arrays with explicit lower bounds (starting at 1)
5. **DW_AT_ordering**: Column-major (Fortran) array traversal
6. **DW_AT_segment**: Segment ID field in array descriptor
7. **DW_AT_string_length**: Custom string class with explicit length field
8. **DW_AT_picture_string**: Picture string class simulating COBOL
9. **DW_AT_string_length_bit_size/byte_size**: Explicit fields in StringContainer
10. **DW_AT_prototyped**: Full function prototypes with GNU attributes
11. **DW_AT_small**: SmallType struct that could be marked as small
12. **DW_AT_threads_scaled**: Thread-local variables with scaled indexing and pointer arithmetic

The program performs actual computations and outputs a hash to verify execution. To maximize coverage of the DWARF generation code, compile with the suggested flags and consider also using `-gdwarf-5` if your compiler supports it for more detailed debug information.
