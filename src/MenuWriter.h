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
enum WindowManager
{
    mwm = 0,
    fvwm,
    fvwm_dynamic,
    fluxbox,
    openbox,
    openbox_pipe,
    olvwm,
    windowmaker,
    icewm
};

#define DEFAULT_CAT_NUM -1
#define DEFAULT_MAX_CAT_NUM -1

#define WRITER_CONSTRUCT const std::string& menuName, WindowManager windowmanager,\
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
        WindowManager windowmanager;
        bool useIcons;
        std::vector<std::string> exclude;
        std::vector<std::string> excludeMatching;
        std::vector<std::string> excludeCategories;
        std::vector<std::string> include;
        std::vector<std::string> excludedFilenames;

        std::vector<Category*> usedCats;

        void entryDisplayHandler();
        bool categoryNotExcluded(Category* c);
        int realNumEntries(std::vector<DesktopFile*> entries);
        int realNumCats(std::vector<Category*> cats);

        virtual void writeMenu(Category* cat, int catNumber, int maxCatNumber) = 0;
};

class MwmMenuWriter : MenuWriter
{
    public:
        MwmMenuWriter(WRITER_CONSTRUCT);

    private:
        void writeMenu(Category* cat, int catNumber = DEFAULT_CAT_NUM, int = DEFAULT_MAX_CAT_NUM);
        void writeMainMenu();
};

class FvwmMenuWriter : MenuWriter
{
    public:
        FvwmMenuWriter(WRITER_CONSTRUCT);

    private:
        void writeMenu(Category* cat, int catNumber = DEFAULT_CAT_NUM, int = DEFAULT_MAX_CAT_NUM);
        void writeMainMenu();
};

class FluxboxMenuWriter : MenuWriter
{
    public:
        FluxboxMenuWriter(WRITER_CONSTRUCT);

    private:
        void writeMenu(Category* cat, int catNumber = DEFAULT_CAT_NUM, int = DEFAULT_MAX_CAT_NUM);
};

class OpenboxMenuWriter : MenuWriter
{
    public:
        OpenboxMenuWriter(WRITER_CONSTRUCT);

    private:
        void writeMenu(Category* cat, int catNumber = DEFAULT_CAT_NUM, int = DEFAULT_MAX_CAT_NUM);
        void writeMainMenu();
};

class OlvwmMenuWriter : MenuWriter
{
    public:
        OlvwmMenuWriter(WRITER_CONSTRUCT);

    private:
        void writeMenu(Category* cat, int catNumber = DEFAULT_CAT_NUM, int = DEFAULT_MAX_CAT_NUM);
};

class WmakerMenuWriter : MenuWriter
{
    public:
        WmakerMenuWriter(WRITER_CONSTRUCT);

    private:
        void writeMenu(Category* cat, int catNumber = DEFAULT_CAT_NUM, int = DEFAULT_MAX_CAT_NUM);
};

class IcewmMenuWriter : MenuWriter
{
    public:
        IcewmMenuWriter(WRITER_CONSTRUCT);

    private:
        void writeMenu(Category* cat, int catNumber = DEFAULT_CAT_NUM, int = DEFAULT_MAX_CAT_NUM);
};

#endif
