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

    const std::filesystem::path shader_dir = std::filesystem::path(__FILE__).parent_path() / "shaders";

    auto builder = vne::sc::ShaderCompilerFactory::createPipelineBuilder(vne::sc::SourceLang::eGLSL);
    if (!builder) {
        VNE_LOG_ERROR << "No pipeline builder";
        return 1;
    }

    vne::sc::PipelineBuildDesc desc;
    desc.name = "triangle";
    desc.targets = {vne::sc::CrossTarget::eMSL};
    desc.validate = false;
    desc.use_cache = false;

    vne::sc::CompileRequest vert;
    vert.file_path = (shader_dir / "triangle.vert.glsl").string();
    vert.stage = vne::sc::ShaderStage::eVertex;

    vne::sc::CompileRequest frag;
    frag.file_path = (shader_dir / "triangle.frag.glsl").string();
    frag.stage = vne::sc::ShaderStage::eFragment;

    desc.stages = {vert, frag};

    VNE_LOG_INFO << "Building pipeline '" << desc.name << "' from " << shader_dir.string();
    auto result = builder->build(desc);
    if (!result.ok()) {
        VNE_LOG_ERROR << "Build failed: " << result.error;
        return 1;
    }

    for (const auto& stage : result.artifact.stages) {
        VNE_LOG_INFO << "Stage " << static_cast<int>(stage.stage) << ": " << stage.reflection.bindings.size()
                     << " binding(s)";
        for (const auto& b : stage.reflection.bindings) {
            if (b.slots.metal) {
                VNE_LOG_INFO << "  " << b.name << " set=" << b.set << " binding=" << b.binding
                             << " metal={buf:" << b.slots.metal->buffer << ",tex:" << b.slots.metal->texture
                             << ",smp:" << b.slots.metal->sampler << "}";
            } else {
                VNE_LOG_INFO << "  " << b.name << " set=" << b.set << " binding=" << b.binding;
            }
        }
    }

    const std::filesystem::path out_dir = std::filesystem::current_path() / "output" / "triangle_bundle";
    std::filesystem::create_directories(out_dir);
    if (!vne::sc::writeShaderBundle(result.artifact, out_dir)) {
        VNE_LOG_ERROR << "Failed to write bundle";
        return 1;
    }
    VNE_LOG_INFO << "Bundle written to: " << out_dir.string();
    return 0;
}
