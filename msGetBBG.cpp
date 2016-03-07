/* To launch this program from within Mathematica use:
 *   In[1]:= link = Install["msGetBBG.exe"]
 *
 * Or, launch this program from a shell and establish a
 * peer-to-peer connection.  When given the prompt Create Link:
 * type a port name. ( On Unix platforms, a port name is a
 * number less than 65536.  On Mac or Windows platforms,
 * it's an arbitrary word.)
 * Then, from within Mathematica use:
 *   In[1]:= link = Install["portname", LinkMode->Connect]
 */

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
#include <blpapi_subscriptionlist.h> // correlationID should be in here

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <stdlib.h>
#include <string.h>

using namespace BloombergLP;
using namespace blpapi;
using namespace std;

namespace {
	const Name SECURITY_DATA("securityData");
	const Name SECURITY("security");
	const Name FIELD_DATA("fieldData");
	const Name SEQ_NUMBER("sequenceNumber");
	const Name RESPONSE_ERROR("responseError");
	const Name SECURITY_ERROR("securityError");
	const Name FIELD_EXCEPTIONS("fieldExceptions");
	const Name FIELD_ID("fieldId");
	const Name ERROR_INFO("errorInfo");
	const Name CATEGORY("category");
	const Name MESSAGE("message");
	const Name REASON("reason");
	const Name SESSION_TERMINATED("SessionTerminated");
	const Name SESSION_STARTUP_FAILURE("SessionStartupFailure");
};

extern "C" {
	void loggingCallback(blpapi_UInt64_t    threadId,
		int                severity,
		blpapi_Datetime_t  timestamp,
		const char        *category,
		const char        *message);
}

void loggingCallback(blpapi_UInt64_t    threadId,
	int                severity,
	blpapi_Datetime_t  timestamp,
	const char        *category,
	const char        *message)
{
	std::stringstream outstream;
	std::string severityString;
	switch (severity) {
		// The following cases will not happen if callback registered at OFF
	case blpapi_Logging_SEVERITY_FATAL:
	{
		severityString = "FATAL";
	} break;
	// The following cases will not happen if callback registered at FATAL
	case blpapi_Logging_SEVERITY_ERROR:
	{
		severityString = "ERROR";
	} break;
	// The following cases will not happen if callback registered at ERROR
	case blpapi_Logging_SEVERITY_WARN:
	{
		severityString = "WARN";
	} break;
	// The following cases will not happen if callback registered at WARN
	case blpapi_Logging_SEVERITY_INFO:
	{
		severityString = "INFO";
	} break;
	// The following cases will not happen if callback registered at INFO
	case blpapi_Logging_SEVERITY_DEBUG:
	{
		severityString = "DEBUG";
	} break;
	// The following case will not happen if callback registered at DEBUG
	case blpapi_Logging_SEVERITY_TRACE:
	{
		severityString = "TRACE";
	} break;

	};
	std::stringstream sstream;
	sstream << category << " [" << severityString << "] Thread ID = "
		<< threadId << ": " << message << std::endl;
	outstream << sstream.str() << std::endl;;
	// return outstream.str();
}

//from BBG 2016
class RefDataExample
{
	std::string              d_host;
	int                      d_port;
	std::vector<std::string> d_securities;
	std::vector<std::string> d_fields;

	bool bloombergLists(const char* Ticker, const char* Field)	// formerly bool parseCommandLine(int argc, char **argv) from RefDataExample.cpp
	{

		int verbosityCount = 0;

		// for the moment assumes a single ticker and single field
		d_securities.push_back(Ticker);
		d_fields.push_back(Field);

/*		for (int i = 1; i < argc; ++i) {
			if (!std::strcmp(argv[i], "-s") && i + 1 < argc) {
				d_securities.push_back(argv[++i]);
			}
			else if (!std::strcmp(argv[i], "-f") && i + 1 < argc) {
				d_fields.push_back(argv[++i]);
			}
			else if (!std::strcmp(argv[i], "-ip") && i + 1 < argc) {
				d_host = argv[++i];
			}
			else if (!std::strcmp(argv[i], "-p") && i + 1 < argc) {
				d_port = std::atoi(argv[++i]);
			}
			else if (!std::strcmp(argv[i], "-v")) {
				++verbosityCount;

			}
			else {
				printUsage();
				return false;
			}
		}	*/
		if (verbosityCount) {
			registerCallback(verbosityCount);
		}
		// handle default arguments
		if (d_securities.size() == 0) {
			d_securities.push_back("IBM US Equity");
		}

		if (d_fields.size() == 0) {
			d_fields.push_back("PX_LAST");
		}

		return true;
	}

