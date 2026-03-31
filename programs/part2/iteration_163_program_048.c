#ifdef __cplusplus
#include <iostream>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <type_traits>
#endif

/* ========== TYPE_UNDEFINED ========== */
struct incomplete_struct;  // Forward declaration only
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
long double global_ldouble = 1.618033988749895L;
_Bool global_bool = 1;
#ifdef __cplusplus
bool global_cppbool = true;
#endif

/* ========== TYPE_STRING ========== */
const char* global_string = "Hello, World!";
const char global_char_array[] = "Character array";
const wchar_t* global_wstring = L"Wide string";
const wchar_t global_wchar_array[] = L"Wide array";

/* ========== TYPE_STRUCT ========== */
struct SimpleStruct {
    int x;
    double y;
    char name[32];
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

/* ========== TYPE_USER_STRUCT (C++ only) ========== */
#ifdef __cplusplus
class BaseClass {
public:
    BaseClass() : base_value(0) {}
    virtual ~BaseClass() {}
    virtual void virtual_func() { std::cout << "Base\n"; }
    int base_value;
protected:
    int protected_value;
private:
    int private_value;
};

class DerivedClass : public BaseClass {
public:
    DerivedClass() : derived_value(0) {}
    void virtual_func() override { std::cout << "Derived\n"; }
    int derived_value;
};

class MultipleInheritance : public BaseClass, public DerivedClass {
public:
    MultipleInheritance() : multi_value(0) {}
    void virtual_func() override { std::cout << "Multiple\n"; }
    int multi_value;
};

template<typename T>
class TemplateClass {
public:
    TemplateClass(T val) : value(val) {}
    T get_value() const { return value; }
private:
    T value;
};

class WithStatic {
public:
    static int static_member;
    const static double const_static;
};
int WithStatic::static_member = 100;
const double WithStatic::const_static = 3.14;
#endif

/* ========== TYPE_UNION ========== */
union SimpleUnion {
    int as_int;
    float as_float;
    char as_char[4];
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
        int x, y;
    } point;
    struct {
        float r, g, b, a;
    } color;
    long long big_value;
};

/* ========== TYPE_POINTER ========== */
int* int_ptr = &global_int;
const int* const_int_ptr = &global_int;
volatile int* volatile_int_ptr = &global_int;
int* restrict restrict_int_ptr = &global_int;
int** double_int_ptr = &int_ptr;
const char* const* const_string_ptr = &global_string;
void* void_ptr = nullptr;
#ifdef __cplusplus
int BaseClass::* member_ptr = &BaseClass::base_value;
void (BaseClass::* member_func_ptr)() = &BaseClass::virtual_func;
#endif

/* ========== TYPE_ARRAY ========== */
int simple_array[10] = {0,1,2,3,4,5,6,7,8,9};
int* array_of_pointers[5] = {&simple_array[0], &simple_array[1], &simple_array[2], &simple_array[3], &simple_array[4]};
int multi_dim_array[2][3][4] = {
    {{1,2,3,4}, {5,6,7,8}, {9,10,11,12}},
    {{13,14,15,16}, {17,18,19,20}, {21,22,23,24}}
};
SimpleStruct array_of_structs[3] = {
    {1, 2.0, "First"},
    {2, 3.0, "Second"},
    {3, 4.0, "Third"}
};

/* ========== TYPE_CALLBACK ========== */
typedef int (*SimpleCallback)(int, int);
typedef void (*ComplexCallback)(const char*, void*);
typedef int (*ArrayCallback)(int[], size_t);

struct CallbackContainer {
    SimpleCallback simple_cb;
    ComplexCallback complex_cb;
};

int add_callback(int a, int b) { return a + b; }
void print_callback(const char* msg, void* data) {
    #ifdef __cplusplus
    std::cout << msg << std::endl;
    #else
    printf("%s\n", msg);
    #endif
}

#ifdef __cplusplus
template<typename Func>
void template_callback(Func f, int x) {
    f(x);
}

auto lambda_callback = [](int x) -> int {
    return x * 2;
};
#endif

/* ========== TYPE_LANG_STRUCT (C++ only) ========== */
#ifdef __cplusplus
// Lambda with different captures
auto lambda_with_capture = [global_int](int x) -> int {
    return x + global_int;
};

auto lambda_with_ref_capture = [&global_double](double x) -> double {
    return x * global_double;
};

// Initializer list usage
std::initializer_list<int> init_list = {1, 2, 3, 4, 5};

// Structured bindings (C++17)
struct Point { int x; int y; };
auto get_point() -> Point { return {10, 20}; }

