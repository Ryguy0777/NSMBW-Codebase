#pragma once

namespace EGG {

class StreamDecomp {
public:
    virtual bool init(void *dst, u32 maxCompressedSize) = 0;
    virtual bool decomp(const void *src, u32 size) = 0;
    virtual u32 getHeaderSize() = 0;
    virtual u32 getUncompressedSize(const void *src) = 0;
};

class StreamDecompLZ : public StreamDecomp {
public:
    virtual bool init(void *dst, u32 maxCompressedSize) override;
    virtual bool decomp(const void *src, u32 size) override;
    virtual u32 getHeaderSize() override;
    virtual u32 getUncompressedSize(const void *src) override;

private:
    u8 mPad[0x20];
};

class StreamDecompRL : public StreamDecomp {
public:
    virtual bool init(void *dst, u32 maxCompressedSize) override;
    virtual bool decomp(const void *src, u32 size) override;
    virtual u32 getHeaderSize() override;
    virtual u32 getUncompressedSize(const void *src) override;

private:
    u8 mPad[0x18];
};

class StreamDecompLH : public StreamDecomp {
public:
    virtual bool init(void *dst, u32 maxCompressedSize) override;
    virtual bool decomp(const void *src, u32 size) override;
    virtual u32 getHeaderSize() override;
    virtual u32 getUncompressedSize(const void *src) override;

private:
    u8 mPad[0x8B0];
};

class StreamDecompLRC : public StreamDecomp {
public:
    virtual bool init(void *dst, u32 maxCompressedSize) override;
    virtual bool decomp(const void *src, u32 size) override;
    virtual u32 getHeaderSize() override;
    virtual u32 getUncompressedSize(const void *src) override;

private:
    u8 mPad[0x9034];
};

class StreamDecompSZS : public StreamDecomp {
public:
    virtual bool init(void *dst, u32 maxCompressedSize) override;
    virtual bool decomp(const void *src, u32 size) override;
    virtual u32 getHeaderSize() override;
    virtual u32 getUncompressedSize(const void *src) override;

private:
    u8 mPad[0x1C];
};

} // namespace EGG