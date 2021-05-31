/* 
*   MenuUI - Improved Menu Interface for SA
*   Author - Grinch_
*/

#include "plugin.h"
#include "CMenuManager.h"
#include "CMenuSystem.h"
#include "CMessages.h"
#include "CText.h"
#include "INIReader.h"
#include <sstream>

#define uchar unsigned char
#define WINDOW_ALPHA 200
#define TITLE_ALPHA 250

using namespace plugin;

void __cdecl DisplayStandardMenu(unsigned __int8 panelId, bool bBrightFont);
INIReader ini(PLUGIN_PATH((char*)"MenuUI.ini"));

short text_font = (short)ini.GetInteger("font", "text", 1);
short title_font = (short)ini.GetInteger("font", "title", 1);

CVector2D text_scale_mul{ ini.GetFloat("font", "text_scaleX", 1.0f) , ini.GetFloat("font", "text_scaleX", 1.0f) };
CVector2D title_scale_mul{ ini.GetFloat("font", "title_scaleX", 1.0f) , ini.GetFloat("font", "title_scaleX", 1.0f) };

CRGBA window_bg_color =
{
    (uchar)ini.GetInteger("color", "window_bg_red", 0),
    (uchar)ini.GetInteger("color", "window_bg_green", 0),
    (uchar)ini.GetInteger("color", "window_bg_blue", 0),
    WINDOW_ALPHA
};

CRGBA title_bg_color =
{
    (uchar)ini.GetInteger("color", "window_bg_red", 0),
    (uchar)ini.GetInteger("color", "window_bg_green", 0),
    (uchar)ini.GetInteger("color", "window_bg_blue", 0),
    TITLE_ALPHA
};

CRGBA text_color =
{
    (uchar)ini.GetInteger("color", "text_red", 0),
    (uchar)ini.GetInteger("color", "text_green", 0),
    (uchar)ini.GetInteger("color", "text_blue", 0),
    WINDOW_ALPHA
};

CRGBA title_color =
{
    (uchar)ini.GetInteger("color", "title_red", 0),
    (uchar)ini.GetInteger("color", "title_green", 0),
    (uchar)ini.GetInteger("color", "title_blue", 0),
    TITLE_ALPHA
};

CRGBA select_text_color =
{
    (uchar)ini.GetInteger("color", "select_text_red", 0),
    (uchar)ini.GetInteger("color", "select_text_green", 0),
    (uchar)ini.GetInteger("color", "select_text_blue", 0),
    WINDOW_ALPHA
};

CRGBA select_text_bg_color =
{
    (uchar)ini.GetInteger("color", "select_text_bg_red", 0),
    (uchar)ini.GetInteger("color", "select_text_bg_green", 0),
    (uchar)ini.GetInteger("color", "select_text_bg_blue", 0),
    WINDOW_ALPHA
};

class MenuSystemUI {
public:
    MenuSystemUI() {
        // Initialise your plugin here
        Events::initGameEvent += []
        {
            patch::ReplaceFunction(0x580E00, DisplayStandardMenu);
        };
    }
} menuSystemUI;

void WrapXCenteredPrint(char* pText, float window_width, float text_posX, float text_posY)
{
    float font_width = CFont::GetStringWidth(pText, true, false);

    if (font_width <= window_width)
    {
        CFont::PrintString(text_posX, text_posY, pText);
        return;
    }

    std::istringstream ss(pText);
    std::string buf, temp;
    do {
        std::string word;
        ss >> word;

        temp += (temp == "") ? word : (" " + word);
        font_width = CFont::GetStringWidth((char*)temp.c_str(), true, false);

        if (font_width < window_width && word != "")
            buf += " " + word;
        else
        {
            temp = word;
            CFont::PrintString(text_posX, text_posY, (char*)buf.c_str());
            CRect rect;
            CFont::GetTextRect(&rect, text_posX, text_posY, (char*)buf.c_str());
            text_posY -= (rect.bottom-rect.top) / 1.5f;
            buf = word;
        }

    } while (ss);
}

