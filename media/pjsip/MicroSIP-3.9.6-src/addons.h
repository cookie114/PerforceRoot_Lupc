#include "const.h"

#ifdef _GLOBAL_ACCOUNT_REG
#include "addons/Reg1.h"
#include "addons/Reg2.h"
#include "addons/Reg3.h"
#endif

#if defined _GLOBAL_JSON || defined _GLOBAL_BALANCE_BEE
#include "addons/jsoncpp/json/json.h"
#endif

#include "addons/CXMLFile/XMLFile.h"

#ifdef _GLOBAL_CONFERENCE_DIALOG
#include "addons/Conference.h"
#endif

#ifdef _GLOBAL_PAGE_BUTTONS
#include "addons/itu/Buttons.h"
#endif
