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
long double global_ldouble = 1.618033988749895L;
_Bool global_bool = 1;
#ifdef __cplusplus
bool global_cppbool = true;
#endif

/* ========== TYPE_STRING ========== */
// String literals and character arrays
const char* global_cstring = "Hello, World!";
const wchar_t* global_wstring = L"Wide String";
char global_char_array[50] = "Character array";
const char global_const_array[] = "Constant array";

/* ========== TYPE_STRUCT ========== */
// Basic C structures
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

// Global struct instances
struct SimpleStruct global_struct = {10, 3.14, "test"};
struct NestedStruct global_nested = {{5, 2.718, "inner"}, 100};
static struct BitFieldStruct static_bitfield = {1, 5, 9, -10};

/* ========== TYPE_USER_STRUCT (C++ only) ========== */
#ifdef __cplusplus
class BaseClass {
private:
    int private_member;
protected:
    float protected_member;
public:
    BaseClass(int x) : private_member(x), protected_member(0.0f) {}
    virtual ~BaseClass() {}
    virtual void virtual_func() { std::cout << "Base virtual\n"; }
    void public_func() { std::cout << "Base public\n"; }
};

class DerivedClass : public BaseClass {
private:
    double derived_private;
public:
    DerivedClass(int x, double d) : BaseClass(x), derived_private(d) {}
    void virtual_func() override { std::cout << "Derived virtual\n"; }
};

class MultipleInheritanceBase1 {
public:
    virtual void base1_func() = 0;
    virtual ~MultipleInheritanceBase1() {}
};

class MultipleInheritanceBase2 {
public:
    virtual void base2_func() = 0;
    virtual ~MultipleInheritanceBase2() {}
};

class MultipleInheritanceDerived : 
    public MultipleInheritanceBase1, 
    public MultipleInheritanceBase2 {
public:
    void base1_func() override {}
    void base2_func() override {}
};

// Template classes
template<typename T>
class TemplateClass {
    T value;
public:
    TemplateClass(T v) : value(v) {}
    T get_value() const { return value; }
    void set_value(T v) { value = v; }
};

// Explicit template instantiations
template class TemplateClass<int>;
template class TemplateClass<double>;
template class TemplateClass<SimpleStruct>;

// Complex template with inheritance
template<typename U, typename V>
class ComplexTemplate : public BaseClass {
    U first;
    V second;
public:
    ComplexTemplate(int x, U u, V v) : BaseClass(x), first(u), second(v) {}
};

// Instantiate complex template
ComplexTemplate<float, double> global_complex_template(1, 2.0f, 3.0);
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
        union {
            int int_value;
            double double_value;
            char* string_value;
        } data;
    } tagged;
    long long raw;
};

#ifdef __cplusplus
struct StructWithAnonymousUnion {
    int type;
    union {
        int int_member;
        float float_member;
        char char_member;
    };
};
#endif

// Global union instances
union SimpleUnion global_union = {.as_int = 0x41424344};
static union ComplexUnion static_complex_union;

/* ========== TYPE_POINTER ========== */
// Various pointer types
int* int_ptr = &global_int;
const int* const_int_ptr = &global_int;
volatile int* volatile_int_ptr = &global_int;
int* restrict restrict_int_ptr = &global_int;
int** double_int_ptr = &int_ptr;
const char* const* const_string_ptr_ptr = &global_cstring;

// Pointers to different types
struct SimpleStruct* struct_ptr = &global_struct;
union SimpleUnion* union_ptr = &global_union;
incomplete_ptr_t incomplete_ptr = nullptr;

// Function pointers
typedef int (*func_ptr_t)(int, double);
typedef void (*void_func_ptr_t)(void);
typedef const char* (*string_func_ptr_t)(int);

// Member function pointers (C++ only)
#ifdef __cplusplus
typedef void (BaseClass::*member_func_ptr_t)();
typedef int (TemplateClass<int>::*template_member_ptr_t)(int) const;
#endif

/* ========== TYPE_ARRAY ========== */
// Various array types
int int_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
const float const_float_array[5] = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f};
char* string_ptr_array[3] = {"first", "second", "third"};
int* pointer_array[5];

// Multi-dimensional arrays
int multi_dim_array[2][3][4] = {
    {{1,2,3,4}, {5,6,7,8}, {9,10,11,12}},
    {{13,14,15,16}, {17,18,19,20}, {21,22,23,24}}
};

// Array of structures
struct SimpleStruct struct_array[3] = {
    {1, 1.1, "one"},
    {2, 2.2, "two"},
    {3, 3.3, "three"}
};

// Array of unions
union SimpleUnion union_array[2] = {
    {.as_int = 0x12345678},
    {.as_float = 3.14f}
};

// Incomplete array in struct (C only)
struct WithIncompleteArray {
    int count;
    int values[];
};

/* ========== TYPE_CALLBACK ========== */
// Function pointer types and usage
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, void*);

struct CallbackContainer {
    callback_t callback;
    void* user_data;
};

// Function that takes callback
void register_callback(callback_t cb, void* data) {
    // Callback would be stored somewhere
    (void)cb;
    (void)data;
}

