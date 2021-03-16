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
std::vector<DesktopFile*> Category::incEntriesR = std::vector<DesktopFile*>();
std::vector<Category*> Category::incSubcatsR = std::vector<Category*>();

//Constructor for custom categories
Category::Category(const char *dirFile, const std::vector<std::string>& menuFiles, 
        bool useIcons, const std::vector<IconSpec>& iconpaths, 
        const std::string& iconsXdgSize, bool iconsXdgOnly) :
    depth(0),
    nodisplay(false),
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
Category::Category(std::vector<std::string> menuDef, const char *dirFile, 
        bool useIcons, const std::vector<IconSpec>& iconpaths, 
        const std::string& iconsXdgSize, bool iconsXdgOnly, int depth) :
    depth(depth),
    nodisplay(false),
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
Category::Category(const std::string& name, bool useIcons, 
        const std::vector<IconSpec>& iconpaths, const std::string& iconsXdgSize, 
        bool iconsXdgOnly) :
    name(name),
    depth(0),
    nodisplay(false),
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
    std::string line;

    while (!dir_f.eof())
    {
        getline(dir_f, line);
        std::string id = GET_ID_INI(line);
        if (id == "Name") name = GET_VAL_INI(line);
        if (id == "Icon") icon = GET_VAL_INI(line);
    }
}

/* A function to read each menu file, creating a std::vector of strings for each
 * menufile where each std::string is a line. For each std::vector, we then call
 * parseMenu */
void Category::readMenufiles()
{
    std::vector<std::string> menuVec;
    for (unsigned int x = 0; x < menuFiles.size(); x++)
    {
        menu_f.open(menuFiles[x].c_str());
        if (!menu_f) continue;
        std::string line;
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
void Category::parseMenu(const std::vector<std::string>& menu)
{   
    std::string line;
    std::string dirLine;
    std::vector<std::string> subMenu;
    bool started = false;
    bool including = false;
    bool menuStarted = false;
    int menuOpenCnt = 0;
    int menuCloseCnt = 0;
    for (unsigned int x = 0; x < menu.size(); x++)
    {
        line = menu[x];
        std::string id = GET_ID_XML(line);
        if (id == "<Directory>")
        { 
            std::string dir = GET_VAL_XML(line);
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
                std::vector<Category*> currentCats = this->getSubcats();
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
        if (id == "<Name>" && started && !menuStarted) this->name = GET_VAL_XML(line);
        if (id == "<Include>" && started && !menuStarted) including = true;
        if (id == "<Exclude>" && started && !menuStarted) including = false;
        if (started && id == "<Filename>" && !menuStarted) 
        {   
            if (including) incEntryFiles.push_back(GET_VAL_XML(line));
            else excEntryFiles.push_back(GET_VAL_XML(line));
        }
        if (started && id == "<Category>" && !menuStarted) 
        {   
            if (including) validNames.push_back(GET_VAL_XML(line));
        }
    }
}

/* Return all DesktopFiles associated with this category */
std::vector<DesktopFile*> Category::getEntries()
{
    sort(incEntries.begin(), incEntries.end(), myCompare<DesktopFile>);
    return incEntries;
}

/* Return all DesktopFiles associated with this category plus all
 * associate subcategories */
std::vector<DesktopFile*> Category::getEntriesR()
{
    getEntriesR(this);
    std::vector<DesktopFile*> result(Category::incEntriesR);
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
std::vector<Category*> Category::getSubcats()
{
    sort(incCategories.begin(), incCategories.end(), myCompare<Category>);
    return incCategories;
}

/* Return all subcategories associated with this category plus all
 * subcategories of the subcategories */
std::vector<Category*> Category::getSubcatsR()
{
    getSubcatsR(this);
    std::vector<Category*> result(Category::incSubcatsR);
    Category::incSubcatsR.clear();
    return result;
}

void Category::getSubcatsR(Category *cat)
{
    for (unsigned int x = 0; x < cat->incCategories.size(); x++)
        Category::incSubcatsR.push_back(cat->incCategories[x]);
    for (unsigned int x = 0; x < cat->incCategories.size(); x++)
        getSubcatsR(cat->incCategories[x]);
}

/* Return a list of filenames included/excluded from this category */
std::vector<std::string> Category::getIncludes()
{
    return incEntryFiles;
}

std::vector<std::string> Category::getExcludes()
{
    return excEntryFiles;
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
    std::string nameGuard = "categories";
    //The icon definition, from which we try to determine a path to an icon
    std::string iconDef;

    /* This is a kludge. If we already have an icon definition and it is a full
     * path instead of a true definition, then check if it conforms to the 
     * required size and is an xdg directory if that has been requested. 
     * If so, exit here. Otherwise, clear the definition and carry on */
    if (icon != "" && icon.find("/") != std::string::npos && 
            icon.find(iconsXdgSize) != std::string::npos) 
    {   
        if (!iconsXdgOnly || (iconsXdgOnly && 
                icon.find("/share/icons/") != std::string::npos))
            return;
        else
            icon = "";
    }

    //Set the icon definition, either the one we already have or the 
    //category name
    if (icon != "") iconDef = icon;
    else iconDef = name;

    //If we already have a definition, we can get the icon from any directory
    if (icon != "") nameGuard = "/";

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
        if (iconpaths[x].path.find(nameGuard) != std::string::npos && 
                iconpaths[x].path.find(iconDef) != std::string::npos)
        {   
            icon = iconpaths[x].path;
            return;
        }
    }
}
