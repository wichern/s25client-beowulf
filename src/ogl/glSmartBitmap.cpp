// Copyright (c) 2005 - 2015 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

///////////////////////////////////////////////////////////////////////////////
// Header
#include "defines.h" // IWYU pragma: keep
#include "glSmartBitmap.h"
#include "drivers/VideoDriverWrapper.h"

#include "Loader.h"

// Include last!
#include "DebugNew.h" // IWYU pragma: keep

void glSmartBitmap::reset()
{
    if (texture && !sharedTexture)
    {
        VIDEODRIVER.DeleteTexture(texture);
    }

    for(std::vector<glBitmapItem>::iterator it = items.begin(); it != items.end(); ++it)
    {
        if(it->isOwning_)
            delete it->bmp;
    }
    items.clear();

    texture = 0;
}

glSmartBitmap::~glSmartBitmap()
{
    reset();
}

unsigned glSmartBitmap::nextPowerOfTwo(unsigned k)
{
    if (k == 0)
    {
        return(1);
    }

    k--;

    for (unsigned i = 1; i < sizeof(unsigned)*CHAR_BIT; i <<= 1)
    {
        k = k | k >> i;
    }

    return(k + 1);
}

void glSmartBitmap::calcDimensions()
{
    if (items.empty())
    {
        nx = ny = 0;
        w = h = 0;
        return;
    }

    int max_x = 0;
    int max_y = 0;

    nx = ny = INT_MIN;

    hasPlayer = false;

    for (std::vector<glBitmapItem>::const_iterator it = items.begin(); it != items.end(); ++it)
    {
        if (it->type == TYPE_ARCHIVITEM_BITMAP_PLAYER)
        {
            hasPlayer = true;
        }

        if (nx < it->nx)
        {
            nx = it->nx;
        }

        if (ny < it->ny)
        {
            ny = it->ny;
        }

        if (max_x < it->w - it->nx)
        {
            max_x = it->w - it->nx;
        }

        if (max_y < it->h - it->ny)
        {
            max_y = it->h - it->ny;
        }
    }

    w = nx + max_x;
    h = ny + max_y;
}

void glSmartBitmap::drawTo(unsigned char* buffer, unsigned stride, unsigned height, int x_offset, int y_offset)
{
    libsiedler2::ArchivItem_Palette* p_colors = LOADER.GetPaletteN("colors");
    libsiedler2::ArchivItem_Palette* p_5 = LOADER.GetPaletteN("pal5");

    for (std::vector<glBitmapItem>::const_iterator it = items.begin(); it != items.end(); ++it)
    {
        if ((it->w == 0) || (it->h == 0))
        {
            continue;
        }

        unsigned xo = (nx - it->nx);
        unsigned yo = (ny - it->ny);

        switch (it->type)
        {
            case TYPE_ARCHIVITEM_BITMAP:
            {
                dynamic_cast<libsiedler2::baseArchivItem_Bitmap*>(it->bmp)
                        ->print(buffer, stride, height, libsiedler2::FORMAT_RGBA, p_5, xo + x_offset, yo + y_offset, it->x, it->y, it->w, it->h);

                break;
            }
            case TYPE_ARCHIVITEM_BITMAP_PLAYER:
            {
                libsiedler2::ArchivItem_Bitmap_Player* bmp = dynamic_cast<libsiedler2::ArchivItem_Bitmap_Player*>(it->bmp);

                bmp->print(buffer, stride, height, libsiedler2::FORMAT_RGBA, p_colors, 128,
                           xo + x_offset, yo + y_offset, it->x, it->y, it->w, it->h, false);

                bmp->print(buffer, stride, height, libsiedler2::FORMAT_RGBA, p_colors, 128,
                           xo + w + x_offset, yo + y_offset, it->x, it->y, it->w, it->h, true);

                break;
            }
            case TYPE_ARCHIVITEM_BITMAP_SHADOW:
            {
                std::vector<unsigned char> tmp(w * h * 4);

                dynamic_cast<libsiedler2::baseArchivItem_Bitmap*>(it->bmp)
                        ->print(&tmp.front(), w, h, libsiedler2::FORMAT_RGBA, p_5, xo, yo, it->x, it->y, it->w, it->h);

                unsigned tmpIdx = 0;

                for (int y = 0; y < h; ++y)
                {
                    unsigned idx = ((y_offset + y) * stride + x_offset) << 2;

                    for (int x = 0; x < w; ++x)
                    {
                        if (tmp[tmpIdx + 3] != 0x00)
                        {
                            if (buffer[idx + 3] == 0x00)
                            {
                                buffer[idx] = 0x00;
                                buffer[idx + 1] = 0x00;
                                buffer[idx + 2] = 0x00;
                                buffer[idx + 3] = 0x40;
                            }
                        }

                        idx += 4;
                        tmpIdx += 4;
                    }
                }

                break;
            }
            default:
                break;
        }
    }
}

