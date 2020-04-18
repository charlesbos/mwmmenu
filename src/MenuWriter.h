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

//Category numbers
//Any positive category numbers indicate an index in the vector of used
//categories. These are not defined here. Negative numbers indicate either a
//main menu (i.e. a category of categories) or a subcategory
#define MAIN_MENU -1
#define SUB_MENU -2

//The real maximum category number will be positive. The default here is just
//used for cases where the real maximum is irrelavent, i.e. if we're printing
// a subcategory we don't need to know how many base categories there are. The
//default max must be negative and must not conflict with any category numbers
//defined above
#define DEFAULT_MAX_CAT -1000

class MenuWriter
{   
    public:
        MenuWriter(const string& menuName, int windowmanager, bool useIcons, 
                vector<string> exclude, vector<string> excludeMatching, 
                vector<string> excludeCategories, vector<string> include, 
                vector<string> excludedFilenames, const vector<Category*>& cats);

    private:
        vector<Category*> cats;
        string menuName;
        int windowmanager;
        bool useIcons;
        vector<string> exclude;
        vector<string> excludeMatching;
        vector<string> excludeCategories;
        vector<string> include;
        vector<string> excludedFilenames;

        void entryDisplayHandler();
        bool categoryNotExcluded(Category* c);
        int realMenuSize(vector<DesktopFile*> entries);

        void writeMenu(Category *cat, int catNumber, 
                const vector<Category*>& usedCats, 
                int maxCatNumber = DEFAULT_MAX_CAT);
};

template <typename T> bool myCompare(T *a, T *b)
{
    string name_a = a->name;
    string name_b = b->name;
    for (unsigned int x = 0; x < name_a.size(); x++) 
        name_a.at(x) = tolower(name_a.at(x));
    for (unsigned int x = 0; x < name_b.size(); x++) 
        name_b.at(x) = tolower(name_b.at(x));
    if (name_a < name_b) return true;
    else return false;
}

#endif
