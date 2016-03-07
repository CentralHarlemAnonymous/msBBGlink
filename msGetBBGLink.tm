
void msGetBBG P(( int, int));

:Begin:
:Function:       msGetBBGLink
:Pattern:        msGetBBGLink[ticker_String, field_String, noisy_Integer]
:Arguments:      { ticker, field, noisy }
:ArgumentTypes:  { String, String, Integer}
:ReturnType:     Manual
:End:

:Evaluate: msGetBBGLink::usage = "msGetBBGLink[ticker, field, noisy] returns the requested fields for the specified tickers by direct call to WSTP. If noisy is set to True, outputs debugging information."