void glSmartBitmap::generateTexture()
{
    if (items.empty())
    {
        return;
    }

    if (!texture)
    {
        texture = VIDEODRIVER.GenerateTexture();

        if (!texture)
        {
            return;
        }
    }

    calcDimensions();

    w = nextPowerOfTwo(w);
    h = nextPowerOfTwo(h);

    // do we have a player-colored overlay?
    unsigned stride = hasPlayer ? w * 2 : w;

    std::vector<unsigned char> buffer(stride * h * 4);
    drawTo(&buffer.front(), stride, h);

    VIDEODRIVER.BindTexture(texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, stride, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, &buffer.front());

    tmpTexData[0].tx = tmpTexData[1].tx = 0.0f;
    tmpTexData[2].tx = tmpTexData[3].tx = hasPlayer ? 0.5f : 1.0f;

    tmpTexData[0].ty = tmpTexData[3].ty = tmpTexData[4].ty = tmpTexData[7].ty = 0.0f;
    tmpTexData[1].ty = tmpTexData[2].ty = tmpTexData[5].ty = tmpTexData[6].ty = 1.0f;

    tmpTexData[4].tx = tmpTexData[5].tx = 0.5f;
    tmpTexData[6].tx = tmpTexData[7].tx = 1.0f;
}

void glSmartBitmap::draw(int x, int y, unsigned color, unsigned player_color)
{
    if (!texture)
    {
        generateTexture();

        if (!texture)
        {
            return;
        }
    }

    tmpTexData[0].x = tmpTexData[1].x = GLfloat(x - nx);
    tmpTexData[2].x = tmpTexData[3].x = GLfloat(x - nx + w);

    tmpTexData[0].y = tmpTexData[3].y = GLfloat(y - ny);
    tmpTexData[1].y = tmpTexData[2].y = GLfloat(y - ny + h);

    tmpTexData[0].r = tmpTexData[1].r = tmpTexData[2].r = tmpTexData[3].r = GetRed(color);
    tmpTexData[0].g = tmpTexData[1].g = tmpTexData[2].g = tmpTexData[3].g = GetGreen(color);
    tmpTexData[0].b = tmpTexData[1].b = tmpTexData[2].b = tmpTexData[3].b = GetBlue(color);
    tmpTexData[0].a = tmpTexData[1].a = tmpTexData[2].a = tmpTexData[3].a = GetAlpha(color);

    int numQuads;
    if ((player_color != 0x00000000) && hasPlayer)
    {
        tmpTexData[4].x = tmpTexData[5].x = tmpTexData[0].x;
        tmpTexData[6].x = tmpTexData[7].x = tmpTexData[2].x;
        tmpTexData[4].y = tmpTexData[7].y = tmpTexData[0].y;
        tmpTexData[5].y = tmpTexData[6].y = tmpTexData[1].y;

        tmpTexData[4].r = tmpTexData[5].r = tmpTexData[6].r = tmpTexData[7].r = GetRed(player_color);
        tmpTexData[4].g = tmpTexData[5].g = tmpTexData[6].g = tmpTexData[7].g = GetGreen(player_color);
        tmpTexData[4].b = tmpTexData[5].b = tmpTexData[6].b = tmpTexData[7].b = GetBlue(player_color);
        tmpTexData[4].a = tmpTexData[5].a = tmpTexData[6].a = tmpTexData[7].a = GetAlpha(player_color);

        numQuads = 8;
    } else
        numQuads = 4;

    glInterleavedArrays(GL_T2F_C4UB_V3F, 0, tmpTexData);
    VIDEODRIVER.BindTexture(texture);
    glDrawArrays(GL_QUADS, 0, numQuads);
}

