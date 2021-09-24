/* 
*   MenuUI - Improved Menu Interface for SA
*   Author - Grinch_
*/
#include "MenuUi.h"
#include <sstream>
#include <CSprite2d.h>
#include <CMessages.h>
#include <CText.h>
#include "INIReader.h"


// ---------------------------------------------------
// Fetching values from ini file
// ---------------------------------------------------
INIReader ini(PLUGIN_PATH((char*)"MenuUI.ini"));

short textStyle = (short)ini.GetInteger("font", "text", 1);
short titleStyle = (short)ini.GetInteger("font", "title", 1);

CVector2D textScaleMul{ ini.GetFloat("font", "text_scaleX", 1.0f) , ini.GetFloat("font", "text_scaleX", 1.0f) };
CVector2D titleScaleMul{ ini.GetFloat("font", "title_scaleX", 1.0f) , ini.GetFloat("font", "title_scaleX", 1.0f) };

CRGBA windowBgColor =
{
    (uchar)ini.GetInteger("color", "window_bg_red", 0),
    (uchar)ini.GetInteger("color", "window_bg_green", 0),
    (uchar)ini.GetInteger("color", "window_bg_blue", 0),
    WINDOW_ALPHA
};

CRGBA titleBgColor =
{
    (uchar)ini.GetInteger("color", "window_bg_red", 0),
    (uchar)ini.GetInteger("color", "window_bg_green", 0),
    (uchar)ini.GetInteger("color", "window_bg_blue", 0),
    TITLE_ALPHA
};

CRGBA textColor =
{
    (uchar)ini.GetInteger("color", "text_red", 0),
    (uchar)ini.GetInteger("color", "text_green", 0),
    (uchar)ini.GetInteger("color", "text_blue", 0),
    255
};

CRGBA titleColor =
{
    (uchar)ini.GetInteger("color", "title_red", 0),
    (uchar)ini.GetInteger("color", "title_green", 0),
    (uchar)ini.GetInteger("color", "title_blue", 0),
    255
};

CRGBA selectTextColor =
{
    (uchar)ini.GetInteger("color", "select_text_red", 0),
    (uchar)ini.GetInteger("color", "select_text_green", 0),
    (uchar)ini.GetInteger("color", "select_text_blue", 0),
    255
};

CRGBA selectTextBgColor =
{
    (uchar)ini.GetInteger("color", "select_text_bg_red", 0),
    (uchar)ini.GetInteger("color", "select_text_bg_green", 0),
    (uchar)ini.GetInteger("color", "select_text_bg_blue", 0),
    255
};

MENU_STYLE menuStyle = (MENU_STYLE)ini.GetInteger("main", "menu_style", MOBILE_STYLE);
// ---------------------------------------------------


void MenuUi::InjectPatches()
{
    Events::initGameEvent += []
    {
        patch::ReplaceFunction(0x580E00, DisplayStandardMenu);
    };
}

/* 
    Printing text at the center pos with wrapping
*/
void MenuUi::WrapXCenteredPrint(char* pGXT, CRect windowRect)
{
    char* pText = TheText.Get(pGXT);
    float textPosX = windowRect.left + (windowRect.right - windowRect.left) / 2;
    float textPosY = windowRect.top + (windowRect.bottom - windowRect.top) / 2 - 20.0f;
    float windowWidth = windowRect.right - windowRect.left;

    float fontWidth = CFont::GetStringWidth(pText, true, false);

    if (fontWidth <= windowWidth)
    {
        CFont::PrintString(textPosX, textPosY, pText);
        return;
    }

    std::istringstream ss(pText);
    std::string buf, temp;
    do {
        std::string word;
        ss >> word;

        temp += (temp == "") ? word : (" " + word);
        fontWidth = CFont::GetStringWidth((char*)temp.c_str(), true, false);

        if (fontWidth < windowWidth && word != "")
        {
            buf += " " + word;
        }
        else
        {
            temp = word;
            CFont::PrintString(textPosX, textPosY, (char*)buf.c_str());
            CRect rect;
            CFont::GetTextRect(&rect, textPosX, textPosY, (char*)buf.c_str());
            textPosY -= (rect.bottom-rect.top) / 1.5f;
            buf = word;
        }

    } while (ss);
}

