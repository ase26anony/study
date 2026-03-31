#ifdef __cplusplus
#include <iostream>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <type_traits>
using namespace std;

// Forward declarations for TYPE_UNDEFINED
struct IncompleteType;  // Never defined
class NeverDefinedClass; // Never defined
#endif

// ==================== TYPE_UNDEFINED ====================
struct IncompleteType* global_incomplete_ptr = nullptr;
#ifdef __cplusplus
class NeverDefinedClass* another_undefined_ptr = nullptr;
#endif

// ==================== TYPE_SCALAR ====================
// Global scalar variables
int global_int = 42;
unsigned int global_uint = 100u;
char global_char = 'A';
signed char global_schar = -10;
unsigned char global_uchar = 200;
short global_short = -1000;
unsigned short global_ushort = 2000;
long global_long = 100000L;
unsigned long global_ulong = 200000UL;
long long global_llong = 10000000000LL;
unsigned long long global_ullong = 20000000000ULL;
float global_float = 3.14159f;
double global_double = 2.718281828459045;
#ifdef __cplusplus
bool global_bool = true;
#endif

// ==================== TYPE_STRING ====================
const char* global_string = "Hello, World!";
const wchar_t* global_wstring = L"Wide String";
char global_char_array[50] = "Character array";
const char* const global_const_string_ptr = "Constant pointer to constant string";

// ==================== TYPE_STRUCT ====================
// Basic structure
struct SimpleStruct {
    int x;
    double y;
    char name[20];
};

// Structure with bit-fields
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    signed int value : 10;
    unsigned int : 2;  // Unnamed bit-field
};

// Packed structure
struct PackedStruct {
    char a;
    int b;
    double c;
} __attribute__((packed));

// Nested structure
struct OuterStruct {
    SimpleStruct inner;
    int outer_value;
    struct {
        int anonymous_x;
        double anonymous_y;
    } anonymous;
};

// Structure with flexible array member (C mode)
#ifdef __STDC__
struct FlexArrayStruct {
    int count;
    double data[];  // Flexible array member
};
#endif

// ==================== TYPE_UNION ====================
// C-style union
union DataUnion {
    int int_value;
    float float_value;
    double double_value;
    char string_value[16];
    void* ptr_value;
};

// Union within structure
struct UnionContainer {
    int type;
    union {
        int int_data;
        float float_data;
        struct {
            int x, y;
        } point_data;
    } value;
};

#ifdef __cplusplus
// C++11 anonymous union
struct AnonymousUnionStruct {
    int tag;
    union {
        int as_int;
        double as_double;
        char* as_string;
    };
};
#endif

// ==================== TYPE_POINTER ====================
// Various pointer types
int* int_ptr = &global_int;
const int* const_int_ptr = &global_int;
volatile int* volatile_int_ptr = &global_int;
int** double_int_ptr = &int_ptr;
int* restrict restrict_ptr = &global_int;

// Pointers to different types
SimpleStruct* struct_ptr = nullptr;
DataUnion* union_ptr = nullptr;
char** string_ptr_ptr = nullptr;

// Function pointers
typedef int (*FuncPtr)(int, double);
typedef void (*VoidFuncPtr)(void);
typedef const char* (*StringFuncPtr)(const char*);

// ==================== TYPE_ARRAY ====================
// Various arrays
int int_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
double double_array[5][3];  // 2D array
char char_3d_array[2][3][4]; // 3D array

// Array of pointers
int* pointer_array[5];

// Array of structures
SimpleStruct struct_array[3];

// Incomplete array in structure (C mode)
#ifdef __STDC__
struct IncompleteArrayStruct {
    int size;
    int elements[/*incomplete*/];
};
#endif

// ==================== TYPE_CALLBACK ====================
// Function pointer types
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, void*);

// Structure with function pointer
struct CallbackContainer {
    Callback callback;
    void* user_data;
};

// Function using callback
void register_callback(Callback cb, void* data) {
    // Callback would be called here
    (void)cb; (void)data;
}

// ==================== C++ SPECIFIC ====================
#ifdef __cplusplus