void __cdecl DisplayStandardMenu(unsigned __int8 panelId, bool bBrightFont)
{
    CRect window_size;
    float header_padding = 0;
    tMenuPanel* hMenu = MenuNumber[panelId];
    float tcolumn_width = 0.0f;
    float window_width = 999.0f;
    float hbox_height = RsGlobal.maximumHeight * 0.035712f - 3.0f;
    float hbox_left = RsGlobal.maximumWidth * 0.015625f + hMenu->m_vPosn.x;
    bool draw_headers = false;

    CVector2D font_scale = { RsGlobal.maximumWidth * 0.00045f , RsGlobal.maximumHeight * 0.0015f };
    CVector2D text_scale = { font_scale.x * text_scale_mul.x,font_scale.y * text_scale_mul.y };
    CVector2D title_scale = { font_scale.x * title_scale_mul.x,font_scale.y * title_scale_mul.y };

    // default font stuff
    CFont::SetFontStyle(text_font);
    CFont::SetColor(title_color);
    CFont::SetOrientation(eFontAlignment::ALIGN_LEFT);
    CFont::SetDropColor(CRGBA(0, 0, 0, 0));

    // check if headers contain text
    for (char column = 0; column < hMenu->m_nNumColumns; ++column)
    {
        char* pText = TheText.Get(hMenu->m_aacColumnHeaders[column]);
        if (pText[0] != ' ' && pText[0] != '\0')
            draw_headers = true;

        tcolumn_width += hMenu->m_afColumnWidth[column];
    }

    // Draw background
    if (hMenu->m_bColumnBackground)
    {
        size_t row_draw_count = hMenu->m_nNumRows;
        // header
        header_padding = 75;
        window_size.left = hMenu->m_vPosn.x;
        window_size.right = RsGlobal.maximumWidth / 3.5f + window_size.left;

        float original_right = RsGlobal.maximumWidth * 0.03124f + hMenu->m_vPosn.x;
        float offset = (original_right - window_size.right) / 4;
        hbox_left -= offset;
        window_size.left -= offset;
        window_size.right -= offset;

        window_size.top = hMenu->m_vPosn.y;
        window_size.bottom = window_size.top + header_padding;
        
        if (draw_headers)
        {
            window_size.bottom += hbox_height;
            row_draw_count += 1;
        }

        // window
        FrontEndMenuManager.DrawWindow(window_size, "", 0, title_bg_color, 0, 1);

        char* pText = TheText.Get(hMenu->m_acTitle);
        float text_posX = window_size.left + (window_size.right - window_size.left) / 2;
        float text_posY = window_size.top + (window_size.bottom - window_size.top) / 2 - 20.0f;
        window_width = window_size.right - window_size.left;

        CFont::SetOrientation(eFontAlignment::ALIGN_CENTER);
        CFont::SetFontStyle(title_font);
        CFont::SetScale(title_scale.x, title_scale.y);
        WrapXCenteredPrint(pText, window_width, text_posX, text_posY);
        CFont::SetFontStyle(text_font);
        CFont::SetOrientation(eFontAlignment::ALIGN_LEFT);
        
        window_size.top += header_padding;
        window_size.bottom = window_size.top + (row_draw_count * hbox_height);
        FrontEndMenuManager.DrawWindow(window_size, "", 0, window_bg_color, 0, 1);

        // select box
        float select_top = hMenu->m_vPosn.y + header_padding + hbox_height * hMenu->m_nSelectedRow;
        CRect pos;
        pos.left = window_size.left;
        pos.right = window_size.right;
        pos.top = select_top + hbox_height;
        pos.bottom = select_top;

        if (draw_headers)
        {
            pos.top += hbox_height;
            pos.bottom += hbox_height;
        }
        FrontEndMenuManager.DrawWindow(pos, "", 0, select_text_bg_color, 0, 1);

        // footer
        window_size.top = window_size.bottom;
        window_size.bottom += hbox_height;
        FrontEndMenuManager.DrawWindow(window_size, "", 0, title_bg_color, 0, 1);
    }

    // draw text
    for (char column = 0; column < hMenu->m_nNumColumns; ++column)
    {
        char* pHeader = TheText.Get(hMenu->m_aacColumnHeaders[column]);
        CVector2D text_pos
        {
            hbox_left + (column == 0 ? 0 : hMenu->m_afColumnWidth[column - 1]),
            hMenu->m_vPosn.y + header_padding
        };
        float scaleX = text_scale.x;
        float font_width = CFont::GetStringWidth(pHeader, true, false);
        float text_area_width = window_width - 25.0f;
        if (font_width > text_area_width)
            scaleX = scaleX - 1 + text_area_width / font_width;

        CFont::SetColor(title_color);
        CFont::SetScale(scaleX, text_scale.y);
        CFont::PrintString(text_pos.x, text_pos.y, pHeader);

        if (draw_headers)
            text_pos.y += hbox_height;

        for (char row = 0; row < hMenu->m_nNumRows; ++row)
        {
            char pText[400];
            int num = hMenu->m_aadwNumberInRowTitle[column][row];
            int num2 = hMenu->m_aadw2ndNumberInRowTitle[column][row];
            char* row_text = TheText.Get(hMenu->m_aaacRowTitles[column][row]);
            CMessages::InsertNumberInString(row_text, num, num2, -1, -1, -1, -1, pText);
            CMessages::InsertPlayerControlKeysInString(pText);
            scaleX = text_scale.x;
            font_width = CFont::GetStringWidth(pText, true, false);

            if (font_width > text_area_width)
                scaleX = scaleX - 1 + text_area_width / font_width;

            CRGBA color = text_color;

            if (hMenu->m_abRowSelectable[row])
            {
                if (row == hMenu->m_nSelectedRow)
                    color = select_text_color;

                if (hMenu->m_abRowAlreadyBought[row])
                    color = { 128, 128, 128, WINDOW_ALPHA };
            }
            else
                color = { 128, 128, 128, WINDOW_ALPHA };

            CFont::SetColor(color);
            CFont::SetScale(scaleX, text_scale.y);
            CFont::PrintString(text_pos.x, text_pos.y, pText);   
            text_pos.y += hbox_height;
        }
    }
}
