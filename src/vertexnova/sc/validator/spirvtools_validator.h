#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   May 2026
 *
 * Autodoc:   yes
 * ----------------------------------------------------------------------
 */

#include "vertexnova/sc/shader_validator.h"

namespace vne::sc {

/**
 * @brief Validates SPIR-V binaries against the Vulkan rules using SPIRV-Tools.
 */
class SpirvToolsValidator final : public IShaderValidator {
   public:
    bool isAvailable() const noexcept override;
    ValidationResult validate(const std::vector<uint32_t>& spirv) override;
};

}  // namespace vne::sc
