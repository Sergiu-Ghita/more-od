#include <cmath>
#include <iostream>
#include <opendavinci/odcore/wrapper/SerialPortFactory.h>
#include <automotivedata/GeneratedHeaders_AutomotiveData.h>
#include <opendavinci/odcore/base/Thread.h>
//#include "opendavinci/odcore/io/protocol/NetstringsProtocol.h"

#include <Proxy.h>

namespace automotive {
    namespace miniature {

        using namespace std;
        using namespace odcore::base;
        using namespace odcore::data;
        using namespace odcore::io::protocol;
        using namespace automotive;

        Proxy::Proxy(const int32_t &argc, char **argv) :
            DataTriggeredConferenceClientModule(argc, argv, "Proxy"),
            m_recorder(),
            m_camera(),
            m_serial(),
            m_nsp(),
            m_busy(false),
            m_counter(0)
        {}

        Proxy::~Proxy() {
        }

        void Proxy::setUp() {
            const string SERIAL_PORT = "/dev/ttyACM1";
            const uint32_t BAUD_RATE = 57600;
            m_busy = false;

            try {
                m_nsp = new NetstringsProtocol();
                m_nsp->setStringSender(this); // Who should receive encoded messages
                m_nsp->setStringListener(this);
            } catch(string &exception) {
                cerr << "Error while setting up NetstringProtocol: " << exception << endl;
            }

            try {
                m_serial = std::unique_ptr<odcore::wrapper::SerialPort>{odcore::wrapper::SerialPortFactory::createSerialPort(SERIAL_PORT, BAUD_RATE)};
                m_serial->setStringListener(m_nsp);
                m_serial->start();
            } catch(string &exception) {
                cerr << "Error while setting up serial port: " << exception << endl;
            }
        }

        void Proxy::tearDown() {
            cout << "Teardown!" << endl;
            if(m_nsp){
                m_nsp->setStringSender(NULL);
                m_nsp = NULL;
            }
            if(m_serial){
                m_serial->stop();
                m_serial = NULL;
            }
        }

        void Proxy::nextString(const string &string){
            cout << "Proxy::nextString " << string << endl;
            if(string.length() == 0){
                cout << "nothing" << endl;
            }
        }

        void Proxy::send(const std::string &s){
            if(m_serial){
                cout << "Proxy::send :"<< s << endl;
                m_serial->send(s);
            } else {
                cout << "Unable to send encoded string. Serial does not exist." << endl;   
            }
        }

        void Proxy::nextContainer(Container &c) {
            m_counter++;
            

            if (c.getDataType() == (automotive::miniature::VehicleControl::ID())) {
                cout << "nextContainer  " << m_counter << endl;
                cout << "Vehicle Control Received! " << endl;
            }
        }

    }
} // automotive::miniature