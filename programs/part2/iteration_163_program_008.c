/* gengtype_test.c - Test program to trigger all gengtype type categories */
#ifdef __cplusplus
#include <iostream>
#include <functional>
#include <initializer_list>
#include <utility>
#include <coroutine>
#include <type_traits>
using namespace std;

// Forward declarations for TYPE_UNDEFINED
struct IncompleteStruct;  // Never defined
class IncompleteClass;    // Never defined
#endif

/* ========== TYPE_UNDEFINED ========== */
struct UndefinedType;  // Forward declaration only
typedef struct UndefinedType* UndefinedPtr;

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
_Bool global_bool = 1;

/* ========== TYPE_STRING ========== */
const char* global_string = "Hello, World!";
const wchar_t* global_wstring = L"Wide String";
char global_char_array[50] = "Character array";
const char global_const_array[] = "Constant array";

/* ========== TYPE_STRUCT ========== */
// Basic structure
struct SimpleStruct {
    int x;
    double y;
    char z;
};

// Nested structure
struct OuterStruct {
    int id;
    struct SimpleStruct inner;
    struct {
        int anonymous_member;
    } anonymous;
};

// Structure with bit-fields
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    signed int value : 10;
    unsigned int : 18;  // Padding
};

// Packed structure
struct PackedStruct {
    char a;
    int b;
    double c;
} __attribute__((packed));

// Structure with flexible array member (C only)
#ifdef __cplusplus
struct FlexArrayStruct {
    int count;
    int data[1];  // Simulate flexible array in C++
};
#else
struct FlexArrayStruct {
    int count;
    int data[];  // Flexible array member
};
#endif

/* ========== TYPE_UNION ========== */
// C-style union
union DataUnion {
    int i;
    float f;
    char str[20];
    void* ptr;
};

// Union within structure
struct UnionContainer {
    int type;
    union {
        int int_val;
        float float_val;
        char* str_val;
    } data;
};

#ifdef __cplusplus
// C++11 anonymous union
struct AnonymousUnionStruct {
    int tag;
    union {
        int as_int;
        double as_double;
        char as_char[8];
    };
};
#endif

/* ========== TYPE_ARRAY ========== */
// Various arrays
int int_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
double double_array[5][3];  // 2D array
char* pointer_array[20];     // Array of pointers
struct SimpleStruct struct_array[5];  // Array of structures

// Multi-dimensional array
int multi_dim[2][3][4] = {
    {{1,2,3,4}, {5,6,7,8}, {9,10,11,12}},
    {{13,14,15,16}, {17,18,19,20}, {21,22,23,24}}
};

// Array of function pointers
typedef int (*FuncPtr)(int, int);
FuncPtr func_array[5];

#ifdef __cplusplus
/* ========== TYPE_USER_STRUCT (C++ Classes) ========== */
// Base class with different access specifiers
class BaseClass {
private:
    int private_member;
protected:
    float protected_member;
public:
    BaseClass() : private_member(0), protected_member(0.0f) {}
    virtual ~BaseClass() {}
    virtual void virtual_func() = 0;
    void public_method() {}
};

