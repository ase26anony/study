#ifdef __cplusplus
#include <iostream>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <functional>
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
const char* global_string = "Hello, World!";
const wchar_t* global_wstring = L"Wide String";
char global_char_array[50] = "Character array";
const char global_const_array[] = "Constant array";

/* ========== TYPE_STRUCT ========== */
// Basic structure
struct SimpleStruct {
    int x;
    double y;
    char name[20];
};

// Nested structure
struct OuterStruct {
    struct InnerStruct {
        int a;
        float b;
    } inner;
    int outer_value;
};

// Structure with bit-fields
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    int value : 8;
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
    int data[];
};
#endif

/* ========== TYPE_USER_STRUCT (C++ only) ========== */
#ifdef __cplusplus
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
public:
    DerivedClass() : BaseClass(), derived_member(0) {}
    ~DerivedClass() override {}
    void virtual_func() override {}
private:
    int derived_member;
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

class MultipleInheritanceClass : public Interface1, public Interface2 {
public:
    void method1() override {}
    void method2() override {}
};

// Template class
template<typename T, typename U>
class TemplateClass {
    T t_value;
    U u_value;
public:
    TemplateClass(T t, U u) : t_value(t), u_value(u) {}
    T get_t() const { return t_value; }
    U get_u() const { return u_value; }
};

// Instantiate template classes
TemplateClass<int, double> template_instance(10, 3.14);
TemplateClass<char*, float*> template_instance2(nullptr, nullptr);

#endif

/* ========== TYPE_UNION ========== */
// C-style union
union DataUnion {
    int int_value;
    float float_value;
    double double_value;
    char* string_value;
};

// Union within structure
struct UnionContainer {
    int type;
    union {
        int int_data;
        float float_data;
        struct {
            short x, y;
        } point_data;
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

/* ========== TYPE_POINTER ========== */
// Various pointer types
int* int_ptr = &global_int;
const int* const_int_ptr = &global_int;
volatile int* volatile_int_ptr = &global_int;
int* restrict restrict_int_ptr = &global_int;
int** double_int_ptr = &int_ptr;
const char* const* const_string_ptr_ptr = &global_string;

// Pointer to structure
struct SimpleStruct* struct_ptr = 0;
struct SimpleStruct** struct_ptr_ptr = &struct_ptr;

// Pointer to function
typedef int (*func_ptr_t)(int, double);
typedef void (*void_func_ptr_t)(void);

// Pointer to array
int (*array_ptr)[10];

#ifdef __cplusplus
// Pointer to member function
typedef void (BaseClass::*member_func_ptr_t)();
// Pointer to data member
typedef int BaseClass::*data_member_ptr_t;
#endif

/* ========== TYPE_ARRAY ========== */
// Various array types
int int_array[10];
float float_array[5][10];
double double_3d_array[2][3][4];
char* string_array[5] = {"one", "two", "three", "four", "five"};
struct SimpleStruct struct_array[3];

// Array of pointers
int* pointer_array[8];

// Incomplete array in structure (C only)
#ifndef __cplusplus
struct IncompleteArrayStruct {
    int count;
    int elements[];
};
#endif

// Variable length array (C99)
#ifndef __cplusplus
void use_vla(int size) {
    int vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = i;
    }
}
#endif

/* ========== TYPE_CALLBACK ========== */
// Function pointer types
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, void*);

// Structure with function pointer
struct CallbackContainer {
    callback_t callback;
    void* user_data;
};

// Function using callback
void register_callback(callback_t cb, void* data) {
    CallbackContainer container = {cb, data};
}

// Actual callback function
void my_callback(int value, void* data) {
    *(int*)data = value * 2;
}

#ifdef __cplusplus
// C++ function objects and lambdas
template<typename Func>
void process_with_callback(Func f) {
    f(42);
}

// Lambda with different captures
auto lambda_with_capture = [global_int](int x) { return x + global_int; };
#endif

/* ========== TYPE_LANG_STRUCT (C++ only) ========== */
#ifdef __cplusplus
// Lambda expressions
auto simple_lambda = [](int x) { return x * 2; };
auto capturing_lambda = [global_int, &global_double](int x) {
    global_double = x + global_int;
    return global_double;
};

// std::initializer_list
void use_initializer_list(std::initializer_list<int> list) {
    for (auto x : list) {
        // Process each element
    }
}

// Structured bindings (C++17)
struct Point { int x; int y; };
void use_structured_binding() {
    Point p{10, 20};
    auto [x_coord, y_coord] = p;
}

