#pragma once

#include <types.h>

namespace EGG {

class MsgRes {
public:
    MsgRes(const void *bmgFile);
    virtual ~MsgRes();

    static void analyzeTag(unsigned short initialTag, const wchar_t *string, u8 *outArgsSize, unsigned long *outCmd, void **args);

    const wchar_t *getMsg(unsigned long group, unsigned long messageID);

    const void *getMsgEntry(unsigned long group, unsigned long messageID);
    u32 getMsgID(unsigned short);

private:
    void extractMsgHeader(const void *ptr);
    void extractMsgInfoDataBlk(const void *ptr);
    void extractMsgDataBlk(const void *ptr);
    void extractStrAttrDataBlk(const void *ptr);
    void extractMsgIDDataBlk(const void *ptr);
    void extractFlowChartInfoDataBlk(const void *ptr);
    void extractFlowLabelInfoDataBlk(const void *ptr);
    int analyzeDataBlkKind(unsigned long magic);

private:
    const u8 *bmg, *INF1, *DAT1, *STR1, *MID1, *FLW1, *FLI1;
};

} // namespace EGG