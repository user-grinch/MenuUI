/* 
*   MenuUI - Improved Menu Interface for SA
*   Author - Grinch_
*/
#include "MenuUi.h"
#include <sstream>
#include <CSprite2d.h>
#include <CMessages.h>
#include <CText.h>
#include <filesystem>
#include <fstream>
#include <CHud.h>

RwTexture* LoadPNGTexture(std::string &&path) 
{
	if (std::filesystem::exists(path)) 
    {
	    int w, h, d, f;
		RwImage* data = RtPNGImageRead(path.c_str());
		RwImageFindRasterFormat(data, rwRASTERTYPETEXTURE, &w, &h, &d, &f);

		RwRaster* raster = RwRasterCreate(w, h, d, f);
		RwRasterSetFromImage(raster, data);
		RwImageDestroy(data);
		return RwTextureCreate(raster);
	}

	return NULL;
}

void MenuUi::InjectPatches()
{
    Events::initGameEvent += []
    {
        ReadConfig();
        ReadHeaderInfo();

        menuBoughtSprite.m_pTexture = LoadPNGTexture(PLUGIN_PATH((char*)"MenuUi/menu_bought.png"));
        menuSelectorSprite.m_pTexture = LoadPNGTexture(PLUGIN_PATH((char*)"MenuUi/menu_selector.png"));

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
    column2color  = config["COLUMN2_COL"].asRGBA(textColor);

    textScaleMul = config["TEXT_SCALE"].asVec2d({1.0f, 1.0f});
    titleScaleMul = config["TITLE_SCALE"].asVec2d({1.25f, 1.25f});

    textStyle = config["TEXT_STYLE"].asInt(1);
    titleStyle = config["TITLE_STYLE"].asInt(1);
    menuStyle = config["MENU_STYLE"].asInt(0);
    flipInterface = config["FLIP_INTERFACE"].asBool(false);
}

void MenuUi::ReadHeaderInfo()
{
    std::string root = PLUGIN_PATH((char*)"MenuUi/headers/");
    std::string path = root  + "headers.dat";

    if (std::filesystem::exists(path)) 
    {
        std::ifstream file(path);
        std::string line;

        while (getline(file, line))
        {
            if (line[0] == '#')
            {
                continue;
            }

            char gxtName[64], logoPath[64], patternPath[64];
            if (sscanf(line.c_str(), "%s %s %s", gxtName, logoPath, patternPath) == 3)
            {
                CSprite2d *pLogo = new CSprite2d();
                CSprite2d *pPattern = new CSprite2d();
                pLogo->m_pTexture = LoadPNGTexture(root + logoPath);
                pPattern->m_pTexture = LoadPNGTexture(root + patternPath);

                vecHeaders.push_back({gxtName, pLogo, pPattern});
            }
        }
    }
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
    float textPaddingX = RsGlobal.maximumWidth/ 64.0f;
    CVector2D menuPos = {textPaddingX, 0};  
    float hBoxHeight = RsGlobal.maximumHeight / 30.0f * (menuStyle == NORMAL_STYLE ? 1.0f : 1.75f);
    float menuWidth = RsGlobal.maximumWidth / 3.5f;
    float headerPadding = SCREEN_MULTIPLIER(125.0f);

    // ---------------------------------------------------
    // font stuff 
    CVector2D fontScale = { RsGlobal.maximumWidth * 0.00045f , RsGlobal.maximumHeight * 0.0015f };
    CVector2D textScale = { fontScale.x * textScaleMul.x, fontScale.y * textScaleMul.y };
    CVector2D titleScale = { fontScale.x * titleScaleMul.x, fontScale.y * titleScaleMul.y };

    CFont::SetFontStyle(textStyle);
    CFont::SetColor(titleColor);
    CFont::SetOrientation(eFontAlignment::ALIGN_LEFT);
    CFont::SetDropColor(CRGBA(0, 0, 0, 0));

    if (menuStyle == DE_STYLE)
    {
        CRect textRect;
        CFont::GetTextRect(&textRect, 0, 0, &pMenuPanel->m_acTitle[0]);
        headerPadding = textRect.top - textRect.bottom + SCREEN_MULTIPLIER(20.0f);
    }

    // hide the help message since it covers the menu top
    if (menuStyle == MOBILE_STYLE || menuStyle == DE_STYLE)
    {
        CHud::m_bHelpMessagePermanent = false;
    }

    // ---------------------------------------------------
    // Draw background
    if (pMenuPanel->m_bColumnBackground)
    {
        // ---------------------------------------------------
        // Draw title section

        // Search for header sprites
        CSprite2d *pLogo = nullptr, *pPattern = nullptr;

        if (menuStyle == NORMAL_STYLE)
        {
            for (auto &data : vecHeaders)
            {
                if (data.gxtName == pMenuPanel->m_acTitle)
                {
                    pLogo = data.m_pLogo;
                    pPattern = data.m_pPattern;
                    break; 
                }
            }
        }

        CRect headerRect;
        if (menuStyle == MOBILE_STYLE)
        {
            headerRect.top = 0.0f;
            headerRect.left = 0.0f;
            headerRect.right = RsGlobal.maximumWidth / 3.75f;
        }
        else if (menuStyle == DE_STYLE)
        {
            headerRect.top = SCREEN_MULTIPLIER(80.0f);
            headerRect.left = SCREEN_MULTIPLIER(80.0f);
            headerRect.right = RsGlobal.maximumWidth / 3.75f;
            menuPos.x += headerRect.left;
            menuPos.y = headerRect.top - hBoxHeight/2;
        }
        else
        {
            headerRect.top = pMenuPanel->m_vPosn.y;
            headerRect.left = pMenuPanel->m_vPosn.x;
            headerRect.right = menuWidth + headerRect.left;
            menuPos.x += headerRect.left;
            menuPos.y = headerRect.top;
        }

        headerRect.bottom = headerRect.top + headerPadding + hBoxHeight;
        
        // draw title sprites if available or go with text
        if (pPattern && pPattern->m_pTexture && menuStyle == NORMAL_STYLE)
        {
            pPattern->Draw(headerRect, CRGBA(255, 255, 255, 255));
        }
        else
        {
            if (menuStyle == DE_STYLE)
            {
                CRect temp = headerRect;
                temp.bottom -= hBoxHeight;
                CSprite2d::DrawRect(temp, titleBgColor);
            }
            else
            {
                CSprite2d::DrawRect(headerRect, titleBgColor);
            }
        }

        if (pLogo && pLogo->m_pTexture && menuStyle == NORMAL_STYLE)
        {
            CRect logoRect = headerRect;
            bool headers = false;

            // check if the panel got header text
            for (char column = 0; column < pMenuPanel->m_nNumColumns; ++column)
            {
                char* pText = TheText.Get(pMenuPanel->m_aacColumnHeaders[column]);

                if (pText[0] != ' ' && pText[0] != '\0')
                {
                    headers = true;
                    break;
                }
            }

            if (headers)
            {
                logoRect.bottom -= hBoxHeight;
            }

            pLogo->Draw(logoRect, CRGBA(255, 255, 255, 255));
        }
        else
        {
            CFont::SetOrientation((menuStyle == DE_STYLE)? eFontAlignment::ALIGN_LEFT : eFontAlignment::ALIGN_CENTER);
            CFont::SetFontStyle(titleStyle);
            CFont::SetScale(titleScale.x, titleScale.y);

            if (menuStyle == DE_STYLE)
            {
                float textPosY = headerRect.top + SCREEN_MULTIPLIER(10.0f);
                CFont::PrintString(headerRect.left + textPaddingX, textPosY, TheText.Get(pMenuPanel->m_acTitle));
            }
            else
            {
                WrapXCenteredPrint(pMenuPanel->m_acTitle, headerRect);
            }
            CFont::SetFontStyle(textStyle);
            CFont::SetOrientation(eFontAlignment::ALIGN_LEFT);
        }
        
        // ---------------------------------------------------
        // Draw text section
        CRect textRect;
        textRect.top = headerRect.bottom;
        textRect.left = headerRect.left;
        textRect.right = headerRect.right;

        if (menuStyle == DE_STYLE)
        {
            textRect.top = headerRect.bottom - hBoxHeight/2;
        }
        else
        {
            textRect.top = headerRect.bottom;
        }

        if (menuStyle == MOBILE_STYLE)
        {
            textRect.bottom = screen::GetScreenHeight();
        }
        else
        {
            // numRows + 1 since we've allocated some space for the column headers
            textRect.bottom = textRect.top + (pMenuPanel->m_nNumRows * hBoxHeight); 
        }
        
        CSprite2d::DrawRect(textRect, windowBgColor);

        // ---------------------------------------------------
        // Draw footer section

        if (menuStyle == NORMAL_STYLE)
        {
            CRect footerRect;
            footerRect.top = textRect.bottom;
            footerRect.bottom = footerRect.top + hBoxHeight;
            footerRect.left = textRect.left;
            footerRect.right = textRect.right;
            CSprite2d::DrawRect(footerRect, titleBgColor);

            // Draw selection sprite
            float posX = footerRect.left + (footerRect.right - footerRect.left - hBoxHeight)/2;
            menuSelectorSprite.Draw(posX, footerRect.top, hBoxHeight, hBoxHeight, textColor);
        }

        // ---------------------------------------------------
        // Draw selection box
        CRect selectRect;
        selectRect.left = textRect.left;
        selectRect.right = textRect.right;
        selectRect.top = ((headerRect.bottom + hBoxHeight * pMenuPanel->m_nSelectedRow)
            - ((menuStyle == DE_STYLE) ? hBoxHeight/2 : 0.0f));

        selectRect.bottom = selectRect.top + hBoxHeight;
        CSprite2d::DrawRect(selectRect, selectTextBgColor);

        // ---------------------------------------------------
    }

    for (char column = 0; column < pMenuPanel->m_nNumColumns; ++column)
    {
        CVector2D textPos;
        float scaleX = textScale.x;
        float fontWidth = 0.0f;

        // ---------------------------------------------------
        // Draw header texts
        
        if (menuStyle == NORMAL_STYLE)
        {
            textPos =   
            {
                menuPos.x + (column ? pMenuPanel->m_afColumnWidth[column - 1] : 0),
                menuPos.y + headerPadding
            };

            char* pHeader = TheText.Get(pMenuPanel->m_aacColumnHeaders[column]);

            fontWidth = CFont::GetStringWidth(pHeader, true, false);
            if (fontWidth > menuWidth)
            {
                scaleX = scaleX - 1 + menuWidth / fontWidth;
            }

            CFont::SetColor(titleColor);
            CFont::SetScale(scaleX, textScale.y);
            CFont::PrintString(textPos.x, textPos.y, pHeader);
            textPos.y += hBoxHeight;
        }
        else 
        {
            textPos =   
            {
                menuPos.x,
                menuPos.y + headerPadding
            };

            CRect textRect;
            CFont::GetTextRect(&textRect, 0, 0, (char*)"DummyText");

            if (column == 1)
            {
                textPos.y += hBoxHeight + textRect.top - textRect.bottom - SCREEN_MULTIPLIER(10.0f);
            }
            else // column == 0
            {
                textPos.y += hBoxHeight; 
            }
        }


        // ---------------------------------------------------
        // Draw texts
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

            CRGBA color;
            if (column == 0) // first column
            {
                color = textColor;
            }
            else // 2nd column
            {
                color = column2color;
            }

            if (pMenuPanel->m_abRowSelectable[row])
            {
                if (row == pMenuPanel->m_nSelectedRow)
                {
                    color = selectTextColor;
                }
            }
            else
            {
                // Draw checkmark
                if (menuStyle == NORMAL_STYLE && column == pMenuPanel->m_nNumColumns-1)
                {
                    float posX = textPos.x + fontWidth;
                    float posY = textPos.y;

                    if (menuStyle == MOBILE_STYLE)
                    {
                        posX += SCREEN_MULTIPLIER(15.0f);
                        posY -= SCREEN_MULTIPLIER(15.0f);
                    }
                    else
                    {
                        posX += SCREEN_MULTIPLIER(50.0f);
                        posY -= SCREEN_MULTIPLIER(2.5f);
                    }

                    menuBoughtSprite.Draw(posX, posY, hBoxHeight, hBoxHeight, grayTextColor);
                }
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