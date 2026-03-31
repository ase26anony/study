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

/* ========== TYPE_STRING ========== */
const char* global_string = "Hello, World!";
const char global_char_array[] = "Character Array";
const wchar_t* global_wstring = L"Wide String";
const wchar_t global_wchar_array[] = L"Wide Array";

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

struct BitfieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
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

/* ========== TYPE_USER_STRUCT (C++ only) ========== */
#ifdef __cplusplus
class BaseClass {
public:
    BaseClass() : public_data(10) {}
    virtual ~BaseClass() {}
    virtual void virtual_func() { std::cout << "BaseClass\n"; }
    int public_data;
protected:
    float protected_data;
private:
    double private_data;
};

class DerivedClass : public BaseClass {
public:
    DerivedClass() : extra_data(20) {}
    void virtual_func() override { std::cout << "DerivedClass\n"; }
    int extra_data;
};

class MultipleInheritance : public BaseClass, public SimpleStruct {
public:
    MultipleInheritance() : combined(30) {}
    int combined;
};

template<typename T>
class TemplateClass {
public:
    TemplateClass(T val) : data(val) {}
    T get_data() const { return data; }
private:
    T data;
};

class ClassWithCallback {
public:
    using CallbackType = void(*)(int);
    void set_callback(CallbackType cb) { callback = cb; }
    void execute(int x) { if(callback) callback(x); }
private:
    CallbackType callback;
};
#endif

/* ========== TYPE_UNION ========== */
union SimpleUnion {
    int as_int;
    float as_float;
    char as_char;
    void* as_ptr;
};

struct StructWithAnonymousUnion {
    int type;
    union {
        int int_value;
        float float_value;
        double double_value;
    };
};

union ComplexUnion {
    struct {
        int x;
        int y;
    } point;
    struct {
        float r;
        float g;
        float b;
        float a;
    } color;
    long long big_value;
};

/* ========== TYPE_POINTER ========== */
int* int_ptr = &global_int;
const int* const_int_ptr = &global_int;
volatile int* volatile_int_ptr = &global_int;
int* restrict restrict_int_ptr = &global_int;
int** double_int_ptr = &int_ptr;
const volatile int* cv_int_ptr = &global_int;

SimpleStruct* struct_ptr = nullptr;
SimpleUnion* union_ptr = nullptr;
incomplete_ptr_t incomplete_ptr = nullptr;

#ifdef __cplusplus
BaseClass* base_class_ptr = nullptr;
DerivedClass* derived_class_ptr = nullptr;
#endif

/* Function pointers */
typedef int (*FuncPtr)(int, int);
typedef void (*VoidFuncPtr)(void);
typedef const char* (*StringFuncPtr)(const char*);

/* Pointer to array */
typedef int (*ArrayPtr)[10];
typedef int (*MultiArrayPtr)[5][10];

/* ========== TYPE_ARRAY ========== */
int simple_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
const char* string_array[] = {"one", "two", "three", "four"};
double multi_array[2][3][4] = {0};
SimpleStruct struct_array[5];

/* Array of pointers */
int* pointer_array[8] = {nullptr};

/* Incomplete array in struct */
struct WithIncompleteArray {
    int count;
    int items[];  // Incomplete array
};

/* ========== TYPE_CALLBACK ========== */
int add(int a, int b) { return a + b; }
int multiply(int a, int b) { return a * b; }

FuncPtr func_ptr_array[2] = {add, multiply};

typedef struct {
    int id;
    FuncPtr operation;
} CallbackContainer;

void execute_callback(FuncPtr fp, int a, int b) {
    if (fp) {
        int result = fp(a, b);
        (void)result;  // Use result to avoid warning
    }
}

#ifdef __cplusplus
template<typename F>
void template_callback(F func, int x) {
    func(x);
}

auto lambda = [](int x) -> int { return x * 2; };
#endif

/* ========== TYPE_LANG_STRUCT (C++ only) ========== */
#ifdef __cplusplus
/* Lambda expressions with different captures */
auto lambda_with_capture = [global_int](int x) { return x + global_int; };
auto mutable_lambda = [value = 0]() mutable { return ++value; };

