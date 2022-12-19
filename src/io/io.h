
#pragma once


class IO {

public:
    virtual int init() = 0;
    virtual uint32_t getMessagesProcessed() = 0;
};