#ifdef __cplusplus
#include <iostream>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <type_traits>
#endif

/* ==================== TYPE_UNDEFINED ==================== */
struct undefined_struct;  // Forward declaration, never defined
typedef undefined_struct* undefined_ptr_t;

/* ==================== TYPE_SCALAR ==================== */
int global_int = 42;
unsigned int global_uint = 100u;
char global_char = 'A';
signed char global_schar = -1;
unsigned char global_uchar = 255;
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
#ifdef __cplusplus
bool global_cppbool = true;
#endif

/* ==================== TYPE_STRING ==================== */
const char* global_cstring = "Hello, World!";
const wchar_t* global_wstring = L"Wide String";
char global_char_array[50] = "Character array";
const char global_const_array[] = "Constant array";
wchar_t global_wchar_array[] = L"Wide array";

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
    int value : 8;
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

/* ==================== TYPE_USER_STRUCT (C++ only) ==================== */
#ifdef __cplusplus
class BaseClass {
public:
    BaseClass() : base_value(0) {}
    virtual ~BaseClass() {}
    virtual void virtual_func() { std::cout << "Base\n"; }
    void non_virtual() {}
private:
    int base_value;
protected:
    double protected_value;
};

class DerivedClass : public BaseClass {
public:
    DerivedClass() : derived_value(0) {}
    void virtual_func() override { std::cout << "Derived\n"; }
    void member_func(int x) { derived_value = x; }
private:
    int derived_value;
};

class MultipleInheritance : public BaseClass, public DerivedClass {
public:
    MultipleInheritance() : multi_value(0) {}
    void virtual_func() override { std::cout << "Multiple\n"; }
private:
    int multi_value;
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

class ClassWithAnonymousUnion {
public:
    union {
        int as_int;
        float as_float;
        char as_char[4];
    };
    int regular_member;
};
#endif

/* ==================== TYPE_UNION ==================== */
union SimpleUnion {
    int as_int;
    float as_float;
    double as_double;
    char as_char[8];
};

union ComplexUnion {
    int i;
    float f;
    SimpleStruct s;
    void* p;
};

#ifdef __cplusplus
struct StructWithAnonymousUnion {
    int type;
    union {
        int int_value;
        float float_value;
        char* string_value;
    };
};
#endif

/* ==================== TYPE_POINTER ==================== */
int* int_ptr = &global_int;
const int* const_int_ptr = &global_int;
volatile int* volatile_int_ptr = &global_int;
int* restrict restrict_int_ptr = &global_int;
int** double_int_ptr = &int_ptr;
const char* const* const_string_ptr = &global_cstring;
SimpleStruct* struct_ptr = nullptr;
undefined_ptr_t undefined_ptr = nullptr;
void* void_ptr = nullptr;

/* ==================== TYPE_ARRAY ==================== */
int int_array[10] = {0,1,2,3,4,5,6,7,8,9};
double double_array[5][3] = {{1,2,3},{4,5,6},{7,8,9},{10,11,12},{13,14,15}};
char* string_array[3] = {"first", "second", "third"};
SimpleStruct struct_array[4];
int (*array_of_pointers[5])[10];
int incomplete_array[] = {1, 2, 3, 4, 5};

/* ==================== TYPE_CALLBACK ==================== */
typedef int (*SimpleCallback)(int, int);
typedef void (*VoidCallback)(void);
typedef const char* (*StringCallback)(const char*);

struct CallbackContainer {
    SimpleCallback math_callback;
    VoidCallback void_callback;
    StringCallback string_callback;
};

int add(int a, int b) { return a + b; }
int multiply(int a, int b) { return a * b; }
void do_nothing() {}
const char* echo(const char* str) { return str; }

#ifdef __cplusplus
template<typename Func>
void call_with_lambda(Func f) {
    f(42);
}

std::function<int(int, int)> std_function = [](int a, int b) { return a + b; };
#endif

/* ==================== TYPE_LANG_STRUCT (C++ only) ==================== */
#ifdef __cplusplus
template<typename... Args>
auto sum(Args... args) {
    return (args + ...);  // Fold expression
}

struct CoroutineType {
    struct promise_type {
        CoroutineType get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

CoroutineType dummy_coroutine() {
    co_return;
}

class ComplexCppClass {
public:
    ComplexCppClass(std::initializer_list<int> init) {
        for (auto x : init) {
            values.push_back(x);
        }
    }
    
