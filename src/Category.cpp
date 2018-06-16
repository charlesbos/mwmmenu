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

#include <algorithm>
#include "Category.h"

Category::Category() {}

//Constructor for custom categories
Category::Category(const char *dirFile, const vector<string>& menuFiles, 
        bool useIcons, const vector<IconSpec>& iconpaths, 
        const string& iconsXdgSize, bool iconsXdgOnly)
{   
    this->dirFile = dirFile;
    this->menuFiles = menuFiles;
    dir_f.open(dirFile);
    if (!dir_f);
    else
    { 
        getCategoryParams();
        getSpecifiedFiles();
        dir_f.close();
        if (useIcons) getCategoryIcon(iconpaths, iconsXdgSize, iconsXdgOnly);
    }
}

//Constructor for base categories
Category::Category(const string& name, bool useIcons, 
        const vector<IconSpec>& iconpaths, const string& iconsXdgSize, 
        bool iconsXdgOnly)
{   
    this->name = name;
    if (useIcons) getCategoryIcon(iconpaths, iconsXdgSize, iconsXdgOnly);
}

Category::Category(const Category& c)
{   
    this->name = c.name;
    this->icon = c.icon;
    this->incEntries = c.incEntries;
    this->incEntryFiles = c.incEntryFiles;
    this->excEntryFiles = c.excEntryFiles;
}

Category& Category::operator=(const Category& c)
{   
    this->name = c.name;
    this->icon = c.icon;
    this->incEntries = c.incEntries;
    this->incEntryFiles = c.incEntryFiles;
    this->excEntryFiles = c.excEntryFiles;
    return *this;
}

bool Category::operator<(const Category& c)
{   
    string name_a = this->name;
    string name_b = c.name;
    for (unsigned int x = 0; x < name_a.size(); x++) 
        name_a.at(x) = tolower(name_a.at(x));
    for (unsigned int x = 0; x < name_b.size(); x++) 
        name_b.at(x) = tolower(name_b.at(x));
    if (name_a < name_b) return true;
    else return false;
}

/* A function to get the category name and icon definition */
void Category::getCategoryParams()
{   
    string line;

    while (!dir_f.eof())
    {
        getline(dir_f, line);
        string id = DesktopFile::getID(line);
        if (id == "Name") name = DesktopFile::getSingleValue(line);
        if (id == "Icon") icon = DesktopFile::getSingleValue(line);
    }
}

/* A function to loop through xdg .menu files, looking for any
 * desktop entry filenames that have been specified as belonging
 * to or excluded from this category */
void Category::getSpecifiedFiles()
{   
    for (unsigned int x = 0; x < menuFiles.size(); x++)
    {
        menu_f.open(menuFiles[x]);
        if (!menu_f) continue;
        string line;
        bool started = false;
        bool including = false;
        while (!menu_f.eof())
        { 
            getline(menu_f, line);
            string id = getID(line);
            if (id == "<Directory>")
            { 
                string dir = getSingleValue(line);
                if (dir == dirFile.substr(dirFile.find_last_of("/") + 1, 
                        dirFile.size() - dirFile.find_last_of("/") - 1))
                    started = true;
                else
                    started = false;
            }
            if (id == "<Include>" && started) including = true;
            if (id == "<Exclude>" && started) including = false;
            if (started && id == "<Filename>") 
            {   
                if (including) incEntryFiles.push_back(getSingleValue(line));
                else excEntryFiles.push_back(getSingleValue(line));
            }
        }
        menu_f.close();
    }
}

/* An xdg .menu file specific function for getting the line
 * id. In this case, the id will be tags like <Directory>
 * or <Include> */
string Category::getID(const string& line)
{   
    vector<char> readChars;
    readChars.reserve(10);
    char c;
    unsigned int counter = 0;
    bool startFilling = false;

    while (counter < line.size())
    {
        c = line[counter];
        if (startFilling) readChars.push_back(c);
        if (c == '>') break;
        if (c == '<') 
        {
            startFilling = true;
            readChars.push_back(c);
        }
        counter++;
    }

    return string(readChars.begin(), readChars.end());
}

/* An xdg .menu file specific function for getting the line
 * value. This should be a value between two enclosing tags.
 * For instance, for the line <tag>value</tag> then this
 * should return the string "value" */
string Category::getSingleValue(const string& line)
{   
    vector<char> readChars;
    readChars.reserve(10);
    char c;
    unsigned int counter = 0;
    bool startFilling = false;

    while (counter < line.size())
    {
        c = line[counter];
        if (c == '>')
        {
            startFilling = true;
            counter++;
            continue;
        }
        if (startFilling && c == '<') break;
        if (startFilling) readChars.push_back(c);
        counter++;
    }

    return string(readChars.begin(), readChars.end());
}

/* Try to set a path to an icon. If the category is custom, we might already
 * have an icon definition. Otherwise, we try and determine it from the category
 * name */
void Category::getCategoryIcon(const vector<IconSpec>& iconpaths, 
        const string& iconsXdgSize, bool iconsXdgOnly)
{   
    //If it's a base category, we want to get the icon from the freedesktop 
    //categories directory
    string nameGuard = "categories";
    //The icon definition, from which we try to determine a path to an icon
    string iconDef;

    /* This is a kludge. If we already have an icon definition and it is a full
     * path instead of a true definition, then check if it conforms to the 
     * required size and is an xdg directory if that has been requested. 
     * If so, exit here. Otherwise, clear the definition and carry on */
    if (icon != "\0" && icon.find("/") != string::npos && 
            icon.find(iconsXdgSize) != string::npos) 
    {   
        if (!iconsXdgOnly || (iconsXdgOnly && 
                icon.find("/share/icons/") != string::npos))
            return;
        else
            icon = "\0";
    }

    //Set the icon definition, either the one we already have or the 
    //category name
    if (icon != "\0") iconDef = icon;
    else iconDef = name;

    //If we already have a definition, we can get the icon from any directory
    if (icon != "\0") nameGuard = "/";

    //Workarounds
    //There is no icon for education so use the science one instead
    if (iconDef == "Education") iconDef = "Science";
    //Chromium Apps category has chromium-browser as its icon def but 
    //chromium does not provide an icon called chromium-browser so change it 
    //to just chromium
    if (iconDef == "chromium-browser") iconDef = "chromium";

    /* The main search loop. Here we try to match the category name against 
     * icon paths, checking that the word 'categories' appears somewhere 
     * in the path, as well as a basic check for size */
    iconDef.at(0) = tolower(iconDef.at(0));
    for (unsigned int x = 0; x < iconpaths.size(); x++)
    {  
        if (iconpaths[x].path.find(nameGuard) != string::npos && 
                iconpaths[x].path.find(iconDef) != string::npos)
        {   
            icon = iconpaths[x].path;
            return;
        }
    }
}