// Fold expressions (C++17)
template<typename... Args>
auto sum_all(Args... args) {
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

SimpleCoroutine simple_coro() {
    co_return;
}
#endif

// Complex template instantiation with language features
template<typename T>
class LanguageFeatureContainer {
    T value;
public:
    LanguageFeatureContainer(T v) : value(v) {}
    
    auto process_with_lambda() {
        return [this](int x) { return x + static_cast<int>(value); };
    }
    
    void use_initializer() {
        std::initializer_list<T> list = {value, value * 2, value * 3};
    }
};

// Instantiate with different types
LanguageFeatureContainer<int> lang_feature_int(10);
LanguageFeatureContainer<double> lang_feature_double(3.14);

#endif

/* ========== Helper functions ========== */
int compare_ints(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

void process_pointer(incomplete_ptr_t ptr) {
    // Force analysis of incomplete pointer type
    (void)ptr;
}

#ifdef __cplusplus
void use_all_cpp_features() {
    // Use template instances
    auto t1 = template_instance.get_t();
    auto t2 = template_instance.get_u();
    
    // Use lambdas
    process_with_callback([](int x) { std::cout << x << std::endl; });
    
    // Use structured binding
    use_structured_binding();
    
    // Use fold expression
    auto total = sum_all(1, 2, 3, 4, 5);
    
    // Use initializer list
    use_initializer_list({1, 2, 3, 4, 5});
    
    // Use language feature containers
    lang_feature_int.process_with_lambda();
    lang_feature_double.use_initializer();
    
    #if __cplusplus >= 202002L
    // Use coroutine if available
    simple_coro();
    #endif
}
#endif

/* ========== Main function ========== */
int main(int argc, char* argv[]) {
    // Use argc to prevent dead code elimination
    int mode = (argc > 1) ? argv[1][0] : 'A';
    
    /* TYPE_UNDEFINED */
    incomplete_ptr_t undefined_ptr = 0;
    process_pointer(undefined_ptr);
    
    /* TYPE_SCALAR */
    // Local scalar variables
    int local_int = 100;
    unsigned local_uint = 200u;
    char local_char = 'Z';
    float local_float = 1.234f;
    double local_double = 9.876;
    _Bool local_bool = 0;
    
    /* TYPE_STRING */
    const char* local_string = "Local string";
    wchar_t local_wchar = L'X';
    char local_char_buffer[100] = "Buffer";
    
    /* TYPE_STRUCT */
    struct SimpleStruct s1 = {1, 2.0, "test"};
    struct OuterStruct s2 = {{10, 20.5f}, 30};
    struct BitFieldStruct s3 = {1, 2, 3, 4};
    struct PackedStruct s4 = {'a', 123, 45.67};
    struct UnionContainer s5 = {0, {.int_data = 42}};
    
    #ifndef __cplusplus
    // C-specific struct usage
    struct FlexArrayStruct* flex = malloc(sizeof(struct FlexArrayStruct) + 10 * sizeof(int));
    if (flex) {
        flex->count = 10;
        free(flex);
    }
    #endif
    
    /* TYPE_USER_STRUCT (C++ only) */
    #ifdef __cplusplus
    DerivedClass derived_obj;
    MultipleInheritanceClass multi_obj;
    
    BaseClass* base_ptr = &derived_obj;
    base_ptr->virtual_func();
    
    use_all_cpp_features();
    #endif
    
    /* TYPE_UNION */
    union DataUnion u1;
    u1.int_value = 100;
    u1.float_value = 3.14f;  // Overwrites int_value
    
    #ifdef __cplusplus
    AnonymousUnionStruct aus = {0};
    aus.as_int = 42;
    aus.as_double = 3.14159;
    #endif
    
    /* TYPE_POINTER */
    // Use various pointers
    int* local_ptr = &local_int;
    *local_ptr = 999;
    
    // Function pointer usage
    func_ptr_t fptr = 0;
    void_func_ptr_t vfptr = 0;
    
    // Pointer arithmetic
    int* array_start = int_array;
    for (int i = 0; i < 10; i++) {
        array_start[i] = i * mode;
    }
    
    /* TYPE_ARRAY */
    // Initialize arrays
    for (int i = 0; i < 10; i++) {
        int_array[i] = i * i;
    }
    
    for (int i = 0; i < 3; i++) {
        struct_array[i].x = i;
        struct_array[i].y = i * 2.0;
    }
    
    #ifndef __cplusplus
    // Use VLA in C mode
    if (argc > 1) {
        use_vla(argc);
    }
    #endif
    
    /* TYPE_CALLBACK */
    // Use function pointers
    comparator_t comp = compare_ints;
    qsort(int_array, 10, sizeof(int), comp);
    
    // Register callback
    int callback_data = 0;
    register_callback(my_callback, &callback_data);
    
    #ifdef __cplusplus
    // C++ callback usage
    process_with_callback(lambda_with_capture);
    #endif
    
    /* TYPE_LANG_STRUCT (C++ only) */
    #ifdef __cplusplus
    // Force instantiation of language features
    auto result = simple_lambda(21);
    capturing_lambda(42);
    
    // Use complex template with language features
    auto lambda_result = lang_feature_int.process_with_lambda()(10);
    #endif
    
    // Generate observable output based on all type usages
    // This prevents optimization from removing the code
    unsigned long long hash = 0;
    
    // Hash scalar values
    hash ^= (unsigned long long)global_int;
    hash ^= (unsigned long long)global_uint << 8;
    hash ^= (unsigned long long)global_char << 16;
    hash ^= (unsigned long long)(global_float * 1000);
    
    // Hash addresses
    hash ^= (unsigned long long)(uintptr_t)&s1;
    hash ^= (unsigned long long)(uintptr_t)int_ptr;
    hash ^= (unsigned long long)(uintptr_t)global_string;
    
    // Hash array contents
    for (int i = 0; i < 10 && i < argc; i++) {
        hash ^= (unsigned long long)int_array[i] << (i * 4);
    }
    
    #ifdef __cplusplus
    std::cout << "Hash: " << hash << std::endl;
    #else
    printf("Hash: %llu\n", hash);
    #endif
    
    // Return value depends on mode to ensure different paths
    return (hash % 256) + (mode % 2);
}
