/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: Geodesy.cpp                                        */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "Geodesy.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"
#include "rapidjson/schema.h"
#include "rapidjson/writer.h"
#include "rapidjson/error/en.h"

using namespace std;
using namespace rapidjson;

//---------------------------------------------------------
// Constructor

Geodesy::Geodesy()
{
}

//---------------------------------------------------------
// Destructor

Geodesy::~Geodesy()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool Geodesy::OnNewMail(MOOSMSG_LIST &NewMail) {
    AppCastingMOOSApp::OnNewMail(NewMail);

    MOOSMSG_LIST::iterator p;
    for(p=NewMail.begin(); p!=NewMail.end(); p++) {
        CMOOSMsg &msg = *p;
        string key    = toupper(msg.GetKey());

         if (key == toupper(m_inputLatVar.c_str())) {
             m_lat = msg.GetDouble();
         } else if (key == toupper(m_inputLonVar.c_str())) {
             m_lon = msg.GetDouble();
         } else if (key == toupper(m_inputFixStatus.c_str())) {
             m_fixStatusValue = (uint8_t)msg.GetDouble();
             if (m_fixStatusValue > 1) {
                 m_fixValid = true;
             } else {m_fixValid = false;}
         } else if (key == toupper(m_rebaseTriggerVar.c_str())) {
             m_originValid = false;
         } else if (key != "APPCAST_REQ") // handled by AppCastingMOOSApp
            reportRunWarning("Unhandled Mail: " + key);
    }

    return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool Geodesy::OnConnectToServer() {
    registerVariables();
    return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool Geodesy::Iterate() {
    AppCastingMOOSApp::Iterate();
    if (m_fixValid && m_originValid) {
        if ((m_outputLocalNorthVar.length() > 0) && (m_outputLocalEastVar.length() > 0)) {
            if (m_geo.LatLong2LocalGrid(m_lat, m_lon, m_localNorth, m_localEast)) {
                Notify(m_outputLocalNorthVar.c_str(), m_localNorth);
                Notify(m_outputLocalEastVar.c_str(), m_localEast);
            }
        }
        if ((m_outputNorthingVar.length() > 0) && (m_outputEastingVar.length() > 0) && (m_outputUTMzone.length() > 0)) {
            if (m_geo.LatLong2LocalUTM(m_lat, m_lon, m_northing, m_easting)) {
                Notify(m_outputNorthingVar.c_str(), m_northing);
                Notify(m_outputEastingVar.c_str(), m_easting);
                Notify(m_outputUTMzone.c_str(), m_geo.GetUTMZone());
            }
        }
    }
    if (m_originValid) {
        if ((m_outputOriginLatVar.length() > 0) && (m_outputOriginLonVar.length() > 0)) {
            if (m_lastOriginTransmission == 0) {
                Notify(m_outputOriginLatVar.c_str(), m_originLat);
                Notify(m_outputOriginLonVar.c_str(), m_originLon);
            } else if (m_lastOriginTransmission > m_originOutputPeriod) {
                m_lastOriginTransmission = 0;
            } else {m_lastOriginTransmission++;}
        }
        if (m_fixValid && ((m_localNorth*m_localNorth + m_localEast*m_localEast) > (m_rebaseDistance*m_rebaseDistance))) {
            m_originValid = false;
        }
    } else {
        if (m_originRebaseCount > m_originRebasePeriod) {
            m_originLat = m_originLatAccumulator/m_originRebaseCount;
            m_originLon = m_originLonAccumulator/m_originRebaseCount;
            m_originValid = m_geo.Initialise(m_originLat, m_originLon);
            m_originRebaseCount = 0;
            m_lastOriginTransmission = 0;
        } else if (m_fixValid && inRange()) {
            m_originLatAccumulator += m_lat;
            m_originLonAccumulator += m_lon;
            m_originRebaseCount++;
        }
    }
    AppCastingMOOSApp::PostReport();
    return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool Geodesy::OnStartUp() {
    AppCastingMOOSApp::OnStartUp();

    STRING_LIST sParams;
    m_MissionReader.EnableVerbatimQuoting(false);
    if(!m_MissionReader.GetConfiguration(GetAppName(), sParams))
        reportConfigWarning("No config block found for " + GetAppName());

    STRING_LIST::iterator p;
    for (p=sParams.begin(); p!=sParams.end(); p++) {
        string orig  = *p;
        string line  = *p;
        string param = toupper(biteStringX(line, '='));
        string value = line;

        bool handled = false;
        if(param == "INPUTLAT") {
            m_inputLatVar = value;
            handled = true;
        } else if(param == "INPUTLON") {
            m_inputLonVar = value;
            handled = true;
        } else if(param == "INPUTFIXSTATUS") {
            m_inputFixStatus = value;
            m_fixValid = false;
            handled = true;
        } else if(param == "OUTPUTX") {
            m_outputLocalNorthVar = value;
            handled = true;
        } else if(param == "OUTPUTY") {
            m_outputLocalEastVar = value;
            handled = true;
        } else if(param == "OUTPUTEASTING") {
            m_outputEastingVar = value;
            handled = true;
        } else if(param == "OUTPUTNORTHING") {
            m_outputNorthingVar = value;
            handled = true;
        } else if(param == "OUTPUTUTMZONE") {
            m_outputUTMzone = value;
            handled = true;
        } else if(param == "OUTPUTORIGINPERIOD") {
            m_originOutputPeriod = atoi(value.c_str());
            handled = true;
        } else if(param == "OUTPUTORIGINLAT") {
            m_outputOriginLatVar = value;
            handled = true;
        } else if(param == "OUTPUTORIGINLON") {
            m_outputOriginLonVar = value;
            handled = true;
        } else if(param == "REBASECONF") {
            string json = value;
            Document conf;
            if (conf.Parse(value.c_str()).HasParseError()) {
                cerr << "JSON parse error " << GetParseError_En(conf.GetParseError());
                cerr << " in configuration JSON at offset " << conf.GetErrorOffset() << endl;
                std::abort();
            }
            if (conf.HasMember("rebase_condition") && conf["rebase_conditions"].IsString()) {
                string condition = conf["rebase_condition"].GetString();
                if ((condition == "distance") && conf.HasMember("rebase_distance") && conf["rebase_distance"].IsNumber()) {
                    m_rebaseDistance = conf["rebase_distance"].GetInt();
                } else if ((condition == "triggered") && conf.HasMember("rebase_trigger_var") && conf["rebase_trigger_var"].IsString()) {
                    m_rebaseTriggerVar = conf["rebase_trigger_var"].GetString();
                } else if ((condition == "both")&& conf.HasMember("rebase_distance") && conf["rebase_distance"].IsNumber() && conf.HasMember("rebase_trigger_var") && conf["rebase_trigger_var"].IsString()) {
                    m_rebaseDistance = conf["rebase_distance"].GetInt();
                    m_rebaseTriggerVar = conf["rebase_trigger_var"].GetString();
                }
            }
            handled = true;
        }

        if(!handled) reportUnhandledConfigWarning(orig);
    }

    if ((m_MissionReader.GetValue("LatOrigin", m_originLat)) && (m_MissionReader.GetValue("LonOrigin", m_originLon))) {
        m_originValid = m_geo.Initialise(m_originLat, m_originLon);
        m_lastOriginValid = m_originValid;
    }

    registerVariables();
    return(true);
}

//---------------------------------------------------------
// Procedure: registerVariables

void Geodesy::registerVariables() {
    AppCastingMOOSApp::RegisterVariables();
    if (m_inputLatVar.length() > 0) Register(m_inputLatVar);
    if (m_inputLonVar.length() > 0) Register(m_inputLonVar);
    if (m_inputFixStatus.length() > 0) Register(m_inputFixStatus);
    if (m_rebaseTriggerVar.length() > 0) Register(m_rebaseTriggerVar);
}


//------------------------------------------------------------
// Procedure: buildReport()

bool Geodesy::buildReport() {
    m_msgs << "============================================ \n";
    m_msgs << "Geodesy                                      \n";
    m_msgs << "============================================ \n";

    if (m_originValid) {
        ACTable actab(6);
        actab << "  | WGS84 | UTM (Local) | UTM (Global) | CMOOSGrid | Origin (WG84)";
        actab.addHeaderLines();
        actab << "North";
        actab << m_lat;
        actab << m_northing;
        actab << m_northing + m_geo.GetOriginNorthing();
        actab << m_localNorth;
        actab << m_originLat;
        actab << "East";
        actab << m_lon;
        actab << m_easting;
        actab << m_easting + m_geo.GetOriginEasting();
        actab << m_localEast;
        actab << m_originLon;
        actab << "Zone";
        actab << m_geo.GetUTMZone();
        actab << " ";
        actab << " ";
        actab << " ";
        actab << " ";
        m_msgs << actab.getFormattedString();
    } else {
        m_msgs << "*** Rebasing Origin *** \n";
        ACTable actab(5);
        actab << "Lat | Lon | Reboase Count | Lat Accumulator | Lon Accumulator";
        actab.addHeaderLines();
        actab << m_lat;
        actab << m_lon;
        actab << m_originRebaseCount;
        actab << m_originLatAccumulator;
        actab << m_originLonAccumulator;
        m_msgs << actab.getFormattedString();
    }
    return(true);
}
