Thread 1: Reads g (value = 0)
Thread 2: Reads g (value = 0)
Thread 1: Compares g > local (0 > 5 = false)
Thread 2: Compares g > local (0 > 5 = false)
// Both threads skip the if block
// No problem in this specific execution, but...

// Another scenario:
Thread 1: Reads g (value = 0)
Thread 1: Compares g > local (0 > 5 = false)
Thread 2: Reads g (value = 0)
Thread 2: Compares g > local (0 > 5 = false)
// Still fine...

// But consider if g starts at 10:
Thread 1: Reads g (value = 10)
Thread 2: Reads g (value = 10)
Thread 1: Compares g > local (10 > 5 = true)
Thread 2: Compares g > local (10 > 5 = true)
Thread 1: Sets g = 5
Thread 2: Sets g = 5
// Both threads enter the if block, both set g to 5
// This is actually safe in terms of final value, but...

// The real issue: between comparison and assignment
Thread 1: Reads g (value = 10)
Thread 1: Compares g > local (10 > 5 = true)
Thread 2: Reads g (value = 10)
Thread 2: Compares g > local (10 > 5 = true)
Thread 1: Sets g = 5
// Now g = 5
Thread 2: Sets g = 5 (but g is already 5!)
// Thread 2's comparison is now stale - g was 10 when checked, but is 5 now
