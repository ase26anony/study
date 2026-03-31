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

// For TYPE_LANG_STRUCT: C++17 fold expression
template<typename... Args>
auto sum(Args... args) {
    return (args + ...);  // Fold expression
}

// For TYPE_LANG_STRUCT: C++17 structured bindings
struct Point3D {
    double x, y, z;
};

// For TYPE_LANG_STRUCT: Coroutine (C++20)
#ifdef __cpp_coroutines
struct task {
    struct promise_type {
        task get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};
#endif

// For TYPE_LANG_STRUCT: Lambda with different captures
auto make_lambda() {
    int x = 10;
    static int y = 20;
    return [x, &y](int z) mutable -> int {
        y = x + z;
        return y;
    };
}

// For TYPE_USER_STRUCT: Template classes
template<typename T>
class Container {
private:
    T* data;
    size_t size;
public:
    Container(size_t n) : size(n), data(new T[n]) {}
    virtual ~Container() { delete[] data; }
    virtual T& operator[](size_t idx) { return data[idx]; }
    
    // Template member function
    template<typename U>
    U convert(size_t idx) { return static_cast<U>(data[idx]); }
};

// For TYPE_USER_STRUCT: Multiple inheritance
class Base1 {
protected:
    int base1_data;
public:
    virtual void foo() = 0;
    virtual ~Base1() {}
};

class Base2 {
protected:
    double base2_data;
public:
    virtual void bar() = 0;
    virtual ~Base2() {}
};

class Derived : public Base1, public Base2 {
private:
    char derived_data;
public:
    void foo() override { base1_data = 42; }
    void bar() override { base2_data = 3.14; }
    virtual void baz() { derived_data = 'A'; }
};

// For TYPE_USER_STRUCT: Class with different access specifiers
class AccessDemo {
public:
    int public_var;
    void public_func() {}
    
protected:
    float protected_var;
    virtual void protected_func() {}
    
private:
    char private_var;
    void private_func() {}
    
    // Nested class
    class Nested {
    public:
        int nested_data;
    };
};

#else
// C mode includes
#include <stdio.h>
#include <stddef.h>
#endif

/* ========== TYPE_UNDEFINED ========== */
// Forward declaration of incomplete type (never defined)
struct undefined_struct;
typedef struct undefined_struct UndefinedType;

// Pointer to undefined type
UndefinedType* global_undefined_ptr = NULL;

/* ========== TYPE_SCALAR ========== */
// Global scalar variables
int global_int = 42;
unsigned int global_uint = 100u;
char global_char = 'X';
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
_Bool global_bool = 1;

#ifdef __cplusplus
bool global_cppbool = true;
#endif

/* ========== TYPE_STRING ========== */
// String literals and character arrays
const char* global_cstring = "Hello, World!";
const wchar_t* global_wstring = L"Wide String";
char global_char_array[50] = "Character array";
const char global_const_array[] = "Constant array";

/* ========== TYPE_STRUCT ========== */
// Simple structure
struct SimpleStruct {
    int id;
    char name[32];
    float value;
};

// Nested structure
struct OuterStruct {
    int outer_id;
    struct InnerStruct {
        int inner_id;
        double inner_data;
    } inner;
    struct SimpleStruct simple;
};

// Structure with bit-fields
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    signed int value : 8;
    unsigned int : 4;  // Padding
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
    double data[];  // Flexible array member
};
#endif

// Anonymous structure (C11/C++)
struct AnonymousStruct {
    union {
        int as_int;
        float as_float;
    };
    struct {
        char x;
        char y;
    };
};

/* ========== TYPE_UNION ========== */
// C-style union
union DataUnion {
    int i;
    float f;
    double d;
    char str[16];
    void* ptr;
};

// Union within structure
struct UnionContainer {
    int type;
    union {
        int int_data;
        float float_data;
        struct SimpleStruct struct_data;
    } value;
};

#ifdef __cplusplus
// C++11 anonymous union within class
class UnionClass {
public:
    int tag;
    union {
        int int_member;
        double double_member;
        char char_member;
    };
};
#endif

/* ========== TYPE_POINTER ========== */
// Various pointer types
int* int_ptr = &global_int;
const int* const_int_ptr = &global_int;
volatile int* volatile_int_ptr = &global_int;
int* restrict restrict_int_ptr = &global_int;
int** double_int_ptr = &int_ptr;
const volatile int* const_volatile_int_ptr = &global_int;

// Pointers to different types
struct SimpleStruct* struct_ptr = NULL;
union DataUnion* union_ptr = NULL;
void* void_ptr = NULL;
const void* const_void_ptr = NULL;

