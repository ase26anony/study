x₀ = 0
for (int i = 0; i < n; i++) {
    // Phi node needed here
    x₁ = φ(x₀, x₃)  // First iteration: x₁ = x₀ (0), subsequent: x₁ = x₃ from previous iteration
    
    if (x₁ == 0) {
        x₂ = 1;
    } else {
        x₃ = 0;
    }
    
    // Another phi needed to merge x₂ and x₃ for next iteration
    x₄ = φ(x₂, x₃)
}
