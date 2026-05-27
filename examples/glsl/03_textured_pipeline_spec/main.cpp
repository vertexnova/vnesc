/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * ----------------------------------------------------------------------
 */

#include "logging_guard.h"

#include <vertexnova/sc/vnesc.h>

#include <filesystem>

int main() {
    vne::sc::examples::LoggingGuard logging;

    const std::filesystem::path spec_path =
        std::filesystem::path(__FILE__).parent_path() / "shaders" / "textured.pipeline.json";

    auto spec = vne::sc::loadShaderPipelineSpec(spec_path.string());
    if (!spec) {
        VNE_LOG_ERROR << "Failed to load spec: " << spec_path.string();
        return 1;
    }

    const auto spec_dir = spec_path.parent_path();
    auto desc = spec->toBuildDesc(spec_dir);

    // Override parsed layout: JSON uses vnerhi default (16); force buffer_base = 0 at runtime.
    VNE_LOG_INFO << "Parsed metal_layout buffer_base=" << desc.metal_layout.buffer_base;
    desc.metal_layout.buffer_base = 0;
    VNE_LOG_INFO << "Override metal_layout buffer_base=" << desc.metal_layout.buffer_base;

    auto builder = vne::sc::ShaderCompilerFactory::createPipelineBuilder(spec->source_lang);
    if (!builder) {
        VNE_LOG_ERROR << "No pipeline builder";
        return 1;
    }

    VNE_LOG_INFO << "Building pipeline '" << desc.name << "' from " << spec_path.string();
    auto result = builder->build(desc);
    if (!result.ok()) {
        VNE_LOG_ERROR << "Build failed: " << result.error;
        return 1;
    }

    for (const auto& stage : result.artifact.stages) {
        VNE_LOG_INFO << "Stage " << static_cast<int>(stage.stage) << " bindings:";
        for (const auto& b : stage.reflection.bindings) {
            if (b.slots.metal) {
                VNE_LOG_INFO << "  " << b.name << " type=" << static_cast<int>(b.type) << " set=" << b.set
                             << " binding=" << b.binding << " metal tex=" << b.slots.metal->texture
                             << " smp=" << b.slots.metal->sampler;
            } else {
                VNE_LOG_INFO << "  " << b.name << " type=" << static_cast<int>(b.type) << " set=" << b.set
                             << " binding=" << b.binding;
            }
        }
    }

    const std::filesystem::path out_dir = std::filesystem::current_path() / "output" / "textured_bundle";
    std::filesystem::create_directories(out_dir);
    if (!vne::sc::writeShaderBundle(result.artifact, out_dir)) {
        VNE_LOG_ERROR << "Failed to write bundle";
        return 1;
    }
    VNE_LOG_INFO << "Bundle written to: " << out_dir.string();
    return 0;
}
