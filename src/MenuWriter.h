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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MENU_WRITER_H_
#define _MENU_WRITER_H_

#include "DesktopFile.h"

//WM id numbers
#define mwm 0
#define fvwm 1
#define fvwm_dynamic 2
#define fluxbox 3
#define openbox 4
#define openbox_pipe 5
#define olvwm 6
#define windowmaker 7
#define icewm 8

class MenuWriter
{   
    public:
        MenuWriter(const vector<DesktopFile>& files, const string& menuName, 
                int windowmanager, bool useIcons, vector<string> exclude, 
                vector<string> excludeMatching, 
                vector<string> excludeCategories, vector<string> include, 
                vector<string> excludedFilenames, const vector<Category>& cats);

    private:
        vector<DesktopFile> files;
        vector<Category> cats;
        string menuName;
        int windowmanager;
        bool useIcons;
        vector<string> exclude;
        vector<string> excludeMatching;
        vector<string> excludeCategories;
        vector<string> include;
        vector<string> excludedFilenames;

        void printHandler();
        void entryDisplayHandler();
        bool checkExcludedCategories(const string& category);

        vector<int> getPositionsPerCat(const Category& category);

        void writeMenu(vector<int> positions, int catNumber,
                const vector<Category>& usedCats);
};

#endif
