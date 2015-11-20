/*
mwmmenu - a program to produce menus for the Motif Window Manager, based on
freedesktop.org desktop entries,

Copyright (C) 2015  Charles Bos

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _MWM_MENU_WRITER_H_
#define _MWM_MENU_WRITER_H_

#include <fstream>
#include "DesktopFile.h"

class MwmMenuWriter
{ public:
    MwmMenuWriter(DesktopFile **files, int filesLength, string menuName);

  private:
    DesktopFile **files;
    int filesLength;
    string menuName;

    void printHandler();

    vector< pair<int,string> > getPositionsPerCat(string category);
    int getLongestNameLength();

    void writeMwmCategoryMenu(vector< pair<int,string> > positions, string category);
    void writeMwmMainMenu(string menuName, const char *usedCats[], int catNumber);
};

#endif