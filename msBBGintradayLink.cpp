// msBBGintradayLink.cpp : Defines the entry point for the console application.
//
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "wstp.h"
#include <blpapi_session.h>
#include <blpapi_eventdispatcher.h>
#include <blpapi_event.h>
#include <blpapi_message.h>
#include <blpapi_element.h>
#include <blpapi_name.h>
#include <blpapi_request.h>
#include <blpapi_subscriptionlist.h>
#include <blpapi_defs.h>
#include <blpapi_exception.h>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern void msBBGintradayLink(const char* Ticker,const char* Field,const char* StartTime, const char* EndTime, int dpdf, int debug);

using namespace std;
using namespace BloombergLP;
using namespace blpapi;

namespace {
    const Name TICK_DATA("tickData");
    const Name COND_CODE("conditionCodes");
    const Name TICK_SIZE("size");
    const Name TIME("time");
    const Name TYPE("type");
    const Name VALUE("value");
    const Name RESPONSE_ERROR("responseError");
    const Name CATEGORY("category");
    const Name MESSAGE("message");
    const Name SESSION_TERMINATED("SessionTerminated");
	const Name SESSION_STARTUP_FAILURE("SessionStartupFailure");
};

class Intraday
{
	std::string         d_host;
    int                 d_port;

    void printUsage()
    {
		std::cout << "Usage:" << std::endl
			<< "	Retrieve reference data " << std::endl
			<< "		[-ip 		<ipAddress	= localhost>" << std::endl
			<< "		[-p 		<tcpPort	= 8194>" << std::endl;
		return;
    }

	void printErrorInfo(const char *leadingStr, const Element &errorInfo)
    {
        std::cout << leadingStr
            << errorInfo.getElementAsString(CATEGORY)
            << " ("
            << errorInfo.getElementAsString(MESSAGE)
            << ")" << std::endl;
    }

	string processEvent(Event &event)
    {
        string output = "Failed";
		MessageIterator msgIter(event);
        while (msgIter.next()) {
            Message msg = msgIter.message();
            if (msg.hasElement(RESPONSE_ERROR)) {
                printErrorInfo("REQUEST FAILED: ", 
                    msg.getElement(RESPONSE_ERROR));
                continue;
            }
            output = processMessage(msg);
        }
		return output;
    }

	string processMessage(Message &msg)
    {
		string output;
        Element data = msg.getElement(TICK_DATA).getElement(TICK_DATA);
        int numItems = data.numValues();
        std::cout << "TIME\t\t\t\tTYPE\tVALUE\t\tSIZE\tCC" << std::endl;
        std::cout << "----\t\t\t\t----\t-----\t\t----\t--" << std::endl;
        std::string cc;
        std::string type;
		output = "{";
        for (int i = 0; i < numItems; ++i) {
            Element item = data.getValueAsElement(i);
            Datetime time = item.getElementAsDatetime(TIME);
            std::string timeString = item.getElementAsString(TIME);
            type = item.getElementAsString(TYPE);
            double value = item.getElementAsFloat64(VALUE);
            int size = item.getElementAsInt32(TICK_SIZE);
            if (item.hasElement(COND_CODE)) {
                cc = item.getElementAsString(COND_CODE);
            }  else {
                cc.clear();
            }

            std::cout.setf(std::ios::fixed, std::ios::floatfield);
            std::cout << timeString <<  "\t"
                << type << "\t" 
                << std::setprecision(3)
                << std::showpoint << value << "\t\t"
                << size << "\t" << std::noshowpoint 
                << cc << std::endl;

			output += "{\"";
			output += timeString;
			output += "\", ";
			output += item.getElementAsString(VALUE);
			output += ", ";
			output += item.getElementAsString(TICK_SIZE);
			output += "}";
			if(i < numItems-1)
				output += ",";
        }
		output += "}";
		return output;
    }

    public:
	string sendIntradayRequest(const char* Ticker,const char* Field,const char* StartTime, const char* EndTime, int useDPDF, int debug)
    {
        string output = "Failed";
		d_host = "localhost";
        d_port = 8194;

        SessionOptions sessionOptions;
        sessionOptions.setServerHost(d_host.c_str());
        sessionOptions.setServerPort(d_port);

        std::cout << "Connecting to " <<  d_host << ":" 
                  << d_port << std::endl;
        Session session(sessionOptions);
        if (!session.start()) {
            std::cerr <<"Failed to start session." << std::endl;
            return "Failed to open";
        }
        if (!session.openService("//blp/refdata")) {
            std::cerr <<"Failed to open //blp/refdata" << std::endl;
            return "Failed to open";
        }

        Service refDataService = session.getService("//blp/refdata");
        Request request = refDataService.createRequest("IntradayTickRequest");

		// only one security/eventType per request
        request.set("security", Ticker);

        // Add fields to request
        Element eventTypes = request.getElement("eventTypes");
        eventTypes.appendValue(Field);
		
		// Times are in GMT
		request.set("startDateTime", StartTime);
		request.set("endDateTime", EndTime);

		request.set("includeConditionCodes", false);

		if (useDPDF == 0)
		{
			request.set("adjustmentFollowDPDF", false);
		}
		else
		{
			request.set("adjustmentFollowDPDF", true);
		}

        std::cout << "Sending Request: " << request << std::endl;
        session.sendRequest(request);

		bool done = false;
        while (!done) {
            Event event = session.nextEvent();
            if (event.eventType() == Event::PARTIAL_RESPONSE) {
                std::cout << "Processing Partial Response" << std::endl;
                output = processEvent(event);
            }
            else if (event.eventType() == Event::RESPONSE) {
                std::cout << "Processing Response" << std::endl;
                output = processEvent(event);
				done = true;
            } else {
                MessageIterator msgIter(event);
                while (msgIter.next()) {
                    Message msg = msgIter.message();
                    if (event.eventType() == Event::SESSION_STATUS) {
                        if (msg.messageType() == SESSION_TERMINATED ||
                            msg.messageType() == SESSION_STARTUP_FAILURE) {
                            done = true;
                        }
                    }
                }
            }
        }
		return output;
	}
};

// this gets the command from Mathematica
void msBBGintradayLink(const char* Ticker,const char* Field,const char* StartTime, const char* EndTime, int dpdf, int debug)
{
	std::string bbgOutput;
	Intraday intradayObj;
	bbgOutput = intradayObj.sendIntradayRequest(Ticker, Field, StartTime, EndTime, dpdf, debug);
	WSPutString(stdlink, bbgOutput.c_str());
	return;
}