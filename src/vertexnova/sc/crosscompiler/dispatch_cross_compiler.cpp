/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * ----------------------------------------------------------------------
 */

#include "dispatch_cross_compiler.h"

namespace vne::sc {

DispatchCrossCompiler::DispatchCrossCompiler(std::shared_ptr<IShaderCrossCompiler> spirv_cross,
                                             std::shared_ptr<IShaderCrossCompiler> tint)
    : spirv_cross_(std::move(spirv_cross)), tint_(std::move(tint)) {}

bool DispatchCrossCompiler::isAvailable() const noexcept {
    return spirv_cross_ && spirv_cross_->isAvailable();
}

CrossCompileResult DispatchCrossCompiler::crossCompile(const CrossCompileRequest& req) {
    if (req.target == CrossTarget::eWGSL) {
        if (!tint_ || !tint_->isAvailable()) {
            CrossCompileResult result;
            result.code  = ResultCode::eUnavailable;
            result.error = "DispatchCrossCompiler: WGSL requires Tint (build with -DVNE_SC_TINT=ON)";
            return result;
        }
        return tint_->crossCompile(req);
    }
    if (!spirv_cross_) {
        CrossCompileResult result;
        result.code  = ResultCode::eUnavailable;
        result.error = "DispatchCrossCompiler: SPIRV-Cross backend not available";
        return result;
    }
    return spirv_cross_->crossCompile(req);
}

}  // namespace vne::sc
