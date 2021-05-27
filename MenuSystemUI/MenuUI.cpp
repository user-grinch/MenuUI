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
#define MENU_ALPHA 200

using namespace plugin;

void __cdecl DisplayStandardMenu(unsigned __int8 panelId, bool bBrightFont);
INIReader ini(PLUGIN_PATH((char*)"MenuUI.ini"));

int font_type = ini.GetInteger("font", "type", 1);
int title_font_type = ini.GetInteger("font", "title_type", 1);
float title_mulX = ini.GetFloat("font", "title_mulX", 1.5f);
float title_mulY = ini.GetFloat("font", "title_mulY", 1.5f);
float normal_scaleX = ini.GetFloat("font", "normal_scaleX", 1.0f);
float normal_scaleY = ini.GetFloat("font", "normal_scaleY", 1.0f);

float box_scaleY = ini.GetFloat("highlight", "box_scaleY", 1.0f);

uchar accent_color_red = (uchar)ini.GetInteger("color", "accent_color_red", 0);
uchar accent_color_green = (uchar)ini.GetInteger("color", "accent_color_green", 0);
uchar accent_color_blue = (uchar)ini.GetInteger("color", "accent_color_blue", 0);

uchar normal_text_red = (uchar)ini.GetInteger("color", "normal_text_red", 0);
uchar normal_text_green = (uchar)ini.GetInteger("color", "normal_text_green", 0);
uchar normal_text_blue = (uchar)ini.GetInteger("color", "normal_text_blue", 0);

uchar title_text_red = (uchar)ini.GetInteger("color", "title_text_red", 0);
uchar title_text_green = (uchar)ini.GetInteger("color", "title_text_green", 0);
uchar title_text_blue = (uchar)ini.GetInteger("color", "title_text_blue", 0);

uchar title_box_red = (uchar)ini.GetInteger("color", "title_box_red", 0);
uchar title_box_green = (uchar)ini.GetInteger("color", "title_box_green", 0);
uchar title_box_blue = (uchar)ini.GetInteger("color", "title_box_blue", 0);

uchar highlight_text_red = (uchar)ini.GetInteger("color", "highlight_text_red", 0);
uchar highlight_text_green = (uchar)ini.GetInteger("color", "highlight_text_green", 0);
uchar highlight_text_blue = (uchar)ini.GetInteger("color", "highlight_text_blue", 0);

uchar highlight_box_red = (uchar)ini.GetInteger("color", "highlight_box_red", 0);
uchar highlight_box_green = (uchar)ini.GetInteger("color", "highlight_box_green", 0);
uchar highlight_box_blue = (uchar)ini.GetInteger("color", "highlight_box_blue", 0);

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
            text_posY -= (rect.bottom-rect.top) / 1.3f;
            buf = word;
        }

    } while (ss);
}

