/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: Geodesy.h                                          */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#ifndef Geodesy_HEADER
#define Geodesy_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "MOOS/libMOOSGeodesy/MOOSGeodesy.h"
#include <string>
#include <cmath>

class Geodesy : public AppCastingMOOSApp {
    public:
        Geodesy();
        ~Geodesy();

    protected: // Standard MOOSApp functions to overload
        bool OnNewMail(MOOSMSG_LIST &NewMail);
        bool Iterate();
        bool OnConnectToServer();
        bool OnStartUp();

    protected: // Standard AppCastingMOOSApp function to overload
        bool buildReport();
        void registerVariables();

    protected:
        bool inRange () {
            bool result = true;
            result &= (m_lat <= 90.0);
            result &= (m_lat >= -90.0);
            result &= (m_lon <= 180.0);
            result &= (m_lon >= -180.0);
            return result;
        }

    private: // Configuration variables
        std::string     m_inputLatVar;
        std::string     m_inputLonVar;
        std::string     m_inputFixStatus;
        std::string     m_outputLocalNorthVar;
        std::string     m_outputLocalEastVar;
        std::string     m_outputNorthingVar;
        std::string     m_outputEastingVar;
        std::string     m_outputUTMzone;
        std::string     m_outputOriginLatVar;
        std::string     m_outputOriginLonVar;
        std::string     m_rebaseTriggerVar;
        std::string     m_rebaseNotifyVar;
        uint64_t        m_rebaseDistance            = 0xffffffff;
        uint16_t        m_originOutputPeriod        = 10;
        uint16_t        m_originRebasePeriod        = 10;

    private: // State variables
        CMOOSGeodesy    m_geo;
        bool            m_fixValid                  = false;
        bool            m_originValid               = false;
        bool            m_lastOriginValid           = false;
        bool            m_newData                   = false;
        double          m_lat                       = NAN;
        double          m_lon                       = NAN;
        double          m_localNorth                = NAN;
        double          m_localEast                 = NAN;
        double          m_northing                  = NAN;
        double          m_easting                   = NAN;
        double          m_originLat                 = NAN;
        double          m_originLon                 = NAN;
        uint8_t         m_fixStatusValue            = 0;
        uint16_t        m_lastOriginTransmission    = 0;
        uint16_t        m_originRebaseCount         = 0;
        double          m_originLatAccumulator      = 0.0;
        double          m_originLonAccumulator      = 0.0;
};

#endif
