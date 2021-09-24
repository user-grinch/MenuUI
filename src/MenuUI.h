#include <plugin.h>
#include <CMenuSystem.h>
#include <CSprite2d.h>
#include <vector>

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
    struct HeaderInfo
    {
        std::string gxtName;
        CSprite2d *m_pLogo, *m_pPattern;
    };
    static inline std::vector<HeaderInfo> vecHeaders;

    // config
    static inline CRGBA windowBgColor, titleBgColor, textColor, titleColor, 
                    selectTextBgColor, selectTextColor, grayTextColor;
    static inline CVector2D textScaleMul, titleScaleMul;
    static inline int menuStyle, textStyle, titleStyle;

    static inline CSprite2d menuBoughtSprite, menuSelectorSprite;

    static void ReadConfig();
    static void ReadHeaderInfo();
    static void WrapXCenteredPrint(char* pGXT, CRect windowRect);
    static void __cdecl DisplayStandardMenu(unsigned char panelId, bool bBrightFont);

public:
    MenuUi()= delete;
    MenuUi(MenuUi&)= delete;

    static void InjectPatches();
};