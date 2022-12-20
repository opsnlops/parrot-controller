
#pragma once

/**
 * An abstraction of a method for the servo controller to intake
 * data from outside sources.
 */
class IOHandler {

public:

    /**
     * A chance for the IOHandler to set itself up
     *
     * @return 1 if everything was good
     */
    virtual int init() = 0;

    /**
     * The number of messages this IOHandler has processed
     *
     * @return the number of messages
     */
    virtual uint32_t getMessagesProcessed() = 0;

};