// ==================== TYPE_USER_STRUCT ====================
// Simple class
class BaseClass {
private:
    int private_data;
protected:
    float protected_data;
public:
    BaseClass() : private_data(0), protected_data(0.0f) {}
    virtual ~BaseClass() {}
    virtual void virtual_method() = 0;
    void public_method() { private_data++; }
};

// Derived class with single inheritance
class DerivedClass : public BaseClass {
private:
    double derived_data;
public:
    DerivedClass() : derived_data(0.0) {}
    void virtual_method() override { derived_data = 1.0; }
};

// Multiple inheritance
class Interface1 {
public:
    virtual void method1() = 0;
    virtual ~Interface1() {}
};

class Interface2 {
public:
    virtual void method2() = 0;
    virtual ~Interface2() {}
};

class MultipleInheritanceClass : public Interface1, public Interface2 {
public:
    void method1() override {}
    void method2() override {}
};

// Template class
template<typename T>
class TemplateClass {
private:
    T data;
public:
    TemplateClass(const T& val) : data(val) {}
    T get_data() const { return data; }
    void set_data(const T& val) { data = val; }
};

// Class with complex members
class ComplexClass {
public:
    ComplexClass() : callback(nullptr) {}
    
    void set_callback(function<void(int)> cb) {
        callback = cb;
    }
    
    void execute(int value) {
        if (callback) callback(value);
    }
    
private:
    function<void(int)> callback;
    TemplateClass<int> template_member{42};
};

// ==================== TYPE_LANG_STRUCT ====================
// Lambda expressions with different captures
auto lambda_no_capture = [](int x) { return x * 2; };
auto lambda_by_value = [global_int](int x) { return x + global_int; };
auto lambda_by_ref = [&global_double](int x) { return x + static_cast<int>(global_double); };
auto lambda_mutable = [counter = 0](int x) mutable { counter += x; return counter; };

// Function using initializer_list
int sum_initializer_list(initializer_list<int> lst) {
    int sum = 0;
    for (int val : lst) sum += val;
    return sum;
}

// Structured bindings (C++17)
pair<int, string> get_pair() {
    return {42, "answer"};
}

// Fold expression (C++17)
template<typename... Args>
auto fold_sum(Args... args) {
    return (args + ...);
}

