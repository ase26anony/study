void schedule_trigger(void) {
    volatile int a = 1, b = 2, c = 3;
    a = b + 1;      /* RAW dependency on b */
    asm volatile("" ::: "memory");
    c = a * 2;      /* RAW dependency on a */
    b = c - a;      /* RAW dependencies on c and a */
}
