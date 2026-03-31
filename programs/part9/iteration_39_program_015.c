int i₀ = 0;          // Initial definition
int i₁ = Φ(i₀, i₂);  // Loop Phi node

while (i₁ < n) {
    if (i₁ == 0) {   // Only true when i₁ comes from i₀ (first iteration)
        // body - executes only once
    }
    i₂ = i₁ + 1;     // Increment creates new SSA name
    i₁ = Φ(i₀, i₂);  // Phi merges for next iteration
}