// Coroutine-related (C++20)
#ifdef __cpp_impl_coroutine
struct DummyCoroutine {
    struct promise_type {
        DummyCoroutine get_return_object() { return {}; }
        suspend_never initial_suspend() { return {}; }
        suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

DummyCoroutine dummy_coroutine() {
    co_return;
}
#endif

#endif // __cplusplus

// ==================== FUNCTION DEFINITIONS ====================
// Function using function pointer
int compare_ints(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

// Function that takes various parameters
void process_data(int scalar, SimpleStruct* struct_ptr, DataUnion union_val) {
    (void)scalar; (void)struct_ptr; (void)union_val;
}

// ==================== MAIN FUNCTION ====================
int main(int argc, char* argv[]) {
    // Use argc to prevent dead code elimination
    int mode = (argc > 1) ? argv[1][0] : '0';
    
    // ==================== TYPE_UNDEFINED ====================
    struct IncompleteType* local_incomplete_ptr = nullptr;
    (void)local_incomplete_ptr;
    
    // ==================== TYPE_SCALAR ====================
    // Local scalar variables
    int local_int = 100;
    unsigned int local_uint = 200u;
    char local_char = 'B';
    float local_float = 1.234f;
    double local_double = 9.876;
#ifdef __cplusplus
    bool local_bool = false;
#endif
    
    // ==================== TYPE_STRING ====================
    const char* local_string = "Local string";
    wchar_t local_wchar = L'X';
    char local_char_buffer[100] = "Buffer contents";
    
    // ==================== TYPE_STRUCT ====================
    SimpleStruct simple = {1, 2.0, "test"};
    BitFieldStruct bitfield = {1, 3, 5, -100};
    PackedStruct packed = {'X', 42, 3.14};
    OuterStruct outer = {{2, 4.0, "inner"}, 10, {20, 40.0}};
    
    // ==================== TYPE_UNION ====================
    DataUnion data_union;
    data_union.int_value = 42;
    
    UnionContainer union_container;
    union_container.type = 1;
    union_container.value.int_data = 100;
    
#ifdef __cplusplus
    AnonymousUnionStruct anon_union;
    anon_union.tag = 1;
    anon_union.as_int = 42;
#endif
    
    // ==================== TYPE_POINTER ====================
    int* local_int_ptr = &local_int;
    SimpleStruct* local_struct_ptr = &simple;
    DataUnion* local_union_ptr = &data_union;
    
    // Function pointers
    FuncPtr func_ptr = nullptr;
    VoidFuncPtr void_func_ptr = nullptr;
    
    // ==================== TYPE_ARRAY ====================
    int local_array[5] = {1, 2, 3, 4, 5};
    double matrix[2][2] = {{1.0, 2.0}, {3.0, 4.0}};
    
    // Variable-length array (C99)
#ifdef __STDC__
    if (argc > 0) {
        int vla_size = argc;
        int vla[vla_size];
        for (int i = 0; i < vla_size; i++) vla[i] = i;
    }
#endif
    
    // ==================== TYPE_CALLBACK ====================
    Comparator comp_ptr = compare_ints;
    CallbackContainer callback_container = {nullptr, nullptr};
    
    // Use callback
    int array_to_sort[] = {5, 2, 8, 1, 9};
    qsort(array_to_sort, 5, sizeof(int), comp_ptr);
    
    // ==================== C++ SPECIFIC ====================
#ifdef __cplusplus
    // ==================== TYPE_USER_STRUCT ====================
    DerivedClass derived_obj;
    MultipleInheritanceClass multi_obj;
    
    // Template instantiations
    TemplateClass<int> int_template(42);
    TemplateClass<double> double_template(3.14);
    TemplateClass<SimpleStruct> struct_template(simple);
    
    ComplexClass complex_obj;
    complex_obj.set_callback([](int x) { /* do nothing */ });
    
    // ==================== TYPE_LANG_STRUCT ====================
    // Use lambdas based on mode
    int result = 0;
    switch(mode) {
        case '1':
            result = lambda_no_capture(10);
            break;
        case '2':
            result = lambda_by_value(20);
            break;
        case '3':
            result = lambda_by_ref(30);
            break;
        case '4':
            result = lambda_mutable(40);
            break;
        default:
            result = 0;
    }
    
    // Use initializer_list
    int sum = sum_initializer_list({1, 2, 3, 4, 5});
    
    // Structured bindings
    auto [num, str] = get_pair();
    
    // Fold expression
    int fold_result = fold_sum(1, 2, 3, 4, 5);
    
    // Coroutine (if supported)
    #ifdef __cpp_impl_coroutine
    if (mode == 'c') {
        dummy_coroutine();
    }
    #endif
    
    // Output to prevent optimization
    cout << "Result: " << result 
         << " Sum: " << sum 
         << " Fold: " << fold_result
         << " Pair: " << num << "," << str << endl;
#else
    // C mode output
    printf("Mode: %c\n", mode);
    printf("Simple struct: %d %f %s\n", simple.x, simple.y, simple.name);
    printf("Union int: %d\n", data_union.int_value);
#endif
    
    // Take addresses of everything to force pointer creation
    void* addresses[] = {
        &global_int,
        &global_string,
        &simple,
        &data_union,
        &int_ptr,
        &int_array,
#ifdef __cplusplus
        &derived_obj,
        &int_template,
#endif
        &comp_ptr
    };
    
    // Compute a simple hash from addresses to ensure they're used
    size_t hash = 0;
    for (size_t i = 0; i < sizeof(addresses)/sizeof(addresses[0]); i++) {
        hash = hash * 31 + (size_t)addresses[i];
    }
    
#ifdef __cplusplus
    cout << "Hash: " << hash << endl;
#else
    printf("Hash: %zu\n", hash);
#endif
    
    return (int)(hash % 256);
}
