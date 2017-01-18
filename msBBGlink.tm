:Begin:
:Function:       msBBGintradayBarsLink
:Pattern:        msBBGintradayBarsLink[ticker_String, field_String, startTime_String, endTime_String, barSize_Integer, fill_Integer, useDPDF_Integer, debug_Integer]
:Arguments:      { ticker, field, startTime, endTime , barSize , fill , useDPDF, debug }
:ArgumentTypes:  { String, String, String, String, Integer, Integer, Integer, Integer }
:ReturnType:     Manual
:End:

:Evaluate: msBBGintradayBarsLink::usage = "msBBGintradayBarsLink[Ticker (e.g. \"AAPL Equity\"), Field (\"TRADE\", \"BID\", or \"ASK\"), StartTime (e.g., \"2016-12-02T14:30:00\"), EndTime(e.g., \"2016-12-02T16:30:00\"), BarSize (integer number of minutes), Fill (0 for False or 1 for True), useDPDF (0 for False or 1 for True), debug (0 for False or 1 for True)] gives Bloomberg data for the period specified."


:Begin:
:Function:       msBBGintradayLink
:Pattern:        msBBGintradayLink[Ticker_String, Field_String, StartTime_String, EndTime_String, useDPDF_Integer, debug_Integer]
:Arguments:      { Ticker, Field, StartTime, EndTime, useDPDF,  debug }
:ArgumentTypes:  { String, String, String, String, Integer, Integer }
:ReturnType:     Manual
:End:

:Evaluate: msBBGintradayLink::usage= "msBBGintradayLink[Ticker (e.g. \"AAPL Equity\"), Field (\"TRADE\", \"BID\", or \"ASK\"), StartTime (e.g., \"2016-12-02T14:30:00\"), EndTime(e.g., \"2016-12-02T16:30:00\"), useDPDF (0 for False or 1 for True), debug (0 for False or 1 for True)] gives Bloomberg data for the period specified."


:Begin:
:Function:       msBBGhistoryLink
:Pattern:        msBBGhistoryLink[ticker_String, field_String, startDate_String, endDate_String, periodicitySelection_String, periodicityAdjustment_String, useDPDF_Integer, debug_Integer]
:Arguments:      { ticker, field, startDate, endDate , periodicitySelection , periodicityAdjustment, useDPDF, debug }
:ArgumentTypes:  { String, String, String, String, String, String, Integer, Integer }
:ReturnType:     Manual
:End:

:Evaluate: msBBGhistoryLink::usage = "msBBGhistoryLink[Ticker (e.g. \"AAPL Equity\"), Field (e.g., \"Px_Last\"), StartDate (e.g., \"20071031\"), EndDate(e.g., \"20080101\"), Frequency (e.g., \"DAILY\",\"WEEKLY\",\"MONTHLY\",\"QUARTERLY\",\"ANNUALLY\"), Adjustment (e.g., \"ACTUAL\",\"CALENDAR\"), useDPDF (0 for False or 1 for True), debug (0 for False or 1 for True)] gives historical Bloomberg data for the period specified."


:Begin:
:Function:       msBBGcurrentLink
:Pattern:        msBBGcurrentLink[ticker_String, field_String, debug_Integer, overrides_String]
:Arguments:      { ticker, field, debug, overrides }
:ArgumentTypes:  { String, String, Integer, String}
:ReturnType:     Manual
:End:

:Evaluate: msBBGcurrentLink::usage = "msBBGcurrentLink[ticker, field, noisy, overrides] returns the requested fields for the specified tickers by direct call to WSTP. If debug is set to 1, outputs debugging information."