// Actual callback functions
int compare_ints(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

void sample_callback(int value, void* data) {
    *(int*)data = value * 2;
}

#ifdef __cplusplus
// C++ callbacks with std::function
std::function<int(int)> cpp_callback;

// Template with callback
template<typename Func>
void template_callback(Func f, int value) {
    f(value);
}

// Lambda expressions as callbacks
auto lambda_callback = [](int x) -> int { return x * x; };
#endif

/* ========== TYPE_LANG_STRUCT (C++ only) ========== */
#ifdef __cplusplus
// Lambda with different captures
auto lambda_with_capture = [global_int](int x) -> int {
    return x + global_int;
};

auto lambda_mutable = [counter = 0]() mutable -> int {
    return ++counter;
};

// std::initializer_list usage
auto init_list_func(std::initializer_list<int> list) -> int {
    int sum = 0;
    for (auto x : list) sum += x;
    return sum;
}

// Structured bindings (C++17)
struct Point { double x, y; };
auto get_point() -> Point { return {1.0, 2.0}; }

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

SimpleCoroutine sample_coroutine() {
    co_return;
}
#endif

// Complex template with language features
template<typename T>
class LanguageFeatureContainer {
    T value;
public:
    LanguageFeatureContainer(std::initializer_list<T> init) {
        for (const auto& v : init) {
            // Process initializer list
            (void)v;
        }
    }
    
    auto process_with_lambda() -> decltype(auto) {
        return [this](T x) -> T { return x + value; };
    }
};

// Instantiate with complex type
LanguageFeatureContainer<int> global_lang_container{1, 2, 3, 4, 5};
#endif

/* ========== Helper Functions ========== */
int process_scalars(int argc) {
    // Use all scalar types
    int local_int = argc;
    unsigned int local_uint = argc * 2u;
    char local_char = 'A' + (argc % 26);
    float local_float = argc * 0.5f;
    double local_double = argc * 1.5;
    _Bool local_bool = argc > 0;
    
    return local_int + local_uint + local_char + local_float + local_double + local_bool;
}

void use_pointers(int argc) {
    // Use various pointers
    int* local_ptr = &global_int;
    *local_ptr = argc;
    
    struct_ptr->x = argc;
    union_ptr->as_int = argc;
    
    // Function pointer usage
    comparator_t comp = compare_ints;
    int a = 5, b = 10;
    comp(&a, &b);
    
#ifdef __cplusplus
    // Member function pointer
    BaseClass base(1);
    member_func_ptr_t mem_ptr = &BaseClass::public_func;
    (base.*mem_ptr)();
#endif
}

void use_arrays(int argc) {
    // Access arrays
    for (int i = 0; i < 10 && i < argc; i++) {
        int_array[i] = i * argc;
    }
    
    // Multi-dimensional access
    if (argc > 0) {
        multi_dim_array[0][0][0] = argc;
    }
    
    // Array of structures
    for (int i = 0; i < 3 && i < argc; i++) {
        struct_array[i].x = i * argc;
    }
}

#ifdef __cplusplus
void use_cpp_features(int argc) {
    // Use C++ classes
    DerivedClass derived(argc, 3.14);
    derived.virtual_func();
    
    // Template usage
    TemplateClass<int> tc(argc);
    int val = tc.get_value();
    
    // Lambda usage
    auto result = lambda_callback(argc);
    
    // Initializer list
    auto sum = init_list_func({1, 2, 3, argc});
    
    // Structured binding
    auto [x, y] = get_point();
    
    // Fold expression
    auto total = sum_all(1, 2, 3, argc);
    
    // Language feature container
    auto lambda = global_lang_container.process_with_lambda();
    auto lambda_result = lambda(argc);
    
    (void)val;
    (void)result;
    (void)sum;
    (void)x;
    (void)y;
    (void)total;
    (void)lambda_result;
}
#endif

/* ========== Main Function ========== */
int main(int argc, char* argv[]) {
    // Use argc to prevent dead code elimination
    int hash = 0;
    
    // Process scalars
    hash ^= process_scalars(argc);
    
    // Use strings
    if (argc > 1) {
        hash ^= (int)global_cstring[0];
        hash ^= (int)global_wstring[0];
    }
    
    // Use structures
    global_struct.x = argc;
    global_nested.outer = argc;
    static_bitfield.flag1 = argc & 1;
    
    // Use unions
    if (argc % 2 == 0) {
        global_union.as_int = argc;
    } else {
        global_union.as_float = argc * 0.5f;
    }
    
    // Use pointers
    use_pointers(argc);
    
    // Use arrays
    use_arrays(argc);
    
    // Use callbacks
    CallbackContainer container = {sample_callback, &hash};
    if (argc > 0) {
        container.callback(argc, container.user_data);
    }
    
    // Register callback
    register_callback(sample_callback, &hash);
    
#ifdef __cplusplus
    // Use C++ features
    use_cpp_features(argc);
    
    // Use template callbacks
    template_callback(lambda_callback, argc);
    
    // Use complex template
    global_complex_template.virtual_func();
    
    #if __cplusplus >= 202002L
    // Coroutine usage if available
    sample_coroutine();
    #endif
#endif
    
    // Use incomplete pointer (TYPE_UNDEFINED)
    incomplete_ptr_t local_incomplete_ptr = incomplete_ptr;
    (void)local_incomplete_ptr;
    
    // Create flexible array struct (simulated)
    struct WithIncompleteArray* flex_array = 
        (struct WithIncompleteArray*)malloc(sizeof(struct WithIncompleteArray) + argc * sizeof(int));
    if (flex_array) {
        flex_array->count = argc;
        for (int i = 0; i < argc; i++) {
            flex_array->values[i] = i;
        }
        hash ^= flex_array->count;
        free(flex_array);
    }
    
    // Final output to prevent optimization
    printf("Result hash: %d\n", hash);
    
    return hash == 0 ? 0 : 1;
}
