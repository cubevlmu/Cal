#include "StringBuilder.hpp"

namespace cal {

    StringBuilder::StringBuilder() = default;
    StringBuilder::StringBuilder(const std::string& str) { m_ss << str; }

    void StringBuilder::append(const std::string& i) { m_ss << i; }
    void StringBuilder::append(const char* text) { m_ss << text; }
    void StringBuilder::append(i32 i) { m_ss << i; }
    void StringBuilder::append(u32 i) { m_ss << i; }
    void StringBuilder::append(i64 i) { m_ss << i; }
    void StringBuilder::append(u64 i) { m_ss << i; }
    void StringBuilder::append(i16 i) { m_ss << i; }
    void StringBuilder::append(u16 i) { m_ss << i; }
    void StringBuilder::append(i8 i) { m_ss << i; }
    void StringBuilder::append(u8 i) { m_ss << i; }
    void StringBuilder::append(bool i) { m_ss << i; }
    void StringBuilder::append(float i) { m_ss << i; }
    void StringBuilder::append(double i) { m_ss << i; }
    void StringBuilder::append(const StringBuilder& builder) { m_ss << builder.m_ss.str(); }
    void StringBuilder::append(const std::stringstream& ss) { m_ss << ss.str(); }
    
    void StringBuilder::clear() { m_ss.clear(); }
} // namespace cal