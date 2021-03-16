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

#ifndef _DESKTOP_FILE_H_
#define _DESKTOP_FILE_H_

#include <string>
#include <fstream>
#include <vector>

struct IconSpec
{   std::string path;
    std::string def;
    std::string id;
};

class Category;

class DesktopFile
{
    public:
        DesktopFile(const char *filename, std::vector<std::string> showFromDesktops, 
                bool useIcons, const std::vector<IconSpec>& iconpaths, 
                std::vector<Category*>& cats, const std::string& iconsXdgSize, 
                bool iconsXdgOnly, const std::string& term);

        std::string filename;
        std::string basename;
        std::string name;
        std::string exec;
        bool nodisplay;
        std::string icon;
        bool terminal;
        std::vector<std::string> foundCategories;

        static std::string getID(const std::string& line, const char start = '\0', const char end = '=');
        static std::string getSingleValue(const std::string& line, const char start = '=', const char end = '\0');
        static std::vector<std::string> getMultiValue(const std::string& line, const char separator = ';', const char start = '=');
 
    private:
        std::ifstream dfile;

        void populate(const std::vector<std::string>& showFromDesktops, bool useIcons, 
                const std::vector<IconSpec>& iconpaths, std::vector<Category*>& cats, 
                const std::string& iconsXdgSize, bool iconsXdgOnly, 
                const std::string& term);
        void matchIcon(const std::string& iconDef, const std::vector<IconSpec>& iconpaths,
                const std::string& iconsXdgSize, bool iconsXdgOnly);
        void processCategories(std::vector<Category*>& cats, 
                std::vector<std::string>& foundCategories);
        void processDesktops(const std::vector<std::string>& showFromDesktops, 
                const std::vector<std::string>& onlyShowInDesktops);
};

#endif
