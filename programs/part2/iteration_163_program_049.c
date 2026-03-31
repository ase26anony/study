#ifdef __cplusplus
#include <iostream>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <type_traits>
#endif

/* ========== TYPE_UNDEFINED ========== */
struct undefined_struct;  // Forward declaration, never defined
typedef undefined_struct* undefined_ptr_t;

/* ========== TYPE_SCALAR ========== */
int global_int = 42;
unsigned int global_uint = 100U;
char global_char = 'A';
signed char global_schar = -1;
unsigned char global_uchar = 255;
short global_short = -100;
unsigned short global_ushort = 200;
long global_long = 1000L;
unsigned long global_ulong = 2000UL;
long long global_llong = 5000LL;
unsigned long long global_ullong = 10000ULL;
float global_float = 3.14f;
double global_double = 2.71828;
long double global_ldouble = 1.41421356L;
_Bool global_bool = 1;

/* ========== TYPE_STRING ========== */
const char* global_cstring = "Hello, World!";
const wchar_t* global_wstring = L"Wide String";
char global_char_array[50] = "Character array";
const char global_const_array[] = "Constant array";

/* ========== TYPE_STRUCT ========== */
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
    int value : 8;
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    double c;
};

#ifdef __cplusplus
extern "C" {
#endif
struct FlexibleArrayStruct {
    int count;
    int data[];  // Flexible array member
};
#ifdef __cplusplus
}
#endif

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
    void public_method() { private_member++; }
};

class DerivedClass : public BaseClass {
public:
    DerivedClass() : extra_data(0) {}
    void virtual_func() override { protected_member = 1.0f; }
private:
    int extra_data;
};

class MultipleInheritanceBase1 {
public:
    virtual void base1_func() {}
};

class MultipleInheritanceBase2 {
public:
    virtual void base2_func() {}
};

class MultipleDerived : public MultipleInheritanceBase1, 
                       public MultipleInheritanceBase2 {
public:
    void base1_func() override {}
    void base2_func() override {}
};

template<typename T>
class TemplateClass {
    T data;
public:
    TemplateClass(const T& val) : data(val) {}
    T get_data() const { return data; }
};

class ClassWithCallback {
public:
    using CallbackType = void(*)(int, const char*);
    CallbackType callback;
    
    void set_callback(CallbackType cb) { callback = cb; }
};
#endif

/* ========== TYPE_UNION ========== */
union SimpleUnion {
    int as_int;
    float as_float;
    char as_char[4];
};

union ComplexUnion {
    struct {
        int type;
        void* data;
    } header;
    double numeric;
    char* string;
};

#ifdef __cplusplus
struct StructWithAnonymousUnion {
    int tag;
    union {
        int int_value;
        float float_value;
        double double_value;
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
const volatile void* cv_void_ptr = nullptr;

/* Function pointers */
typedef int (*FuncPtr)(int, char);
typedef void (*VoidFuncPtr)(void);
typedef const char* (*StringFuncPtr)(const char*);

/* Pointer to array */
int (*array_ptr)[10];

/* Pointer to member (C++) */
#ifdef __cplusplus
typedef int (BaseClass::*MemberFuncPtr)();
typedef float BaseClass::*MemberDataPtr;
#endif

/* ========== TYPE_ARRAY ========== */
int simple_array[10];
int multi_dim_array[3][4][5];
char* pointer_array[20];
SimpleStruct struct_array[5];
int incomplete_array[] = {1, 2, 3, 4, 5};

/* Variable Length Array (C only) */
#ifndef __cplusplus
void use_vla(int size) {
    int vla[size];
    for (int i = 0; i < size; i++) vla[i] = i;
}
#endif

/* Array as struct member */
struct ArrayStruct {
    int fixed_array[8];
    char* ptr_array[4];
};

/* ========== TYPE_CALLBACK ========== */
typedef int (*Comparator)(const void*, const void*);
typedef void (*EventHandler)(int event_id, void* user_data);

struct CallbackContainer {
    Comparator compare;
    EventHandler handler;
};

void callback_user(int (*func)(int), EventHandler eh) {
    if (func) func(42);
    if (eh) eh(1, nullptr);
}

#ifdef __cplusplus
template<typename Func>
void template_callback(Func f) {
    f(100);
}

void lambda_test() {
    auto lambda = [](int x) -> int { return x * 2; };
    template_callback(lambda);
    
    int capture = 10;
    auto capturing_lambda = [capture](int x) -> int { 
        return x + capture; 
    };
    template_callback(capturing_lambda);
}
#endif

/* ========== TYPE_LANG_STRUCT (C++ only) ========== */
#ifdef __cplusplus

/* Lambda expressions */
auto global_lambda = [](int x) { return x * x; };

/* Initializer list usage */
class InitializerListUser {
    std::initializer_list<int> init_list;
public:
    InitializerListUser(std::initializer_list<int> list) : init_list(list) {}
};

/* Structured bindings (C++17) */
struct Point { int x; int y; };
auto get_point() -> Point { return {10, 20}; }

/* Fold expressions (C++17) */
template<typename... Args>
auto sum_all(Args... args) {
    return (args + ...);
}

/* Coroutine handle (C++20) */
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

SimpleCoroutine test_coroutine() {
    co_return;
}
#endif

/* Complex template instantiation */
template<typename T, typename U, typename V>
class TripleTemplate {
    T first;
    U second;
    V third;
    
