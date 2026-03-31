/* gengtype_test.cc - Comprehensive type coverage test */

#ifdef __cplusplus
#include <iostream>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <type_traits>
#endif

/* ========== TYPE_UNDEFINED ========== */
struct incomplete_struct;  /* Forward declaration - never defined */
typedef incomplete_struct* incomplete_ptr_t;

/* ========== TYPE_SCALAR ========== */
/* Global scalar variables */
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
/* Basic C structure */
struct SimpleStruct {
    int x;
    double y;
    char name[32];
};

/* Nested structure */
struct OuterStruct {
    int id;
    struct InnerStruct {
        float data;
        char tag;
    } inner;
    SimpleStruct simple;
};

/* Structure with bit-fields */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    signed int value : 8;
    unsigned int : 4;  /* Padding */
};

/* Packed structure */
struct PackedStruct {
    char a;
    int b;
    double c;
} __attribute__((packed));

/* Structure with flexible array member (C only) */
#ifndef __cplusplus
struct FlexArrayStruct {
    int count;
    double data[];  /* Flexible array member */
};
#endif

/* ========== TYPE_USER_STRUCT (C++ only) ========== */
#ifdef __cplusplus
/* Base class with different access specifiers */
class BaseClass {
public:
    BaseClass() : public_data(100) {}
    virtual ~BaseClass() {}
    
    virtual void virtual_func() { std::cout << "BaseClass::virtual_func\n"; }
    void public_method() {}
    
    int public_data;
    
protected:
    void protected_method() {}
    float protected_data;
    
private:
    void private_method() {}
    double private_data;
};

/* Derived class with single inheritance */
class DerivedClass : public BaseClass {
public:
    DerivedClass() : derived_data(200) {}
    void virtual_func() override { std::cout << "DerivedClass::virtual_func\n"; }
    
    int derived_data;
};

/* Multiple inheritance */
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

/* Template class */
template<typename T, typename U>
class TemplateClass {
public:
    TemplateClass(T t, U u) : t_val(t), u_val(u) {}
    
    T get_t() const { return t_val; }
    U get_u() const { return u_val; }
    
    template<typename V>
    V process(V v) { return v + static_cast<V>(t_val); }
    
private:
    T t_val;
    U u_val;
};

/* Class with static members */
class StaticMemberClass {
public:
    static int static_counter;
    static const double static_const;
    
    static void static_method() {}
};

int StaticMemberClass::static_counter = 0;
const double StaticMemberClass::static_const = 3.14159;

#endif  /* __cplusplus */

/* ========== TYPE_UNION ========== */
/* C-style union */
union DataUnion {
    int int_value;
    float float_value;
    double double_value;
    char* string_value;
    void* ptr_value;
};

/* Union within structure */
struct UnionContainer {
    int type;
    union {
        int int_data;
        float float_data;
        struct {
            char a;
            char b;
        } char_pair;
    } data;
};

/* Anonymous union (C++11) */
#ifdef __cplusplus
struct AnonymousUnionStruct {
    int tag;
    union {
        int x;
        double y;
        char* z;
    };  /* Anonymous union */
};
#endif

/* ========== TYPE_POINTER ========== */
/* Various pointer types */
int* int_ptr = &global_int;
const int* const_int_ptr = &global_int;
volatile int* volatile_int_ptr = &global_int;
const volatile int* const_volatile_int_ptr = &global_int;
int** double_int_ptr = &int_ptr;
int* restrict restrict_ptr = &global_int;

/* Pointer to array */
int (*array_ptr)[10];

/* Pointer to function */
typedef int (*func_ptr_t)(int, double);
typedef void (*void_func_ptr_t)(void);

/* Pointer to structure */
SimpleStruct* struct_ptr = 0;
incomplete_ptr_t incomplete_ptr = 0;  /* Pointer to undefined type */

