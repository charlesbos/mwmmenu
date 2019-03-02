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

int Category::registerCount = 0;
vector<DesktopFile*> Category::incEntriesR = vector<DesktopFile*>();
vector<Category*> Category::subCatsR = vector<Category*>();

//Constructor for custom categories
Category::Category(const char *dirFile, const vector<string>& menuFiles, 
        bool useIcons, const vector<IconSpec>& iconpaths, 
        const string& iconsXdgSize, bool iconsXdgOnly) :
    depth(0),
    dirFile(dirFile),
    menuFiles(menuFiles),
    iconpaths(iconpaths),
    iconsXdgSize(iconsXdgSize),
    iconsXdgOnly(iconsXdgOnly),
    useIcons(useIcons)
{   
    dir_f.open(dirFile);
    if (!dir_f);
    else
    { 
        parseDir();
        if (this->name != "Other") this->validNames.push_back(this->name);
        readMenufiles();
        dir_f.close();
        if (useIcons) getCategoryIcon();
    }
}

//Constructor for subcategories
Category::Category(vector<string> menuDef, const char *dirFile, 
        bool useIcons, const vector<IconSpec>& iconpaths, 
        const string& iconsXdgSize, bool iconsXdgOnly, int depth) :
    depth(depth),
    dirFile(dirFile),
    iconpaths(iconpaths),
    iconsXdgSize(iconsXdgSize),
    iconsXdgOnly(iconsXdgOnly),
    useIcons(useIcons)
{
    parseMenu(menuDef);
    if (this->name != "Other") this->validNames.push_back(this->name);
    if (useIcons) getCategoryIcon();
}

//Constructor for base categories
Category::Category(const string& name, bool useIcons, 
        const vector<IconSpec>& iconpaths, const string& iconsXdgSize, 
        bool iconsXdgOnly) :
    name(name),
    depth(0),
    iconpaths(iconpaths),
    iconsXdgSize(iconsXdgSize),
    iconsXdgOnly(iconsXdgOnly),
    useIcons(useIcons)
{   
    if (this->name != "Other") this->validNames.push_back(this->name);
    if (useIcons) getCategoryIcon();
}

/* A function to parse a directory file to get get the category name and 
 * icon definition */
void Category::parseDir()
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

/* A function to read each menu file, creating a vector of strings for each
 * menufile where each string is a line. For each vector, we then call
 * parseMenu */
void Category::readMenufiles()
{
    vector<string> menuVec;
    for (unsigned int x = 0; x < menuFiles.size(); x++)
    {
        menu_f.open(menuFiles[x].c_str());
        if (!menu_f) continue;
        string line;
        while (!menu_f.eof())
        { 
            getline(menu_f, line);
            menuVec.push_back(line);
        }
        menu_f.close();
        parseMenu(menuVec);
        menuVec.clear();
    }
}

/* A function to parse an xdg menu files to get includes/excludes 
 * and submenus */
void Category::parseMenu(const vector<string>& menu)
{   
    string line;
    string dirLine;
    vector<string> subMenu;
    bool started = false;
    bool including = false;
    bool menuStarted = false;
    int menuOpenCnt = 0;
    int menuCloseCnt = 0;
    for (unsigned int x = 0; x < menu.size(); x++)
    {
        line = menu[x];
        string id = getID(line);
        if (id == "<Directory>")
        { 
            string dir = getSingleValue(line);
            dirLine = line;
            if (dir == dirFile.substr(dirFile.find_last_of("/") + 1, 
                    dirFile.size() - dirFile.find_last_of("/") - 1))
                started = true;
            else
                started = false;
        }
        if (id == "<Menu>" && started)
        {
            menuStarted = true;
            menuOpenCnt++;
        }
        if (menuStarted)
        {
            subMenu.push_back(line);
        }
        if (id == "</Menu>" && menuStarted)
        {
            menuCloseCnt++;
            if (menuOpenCnt == menuCloseCnt)
            {
                subMenu.erase(subMenu.begin());
                subMenu.erase(subMenu.end());
                subMenu.insert(subMenu.begin(), dirLine);
                Category *c = new Category(subMenu, dirFile.c_str(), useIcons, iconpaths, 
                        iconsXdgSize, iconsXdgOnly, depth + 1);
                bool replaced = false;
                vector<Category*> currentCats = this->getSubcats();
                for (unsigned int y = 0; y < currentCats.size(); y++)
                {
                    Category *curCat = currentCats[y];
                    if (c->name == curCat->name)
                    {
                        currentCats[y] = c;
                        replaced = true;
                        break;
                    }
                }
                if (!replaced) incCategories.push_back(c);
                subMenu.clear();
                menuStarted = false;
                menuOpenCnt = 0;
                menuCloseCnt = 0;
            }
        }
        if (id == "<Name>" && started && !menuStarted) this->name = getSingleValue(line);
        if (id == "<Include>" && started && !menuStarted) including = true;
        if (id == "<Exclude>" && started && !menuStarted) including = false;
        if (started && id == "<Filename>" && !menuStarted) 
        {   
            if (including) incEntryFiles.push_back(getSingleValue(line));
            else excEntryFiles.push_back(getSingleValue(line));
        }
        if (started && id == "<Category>" && !menuStarted) 
        {   
            if (including) validNames.push_back(getSingleValue(line));
        }
    }
}

