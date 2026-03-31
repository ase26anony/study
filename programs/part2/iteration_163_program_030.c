/* gengtype_test.c - Test program to trigger all gengtype type categories */
#ifdef __cplusplus
#include <iostream>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <type_traits>
#include <vector>
using namespace std;

// C++20 coroutine support if available
#ifdef __cpp_coroutines
#include <coroutine>
#endif
#endif

/* ========== TYPE_UNDEFINED ========== */
struct incomplete_type;  // Forward declaration only - TYPE_UNDEFINED
typedef incomplete_type* incomplete_ptr_t;

/* ========== TYPE_SCALAR ========== */
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
long double global_ldouble = 1.618033988749895L;
_Bool global_bool = 1;

/* ========== TYPE_STRING ========== */
// String literals and character arrays
const char* global_string = "Hello, World!";
const wchar_t* global_wstring = L"Wide String";
char global_char_array[50] = "Character array";
const char global_const_array[] = "Constant array";

/* ========== TYPE_STRUCT ========== */
// Basic C structures
struct SimpleStruct {
    int x;
    double y;
    char name[20];
};

// Nested structure
struct OuterStruct {
    int id;
    struct SimpleStruct inner;
    struct {
        int anonymous_member;
    } anon;
};

// Structure with bit-fields
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    signed int value : 10;
};

// Packed structure
struct PackedStruct {
    char a;
    int b;
    double c;
} __attribute__((packed));

// Structure with flexible array member (C only)
#ifndef __cplusplus
struct FlexArrayStruct {
    int count;
    int data[];
};
#endif

// Global struct instances
struct SimpleStruct global_struct = {10, 3.14, "Test"};
struct OuterStruct global_outer = {1, {5, 2.71, "Inner"}, {42}};

/* ========== TYPE_USER_STRUCT (C++ only) ========== */
#ifdef __cplusplus
// Base class with different access specifiers
class BaseClass {
private:
    int private_member;
protected:
    float protected_member;
public:
    BaseClass() : private_member(0), protected_member(0.0f) {}
    virtual ~BaseClass() {}
    virtual void virtual_func() { cout << "BaseClass::virtual_func" << endl; }
    void public_method() {}
};

// Derived class with single inheritance
class DerivedClass : public BaseClass {
private:
    double derived_private;
public:
    DerivedClass() : derived_private(0.0) {}
    void virtual_func() override { cout << "DerivedClass::virtual_func" << endl; }
};

// Multiple inheritance
class Interface1 {
public:
    virtual void interface1_method() = 0;
    virtual ~Interface1() {}
};

class Interface2 {
public:
    virtual void interface2_method() = 0;
    virtual ~Interface2() {}
};

class MultipleInheritanceClass : public BaseClass, public Interface1, public Interface2 {
public:
    void interface1_method() override {}
    void interface2_method() override {}
    void virtual_func() override {}
};

// Template class
template<typename T, typename U>
class TemplateClass {
private:
    T template_member1;
    U template_member2;
public:
    TemplateClass(T t, U u) : template_member1(t), template_member2(u) {}
    T get_first() { return template_member1; }
    U get_second() { return template_member2; }
};

// Instantiate template classes
TemplateClass<int, double> global_template_int_double(42, 3.14);
TemplateClass<char*, SimpleStruct> global_template_char_struct("Test", {1, 2.0, "Name"});
#endif

/* ========== TYPE_UNION ========== */
// C-style union
union DataUnion {
    int i;
    float f;
    double d;
    char str[20];
};

// Union within structure
struct UnionContainer {
    int type;
    union {
        int int_value;
        float float_value;
        char* string_value;
    } data;
};

// C++11 anonymous union within structure
#ifdef __cplusplus
struct AnonymousUnionStruct {
    int tag;
    union {
        int as_int;
        double as_double;
        void* as_pointer;
    };
};
#endif

// Global union instances
union DataUnion global_union = {.i = 100};
struct UnionContainer global_union_container = {1, {.int_value = 42}};

/* ========== TYPE_POINTER ========== */
// Various pointer types
int* int_ptr = &global_int;
const int* const_int_ptr = &global_int;
volatile int* volatile_int_ptr = &global_int;
int** double_int_ptr = &int_ptr;
int* restrict restrict_ptr = &global_int;

// Pointers to different types
struct SimpleStruct* struct_ptr = &global_struct;
union DataUnion* union_ptr = &global_union;
incomplete_ptr_t incomplete_ptr = nullptr;

// Function pointers
typedef int (*FuncPtr)(int, double);
typedef void (*VoidFuncPtr)(void);
typedef const char* (*StringFuncPtr)(const char*);

// Pointer to array
int (*array_ptr)[10];

// Pointer to pointer to function
int (*(*func_ptr_ptr))(int);

