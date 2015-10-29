#include "stdafx.h"
#include "microsipDlg.h"
#include "settings.h"

static CmicrosipDlg *microsipDlg;

#ifdef _GLOBAL_ACCOUNT_REG
#include "addons/Reg1.cpp"
#include "addons/Reg2.cpp"
#include "addons/Reg3.cpp"
#endif

#if defined _GLOBAL_JSON || defined _GLOBAL_BALANCE_BEE
#include "addons/jsoncpp/json_reader.cpp"
#include "addons/jsoncpp/json_value.cpp"
#include "addons/jsoncpp/json_writer.cpp"
#endif

#include "addons/CXMLFile/XMLFile.cpp"

#ifdef _GLOBAL_CONFERENCE_DIALOG
#include "addons/Conference.cpp"
#endif

#ifdef _GLOBAL_PAGE_BUTTONS
#include "addons/itu/Buttons.cpp"
#endif
