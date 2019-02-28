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

void usage()
{   cout << 
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

//A function to extract a filename from a full path
string getFilename(const string& path) 
{ 
    return path.substr(path.find_last_of("/") + 1, string::npos); 
}

//Function that attempts to get the user icon theme from ~/.gtkrc-2.0
string getIconTheme(const string& homedir)
{
    ifstream themefile;
    string path = homedir + "/.gtkrc-2.0";
    string id = "gtk-icon-theme-name";
    themefile.open(path.c_str());
    if (!themefile) return "\0";
    else
    {
        string line;
        char c;
        unsigned int counter = 0;
        while (!themefile.eof())
        {
            getline(themefile, line);
            vector<char> readChars;
            readChars.reserve(10);
            while (counter < line.size())
            {
                c = line[counter];
                if (c == '=') break;
                readChars.push_back(c);
                counter++;
            }
            string read_id = string(readChars.begin(), readChars.end());
            read_id.erase(remove(read_id.begin(), read_id.end(), ' '), 
                    read_id.end());
            if (id == read_id)
            {
                vector<char> themeNameChars;
                themeNameChars.reserve(10);
                while (counter < line.size())
                { 
                    c = line[counter];
                    if (c != '"' && c != '=') themeNameChars.push_back(c);
                    counter ++;
                }
                themefile.close();
                string themename = string(themeNameChars.begin(), 
                        themeNameChars.end());
                themename.erase(remove(themename.begin(), 
                        themename.end(), ' '), themename.end());
                return themename;
            }
            counter = 0;
        }
        themefile.close();
        return "\0";
    }
}

/* Function to split string argument separated by commas into
 * a string vector */
vector<string> splitCommaArgs(const string& arg)
{   
    vector<string> splitArgs;
    vector<char> buffer;
    splitArgs.reserve(5);
    buffer.reserve(10);

    for (unsigned int x = 0; x < arg.size(); x++)
    {
        if (arg[x] == ',') 
        {
            splitArgs.push_back(string(buffer.begin(), buffer.end()));
            buffer.clear();
            continue;
        }
        buffer.push_back(arg[x]);
    }
    if (!buffer.empty()) 
        splitArgs.push_back(string(buffer.begin(), buffer.end()));

    return splitArgs;
}

//A function to make sure we only add unique categories to the categories list
void addCategory(Category *c, vector<Category*> &categories)
{  
    for (unsigned int x = 0; x < categories.size(); x++) 
    {
        if (categories[x]->name == c->name)
        {
            //Replace default category object with custom object of the same
            //name if the definitions differ
            if (!c->getIncludes().empty() || !c->getExcludes().empty() || 
                    (c->icon != categories[x]->icon && c->icon != "\0")) 
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
bool addID(const string& path, vector<string> &ids)
{  
    string id = getFilename(path);
    for (unsigned int x = 0; x < ids.size(); x++)
        if (id == ids[x]) return false;
    ids.push_back(id);
    return true;
}

int main(int argc, char *argv[])
{  
    //Handle args
    string homedir = getenv("HOME");
    string term = "xterm -e";
    string menuName = "Applications";
    int windowmanager = mwm;
    bool useIcons = false;
    bool iconsXdgOnly = false;
    string iconsXdgSize = "all";
    string exclude;
    string excludeMatching;
    string excludeCategories;
    string excludedFilenames;
    string include;
    string showFromDesktops = "none";
    string extraDesktopPaths;
    string extraIconPaths;
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

    //Get string vector of paths to .desktop files
    vector<string> paths;
    vector<string> pathIDS;
    paths.reserve(300);
    pathIDS.reserve(300);
    vector<string> appdirs;
    if (extraDesktopPaths != "\0")
    {   
        vector<string> newDPaths = splitCommaArgs(extraDesktopPaths);
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
                if (!is_directory(i->path()) && addID(i->path().string(), 
                        pathIDS)) 
                    paths.push_back(i->path().string());
        }
        catch (boost::filesystem::filesystem_error&) 
        { 
            continue; 
        }
    }

    //Get string vector of paths to icons
    vector<IconSpec> iconpaths;
    vector<string> iconpathIDS;
    if (useIcons)
    {   
        iconpaths.reserve(500);
        iconpathIDS.reserve(500);
        vector<string> icondirs;
        if (extraIconPaths != "\0" && !iconsXdgOnly)
        {
            vector<string> newIPaths = splitCommaArgs(extraIconPaths);
            for (unsigned int x = 0; x < newIPaths.size(); x++)
                icondirs.push_back(newIPaths[x]);
        }
        if (homedir.c_str() != NULL)
        {
            icondirs.push_back(homedir + "/.icons/hicolor");
            icondirs.push_back(homedir + "/.local/share/icons/hicolor");
            string themename = getIconTheme(homedir); 
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
                if (icondirs[x].find("/share/icons/") != string::npos)
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
                        string ipath = i->path().string();
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
    vector<string> baseCategories(baseCatsArr, 
            baseCatsArr + sizeof(baseCatsArr) / sizeof(*baseCatsArr));
    vector<string> catPaths;
    catPaths.reserve(10);
    vector<string> menuPaths;
    menuPaths.reserve(10);
    if (!noCustomCats)
    {   
        vector<string> catDirs;
        catDirs.reserve(10);
        catDirs.push_back("/usr/share/desktop-directories");
        vector<string> menuDirs;
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
                        catPaths.push_back(i->path().string());
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
                    if (!is_directory(i->path())) 
                        menuPaths.push_back(i->path().string());
            }
            catch (boost::filesystem::filesystem_error&) 
            { 
                continue; 
            }
        }
    }
    vector<Category*> cats;
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
        if (c->name != "\0") addCategory(c, cats);
    }
    sort(cats.begin(), cats.end(), myCompare<Category>);

    //Create DesktopFile objects, they will associate themselves with the
    //appropriate categories
    vector<DesktopFile*> files;
    files.reserve(300);
    for (vector<string>::iterator it = paths.begin(); it < paths.end(); it++)
    {   
        DesktopFile *df = new DesktopFile((*it).c_str(), 
                splitCommaArgs(showFromDesktops), useIcons, iconpaths, cats, 
                iconsXdgSize, iconsXdgOnly, term);
        if (df->name != "\0" && df->exec != "\0") files.push_back(df);
        else delete df;
    }

    //Create a MenuWriter which will write the menu out to the console
    MenuWriter(menuName, windowmanager, useIcons, 
            splitCommaArgs(exclude), splitCommaArgs(excludeMatching), 
            splitCommaArgs(excludeCategories), splitCommaArgs(include),
            splitCommaArgs(excludedFilenames), cats);

    for (unsigned int x = 0; x < cats.size(); x++) delete cats[x];
    for (unsigned int x = 0; x < files.size(); x++) delete files[x];

    return 0;
}