#ifdef __cplusplus
/* Pointer to member function */
typedef void (BaseClass::*member_func_ptr_t)();
/* Pointer to member data */
typedef int BaseClass::*member_data_ptr_t;
#endif

/* ========== TYPE_ARRAY ========== */
/* Various array types */
int int_array[10];
float float_array[5][10];  /* 2D array */
double double_array[2][3][4];  /* 3D array */
char* pointer_array[20];  /* Array of pointers */

/* Array of structures */
SimpleStruct struct_array[5];

/* Array with initialization */
int initialized_array[] = {1, 2, 3, 4, 5};

/* String array */
const char* string_array[] = {"one", "two", "three", NULL};

/* Variable Length Array (C99) */
#ifndef __cplusplus
void use_vla(int n) {
    int vla[n];  /* Variable length array */
    for (int i = 0; i < n; i++) {
        vla[i] = i * i;
    }
}
#endif

/* ========== TYPE_CALLBACK ========== */
/* Function pointer types */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, void*);

/* Structure with function pointer */
struct CallbackContainer {
    const char* name;
    callback_t callback;
    void* user_data;
};

/* Function using callback */
void register_callback(callback_t cb, void* data) {
    if (cb) cb(42, data);
}

/* Actual callback function */
void my_callback(int value, void* data) {
    (void)value;
    (void)data;
}

#ifdef __cplusplus
/* C++ function objects and lambdas */
template<typename Func>
void template_callback(Func f) {
    f(100);
}

/* Function returning function pointer */
std::function<int(int)> get_function() {
    return [](int x) { return x * 2; };
}
#endif

/* ========== TYPE_LANG_STRUCT (C++ only) ========== */
#ifdef __cplusplus
/* Lambda expressions with different captures */
auto lambda_no_capture = []() { return 42; };
auto lambda_by_value = [global_int]() { return global_int + 10; };
auto lambda_by_ref = [&global_double]() { return global_double * 2.0; };
auto lambda_mutable = [counter = 0]() mutable { return ++counter; };

/* std::initializer_list usage */
auto init_list_func(std::initializer_list<int> list) {
    int sum = 0;
    for (auto x : list) sum += x;
    return sum;
}

/* Structured bindings (C++17) */
auto structured_binding_func() {
    struct Point { int x; int y; };
    Point p{10, 20};
    auto [x_coord, y_coord] = p;
    return x_coord + y_coord;
}

/* Fold expressions (C++17) */
template<typename... Args>
auto fold_sum(Args... args) {
    return (args + ...);
}

/* Coroutine-related types (C++20) */
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

/* Complex template instantiation */
template<typename... Ts>
struct VariadicTemplate {
    static constexpr size_t size = sizeof...(Ts);
};

using ComplexType = VariadicTemplate<int, double, char, BaseClass*>;

#endif  /* __cplusplus */