	std::string printErrorInfo(const char *leadingStr, const Element &errorInfo)
	{
		std::stringstream outstream;

		outstream << leadingStr
			<< errorInfo.getElementAsString(CATEGORY)
			<< " ("
			<< errorInfo.getElementAsString(MESSAGE)
			<< ")" << std::endl;

		return outstream.str();
	}

	void registerCallback(int verbosityCount)
	{
		blpapi_Logging_Severity_t severity = blpapi_Logging_SEVERITY_OFF;
		switch (verbosityCount) {
		case 1: {
			severity = blpapi_Logging_SEVERITY_INFO;
		}break;
		case 2: {
			severity = blpapi_Logging_SEVERITY_DEBUG;
		}break;
		default: {
			severity = blpapi_Logging_SEVERITY_TRACE;
		}
		};
		blpapi_Logging_registerCallback(loggingCallback, severity);
	}


	// per the Bloomberg documentation, a messageIterator can have multiple messages, each of which is also an element and can have sub-elements. Each element can be an array.
	// securityData is typically the most interesting element, and it is always (?) an array.
	std::string sendRefDataRequest(Session &session)
	{
		std::stringstream outstream;
		Service refDataService = session.getService("//blp/refdata");
		Request request = refDataService.createRequest("ReferenceDataRequest");

		// Add securities to request
		Element securities = request.getElement("securities");
		for (size_t i = 0; i < d_securities.size(); ++i) {
			securities.appendValue(d_securities[i].c_str());
		}

		// Add fields to request
		Element fields = request.getElement("fields");
		for (size_t i = 0; i < d_fields.size(); ++i) {
			fields.appendValue(d_fields[i].c_str());
		}

		request.set("returnNullValue", "True"); // thanks to Jose Paula Bloomberg Support -- this prevents crash on oddball outcomes like dividends announced but cancelled before record date
		outstream << "Sending Request: " << request << std::endl;
		session.sendRequest(request);
		return outstream.str();
	}

	// return true if processing is completed, false otherwise
	std::string processResponseEvent(Event event, int debug)
	{
		std::stringstream outstream;
		MessageIterator msgIter(event);
		while (msgIter.next()) {
			Message msg = msgIter.message();
			if (msg.asElement().hasElement(RESPONSE_ERROR)) {
				outstream << printErrorInfo("REQUEST FAILED: ",	msg.getElement(RESPONSE_ERROR));
				continue;
			}

			Element securities = msg.getElement(SECURITY_DATA);
			size_t numSecurities = securities.numValues();
			if (debug > 0) {
				outstream << "Processing " << (unsigned int)numSecurities << " securities:" << std::endl;
			}
			for (size_t i = 0; i < numSecurities; ++i) {
				Element security = securities.getValueAsElement(i);
				std::string ticker = security.getElementAsString(SECURITY);
				if (debug > 0) {
					outstream << "\nTicker: " + ticker << std::endl;
				}
				if (security.hasElement(SECURITY_ERROR)) {
					outstream << printErrorInfo("\tSECURITY FAILED: ", security.getElement(SECURITY_ERROR));
					continue;
				}

				if (security.hasElement(FIELD_DATA)) {
					const Element fields = security.getElement(FIELD_DATA);
					if (fields.numElements() > 0) {
						if (debug > 0) {
							outstream << "FIELD\t\tVALUE" << std::endl;
							outstream << "-----\t\t-----" << std::endl;
						}
						size_t numElements = fields.numElements();
						for (size_t j = 0; j < numElements; ++j) {
							Element field = fields.getElement(j);
							if (field.isArray()) {
								outstream << processBulkData(msg, debug);
							}
							else {
								if (debug > 0) {
									outstream << field.name() << "\t\t";
								}
								outstream << field.getValueAsString();
							}
						}
					}
				}
	//			outstream << std::endl;
				Element fieldExceptions = security.getElement(FIELD_EXCEPTIONS);
				if (fieldExceptions.numValues() > 0) {
					outstream << "FIELD\t\tEXCEPTION" << std::endl;
					outstream << "-----\t\t---------" << std::endl;
					for (size_t k = 0; k < fieldExceptions.numValues(); ++k) {
						Element fieldException =
							fieldExceptions.getValueAsElement(k);
						Element errInfo = fieldException.getElement(ERROR_INFO);
						outstream
							<< fieldException.getElementAsString(FIELD_ID) << "\t\t"
							<< errInfo.getElementAsString(CATEGORY) << " ( "
							<< errInfo.getElementAsString(MESSAGE) << ")" << std::endl;
					}
				}
			}
		}
		return outstream.str();
	}

