# SandboxOGL

Modern OpenGL renderer used as a sandbox to implement papers, test various techniques… It started as a way to learn OpenGL a few months maybe years ago, and now it has become a quick prototyping tool so it's entirely possible (expected?) to find dead code, too many 'auto' uses, hardcoded values, or bad design choices, even though I tried to refactor

List of features : 
- Object loading using Assimp, stb. Math using GLM, Simple GUI using ImGui
- Profiling CPU and GPU using RAII timers
- Shader hot reloading
- Deferred lighting using light accumulation buffers
- Metalness PBR pipeline, HDR, shading using Lambert/GGX-Fresnel-Smith
- PCF shadow mapping
- Area light (See "Real-Time Shading With Area Light Sources with Linearly Transformed Cosines ") 
- Deformable snow (See "Deformable Snow Rendering in Batman: Arkham Origins")
- Ocean rendering using Gerstner waves on a polar grid
- Frustum culling
- Deferred decals
- Stencil-based picking system
- IBL, parrallax corrected cubemap
- Grass using geometry shader generated & animated blades
- Scalable Ambient Obscurance
- FXAA
- Ongoing : Screen space reflection, temporal anti-aliasing
- Deleted (but might still find remnants in the code) : Preetham sky, (botched) Bruneton sky, particles system, depth of field...

A few GIF [here](https://mehdins.wordpress.com/portfolio0/)
