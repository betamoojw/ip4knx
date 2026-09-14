#pragma once
#include <functional>

namespace TPUart
{
    namespace Interface
    {
        class Abstract
        {
          protected:
            bool _running = false;

          public:
            virtual void flush() = 0;
            virtual void begin(int baud) = 0;
            virtual void end() = 0;
            virtual bool available() = 0;
            virtual bool availableForWrite() = 0;
            virtual bool write(char value) = 0;
            virtual int read() = 0;
            virtual bool overflow() { return false; };
            // Line-integrity events reported by the driver. The KNX UART runs
            // with even parity, so a single flipped bit is detectable — but the
            // byte is still handed to the application, which has no way of its
            // own to tell it apart from good data. Interfaces that cannot report
            // this return 0.
            virtual unsigned int parityErrors() { return 0; }
            virtual unsigned int frameErrors() { return 0; }
            virtual bool hasCallback() { return false; }
            virtual void registerCallback(std::function<bool()> callback) {}
        };
    } // namespace Interface
} // namespace TPUart