	std::string processBulkData(Message &msg, int debug)	// processMessage() from RefDataOverrideExample.cpp
	{
		std::stringstream outstream;
		std::stringstream innerstream;
		bool justDidAnArray=false;
		Element securityDataArray = msg.getElement(SECURITY_DATA);
		int numSecurities = securityDataArray.numValues();
		for (int i = 0; i < numSecurities; ++i) {
			Element securityData = securityDataArray.getValueAsElement(i);
			if (debug > 0) {
				outstream << securityData.getElementAsString(SECURITY)
					<< std::endl;
			}
			const Element fieldData = securityData.getElement(FIELD_DATA);
			for (size_t j = 0; j < fieldData.numElements(); ++j) {
				Element field = fieldData.getElement(j);
				if (!field.isValid()) {			// possibility 1 - field is not valid
					outstream << " field " << field.name() << " is NULL." << std::endl;
				}
				else if (field.isArray()) {		// possibility 2 - field is an array or matrix
					for (size_t rowNo = 0; rowNo < field.numValues(); ++rowNo) {	// get every row
						Element row = field.getValueAsElement(rowNo);	// http://bloomberg.github.io/blpapi-docs/dotnet/3.7/html/T_Bloomberglp_Blpapi_Element.htm
						if (!row.isValid()) {					//	possibility 1 - row is not valid
							outstream << " row " << row.name() << " is NULL." << std::endl;
						}
						else if (row.isComplexType()) {			// possibility 2 - row has sub-elements
							innerstream.str(std::string()); // reset innerstream
							size_t maxsize = row.numElements();
							for (size_t itemNo = 0; itemNo < maxsize; ++itemNo) {
								Element item = row.getElement(itemNo);

								Name name1 = item.name();
								const char * valString;
								int whatType = item.datatype();
								std::string nameString = name1.string();

								if (! item.hasElement(name1)) {
										valString = item.getValueAsString();
								}
								else { valString = "no entry"; }
								
								if (whatType == BLPAPI_DATATYPE_STRING ||
									whatType == BLPAPI_DATATYPE_BYTEARRAY ||
									whatType == BLPAPI_DATATYPE_DATE ||
									whatType == BLPAPI_DATATYPE_TIME) {
									innerstream << "\"" << nameString << "\" -> \"" << valString << "\", "; // whether or not we put quotes around this depends on what format it's in.
								}
								else
								{
									innerstream << "\"" << nameString << "\" -> " << valString << ", ";
								}
							}	// finish loop over items
							if (innerstream.str().size() > 0) {	// trim the trailing comma and space
								std::string wholeString = innerstream.str();
								wholeString.pop_back();
								wholeString.pop_back();
								innerstream.str(wholeString);
								outstream << "<|" << innerstream.str() << "|>, ";
								justDidAnArray = true;
							}
						}
						else {	// if row is not a complex type, returns the entire row
							outstream << "StartRow " << rowNo << ": " << row << " EndRow" << std::endl;
						}
					}	// end of row review
				}
				else {							// possibility 3, field is a simple datum
					outstream << "SimpleDatum :" << field.name() << " = "	<< field.getValueAsString() << std::endl;
				}
			}	// end of field review
			if (justDidAnArray) {	// trim the trailing comma and space. somehow this is killing the previous ticker reference too
				std::string wholeString = outstream.str();
				wholeString.pop_back();
				wholeString.pop_back();
				outstream.str(std::string());	// clear outstream
				outstream << "{" << wholeString << "}";
				justDidAnArray = false;
			}


			Element fieldExceptionArray =
				securityData.getElement(FIELD_EXCEPTIONS);
			for (size_t k = 0; k < fieldExceptionArray.numValues(); ++k) {
				Element fieldException =
					fieldExceptionArray.getValueAsElement(k);
				outstream <<
					fieldException.getElement(ERROR_INFO).getElementAsString(
						"category")
					<< ": " << fieldException.getElementAsString(FIELD_ID);
			}
		}
		return outstream.str();
	}


