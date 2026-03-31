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
typedef struct undefined_type* undefined_ptr_t;

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

/* ========== TYPE_STRING ========== */
const char* global_string = "Hello, World!";
const wchar_t* global_wstring = L"Wide String";
char global_char_array[50] = "Character array";
const char* const global_const_string_ptr = "Constant pointer to constant string";

/* ========== TYPE_STRUCT ========== */
struct SimpleStruct {
    int x;
    double y;
    char name[32];
};

struct NestedStruct {
    struct SimpleStruct inner;
    int outer_value;
};

struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int : 24;  // Padding
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
    void public_method() {}
};

class DerivedClass : public BaseClass {
public:
    DerivedClass() : additional_member(0) {}
    void virtual_func() override {}
private:
    int additional_member;
};

class MultipleInheritanceBase1 {
public:
    virtual void base1_method() {}
};

class MultipleInheritanceBase2 {
public:
    virtual void base2_method() {}
};

class MultipleDerived : public MultipleInheritanceBase1, 
                       public MultipleInheritanceBase2 {
public:
    void base1_method() override {}
    void base2_method() override {}
};

template<typename T>
class TemplateClass {
    T value;
public:
    TemplateClass(T v) : value(v) {}
    T get_value() const { return value; }
};

class ClassWithStatic {
    static int static_member;
    int instance_member;
public:
    ClassWithStatic() : instance_member(0) {}
};
int ClassWithStatic::static_member = 42;

#endif  // __cplusplus

/* ========== TYPE_UNION ========== */
union SimpleUnion {
    int as_int;
    float as_float;
    char as_char[4];
};

union ComplexUnion {
    struct SimpleStruct as_struct;
    double as_double;
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

double** double_ptr_ptr = &int_ptr;  // Will be reassigned
void* void_ptr = nullptr;

struct SimpleStruct* struct_ptr = nullptr;
union SimpleUnion* union_ptr = nullptr;

typedef int (*func_ptr_t)(int, double);
typedef void (*void_func_ptr_t)(void);
typedef int (*array_ptr_t)[10];

#ifdef __cplusplus
typedef void (BaseClass::*member_func_ptr_t)();
#endif

/* ========== TYPE_ARRAY ========== */
int int_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
int multi_dim_array[2][3][4];
char string_array[3][20] = {"First", "Second", "Third"};
struct SimpleStruct struct_array[5];
int* pointer_array[8];

#ifdef __cplusplus
extern "C" {
#endif
struct ArrayInStruct {
    int count;
    int variable_array[];  // Incomplete array
};
#ifdef __cplusplus
}
#endif

/* ========== TYPE_CALLBACK ========== */
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, void*);

struct CallbackContainer {
    Callback callback;
    void* user_data;
};

