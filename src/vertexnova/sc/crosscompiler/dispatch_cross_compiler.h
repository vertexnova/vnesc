/**
 * @file dispatch_cross_compiler.h
 * @brief Routes SPIR-V cross-compilation to SPIRV-Cross (MSL/GLSL) or Tint (WGSL).
 *
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#pragma once

#include "vertexnova/sc/shader_cross_compiler.h"

#include <memory>

namespace vne::sc {

/**
 * @brief Delegates @ref CrossTarget::eWGSL to @ref TintCrossCompiler; all other targets to SPIRV-Cross.
 */
class DispatchCrossCompiler final : public IShaderCrossCompiler {
   public:
    DispatchCrossCompiler(std::shared_ptr<IShaderCrossCompiler> spirv_cross,
                          std::shared_ptr<IShaderCrossCompiler> tint);

    bool isAvailable() const noexcept override;
    CrossCompileResult crossCompile(const CrossCompileRequest& req) override;

   private:
    std::shared_ptr<IShaderCrossCompiler> spirv_cross_;
    std::shared_ptr<IShaderCrossCompiler> tint_;
};

}  // namespace vne::sc