// Function pointers
typedef int (*FuncPtr)(int, int);
typedef void (*VoidFuncPtr)(void);
typedef const char* (*StringFuncPtr)(const char*);

// Pointer to array
int (*array_ptr)[10];

// Pointer to pointer to function
int (*(*func_ptr_ptr))(int, int);

#ifdef __cplusplus
// Pointer to member function
typedef void (Derived::*MemberFuncPtr)();
// Pointer to data member
typedef int AccessDemo::*DataMemberPtr;
#endif

/* ========== TYPE_ARRAY ========== */
// Various arrays
int int_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
float float_array[5][3];  // 2D array
double double_array[2][3][4];  // 3D array
char* string_array[] = {"one", "two", "three", NULL};

// Array of structures
struct SimpleStruct struct_array[5];

// Array of pointers
int* pointer_array[8];

// Array of function pointers
FuncPtr func_array[3];

// Incomplete array in structure (C only)
#ifndef __cplusplus
struct IncompleteArray {
    int count;
    int items[];
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

// Function using callback
void register_callback(Callback cb, void* data) {
    static CallbackContainer container;
    container.callback = cb;
    container.user_data = data;
}

// Example callback function
void example_callback(int value, void* data) {
    *(int*)data = value * 2;
}

#ifdef __cplusplus
// C++ function objects and lambdas
template<typename F>
void process_with_callback(F func) {
    func(42);
}

// std::function callback
function<void(int)> std_callback;
#endif

/* ========== Helper Functions ========== */
// Function using various types
int process_data(struct SimpleStruct* s, union DataUnion* u, int* arr, size_t len) {
    // Local scalar variables
    int local_int = 0;
    unsigned local_uint = 0;
    float local_float = 0.0f;
    double local_double = 0.0;
    
    // Local pointer
    int* local_ptr = &local_int;
    
    // Local array
    char local_buffer[256];
    
    // Use all parameters to prevent optimization
    if (s) local_int += s->id;
    if (u) local_int += u->i;
    for (size_t i = 0; i < len; i++) {
        local_int += arr[i];
    }
    
    return local_int;
}

// Function with variable arguments
#ifdef __cplusplus
template<typename... Args>
int variadic_sum(Args... args) {
    return sum(args...);
}
#endif

/* ========== Main Function ========== */
int main(int argc, char* argv[]) {
    // Use argc to prevent dead code elimination
    int mode = (argc > 1) ? argv[1][0] : 'A';
    
    /* ========== TYPE_UNDEFINED usage ========== */
    // Create pointer to undefined type
    UndefinedType* local_undefined = NULL;
    if (mode == 'U') {
        // This branch ensures the type is referenced
        printf("Undefined type pointer size: %zu\n", sizeof(local_undefined));
    }
    
    /* ========== TYPE_SCALAR usage ========== */
    // Use all global scalars
    int scalar_sum = global_int + global_uint + global_char + global_schar;
    scalar_sum += global_short + global_ushort + global_long + global_ulong;
    scalar_sum += (int)global_float + (int)global_double;
    
    /* ========== TYPE_STRING usage ========== */
    // Use strings
    const char* local_string = "Local string";
    wchar_t local_wstring[] = L"Local wide string";
    
    if (mode == 'S') {
        printf("String: %s\n", global_cstring);
        printf("Wide string length: %d\n", (int)wcslen(global_wstring));
    }
    
    /* ========== TYPE_STRUCT usage ========== */
    // Instantiate structures
    struct SimpleStruct s1 = {1, "Test", 3.14f};
    struct OuterStruct outer = {100, {200, 2.718}, {300, "Nested", 1.414f}};
    struct BitFieldStruct bits = {1, 7, 15, -50};
    struct PackedStruct packed = {'X', 42, 3.14159};
    
    // Use anonymous struct members
    struct AnonymousStruct anon;
    anon.as_int = 42;
    anon.x = 'A';
    anon.y = 'B';
    
    /* ========== TYPE_USER_STRUCT usage (C++) ========== */
#ifdef __cplusplus
    if (mode == 'C') {
        // Instantiate C++ classes
        Container<int> int_container(10);
        Container<double> double_container(5);
        
        Derived derived_obj;
        Base1* b1 = &derived_obj;
        Base2* b2 = &derived_obj;
        
        b1->foo();
        b2->bar();
        derived_obj.baz();
        
        AccessDemo access_obj;
        access_obj.public_var = 100;
        
        // Use template member function
        double converted = int_container.convert<double>(0);
        cout << "Converted: " << converted << endl;
    }
#endif
    
    /* ========== TYPE_UNION usage ========== */
    union DataUnion data_union;
    data_union.i = 42;
    data_union.f = 3.14f;  // Overwrites i
    
    struct UnionContainer union_container;
    union_container.type = 1;
    union_container.value.int_data = 100;
    
#ifdef __cplusplus
    UnionClass union_class;
    union_class.tag = 2;
    union_class.double_member = 2.71828;
#endif
    
    /* ========== TYPE_POINTER usage ========== */
    // Use various pointers
    int value = 42;
    int* ptr1 = &value;
    int** ptr2 = &ptr1;
    int*** ptr3 = &ptr2;
    
    // Use function pointers
    FuncPtr add_func = [](int a, int b) -> int { return a + b; };
    int result = add_func(10, 20);
    
    // Complex pointer usage
    func_ptr_ptr = &add_func;
    result = (*func_ptr_ptr)(30, 40);
    
#ifdef __cplusplus
    // Pointer to member
    MemberFuncPtr mem_func = &Derived::baz;
    DataMemberPtr data_mem = &AccessDemo::public_var;
#endif
    
    /* ========== TYPE_ARRAY usage ========== */
    // Initialize and use arrays
    for (int i = 0; i < 10; i++) {
        int_array[i] = i * i;
    }
    
    // Multi-dimensional array
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 4; k++) {
                double_array[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    // Array of structures
    for (int i = 0; i < 5; i++) {
        struct_array[i].id = i;
        snprintf(struct_array[i].name, 32, "Struct%d", i);
        struct_array[i].value = i * 1.1f;
    }
    
    /* ========== TYPE_CALLBACK usage ========== */
    // Use function pointers as callbacks
    int callback_data = 0;
    register_callback(example_callback, &callback_data);
    
    // Call through function pointer
    if (mode == 'F') {
        example_callback(21, &callback_data);
    }
    
#ifdef __cplusplus
    // C++ callbacks
    process_with_callback([](int x) {
        cout << "Lambda callback: " << x << endl;
    });
    
    // std::function
    std_callback = [](int val) {
        cout << "std::function callback: " << val << endl;
    };
    std_callback(99);
    
    // Use fold expression
    int fold_result = variadic_sum(1, 2, 3, 4, 5);
    cout << "Fold result: " << fold_result << endl;
    
    // Use structured bindings
    Point3D point{1.0, 2.0, 3.0};
    auto [x, y, z] = point;
    cout << "Point: " << x << ", " << y << ", " << z << endl;
    
    // Use lambda with captures
    auto lambda = make_lambda();
    lambda(5);
    
    // Use initializer_list
    initializer_list<int> init_list = {1, 2, 3, 4, 5};
    for (auto n : init_list) {
        cout << n << " ";
    }
    cout << endl;
#endif
    
    /* ========== TYPE_LANG_STRUCT usage (C++) ========== */
#ifdef __cplusplus
    if (mode == 'L') {
        // Complex template instantiation
        vector<Container<int>> container_vec;
        container_vec.emplace_back(5);
        container_vec.emplace_back(10);
        
        // Tuple with structured binding
        auto tuple_data = make_tuple(1, 2.0, "three");
        auto [a, b, c] = tuple_data;
        
        // Type traits
        cout << "Is pointer: " << is_pointer<decltype(int_ptr)>::value << endl;
        
#ifdef __cpp_coroutines
        // Coroutine type (if supported)
        auto coro = []() -> task {
            co_return;
        };
#endif
    }
#endif
    
    /* ========== Final output ========== */
    // Compute a hash using all type information
    size_t hash = 0;
    hash ^= (size_t)&global_int;
    hash ^= (size_t)global_cstring;
    hash ^= sizeof(struct SimpleStruct);
    hash ^= sizeof(union DataUnion);
    hash ^= (size_t)add_func;
    hash ^= sizeof(int_array);
    
#ifdef __cplusplus
    hash ^= typeid(Derived).hash_code();
#endif
    
    // Print something to ensure execution
    printf("Mode: %c, Hash: 0x%zx, Result: %d\n", mode, hash, result + scalar_sum + callback_data);
    
    // Return based on mode to ensure all code paths are considered
    switch (mode) {
        case 'U': return (int)(hash & 0xFF);
        case 'S': return scalar_sum & 0xFF;
        case 'C': 
#ifdef __cplusplus
            return result & 0xFF;
#else
            return 0;
#endif
        case 'F': return callback_data;
        case 'L': 
#ifdef __cplusplus
            return fold_result & 0xFF;
#else
            return 0;
#endif
        default: return (int)hash;
    }
}