int compare_ints(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

void example_callback(int value, void* data) {
    // Do nothing
}

#ifdef __cplusplus
template<typename Func>
void template_callback(Func f) {
    f(42);
}

std::function<void(int)> std_function_callback;
#endif

/* ========== TYPE_LANG_STRUCT (C++ only) ========== */
#ifdef __cplusplus
auto lambda_with_capture = [global_int](int x) -> int {
    return x + global_int;
};

auto lambda_no_capture = [](double x, double y) -> double {
    return x * y;
};

void use_initializer_list(std::initializer_list<int> list) {
    for (auto x : list) {
        // Process
    }
}

template<typename... Args>
auto sum(Args... args) {
    return (args + ...);  // Fold expression
}

struct CoroutineExample {
    struct promise_type {
        CoroutineExample get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

CoroutineExample example_coroutine() {
    co_return;
}

template<typename T>
class TypeDependentClass {
    T value;
public:
    TypeDependentClass(T v) : value(v) {}
    
    auto get_value() const {
        if constexpr (std::is_pointer_v<T>) {
            return *value;
        } else {
            return value;
        }
    }
};

#endif  // __cplusplus

/* ========== Main function with execution flow ========== */
int main(int argc, char* argv[]) {
    /* Force TYPE_UNDEFINED usage */
    undefined_ptr_t undefined_ptr = nullptr;
    
    /* Force TYPE_SCALAR usage */
    int local_int = argc;
    unsigned int local_uint = argc * 2u;
    char local_char = (char)argc;
    float local_float = argc * 0.5f;
    double local_double = argc * 1.5;
    _Bool local_bool = argc > 0;
    
    /* Force TYPE_STRING usage */
    const char* local_string = argv[0] ? argv[0] : "default";
    wchar_t local_wchar = L'X';
    
    /* Force TYPE_STRUCT usage */
    struct SimpleStruct local_struct = {1, 2.0, "test"};
    struct NestedStruct nested = {{10, 3.14, "inner"}, 20};
    struct BitFieldStruct bitfield = {1, 5, 9};
    struct PackedStruct packed = {'Z', 100, 3.14};
    
    /* Force TYPE_UNION usage */
    union SimpleUnion simple_union;
    simple_union.as_int = 0x41424344;
    
#ifdef __cplusplus
    /* Force TYPE_USER_STRUCT usage */
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    TemplateClass<int> int_template(42);
    TemplateClass<double> double_template(3.14);
    TemplateClass<SimpleStruct> struct_template({1, 2.0, "template"});
    
    MultipleDerived multiple;
    
    StructWithAnonymousUnion anon_union_struct;
    anon_union_struct.type = 1;
    anon_union_struct.int_value = 100;
#endif
    
    /* Force TYPE_POINTER usage */
    int** local_int_ptr_ptr = &int_ptr;
    void (*local_func_ptr)(void) = nullptr;
    
    double_ptr_ptr = (double**)&int_ptr;  // Type punning
    
#ifdef __cplusplus
    member_func_ptr_t mem_func_ptr = &BaseClass::public_method;
#endif
    
    /* Force TYPE_ARRAY usage */
    int local_array[argc > 10 ? 10 : 5];  // Variable size based on argc
    for (int i = 0; i < (argc > 10 ? 10 : 5); i++) {
        local_array[i] = i * argc;
    }
    
    struct SimpleStruct local_struct_array[3] = {
        {1, 1.0, "first"},
        {2, 2.0, "second"},
        {3, 3.0, "third"}
    };
    
    /* Force TYPE_CALLBACK usage */
    Comparator comp = compare_ints;
    struct CallbackContainer cb_container = {example_callback, nullptr};
    
    if (argc > 1) {
        qsort(local_array, argc > 10 ? 10 : 5, sizeof(int), comp);
        cb_container.callback(argc, nullptr);
    }
    
#ifdef __cplusplus
    /* Force TYPE_LANG_STRUCT usage */
    auto result = lambda_with_capture(argc);
    auto product = lambda_no_capture(2.0, 3.0);
    
    use_initializer_list({1, 2, 3, 4, 5});
    
    auto fold_result = sum(1, 2, 3, 4, 5);
    
    TypeDependentClass<int*> ptr_wrapper(&local_int);
    TypeDependentClass<int> val_wrapper(42);
    
    // Structured binding (C++17)
    struct Point { int x; int y; };
    Point p{10, 20};
    auto [x_coord, y_coord] = p;
    
    // Coroutine usage
    example_coroutine();
    
    // Template callback with lambda
    template_callback([argc](int x) {
        // Use argc to prevent optimization
        if (argc > 0) {
            std::cout << "Callback: " << x * argc << std::endl;
        }
    });
    
    std_function_callback = [](int x) {
        std::cout << "Std function: " << x << std::endl;
    };
    std_function_callback(argc);
#endif
    
    /* Prevent dead code elimination */
    volatile int checksum = 0;
    checksum += sizeof(local_struct);
    checksum += (int)(void*)&global_string;
    checksum += (int)simple_union.as_int;
    checksum += argc;
    
#ifdef __cplusplus
    checksum += (int)(void*)base_ptr;
    checksum += int_template.get_value();
    checksum += (int)mem_func_ptr;
#endif
    
    /* Output to ensure execution */
    printf("Checksum: %d\n", checksum);
    
    return checksum == 0 ? 0 : 1;
}
