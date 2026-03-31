/* gengtype_test.c - Test program to trigger all type categories in gengtype.cc */

#ifdef __cplusplus
#include <iostream>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <type_traits>
using namespace std;

// Forward declarations for TYPE_UNDEFINED
struct NeverDefinedStruct;
class NeverDefinedClass;
template<typename T> class NeverDefinedTemplate;

#else
#include <stdio.h>
#include <stdbool.h>

// Forward declarations for TYPE_UNDEFINED (C mode)
struct NeverDefinedStruct;
typedef struct NeverDefinedStruct* UndefPtr;
#endif

/* ========== TYPE_UNDEFINED ========== */
// Forward declared but never defined types
struct ForwardDeclared;
typedef struct ForwardDeclared* ForwardPtr;

#ifdef __cplusplus
class ForwardClassDecl;
template<typename T> class ForwardTemplateDecl;
#endif

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
long double global_ldouble = 1.4142135623730950488L;
bool global_bool = true;

/* ========== TYPE_STRING ========== */
// String literals and character arrays
const char* global_string = "Hello, World!";
const wchar_t* global_wstring = L"Wide String";
char char_array[50] = "Character array";
const char* const_string_array[] = {"String1", "String2", "String3"};
wchar_t wide_array[] = L"Wide Array";

/* ========== TYPE_STRUCT ========== */
// Basic C structures
struct SimpleStruct {
    int x;
    double y;
    char z;
};

struct NestedStruct {
    struct SimpleStruct inner;
    int outer;
};

struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int reserved : 24;
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    double c;
};

struct FlexibleArrayStruct {
    int count;
    int data[];  // Flexible array member
};

// Global struct instances
struct SimpleStruct global_struct = {10, 20.5, 'X'};
static struct NestedStruct static_nested_struct = {{5, 10.2, 'Y'}, 100};

/* ========== TYPE_USER_STRUCT (C++ only) ========== */
#ifdef __cplusplus

class BaseClass {
private:
    int private_member;
protected:
    float protected_member;
public:
    BaseClass(int x) : private_member(x), protected_member(0.0f) {}
    virtual ~BaseClass() {}
    virtual void virtual_func() { cout << "BaseClass::virtual_func" << endl; }
    void non_virtual_func() { cout << "BaseClass::non_virtual_func" << endl; }
};

class DerivedClass : public BaseClass {
private:
    double derived_private;
public:
    DerivedClass(int x, double d) : BaseClass(x), derived_private(d) {}
    virtual void virtual_func() override { 
        cout << "DerivedClass::virtual_func" << endl; 
    }
};

class MultipleInheritanceBase1 {
public:
    virtual void base1_func() = 0;
    virtual ~MultipleInheritanceBase1() {}
};

class MultipleInheritanceBase2 {
public:
    virtual void base2_func() = 0;
    virtual ~MultipleInheritanceBase2() {}
};

class MultipleInheritanceDerived : 
    public MultipleInheritanceBase1, 
    public MultipleInheritanceBase2 {
public:
    void base1_func() override {}
    void base2_func() override {}
};

// Template class
template<typename T, typename U = int>
class TemplateClass {
private:
    T value1;
    U value2;
public:
    TemplateClass(T v1, U v2) : value1(v1), value2(v2) {}
    T get_value1() const { return value1; }
    U get_value2() const { return value2; }
    
    template<typename V>
    V process(V input) {
        return input + static_cast<V>(value1 + value2);
    }
};

// Instantiate template classes
TemplateClass<int, double> template_instance(10, 20.5);
TemplateClass<float> template_instance2(3.14f, 42);

#endif

/* ========== TYPE_UNION ========== */
// C-style union
union DataUnion {
    int int_value;
    float float_value;
    double double_value;
    char char_value;
    void* ptr_value;
};