/* ========== MAIN FUNCTION ========== */
int main(int argc, char* argv[]) {
    /* Use argc to prevent dead code elimination */
    int mode = (argc > 1) ? argv[1][0] : 'A';
    
    /* TYPE_SCALAR - local variables */
    int local_int = 100;
    unsigned int local_uint = 200u;
    char local_char = 'Z';
    float local_float = 1.234f;
    double local_double = 5.678;
    _Bool local_bool = 0;
    
    /* TYPE_STRUCT - instances */
    struct SimpleStruct s1 = {10, 3.14, "Test"};
    struct OuterStruct s2 = {1, {2.5f, 'X'}, {20, 6.28, "Nested"}};
    struct BitFieldStruct s3 = {1, 5, 9, -50};
    struct PackedStruct s4 = {'A', 100, 3.14159};
    
    /* TYPE_UNION - usage */
    union DataUnion u1;
    u1.int_value = 42;
    
    struct UnionContainer uc;
    uc.type = 1;
    uc.data.int_data = 100;
    
    /* TYPE_POINTER - operations */
    int* local_ptr = &local_int;
    *local_ptr = 200;
    
    void* void_ptr = &s1;
    incomplete_ptr_t inc_ptr = 0;  /* Pointer to undefined type */
    
    /* TYPE_ARRAY - usage */
    for (int i = 0; i < 10; i++) {
        int_array[i] = i * i;
    }
    
    float_array[2][3] = 3.14f;
    struct_array[0] = s1;
    
#ifndef __cplusplus
    /* VLA in C mode */
    if (argc > 1) {
        use_vla(argc);
    }
#endif
    
    /* TYPE_CALLBACK - usage */
    struct CallbackContainer cc = {"test", my_callback, &local_int};
    register_callback(my_callback, &s1);
    
    /* Switch based on mode to ensure all code paths are potentially reachable */
    int result = 0;
    
    switch (mode) {
        case 'A':
            /* Use TYPE_UNDEFINED pointer */
            result = (int)(intptr_t)inc_ptr;
            break;
            
        case 'B':
            /* Use TYPE_STRING */
            result = global_string[0] + global_char_array[0];
            break;
            
        case 'C':
            /* Use TYPE_STRUCT */
            result = s1.x + s2.id + s3.flag1;
            break;
            
        case 'D':
            /* Use TYPE_UNION */
            result = u1.int_value + uc.data.int_data;
            break;
            
        case 'E':
            /* Use TYPE_POINTER */
            result = *int_ptr + *local_ptr;
            break;
            
        case 'F':
            /* Use TYPE_ARRAY */
            for (int i = 0; i < 10; i++) {
                result += int_array[i];
            }
            break;
            
        case 'G':
            /* Use TYPE_CALLBACK */
            cc.callback(cc.name[0], cc.user_data);
            result = 1;
            break;
            
#ifdef __cplusplus
        case 'H':
            /* Use TYPE_USER_STRUCT */
            {
                BaseClass* base = new DerivedClass();
                base->virtual_func();
                result = base->public_data;
                delete base;
                
                TemplateClass<int, double> tc(10, 3.14);
                result += tc.get_t();
                
                StaticMemberClass::static_counter++;
            }
            break;
            
        case 'I':
            /* Use TYPE_LANG_STRUCT */
            {
                result = lambda_no_capture();
                result += lambda_by_value();
                result += init_list_func({1, 2, 3, 4});
                result += structured_binding_func();
                result += fold_sum(1, 2, 3, 4, 5);
                
                template_callback([](int x) { (void)x; });
                
                auto func = get_function();
                result += func(10);
            }
            break;
#endif
            
        default:
            result = -1;
            break;
    }
    
    /* Print observable output */
    printf("Result: %d\n", result);
    printf("Sizeof summary: %zu %zu %zu %zu %zu %zu\n",
           sizeof(s1), sizeof(s2), sizeof(u1), sizeof(int_array),
           sizeof(int_ptr), sizeof(my_callback));
    
    /* Take addresses of everything to force symbol usage */
    (void)&global_int;
    (void)&global_string;
    (void)&s1;
    (void)&u1;
    (void)&int_ptr;
    (void)&int_array;
    (void)&my_callback;
    
#ifdef __cplusplus
    /* Force template instantiation */
    TemplateClass<int, float> tc_inst(1, 2.0f);
    (void)tc_inst;
    
    TemplateClass<char, double> tc_inst2('X', 3.14);
    (void)tc_inst2;
    
    /* Force variadic template instantiation */
    ComplexType ct_inst;
    (void)ct_inst;
#endif
    
    return result != 0 ? 0 : 1;
}

/* Additional global instances to ensure types are analyzed */
struct SimpleStruct global_struct_instance = {0, 0.0, ""};
union DataUnion global_union_instance;
int* global_ptr_array[5];
callback_t global_callback_array[3] = {my_callback, NULL, my_callback};

#ifdef __cplusplus
/* Global C++ instances */
BaseClass* global_base_ptr = nullptr;
TemplateClass<long, float> global_template_instance(100L, 2.5f);
#endif
