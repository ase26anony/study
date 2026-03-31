// Typical addc usage
asm volatile ("addc %0, %1, %2"
              : "=r"(result)
              : "r"(a), "r"(b)
              : "cc");  // Condition codes clobbered
