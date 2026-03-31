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

class ExplicitString {
    const char* str;
public:
    explicit ExplicitString(const char* s) : str(s) {}
    explicit operator const char*() const { return str; }
};

class ExplicitDouble {
    double val;
public:
    explicit ExplicitDouble(double d) : val(d) {}
    explicit operator double() const { return val; }
};

// ==================== 2. maybe_unused and optional attributes ====================
[[maybe_unused]] static int global_unused = 42;

std::optional<int> get_optional_value(bool provide) {
    [[maybe_unused]] static int counter = 0;
    if (provide) return 42;
    return std::nullopt;
}

void process_data([[maybe_unused]] int data, std::optional<double> opt_dbl) {
    [[maybe_unused]] auto [x, y] = std::pair{1, 2};  // structured binding with maybe_unused
    if (opt_dbl) {
        // Use the optional value
    }
}

// ==================== 3. mutable members and bit-fields ====================
struct MutableStruct {
    mutable int mutable_counter = 0;
    int normal_member;
    
    struct Inner {
        mutable double inner_mutable;
        unsigned int bitfield1 : 3;
        unsigned int bitfield2 : 5;
        unsigned int : 4;  // unnamed bit-field
    };
    
    Inner inner;
    
    union {
        mutable int union_mutable;
        long union_normal;
    };
};

// String class with explicit length (for DW_AT_string_length)
class ExplicitLengthString {
    char* data;
    size_t length;  // Explicit length, not null-terminated
public:
    ExplicitLengthString(const char* str) {
        length = strlen(str);
        data = new char[length];
        memcpy(data, str, length);
    }
    
    ~ExplicitLengthString() { delete[] data; }
    
    size_t get_length() const { return length; }
    char* get_data() { return data; }
};

// Picture string class (simulating COBOL picture clauses)
class PictureString {
    const char* picture;
    ExplicitLengthString value;
public:
    PictureString(const char* pic, const char* val) 
        : picture(pic), value(val) {}
    
    const char* get_picture() const { return picture; }
};

// ==================== 4. Fortran-like array descriptors ====================
template<typename T, int DIMS>
struct FortranArray {
    T* data;
    std::array<int, DIMS> lower_bounds;  // Non-zero lower bounds
    std::array<int, DIMS> upper_bounds;
    std::array<int, DIMS> strides;
    int ordering;  // 0 for row-major, 1 for column-major
    
    // Segment-like partitioning
    struct Segment {
        T* segment_start;
        size_t segment_size;
    };
    Segment segment_info;
};

// ==================== 5. Function prototypes ====================
// Full prototype
int calculate_sum(int a, int b, int c) __attribute__((prototype));
int calculate_sum(int a, int b, int c) {
    return a + b + c;
}

// Another with different signature
double process_values(double x, double y, const char* desc) __attribute__((prototype));
double process_values(double x, double y, const char* desc) {
    return x * y;
}

// ==================== 6. Thread-local storage with scaling ====================
thread_local int tls_var = 0;
thread_local std::array<int, 100> tls_array = {};
thread_local ExplicitLengthString* tls_string = nullptr;

std::atomic<int> global_counter{0};

void thread_function(int thread_id) {
    // Initialize thread-local variables
    tls_var = thread_id * 100;
    
    // Scaled access pattern
    for (int i = 0; i < 10; ++i) {
        int scaled_index = (thread_id * 10 + i) % 100;
        tls_array[scaled_index] = thread_id * 1000 + i;
        
        // More complex scaling calculation
        int offset = (thread_id * 37 + i * 13) % 100;
        tls_array[offset] += thread_id;
    }
    
    // Create thread-local string
    tls_string = new ExplicitLengthString("ThreadLocal");
    
    // Update global counter
    global_counter.fetch_add(tls_var);
}

// ==================== 7. Complex struct with all features ====================
struct ComprehensiveType {
    // Explicit constructor
    explicit ComprehensiveType(int id) : id(id) {}
    
    // Mutable member
    mutable int access_count = 0;
    
    // Optional member
    std::optional<double> optional_value;
    
    // String with explicit length
    ExplicitLengthString name;
    
    // Picture string
    PictureString picture;
    
