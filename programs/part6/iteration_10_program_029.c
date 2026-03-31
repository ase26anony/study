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
#include <cstdint>

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

class ExplicitString {
    const char* str;
public:
    explicit ExplicitString(const char* s) : str(s) {}
    explicit operator const char*() const { return str; }
};

// ==================== 2. maybe_unused and optional attributes ====================
[[maybe_unused]] static int global_unused = 42;

std::optional<int> get_optional_value(bool provide) {
    if (provide) return 42;
    return std::nullopt;
}

void process_data([[maybe_unused]] int required, 
                  [[maybe_unused]] std::optional<int> optional_param) {
    [[maybe_unused]] auto [x, y] = std::make_pair(10, 20); // structured binding
    std::optional<double> local_opt = 3.14;
    if (local_opt) {
        // Use it
    }
}

// ==================== 3. mutable members and bit-fields ====================
struct MutableStruct {
    int normal;
    mutable int mutable_counter;
    mutable double mutable_value;
    
    // Bit-fields
    unsigned int length : 8;
    unsigned int : 4; // padding
    unsigned int string_bit_size : 12;
    
    MutableStruct() : normal(0), mutable_counter(0), mutable_value(0.0), 
                      length(0), string_bit_size(16) {}
    
    void modify() const {
        mutable_counter++; // Allowed even in const function
        mutable_value = 3.14159;
    }
};

union ComplexUnion {
    struct {
        mutable int union_mutable;
        int normal_field;
        unsigned int bit_field : 10;
    } s;
    long long as_long;
    
    ComplexUnion() : as_long(0) {}
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
        // Simulate column-major: i + (j-1)*rows
        int idx = (i - lower_bounds[0]) + 
                  (j - lower_bounds[1]) * (upper_bounds[0] - lower_bounds[0] + 1);
        return data[idx];
    }
    
    void allocate() {
        int total = 1;
        for (int i = 0; i < DIMS; ++i) {
            total *= (upper_bounds[i] - lower_bounds[i] + 1);
        }
        data = new T[total];
    }
    
    ~FortranArray() {
        delete[] data;
    }
};

// ==================== 5. String types with explicit length ====================
class ExplicitLengthString {
    char* buffer;
    size_t length; // Explicit length field
    size_t capacity;
    
public:
    ExplicitLengthString(const char* str) {
        length = strlen(str);
        capacity = length + 1;
        buffer = new char[capacity];
        memcpy(buffer, str, length);
        buffer[length] = '\0';
    }
    
    size_t get_length() const { return length; }
    size_t get_byte_size() const { return length; }
    size_t get_bit_size() const { return length * 8; }
    
    ~ExplicitLengthString() {
        delete[] buffer;
    }
};

// Picture string class (simulating COBOL picture clauses)
class PictureString {
    const char* picture;
    double value;
    
public:
    PictureString(const char* pic, double val = 0.0) 
        : picture(pic), value(val) {}
    
    const char* get_picture() const { return picture; }
    double get_value() const { return value; }
    
    void set_value(double val) { 
        // Simulate picture constraint: "999V.99" means max 999.99
        if (strcmp(picture, "999V.99") == 0 && val < 1000.0) {
            value = val;
        }
    }
};

struct Container {
    ExplicitLengthString str_member;
    PictureString pic_member;
    
    Container(const char* str, const char* pic) 
        : str_member(str), pic_member(pic, 123.45) {}
};

// ==================== 6. Function prototypes ====================
// Full prototypes
int fully_prototyped_function(int a, double b, const char* c);
void another_prototyped_function();

// Old-style declaration (C compatibility)
#ifdef __cplusplus
extern "C" {
#endif
    int old_style_function(); // No prototype in declaration
#ifdef __cplusplus
}
#endif

// Definition with full prototype
int old_style_function(int x, float y) {
    return x + static_cast<int>(y);
}

int fully_prototyped_function(int a, double b, const char* c) {
    return a + static_cast<int>(b) + strlen(c);
}

void another_prototyped_function() {
    // Do nothing
}

// ==================== 7. Thread-local storage ====================
thread_local int thread_specific = 0;
thread_local double thread_scaled_array[100];
thread_local std::vector<int> thread_local_vec;

void init_thread_local() {
    thread_specific = std::hash<std::thread::id>{}(std::this_thread::get_id()) % 1000;
    for (int i = 0; i < 100; ++i) {
        thread_scaled_array[i] = i * thread_specific * 0.01;
    }
    thread_local_vec.resize(10);
    for (size_t i = 0; i < thread_local_vec.size(); ++i) {
        thread_local_vec[i] = i * thread_specific;
    }
}

