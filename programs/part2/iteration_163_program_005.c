#ifdef __cplusplus
#include <iostream>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <type_traits>
#endif

/* ========== TYPE_UNDEFINED ========== */
struct incomplete_struct;  // Forward declaration, never defined
typedef incomplete_struct* incomplete_ptr_t;

/* ========== TYPE_SCALAR ========== */
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
#ifdef __cplusplus
bool global_cppbool = true;
#endif

/* ========== TYPE_STRING ========== */
const char* global_cstring = "Hello, World!";
const wchar_t* global_wstring = L"Wide String";
char global_char_array[50] = "Character array";
const char global_const_array[] = "Constant array";

/* ========== TYPE_STRUCT ========== */
struct SimpleStruct {
    int x;
    double y;
    char z;
};

struct NestedStruct {
    SimpleStruct inner;
    int outer;
};

struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    int signed_field : 8;
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

/* ========== TYPE_USER_STRUCT (C++ only) ========== */
#ifdef __cplusplus
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

class DerivedClass : public BaseClass {
public:
    DerivedClass() : additional_member(0) {}
    void virtual_func() override {}
private:
    int additional_member;
};

class MultipleInheritance : public SimpleStruct, public BaseClass {
public:
    void virtual_func() override {}
};

template<typename T>
class TemplateClass {
    T value;
public:
    TemplateClass(T v) : value(v) {}
    T get_value() const { return value; }
};

template<typename K, typename V>
class Pair {
    K key;
    V value;
public:
    Pair(K k, V v) : key(k), value(v) {}
};
#endif

/* ========== TYPE_UNION ========== */
union SimpleUnion {
    int as_int;
    float as_float;
    char as_char;
    void* as_ptr;
};

union ComplexUnion {
    struct {
        int type;
        union {
            int int_val;
            float float_val;
            char* str_val;
        } data;
    } tagged;
    double raw_data[2];
};

#ifdef __cplusplus
struct StructWithAnonymousUnion {
    int tag;
    union {
        int int_member;
        float float_member;
        char* ptr_member;
    };
};
#endif

/* ========== TYPE_POINTER ========== */
int* int_ptr = &global_int;
const int* const_int_ptr = &global_int;
volatile int* volatile_int_ptr = &global_int;
int* restrict restrict_int_ptr = &global_int;
int** double_int_ptr = &int_ptr;
const char* const* const_string_ptr = &global_cstring;
void* void_ptr = nullptr;
incomplete_ptr_t incomplete_ptr = nullptr;

/* Function pointers */
typedef int (*func_ptr_t)(int, double);
typedef void (*void_func_ptr_t)(void);
typedef const char* (*string_func_ptr_t)(const char*);

#ifdef __cplusplus
typedef void (BaseClass::*member_func_ptr_t)();
#endif

/* ========== TYPE_ARRAY ========== */
int int_array[10] = {0,1,2,3,4,5,6,7,8,9};
int* ptr_array[5];
int multi_dim_array[2][3][4];
char string_array[3][20] = {"first", "second", "third"};
SimpleStruct struct_array[5];
SimpleUnion union_array[3];

/* Variable Length Array (C99) */
#ifndef __cplusplus
void use_vla(int size) {
    int vla[size];
    for (int i = 0; i < size; i++) vla[i] = i;
}
#endif

/* ========== TYPE_CALLBACK ========== */
int callback_function(int x, double y) {
    return (int)(x * y);
}

void void_callback(void) {
    /* Do nothing */
}

const char* string_callback(const char* str) {
    return str;
}

struct CallbackContainer {
    func_ptr_t func_ptr;
    void_func_ptr_t void_func;
};

typedef int (*comparator_t)(const void*, const void*);

#ifdef __cplusplus
template<typename Func>
void template_callback(Func f) {
    f(42);
}

std::function<int(int)> std_function_callback;
#endif

/* ========== TYPE_LANG_STRUCT (C++ only) ========== */
#ifdef __cplusplus
/* Lambda expressions */
auto lambda1 = []() { return 1; };
auto lambda2 = [global_int](int x) { return x + global_int; };
auto lambda3 = [&global_double]() { return global_double * 2.0; };
auto lambda4 = [=]() { return global_float; };
auto lambda5 = [&]() { return global_char; };

/* Initializer list */
std::initializer_list<int> init_list = {1, 2, 3, 4, 5};

/* Structured bindings (C++17) */
struct Point { int x; int y; };
auto get_point() -> Point { return {10, 20}; }

/* Fold expressions (C++17) */
template<typename... Args>
auto sum(Args... args) {
    return (args + ...);
}