/* Return all DesktopFiles associated with this category */
vector<DesktopFile*> Category::getEntries()
{
    return incEntries;
}

/* Return all DesktopFiles associated with this category plus all
 * associate subcategories */
vector<DesktopFile*> Category::getEntriesR()
{
    getEntriesR(this);
    vector<DesktopFile*> result(Category::incEntriesR);
    Category::incEntriesR.clear();
    return result;
}

void Category::getEntriesR(Category *cat)
{
    for (unsigned int x = 0; x < cat->incEntries.size(); x++)
        Category::incEntriesR.push_back(cat->incEntries[x]);
    for (unsigned int x = 0; x < cat->incCategories.size(); x++)
        getEntriesR(cat->incCategories[x]);
}

/* Return all subcategories associated with this category */
vector<Category*> Category::getSubcats()
{
    return incCategories;
}

/* Return all subcategories associated with this category plus subcategories
 * of associated subcategories */
vector<Category*> Category::getSubcatsR()
{
    getSubcatsR(this);
    vector<Category*> result(Category::subCatsR);
    Category::subCatsR.clear();
    return result;
}

void Category::getSubcatsR(Category *cat)
{
    for (unsigned int x = 0; x < cat->incCategories.size(); x++)
        Category::subCatsR.push_back(cat->incCategories[x]);
    for (unsigned int x = 0; x < cat->incCategories.size(); x++)
        getEntriesR(cat->incCategories[x]);
}

/* Return a list of filenames included/excluded from this category */
vector<string> Category::getIncludes()
{
    return incEntryFiles;
}

vector<string> Category::getExcludes()
{
    return excEntryFiles;
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

/* Add a DesktopFile to the list of included entries if its specified category
 * matches the category name or an included category name for the category or 
 * for any child subcategories. Return true to indicate the entry was included 
 * and false to indicate that it was not */
bool Category::registerDF(DesktopFile *df, bool force)
{
    bool result = false;
    registerDF(this, df, force);
    if (registerCount > 0)
    {
        result = true;
        registerCount = 0;
    }
    return result;
}

void Category::registerDF(Category *cat, DesktopFile *df, bool force)
{
    if (force)
    {
        cat->incEntries.push_back(df);
        registerCount++;
        return;
    }
    for (unsigned int x = 0; x < cat->validNames.size(); x++)
    {
        //Add to category if foundCategories contains the category name
        if (find(df->foundCategories.begin(), df->foundCategories.end(), 
                cat->validNames[x]) != df->foundCategories.end() &&
                find(cat->excEntryFiles.begin(), cat->excEntryFiles.end(), 
                df->basename) == cat->excEntryFiles.end())
        {   
            cat->incEntries.push_back(df);
            registerCount++;
        }
    }
    //Add to category if the category specifies a particular desktop file 
    //by filename
    if (find(cat->incEntryFiles.begin(), cat->incEntryFiles.end(), 
            df->basename) != cat->incEntryFiles.end() && 
            find(cat->excEntryFiles.begin(), cat->excEntryFiles.end(), 
            df->basename) == cat->excEntryFiles.end())

    {   
        cat->incEntries.push_back(df);
        registerCount++;
    }
    //Call this function recursively on subcategories
    for (unsigned int x = 0; x < cat->incCategories.size(); x++)
        registerDF(cat->incCategories[x], df);
}

/* Try to set a path to an icon. If the category is custom, we might already
 * have an icon definition. Otherwise, we try and determine it from the category
 * name */
void Category::getCategoryIcon() 
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
