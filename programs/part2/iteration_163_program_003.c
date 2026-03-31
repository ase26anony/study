#ifdef __cplusplus
#include <iostream>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <type_traits>
#include <vector>
#endif

/* ========== TYPE_UNDEFINED ========== */
struct incomplete_struct;  // Forward declaration, never defined
typedef incomplete_struct* incomplete_ptr_t;

/* ========== TYPE_SCALAR ========== */
// Global scalar variables
int global_int = 42;
unsigned int global_uint = 100u;
char global_char = 'A';
signed char global_schar = -10;
unsigned char global_uchar = 200;
short global_short = -1000;
unsigned short global_ushort = 5000;
long global_long = 100000L;
unsigned long global_ulong = 200000UL;
long long global_llong = 10000000000LL;
unsigned long long global_ullong = 20000000000ULL;
float global_float = 3.14159f;
double global_double = 2.718281828459045;
long double global_ldouble = 1.618033988749895L;
#ifdef __cplusplus
bool global_bool = true;
#endif

/* ========== TYPE_STRING ========== */
const char* global_string = "Hello, World!";
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
    int outer_value;
};

struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    int value : 10;
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
    int private_data;
protected:
    float protected_data;
public:
    BaseClass() : private_data(0), protected_data(0.0f) {}
    virtual ~BaseClass() {}
    virtual void virtual_func() = 0;
    void public_method() { private_data++; }
};

class DerivedClass : public BaseClass {
private:
    double derived_data;
public:
    DerivedClass() : derived_data(0.0) {}
    virtual void virtual_func() override { derived_data = 1.0; }
    void derived_method() { protected_data = 2.0f; }
};

class MultipleInheritanceBase1 {
public:
    virtual void base1_method() {}
};

class MultipleInheritanceBase2 {
public:
    virtual void base2_method() {}
};

class MultipleInheritedClass : public MultipleInheritanceBase1, 
                               public MultipleInheritanceBase2 {
public:
    void both_methods() { base1_method(); base2_method(); }
};

template<typename T>
class TemplateClass {
    T data;
public:
    TemplateClass(T val) : data(val) {}
    T get_data() const { return data; }
    void set_data(T val) { data = val; }
};

// Explicit template instantiations
template class TemplateClass<int>;
template class TemplateClass<double>;
template class TemplateClass<SimpleStruct>;

class ComplexClass {
    TemplateClass<int> int_template;
    TemplateClass<double> double_template;
public:
    ComplexClass() : int_template(42), double_template(3.14) {}
};
#endif

/* ========== TYPE_UNION ========== */
union SimpleUnion {
    int as_int;
    float as_float;
    char as_char[4];
};

union ComplexUnion {
    long long as_llong;
    double as_double;
    SimpleStruct as_struct;
    void* as_pointer;
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

/* ========== TYPE_POINTER ========== */
int* int_ptr = &global_int;
const int* const_int_ptr = &global_int;
volatile int* volatile_int_ptr = &global_int;
int* restrict restrict_int_ptr = &global_int;
const volatile int* const_volatile_int_ptr = &global_int;

double** double_ptr_ptr = &int_ptr;  // Will be reassigned
SimpleStruct* struct_ptr = nullptr;
void* void_ptr = nullptr;
incomplete_ptr_t incomplete_ptr = nullptr;

// Function pointers
typedef int (*func_ptr_t)(int, double);
typedef void (*void_func_ptr_t)(void);
typedef const char* (*string_func_ptr_t)(const char*);

#ifdef __cplusplus
typedef void (BaseClass::*member_func_ptr_t)();
#endif

/* ========== TYPE_ARRAY ========== */
int int_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
double double_array[5][3];
char char_3d_array[2][3][4];
SimpleStruct struct_array[5];
int* pointer_array[20];

// Array of function pointers
func_ptr_t func_ptr_array[5];

// Incomplete array in struct (already defined in FlexibleArrayStruct)

/* ========== TYPE_CALLBACK ========== */
// Function pointer typedefs
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, void*);

// Structure with function pointer member
struct CallbackContainer {
    callback_t callback;
    void* user_data;
};

