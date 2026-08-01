#pragma once

#include "vector"

struct TileMapComponent
{
    std::vector<std::vector<int>> tiles;
    int tileWidth = 0;
    int tileHeight = 0;
    int sheetColumns = 0;
};