// Derived class with single inheritance
class DerivedClass : public BaseClass {
private:
    double derived_private;
public:
    DerivedClass() : derived_private(0.0) {}
    void virtual_func() override {}
    void derived_method() {}
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

class MultiInherit : public Interface1, public Interface2 {
public:
    void method1() override {}
    void method2() override {}
};

// Template class
template<typename T, typename U>
class TemplateClass {
private:
    T t_value;
    U u_value;
public:
    TemplateClass(T t, U u) : t_value(t), u_value(u) {}
    T get_t() const { return t_value; }
    U get_u() const { return u_value; }
};

// Instantiate template classes
TemplateClass<int, double> template_instance(42, 3.14);
TemplateClass<char*, BaseClass*> template_instance2(nullptr, nullptr);

/* ========== TYPE_CALLBACK ========== */
// Function pointer types
typedef int (*BinaryOp)(int, int);
typedef void (*VoidCallback)(void*);
typedef const char* (*StringFunc)(int);

// Structure with function pointer member
struct CallbackContainer {
    BinaryOp operation;
    VoidCallback callback;
    void* user_data;
};

// Function taking callback
void register_callback(VoidCallback cb, void* data) {
    if (cb) cb(data);
}

// Lambda expressions as callbacks
auto lambda_callback = [](int x) -> int { return x * 2; };

// Template with callback
template<typename Func>
void template_callback(Func f, int value) {
    f(value);
}

/* ========== TYPE_LANG_STRUCT ========== */
// Lambda with different captures
auto lambda_with_capture = [global_int](int x) {
    return x + global_int;
};

// std::initializer_list usage
void use_initializer_list(std::initializer_list<int> list) {
    for (auto& item : list) {
        // Process items
    }
}

// Structured bindings (C++17)
#ifdef __has_cpp_attribute
#if __has_cpp_attribute(fallthrough)
struct Point { double x, y; };
auto get_point() -> Point { return {1.0, 2.0}; }
#endif
#endif

// Fold expressions (C++17)
template<typename... Args>
auto sum(Args... args) {
    return (args + ...);
}

// Coroutine handle (C++20)
#if __cplusplus >= 202002L
#include <coroutine>
struct SimpleCoroutine {
    struct promise_type {
        SimpleCoroutine get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};
#endif

#endif /* __cplusplus */

/* ========== TYPE_POINTER ========== */
// Various pointer types
int* int_ptr = &global_int;
const int* const_int_ptr = &global_int;
volatile int* volatile_int_ptr = &global_int;
int** double_ptr = &int_ptr;
int* restrict restrict_ptr = &global_int;

// Pointer to array
int (*array_ptr)[10] = &int_array;

// Pointer to structure
struct SimpleStruct* struct_ptr = 0;
struct SimpleStruct** struct_double_ptr = 0;

// Pointer to union
union DataUnion* union_ptr = 0;

// Pointer to function
int (*func_ptr)(int, int);

#ifdef __cplusplus
// Pointer to member function
typedef void (BaseClass::*MemberFuncPtr)();
MemberFuncPtr member_func_ptr = &BaseClass::public_method;

// Pointer to member data
int BaseClass::*member_data_ptr = nullptr;
#endif

/* ========== Function Definitions ========== */
int add(int a, int b) {
    return a + b;
}

int multiply(int a, int b) {
    return a * b;
}

#ifdef __cplusplus
// Function using TYPE_CALLBACK
void process_with_callback(int x, int y, BinaryOp op) {
    if (op) {
        int result = op(x, y);
        // Use result
    }
}

// Function using TYPE_LANG_STRUCT features
void use_language_features() {
    // Lambda
    auto result = lambda_with_capture(10);
    
    // Initializer list
    use_initializer_list({1, 2, 3, 4, 5});
    
    // Structured binding
    #ifdef __has_cpp_attribute
    #if __has_cpp_attribute(fallthrough)
    auto [x, y] = get_point();
    #endif
    #endif
    
    // Fold expression
    auto total = sum(1, 2, 3, 4, 5);
}
#endif

/* ========== Main Function ========== */
int main(int argc, char* argv[]) {
    // Use argc to prevent dead code elimination
    int mode = (argc > 1) ? argv[1][0] : 'A';
    
    /* TYPE_SCALAR - Local variables */
    int local_int = 100;
    unsigned int local_uint = 200u;
    char local_char = 'Z';
    float local_float = 1.234f;
    double local_double = 9.876;
    _Bool local_bool = 0;
    
    /* TYPE_STRUCT - Instances */
    struct SimpleStruct s1 = {1, 2.0, 'a'};
    struct OuterStruct s2 = {100, {2, 3.0, 'b'}, {99}};
    struct BitFieldStruct s3 = {1, 3, 7, -500};
    struct PackedStruct s4 = {'X', 42, 3.14};
    struct FlexArrayStruct* s5 = (struct FlexArrayStruct*)malloc(sizeof(struct FlexArrayStruct) + 10 * sizeof(int));
    
    if (s5) {
        s5->count = 10;
        for (int i = 0; i < 10; i++) {
            s5->data[i] = i * i;
        }
    }
    
    /* TYPE_UNION - Instances */
    union DataUnion u1;
    u1.i = 42;
    
    struct UnionContainer uc;
    uc.type = 1;
    uc.data.int_val = 100;
    
    #ifdef __cplusplus
    AnonymousUnionStruct aus;
    aus.tag = 2;
    aus.as_int = 200;
    #endif
    
    /* TYPE_ARRAY - Usage */
    for (int i = 0; i < 10; i++) {
        int_array[i] = i * mode;
    }
    
    /* TYPE_POINTER - Usage */
    int_ptr = &local_int;
    func_ptr = (mode == 'A') ? add : multiply;
    
    #ifdef __cplusplus
    /* TYPE_USER_STRUCT - Instances */
    DerivedClass derived_obj;
    MultiInherit multi_obj;
    
    BaseClass* base_ptr = &derived_obj;
    base_ptr->virtual_func();
    
    // Use template instances
    int t1 = template_instance.get_t();
    double t2 = template_instance.get_u();
    
    /* TYPE_CALLBACK - Usage */
    process_with_callback(10, 20, add);
    
    // Lambda as callback
    template_callback(lambda_callback, 42);
    
    /* TYPE_LANG_STRUCT - Usage */
    use_language_features();
    
    // Coroutine if C++20
    #if __cplusplus >= 202002L
    auto coro = []() -> SimpleCoroutine {
        co_return;
    };
    #endif
    #endif
    
    /* TYPE_UNDEFINED - Usage (pointer to incomplete type) */
    UndefinedPtr undefined_pointer = 0;
    
    // Force analysis by taking address
    if (mode == 'B') {
        undefined_pointer = (UndefinedPtr)&local_int;
    }
    
    /* Generate observable output */
    unsigned long hash = 0;
    
    // Hash sizes of various types
    hash ^= (unsigned long)sizeof(struct SimpleStruct);
    hash ^= (unsigned long)sizeof(union DataUnion);
    hash ^= (unsigned long)sizeof(int_array);
    #ifdef __cplusplus
    hash ^= (unsigned long)sizeof(DerivedClass);
    hash ^= (unsigned long)sizeof(template_instance);
    #endif
    
    // Hash addresses
    hash ^= (unsigned long)&global_int;
    hash ^= (unsigned long)&s1;
    hash ^= (unsigned long)&u1;
    hash ^= (unsigned long)int_ptr;
    hash ^= (unsigned long)func_ptr;
    
    // Use function pointer
    if (func_ptr) {
        int result = func_ptr(10, 20);
        hash ^= (unsigned long)result;
    }
    
    // Use arrays
    for (int i = 0; i < 10; i++) {
        hash ^= (unsigned long)int_array[i];
    }
    
    // Print hash to prevent optimization
    #ifdef __cplusplus
    cout << "Hash: " << hash << endl;
    #else
    printf("Hash: %lu\n", hash);
    #endif
    
    // Cleanup
    if (s5) free(s5);
    
    return (int)(hash % 256);
}