/* std::initializer_list usage */
class InitializerListUser {
public:
    InitializerListUser(std::initializer_list<int> list) {
        for (auto val : list) {
            sum += val;
        }
    }
    int sum = 0;
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

/* Template with complex type deduction */
template<typename T, typename U>
auto complex_template(T t, U u) -> decltype(t + u) {
    return t + u;
}

#endif

/* ========== MAIN FUNCTION ========== */
int main(int argc, char* argv[]) {
    /* Use argc to prevent dead code elimination */
    int mode = (argc > 1) ? argv[1][0] : 'A';
    
    /* TYPE_UNDEFINED usage */
    incomplete_ptr_t local_incomplete_ptr = incomplete_ptr;
    (void)local_incomplete_ptr;
    
    /* TYPE_SCALAR usage */
    int local_int = global_int + 1;
    unsigned int local_uint = global_uint * 2;
    float local_float = global_float * 3.0f;
    double local_double = global_double / 2.0;
    _Bool local_bool = global_bool;
    
    /* TYPE_STRING usage */
    const char* local_string = global_string;
    const wchar_t* local_wstring = global_wstring;
    
    /* TYPE_STRUCT usage */
    SimpleStruct s1 = {1, 2.0, 'A'};
    NestedStruct ns = {{2, 3.0, 'B'}, 4};
    BitfieldStruct bfs = {1, 3, 7, -50};
    PackedStruct ps = {'X', 100, 200.5};
    
    /* TYPE_UNION usage */
    SimpleUnion su;
    su.as_int = 42;
    StructWithAnonymousUnion sau;
    sau.type = 1;
    sau.int_value = 100;
    
    /* TYPE_POINTER usage */
    int* local_int_ptr = &local_int;
    SimpleStruct* local_struct_ptr = &s1;
    FuncPtr local_func_ptr = add;
    
    /* TYPE_ARRAY usage */
    int local_array[5] = {0};
    for (int i = 0; i < 5; i++) {
        local_array[i] = i * mode;
    }
    
    /* TYPE_CALLBACK usage */
    execute_callback(add, 10, 20);
    execute_callback(multiply, 5, 6);
    
    /* Variable length array (C99) */
    int vla_size = (argc > 2) ? atoi(argv[2]) : 10;
    int vla[vla_size];
    for (int i = 0; i < vla_size; i++) {
        vla[i] = i * mode;
    }
    
#ifdef __cplusplus
    /* TYPE_USER_STRUCT usage */
    BaseClass base;
    DerivedClass derived;
    TemplateClass<int> tc_int(100);
    TemplateClass<double> tc_double(3.14159);
    
    ClassWithCallback cwc;
    cwc.set_callback([](int x) { std::cout << "Callback: " << x << "\n"; });
    cwc.execute(42);
    
    /* TYPE_LANG_STRUCT usage */
    auto result1 = lambda_with_capture(10);
    auto result2 = mutable_lambda();
    
    InitializerListUser ilu{1, 2, 3, 4, 5};
    
    auto [x, y] = get_point();  // Structured binding
    
    auto fold_result = sum_all(1, 2, 3, 4, 5);
    
    /* Complex template instantiation */
    auto ct_result = complex_template(10, 3.14);
    
    /* Use template callbacks */
    template_callback(lambda, 5);
    template_callback([](int x) { return x * x; }, 4);
#endif
    
    /* Generate observable output based on all types */
    unsigned long hash = 0;
    
    /* Hash sizes of different types */
    hash ^= sizeof(incomplete_ptr_t);
    hash ^= sizeof(int) * 7;
    hash ^= sizeof(double) * 3;
    hash ^= sizeof(SimpleStruct);
    hash ^= sizeof(NestedStruct);
    hash ^= sizeof(SimpleUnion);
    hash ^= sizeof(FuncPtr);
    hash ^= sizeof(simple_array);
    
#ifdef __cplusplus
    hash ^= sizeof(BaseClass);
    hash ^= sizeof(DerivedClass);
    hash ^= sizeof(TemplateClass<int>);
#endif
    
    /* Hash addresses (shifted to avoid exposing actual addresses) */
    hash ^= ((unsigned long)&global_int >> 4);
    hash ^= ((unsigned long)&s1 >> 4);
    hash ^= ((unsigned long)&su >> 4);
    hash ^= ((unsigned long)add >> 4);
    
    /* Print hash to ensure execution */
    printf("Type coverage hash: %lu\n", hash);
    
    /* Use mode to select different code paths */
    switch (mode) {
        case 'A':
            return local_int + s1.x + su.as_int;
        case 'B':
            return local_uint + ns.outer + bfs.flag3;
        case 'C':
            return (int)local_float + ps.b + local_array[2];
#ifdef __cplusplus
        case 'D':
            return base.public_data + derived.extra_data + tc_int.get_data();
#endif
        default:
            return hash % 256;
    }
}