void thread_worker(std::atomic<int>& result, int thread_id) {
    init_thread_local();
    
    // Access thread-local with scaled indexing
    int base_index = thread_id * 10;
    double sum = 0.0;
    for (int i = 0; i < 10; ++i) {
        int scaled_idx = (base_index + i) % 100;
        sum += thread_scaled_array[scaled_idx];
    }
    
    // Modify thread-local
    thread_specific += thread_id;
    
    // Accumulate result
    result.fetch_add(static_cast<int>(sum) + thread_specific);
}

// ==================== Main test driver ====================
int main() {
    uint64_t hash = 0;
    
    // 1. Test explicit constructors
    ExplicitInt ei(42);
    ExplicitDouble ed(3.14);
    ExplicitString es("test");
    
    // These would fail implicit conversion:
    // bool b = ei; // Error
    // int i = ed; // Error
    // const char* s = es; // Error
    
    // Explicit conversions work:
    if (static_cast<bool>(ei)) {
        hash ^= ei.get();
    }
    hash ^= static_cast<int>(ed);
    hash ^= std::hash<const char*>{}(static_cast<const char*>(es));
    
    // 2. Test maybe_unused and optional
    process_data(1, std::nullopt);
    auto opt_val = get_optional_value(true);
    if (opt_val) {
        hash ^= *opt_val;
    }
    
    [[maybe_unused]] int local_unused = 99;
    
    // 3. Test mutable structs and bit-fields
    MutableStruct ms;
    ms.normal = 100;
    ms.modify(); // Modifies mutable members in const function
    ms.string_bit_size = 32; // Set bit-field
    
    ComplexUnion cu;
    cu.s.union_mutable = 42;
    cu.s.bit_field = 511;
    
    hash ^= ms.mutable_counter;
    hash ^= static_cast<int>(ms.mutable_value);
    hash ^= cu.s.bit_field;
    
    // 4. Test Fortran-like arrays
    FortranArray<int, 2> farray;
    farray.lower_bounds[0] = 1;
    farray.lower_bounds[1] = 1;
    farray.upper_bounds[0] = 5;
    farray.upper_bounds[1] = 5;
    farray.segment_id = 1;
    farray.allocate();
    
    // Fill with column-major ordering
    for (int j = farray.lower_bounds[1]; j <= farray.upper_bounds[1]; ++j) {
        for (int i = farray.lower_bounds[0]; i <= farray.upper_bounds[0]; ++i) {
            farray.at(i, j) = i * j;
        }
    }
    
    // Access with non-zero lower bounds
    hash ^= farray.at(2, 3);
    hash ^= farray.at(5, 5);
    
    // 5. Test string types
    ExplicitLengthString els("Hello, DWARF!");
    PictureString ps("999V.99", 123.45);
    
    hash ^= els.get_length();
    hash ^= els.get_bit_size();
    hash ^= std::hash<const char*>{}(ps.get_picture());
    
    Container container("Nested string", "ZZZ.99");
    hash ^= container.str_member.get_byte_size();
    
    // 6. Test function prototypes
    hash ^= fully_prototyped_function(10, 20.5, "proto");
    hash ^= old_style_function(5, 3.14f);
    another_prototyped_function();
    
    // 7. Test thread-local storage
    std::atomic<int> thread_result(0);
    std::vector<std::thread> threads;
    
    // Initialize main thread's TLS
    init_thread_local();
    hash ^= thread_specific;
    
    // Launch worker threads
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(thread_worker, std::ref(thread_result), i + 1);
    }
    
    // Wait for threads
    for (auto& t : threads) {
        t.join();
    }
    
    hash ^= thread_result.load();
    
    // Final output
    std::cout << "Computed hash: " << hash << std::endl;
    std::cout << "All constructs exercised. Check DWARF output with:" << std::endl;
    std::cout << "  objdump --dwarf=info ./dwarf_test | grep -E 'DW_AT_(explicit|is_optional|mutable|lower_bound|ordering|picture_string|prototyped|small|segment|string_length|threads_scaled)'" << std::endl;
    
    return 0;
}
```

This program systematically exercises all the required language features:

1. **Explicit Constructors**: `ExplicitInt`, `ExplicitDouble`, and `ExplicitString` classes with `explicit` constructors and conversion operators.

2. **Optional Attributes**: Uses `[[maybe_unused]]` on parameters, variables, and structured bindings, combined with `std::optional`.

3. **Mutable Members**: `MutableStruct` contains `mutable` data members and bit-fields, accessed in const member functions.

4. **Fortran-like Arrays**: `FortranArray` template simulates Fortran arrays with non-zero lower bounds, column-major ordering, and segment IDs.

5. **String Types**: `ExplicitLengthString` stores explicit length, and `PictureString` simulates COBOL picture clauses.

6. **Function Prototypes**: Mixes full prototypes with an old-style declaration to create contrast in debug info.

7. **Thread-Local Storage**: Uses `thread_local` variables with dynamic initialization and scaled array access across multiple threads.

The program computes a hash from all operations to ensure everything executes and produces verifiable output. Compile with the suggested flags to maximize DWARF attribute generation.
