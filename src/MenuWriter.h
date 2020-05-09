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

#define WRITER_CONSTRUCT const std::string& menuName, int windowmanager,\
        bool useIcons, std::vector<std::string> exclude, std::vector<std::string> excludeMatching,\
        std::vector<std::string> excludeCategories, std::vector<std::string> include,\
        std::vector<std::string> excludedFilenames, const std::vector<Category*>& cats

#define WRITER_PARAMS menuName, windowmanager, useIcons, exclude, excludeMatching,\
        excludeCategories, include, excludedFilenames, cats

class MenuWriter
{   
    public:
        MenuWriter(WRITER_CONSTRUCT);

    protected:
        std::vector<Category*> cats;
        std::string menuName;
        int windowmanager;
        bool useIcons;
        std::vector<std::string> exclude;
        std::vector<std::string> excludeMatching;
        std::vector<std::string> excludeCategories;
        std::vector<std::string> include;
        std::vector<std::string> excludedFilenames;

        std::vector<Category*> usedCats;

        void entryDisplayHandler();
        bool categoryNotExcluded(Category* c);
        int realNumEntries(std::vector<DesktopFile*>);
        int realNumCats(std::vector<Category*>);

        virtual void writeMenu(Category*, int, int) = 0;
};

class MwmMenuWriter : MenuWriter
{
    public:
        MwmMenuWriter(WRITER_CONSTRUCT);

    private:
        void writeMenu(Category*, int = -1, int = -1);
        void writeMainMenu();
};

class FvwmMenuWriter : MenuWriter
{
    public:
        FvwmMenuWriter(WRITER_CONSTRUCT);

    private:
        void writeMenu(Category*, int = -1, int = -1);
        void writeMainMenu();
};

class FluxboxMenuWriter : MenuWriter
{
    public:
        FluxboxMenuWriter(WRITER_CONSTRUCT);

    private:
        void writeMenu(Category*, int = -1, int = -1);
};

class OpenboxMenuWriter : MenuWriter
{
    public:
        OpenboxMenuWriter(WRITER_CONSTRUCT);

    private:
        void writeMenu(Category*, int = -1, int = -1);
        void writeMainMenu();
};

class OlvwmMenuWriter : MenuWriter
{
    public:
        OlvwmMenuWriter(WRITER_CONSTRUCT);

    private:
        void writeMenu(Category*, int = -1, int = -1);
};

class WmakerMenuWriter : MenuWriter
{
    public:
        WmakerMenuWriter(WRITER_CONSTRUCT);

    private:
        void writeMenu(Category*, int = -1, int = -1);
};

class IcewmMenuWriter : MenuWriter
{
    public:
        IcewmMenuWriter(WRITER_CONSTRUCT);

    private:
        void writeMenu(Category*, int = -1, int = -1);
};

#endif
