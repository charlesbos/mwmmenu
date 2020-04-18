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
        Category(const char *dirFile, const std::vector<std::string>& menuFiles, 
                bool useIcons, const std::vector<IconSpec>& iconpaths, 
                const std::string& iconsXdgSize, bool iconsXdgOnly);
        Category(std::vector<std::string> menuDef, const char *dirFile,
                bool useIcons, const std::vector<IconSpec>& iconpaths, 
                const std::string& iconsXdgSize, bool iconsXdgOnly, int depth);
        Category(const std::string& name, bool useIcons, 
                const std::vector<IconSpec>& iconpaths, const std::string& iconsXdgSize, 
                bool iconsXdgOnly);
        
        std::string name;
        std::string icon;
        int depth;
        bool nodisplay;

        std::vector<DesktopFile*> getEntries();
        std::vector<DesktopFile*> getEntriesR();
        std::vector<Category*> getSubcats();
        std::vector<std::string> getIncludes();
        std::vector<std::string> getExcludes();

        static std::string getID(const std::string& line);
        static std::string getSingleValue(const std::string& line);

        bool registerDF(DesktopFile *df, bool force = false);

    private:
        std::string dirFile;
        std::vector<std::string> menuFiles;
        std::ifstream dir_f;
        std::ifstream menu_f;
        std::vector<std::string> validNames;
        std::vector<IconSpec> iconpaths;
        std::string iconsXdgSize;
        bool iconsXdgOnly;
        bool useIcons;
        std::vector<DesktopFile*> incEntries;
        std::vector<Category*> incCategories;
        std::vector<std::string> incEntryFiles;
        std::vector<std::string> excEntryFiles;

        static int registerCount;
        static std::vector<DesktopFile*> incEntriesR;

        void registerDF(Category *cat, DesktopFile *df, bool force = false);
        void getEntriesR(Category *cat);

        void readMenufiles();
        void parseDir();
        void parseMenu(const std::vector<std::string>& menu);
        void getCategoryIcon();
};

#endif
