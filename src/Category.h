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

#ifndef _CATEGORY_H_
#define _CATEGORY_H_

#include "DesktopFile.h"

class Category
{
    public:
        Category(const char *dirFile, const vector<string>& menuFiles, 
                bool useIcons, const vector<IconSpec>& iconpaths, 
                const string& iconsXdgSize, bool iconsXdgOnly);
        Category(vector<string> menuDef, const char *dirFile,
                bool useIcons, const vector<IconSpec>& iconpaths, 
                const string& iconsXdgSize, bool iconsXdgOnly, int depth);
        Category(const string& name, bool useIcons, 
                const vector<IconSpec>& iconpaths, const string& iconsXdgSize, 
                bool iconsXdgOnly);
        
        string name;
        string icon;
        int depth;
        bool nodisplay;

        vector<DesktopFile*> getEntries();
        vector<DesktopFile*> getEntriesR();
        vector<Category*> getSubcats();
        vector<string> getIncludes();
        vector<string> getExcludes();

        static string getID(const string& line);
        static string getSingleValue(const string& line);

        bool registerDF(DesktopFile *df, bool force = false);

    private:
        string dirFile;
        vector<string> menuFiles;
        ifstream dir_f;
        ifstream menu_f;
        vector<string> validNames;
        vector<IconSpec> iconpaths;
        string iconsXdgSize;
        bool iconsXdgOnly;
        bool useIcons;
        vector<DesktopFile*> incEntries;
        vector<Category*> incCategories;
        vector<string> incEntryFiles;
        vector<string> excEntryFiles;

        static int registerCount;
        static vector<DesktopFile*> incEntriesR;

        void registerDF(Category *cat, DesktopFile *df, bool force = false);
        void getEntriesR(Category *cat);

        void readMenufiles();
        void parseDir();
        void parseMenu(const vector<string>& menu);
        void getCategoryIcon();
};

#endif
