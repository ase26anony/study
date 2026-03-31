#ifdef __cplusplus
#include <iostream>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <type_traits>
#endif

/* ==================== TYPE_UNDEFINED ==================== */
struct incomplete_struct;  // Forward declaration - never defined
typedef incomplete_struct* incomplete_ptr_t;

/* ==================== TYPE_SCALAR ==================== */
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
float global_float = 3.14f;
double global_double = 2.71828;
long double global_ldouble = 1.41421356L;
_Bool global_bool = 1;

/* ==================== TYPE_STRING ==================== */
const char* global_cstring = "Hello, World!";
const wchar_t* global_wstring = L"Wide String";
char global_char_array[50] = "Character array";
const char global_const_array[] = "Constant array";

/* ==================== TYPE_STRUCT ==================== */
struct SimpleStruct {
    int x;
    double y;
    char name[20];
};

struct NestedStruct {
    SimpleStruct inner;
    int outer;
};

struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int : 4;  // Padding
    signed int value : 8;
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

/* ==================== TYPE_UNION ==================== */
union SimpleUnion {
    int as_int;
    float as_float;
    char as_char[4];
};

struct StructWithAnonymousUnion {
    int type;
    union {
        int int_val;
        float float_val;
        void* ptr_val;
    };
};

/* ==================== TYPE_ARRAY ==================== */
int global_1d_array[10] = {0,1,2,3,4,5,6,7,8,9};
int global_2d_array[3][4] = {{1,2,3,4},{5,6,7,8},{9,10,11,12}};
int global_3d_array[2][3][4];
char* global_ptr_array[5];
SimpleStruct global_struct_array[5];

/* ==================== TYPE_CALLBACK ==================== */
typedef int (*SimpleCallback)(int, double);
typedef void (*VoidCallback)(void);
typedef const char* (*StringCallback)(const char*);

struct CallbackContainer {
    SimpleCallback func1;
    VoidCallback func2;
    StringCallback func3;
};

/* ==================== C++ Specific Types ==================== */
#ifdef __cplusplus

/* ==================== TYPE_USER_STRUCT ==================== */
class BaseClass {
public:
    BaseClass() : base_value(0) {}
    virtual ~BaseClass() {}
    virtual void virtual_func() = 0;
    int get_value() const { return base_value; }
protected:
    int base_value;
};

class DerivedClass : public BaseClass {
public:
    DerivedClass(int val) { base_value = val; }
    void virtual_func() override { base_value *= 2; }
private:
    double private_data;
};

class MultipleInheritanceBase1 {
public:
    virtual void func1() {}
    int data1;
};

class MultipleInheritanceBase2 {
public:
    virtual void func2() {}
    double data2;
};

class MultipleInherited : public MultipleInheritanceBase1, 
                          public MultipleInheritanceBase2 {
public:
    char extra_data;
};

template<typename T>
class TemplateClass {
public:
    TemplateClass(T val) : value(val) {}
    T get_value() const { return value; }
    void set_value(T val) { value = val; }
private:
    T value;
};

/* ==================== TYPE_LANG_STRUCT ==================== */
// Lambda expressions with different captures
auto lambda_no_capture = [](int x) { return x * 2; };
auto lambda_with_capture = [global_int](int x) { return x + global_int; };
auto lambda_mutable = [counter = 0]() mutable { return ++counter; };

// std::initializer_list usage
template<typename T>
T sum_initializer_list(std::initializer_list<T> list) {
    T total = 0;
    for (const auto& item : list) {
        total += item;
    }
    return total;
}

// Structured bindings (C++17)
struct Point { int x; int y; };
auto get_point() -> Point { return {10, 20}; }

// Fold expressions (C++17)
template<typename... Args>
auto sum_fold(Args... args) {
    return (args + ...);
}

