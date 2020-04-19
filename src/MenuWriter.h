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
//Any positive category numbers indicate an index in the std::vector of used
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
        MenuWriter(const std::string& menuName, bool useIcons, int windowmanager,
                std::vector<std::string> exclude, std::vector<std::string> excludeMatching, 
                std::vector<std::string> excludeCategories, std::vector<std::string> include, 
                std::vector<std::string> excludedFilenames, const std::vector<Category*>& cats);

    protected:
        std::string menuName;
        bool useIcons;
        int windowmanager;
        std::vector<std::string> exclude;
        std::vector<std::string> excludeMatching;
        std::vector<std::string> excludeCategories;
        std::vector<std::string> include;
        std::vector<std::string> excludedFilenames;
        std::vector<Category*> cats;

        void entryDisplayHandler();
        bool categoryNotExcluded(Category* c);
        int realMenuSize(std::vector<DesktopFile*> entries);

        virtual void writeMenu(Category *cat, int catNumber, 
                const std::vector<Category*>& usedCats, 
                int maxCatNumber = DEFAULT_MAX_CAT) = 0;

        //writeMenu variables
        //Variable for the std::vector of categories
        std::vector<Category*> usedCats;
        //Variable for the number of used categories
        int maxCatNumber;
        //Variable for the std::vector of desktop entries
        std::vector<DesktopFile*> dfiles; 
        //Variable for the category name
        std::string category; 
        //Variable for the category icon
        std::string catIcon;
        //Variable for category depth
        int depth;
        //Variable for std::vector of subcategories
        std::vector<Category*> subCats;
        //Variable for knowing how many items (entries and submenus) there are in
        //a menu
        int numOfItems;
        //Variable for knowing how many entries in a given category have been printed
        int realPos;
        //Variable for a formatted version of the name, e.g. quotes added
        std::string nameFormatted;
        //Variable for a formatted version of the exec, e.g. quotes added
        std::string execFormatted;
        //Variable for a formatted version of the category name, e.g. quotes added
        std::string catFormatted;
        //Variable for a formatted version of the menu, e.g. quotes added
        std::string menuFormatted;
};

class MwmMenuWriter : MenuWriter
{
    public:
        MwmMenuWriter(const std::string& menuName, bool useIcons, int windowmanager,
                std::vector<std::string> exclude, std::vector<std::string> excludeMatching, 
                std::vector<std::string> excludeCategories, std::vector<std::string> include, 
                std::vector<std::string> excludedFilenames, const std::vector<Category*>& cats);

    private:
        void writeMenu(Category *cat, int catNumber, 
                const std::vector<Category*>& usedCats, 
                int maxCatNumber = DEFAULT_MAX_CAT);
};

class FvwmMenuWriter : MenuWriter
{
    public:
        FvwmMenuWriter(const std::string& menuName, bool useIcons, int windowmanager,
                std::vector<std::string> exclude, std::vector<std::string> excludeMatching, 
                std::vector<std::string> excludeCategories, std::vector<std::string> include, 
                std::vector<std::string> excludedFilenames, const std::vector<Category*>& cats);

    private:
        void writeMenu(Category *cat, int catNumber, 
                const std::vector<Category*>& usedCats, 
                int maxCatNumber = DEFAULT_MAX_CAT);
};

class FluxboxMenuWriter : MenuWriter
{
    public:
        FluxboxMenuWriter(const std::string& menuName, bool useIcons, int windowmanager,
                std::vector<std::string> exclude, std::vector<std::string> excludeMatching, 
                std::vector<std::string> excludeCategories, std::vector<std::string> include, 
                std::vector<std::string> excludedFilenames, const std::vector<Category*>& cats);

    private:
        void writeMenu(Category *cat, int catNumber, 
                const std::vector<Category*>& usedCats, 
                int maxCatNumber = DEFAULT_MAX_CAT);
};

class OpenboxMenuWriter : MenuWriter
{
    public:
        OpenboxMenuWriter(const std::string& menuName, bool useIcons, int windowmanager,
                std::vector<std::string> exclude, std::vector<std::string> excludeMatching, 
                std::vector<std::string> excludeCategories, std::vector<std::string> include, 
                std::vector<std::string> excludedFilenames, const std::vector<Category*>& cats);

    private:
        void writeMenu(Category *cat, int catNumber, 
                const std::vector<Category*>& usedCats, 
                int maxCatNumber = DEFAULT_MAX_CAT);
};

class OlvwmMenuWriter : MenuWriter
{
    public:
        OlvwmMenuWriter(const std::string& menuName, bool useIcons, int windowmanager,
                std::vector<std::string> exclude, std::vector<std::string> excludeMatching, 
                std::vector<std::string> excludeCategories, std::vector<std::string> include, 
                std::vector<std::string> excludedFilenames, const std::vector<Category*>& cats);

    private:
        void writeMenu(Category *cat, int catNumber, 
                const std::vector<Category*>& usedCats, 
                int maxCatNumber = DEFAULT_MAX_CAT);
};

class WmakerMenuWriter : MenuWriter
{
    public:
        WmakerMenuWriter(const std::string& menuName, bool useIcons, int windowmanager,
                std::vector<std::string> exclude, std::vector<std::string> excludeMatching, 
                std::vector<std::string> excludeCategories, std::vector<std::string> include, 
                std::vector<std::string> excludedFilenames, const std::vector<Category*>& cats);

    private:
        void writeMenu(Category *cat, int catNumber, 
                const std::vector<Category*>& usedCats, 
                int maxCatNumber = DEFAULT_MAX_CAT);
};

class IcewmMenuWriter : MenuWriter
{
    public:
        IcewmMenuWriter(const std::string& menuName, bool useIcons, int windowmanager,
                std::vector<std::string> exclude, std::vector<std::string> excludeMatching, 
                std::vector<std::string> excludeCategories, std::vector<std::string> include, 
                std::vector<std::string> excludedFilenames, const std::vector<Category*>& cats);

    private:
        void writeMenu(Category *cat, int catNumber, 
                const std::vector<Category*>& usedCats, 
                int maxCatNumber = DEFAULT_MAX_CAT);
};

template <typename T> bool myCompare(T *a, T *b)
{
    std::string name_a = a->name;
    std::string name_b = b->name;
    for (unsigned int x = 0; x < name_a.size(); x++) 
        name_a.at(x) = tolower(name_a.at(x));
    for (unsigned int x = 0; x < name_b.size(); x++) 
        name_b.at(x) = tolower(name_b.at(x));
    if (name_a < name_b) return true;
    else return false;
}

#endif
