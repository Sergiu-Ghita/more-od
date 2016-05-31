#ifndef OPENDAVINCI_CORE_IO_PROTOCOL_CustomNetstringsProtocol_H_
#define OPENDAVINCI_CORE_IO_PROTOCOL_CustomNetstringsProtocol_H_

#include <sstream>
#include <string>

#include "opendavinci/odcore/opendavinci.h"
#include "opendavinci/odcore/base/Mutex.h"
#include "opendavinci/odcore/io/StringObserver.h"
#include "opendavinci/odcore/io/protocol/AbstractProtocol.h"

using namespace std;

class CustomNetstringsProtocol : public odcore::io::StringObserver, public odcore::io::protocol::AbstractProtocol {
    private:
        /**
         * "Forbidden" copy constructor. Goal: The compiler should warn
         * already at compile time for unwanted bugs caused by any misuse
         * of the copy constructor.
         */
        CustomNetstringsProtocol(const CustomNetstringsProtocol &);

        /**
         * "Forbidden" assignment operator. Goal: The compiler should warn
         * already at compile time for unwanted bugs caused by any misuse
         * of the assignment operator.
         */
        CustomNetstringsProtocol& operator=(const CustomNetstringsProtocol &);

    public:
        /**
         * Constructor.
         */
        CustomNetstringsProtocol();

        virtual ~CustomNetstringsProtocol();

        /**
         * This method sends the data in the following format:
         *
         * Size : Data ,
         *
         * Size: data.length() as string
         *
         * @param data Data to be sent.
         */
        void send(const string& data);

        /**
         * This method sets the StringListener that will receive
         * incoming data.
         *
         * @param listener StringListener that will receive incoming data.
         */
        void setStringListener(StringListener *listener);

        virtual void nextString(const string &s);

    private:
        void decodeNetstring();

        /**
         * This method is used to pass received data thread-safe
         * to the registered StringListener.
         */
        void invokeStringListener(const string& data);

        odcore::base::Mutex m_stringListenerMutex;
        StringListener *m_stringListener;

        odcore::base::Mutex m_partialDataMutex;
        stringstream m_partialData;
};

#endif /* OPENDAVINCI_CORE_IO_PROTOCOL_CustomNetstringsProtocol_H_ */