#ifdef __cplusplus
// Pointer to member function
typedef void (BaseClass::*MemberFuncPtr)();
// Pointer to data member
typedef int BaseClass::*DataMemberPtr;
#endif

/* ========== TYPE_ARRAY ========== */
// Various arrays
int int_array[10] = {0,1,2,3,4,5,6,7,8,9};
float float_array[5][3];  // 2D array
double double_3d_array[2][3][4];  // 3D array
char* string_array[] = {"one", "two", "three", NULL};

// Array of structures
struct SimpleStruct struct_array[5];

// Array of pointers
int* pointer_array[10];

// Array of function pointers
FuncPtr func_ptr_array[3];

// Incomplete array in struct (C only)
#ifndef __cplusplus
struct IncompleteArrayStruct {
    int count;
    int elements[];
};
#endif

/* ========== TYPE_CALLBACK ========== */
// Function pointer types
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, void*);

// Structure with function pointer member
struct CallbackContainer {
    Callback callback;
    void* user_data;
};

// Functions to use as callbacks
int compare_ints(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

void sample_callback(int value, void* data) {
    // Do nothing
}

#ifdef __cplusplus
// C++ function objects and lambdas
template<typename Func>
void execute_callback(Func f) {
    f(42);
}

// Function taking std::function
void process_function(std::function<int(int)> func) {
    func(100);
}
#endif

/* ========== TYPE_LANG_STRUCT (C++ only) ========== */
#ifdef __cplusplus
// Lambda expressions with different captures
auto lambda_no_capture = []() { return 42; };
auto lambda_by_value = [global_int]() { return global_int; };
auto lambda_by_ref = [&global_int]() { return global_int; };
auto lambda_mutable = [global_int]() mutable { return global_int++; };

// std::initializer_list usage
auto init_list_func(std::initializer_list<int> list) {
    return list.size();
}

// Structured bindings (C++17)
#ifdef __cpp_structured_bindings
auto structured_binding_func() {
    struct Point { int x; int y; };
    return Point{10, 20};
}
#endif

// Fold expressions (C++17)
#ifdef __cpp_fold_expressions
template<typename... Args>
auto sum(Args... args) {
    return (args + ...);
}
#endif

// Coroutine support (C++20)
#ifdef __cpp_coroutines
struct SimpleCoroutine {
    struct promise_type {
        SimpleCoroutine get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

SimpleCoroutine test_coroutine() {
    co_return;
}
#endif

// Variadic templates
template<typename... Ts>
struct VariadicTemplate {};

// Alias template
template<typename T>
using AliasTemplate = vector<T>;
#endif

/* ========== Helper functions ========== */
// Function using function pointer
void sort_with_comparator(int* array, size_t size, Comparator comp) {
    // qsort(array, size, sizeof(int), comp);
}

// Function taking various pointer types
void process_pointers(int* p1, const int* p2, volatile int* p3, int** p4) {
    *p1 = *p2 + (int)*p3;
    **p4 = *p1;
}

#ifdef __cplusplus
// Template function to force instantiation
template<typename T>
T template_function(T value) {
    static int counter = 0;
    counter++;
    return value + static_cast<T>(counter);
}

// Explicit template instantiations
template int template_function<int>(int);
template double template_function<double>(double);
#endif

/* ========== Main function ========== */
int main(int argc, char* argv[]) {
    // Use argc to prevent dead code elimination
    int mode = (argc > 1) ? argv[1][0] : 'A';
    
    /* ========== TYPE_SCALAR ========== */
    // Local scalar variables
    int local_int = 100;
    unsigned int local_uint = 200u;
    char local_char = 'B';
    float local_float = 1.234f;
    double local_double = 4.567;
    _Bool local_bool = 0;
    
    /* ========== TYPE_STRING ========== */
    const char* local_string = "Local string";
    wchar_t local_wchar = L'X';
    char local_char_buffer[100] = "Buffer";
    
    /* ========== TYPE_STRUCT ========== */
    struct SimpleStruct local_struct = {20, 6.28, "Local"};
    struct OuterStruct local_outer = {2, {10, 3.14, "Nested"}, {99}};
    struct BitFieldStruct local_bitfield = {1, 3, 7, -100};
    struct PackedStruct local_packed = {'Z', 123, 45.67};
    
    /* ========== TYPE_USER_STRUCT (C++ only) ========== */
#ifdef __cplusplus
    BaseClass* base_ptr = nullptr;
    DerivedClass derived_obj;
    MultipleInheritanceClass multi_obj;
    
    // Use template classes
    TemplateClass<float, int> local_template(3.14f, 42);
    
    // Virtual function call
    if (mode == 'V') {
        base_ptr = &derived_obj;
        base_ptr->virtual_func();
    }
#endif
    
    /* ========== TYPE_UNION ========== */
    union DataUnion local_union;
    local_union.i = 999;
    
    struct UnionContainer local_union_container = {2, {.float_value = 3.14f}};
    
#ifdef __cplusplus
    AnonymousUnionStruct anon_union_struct;
    anon_union_struct.tag = 1;
    anon_union_struct.as_int = 1000;
#endif
    
    /* ========== TYPE_POINTER ========== */
    // Take addresses of various types
    int* local_int_ptr = &local_int;
    struct SimpleStruct* local_struct_ptr = &local_struct;
    union DataUnion* local_union_ptr = &local_union;
    
    // Function pointer assignment
    FuncPtr local_func_ptr = NULL;
    VoidFuncPtr local_void_func_ptr = NULL;
    
    /* ========== TYPE_ARRAY ========== */
    // Initialize arrays
    for (int i = 0; i < 10; i++) {
        int_array[i] = i * 2;
        pointer_array[i] = &int_array[i];
    }
    
    // Multi-dimensional array access
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 4; k++) {
                double_3d_array[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    // Variable-length array (C99)
#ifndef __cplusplus
    if (argc > 1) {
        int vla_size = argc;
        int vla[vla_size];
        for (int i = 0; i < vla_size; i++) {
            vla[i] = i * 10;
        }
    }
#endif
    
    /* ========== TYPE_CALLBACK ========== */
    // Use function pointers
    CallbackContainer callback_container = {sample_callback, NULL};
    
    if (mode == 'C') {
        sort_with_comparator(int_array, 10, compare_ints);
        callback_container.callback(42, NULL);
    }
    
#ifdef __cplusplus
    // Use lambdas as callbacks
    if (mode == 'L') {
        auto lambda = [](int x) { return x * 2; };
        execute_callback([&local_int](int val) { local_int += val; });
        process_function(lambda);
        
        // Lambda with capture
        int capture_value = 100;
        auto capturing_lambda = [capture_value](int x) { return x + capture_value; };
        process_function(capturing_lambda);
    }
#endif
    
    /* ========== TYPE_LANG_STRUCT (C++ only) ========== */
#ifdef __cplusplus
    // Use various C++ language constructs
    if (mode == 'S') {
        // Lambda usage
        int lambda_result = lambda_no_capture();
        lambda_by_value();
        lambda_by_ref();
        
        // initializer_list
        auto list = {1, 2, 3, 4, 5};
        init_list_func(list);
        
        // Structured binding
        #ifdef __cpp_structured_bindings
        auto [x, y] = structured_binding_func();
        #endif
        
        // Fold expression
        #ifdef __cpp_fold_expressions
        int fold_result = sum(1, 2, 3, 4, 5);
        #endif
        
        // Coroutine
        #ifdef __cpp_coroutines
        test_coroutine();
        #endif
        
        // Variadic template
        VariadicTemplate<int, double, char> variadic_instance;
        
        // Alias template
        AliasTemplate<float> alias_instance;
        alias_instance.push_back(1.0f);
    }
#endif
    
    /* ========== TYPE_UNDEFINED ========== */
    // Use incomplete type pointer (even though type is undefined)
    incomplete_ptr_t local_incomplete_ptr = NULL;
    if (mode == 'U') {
        // This would normally cause issues, but we're just testing type analysis
        // local_incomplete_ptr = (incomplete_ptr_t)&local_int; // Cast for demonstration
    }
    
    /* ========== Generate observable output ========== */
    // Compute a hash based on sizes and addresses to ensure code executes
    unsigned long hash = 0;
    
    // Mix in sizes of types
    hash ^= sizeof(int);
    hash ^= sizeof(double) << 8;
    hash ^= sizeof(struct SimpleStruct) << 16;
    hash ^= sizeof(union DataUnion) << 24;
    
    // Mix in addresses (masked)
    hash ^= ((unsigned long)&global_int) & 0xFFFF;
    hash ^= ((unsigned long)&global_struct) & 0xFFFF0000;
    hash ^= ((unsigned long)int_ptr) & 0xFF000000;
    
    // Mix in values
    hash ^= global_int;
    hash ^= local_int;
    hash ^= (unsigned long)local_struct.x;
    
#ifdef __cplusplus
    // C++ specific hashing
    hash ^= sizeof(BaseClass);
    #ifdef __cpp_structured_bindings
    hash ^= 0x12345678;
    #endif
#endif
    
    // Print hash to ensure execution
#ifdef __cplusplus
    cout << "Hash: " << hash << endl;
    cout << "Mode: " << mode << endl;
#else
    printf("Hash: %lu\n", hash);
    printf("Mode: %c\n", mode);
#endif
    
    // Return hash as exit code (masked to 8 bits)
    return (int)(hash & 0xFF);
}