// Coroutine handle (C++20)
#ifdef __cpp_coroutines
struct DummyCoroutine {
    struct promise_type {
        DummyCoroutine get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};
#endif

#endif /* __cplusplus */

/* ==================== Function Definitions ==================== */
int simple_callback_impl(int a, double b) {
    return a + (int)b;
}

void void_callback_impl(void) {
    global_int++;
}

const char* string_callback_impl(const char* str) {
    return str;
}

/* ==================== Main Function ==================== */
int main(int argc, char* argv[]) {
    /* Force usage of argc/argv to prevent dead code elimination */
    int mode = (argc > 1) ? argv[1][0] : 'A';
    
    /* ========== TYPE_UNDEFINED usage ========== */
    incomplete_ptr_t undefined_ptr = nullptr;
    
    /* ========== TYPE_SCALAR usage ========== */
    int local_int = 100;
    unsigned int local_uint = 200u;
    float local_float = 1.5f;
    double local_double = 3.14159;
    _Bool local_bool = 0;
    
    /* ========== TYPE_STRING usage ========== */
    const char* local_string = "Local string";
    wchar_t wide_local[] = L"Local wide";
    
    /* ========== TYPE_STRUCT usage ========== */
    SimpleStruct s1 = {10, 20.5, "Test"};
    NestedStruct ns = {{5, 10.2, "Inner"}, 100};
    BitFieldStruct bfs = {1, 5, 9, -50};
    PackedStruct ps = {'X', 42, 3.14};
    
    /* ========== TYPE_UNION usage ========== */
    SimpleUnion su;
    su.as_int = 0xDEADBEEF;
    
    StructWithAnonymousUnion sau;
    sau.type = 1;
    sau.int_val = 100;
    
    /* ========== TYPE_ARRAY usage ========== */
    int local_array[5] = {1, 2, 3, 4, 5};
    int multi_dim[2][3] = {{1,2,3},{4,5,6}};
    
    // Variable length array (C99)
    #ifdef __STDC_VERSION__
    #if __STDC_VERSION__ >= 199901L
    int vla_size = (argc > 2) ? 10 : 5;
    int vla[vla_size];
    for (int i = 0; i < vla_size; i++) vla[i] = i;
    #endif
    #endif
    
    /* ========== TYPE_POINTER usage ========== */
    int* int_ptr = &local_int;
    const int* const_int_ptr = &global_int;
    volatile int* volatile_ptr = &local_int;
    int** double_ptr = &int_ptr;
    int* restrict restrict_ptr = int_ptr;
    
    SimpleStruct* struct_ptr = &s1;
    SimpleUnion* union_ptr = &su;
    
    /* ========== TYPE_CALLBACK usage ========== */
    SimpleCallback cb1 = simple_callback_impl;
    VoidCallback cb2 = void_callback_impl;
    StringCallback cb3 = string_callback_impl;
    
    CallbackContainer callbacks = {cb1, cb2, cb3};
    
    /* ========== Execute based on mode ========== */
    int result = 0;
    
    switch(mode) {
        case 'A':
            result = cb1(10, 20.5);
            break;
        case 'B':
            cb2();
            result = global_int;
            break;
        case 'C':
            result = s1.x + ns.outer;
            break;
        case 'D':
            result = su.as_int & 0xFF;
            break;
        default:
            result = -1;
    }
    
    /* ========== C++ Specific Code ========== */
    #ifdef __cplusplus
    
    /* TYPE_USER_STRUCT usage */
    DerivedClass derived(42);
    derived.virtual_func();
    
    MultipleInherited multi;
    multi.data1 = 10;
    multi.data2 = 20.5;
    multi.extra_data = 'Z';
    
    TemplateClass<int> template_int(100);
    TemplateClass<double> template_double(3.14);
    TemplateClass<SimpleStruct> template_struct({1, 2.0, "Template"});
    
    /* TYPE_LANG_STRUCT usage */
    auto lambda_result = lambda_no_capture(21);
    auto lambda_capture_result = lambda_with_capture(10);
    auto lambda_mutable_result = lambda_mutable();
    
    auto init_list_sum = sum_initializer_list({1, 2, 3, 4, 5});
    
    auto [px, py] = get_point();  // Structured binding
    
    auto fold_sum = sum_fold(1, 2, 3, 4, 5);
    
    #ifdef __cpp_coroutines
    DummyCoroutine coro;
    #endif
    
    // Force template instantiation
    result += template_int.get_value();
    result += (int)template_double.get_value();
    result += template_struct.get_value().x;
    
    #endif /* __cplusplus */
    
    /* ========== Generate observable output ========== */
    // Create a hash from various addresses and sizes
    unsigned long hash = 0;
    
    // Mix in addresses
    hash ^= (unsigned long)&global_int;
    hash ^= (unsigned long)&s1;
    hash ^= (unsigned long)&su;
    hash ^= (unsigned long)cb1;
    hash ^= (unsigned long)int_ptr;
    
    // Mix in sizes
    hash ^= sizeof(SimpleStruct);
    hash ^= sizeof(SimpleUnion);
    hash ^= sizeof(global_2d_array);
    hash ^= sizeof(incomplete_ptr_t);
    
    #ifdef __cplusplus
    hash ^= sizeof(DerivedClass);
    hash ^= sizeof(TemplateClass<int>);
    #endif
    
    // Mix in the result
    hash ^= (unsigned long)result;
    
    // Print something to ensure execution
    #ifdef __cplusplus
    std::cout << "Hash: " << hash << "\n";
    #else
    printf("Hash: %lu\n", hash);
    #endif
    
    return (int)(hash % 256);
}
