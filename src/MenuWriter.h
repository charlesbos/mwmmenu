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

#ifndef _MENU_WRITER_H_
#define _MENU_WRITER_H_

#include "DesktopFile.h"

class MenuWriter
{ public:
    MenuWriter(DesktopFile **files, 
               int filesLength, 
               string menuName, 
               string windowmanager, 
               bool useIcons, 
               vector<string> iconpaths, 
               vector<string> exclude, 
               vector<string> excludeMatching, 
               vector<string> excludeCategories, 
               string iconSize, 
               vector<string> include, 
               vector<string> excludedFilenames, 
               Category **cats, 
               int customCatNum);

  private:
    DesktopFile **files;
    Category **cats;
    int customCatNum;
    int filesLength;
    string menuName;
    string windowmanager;
    bool useIcons;
    vector<string> iconpaths;
    vector<string> exclude;
    vector<string> excludeMatching;
    vector<string> excludeCategories;
    string iconSize;
    vector<string> include;
    vector<string> excludedFilenames;

    void printHandler();
    void entryDisplayHandler();
    bool checkExcludedCategories(string category);

    vector< pair<int,string> > getPositionsPerCat(string category);
    int getLongestNameLength();

    int getWmID();
    string getCategoryIcon(string catName);

    void writeMenu(vector< pair<int,string> > positions, string category, int wmID, int catNumber, int maxCatNumber, int longest, vector<string> usedCats);
};

#endif
