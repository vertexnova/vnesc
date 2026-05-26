/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * ----------------------------------------------------------------------
 */

#include "vertexnova/sc/pipeline_build_manifest.h"

#include "vertexnova/logging/logging.h"

#include <fstream>
#include <sstream>

#ifdef VNE_SC_JSON_ENABLED
#include <nlohmann/json.hpp>
#endif

CREATE_VNE_LOGGER_CATEGORY("vne.sc.manifest")

namespace vne::sc {

namespace {

std::optional<ShaderStage> parseStageString(const std::string& s) {
    if (s == "vertex")
        return ShaderStage::eVertex;
    if (s == "fragment")
        return ShaderStage::eFragment;
    if (s == "compute")
        return ShaderStage::eCompute;
    if (s == "geometry")
        return ShaderStage::eGeometry;
    if (s == "tessellation_control" || s == "tess_control")
        return ShaderStage::eTessellationControl;
    if (s == "tessellation_evaluation" || s == "tess_eval")
        return ShaderStage::eTessellationEvaluation;
    return std::nullopt;
}

std::optional<CrossTarget> parseTargetString(const std::string& s) {
    if (s == "MSL" || s == "msl")
        return CrossTarget::eMSL;
    if (s == "WGSL" || s == "wgsl")
        return CrossTarget::eWGSL;
    if (s == "GLSL" || s == "glsl")
        return CrossTarget::eGLSL;
    if (s == "GLSLES" || s == "glsles")
        return CrossTarget::eGLSLES;
    if (s == "HLSL" || s == "hlsl")
        return CrossTarget::eHLSL;
    return std::nullopt;
}

std::optional<SourceLang> parseSourceLangString(const std::string& s) {
    if (s == "GLSL" || s == "glsl")
        return SourceLang::eGLSL;
    if (s == "HLSL" || s == "hlsl")
        return SourceLang::eHLSL;
    if (s == "MSL" || s == "msl")
        return SourceLang::eMSL;
    if (s == "WGSL" || s == "wgsl")
        return SourceLang::eWGSL;
    if (s == "Slang" || s == "slang")
        return SourceLang::eSlang;
    return std::nullopt;
}

}  // namespace

PipelineBuildDesc PipelineBuildManifest::toBuildDesc() const {
    PipelineBuildDesc desc;
    desc.name     = name;
    desc.validate = validate;
    desc.targets  = targets;
    for (const auto& ms : stages) {
        CompileRequest req;
        req.file_path   = ms.file;
        req.entry_point = ms.entry;
        req.stage       = ms.stage;
        req.lang        = source_lang;
        desc.stages.push_back(std::move(req));
    }
    return desc;
}

std::optional<PipelineBuildManifest> parsePipelineBuildManifestJson(const std::string& json) {
#ifndef VNE_SC_JSON_ENABLED
    (void)json;
    VNE_LOG_ERROR << "parsePipelineBuildManifestJson: JSON support not compiled in";
    return std::nullopt;
#else
    PipelineBuildManifest manifest;
    try {
        const auto doc = nlohmann::json::parse(json);
        if (!doc.contains("name") || !doc["name"].is_string()) {
            manifest.errors.push_back("missing or invalid 'name'");
            return std::nullopt;
        }
        manifest.name = doc["name"].get<std::string>();

        if (doc.contains("source_lang") && doc["source_lang"].is_string()) {
            if (auto lang = parseSourceLangString(doc["source_lang"].get<std::string>())) {
                manifest.source_lang = *lang;
            } else {
                manifest.errors.push_back("unknown source_lang");
            }
        }

        if (doc.contains("validate") && doc["validate"].is_boolean()) {
            manifest.validate = doc["validate"].get<bool>();
        }

        if (doc.contains("targets") && doc["targets"].is_array()) {
            for (const auto& t : doc["targets"]) {
                if (!t.is_string()) {
                    continue;
                }
                if (auto target = parseTargetString(t.get<std::string>())) {
                    manifest.targets.push_back(*target);
                } else {
                    manifest.errors.push_back("unknown target: " + t.get<std::string>());
                }
            }
        }

        if (!doc.contains("stages") || !doc["stages"].is_array()) {
            manifest.errors.push_back("missing or invalid 'stages' array");
            return std::nullopt;
        }

        for (const auto& stage_entry : doc["stages"]) {
            ManifestStageDesc ms;
            if (!stage_entry.contains("stage") || !stage_entry["stage"].is_string()) {
                manifest.errors.push_back("stage entry missing 'stage'");
                continue;
            }
            if (!stage_entry.contains("file") || !stage_entry["file"].is_string()) {
                manifest.errors.push_back("stage entry missing 'file'");
                continue;
            }
            auto stage = parseStageString(stage_entry["stage"].get<std::string>());
            if (!stage) {
                manifest.errors.push_back("unknown stage: " + stage_entry["stage"].get<std::string>());
                continue;
            }
            ms.stage = *stage;
            ms.file  = stage_entry["file"].get<std::string>();
            if (stage_entry.contains("entry") && stage_entry["entry"].is_string()) {
                ms.entry = stage_entry["entry"].get<std::string>();
            }
            manifest.stages.push_back(std::move(ms));
        }

        if (manifest.stages.empty()) {
            manifest.errors.push_back("no valid stages");
            return std::nullopt;
        }

        return manifest;
    } catch (const std::exception& ex) {
        VNE_LOG_ERROR << "parsePipelineBuildManifestJson: " << ex.what();
        return std::nullopt;
    }
#endif
}

std::optional<PipelineBuildManifest> loadPipelineBuildManifestFile(const std::string& path) {
    std::ifstream file{path};
    if (!file.is_open()) {
        VNE_LOG_ERROR << "loadPipelineBuildManifestFile: cannot open '" << path << "'";
        return std::nullopt;
    }
    std::ostringstream buf;
    buf << file.rdbuf();
    return parsePipelineBuildManifestJson(buf.str());
}

}  // namespace vne::sc
