#ifdef __cplusplus
#include <iostream>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <type_traits>
#endif

/* ========== TYPE_UNDEFINED ========== */
struct undefined_type;  // Forward declaration only
undefined_type* global_undefined_ptr;

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
// Basic C structure
struct BasicStruct {
    int x;
    double y;
    char name[32];
};

// Nested structure
struct OuterStruct {
    int id;
    struct BasicStruct inner;
    struct {
        int anonymous_member;
    } anonymous;
};

// Structure with bit-fields
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int : 4;  // Padding
    signed int value : 8;
};

// Packed structure
struct PackedStruct {
    char a;
    int b;
    double c;
} __attribute__((packed));

// Structure with flexible array member (C mode)
#ifdef __cplusplus
extern "C" {
#endif
struct FlexArrayStruct {
    int count;
    int data[];  // Flexible array member
};
#ifdef __cplusplus
}
#endif

/* ========== TYPE_USER_STRUCT (C++ only) ========== */
#ifdef __cplusplus
// Simple class
class SimpleClass {
public:
    SimpleClass() : value(0) {}
    explicit SimpleClass(int v) : value(v) {}
    ~SimpleClass() {}
    
    int getValue() const { return value; }
    void setValue(int v) { value = v; }
    
private:
    int value;
};

// Class with inheritance
class BaseClass {
public:
    BaseClass() : base_data(0) {}
    virtual ~BaseClass() {}
    virtual void virtualFunc() { std::cout << "Base\n"; }
    
protected:
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    DerivedClass() : derived_data(0) {}
    void virtualFunc() override { std::cout << "Derived\n"; }
    
private:
    int derived_data;
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

class MultiDerived : public Interface1, public Interface2 {
public:
    void method1() override {}
    void method2() override {}
};

// Template class
template<typename T>
class TemplateClass {
public:
    TemplateClass(T val) : data(val) {}
    T getData() const { return data; }
    
private:
    T data;
};

// Class with different access specifiers
class AccessClass {
public:
    int public_var;
    
    AccessClass() : public_var(0), protected_var(0), private_var(0) {}
    
protected:
    int protected_var;
    
private:
    int private_var;
    friend void friendFunction(AccessClass&);
};

void friendFunction(AccessClass& obj) {
    obj.private_var = 42;
}

#endif  // __cplusplus

/* ========== TYPE_UNION ========== */
// C-style union
union DataUnion {
    int int_val;
    float float_val;
    double double_val;
    char* string_val;
};

// Union within structure
struct UnionContainer {
    int type;
    union {
        int int_data;
        float float_data;
        struct BasicStruct struct_data;
    } data;
};

// Anonymous union (C++11)
#ifdef __cplusplus
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

double** double_ptr_ptr = nullptr;
char*** char_ptr_ptr_ptr = nullptr;

// Pointer to array
int (*array_ptr)[10] = nullptr;

// Pointer to structure
struct BasicStruct* struct_ptr = nullptr;

// Pointer to union
union DataUnion* union_ptr = nullptr;

/* ========== TYPE_ARRAY ========== */
// Various array types
int int_array[10] = {0};
double double_array[5][10];
char char_3d_array[2][3][4];

// Array of pointers
int* ptr_array[5];

// Array of structures
struct BasicStruct struct_array[3];

// Incomplete array in structure (C mode)
#ifdef __cplusplus
extern "C" {
#endif
struct IncompleteArray {
    int count;
    int items[];
};
#ifdef __cplusplus
}
#endif

// Variable Length Array (VLA) - C99 only
#ifdef __STDC_VERSION__
#if __STDC_VERSION__ >= 199901L
void use_vla(int size) {
    int vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = i;
    }
}
#endif
#endif

/* ========== TYPE_CALLBACK ========== */
// Function pointer types
typedef int (*BinaryFunc)(int, int);
typedef void (*VoidFunc)(void);
typedef void (*Callback)(int, void*);

// Structure with function pointer
struct CallbackContainer {
    Callback callback;
    void* user_data;
};

// Function using callback
void register_callback(Callback cb, void* data) {
    CallbackContainer container = {cb, data};
}

// Actual callback function
void my_callback(int value, void* data) {
    *(int*)data = value * 2;
}

#ifdef __cplusplus
// C++ function objects and lambdas
template<typename Func>
void template_callback(Func f) {
    f(42);
}

// Lambda with different captures
auto lambda_with_capture = [global_int](int x) {
    return x + global_int;
};

#endif

/* ========== TYPE_LANG_STRUCT (C++ only) ========== */
#ifdef __cplusplus
// Lambda expressions
auto simple_lambda = [](int x) { return x * 2; };
auto capturing_lambda = [global_int, &global_double](int x) {
    return x + global_int + static_cast<int>(global_double);
};