void __cdecl DisplayStandardMenu(unsigned __int8 panelId, bool bBrightFont)
{
    CRect window_size;
    float header_padding = 0;
    tMenuPanel* hMenu = MenuNumber[panelId];
    size_t column_count = hMenu->m_nNumColumns;
    float tcolumn_width = 0.0f;
    float window_width = 999.0f;
    size_t row_count = hMenu->m_nNumRows;
    float font_scaleX = RsGlobal.maximumWidth * 0.00055f * normal_scaleX;
    float font_scaleY = RsGlobal.maximumHeight * 0.0015f * normal_scaleY;
    float hbox_height = (RsGlobal.maximumHeight * 0.035712f - 3.0f) * box_scaleY;
    float hbox_left = RsGlobal.maximumWidth * 0.015625f + hMenu->m_vPosn.x;
    bool draw_headers = false;

    // default font stuff
    CFont::SetFontStyle(font_type);
    CFont::SetColor(CRGBA(title_text_red, title_text_green, title_text_blue, 255));
    CFont::SetOrientation(eFontAlignment::ALIGN_LEFT);
    CFont::SetScale(font_scaleX, font_scaleY);
    CFont::SetDropColor(CRGBA(0, 0, 0, 0));

    // check if headers contain text
    for (size_t column = 0; column < column_count; ++column)
    {
        char* pText = TheText.Get(hMenu->m_aacColumnHeaders[column]);
        if (pText[0] != ' ' && pText[0] != '\0')
        {
            draw_headers = true;
        }
        tcolumn_width += hMenu->m_afColumnWidth[column];
    }

    // Draw menu background
    if (hMenu->m_bColumnBackground)
    {
        size_t row_draw_count = row_count;
        // header
        header_padding = 75;
        window_size.left = hMenu->m_vPosn.x;
        window_size.right = RsGlobal.maximumWidth / 3.75f + window_size.left;

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
            ++row_draw_count;
        }

        // window
        FrontEndMenuManager.DrawWindow(window_size, "", 0, CRGBA(title_box_red, title_box_green, title_box_blue, 250), 0, 1);

        char* pText = TheText.Get(hMenu->m_acTitle);
        float text_posX = window_size.left + (window_size.right - window_size.left) / 2;
        float text_posY = window_size.top + (window_size.bottom - window_size.top) / 2 - 20.0f;
        window_width = window_size.right - window_size.left;

        CFont::SetOrientation(eFontAlignment::ALIGN_CENTER);
        CFont::SetFontStyle(title_font_type);
        CFont::SetScale(font_scaleX * title_mulX, font_scaleY * title_mulY);
        WrapXCenteredPrint(pText, window_width, text_posX, text_posY);
        CFont::SetScale(font_scaleX, font_scaleY);
        CFont::SetFontStyle(font_type);
        CFont::SetOrientation(eFontAlignment::ALIGN_LEFT);
        
        window_size.top += header_padding;
        window_size.bottom = window_size.top + (row_draw_count * hbox_height);
        FrontEndMenuManager.DrawWindow(window_size, "", 0, CRGBA(accent_color_red, accent_color_green, accent_color_blue, MENU_ALPHA), 0, 1);

        // footer
        window_size.top = window_size.bottom;
        window_size.bottom += hbox_height;
        FrontEndMenuManager.DrawWindow(window_size, "", 0, CRGBA(title_box_red, title_box_green, title_box_blue, 250), 0, 1);
    }

    float hbox_top =  hMenu->m_vPosn.y + header_padding;

    // Draw Headers
    if (draw_headers)
    {
        float text_top = hbox_top;
        for (size_t column = 0; column < column_count; ++column)
        {
            char* pText = TheText.Get(hMenu->m_aacColumnHeaders[column]);
            float width_offset = column == 0 ? 0 : hMenu->m_afColumnWidth[column - 1] + 50.0f;
            float scaleX = font_scaleX;
            float font_width = CFont::GetStringWidth(pText, true, false);

            if (font_width > window_width)
                scaleX = font_scaleX - 1 + window_width / font_width;

            CFont::SetScale(scaleX, font_scaleY);
            CFont::PrintString(hbox_left + width_offset, text_top, pText);
            CFont::SetScale(font_scaleX, font_scaleY);
            hbox_top += hbox_height;
        }
        hbox_top -= hbox_height; // fix
    }

    // Draw row & columns
    for (size_t row = 0; row < row_count; ++row)
    {
        // setup row text
        char pText[400];

        for (size_t column = 0; column < column_count; ++column)
        {
            char* row_gxt = hMenu->m_aaacRowTitles[column][row];

            if (row_gxt)
            {
                int num = hMenu->m_aadwNumberInRowTitle[column][row];
                int num2 = hMenu->m_aadw2ndNumberInRowTitle[column][row];
                char* row_title = TheText.Get(row_gxt);
                CMessages::InsertNumberInString(row_title, num, num2, -1, -1, -1, -1, pText);
                CMessages::InsertPlayerControlKeysInString(pText);
                bool high_light = false;
                CRGBA high_light_color = { highlight_box_red, highlight_box_green, highlight_box_blue, MENU_ALPHA };
                CRGBA color = { normal_text_red, normal_text_green, normal_text_blue, MENU_ALPHA };

                if (hMenu->m_abRowSelectable[row])
                {
                    if (row == hMenu->m_nSelectedRow)
                    {
                        high_light = true;
                        color = { highlight_text_red, highlight_text_green, highlight_text_blue, MENU_ALPHA };
                    }

                    if (hMenu->m_abRowAlreadyBought[row])
                    {
                        color = { 128, 128, 128, MENU_ALPHA };
                    }
                }
                else
                    color = { 128, 128, 128, MENU_ALPHA };

                CFont::SetColor(color);
                float width_offset = column == 0 ? 0 : hMenu->m_afColumnWidth[column - 1] + 50.0f;

                if (high_light)
                {
                    CRect size;
                    CFont::GetTextRect(&size, hbox_left + width_offset, hbox_top, pText);

                    float text_height = size.bottom - size.top + 5.0f;
                    CRect pos;
                    pos.left = window_size.left;
                    pos.right = window_size.right;
                    pos.top = hbox_top + hbox_height;
                    pos.bottom = hbox_top;
                    FrontEndMenuManager.DrawWindow(pos, "", 0, high_light_color, 0, 1);
                }
                float scaleX = font_scaleX;
                float font_width = CFont::GetStringWidth(pText, true, false);

                if (font_width > window_width)
                {
                    scaleX = font_scaleX - 1 + window_width / font_width;
                }
                
                CFont::SetScale(scaleX, font_scaleY);
                CFont::PrintString(hbox_left + width_offset, hbox_top, pText);
                CFont::SetScale(font_scaleX, font_scaleY);
            }
        }
        hbox_top += hbox_height;
    }
}
