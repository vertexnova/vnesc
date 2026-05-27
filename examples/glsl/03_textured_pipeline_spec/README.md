# 03_textured_pipeline_spec

Loads `shaders/textured.pipeline.json` via `loadShaderPipelineSpec()`, builds an MSL bundle with a `sampler2D` uniform, and demonstrates `MetalBindingLayout` override: the JSON sets `buffer_base: 16` (vnerhi default), then the example sets `desc.metal_layout.buffer_base = 0` after `toBuildDesc()`.

```bash
cmake --build build --target vnesc_example_03_textured_pipeline_spec
cd build/bin/examples/glsl/03_textured_pipeline_spec
./vnesc_example_03_textured_pipeline_spec
```

Requires `VNE_SC_JSON=ON` for the pipeline spec parser.