/* Coroutine (C++20) */
#if __cplusplus >= 202002L
struct ReturnObject {
    struct promise_type {
        ReturnObject get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

ReturnObject coroutine_func() {
    co_await std::suspend_never{};
}
#endif

/* Template with complex type deduction */
template<typename... Ts>
struct VariadicTemplate {
    std::tuple<Ts...> values;
};
#endif

/* ========== Function using all types ========== */
void use_all_types(int argc, char** argv) {
    /* Use scalars */
    int local_int = argc;
    unsigned int local_uint = (unsigned int)argc;
    char local_char = (char)argc;
    float local_float = (float)argc;
    double local_double = (double)argc;
    
    /* Use strings */
    const char* local_string = argc > 0 ? argv[0] : "default";
    
    /* Use structs */
    SimpleStruct s1 = {1, 2.0, 'A'};
    NestedStruct ns = {{2, 3.0, 'B'}, 4};
    BitFieldStruct bfs = {1, 2, 3, -1};
    PackedStruct ps = {'X', 42, 3.14};
    
    /* Use unions */
    SimpleUnion su;
    su.as_int = 42;
    ComplexUnion cu;
    cu.tagged.type = 1;
    cu.tagged.data.int_val = 100;
    
#ifdef __cplusplus
    StructWithAnonymousUnion sau;
    sau.tag = 0;
    sau.int_member = 50;
#endif
    
    /* Use pointers */
    int* local_ptr = &local_int;
    const int* local_const_ptr = &local_int;
    void* local_void_ptr = local_ptr;
    
    /* Use arrays */
    int local_array[5] = {1, 2, 3, 4, 5};
    SimpleStruct local_struct_array[2] = {{1, 2.0, 'A'}, {2, 3.0, 'B'}};
    
#ifndef __cplusplus
    /* Use VLA in C mode */
    if (argc > 0) {
        use_vla(argc);
    }
#endif
    
    /* Use callbacks */
    func_ptr_t local_func_ptr = callback_function;
    int result = local_func_ptr(10, 2.5);
    
    CallbackContainer cc = {callback_function, void_callback};
    cc.func_ptr(5, 1.5);
    cc.void_func();
    
#ifdef __cplusplus
    /* Use C++ specific types */
    DerivedClass dc;
    BaseClass* bc = &dc;
    bc->virtual_func();
    
    TemplateClass<int> tc_int(42);
    TemplateClass<double> tc_double(3.14);
    Pair<int, const char*> pair(1, "one");
    
    /* Use lambdas */
    auto local_lambda = [local_int](int x) { return x + local_int; };
    template_callback(local_lambda);
    
    /* Use structured bindings */
    auto [x, y] = get_point();
    
    /* Use fold expression */
    int fold_result = sum(1, 2, 3, 4, 5);
    
    /* Use variadic template */
    VariadicTemplate<int, double, const char*> vt{std::make_tuple(1, 2.0, "three")};
#endif
    
    /* Prevent dead code elimination */
    volatile int anti_opt = 0;
    anti_opt += (int)&s1;
    anti_opt += (int)&su;
    anti_opt += (int)local_ptr;
    anti_opt += result;
    
#ifdef __cplusplus
    anti_opt += tc_int.get_value();
    anti_opt += fold_result;
    anti_opt += x + y;
#endif
    
    /* Print something observable */
#ifdef __cplusplus
    std::cout << "Type coverage test: " << anti_opt << std::endl;
#else
    printf("Type coverage test: %d\n", anti_opt);
#endif
}

/* ========== Main function ========== */
int main(int argc, char** argv) {
    /* Force instantiation of all global variables */
    volatile int dummy = 0;
    dummy += global_int;
    dummy += (int)global_cstring[0];
    dummy += (int)&global_cstring;
    
    /* Use incomplete type pointer */
    incomplete_ptr_t local_incomplete_ptr = incomplete_ptr;
    dummy += (int)local_incomplete_ptr;
    
    /* Use all arrays */
    for (int i = 0; i < 10; i++) dummy += int_array[i];
    for (int i = 0; i < 5; i++) dummy += (int)&struct_array[i];
    
    /* Call function that uses all types */
    use_all_types(argc, argv);
    
#ifdef __cplusplus
    /* Instantiate template classes */
    TemplateClass<long> tc_long(100L);
    TemplateClass<SimpleStruct> tc_struct{{1, 2.0, 'A'}};
    
    /* Use std::function callback */
    std_function_callback = [](int x) { return x * 2; };
    dummy += std_function_callback(21);
    
#if __cplusplus >= 202002L
    /* Use coroutine if C++20 */
    coroutine_func();
#endif
#endif
    
    return dummy > 0 ? 0 : 1;
}