    template<typename X>
    auto process(X x) -> decltype(first + second + third + x) {
        return first + second + third + x;
    }
};

/* Variadic templates */
template<typename... Ts>
class TupleWrapper {};

#endif /* __cplusplus */

/* ========== Function Definitions ========== */
int compare_int(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

void event_handler(int event_id, void* user_data) {
    (void)event_id;
    (void)user_data;
}

int process_int(int x) {
    return x * 2;
}

#ifdef __cplusplus
void BaseClass::virtual_func() {
    // Pure virtual implementation
}
#endif

/* ========== Main Function ========== */
int main(int argc, char* argv[]) {
    unsigned long hash = 0;
    
    /* Use argc to prevent dead code elimination */
    int mode = (argc > 1) ? argv[1][0] : 'A';
    
    /* TYPE_UNDEFINED usage */
    undefined_ptr_t undefined_ptr = nullptr;
    hash ^= (unsigned long)undefined_ptr;
    
    /* TYPE_SCALAR usage */
    int local_int = 100;
    unsigned int local_uint = 200;
    float local_float = 3.14f;
    double local_double = 2.71828;
    _Bool local_bool = 0;
    
    hash ^= sizeof(local_int);
    hash ^= sizeof(local_uint);
    hash ^= sizeof(local_float);
    hash ^= sizeof(local_double);
    hash ^= sizeof(local_bool);
    
    /* TYPE_STRING usage */
    const char* local_string = "Local string";
    wchar_t wide_local[] = L"Local wide";
    hash ^= (unsigned long)local_string;
    hash ^= (unsigned long)wide_local;
    
    /* TYPE_STRUCT usage */
    SimpleStruct simple = {1, 2.0, "test"};
    NestedStruct nested = {{2, 3.0, "inner"}, 4};
    BitFieldStruct bitfield = {1, 3, 7, 42};
    PackedStruct packed = {'X', 123, 45.67};
    
    hash ^= sizeof(simple);
    hash ^= sizeof(nested);
    hash ^= sizeof(bitfield);
    hash ^= sizeof(packed);
    
    /* TYPE_UNION usage */
    SimpleUnion su;
    su.as_int = 0xDEADBEEF;
    ComplexUnion cu;
    cu.numeric = 3.14159;
    
#ifdef __cplusplus
    StructWithAnonymousUnion sau;
    sau.tag = 1;
    sau.int_value = 42;
#endif
    
    hash ^= su.as_int;
    hash ^= (unsigned long)&cu;
    
    /* TYPE_POINTER usage */
    int* local_ptr = &local_int;
    int** local_dbl_ptr = &local_ptr;
    FuncPtr func_ptr = nullptr;
    
    hash ^= (unsigned long)local_ptr;
    hash ^= (unsigned long)local_dbl_ptr;
    hash ^= (unsigned long)func_ptr;
    
    /* TYPE_ARRAY usage */
    int local_array[5] = {1, 2, 3, 4, 5};
    SimpleStruct local_struct_array[2] = {{1, 1.0, "a"}, {2, 2.0, "b"}};
    
    for (int i = 0; i < 5; i++) {
        hash ^= local_array[i];
    }
    
#ifndef __cplusplus
    /* VLA in C mode */
    if (argc > 0) {
        use_vla(argc);
    }
#endif
    
    /* TYPE_CALLBACK usage */
    CallbackContainer callbacks = {compare_int, event_handler};
    callback_user(process_int, event_handler);
    
#ifdef __cplusplus
    /* C++ specific callbacks */
    lambda_test();
    
    ClassWithCallback cpp_callback_obj;
    cpp_callback_obj.set_callback([](int, const char*) {});
#endif
    
    /* TYPE_USER_STRUCT usage (C++) */
#ifdef __cplusplus
    DerivedClass derived;
    MultipleDerived multiple;
    TemplateClass<int> tc_int(42);
    TemplateClass<double> tc_double(3.14);
    
    hash ^= sizeof(derived);
    hash ^= sizeof(multiple);
    hash ^= tc_int.get_data();
    hash ^= (unsigned long)tc_double.get_data();
    
    /* TYPE_LANG_STRUCT usage */
    InitializerListUser ilu{1, 2, 3, 4, 5};
    
    if (mode == 'B') {
        /* Structured bindings */
        auto [x, y] = get_point();
        hash ^= x + y;
        
        /* Fold expression */
        hash ^= sum_all(1, 2, 3, 4, 5);
        
        /* Lambda with different captures */
        int capture1 = 10, capture2 = 20;
        auto complex_lambda = [capture1, &capture2](auto x) {
            return x + capture1 + capture2;
        };
        hash ^= complex_lambda(5);
        
        /* Template instantiation */
        TripleTemplate<int, double, char> triple{1, 2.0, 'A'};
        TupleWrapper<int, float, double, char> tuple_wrapper;
    }
    
#if __cplusplus >= 202002L
    if (mode == 'C') {
        test_coroutine();
    }
#endif
#endif
    
    /* Final output to prevent optimization */
    std::cout << "Hash: " << hash << std::endl;
    
    return (int)(hash % 256);
}
