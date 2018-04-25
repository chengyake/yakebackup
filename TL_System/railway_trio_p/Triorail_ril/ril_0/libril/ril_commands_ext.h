 //telephony/ril_commands_ext.h

    {RIL_REQUEST_DIAL_VT, dispatchDial, responseVoid},
    {RIL_REQUEST_HANGUP_VT, dispatchInts, responseVoid},
    {RIL_REQUEST_SET_ACM, dispatchStrings, responseVoid},
    {RIL_REQUEST_GET_ACM, dispatchVoid, responseStrings},
    {RIL_REQUEST_SET_AMM, dispatchStrings, responseVoid},
    {RIL_REQUEST_GET_AMM, dispatchVoid, responseStrings},
    {RIL_REQUEST_SET_CPUC, dispatchStrings, responseVoid},
    {RIL_REQUEST_GET_CPUC, dispatchVoid, responseStrings},
    {RIL_REQUEST_FAST_DORMANCY, dispatchVoid, responseVoid},
    {RIL_REQUEST_SELECT_BAND, dispatchInts, responseVoid},
    {RIL_REQUEST_GET_BAND, dispatchVoid, responseInts},
    {RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL_EXT, dispatchNetworkSelectionManual, responseVoid},
    {RIL_REQUEST_QUERY_COLP, dispatchVoid, responseInts},
    {RIL_REQUEST_SET_CLIP,dispatchInts, responseVoid},
    {RIL_REQUEST_SET_COLP,dispatchInts, responseVoid},
    {RIL_REQUEST_GET_CNAP, dispatchVoid, responseInts},
    {RIL_REQUEST_SET_CNAP, dispatchInts, responseVoid},
    {RIL_REQUEST_QUERY_COLR, dispatchVoid, responseInts},
    {RIL_REQUEST_SET_COLR,dispatchInts, responseVoid},
    {RIL_REQUEST_LOCK_INFO,dispatchVoid, responseInts},
    {RIL_REQUEST_SIM_TRANSMIT_BASIC, dispatchSIM_IO, responseSIM_IO},
    {RIL_REQUEST_SIM_OPEN_CHANNEL, dispatchString, responseInts},
    {RIL_REQUEST_SIM_CLOSE_CHANNEL, dispatchInts, responseVoid},
    {RIL_REQUEST_SIM_TRANSMIT_CHANNEL, dispatchSIM_IO, responseSIM_IO},
    {RIL_REQUEST_SIM_GET_ATR, dispatchVoid, responseRaw},
    {RIL_REQUEST_SET_FDY, dispatchInts, responseVoid},
    {RIL_REQUEST_SET_COMCFG, dispatchInts, responseVoid},
    {RIL_REQUEST_GET_COMCFG, dispatchInts, responseInts},
    {RIL_REQUEST_SWITCH_MODEM, dispatchInts, responseVoid},
    {RIL_REQUEST_GET_IMS_REGISTRATION_STATE, dispatchVoid, responseInts}
