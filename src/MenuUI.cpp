/* 
*   MenuUI - Improved Menu Interface for SA
*   Author - Grinch_
*/
#include "MenuUi.h"
#include <sstream>
#include <CSprite2d.h>
#include <CMessages.h>
#include <CText.h>

void MenuUi::InjectPatches()
{
    Events::initGameEvent += []
    {
        ReadConfig();
        patch::ReplaceFunction(0x580E00, DisplayStandardMenu);
    };
}

void MenuUi::ReadConfig()
{
    config_file config(PLUGIN_PATH((char*)"MenuUi/config.dat"));

    windowBgColor = config["WINDOW_BG"].asRGBA({0, 0, 0, 200});
    titleBgColor = config["TITLE_BG"].asRGBA({255, 255, 255, 200});
    textColor  = config["TEXT_COLOR"].asRGBA({255, 255, 255, 255});
    titleColor = config["TITLE_COLOR"].asRGBA({255, 255, 255, 255});
    selectTextColor = config["SELECT_TEXT"].asRGBA({0, 0, 0, 255});
    selectTextBgColor = config["SELECT_BG"].asRGBA({0, 0, 0, 255});
    grayTextColor  = config["GRAY_TEXT_COLOR"].asRGBA({128, 128, 128, 255});

    textScaleMul = config["TEXT_SCALE"].asVec2d({1.0f, 1.0f});
    titleScaleMul = config["TITLE_SCALE"].asVec2d({1.25f, 1.25f});

    textStyle = config["TEXT_STYLE"].asInt(1);
    titleStyle = config["TITLE_STYLE"].asInt(1);
    menuStyle = config["MENU_STYLE"].asInt(0);
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
            headerRect.top = 0.0f;
            headerRect.left = 0.0f;
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
        
        // adding extra padding for mobile
        if (menuStyle == MOBILE_STYLE)
        {
            textPos.x += 10.0f;
            textPos.y += 5.0f;
        }

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
                    color = grayTextColor;
                }
            }
            else
            {
                color = grayTextColor;
            }

            CFont::SetColor(color);
            CFont::SetScale(scaleX, textScale.y);
            CFont::PrintString(textPos.x, textPos.y, pText);   
            textPos.y += hBoxHeight;
        }

        // ---------------------------------------------------
    }
}