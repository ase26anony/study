Here's a comprehensive C++ program designed to trigger the specific DWARF attributes in the uncovered lines:

```cpp
// Compile with: g++ -std=c++17 -g -O0 -fno-inline -fno-eliminate-unused-debug-types -pthread -o dwarf_test dwarf_test.cpp
// Also try: g++ -std=c++17 -g -O2 -fvar-tracking-assignments -fno-omit-frame-pointer -pthread -o dwarf_test_opt dwarf_test.cpp

#include <iostream>
#include <optional>
#include <thread>
#include <vector>
#include <atomic>
#include <cstring>
#include <array>
#include <functional>

// ==================== 1. C++11 explicit constructors and conversion operators ====================
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

class MultiExplicit {
    double x, y;
public:
    explicit MultiExplicit(double a) : x(a), y(a) {}
    explicit MultiExplicit(double a, double b) : x(a), y(b) {}
    explicit operator double() const { return x + y; }
};

// ==================== 2. maybe_unused and optional attributes ====================
[[maybe_unused]] static int global_unused = 42;

std::optional<int> compute_optional(bool flag) {
    if (flag) return 42;
    return std::nullopt;
}

void process_data([[maybe_unused]] int required, 
                  [[maybe_unused]] std::optional<double> optional_val) {
    [[maybe_unused]] auto [x, y] = std::pair{1, 2};  // structured binding with maybe_unused
}

// ==================== 3. Complex types with mutable and bit-fields ====================
struct MutableStruct {
    mutable int counter;
    int normal;
    mutable double cached_value;
    
    struct Inner {
        mutable int inner_mutable;
        unsigned int bitfield1 : 3;
        unsigned int bitfield2 : 5;
        mutable unsigned int mutable_bit : 2;
    };
    
    Inner inner;
    
    union {
        mutable int union_mutable;
        long union_normal;
    };
};

// String class with explicit length (not null-terminated)
class ExplicitLengthString {
    char* data;
    size_t length;  // Explicit length field
    mutable size_t access_count;  // mutable member
    
    // Bit-field for flags
    struct {
        mutable unsigned int is_owned : 1;
        unsigned int is_const : 1;
        unsigned int : 6;  // padding
    } flags;
    
public:
    ExplicitLengthString(const char* str) : 
        length(strlen(str)), 
        access_count(0) {
        data = new char[length];
        memcpy(data, str, length);
        flags.is_owned = 1;
        flags.is_const = 0;
    }
    
    ~ExplicitLengthString() {
        if (flags.is_owned) delete[] data;
    }
    
    size_t get_length() const {
        ++access_count;  // modifying mutable in const method
        return length;
    }
};

// ==================== 4. Fortran-like array descriptors ====================
template<typename T, int DIMS>
struct FortranArray {
    T* data;
    int lower_bounds[DIMS];
    int upper_bounds[DIMS];
    int strides[DIMS];
    mutable int access_counter;  // For DW_AT_mutable
    
    // Simulate column-major ordering (Fortran)
    void compute_strides() {
        strides[DIMS-1] = 1;
        for (int i = DIMS-2; i >= 0; --i) {
            strides[i] = strides[i+1] * (upper_bounds[i+1] - lower_bounds[i+1] + 1);
        }
    }
    
    T& at(int indices[DIMS]) {
        ++access_counter;
        int idx = 0;
        for (int i = 0; i < DIMS; ++i) {
            idx += (indices[i] - lower_bounds[i]) * strides[i];
        }
        return data[idx];
    }
};

// Segment-like partitioning
struct SegmentedArray {
    void* segments[4];
    mutable int current_segment;
    int segment_size;
    
    int* get_segment(int idx) const {
        current_segment = idx;  // mutable modification
        return static_cast<int*>(segments[idx]);
    }
};

// ==================== 5. Picture string class ====================
class PictureString {
    const char* picture;  // e.g., "999V.99"
    char* data;
    size_t max_length;
    mutable size_t validation_count;
    
public:
    PictureString(const char* pic, size_t max_len) : 
        picture(pic), 
        max_length(max_len),
        validation_count(0) {
        data = new char[max_len];
        memset(data, '0', max_len);
    }
    
    ~PictureString() {
        delete[] data;
    }
    
    bool validate() const {
        ++validation_count;
        // Simple validation based on picture
        return strlen(picture) > 0;
    }
    
    const char* get_picture() const { return picture; }
};

// ==================== 6. Function prototypes ====================
// Full prototypes with different attributes
#ifdef __GNUC__
int __attribute__((prototype)) prototyped_func(int a, double b);
#endif

int fully_prototyped(int a, int b, int c);
double complex_prototype(const char* str, std::optional<int> opt, ...);

int fully_prototyped(int a, int b, int c) {
    return a + b + c;
}

double complex_prototype(const char* str, std::optional<int> opt, ...) {
    return opt.value_or(0) * 1.5;
}

// ==================== 7. Thread-local scaled storage ====================
thread_local int tls_scaled = 0;
thread_local std::array<int, 100> tls_array;
thread_local int* tls_dynamic = nullptr;

void init_tls() {
    static std::atomic<int> counter{0};
    int id = counter.fetch_add(1, std::memory_order_relaxed);
    tls_scaled = id * 1000;
    
    if (!tls_dynamic) {
        tls_dynamic = new int[50];
        for (int i = 0; i < 50; ++i) {
            tls_dynamic[i] = id * 100 + i;  // Scaled by thread ID
        }
    }
    
    // Fill array with scaled values
    for (size_t i = 0; i < tls_array.size(); ++i) {
        tls_array[i] = id * 1000 + static_cast<int>(i);
    }
}

int compute_thread_hash() {
    init_tls();
    
    // Access with scaling
    int hash = tls_scaled;
    for (int i = 0; i < 50; ++i) {
        hash += tls_dynamic[i * (tls_scaled % 5 + 1) / 10];  // Scaled indexing
    }
    
    // Access array with scaled indices
    for (size_t i = 0; i < tls_array.size(); i += (tls_scaled % 7 + 1)) {
        hash ^= tls_array[i];
    }
    
    return hash;
}

// ==================== Main test driver ====================
int main() {
    int total_hash = 0;
    
    // 1. Test explicit constructors and conversions
    ExplicitInt ei(42);
    ExplicitString es("test");
    MultiExplicit me(3.14);
    
    // Explicit conversions (would fail implicitly)
    bool b = static_cast<bool>(ei);
    const char* s = static_cast<const char*>(es);
    double d = static_cast<double>(me);
    
    total_hash += ei.get() + b + static_cast<int>(d);
    
    // 2. Test maybe_unused and optional
    [[maybe_unused]] int local_unused = 100;
    auto opt1 = compute_optional(true);
    auto opt2 = compute_optional(false);
    
    process_data(42, 3.14);
    process_data(42, std::nullopt);
    
    if (opt1) total_hash += *opt1;
    if (opt2) total_hash += 1;  // won't execute
    
    // 3. Test mutable structs and bit-fields
    MutableStruct ms{0, 1, 2.0, {3, 1, 2, 1}, {5}};
    ms.counter = 10;  // Direct mutable access
    ms.inner.inner_mutable = 20;
    ms.union_mutable = 30;
    
    ExplicitLengthString els("Hello, DWARF!");
    total_hash += static_cast<int>(els.get_length());
    
    // 4. Test Fortran-like arrays
    FortranArray<int, 2> fa2d;
    fa2d.lower_bounds[0] = -5;  // Non-zero lower bound
    fa2d.lower_bounds[1] = 1;   // Non-zero lower bound
    fa2d.upper_bounds[0] = 5;
    fa2d.upper_bounds[1] = 10;
    fa2d.data = new int[11 * 10];  // (5 - (-5) + 1) * (10 - 1 + 1)
    fa2d.compute_strides();
    
    int indices[2] = {0, 2};
    fa2d.at(indices) = 42;
    total_hash += fa2d.at(indices);
    
    SegmentedArray sa;
    for (int i = 0; i < 4; ++i) {
        sa.segments[i] = new int[100];
        sa.get_segment(i)[0] = i * 100;
    }
    total_hash += static_cast<int*>(sa.segments[0])[0];
    
    // 5. Test picture strings
    PictureString ps("999V.99", 10);
    ps.validate();
    total_hash += static_cast<int>(strlen(ps.get_picture()));
    
    // 6. Call prototyped functions
    total_hash += fully_prototyped(1, 2, 3);
    total_hash += static_cast<int>(complex_prototype("test", 10));
    
    // 7. Test thread-local scaled storage
    std::vector<std::thread> threads;
    std::atomic<int> thread_hash_sum{0};
    
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&thread_hash_sum, i]() {
            init_tls();
            int h = compute_thread_hash();
            thread_hash_sum.fetch_add(h, std::memory_order_relaxed);
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    total_hash += thread_hash_sum.load();
    
    // Cleanup
    delete[] fa2d.data;
    for (int i = 0; i < 4; ++i) {
        delete[] static_cast<int*>(sa.segments[i]);
    }
    
    // Final output
    std::cout << "Total hash: " << total_hash << std::endl;
    std::cout << "Program executed successfully, triggering DWARF attributes." << std::endl;
    
    return 0;
}
```

This program comprehensively addresses all the requirements:

1. **Explicit Constructors**: `ExplicitInt`, `ExplicitString`, and `MultiExplicit` classes with `explicit` constructors and conversion operators.

2. **Optional Attributes**: Uses `[[maybe_unused]]` on globals, parameters, locals, and structured bindings. Uses `std::optional` return types and parameters.

3. **Mutable Members**: `MutableStruct` contains multiple `mutable` members, bit-fields, and nested structures. `ExplicitLengthString` has mutable access counters and bit-fields.

4. **Fortran-like Arrays**: `FortranArray` simulates non-zero lower bounds and column-major ordering. `SegmentedArray` provides segment-like partitioning.

5. **String Types**: `ExplicitLengthString` stores explicit length. `PictureString` simulates COBOL picture clauses with a picture descriptor.

6. **Function Prototypes**: All functions have full prototypes, with some using GNU attributes.

7. **Thread-Local Scaled**: Uses `thread_local` variables with dynamic initialization and scaled indexing based on thread ID.

The `main()` function exercises all these constructs, performing calculations that ensure the code paths are executed. The compilation flags recommended will help generate detailed debug information, increasing the likelihood that the DWARF generator will emit the specific attributes in the uncovered switch cases.