// Union within struct
struct UnionContainer {
    int type;
    union {
        int int_data;
        float float_data;
        double double_data;
        struct SimpleStruct struct_data;
    } data;
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

// Global union instance
union DataUnion global_union = { .int_value = 100 };

/* ========== TYPE_POINTER ========== */
// Various pointer types
int* int_ptr = &global_int;
const int* const_int_ptr = &global_int;
volatile int* volatile_int_ptr = &global_int;
int* restrict restrict_int_ptr = &global_int;

double** double_ptr_ptr = &int_ptr;  // Will be reassigned
void* void_ptr = NULL;
const void* const_void_ptr = NULL;

// Function pointers
typedef int (*FuncPtr)(int, int);
typedef void (*VoidFuncPtr)(void);
typedef const char* (*StringFuncPtr)(const char*);

// Pointer to array
int (*array_ptr)[10];

#ifdef __cplusplus
// Pointer to member function
typedef void (BaseClass::*MemberFuncPtr)();
// Pointer to member data
typedef int BaseClass::*MemberDataPtr;
#endif

/* ========== TYPE_ARRAY ========== */
// Various array types
int int_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
float float_array[5][3];  // 2D array
double double_array[2][3][4];  // 3D array
char* string_array[] = {"one", "two", "three"};

// Array of structs
struct SimpleStruct struct_array[5];

// Array of pointers
int* pointer_array[10];

#ifdef __cplusplus
// Array of template instances
TemplateClass<int>* template_array[5];
#endif

/* ========== TYPE_CALLBACK ========== */
// Function pointer types and usage
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, void*);

// Structure with function pointer
struct CallbackContainer {
    Callback callback;
    void* user_data;
};

// Function using callback
void register_callback(Callback cb, void* data) {
    // Callback would be registered here
    (void)cb; (void)data;
}

// Actual callback function
void my_callback(int value, void* data) {
    (void)value; (void)data;
    // Callback implementation
}

#ifdef __cplusplus
// C++ function objects and lambdas
template<typename Func>
void process_with_callback(Func f, int value) {
    f(value);
}

// Lambda expressions
auto lambda = [](int x) { return x * 2; };
auto capturing_lambda = [global_int](int x) { return x + global_int; };
#endif

/* ========== TYPE_LANG_STRUCT (C++ only) ========== */
#ifdef __cplusplus

// Lambda with different captures
auto lambda_by_ref = [&global_int]() { return global_int; };
auto lambda_by_value = [global_int]() { return global_int; };
auto generic_lambda = [](auto x) { return x * 2; };

// std::initializer_list usage
void use_initializer_list(initializer_list<int> list) {
    for (auto x : list) {
        (void)x;
    }
}

// Structured bindings (C++17)
struct Point { int x; int y; };
auto get_point() -> Point { return {10, 20}; }

// Fold expressions (C++17)
template<typename... Args>
auto sum(Args... args) {
    return (args + ...);
}

