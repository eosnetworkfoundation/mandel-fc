uint32_t to_uint32(const std::string& s) {
    size_t l = s.size();
    return (uint32_t)std::stoul(s.c_str(), &l, 16);
}

bytes to_bytes(const std::string& source) {
    bytes output(source.length()/2);
    fc::from_hex(source, output.data(), output.size());
    return output;
}
