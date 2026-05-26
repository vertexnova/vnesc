/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * Autodoc:   yes
 * ----------------------------------------------------------------------
 */

#include <gtest/gtest.h>

#include "vertexnova/sc/vnesc.h"

namespace vne::sc {

class PipelineBuildManifestTest : public ::testing::Test {};

TEST_F(PipelineBuildManifestTest, ParsesBasicJson) {
    const char* json = R"({
        "name": "scene",
        "source_lang": "GLSL",
        "targets": ["MSL"],
        "stages": [
            { "stage": "vertex", "file": "scene.vert", "entry": "main" }
        ]
    })";
#ifdef VNE_SC_JSON_ENABLED
    auto manifest = parsePipelineBuildManifestJson(json);
    ASSERT_TRUE(manifest.has_value());
    EXPECT_EQ(manifest->name, "scene");
    EXPECT_EQ(manifest->stages.size(), 1u);
    EXPECT_EQ(manifest->targets.size(), 1u);
#else
    (void)json;
    GTEST_SKIP() << "JSON support not enabled";
#endif
}

}  // namespace vne::sc
