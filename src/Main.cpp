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

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "boost/filesystem.hpp"
#include "DesktopFile.h"
#include "MenuWriter.h"
#include "Category.h"

#define GET_COMMA_VALUES(X) DesktopFile::getMultiValue(X, ',', '\0')

#define WRITER_ARGS menuName, windowmanager, useIcons,\
        GET_COMMA_VALUES(exclude), GET_COMMA_VALUES(excludeMatching),\
        GET_COMMA_VALUES(excludeCategories), GET_COMMA_VALUES(include),\
        GET_COMMA_VALUES(excludedFilenames), cats

void usage()
{   
    std::cout << 
        "mwmmenu - creates application menus for MWM and other window managers.\n\n"
        "Usage:\n"
        "  # Note: all options must be spaced.\n"
        "  mwmmenu [OPTIONS]\n\n"
        "Options:\n"
        "  -h, --help:            show this dialogue\n"
        "  -n, --name:            name used for the main menu - by default, use\n" 
        "                         Applications\n"
        "  -i, --icons:           use icons with menu entries, only compatible with\n" 
        "                         some window managers\n"
        "  -t, --terminal:        specify terminal to be used for terminal based\n"
        "                         applications. If this option is not used, then\n"
        "                         xterm -e is used as the default.\n"
        "  --icons-xdg-only:      exclude any non-xdg icons. Note that this will\n" 
        "                         disable the --add-icon-paths option.\n"
        "  --icons-xdg-size:      can be 16x16, 32x32 etc. Can also be scalable or\n" 
        "                         all. Note that this cannot control sizes for\n" 
        "                         non-xdg icons. Defaults to all.\n"
        "  --no-custom-categories: do not add entries to or print non-standard\n" 
        "                         categories, 'Other' will be used instead if\n"
        "                         required.\n\n"
        "  # Note:\n"
        "  * The following options accept a single string which can contain multiple\n"
        "    parameters.\n"
        "  * Multiple parameters should be separated by commas: e.g. --option\n" 
        "    Param1,Param2,Param3\n"
        "  * If the option asks for a name, assume it means a name as specified in\n" 
        "    an entry file, e.g. Name=Firefox.\n"
        "  * If a parameter contains spaces, that parameter must be enclosed in\n"
        "    quotes.\n\n"
        "  --exclude:             do not add desktop entries that have the names\n" 
        "                         specified.\n"
        "  --exclude-matching:    do not add desktop entries where the entry's name\n" 
        "                         contains one of the strings specified.\n"
        "  --exclude-categories:  do not print category menus for the following\n" 
        "                         category names.\n"
        "  --exclude-by-filename: exclude desktop entries based on their full paths.\n"
        "  --include:             force entries with the following names to be\n" 
        "                         included in menus even if their no display value\n" 
        "                         is true.\n"
        "  --show-from-desktops:  show entries from the specified desktops if\n" 
        "                         OnlyShowIn is set. Can be values like GNOME or\n" 
        "                         XFCE. Can also be none or all, The default is none.\n"
        "  --add-desktop-paths:   add extra search paths for desktop entries.\n"
        "  --add-icon-paths:      add extra search paths for icons.\n\n"
        "Menu format options:\n"
        "  # No format argument:  produce menus for MWM and TWM\n"
        "  --fvwm:                produce static menus for FVWM\n"
        "  --fvwm-dynamic:        produce dynamic menus for FVWM\n"
        "  --fluxbox:             produce menus for Fluxbox or Blackbox\n"
        "  --openbox:             produce static menus for Openbox\n"
        "  --openbox-pipe:        produce pipe menus for Openbox\n"
        "  --olvwm:               produce menus for Olwm and Olvwm\n"
        "  --windowmaker:         produce menus for Windowmaker\n"
        "  --icewm:               produce menus for IceWM\n";
}

//Function that attempts to get the user icon theme from ~/.gtkrc-2.0
std::string getIconTheme(const std::string& homedir)
{
    std::ifstream themefile;
    std::string path = homedir + "/.gtkrc-2.0";
    std::string id = "gtk-icon-theme-name";
    themefile.open(path.c_str());
    if (!themefile) return "";
    else
    {
        std::string line;
        while (!themefile.eof())
        {
            getline(themefile, line);
            std::string read_id = DesktopFile::getID(line);
            if (id == read_id)
            {
                std::string themename = DesktopFile::getSingleValue(line);
                return themename;
            }
        }
        themefile.close();
        return "";
    }
}