void __cdecl MenuUi::DisplayStandardMenu(unsigned char panelId, bool bBrightFont)
{
    tMenuPanel* pMenuPanel = MenuNumber[panelId];
    CVector2D menuPos = {RsGlobal.maximumWidth/ 64.0f, 0};  
    float hBoxHeight = RsGlobal.maximumHeight / 30.0f * (menuStyle == MOBILE_STYLE ? 1.5f : 1.0f);
    float menuWidth = RsGlobal.maximumWidth / 3.5f;

    // ---------------------------------------------------
    // font stuff 
    CVector2D fontScale = { RsGlobal.maximumWidth * 0.00045f , RsGlobal.maximumHeight * 0.0015f };
    CVector2D textScale = { fontScale.x * textScaleMul.x, fontScale.y * textScaleMul.y };
    CVector2D titleScale = { fontScale.x * titleScaleMul.x, fontScale.y * titleScaleMul.y };

    CFont::SetFontStyle(textStyle);
    CFont::SetColor(titleColor);
    CFont::SetOrientation(eFontAlignment::ALIGN_LEFT);
    CFont::SetDropColor(CRGBA(0, 0, 0, 0));

    // ---------------------------------------------------
    // Draw background
    if (pMenuPanel->m_bColumnBackground)
    {
        // ---------------------------------------------------
        // Draw title section
        CRect headerRect;
        if (menuStyle == MOBILE_STYLE)
        {
            headerRect.right = RsGlobal.maximumWidth / 3.75f;
        }
        else
        {
            headerRect.top = pMenuPanel->m_vPosn.y;
            headerRect.left = pMenuPanel->m_vPosn.x;
            headerRect.right = menuWidth + headerRect.left;
            menuPos.x += headerRect.left;
            menuPos.y = headerRect.top;
        }
        headerRect.bottom = headerRect.top + HEADER_PADDING + hBoxHeight;

        CSprite2d::DrawRect(headerRect, titleBgColor);

        // title text
        CFont::SetOrientation(eFontAlignment::ALIGN_CENTER);
        CFont::SetFontStyle(titleStyle);
        CFont::SetScale(titleScale.x, titleScale.y);
        WrapXCenteredPrint(pMenuPanel->m_acTitle, headerRect);
        CFont::SetFontStyle(textStyle);
        CFont::SetOrientation(eFontAlignment::ALIGN_LEFT);
        
        // ---------------------------------------------------
        // Draw text section
        CRect textRect;
        textRect.top = headerRect.bottom;
        textRect.left = headerRect.left;
        textRect.right = headerRect.right;

        if (menuStyle == MOBILE_STYLE)
        {
            textRect.bottom = screen::GetScreenHeight() - hBoxHeight;
        }
        else
        {
            // numRows + 1 since we've allocated some space for the column headers
            textRect.bottom = textRect.top + (pMenuPanel->m_nNumRows * hBoxHeight); 
        }
        
        CSprite2d::DrawRect(textRect, windowBgColor);

        // ---------------------------------------------------
        // Draw footer section
        CRect footerRect;
        footerRect.top = textRect.bottom;
        footerRect.bottom = footerRect.top + hBoxHeight;
        footerRect.left = textRect.left;
        footerRect.right = textRect.right;
        CSprite2d::DrawRect(footerRect, titleBgColor);

        // ---------------------------------------------------
        // Draw selection box
        CRect selectRect;
        selectRect.left = textRect.left;
        selectRect.right = textRect.right;
        selectRect.top = headerRect.bottom + hBoxHeight * pMenuPanel->m_nSelectedRow;
        selectRect.bottom = selectRect.top + hBoxHeight;
        CSprite2d::DrawRect(selectRect, selectTextBgColor);

        // ---------------------------------------------------
    }

    for (char column = 0; column < pMenuPanel->m_nNumColumns; ++column)
    {
        // ---------------------------------------------------
        // Draw header texts
        char* pHeader = TheText.Get(pMenuPanel->m_aacColumnHeaders[column]);
        CVector2D textPos
        {
            menuPos.x + (column ? pMenuPanel->m_afColumnWidth[column - 1] : 0),
            menuPos.y + HEADER_PADDING
        };
        
        float scaleX = textScale.x;
        float fontWidth = CFont::GetStringWidth(pHeader, true, false);
        if (fontWidth > menuWidth)
        {
            scaleX = scaleX - 1 + menuWidth / fontWidth;
        }

        CFont::SetColor(titleColor);
        CFont::SetScale(scaleX, textScale.y);
        CFont::PrintString(textPos.x, textPos.y, pHeader);

        // ---------------------------------------------------
        // Draw texts
        textPos.y += hBoxHeight;

        // adding extra padding for mobile
        if (menuStyle == MOBILE_STYLE)
        {
            textPos.x += 10.0f;
            textPos.y += 5.0f;
        }

        for (char row = 0; row < pMenuPanel->m_nNumRows; ++row)
        {
            char pText[400];
            int num = pMenuPanel->m_aadwNumberInRowTitle[column][row];
            int num2 = pMenuPanel->m_aadw2ndNumberInRowTitle[column][row];
            char* row_text = TheText.Get(pMenuPanel->m_aaacRowTitles[column][row]);
            CMessages::InsertNumberInString(row_text, num, num2, -1, -1, -1, -1, pText);
            CMessages::InsertPlayerControlKeysInString(pText);

            scaleX = textScale.x;
            fontWidth = CFont::GetStringWidth(pText, true, false);
            if (fontWidth > menuWidth)
            {
                scaleX = scaleX - 1 + menuWidth / fontWidth;
            }

            CRGBA color = textColor;
            if (pMenuPanel->m_abRowSelectable[row])
            {
                if (row == pMenuPanel->m_nSelectedRow)
                {
                    color = selectTextColor;
                }

                if (pMenuPanel->m_abRowAlreadyBought[row])
                {
                    color = { 128, 128, 128, 255 };
                }
            }
            else
            {
                color = { 128, 128, 128, 255 };
            }

            CFont::SetColor(color);
            CFont::SetScale(scaleX, textScale.y);
            CFont::PrintString(textPos.x, textPos.y, pText);   
            textPos.y += hBoxHeight;
        }

        // ---------------------------------------------------
    }
}