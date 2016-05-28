/*
 * mwmmenu - a program to produce application menus for MWM and other window 
 * managers, based on freedesktop.org desktop entries.
 *
 * Copyright (C) 2015  Charles Bos
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _DESKTOP_FILE_H_
#define _DESKTOP_FILE_H_

#include <string>
#include <fstream>
#include <vector>

using namespace std;

class Categories;

class DesktopFile
{
  public:
    DesktopFile();
    DesktopFile(const char *filename, bool hideOSI, bool useIcons, vector<string> iconpaths, Categories **cats, bool noCustomCats);

    string filename;
    string name;
    string exec;
    vector<string> categories;
    bool nodisplay;
    bool onlyShowIn;
    string icon;

    static string getID(string line);
    static string getSingleValue(string line);
    static vector<string> getMultiValue(string line);
 
  private:
    ifstream dfile;

    void populate(bool hideOSI, bool useIcons, vector<string> iconpaths, Categories **cats, bool noCustomCats);
    void matchIcon(string iconDef, vector<string> iconpaths);
    void processCategories(Categories **cats, bool noCustomCats);
};

#endif
