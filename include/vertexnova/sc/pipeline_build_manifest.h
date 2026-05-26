/**
 * @file pipeline_build_manifest.h
 * @brief JSON manifest describing an offline shader pipeline build job.
 *
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#pragma once

#include "shader_pipeline_builder.h"

#include <optional>
#include <string>
#include <vector>

namespace vne::sc {

/**
 * @brief One stage entry in a pipeline build manifest file.
 */
struct ManifestStageDesc {
    ShaderStage stage = ShaderStage::eVertex;
    std::string file;
    std::string entry = "main";
};

/**
 * @brief Parsed pipeline build manifest (typically loaded from JSON).
 */
struct PipelineBuildManifest {
    std::string name;
    SourceLang source_lang = SourceLang::eGLSL;
    std::vector<ManifestStageDesc> stages;
    std::vector<CrossTarget> targets;
    bool validate = true;
    std::vector<std::string> errors;

    /// Converts this manifest into a @ref PipelineBuildDesc for @ref IShaderPipelineBuilder.
    PipelineBuildDesc toBuildDesc() const;
};

/**
 * @brief Parses a pipeline build manifest from a JSON string.
 * @returns Parsed manifest, or @c std::nullopt on fatal parse errors.
 */
std::optional<PipelineBuildManifest> parsePipelineBuildManifestJson(const std::string& json);

/**
 * @brief Loads a pipeline build manifest from a file path.
 */
std::optional<PipelineBuildManifest> loadPipelineBuildManifestFile(const std::string& path);

}  // namespace vne::sc
