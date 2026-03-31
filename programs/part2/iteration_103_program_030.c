// main.cpp - C++ main program targeting multiple DWARF attributes
#include <iostream>
#include <thread>
#include <atomic>

// For DW_AT_explicit
class ExplicitConstructor {
public:
    int value;
    explicit ExplicitConstructor(int v) : value(v) {}
    explicit operator bool() const { return value != 0; }
};

// For DW_AT_mutable
class WithMutable {
private:
    mutable int cache;
    int expensive_computation() const { return 42; }
public:
    int get_value() const {
        if (cache == 0) {
            cache = expensive_computation();  // mutable allows modification in const method
        }
        return cache;
    }
};

// For DW_AT_prototyped
extern "C" {
    void fortran_subroutine(char* str, int* len);  // Prototyped C declaration
    void ada_procedure();
}

// For DW_AT_location and DW_AT_segment
__attribute__((section("mysection"))) int custom_section_var = 42;
thread_local int thread_local_var = 100;
register int register_var asm ("r12") = 0;  // May generate complex location info

// For DW_AT_lower_bound
template<typename T, size_t N>
struct ArrayWrapper {
    T data[N];
    T& operator[](size_t i) { return data[i]; }
};

// Complex inheritance to force detailed type info
class Base {
public:
    virtual ~Base() = default;
    virtual void foo() = 0;
};

class Derived : public Base {
    mutable int mutable_counter = 0;  // Another mutable
public:
    explicit Derived(int init) : mutable_counter(init) {}
    void foo() override { ++mutable_counter; }
};

// Use inline assembly to force location lists
void use_assembly() {
    int x = 10;
    asm volatile (
        "mov %0, %%eax\n\t"
        : : "r" (x) : "%eax"
    );
}

int main() {
    // Trigger DW_AT_explicit
    ExplicitConstructor ec(5);
    if (static_cast<bool>(ec)) {
        std::cout << "Explicit constructor used\n";
    }
    
    // Trigger DW_AT_mutable
    WithMutable wm;
    std::cout << "Mutable value: " << wm.get_value() << "\n";
    
    // Use custom section variable (DW_AT_location, DW_AT_segment)
    custom_section_var = 43;
    std::cout << "Custom section var: " << custom_section_var << "\n";
    
    // Use thread-local (complex location)
    thread_local_var = 101;
    std::cout << "Thread local: " << thread_local_var << "\n";
    
    // Use array with bounds (DW_AT_lower_bound)
    ArrayWrapper<int, 10> arr;
    arr[0] = 1;
    
    // Use inheritance hierarchy
    Derived d(10);
    Base* b = &d;
    b->foo();
    
    // Call Fortran subroutine
    char str[] = "Hello";
    int len = 5;
    fortran_subroutine(str, &len);
    
    // Call Ada procedure if available
    // ada_procedure();
    
    // Use assembly
    use_assembly();
    
    // Prevent dead code elimination
    volatile int keep_alive = 0;
    keep_alive += ec.value;
    keep_alive += custom_section_var;
    keep_alive += thread_local_var;
    
    return 0;
}
