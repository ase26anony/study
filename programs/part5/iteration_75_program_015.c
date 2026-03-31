// High priority (frequently used, long-lived):
// - sum (accumulator, used every iteration)
// - i (loop counter)
// - arr (base pointer)

// Medium priority (used within iteration):
// - t1, t2, t3, t4 (sequential computation chain)
// - t5, t6 (later in iteration)

// Low priority (temporary):
// - Address calculation temporaries