//A function to make sure we only add unique categories to the categories list
void addCategory(Category *c, std::vector<Category*> &categories)
{  
    for (unsigned int x = 0; x < categories.size(); x++) 
    {
        if (categories[x]->name == c->name)
        {
            //Replace default category object with custom object of the same
            //name if the definitions differ
            if (!c->getIncludes().empty() || !c->getExcludes().empty() || 
                    (c->icon != categories[x]->icon && c->icon != "")) 
                categories[x] = c;
            return;
        }
    }
    categories.push_back(c);
}

//A function to add an id (meaning a base filename - no full path) to a list
//if it isn't already present in the list. Return true if we add the item
//and false if we do not because it's already in the list. This is useful for 
//local overrides for XDG desktop entries and icons.
bool addID(const std::string& path, std::vector<std::string> &ids)
{  
    std::string id = boost::filesystem::path(path).stem().string();
    for (unsigned int x = 0; x < ids.size(); x++)
        if (id == ids[x]) return false;
    ids.push_back(id);
    return true;
}

int main(int argc, char *argv[])
{  
    //Handle args
    std::string homedir = getenv("HOME");
    std::string term = "xterm -e";
    std::string menuName = "Applications";
    WindowManager windowmanager = mwm;
    bool useIcons = false;
    bool iconsXdgOnly = false;
    std::string iconsXdgSize = "all";
    std::string exclude;
    std::string excludeMatching;
    std::string excludeCategories;
    std::string excludedFilenames;
    std::string include;
    std::string showFromDesktops = "none";
    std::string extraDesktopPaths;
    std::string extraIconPaths;
    bool noCustomCats = false;

    for (int x = 0; x < argc; x++)
    {
        if (strcmp(argv[x], "-h") == 0 || strcmp(argv[x], "--help") == 0)
        {
            usage();
            return 0; 
        }
        if (strcmp(argv[x], "-n") == 0 || strcmp(argv[x], "--name") == 0) 
        {
            if (x + 1 < argc) menuName = argv[x + 1];
            continue;
        }
        if (strcmp(argv[x], "-i") == 0 || strcmp(argv[x], "--icons") == 0)
        {
            useIcons = true;
            continue;
        }
        if (strcmp(argv[x], "-t") == 0 || strcmp(argv[x], "--terminal") == 0)
        {
            if (x + 1 < argc) 
            {
                term = argv[x + 1];
                term += " -e";
            }
            continue;
        }
        if (strcmp(argv[x], "--icons-xdg-only") == 0) 
        {  
            iconsXdgOnly = true;
            continue;
        }
        if (strcmp(argv[x], "--icons-xdg-size") == 0) 
        {  
            if (x + 1 < argc) iconsXdgSize = argv[x + 1] ;
            continue;
        }
        if (strcmp(argv[x], "--fvwm") == 0) 
        {  
            windowmanager = fvwm;
            continue;
        }
        if (strcmp(argv[x], "--fvwm-dynamic") == 0) 
        {  
            windowmanager = fvwm_dynamic;
            continue;
        }
        if (strcmp(argv[x], "--fluxbox") == 0) 
        {  
            windowmanager = fluxbox;
            continue;
        }
        if (strcmp(argv[x], "--openbox") == 0) 
        {  
            windowmanager = openbox;
            continue;
        }
        if (strcmp(argv[x], "--openbox-pipe") == 0) 
        {  
            windowmanager = openbox_pipe;
            continue;
        }
        if (strcmp(argv[x], "--olvwm") == 0) 
        {  
            windowmanager = olvwm;
            continue;
        }
        if (strcmp(argv[x], "--windowmaker") == 0) 
        {  
            windowmanager = windowmaker;
            continue;
        }
        if (strcmp(argv[x], "--icewm") == 0) 
        {  
            windowmanager = icewm;
            continue;
        }
        if (strcmp(argv[x], "--exclude") == 0) 
        {  
            if (x + 1 < argc) exclude = argv[x + 1];
            continue;
        }
        if (strcmp(argv[x], "--exclude-matching") == 0) 
        {  
            if (x + 1 < argc) excludeMatching = argv[x + 1];
            continue;
        }
        if (strcmp(argv[x], "--exclude-categories") == 0) 
        {  
            if (x + 1 < argc) excludeCategories = argv[x + 1];
            continue;
        }
        if (strcmp(argv[x], "--exclude-by-filename") == 0)
        {  
            if (x + 1 < argc) excludedFilenames = argv[x + 1];
            continue;
        }
        if (strcmp(argv[x], "--include") == 0)
        {  
            if (x + 1 < argc) include = argv[x + 1];
            continue;
        }
        if (strcmp(argv[x], "--show-from-desktops") == 0)
        {  
            if (x + 1 < argc) showFromDesktops = argv[x + 1];
            continue;
        }
        if (strcmp(argv[x], "--add-desktop-paths") == 0) 
        {  
            if (x + 1 < argc) extraDesktopPaths = argv[x + 1];
            continue;
        }
        if (strcmp(argv[x], "--add-icon-paths") == 0) 
        {  
            if (x + 1 < argc) extraIconPaths = argv[x + 1];
            continue;
        }
        if (strcmp(argv[x], "--no-custom-categories") == 0)
        {  
            noCustomCats = true;
            continue;
        }
    }
    if (windowmanager == mwm || 
            windowmanager == olvwm ||
            windowmanager == windowmaker) 
        useIcons = false;
    if (iconsXdgSize == "all") iconsXdgSize = "/";

    //Get std::string std::vector of paths to .desktop files
    std::vector<std::string> paths;
    std::vector<std::string> pathIDS;
    paths.reserve(300);
    pathIDS.reserve(300);
    std::vector<std::string> appdirs;
    if (extraDesktopPaths != "")
    {   
        std::vector<std::string> newDPaths = GET_COMMA_VALUES(extraDesktopPaths);
        for (unsigned int x = 0; x < newDPaths.size(); x++)
            appdirs.push_back(newDPaths[x]);
    }
    if (homedir.c_str() != NULL) 
        appdirs.push_back(homedir + "/.local/share/applications/");
    appdirs.push_back("/usr/local/share/applications");
    appdirs.push_back("/usr/share/applications");
    for (unsigned int x = 0; x < appdirs.size(); x++)
    {   
        try
        {
            for (boost::filesystem::recursive_directory_iterator i(appdirs[x]),
                    end; i != end; ++i)
                if (!is_directory(i->path()))
                {
                    std::string thePath = i->path().string();
                    if (thePath.size() > 8 && 
                            thePath.substr(thePath.size() - 8, 8) == ".desktop" &&
                            addID(i->path().string(), pathIDS)) 
                        paths.push_back(i->path().string());
                }
        }
        catch (boost::filesystem::filesystem_error&) 
        { 
            continue; 
        }
    }

    //Get std::string std::vector of paths to icons
    std::vector<IconSpec> iconpaths;
    std::vector<std::string> iconpathIDS;
    if (useIcons)
    {   
        iconpaths.reserve(500);
        iconpathIDS.reserve(500);
        std::vector<std::string> icondirs;
        if (extraIconPaths != "" && !iconsXdgOnly)
        {
            std::vector<std::string> newIPaths = GET_COMMA_VALUES(extraIconPaths);
            for (unsigned int x = 0; x < newIPaths.size(); x++)
                icondirs.push_back(newIPaths[x]);
        }
        if (homedir.c_str() != NULL)
        {
            icondirs.push_back(homedir + "/.icons/hicolor");
            icondirs.push_back(homedir + "/.local/share/icons/hicolor");
            std::string themename = getIconTheme(homedir); 
            icondirs.push_back("/usr/share/icons/" + themename);
            if (find(icondirs.begin(), icondirs.end(), 
                    "/usr/share/icons/gnome") != icondirs.end()) 
                icondirs.push_back("/usr/share/icons/gnome");
        }
        icondirs.push_back("/usr/share/icons/hicolor");
        if (!iconsXdgOnly) 
        {   
            icondirs.push_back("/usr/local/share/pixmaps");
            icondirs.push_back("/usr/share/pixmaps");
        }
        //If an xdg icon size has been specified, limit the icon search to the 
        //appropriate directory
        if (iconsXdgSize != "/")
        { 
            for (unsigned int x = 0; x < icondirs.size(); x++)
            { 
                if (icondirs[x].find("/share/icons/") != std::string::npos)
                    icondirs[x] = icondirs[x] + "/" + iconsXdgSize;
            }
        }
        for (unsigned int x = 0; x < icondirs.size(); x++)
        {   
            try
            {
                for (boost::filesystem::recursive_directory_iterator 
                        i(icondirs[x]), end; i != end; ++i)
                {   
                    if (!is_directory(i->path()))
                    {
                        std::string ipath = i->path().string();
                        if (!addID(ipath, iconpathIDS)) continue;
                        IconSpec spec;
                        spec.path = ipath;
                        spec.id = iconpathIDS[iconpathIDS.size() - 1];
                        spec.def = 
                            spec.id.substr(spec.id.find_last_of("/") + 1, 
                            spec.id.find_last_of(".") - 
                            spec.id.find_last_of("/") - 1);
                        iconpaths.push_back(spec);
                    }
                }
            }
            catch (boost::filesystem::filesystem_error&) 
            { 
                continue; 
            }
        }
    }

    /* Create categories
     * Note that for baseCategories we combine Audio, Video and AudioVideo 
     * into Multimedia. We also rename Network to Internet and Utility to 
     * Accessories as this is what is commonly done elsewhere. Otherwise, our 
     * categories are the same as the freedesktop.org base categories */
    const char *baseCatsArr[] = {"Accessories", "Development", "Education",
        "Game", "Graphics", "Multimedia", "Internet", "Office", "Other",
        "Science", "Settings", "System"};
    std::vector<std::string> baseCategories(baseCatsArr, 
            baseCatsArr + sizeof(baseCatsArr) / sizeof(*baseCatsArr));
    std::vector<std::string> catPaths;
    catPaths.reserve(10);
    std::vector<std::string> menuPaths;
    menuPaths.reserve(10);
    if (!noCustomCats)
    {   
        std::vector<std::string> catDirs;
        catDirs.reserve(10);
        catDirs.push_back("/usr/share/desktop-directories");
        std::vector<std::string> menuDirs;
        menuDirs.reserve(10);
        menuDirs.push_back("/etc/xdg/menus/applications-merged");
        if (homedir.c_str() != NULL) 
        {
            catDirs.push_back(homedir + "/.local/share/desktop-directories");
            menuDirs.push_back(homedir + "/.config/menus/applications-merged");
        }
        for (unsigned int x = 0; x < catDirs.size(); x++)
        {
            try
            {
                for (boost::filesystem::recursive_directory_iterator 
                        i(catDirs[x]), end; i != end; ++i)
                    if (!is_directory(i->path()))
                    {
                        std::string thePath = i->path().string();
                        if (thePath.size() > 10 &&
                                thePath.substr(thePath.size() - 10, 19) == ".directory")
                            catPaths.push_back(i->path().string());
                    }
            }
            catch (boost::filesystem::filesystem_error&) 
            { 
                continue; 
            }
        }
        for (unsigned int x = 0; x < menuDirs.size(); x++)
        {   
            try
            {
                for (boost::filesystem::recursive_directory_iterator 
                        i(menuDirs[x]), end; i != end; ++i)
                {
                    if (!is_directory(i->path()))
                    { 
                        std::string thePath = i->path().string();
                        if (thePath.size() > 5 && 
                                thePath.substr(thePath.size() - 5, 5) == ".menu")
                            menuPaths.push_back(thePath);
                    }
                }
            }
            catch (boost::filesystem::filesystem_error&) 
            { 
                continue; 
            }
        }
    }
    std::vector<Category*> cats;
    cats.reserve(20);
    //Create the base categories
    for (unsigned int x = 0; x < baseCategories.size(); x++)
    {   
        Category *c = new Category(baseCategories[x], useIcons, iconpaths, 
                iconsXdgSize, iconsXdgOnly);
        cats.push_back(c);
    }
    //Create the custom categories (if there are any)
    for (unsigned int x = 0; x < catPaths.size(); x++)
    {   
        Category *c = new Category(catPaths[x].c_str(), menuPaths, useIcons, 
                iconpaths, iconsXdgSize, iconsXdgOnly);
        if (c->name != "") addCategory(c, cats);
    }
    sort(cats.begin(), cats.end(), myCompare<Category>);

    //Create DesktopFile objects, they will associate themselves with the
    //appropriate categories
    std::vector<DesktopFile*> files;
    files.reserve(300);
    for (std::vector<std::string>::iterator it = paths.begin(); it < paths.end(); it++)
    {   
        DesktopFile *df = new DesktopFile((*it).c_str(), 
                GET_COMMA_VALUES(showFromDesktops), useIcons, iconpaths, cats, 
                iconsXdgSize, iconsXdgOnly, term);
        if (df->name != "" && df->exec != "") files.push_back(df);
        else delete df;
    }

    //Create a MenuWriter which will write the menu out to the console
    switch (windowmanager)
    {
        case mwm:
            MwmMenuWriter(WRITER_ARGS);
            break;
        case fvwm:
        case fvwm_dynamic:
            FvwmMenuWriter(WRITER_ARGS);
            break;
        case fluxbox:
            FluxboxMenuWriter(WRITER_ARGS);
            break;
        case openbox:
        case openbox_pipe:
            OpenboxMenuWriter(WRITER_ARGS);
            break;
        case olvwm:
            OlvwmMenuWriter(WRITER_ARGS);
            break;
        case windowmaker:
            WmakerMenuWriter(WRITER_ARGS);
            break;
        case icewm:
            IcewmMenuWriter(WRITER_ARGS);
            break;
    }

    for (unsigned int x = 0; x < cats.size(); x++) delete cats[x];
    for (unsigned int x = 0; x < files.size(); x++) delete files[x];

    return 0;
}