// Fold expressions (C++17)
template<typename... Args>
auto sum_all(Args... args) {
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

DummyCoroutine dummy_coro() {
    co_return;
}
#endif

// Variadic template with complex pattern
template<typename... Ts>
struct VariadicStruct {};

template<typename T, typename... Rest>
void process_variadic(T first, Rest... rest) {
    // Complex template instantiation
}
#endif

/* ========== Function using incomplete pointer ========== */
void use_incomplete_pointer(incomplete_ptr_t ptr) {
    // Can't dereference, but can pass around
    (void)ptr;
}

/* ========== Main function with execution flow ========== */
int main(int argc, char* argv[]) {
    unsigned long hash = 0;
    
    /* Force TYPE_UNDEFINED usage */
    incomplete_ptr_t incomplete_ptr = nullptr;
    use_incomplete_pointer(incomplete_ptr);
    
    /* Force TYPE_SCALAR usage */
    hash += sizeof(global_int);
    hash += sizeof(global_float);
    hash += sizeof(global_double);
    
    /* Force TYPE_STRING usage */
    hash += (unsigned long)global_string[0];
    hash += (unsigned long)global_char_array[0];
    
    /* Force TYPE_STRUCT usage */
    SimpleStruct local_struct = {10, 20.5, "Local"};
    NestedStruct nested = {{5, 10.0, "Inner"}, 100};
    BitFieldStruct bitfield = {1, 3, 7, -50};
    PackedStruct packed = {'X', 42, 3.14};
    
    hash += local_struct.x;
    hash += (unsigned long)nested.outer;
    hash += bitfield.flag1;
    hash += packed.a;
    
    /* Force TYPE_UNION usage */
    SimpleUnion su;
    su.as_int = 0x41424344;
    hash += su.as_char[0];
    
    StructWithAnonymousUnion sau;
    sau.type = 1;
    sau.int_value = 100;
    hash += sau.int_value;
    
    /* Force TYPE_POINTER usage */
    int local_int = 42;
    int* local_ptr = &local_int;
    int** local_double_ptr = &local_ptr;
    hash += (unsigned long)local_ptr;
    hash += (unsigned long)*local_double_ptr;
    
    /* Force TYPE_ARRAY usage */
    for (int i = 0; i < 10; i++) {
        hash += simple_array[i];
    }
    
    int matrix[2][3] = {{1,2,3}, {4,5,6}};
    hash += matrix[0][0];
    
    /* Force TYPE_CALLBACK usage */
    SimpleCallback cb = add_callback;
    int result = cb(10, 20);
    hash += result;
    
    CallbackContainer container = {add_callback, print_callback};
    result = container.simple_cb(30, 40);
    hash += result;
    
    #ifdef __cplusplus
    /* Force TYPE_USER_STRUCT usage */
    BaseClass* base = new DerivedClass();
    base->virtual_func();
    hash += base->base_value;
    delete base;
    
    TemplateClass<int> tc_int(100);
    TemplateClass<double> tc_double(3.14);
    hash += tc_int.get_value();
    hash += (unsigned long)tc_double.get_value();
    
    DerivedClass derived;
    MultipleInheritance multi;
    hash += derived.derived_value;
    hash += multi.multi_value;
    
    /* Force TYPE_LANG_STRUCT usage */
    auto lambda_result = lambda_with_capture(10);
    hash += lambda_result;
    
    auto lambda_ref_result = lambda_with_ref_capture(2.0);
    hash += (unsigned long)lambda_ref_result;
    
    for (auto val : init_list) {
        hash += val;
    }
    
    auto [x, y] = get_point();
    hash += x + y;
    
    auto fold_result = sum_all(1, 2, 3, 4, 5);
    hash += fold_result;
    
    #ifdef __cpp_coroutines
    dummy_coro();
    #endif
    
    VariadicStruct<int, double, char> variadic;
    process_variadic(1, 2.0, 'c', "string");
    
    /* Use template callback with lambda */
    template_callback(lambda_callback, 42);
    #endif
    
    /* Use argc to prevent dead code elimination */
    if (argc > 1) {
        /* Branch 1: Use different type combinations */
        ComplexUnion cu;
        cu.point.x = 10;
        cu.point.y = 20;
        hash += cu.big_value;
        
        #ifdef __cplusplus
        WithStatic ws;
        hash += WithStatic::static_member;
        #endif
    } else {
        /* Branch 2: Alternative type usage */
        FlexibleArrayStruct* fas = (FlexibleArrayStruct*)malloc(
            sizeof(FlexibleArrayStruct) + 10 * sizeof(int));
        fas->count = 10;
        for (int i = 0; i < 10; i++) {
            fas->data[i] = i * 2;
            hash += fas->data[i];
        }
        free(fas);
    }
    
    /* Produce observable output */
    #ifdef __cplusplus
    std::cout << "Type coverage hash: " << hash << std::endl;
    #else
    printf("Type coverage hash: %lu\n", hash);
    #endif
    
    return (int)(hash % 256);
}
