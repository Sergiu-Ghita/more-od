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
                m_nsp = new CustomNetstringsProtocol();
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
            cout << "Proxy::teardown start!" << endl;
            if(m_serial){
                cout << "if m_serial start" << endl;
                m_serial->stop();
                m_serial = NULL;
                cout << "if m_serial end" << endl;
            }
            if(m_nsp){
                cout << "if m_nsp start" << endl;
                m_nsp->setStringSender(NULL);
                m_nsp = NULL;
                cout << "if m_nsp end" << endl;
            }

            cout << "Proxy::teardown end!" << endl;
        }

        void Proxy::nextString(const string &string){
            // cout << m_counter << endl;
            // m_counter++;
            cout << "Proxy::nextString start: " << string << endl;
            if(string.length() == 0){
                cout << "nothing" << endl;
            }
            cout << "Proxy::nextString end." << endl;
        }

        void Proxy::send(const std::string &s){
            cout << "Proxy::send start: " << s << endl;
            if(m_serial){
                cout << "if m_serial start" << endl;
                m_serial->send(s);
                cout << "if m_serial end" << endl;
            } else {
                cout << "else no m_serial." << endl;   
            }
            cout << "Proxy::send end!" << endl;
        }

        void Proxy::nextContainer(Container &c) {
            cout << "Proxy::nextContainer start" << endl;
            if (c.getDataType() == (automotive::miniature::VehicleControl::ID())) {
                m_counter++;
                cout << "nextContainer  " << m_counter << endl;
                cout << "Vehicle Control Received! " << endl;
            }
            cout << "Proxy::nextContainer end" << endl;
        }
    }
} // automotive::miniature