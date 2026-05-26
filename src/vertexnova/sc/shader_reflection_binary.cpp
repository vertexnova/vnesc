/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * ----------------------------------------------------------------------
 */

#include "vertexnova/sc/shader_reflection_binary.h"

#include <sstream>
#include <stdexcept>

namespace {

constexpr uint32_t kMaxStringLen = 4096;
constexpr uint32_t kMaxProgramStageCount = 16;
constexpr uint32_t kMaxBindingCount = 256;
constexpr uint32_t kMaxStructMemberCount = 256;

struct Writer {
    std::ostringstream os{std::ios::binary};
    void u8(uint8_t v) { os.write(reinterpret_cast<const char*>(&v), 1); }
    void u32(uint32_t v) { os.write(reinterpret_cast<const char*>(&v), 4); }
    void boolean(bool v) { u8(static_cast<uint8_t>(v ? 1 : 0)); }
    void str(const std::string& s) {
        u32(static_cast<uint32_t>(s.size()));
        os.write(s.data(), static_cast<std::streamsize>(s.size()));
    }
};

struct Reader {
    std::istringstream is;
    const size_t data_size_;

    explicit Reader(const std::string& data)
        : is(data, std::ios::binary), data_size_(data.size()) {}

    size_t remaining() {
        const auto pos = is.tellg();
        if (pos < 0) {
            return 0;
        }
        return data_size_ - static_cast<size_t>(pos);
    }

    void require_bytes(size_t n) {
        if (remaining() < n) {
            throw std::runtime_error("truncated reflection binary");
        }
    }

    uint8_t u8() {
        require_bytes(1);
        uint8_t v{};
        is.read(reinterpret_cast<char*>(&v), 1);
        return v;
    }
    uint32_t u32() {
        require_bytes(4);
        uint32_t v{};
        is.read(reinterpret_cast<char*>(&v), 4);
        return v;
    }
    uint32_t bounded_count(uint32_t max_count) {
        const uint32_t count = u32();
        if (count > max_count) {
            throw std::runtime_error("reflection count exceeds maximum");
        }
        return count;
    }
    bool boolean() { return u8() != 0u; }
    std::string str() {
        const uint32_t len = u32();
        if (len > kMaxStringLen) {
            throw std::runtime_error("reflection string length exceeds maximum");
        }
        require_bytes(len);
        std::string s(len, '\0');
        if (len > 0) {
            is.read(s.data(), static_cast<std::streamsize>(len));
        }
        return s;
    }
    bool ok() const { return is.good() || is.eof(); }
};

void writeReflectedStructMember(Writer& w, const vne::sc::ReflectedStructMember& m) {
    w.str(m.name);
    w.u32(m.offset);
    w.u32(m.size);
    w.u32(m.array_count);
    w.u32(m.array_stride);
    w.boolean(m.is_matrix);
    w.u32(m.matrix_columns);
    w.u32(m.matrix_rows);
    w.str(m.type_name);
}

void readReflectedStructMember(Reader& r, vne::sc::ReflectedStructMember& m) {
    m.name = r.str();
    m.offset = r.u32();
    m.size = r.u32();
    m.array_count = r.u32();
    m.array_stride = r.u32();
    m.is_matrix = r.boolean();
    m.matrix_columns = r.u32();
    m.matrix_rows = r.u32();
    m.type_name = r.str();
}

void writeBackendSlot(Writer& w, const vne::sc::BackendSlot& s) {
    w.u32(s.metal_buffer_index);
    w.u32(s.metal_texture_index);
    w.u32(s.metal_sampler_index);
    w.u32(s.wgpu_group);
    w.u32(s.wgpu_binding);
    w.boolean(s.populated);
}

void readBackendSlot(Reader& r, vne::sc::BackendSlot& s) {
    s.metal_buffer_index = r.u32();
    s.metal_texture_index = r.u32();
    s.metal_sampler_index = r.u32();
    s.wgpu_group = r.u32();
    s.wgpu_binding = r.u32();
    s.populated = r.boolean();
}

void writeReflectedBindingInfo(Writer& w, const vne::sc::ReflectedBindingInfo& b) {
    w.str(b.name);
    w.u8(static_cast<uint8_t>(b.type));
    w.u32(b.set);
    w.u32(b.binding);
    w.u32(b.array_size);
    w.u32(static_cast<uint32_t>(b.stages));
    writeBackendSlot(w, b.backend_slot);
    w.u32(static_cast<uint32_t>(b.struct_members.size()));
    for (const auto& m : b.struct_members) {
        writeReflectedStructMember(w, m);
    }
}

void readReflectedBindingInfo(Reader& r, vne::sc::ReflectedBindingInfo& b) {
    b.name = r.str();
    b.type = static_cast<vne::sc::ReflectedResourceType>(r.u8());
    b.set = r.u32();
    b.binding = r.u32();
    b.array_size = r.u32();
    b.stages = static_cast<vne::sc::ShaderStageFlags>(r.u32());
    readBackendSlot(r, b.backend_slot);
    const uint32_t member_count = r.bounded_count(kMaxStructMemberCount);
    b.struct_members.resize(member_count);
    for (auto& m : b.struct_members) {
        readReflectedStructMember(r, m);
    }
}

void writeStageReflection(Writer& w, const vne::sc::StageReflection& sr) {
    w.u8(static_cast<uint8_t>(sr.stage));
    w.u32(static_cast<uint32_t>(sr.bindings.size()));
    for (const auto& b : sr.bindings) {
        writeReflectedBindingInfo(w, b);
    }
    w.u32(sr.push_constant_size);
    w.u32(sr.workgroup_size.x);
    w.u32(sr.workgroup_size.y);
    w.u32(sr.workgroup_size.z);
}

void readStageReflection(Reader& r, vne::sc::StageReflection& sr) {
    sr.stage = static_cast<vne::sc::ShaderStage>(r.u8());
    const uint32_t binding_count = r.bounded_count(kMaxBindingCount);
    sr.bindings.resize(binding_count);
    for (auto& b : sr.bindings) {
        readReflectedBindingInfo(r, b);
    }
    sr.push_constant_size = r.u32();
    sr.workgroup_size.x = r.u32();
    sr.workgroup_size.y = r.u32();
    sr.workgroup_size.z = r.u32();
}

}  // namespace

namespace vne::sc {

std::string serializeStageReflection(const StageReflection& reflection) {
    Writer w;
    writeStageReflection(w, reflection);
    return w.os.str();
}

bool deserializeStageReflection(const std::string& data, StageReflection& out) {
    try {
        Reader r(data);
        readStageReflection(r, out);
        return r.ok();
    } catch (...) {
        return false;
    }
}

std::string serializeProgramReflection(const ProgramReflection& reflection) {
    Writer w;
    w.u32(static_cast<uint32_t>(reflection.stages.size()));
    for (const auto& stage : reflection.stages) {
        writeStageReflection(w, stage);
    }
    return w.os.str();
}

bool deserializeProgramReflection(const std::string& data, ProgramReflection& out) {
    try {
        Reader r(data);
        const uint32_t count = r.bounded_count(kMaxProgramStageCount);
        out.stages.resize(count);
        for (auto& stage : out.stages) {
            readStageReflection(r, stage);
        }
        return r.ok();
    } catch (...) {
        return false;
    }
}

}  // namespace vne::sc