// Coroutine handle (C++20)
#if __has_include(<coroutine>)
struct SimpleCoroutine {
    struct promise_type {
        SimpleCoroutine get_return_object() { return {}; }
        suspend_never initial_suspend() { return {}; }
        suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};
#endif

// Complex template instantiation with language features
template<typename T>
class LanguageFeatureContainer {
public:
    void process(T value) {
        // Lambda inside template
        auto lambda = [this, value]() {
            return value * 2;
        };
        
        // Structured binding
        if constexpr (is_same_v<T, Point>) {
            auto [x, y] = value;
            (void)x; (void)y;
        }
        
        // Fold expression
        auto result = sum(1, 2, 3, 4, 5);
        (void)result;
    }
    
private:
    T data;
};

// Instantiate with different types
LanguageFeatureContainer<int> lfc_int;
LanguageFeatureContainer<Point> lfc_point;

#endif

/* ========== Helper Functions ========== */
int add(int a, int b) {
    return a + b;
}

int subtract(int a, int b) {
    return a - b;
}

#ifdef __cplusplus
void cpp_specific_function() {
    // Use all C++ specific features
    DerivedClass derived(10, 3.14);
    derived.virtual_func();
    
    process_with_callback(lambda, 42);
    process_with_callback(capturing_lambda, 100);
    
    use_initializer_list({1, 2, 3, 4, 5});
    
    auto [x, y] = get_point();
    (void)x; (void)y;
    
    auto total = sum(1.0, 2.0, 3.0, 4.0);
    (void)total;
    
    lfc_int.process(10);
    lfc_point.process({5, 6});
}
#endif

/* ========== Main Function ========== */
int main(int argc, char* argv[]) {
    // Use argc to prevent dead code elimination
    int mode = (argc > 1) ? argv[1][0] : '0';
    
    /* TYPE_UNDEFINED usage */
    ForwardPtr undefined_ptr = NULL;
    (void)undefined_ptr;
    
    /* TYPE_SCALAR usage */
    int local_int = 100;
    unsigned local_uint = 200u;
    float local_float = 1.234f;
    double local_double = 5.678;
    bool local_bool = false;
    
    // Use scalars in computation
    int scalar_result = global_int + local_int - global_short * local_uint;
    (void)scalar_result;
    
    /* TYPE_STRING usage */
    const char* local_string = "Local string";
    wchar_t local_wstring[] = L"Local wide string";
    printf("String: %s\n", global_string);
    (void)local_string;
    (void)local_wstring;
    
    /* TYPE_STRUCT usage */
    struct SimpleStruct local_struct = {1, 2.0, 'A'};
    struct NestedStruct local_nested = {{2, 4.0, 'B'}, 8};
    struct BitFieldStruct bitfield = {1, 3, 7, 0};
    struct PackedStruct packed = {'Z', 99, 88.8};
    
    // Create flexible array struct (simulated)
    struct FlexibleArrayStruct* flex = (struct FlexibleArrayStruct*)
        malloc(sizeof(struct FlexibleArrayStruct) + 10 * sizeof(int));
    if (flex) {
        flex->count = 10;
        for (int i = 0; i < 10; i++) {
            flex->data[i] = i;
        }
        free(flex);
    }
    
    /* TYPE_USER_STRUCT usage (C++ only) */
#ifdef __cplusplus
    if (mode == 'c') {
        cpp_specific_function();
    }
    
    BaseClass* base_ptr = new DerivedClass(5, 10.5);
    base_ptr->virtual_func();
    delete base_ptr;
    
    MultipleInheritanceDerived multiple;
    multiple.base1_func();
    multiple.base2_func();
    
    auto result1 = template_instance.get_value1();
    auto result2 = template_instance2.process(3.14f);
    (void)result1; (void)result2;
#endif
    
    /* TYPE_UNION usage */
    union DataUnion local_union;
    local_union.int_value = 42;
    local_union.float_value = 3.14f;  // Overwrites int_value
    
    struct UnionContainer union_container;
    union_container.type = 1;
    union_container.data.int_data = 100;
    
    /* TYPE_POINTER usage */
    int* local_int_ptr = &local_int;
    int** local_int_ptr_ptr = &local_int_ptr;
    void* local_void_ptr = &local_struct;
    
    // Function pointer usage
    FuncPtr func_ptr = (mode == 'a') ? add : subtract;
    int func_result = func_ptr(10, 5);
    (void)func_result;
    
    /* TYPE_ARRAY usage */
    int local_array[5] = {1, 2, 3, 4, 5};
    int matrix[3][3] = {{1,2,3}, {4,5,6}, {7,8,9}};
    
    // Variable-length array (C99)
    int vla_size = (argc > 2) ? atoi(argv[2]) : 5;
    int vla[vla_size];
    for (int i = 0; i < vla_size; i++) {
        vla[i] = i * i;
    }
    
    // Array of structs
    for (int i = 0; i < 5; i++) {
        struct_array[i].x = i;
        struct_array[i].y = i * 1.5;
        struct_array[i].z = 'A' + i;
    }
    
    /* TYPE_CALLBACK usage */
    CallbackContainer callback_container = {my_callback, NULL};
    register_callback(my_callback, &local_int);
    
#ifdef __cplusplus
    // C++ callback usage
    process_with_callback([](int x) { return x * 3; }, 7);
#endif
    
    /* TYPE_LANG_STRUCT usage (C++ only) */
#ifdef __cplusplus
    if (mode == 'l') {
        // Force usage of language features
        auto lambda_result = lambda_by_ref();
        auto generic_result = generic_lambda(3.14);
        (void)lambda_result; (void)generic_result;
        
        #if __has_include(<coroutine>)
        SimpleCoroutine coro;
        (void)coro;
        #endif
    }
#endif
    
    /* Generate observable output based on all types */
    // Create a hash using addresses and sizes
    size_t hash = 0;
    
    // Mix in addresses
    hash ^= (size_t)&global_int;
    hash ^= (size_t)&global_struct;
    hash ^= (size_t)&global_union;
    hash ^= (size_t)int_ptr;
    hash ^= (size_t)func_ptr;
    
    // Mix in sizes
    hash ^= sizeof(struct SimpleStruct);
    hash ^= sizeof(union DataUnion);
    hash ^= sizeof(int_array);
    
    // Mix in some values
    hash ^= global_int;
    hash ^= (size_t)global_double;
    hash ^= local_int;
    
#ifdef __cplusplus
    // Add C++ specific hashes
    hash ^= (size_t)&template_instance;
    hash ^= sizeof(DerivedClass);
#endif
    
    // Print hash to ensure execution
    printf("Type analysis hash: 0x%zx\n", hash);
    
    // Use command line arguments to affect control flow
    if (argc > 3) {
        // Force different code paths
        switch (argv[3][0]) {
            case '1': return scalar_result;
            case '2': return func_result;
            case '3': return (int)hash;
            default: return 0;
        }
    }
    
    return 0;
}