void glSmartBitmap::drawPercent(int x, int y, unsigned percent, unsigned color, unsigned player_color)
{
    if (!texture)
    {
        generateTexture();

        if (!texture)
        {
            return;
        }
    }

    // nothing to draw?
    if (!percent)
    {
        return;
    }

    tmpTexData[0].x = tmpTexData[1].x = GLfloat(x - nx);
    tmpTexData[2].x = tmpTexData[3].x = GLfloat(x - nx + w);

    tmpTexData[0].y = tmpTexData[3].y = GLfloat(y - ny + h - h * (float) percent / 100.0f);
    tmpTexData[1].y = tmpTexData[2].y = GLfloat(y - ny + h);

    tmpTexData[0].r = tmpTexData[1].r = tmpTexData[2].r = tmpTexData[3].r = GetRed(color);
    tmpTexData[0].g = tmpTexData[1].g = tmpTexData[2].g = tmpTexData[3].g = GetGreen(color);
    tmpTexData[0].b = tmpTexData[1].b = tmpTexData[2].b = tmpTexData[3].b = GetBlue(color);
    tmpTexData[0].a = tmpTexData[1].a = tmpTexData[2].a = tmpTexData[3].a = GetAlpha(color);

    float cache = tmpTexData[0].ty;

    tmpTexData[0].ty = tmpTexData[3].ty = tmpTexData[1].ty - (tmpTexData[1].ty - tmpTexData[0].ty) * (float) percent / 100.0f;

    if ((player_color != 0x00000000) && hasPlayer)
    {
        tmpTexData[4].x = tmpTexData[5].x = tmpTexData[0].x;
        tmpTexData[6].x = tmpTexData[7].x = tmpTexData[2].x;
        tmpTexData[4].y = tmpTexData[7].y = tmpTexData[0].y;
        tmpTexData[5].y = tmpTexData[6].y = tmpTexData[1].y;

        tmpTexData[4].r = tmpTexData[5].r = tmpTexData[6].r = tmpTexData[7].r = GetRed(player_color);
        tmpTexData[4].g = tmpTexData[5].g = tmpTexData[6].g = tmpTexData[7].g = GetGreen(player_color);
        tmpTexData[4].b = tmpTexData[5].b = tmpTexData[6].b = tmpTexData[7].b = GetBlue(player_color);
        tmpTexData[4].a = tmpTexData[5].a = tmpTexData[6].a = tmpTexData[7].a = GetAlpha(player_color);

        tmpTexData[4].ty = tmpTexData[7].ty = tmpTexData[3].ty;

        glInterleavedArrays(GL_T2F_C4UB_V3F, 0, tmpTexData);
        VIDEODRIVER.BindTexture(texture);
        glDrawArrays(GL_QUADS, 0, 8);

        tmpTexData[0].ty = tmpTexData[3].ty = tmpTexData[4].ty = tmpTexData[7].ty = cache;

        return;
    }

    glInterleavedArrays(GL_T2F_C4UB_V3F, 0, tmpTexData);
    VIDEODRIVER.BindTexture(texture);
    glDrawArrays(GL_QUADS, 0, 4);

    tmpTexData[0].ty = tmpTexData[3].ty = cache;

}