	std::string eventLoop(Session &session, int debug)
	{
		std::stringstream outstream;
		bool done = false;
		while (!done) {
			Event event = session.nextEvent();
			if (event.eventType() == Event::PARTIAL_RESPONSE) {
				if (debug > 0) {
					outstream << "Processing Partial Response" << std::endl;
				}
				outstream << processResponseEvent(event, debug);
			}
			else if (event.eventType() == Event::RESPONSE) {
				if (debug > 0) {
					outstream << "Processing Response" << std::endl;
				}
				outstream << processResponseEvent(event, debug);
				done = true;
			}
			else {
				MessageIterator msgIter(event);
				while (msgIter.next()) {
					Message msg = msgIter.message();
					if (event.eventType() == Event::REQUEST_STATUS) {
						outstream << "REQUEST FAILED: " << msg.getElement(REASON) << std::endl;
						done = true;
					}
					else if (event.eventType() == Event::SESSION_STATUS) {
						if (msg.messageType() == SESSION_TERMINATED ||
							msg.messageType() == SESSION_STARTUP_FAILURE) {
							outstream << "session terminated";
							done = true;
						}
					}
				}
			}
		}
		return outstream.str();
	}


public:

	RefDataExample()
	{
		d_host = "localhost";
		d_port = 8194;
	}

	~RefDataExample()
	{
	}

	// makes one or more Bloomberg calls, gets the Event response and associated MessageIterator, then calls ProcessMessage() to deal with what it's gotten. Returns the result to msGetBBG()
	// adapted from BBG 2016, closely related to the "Retrieve" function in the 2008/2009 code's BBRetrieveCurrent
	std::string run(const char* Ticker, const char* Field, int debug)
	{
		std::stringstream outstream;
		bloombergLists(Ticker, Field); /* sets up tickers and fields */

		SessionOptions sessionOptions;
		sessionOptions.setServerHost(d_host.c_str());
		sessionOptions.setServerPort(d_port);

		if (debug > 0) { outstream << "Connecting to " + d_host + ":" << d_port << std::endl; }
		Session session(sessionOptions);
		if (!session.start()) {
			outstream << "Failed to start session." << std::endl;
			return outstream.str();
		}
		if (!session.openService("//blp/refdata")) {
			outstream << "Failed to open //blp/refdata" << std::endl;
			return outstream.str();
		}
		sendRefDataRequest(session);


		// wait for events from session.
		try {
			outstream << eventLoop(session, debug);
		}
		catch (Exception &e) {
			outstream << "Library Exception !!!"
				<< e.description()
				<< std::endl;
		}
		catch (...) {
			outstream << "Unknown Exception !!!" << std::endl;
		}

		session.stop();
		return outstream.str();
	}
};

// This is effectively our entry point. It's called by WSTP template code. alvin's Retrieve function returned a string
// BBRetrieve per the template file has a returntype of Manual, which means I need to use a function like WSPutInteger32() to return an integer
void msGetBBG(const char* ticker, const char* field, int debug)
{
	RefDataExample bbgCallResponse;
	std::string bbgOutput;
	bbgOutput = bbgCallResponse.run(ticker, field, debug);
	WSPutString(stdlink, bbgOutput.c_str()/* std::string converted to const char */);	// https://reference.wolfram.com/language/ref/c/WSPutString.html

	return;
}


// the following is from addTwo
#if WINDOWS_WSTP

#if __BORLANDC__
#pragma argsused
#endif

int PASCAL WinMain( HINSTANCE hinstCurrent, HINSTANCE hinstPrevious, LPSTR lpszCmdLine, int nCmdShow)
{
	char  buff[512];
	char FAR * buff_start = buff;
	char FAR * argv[32];
	char FAR * FAR * argv_end = argv + 32;

	hinstPrevious = hinstPrevious; /* suppress warning */

	if( !WSInitializeIcon( hinstCurrent, nCmdShow)) return 1;
	WSScanString( argv, &argv_end, &lpszCmdLine, &buff_start);
	return WSMain( (int)(argv_end - argv), argv);
}

#else

int main(int argc, char* argv[])
{
	return WSMain(argc, argv);
}

#endif