// std::initializer_list
void use_initializer_list(std::initializer_list<int> list) {
    for (auto val : list) {
        // Process values
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

SimpleCoroutine simple_coro() {
    co_return;
}
#endif

// Complex template with language features
template<typename T>
class LanguageFeatureContainer {
public:
    void process(T value) {
        // Lambda in template
        auto lambda = [this, value]() {
            return value * 2;
        };
        
        // Initializer list
        std::initializer_list<T> init_list = {value, value * 2, value * 3};
        
        // Structured binding if T is tuple-like
        if constexpr (std::is_same_v<T, std::tuple<int, int>>) {
            auto [a, b] = value;
        }
    }
    
private:
    T data;
};

#endif  // __cplusplus

/* ========== Main function ========== */
int main(int argc, char* argv[]) {
    // Use argc to prevent dead code elimination
    int mode = (argc > 1) ? argv[1][0] : 'A';
    
    /* TYPE_UNDEFINED usage */
    undefined_type* local_undefined_ptr = nullptr;
    if (mode == 'U') {
        // Force analysis of undefined type pointer
        struct UndefContainer {
            undefined_type* ptr;
        } container;
        container.ptr = local_undefined_ptr;
    }
    
    /* TYPE_SCALAR usage */
    int local_int = 10;
    unsigned int local_uint = 20u;
    char local_char = 'B';
    float local_float = 1.5f;
    double local_double = 3.14;
    _Bool local_bool = 0;
    
    // Use scalars in computation
    int scalar_result = global_int + local_int - global_uint + local_uint;
    
    /* TYPE_STRING usage */
    const char* local_string = "Local string";
    wchar_t local_wstring[] = L"Local wide string";
    char local_char_array[] = "Local array";
    
    /* TYPE_STRUCT usage */
    struct BasicStruct bs = {1, 2.0, "Test"};
    struct OuterStruct os;
    os.id = 100;
    os.inner = bs;
    os.anonymous.anonymous_member = 50;
    
    struct BitFieldStruct bfs = {1, 7, -50};
    struct PackedStruct ps = {'X', 123, 45.67};
    
    // Take addresses to force pointer types
    struct BasicStruct* bs_ptr = &bs;
    struct OuterStruct* os_ptr = &os;
    
    /* TYPE_USER_STRUCT usage (C++ only) */
#ifdef __cplusplus
    SimpleClass simple_obj(42);
    DerivedClass derived_obj;
    BaseClass* base_ptr = &derived_obj;
    
    TemplateClass<int> template_int(100);
    TemplateClass<double> template_double(3.14);
    
    AccessClass access_obj;
    friendFunction(access_obj);
    
    // Virtual function call
    if (mode == 'V') {
        base_ptr->virtualFunc();
    }
#endif
    
    /* TYPE_UNION usage */
    union DataUnion du;
    du.int_val = 42;
    
    struct UnionContainer uc;
    uc.type = 1;
    uc.data.int_data = 100;
    
#ifdef __cplusplus
    AnonymousUnionStruct aus;
    aus.tag = 2;
    aus.as_double = 3.14159;
#endif
    
    /* TYPE_POINTER usage */
    int* local_int_ptr = &local_int;
    const volatile int* cv_int_ptr = &global_int;
    
    // Double pointer
    int** pp = &local_int_ptr;
    
    // Function pointer
    BinaryFunc add_func = [](int a, int b) { return a + b; };
    int func_result = add_func(10, 20);
    
    /* TYPE_ARRAY usage */
    int local_array[5] = {1, 2, 3, 4, 5};
    double matrix[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    
    // Array of structures
    struct BasicStruct structs[2] = {{1, 1.0, "First"}, {2, 2.0, "Second"}};
    
    // Use VLA if available
#ifdef __STDC_VERSION__
#if __STDC_VERSION__ >= 199901L
    if (mode == 'V') {
        use_vla(10);
    }
#endif
#endif
    
    /* TYPE_CALLBACK usage */
    CallbackContainer cc = {my_callback, &local_int};
    register_callback(my_callback, &local_int);
    
#ifdef __cplusplus
    // Use template callback with lambda
    template_callback([](int x) {
        // Do something
    });
    
    // Use capturing lambda
    int capture_result = lambda_with_capture(10);
#endif
    
    /* TYPE_LANG_STRUCT usage (C++ only) */
#ifdef __cplusplus
    // Use initializer_list
    use_initializer_list({1, 2, 3, 4, 5});
    
    // Use structured binding
    if (mode == 'S') {
        use_structured_binding();
    }
    
    // Use fold expression
    int fold_result = sum(1, 2, 3, 4, 5);
    
    // Instantiate template with language features
    LanguageFeatureContainer<int> lfc;
    lfc.process(42);
    
#if __cplusplus >= 202002L
    // Use coroutine if C++20
    if (mode == 'C') {
        auto coro = simple_coro();
    }
#endif
    
    // Complex lambda in template context
    auto complex_lambda = [&local_int, global_double](auto x) -> decltype(x + local_int) {
        return x + local_int + static_cast<decltype(x)>(global_double);
    };
    auto lambda_result = complex_lambda(3.14);
#endif
    
    // Generate observable output based on all types
    // Compute a hash using addresses and sizes
    unsigned long hash = 0;
    
    // Mix in scalar values
    hash ^= (unsigned long)global_int;
    hash ^= (unsigned long)global_double * 2;
    
    // Mix in addresses
    hash ^= (unsigned long)&bs;
    hash ^= (unsigned long)&du;
    hash ^= (unsigned long)int_ptr;
    
    // Mix in sizes
    hash ^= sizeof(struct BasicStruct);
    hash ^= sizeof(union DataUnion);
    hash ^= sizeof(int_array);
    
#ifdef __cplusplus
    // C++ specific hashing
    hash ^= (unsigned long)&simple_obj;
    hash ^= sizeof(SimpleClass);
    
    std::cout << "Type coverage hash: " << hash << std::endl;
#else
    // C output
    printf("Type coverage hash: %lu\n", hash);
#endif
    
    // Return value based on mode to ensure all code paths are considered
    return (mode % 2) ? 0 : 1;
}