    auto get_tuple() {
        return std::make_tuple(1, 2.0, "three");  // Structured binding candidate
    }
    
private:
    std::vector<int> values;
};

template<typename T>
class TypeDependentClass {
    typename std::conditional<std::is_integral<T>::value, int, double>::type value;
};
#endif

/* ==================== Main execution ==================== */
int main(int argc, char* argv[]) {
    /* Force usage of TYPE_UNDEFINED */
    undefined_ptr_t local_undefined_ptr = nullptr;
    
    /* Force usage of TYPE_SCALAR */
    int local_int = argc;
    unsigned int local_uint = static_cast<unsigned int>(argc);
    float local_float = argc * 0.1f;
    double local_double = argc * 0.01;
    _Bool local_bool = argc > 1;
    
    /* Force usage of TYPE_STRING */
    const char* local_string = argv[0];
    char local_char_array[100];
    
    /* Force usage of TYPE_STRUCT */
    SimpleStruct s1 = {10, 3.14, "test"};
    NestedStruct ns = {{5, 2.718, "inner"}, 100};
    BitFieldStruct bfs = {1, 7, 15, -128};
    PackedStruct ps = {'X', 42, 3.14159};
    
    /* Force usage of TYPE_UNION */
    SimpleUnion su;
    su.as_int = 42;
    ComplexUnion cu;
    cu.i = 100;
    
    /* Force usage of TYPE_POINTER */
    int* local_int_ptr = &local_int;
    SimpleStruct* local_struct_ptr = &s1;
    void** local_void_double_ptr = &void_ptr;
    
    /* Force usage of TYPE_ARRAY */
    int local_array[argc > 10 ? 10 : 5];  // Variable length array (C mode)
    for (int i = 0; i < (argc > 10 ? 10 : 5); i++) {
        local_array[i] = i * argc;
    }
    
    int multi_dim[2][3] = {{1,2,3},{4,5,6}};
    
    /* Force usage of TYPE_CALLBACK */
    SimpleCallback callback = (argc % 2 == 0) ? add : multiply;
    int result = callback(10, 20);
    
    CallbackContainer callbacks = {add, do_nothing, echo};
    callbacks.math_callback(5, 6);
    
    #ifdef __cplusplus
    /* Force usage of TYPE_USER_STRUCT */
    BaseClass* base = new DerivedClass();
    base->virtual_func();
    
    TemplateClass<int> tc_int(42);
    TemplateClass<double> tc_double(3.14);
    TemplateClass<SimpleStruct> tc_struct(s1);
    
    ClassWithAnonymousUnion cwau;
    cwau.as_int = 0xDEADBEEF;
    
    /* Force usage of TYPE_LANG_STRUCT */
    auto lambda = [&](int x) { return x + local_int; };
    call_with_lambda(lambda);
    
    auto fold_result = sum(1, 2, 3, 4, 5);
    
    ComplexCppClass ccc{1, 2, 3, 4, 5};
    auto [a, b, c] = ccc.get_tuple();  // Structured binding
    
    TypeDependentClass<int> tdc_int;
    TypeDependentClass<float> tdc_float;
    
    // Force coroutine instantiation
    if (argc > 5) {
        dummy_coroutine();
    }
    
    delete base;
    #endif
    
    /* Prevent dead code elimination */
    unsigned long hash = 0;
    hash ^= (unsigned long)global_int;
    hash ^= (unsigned long)global_float;
    hash ^= (unsigned long)&s1;
    hash ^= (unsigned long)callback;
    hash ^= (unsigned long)local_array[0];
    #ifdef __cplusplus
    hash ^= (unsigned long)&tc_int;
    hash ^= (unsigned long)fold_result;
    #endif
    
    /* Print something observable */
    #ifdef __cplusplus
    std::cout << "Hash: " << hash << "\n";
    std::cout << "Result: " << result << "\n";
    #else
    printf("Hash: %lu\n", hash);
    printf("Result: %d\n", result);
    #endif
    
    return (int)(hash % 256);
}
