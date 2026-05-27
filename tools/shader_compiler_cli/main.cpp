/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * vnesc_shader_compiler — offline shader compilation CLI
 *
 * Usage:
 *   vnesc_shader_compiler --manifest <path> --output <bundle_dir> [--cache <dir>]
 *
 * Reads:  <path> — JSON file parseable by PipelineBuildManifest
 * Writes: <bundle_dir>.vneshader/ directory
 * ----------------------------------------------------------------------
 */

#include "vertexnova/sc/pipeline_build_manifest.h"
#include "vertexnova/sc/shader_bundle.h"
#include "vertexnova/sc/shader_compiler_factory.h"
#include "vertexnova/sc/shader_pipeline_builder.h"

#include <filesystem>
#include <iostream>
#include <string>

namespace {

void printUsage(const char* argv0) {
    std::cerr << "Usage: " << argv0 << " --manifest <path> --output <bundle_dir> [--cache <dir>]\n"
              << "\n"
              << "  --manifest <path>    JSON manifest describing the pipeline build job\n"
              << "  --output  <dir>      Output .vneshader bundle directory\n"
              << "  --cache   <dir>      Optional shader artifact cache directory\n";
}

}  // namespace

int main(int argc, char** argv) {
    std::string manifest_path;
    std::string output_dir;
    std::string cache_dir;

    for (int i = 1; i < argc; ++i) {
        const std::string arg(argv[i]);
        if ((arg == "--manifest" || arg == "-m") && i + 1 < argc) {
            manifest_path = argv[++i];
        } else if ((arg == "--output" || arg == "-o") && i + 1 < argc) {
            output_dir = argv[++i];
        } else if ((arg == "--cache" || arg == "-c") && i + 1 < argc) {
            cache_dir = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }

    if (manifest_path.empty()) {
        std::cerr << "Error: --manifest is required\n";
        printUsage(argv[0]);
        return 1;
    }
    if (output_dir.empty()) {
        std::cerr << "Error: --output is required\n";
        printUsage(argv[0]);
        return 1;
    }

    // ── Load manifest ─────────────────────────────────────────────────────────
    auto manifest = vne::sc::loadPipelineBuildManifestFile(manifest_path);
    if (!manifest.has_value()) {
        std::cerr << "Error: failed to parse manifest: " << manifest_path << "\n";
        return 1;
    }
    if (!manifest->errors.empty()) {
        for (const auto& e : manifest->errors) {
            std::cerr << "Manifest error: " << e << "\n";
        }
        return 1;
    }

    // ── Build ─────────────────────────────────────────────────────────────────
    auto builder = vne::sc::ShaderCompilerFactory::createPipelineBuilder(manifest->source_lang);
    if (!builder) {
        std::cerr << "Error: no pipeline builder available for the requested source language\n";
        return 1;
    }

    vne::sc::PipelineBuildDesc desc = manifest->toBuildDesc();
    if (!cache_dir.empty()) {
        desc.use_cache = true;
        desc.cache_dir = cache_dir;
    }

    auto result = builder->build(desc);
    if (!result.ok()) {
        std::cerr << "Error: shader build failed:\n" << result.error << "\n";
        return 1;
    }

    // ── Write bundle ──────────────────────────────────────────────────────────
    const std::filesystem::path bundle_path(output_dir);
    if (!vne::sc::writeShaderBundle(result.artifact, bundle_path)) {
        std::cerr << "Error: failed to write bundle to " << output_dir << "\n";
        return 1;
    }

    std::cout << "Bundle written to: " << bundle_path << "\n";
    std::cout << "  Stages: " << result.artifact.stages.size() << "\n";
    for (const auto& stage : result.artifact.stages) {
        std::cout << "  - stage " << static_cast<int>(stage.stage)
                  << " | spirv=" << stage.spirv.size() << " words"
                  << " | cross=" << stage.cross_compiled.size() << " target(s)\n";
    }
    return 0;
}