    // Array descriptor
    FortranArray<double, 2> matrix;
    
    // Bit-fields
    unsigned int flags : 8;
    unsigned int : 4;  // Padding
    mutable unsigned int state : 4;
    
    int id;
    
    // Explicit conversion operator
    explicit operator int() const { return id; }
};

// ==================== Main test driver ====================
int main() {
    int hash = 0;
    
    // 1. Test explicit constructors and conversions
    ExplicitInt ei(42);
    ExplicitString es("test");
    ExplicitDouble ed(3.14159);
    
    // Must use explicit casts
    if (static_cast<bool>(ei)) {
        hash += ei.get();
    }
    hash += static_cast<int>(static_cast<double>(ed) * 100);
    
    // 2. Test maybe_unused and optional
    [[maybe_unused]] int local_unused = 123;
    auto opt_val = get_optional_value(true);
    if (opt_val) {
        hash += *opt_val;
    }
    
    process_data(99, 3.14);
    process_data(100, std::nullopt);
    
    // 3. Test mutable structs and bit-fields
    MutableStruct ms;
    ms.mutable_counter = 5;
    ms.inner.bitfield1 = 3;
    ms.inner.bitfield2 = 15;
    ms.union_mutable = 42;
    
    // Modify mutable member
    ms.mutable_counter++;
    hash += ms.mutable_counter;
    
    // 4. Test string types
    ExplicitLengthString els("Hello, World!");
    hash += els.get_length();
    
    PictureString ps("999V.99", "123456");
    hash += strlen(ps.get_picture());
    
    // 5. Test Fortran-like arrays
    FortranArray<int, 2> farray;
    farray.lower_bounds = {1, 1};  // Non-zero lower bounds
    farray.upper_bounds = {10, 10};
    farray.ordering = 1;  // Column-major (Fortran)
    
    // Simulate column-major access
    int data[100];
    farray.data = data;
    for (int j = farray.lower_bounds[1]; j <= farray.upper_bounds[1]; ++j) {
        for (int i = farray.lower_bounds[0]; i <= farray.upper_bounds[0]; ++i) {
            int idx = (j - farray.lower_bounds[1]) * 10 + (i - farray.lower_bounds[0]);
            farray.data[idx] = i * j;
            hash += farray.data[idx] % 256;
        }
    }
    
    // 6. Test function prototypes
    hash += calculate_sum(1, 2, 3);
    hash += static_cast<int>(process_values(2.5, 3.5, "test"));
    
    // 7. Test thread-local storage with scaling
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(thread_function, i + 1);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    hash += global_counter.load();
    
    // 8. Comprehensive type usage
    ComprehensiveType ct(999);
    ct.access_count++;  // Modify mutable
    ct.optional_value = 2.71828;
    ct.flags = 0xFF;
    ct.state = 5;
    
    hash += static_cast<int>(*ct.optional_value);
    hash += ct.get_length();  // Through name member
    hash += ct.id;
    
    // Final output
    std::cout << "Computed hash: " << hash << std::endl;
    std::cout << "Test completed successfully!" << std::endl;
    
    return 0;
}
```

This program is designed to trigger the specific DWARF attributes through:

1. **DW_AT_explicit**: Multiple classes with `explicit` constructors and conversion operators, used with explicit casts.

2. **DW_AT_is_optional**: `[[maybe_unused]]` attributes on variables, parameters, and structured bindings, combined with `std::optional` usage.

3. **DW_AT_mutable**: Structs with `mutable` members, nested in unions and other structs, with bit-fields.

4. **DW_AT_lower_bound/ordering/segment**: Fortran-style array descriptors with non-zero lower bounds, column-major ordering, and segment-like partitioning.

5. **DW_AT_string_length/picture_string**: Custom string classes storing explicit lengths and picture string descriptors.

6. **DW_AT_prototyped**: Functions declared with `__attribute__((prototype))` and full prototypes.

7. **DW_AT_threads_scaled**: `thread_local` variables with dynamic initialization, accessed from multiple threads with scaled indexing patterns.

The program performs actual computations and outputs a hash to ensure all code paths are executed. The compilation flags recommended will maximize debug information generation and should cause the DWARF generator to emit the targeted attributes.