// Functions to be used as callbacks
int compare_ints(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

void simple_callback(int value, void* data) {
    *(int*)data = value * 2;
}

#ifdef __cplusplus
// C++ specific callbacks
template<typename Func>
void template_callback(Func f, int value) {
    f(value);
}

auto lambda_callback = [](int x) -> int {
    return x * x;
};

std::function<int(int)> std_function_callback = lambda_callback;
#endif

/* ========== TYPE_LANG_STRUCT (C++ only) ========== */
#ifdef __cplusplus
// Lambda expressions with different captures
auto lambda_with_capture = [global_int](int x) -> int {
    return x + global_int;
};

auto lambda_with_ref_capture = [&global_double](double x) -> double {
    return x * global_double;
};

auto mutable_lambda = [counter = 0](int x) mutable -> int {
    counter += x;
    return counter;
};

// std::initializer_list usage
template<typename T>
T sum_initializer_list(std::initializer_list<T> list) {
    T total = T();
    for (const auto& item : list) {
        total += item;
    }
    return total;
}

// Structured bindings (C++17)
struct Point3D {
    float x, y, z;
};

auto get_point() -> Point3D {
    return {1.0f, 2.0f, 3.0f};
}

// Fold expressions (C++17)
template<typename... Args>
auto sum_fold(Args... args) {
    return (args + ...);
}

// Coroutine-related types (C++20)
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

// Complex template with language features
template<typename... Ts>
class VariadicTemplate {
    std::tuple<Ts...> data;
    
public:
    VariadicTemplate(Ts... args) : data(args...) {}
    
    auto process() {
        return std::apply([](auto&&... args) {
            return sum_fold(args...);
        }, data);
    }
};
#endif

/* ========== Function Definitions ========== */
// Function using various types
void process_types(int argc, char** argv) {
    // Use incomplete pointer (TYPE_UNDEFINED)
    incomplete_ptr_t local_incomplete_ptr = incomplete_ptr;
    
    // Local scalar variables (TYPE_SCALAR)
    int local_int = argc;
    unsigned int local_uint = static_cast<unsigned int>(argc);
    char local_char = argv[0][0];
    float local_float = 3.14f * argc;
    double local_double = 2.71828 * argc;
    
    // String operations (TYPE_STRING)
    const char* local_string = argc > 1 ? argv[1] : "default";
    wchar_t local_wchar = L'X';
    
    // Struct operations (TYPE_STRUCT)
    SimpleStruct local_struct = {10, 20.5, "local"};
    NestedStruct nested = {{5, 10.2, "nested"}, 100};
    BitFieldStruct bitfield = {1, 5, 9, 255};
    
    // Union operations (TYPE_UNION)
    SimpleUnion su;
    su.as_int = 0x41424344;
    
    ComplexUnion cu;
    cu.as_double = 3.14159;
    
    #ifdef __cplusplus
    StructWithAnonymousUnion sau;
    sau.type = 1;
    sau.int_value = 42;
    #endif
    
    // Pointer operations (TYPE_POINTER)
    int* local_int_ptr = &local_int;
    double_ptr_ptr = (double**)&local_int_ptr;  // Type punning
    
    // Array operations (TYPE_ARRAY)
    int local_array[argc > 2 ? atoi(argv[2]) : 5];  // VLA in C mode
    for (int i = 0; i < sizeof(local_array)/sizeof(local_array[0]); i++) {
        local_array[i] = i * argc;
    }
    
    // Callback operations (TYPE_CALLBACK)
    func_ptr_array[0] = (func_ptr_t)compare_ints;
    CallbackContainer container = {simple_callback, &local_int};
    
    #ifdef __cplusplus
    // C++ specific operations
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    base_ptr->virtual_func();
    
    TemplateClass<int> int_template(100);
    TemplateClass<double> double_template(2.71828);
    
    // Use lambdas and std::function
    template_callback(lambda_callback, 10);
    template_callback(std_function_callback, 20);
    
    // Use initializer_list
    auto list_sum = sum_initializer_list({1, 2, 3, 4, 5});
    
    // Use structured bindings
    auto [x, y, z] = get_point();
    
    // Use fold expressions
    auto fold_result = sum_fold(1, 2, 3, 4, 5);
    
    // Use variadic template
    VariadicTemplate<int, double, float> variadic(1, 2.0, 3.0f);
    auto variadic_result = variadic.process();
    
    // Complex type combinations
    member_func_ptr_t member_ptr = &BaseClass::public_method;
    (derived.*member_ptr)();
    #endif
    
    // Force usage of all types to prevent optimization
    volatile int anti_opt = 0;
    anti_opt += (int)local_incomplete_ptr;
    anti_opt += local_int + local_uint + local_char;
    anti_opt += (int)local_string[0] + (int)local_wchar;
    anti_opt += local_struct.x + (int)nested.outer_value;
    anti_opt += su.as_char[0] + (int)cu.as_double;
    anti_opt += *local_int_ptr;
    anti_opt += local_array[0];
    anti_opt += (int)func_ptr_array[0];
    
    #ifdef __cplusplus
    anti_opt += (int)list_sum + (int)fold_result + (int)variadic_result;
    #endif
}

/* ========== Main Function ========== */
int main(int argc, char** argv) {
    // Initialize arrays
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 3; j++) {
            double_array[i][j] = i * 10.0 + j;
        }
    }
    
    for (int i = 0; i < 5; i++) {
        struct_array[i].x = i * 10;
        struct_array[i].y = i * 20.0;
        snprintf(struct_array[i].name, 20, "Struct%d", i);
    }
    
    // Process based on command line arguments
    process_types(argc, argv);
    
    // Use all global variables
    volatile int result = 0;
    result += global_int + global_uint + global_char;
    result += (int)global_string[0] + (int)global_wstring[0];
    result += int_array[0] + (int)double_array[0][0];
    result += (int)struct_ptr + (int)void_ptr;
    
    // Call functions through pointers
    if (argc > 1) {
        int data = 10;
        simple_callback(5, &data);
        result += data;
        
        #ifdef __cplusplus
        int squared = lambda_callback(5);
        result += squared;
        #endif
    }
    
    // Output something observable
    #ifdef __cplusplus
    std::cout << "Result hash: " << result << std::endl;
    #else
    printf("Result hash: %d\n", result);
    #endif
    
    return 0;
}
