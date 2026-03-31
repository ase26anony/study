x₀ = 0
for (int i = 0; i < n; i++) {
    // Phi node needed here
    x₁ = φ(x₀, x₄)  // x₁ gets x₀ on first iteration, x₄ on subsequent iterations
    
    if (x₁ == 0) {
        x₂ = 1;
    } else {
        x₃ = 0;
    }
    
    x₄ = φ(x₂, x₃)  // Merge the two possible definitions
}
