// msBBGintradayBarsLink.cpp : Defines the entry point for the console application.
//
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "wstp.h"
#include <blpapi_defs.h>
#include <blpapi_event.h>
#include <blpapi_element.h>
#include <blpapi_eventdispatcher.h>
#include <blpapi_exception.h>
#include <blpapi_logging.h>
#include <blpapi_message.h>
#include <blpapi_name.h>
#include <blpapi_request.h>
#include <blpapi_session.h>

#include <time.h>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <stdlib.h>
#include <string.h>

extern void msBBGintradayBarsLink(const char* ticker,const char* field,const char* startTime, const char* endTime, int barSize, int fill, int dpdf, int debug);

using namespace BloombergLP;
using namespace blpapi;
using namespace std;

namespace {
	const Name BAR_DATA("barData");
    const Name BAR_TICK_DATA("barTickData");
    const Name OPEN("open");
    const Name HIGH("high");
    const Name LOW("low");
    const Name CLOSE("close");
    const Name VOLUME("volume");
    const Name NUM_EVENTS("numEvents");
    const Name TIME("time");
    const Name RESPONSE_ERROR("responseError");
    const Name SESSION_TERMINATED("SessionTerminated");
    const Name CATEGORY("category");
    const Name MESSAGE("message");
	const Name SESSION_STARTUP_FAILURE("SessionStartupFailure");
}

class IntradayBars
{
	std::string         d_host;
    int                 d_port;

    void printUsage()
    {
		std::cout << "Usage:" << std::endl
			<< "	sendIntradayBarsRequest reference data " << std::endl
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
        Element data = msg.getElement(BAR_DATA).getElement(BAR_TICK_DATA);
        int numBars = data.numValues();
        std::cout <<"Response contains " << numBars << " bars" << std::endl;
        std::cout <<"Datetime\t\tOpen\t\tHigh\t\tLow\t\tClose" <<
            "\t\tNumEvents\tVolume" << std::endl;
		output = "{";
        for (int i = 0; i < numBars; ++i) {
            Element bar = data.getValueAsElement(i);
            Datetime time = bar.getElementAsDatetime(TIME);
			std::string timeString = bar.getElementAsString(TIME);
            double open = bar.getElementAsFloat64(OPEN);
            double high = bar.getElementAsFloat64(HIGH);
            double low = bar.getElementAsFloat64(LOW);
            double close = bar.getElementAsFloat64(CLOSE);
            int numEvents = bar.getElementAsInt32(NUM_EVENTS);
            long long volume = bar.getElementAsInt64(VOLUME);

            std::cout.setf(std::ios::fixed, std::ios::floatfield);
            std::cout << time.month() << '/' << time.day() << '/' << time.year()
                << " " << time.hours() << ":" << time.minutes()
                <<  "\t\t" << std::showpoint
                << std::setprecision(3) << open << "\t\t"
                << high << "\t\t"
                << low <<  "\t\t"
                << close <<  "\t\t"
                << numEvents <<  "\t\t"
                << std::noshowpoint
                << volume << std::endl;

			output += "{\"";
			output += timeString;
			output += "\", ";
			output += bar.getElementAsString(OPEN);
			output += ", ";
			output += bar.getElementAsString(HIGH);
			output += ", ";
			output += bar.getElementAsString(LOW);
			output += ", ";
			output += bar.getElementAsString(CLOSE);
			output += ", ";
			output += bar.getElementAsString(NUM_EVENTS);
			output += ", ";
			output += bar.getElementAsString(VOLUME);
			output += "}";
			if(i < numBars-1)
				output += ",";
        }
		output += "}";
		return output;
    }

    public:
	string sendIntradayBarsRequest(const char* ticker,const char* field,const char* startTime, const char* endTime, int barSize, int fill, int dpdf, int debug)
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
        Request request = refDataService.createRequest("IntradayBarRequest");

		// only one security/eventType per request
        request.set("security", ticker);
		request.set("eventType", field);
        request.set("interval", barSize);        
		
		// Times are in GMT
		request.set("startDateTime", startTime);
		request.set("endDateTime", endTime);

		if (fill)
			request.set("gapFillInitialBar", true);
		else
			request.set("gapFillInitialBar", false);

		if (dpdf)
			request.set("adjustmentFollowDPDF", true);
		else
			request.set("adjustmentFollowDPDF", false);

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


// This is effectively our entry point. It's called by WSTP template code.
// per the template file, msGetBBGLink() has a returntype of Manual, which means I need to use a function like WSPutInteger32() to return an integer
void msBBGintradayBarsLink(const char* ticker,const char* field,const char* startTime, const char* endTime, int barSize, int fill, int dpdf, int debug)
{
	std::string bbgOutput;
	IntradayBars bbgIntradayObj;
	bbgOutput = bbgIntradayObj.sendIntradayBarsRequest(ticker,field,startTime,endTime,barSize,fill,dpdf,debug);
	WSPutString(stdlink, bbgOutput.c_str());	// https://reference.wolfram.com/language/ref/c/WSPutString.html
	return;
}


// the following is from addTwo
#if WINDOWS_WSTP

#if __BORLANDC__
#pragma argsused
#endif

int PASCAL WinMain(HINSTANCE hinstCurrent, HINSTANCE hinstPrevious, LPSTR lpszCmdLine, int nCmdShow)
{
	char  buff[512];
	char FAR * buff_start = buff;
	char FAR * argv[32];
	char FAR * FAR * argv_end = argv + 32;

	hinstPrevious = hinstPrevious; /* suppress warning */

	if (!WSInitializeIcon(hinstCurrent, nCmdShow)) return 1;
	WSScanString(argv, &argv_end, &lpszCmdLine, &buff_start);
	return WSMain((int)(argv_end - argv), argv);
}

#else

int main(int argc, char* argv[])
{
	return WSMain(argc, argv);
}

#endif
