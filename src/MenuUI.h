#include <plugin.h>
#include <CMenuSystem.h>

#define WINDOW_ALPHA 200
#define TITLE_ALPHA 250
#define HEADER_PADDING 75.0f

using namespace plugin;

enum MENU_STYLE 
{
    NORMAL_STYLE,
    MOBILE_STYLE,
};

class MenuUi 
{
private:
    static void WrapXCenteredPrint(char* pGXT, CRect windowRect);
    static void __cdecl DisplayStandardMenu(unsigned char panelId, bool bBrightFont);

public:
    MenuUi()= delete;
    MenuUi(MenuUi&)= delete;

    static void InjectPatches();
};