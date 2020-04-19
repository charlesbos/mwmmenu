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

#include <string>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <boost/algorithm/string/replace.hpp>
#include "MenuWriter.h"
#include "Category.h"

MenuWriter::MenuWriter(const std::string& menuName, int windowmanager, 
        bool useIcons, std::vector<std::string> exclude, std::vector<std::string> excludeMatching,
        std::vector<std::string> excludeCategories, std::vector<std::string> include, 
        std::vector<std::string> excludedFilenames, const std::vector<Category*>& cats) :
    cats(cats),
    menuName(menuName),
    windowmanager(windowmanager),
    useIcons(useIcons),
    exclude(exclude),
    excludeMatching(excludeMatching),
    excludeCategories(excludeCategories),
    include(include),
    excludedFilenames(excludedFilenames)
{   
    std::vector<Category*> usedCats;

    entryDisplayHandler();

    //Get the used categories
    for (unsigned int x = 0; x < cats.size(); x++)
    { 
        if (categoryNotExcluded(cats[x])) 
        {
            usedCats.push_back(cats[x]);
        }
    }

    //Now write the menus
    int maxCatNumber = usedCats.size() - 1;
    for (unsigned int x = 0; x < usedCats.size(); x++)
        writeMenu(usedCats[x], x, usedCats, maxCatNumber);
    //For WMs which require a menu which sources the individual category menus
    if ((windowmanager == mwm ||
            windowmanager == fvwm ||
            windowmanager == fvwm_dynamic ||
            windowmanager == openbox) &&
            !usedCats.empty())
        writeMenu(NULL, MAIN_MENU, usedCats);
}

/* Function to filter out desktop entries specified from the command line
 * based on various criteria. The entries are excluded simply by setting the
 * nodisplay value to true */
void MenuWriter::entryDisplayHandler()
{  
    for (unsigned int w = 0; w < cats.size(); w++)
    {
        std::vector<DesktopFile*> files = cats[w]->getEntriesR();
        if (!exclude.empty())
        {   
            for (unsigned int x = 0; x < files.size(); x++)
                if (find(exclude.begin(), exclude.end(), files[x]->name) != 
                        exclude.end()) 
                    files[x]->nodisplay = true;
        }
        if (!excludeMatching.empty())
        {
            for (unsigned int x = 0; x < files.size(); x++)
            {
                for (unsigned int y = 0; y < excludeMatching.size(); y++)
                {
                    if (files[x]->name.find(excludeMatching[y]) != std::string::npos)
                    {
                        files[x]->nodisplay = true;
                        break;
                    }
                }
            }
        }
        if (!excludedFilenames.empty())
        {   
            for (unsigned int x = 0; x < files.size(); x++)
                if (find(excludedFilenames.begin(), excludedFilenames.end(), 
                        files[x]->filename) != excludedFilenames.end()) 
                    files[x]->nodisplay = true;
        }
        if (!include.empty())
        {   
            for (unsigned int x = 0; x < files.size(); x++)
                if (find(include.begin(), include.end(), files[x]->name) != 
                        include.end()) 
                    files[x]->nodisplay = false;
        }
        if (!excludeCategories.empty())
        {
            if (find(excludeCategories.begin(), excludeCategories.end(),
                    cats[w]->name) != excludeCategories.end())
            {
                cats[w]->nodisplay = true;
                continue;
            }
            std::vector<Category*> subCats = cats[w]->getSubcats();
            for (unsigned int x = 0; x < subCats.size(); x++)
            {
                Category *subCat = subCats[x];
                if (find(excludeCategories.begin(), excludeCategories.end(),
                        subCat->name) != excludeCategories.end())
                {
                    subCat->nodisplay = true;
                }
            }
        }
    }
}

/* A function to check whether a category is not present in the list of 
 * excluded categories and also whether a category has any non-hidden desktop 
 * files. If so, return true, otherwise return false */
bool MenuWriter::categoryNotExcluded(Category* c)
{   
    if (c->nodisplay) return false;
    bool visibleFound = false;
    std::vector<DesktopFile*> entries = c->getEntriesR();
    for (unsigned int x = 0; x < entries.size(); x++)
    {
        if (entries[x]->nodisplay == false)
        {
            visibleFound = true;
            break;
        }
    }
    if (!visibleFound) return false;
    return true;
}

/* This function returns the number of visible entries per category */
int MenuWriter::realMenuSize(std::vector<DesktopFile*> entries)
{
    int size = 0;
    for (unsigned int x = 0; x < entries.size(); x++)
    {
        if (!entries[x]->nodisplay) size++;
    }
    return size;
}

/* This function is called multiple times. Each time, it prints out the submenu
 * for a given category. Some WMs (MWM and FVWM) require a menu which sources
 * the individual category menus. Such a menu will be printed for those window 
 * managers if a negative category number is provided */
void MenuWriter::writeMenu(Category *cat, int catNumber, 
        const std::vector<Category*>& usedCats, int maxCatNumber)
{   
    //Variable for the std::vector of desktop entries
    std::vector<DesktopFile*> dfiles; 
    if (cat != NULL) 
    {
        dfiles = cat->getEntries();
        sort(dfiles.begin(), dfiles.end(), myCompare<DesktopFile>);
    }
    //Variable for the category name
    std::string category; 
    if (cat != NULL) category = cat->name;
    //Variable for the category icon
    std::string catIcon;
    if (cat != NULL) catIcon = cat->icon;
    //Variable for category depth
    int depth = 0;
    if (cat != NULL) depth = cat->depth;
    //Variable for std::vector of subcategories
    std::vector<Category*> subCats;
    if (cat != NULL) 
    {
        subCats = cat->getSubcats();
        sort(subCats.begin(), subCats.end(), myCompare<Category>);
    }
    //Variable for knowing how many items (entries and submenus) there are in
    //a menu
    int numOfItems = 0;
    //Variable for knowing how many entries in a given category have been printed
    int realPos = 0;
    //Variable for a formatted version of the name, e.g. quotes added
    std::string nameFormatted;
    //Variable for a formatted version of the exec, e.g. quotes added
    std::string execFormatted;
    //Variable for a formatted version of the category name, e.g. quotes added
    std::string catFormatted;
    //Variable for a formatted version of the menu, e.g. quotes added
    std::string menuFormatted;

    switch(windowmanager)
    {
        case mwm :
            if (cat != NULL)
            {  
                for (unsigned int x = 0; x < subCats.size(); x++)
                {
                    if (categoryNotExcluded(subCats[x]))
                        writeMenu(subCats[x], SUB_MENU, usedCats);
                }  
                catFormatted = '"' + category + '"';
                std::cout << "menu " << catFormatted << std::endl << "{" << std::endl;
                std::cout << "    " << catFormatted << " " << "f.title" << std::endl;
                writeMenu(NULL, SUB_MENU, subCats);
                for (std::vector<DesktopFile*>::iterator it = dfiles.begin(); 
                        it < dfiles.end(); it++)
                {
                    if ((*it)->nodisplay) continue;
                    nameFormatted = '"' + (*it)->name + '"';
                    execFormatted = "\"exec " + (*it)->exec + " &" + '"';
                    std::cout << "    " << nameFormatted << " " << "f.exec " << 
                        execFormatted << std::endl;
                }
                std::cout << "}" << std::endl << std::endl;
            }
            else
            {  
                if (catNumber == MAIN_MENU)
                { 
                    menuFormatted = '"' + menuName + '"';
                    std::cout << "menu " << menuFormatted << std::endl << "{" << std::endl;
                    std::cout << "    " << menuFormatted << " " << "f.title" << std::endl;
                }
                for (unsigned int x = 0; x < usedCats.size(); x++)
                {  
                    if (catNumber == SUB_MENU)
                        if (!categoryNotExcluded(usedCats[x])) continue;
                    catFormatted = '"' + std::string(usedCats[x]->name) + '"';
                    std::cout << "    " << catFormatted << " " << "f.menu " <<
                        catFormatted << std::endl;
                }
                if (catNumber == MAIN_MENU) std::cout << "}" << std::endl << std::endl;
            }
            break;
        case fvwm :
        case fvwm_dynamic :
            if (cat != NULL)
            {
                for (unsigned int x = 0; x < subCats.size(); x++)
                {
                    if (categoryNotExcluded(subCats[x]))
                        writeMenu(subCats[x], SUB_MENU, usedCats);
                }  
                catFormatted = '"' + category + '"';
                if (windowmanager == fvwm)
                    std::cout << "DestroyMenu " << catFormatted << std::endl;
                else
                    std::cout << "DestroyMenu recreate " << catFormatted << std::endl;
                std::cout << "AddToMenu " << catFormatted << " " << 
                    catFormatted << " Title" << std::endl;
                writeMenu(NULL, SUB_MENU, subCats);
                for (std::vector<DesktopFile*>::iterator it = dfiles.begin(); 
                        it < dfiles.end(); it++)
                {   
                    if ((*it)->nodisplay) continue;
                    if (useIcons && (*it)->icon != "\0") 
                        nameFormatted = '"' + (*it)->name + " %" + 
                            (*it)->icon + "%" + '"';
                    else nameFormatted = '"' + (*it)->name + '"';
                    execFormatted = (*it)->exec;
                    std::cout << "+ " << nameFormatted << " " << "Exec exec " << 
                        execFormatted << std::endl;
                }
                std::cout << std::endl;
            }
            else
            {   
                if (catNumber == MAIN_MENU)
                {
                    menuFormatted = '"' + menuName + '"';
                    if (windowmanager == fvwm)
                        std::cout << "DestroyMenu " << menuFormatted << std::endl;
                    else
                        std::cout << "DestroyMenu recreate " << menuFormatted << std::endl;
                    std::cout << "AddToMenu " << menuFormatted << " " << 
                        menuFormatted << " Title" << std::endl;
                }
                for (unsigned int x = 0; x < usedCats.size(); x++)
                {   
                    if (catNumber == SUB_MENU)
                        if (!categoryNotExcluded(usedCats[x])) continue;
                    if (useIcons)
                    {
                        catIcon = usedCats[x]->icon;
                        if (catIcon != "\0") 
                            catFormatted = '"' +  std::string(usedCats[x]->name) + 
                                " %" + catIcon + "%" + '"';
                        else 
                            catFormatted = '"' + std::string(usedCats[x]->name) + '"';
                    }
                    else 
                        catFormatted = '"' + std::string(usedCats[x]->name) + '"';
                    std::cout << "+ " << catFormatted << " " << "Popup " << 
                        '"' + usedCats[x]->name + '"' << std::endl;
                }
                if (catNumber == MAIN_MENU) std::cout << std::endl;
            }
            break;
        case fluxbox :
            if (catNumber == 0) 
                std::cout << "[submenu] (" << menuName << ')' << std::endl;
            if (useIcons)
            {   
                if (catIcon != "\0") 
                    catFormatted = '(' + category + ") <" + catIcon + '>';
                else 
                    catFormatted = '(' + category + ')';
            }
            else catFormatted = '(' + category + ')';
            for (int x = 0; x < depth; x++) std::cout << "    ";
            std::cout << "    [submenu] " + catFormatted + " {}" << std::endl;
            for (unsigned int x = 0; x < subCats.size(); x++)
            {
                if (categoryNotExcluded(subCats[x]))
                    writeMenu(subCats[x], SUB_MENU, usedCats);
            }
            for (std::vector<DesktopFile*>::iterator it = dfiles.begin(); 
                    it < dfiles.end(); it++)
            {   
                if ((*it)->nodisplay) continue;
                if (useIcons && (*it)->icon != "\0") 
                    execFormatted = 
                        '{' + (*it)->exec + "} <" + (*it)->icon + ">";
                else execFormatted = '{' + (*it)->exec + '}';
                nameFormatted = (*it)->name;
                //If a name has brackets, we need to escape the closing 
                //bracket or it will be missed out
                if (nameFormatted.find(std::string(")").c_str()) != std::string::npos)
                    boost::replace_all(nameFormatted, ")", "\\)");
                nameFormatted = '(' + nameFormatted + ')';
                for (int x = 0; x < depth; x++) std::cout << "    ";
                std::cout << "        [exec] " << nameFormatted << " " << 
                    execFormatted << std::endl;
            }
            for (int x = 0; x < depth; x++) std::cout << "    ";
            std::cout << "    [end]" << std::endl;
            if (catNumber == maxCatNumber) std::cout << "[end]" << std::endl;
            break;
        case openbox :
        case openbox_pipe :
            if (windowmanager == openbox_pipe && catNumber == 0) 
                std::cout << 
                    "<openbox_pipe_menu xmlns=\"http://openbox.org/3.4/menu\">"
                    << std::endl << std::endl;
            if (cat != NULL)
            {  
                if (windowmanager == openbox)
                {
                    for (unsigned int x = 0; x < subCats.size(); x++)
                    {
                        if (categoryNotExcluded(subCats[x]))
                            writeMenu(subCats[x], SUB_MENU, usedCats);
                    }
                }  
                catFormatted = '"' + category + '"';
                if (useIcons)
                {
                    if (catIcon != "\0")
                    {
                        if (windowmanager == openbox_pipe)
                           for (int x = 0; x < depth; x++) std::cout << "    ";
                        std::cout << "<menu id=" << catFormatted << " label=" << 
                            catFormatted << " icon=" << '"' + catIcon + '"' << 
                            ">" << std::endl;
                    }
                    else
                    {
                        if (windowmanager == openbox_pipe)
                            for (int x = 0; x < depth; x++) std::cout << "    ";
                        std::cout << "<menu id=" << catFormatted << " label=" << 
                            catFormatted << ">" << std::endl;
                    }
                }
                else 
                {
                    if (windowmanager == openbox_pipe)
                        for (int x = 0; x < depth; x++) std::cout << "    ";
                    std::cout << "<menu id=" << catFormatted << " label=" << 
                        catFormatted << ">" << std::endl;
                }
                if (windowmanager == openbox) writeMenu(NULL, SUB_MENU, subCats);
                if (windowmanager == openbox_pipe)
                {
                    for (unsigned int x = 0; x < subCats.size(); x++)
                    {
                        if (categoryNotExcluded(subCats[x]))
                            writeMenu(subCats[x], SUB_MENU, usedCats);
                    }
                }  
                for (std::vector<DesktopFile*>::iterator it = dfiles.begin(); 
                        it < dfiles.end(); it++)
                {   
                    if ((*it)->nodisplay) continue;
                    if (useIcons && (*it)->icon != "\0") 
                        nameFormatted = '"' + (*it)->name + '"' + 
                            " icon=\"" + (*it)->icon + "\">";
                    else nameFormatted = '"' + (*it)->name + "\">";
                    execFormatted = (*it)->exec;
                    if (windowmanager == openbox_pipe)
                        for (int x = 0; x < depth; x++) std::cout << "    ";
                    std::cout << "    <item label=" << nameFormatted << std::endl;
                    if (windowmanager == openbox_pipe)
                        for (int x = 0; x < depth; x++) std::cout << "    ";
                    std::cout << "        <action name=\"Execute\">" << std::endl;
                    if (windowmanager == openbox_pipe)
                        for (int x = 0; x < depth; x++) std::cout << "    ";
                    std::cout << "            <execute>" << execFormatted << 
                        "</execute>" << std::endl;
                    if (windowmanager == openbox_pipe)
                        for (int x = 0; x < depth; x++) std::cout << "    ";
                    std::cout << "        </action>" << std::endl;
                    if (windowmanager == openbox_pipe)
                        for (int x = 0; x < depth; x++) std::cout << "    ";
                    std::cout << "    </item>" << std::endl;
                }
                if (windowmanager == openbox_pipe) 
                    for (int x = 0; x < depth; x++) std::cout << "    ";
                if (windowmanager == openbox_pipe && depth != 0)
                    std::cout << "</menu>" << std::endl;
                else std::cout << "</menu>" << std::endl << std::endl;
                if (windowmanager == openbox_pipe && catNumber == maxCatNumber)
                    std::cout << "</openbox_pipe_menu>" << std::endl << std::endl;
            }
            else
            {  
                if (catNumber == MAIN_MENU)
                { 
                    menuFormatted = '"' + menuName + '"';
                    std::cout << "<menu id=" << menuFormatted << " label=" << 
                        menuFormatted << ">" << std::endl;
                }
                for (unsigned int x = 0; x < usedCats.size(); x++)
                {   
                    if (catNumber == SUB_MENU)
                        if (!categoryNotExcluded(usedCats[x])) continue;
                    if (useIcons)
                    {
                        catIcon = usedCats[x]->icon;
                        if (catIcon != "\0") 
                            catFormatted = '"' + std::string(usedCats[x]->name) + 
                                '"' + " icon=\"" + catIcon + "\"/>";
                        else 
                            catFormatted = '"' + std::string(usedCats[x]->name) + 
                                "\"/>";
                    }
                    else 
                        catFormatted = '"' + std::string(usedCats[x]->name) + "\"/>";
                    std::cout << "    <menu id=" << catFormatted << std::endl;
                }
                if (catNumber == MAIN_MENU) std::cout << "</menu>" << std::endl << std::endl;
            }
            break;
        case olvwm :
            if (catNumber == 0) 
                std::cout << '"' + menuName + '"' << " MENU" << std::endl << std::endl;
            catFormatted = '"' + category + '"';
            for (int x = 0; x < depth; x++) std::cout << "    ";
            std::cout << catFormatted << " MENU" << std::endl;
            for (unsigned int x = 0; x < subCats.size(); x++)
            {
                if (categoryNotExcluded(subCats[x]))
                    writeMenu(subCats[x], SUB_MENU, usedCats);
            }
            for (std::vector<DesktopFile*>::iterator it = dfiles.begin(); 
                    it < dfiles.end(); it++)
            {   
                if ((*it)->nodisplay) continue;
                nameFormatted = '"' + (*it)->name + '"';
                execFormatted = (*it)->exec;
                for (int x = 0; x < depth; x++) std::cout << "    ";
                std::cout << nameFormatted << " " << execFormatted << std::endl;
            }
            for (int x = 0; x < depth; x++) std::cout << "    ";
            if (depth == 0)
                std::cout << catFormatted << " END PIN" << std::endl << std::endl;
            else
                std::cout << catFormatted << " END PIN" << std::endl;
            if (catNumber == maxCatNumber) 
                std::cout << '"' + menuName + '"' << " END PIN" << std::endl;
            break;     
        case windowmaker :
            if (catNumber == 0 && depth == 0) 
                std::cout << "(\n    " << '"' << menuName << '"' << ',' << std::endl;
            catFormatted = '"' + category + '"';
            for (int x = 0; x < depth; x++) std::cout << "    ";
            std::cout << "    (" << std::endl;
            for (int x = 0; x < depth; x++) std::cout << "    ";
            std::cout << "        " << catFormatted << ',' << std::endl;
            if (subCats.size() > 0)
            {
                //For Windowmaker we have to exactly how many items there are
                //in menu (submenus + desktop entries) because we have to
                //terminate each entry other than the final one with a comma
                for (unsigned int x = 0; x < subCats.size(); x++)
                    if (categoryNotExcluded(subCats[x])) numOfItems++;
                for (unsigned int x = 0; x < dfiles.size(); x++)
                    if (!dfiles[x]->nodisplay) numOfItems++;
            }
            for (unsigned int x = 0; x < subCats.size(); x++)
            {
                if (categoryNotExcluded(subCats[x]))
                    writeMenu(subCats[x], x + 1, usedCats, numOfItems);
            }
            realPos = 0;
            for (std::vector<DesktopFile*>::iterator it = dfiles.begin(); 
                    it < dfiles.end(); it++)
            {   
                if ((*it)->nodisplay) continue;
                realPos++;
                nameFormatted = '"' + (*it)->name + '"';
                execFormatted = '"' + (*it)->exec + '"';
                for (int x = 0; x < depth; x++) std::cout << "    ";
                std::cout << "        (" << nameFormatted << ", " << "EXEC, " << 
                    execFormatted << ")";
                if (realPos < realMenuSize(dfiles))
                    std::cout << ',' << std::endl;
                else 
                    std::cout << std::endl;
            }
            if (catNumber != maxCatNumber) 
            {
                for (int x = 0; x < depth; x++) std::cout << "    ";
                std::cout << "    )," << std::endl;
            }
            else 
            {
                if (depth == 0) std::cout << "    )\n)" << std::endl;
                else
                {
                    for (int x = 0; x < depth; x++) std::cout << "    ";
                    std::cout << "    )" << std::endl;
                }
            }
            break;
        case icewm :
            if (useIcons)
            {   
                if (catIcon != "\0") 
                    catFormatted = '"' + category + "\" " + catIcon;
                else 
                    catFormatted = '"' + category + "\" -";
            }
            else catFormatted = '"' + category + "\" folder";
            for (int x = 0; x < depth; x++) std::cout << "    ";
            std::cout << "menu " << catFormatted << " {" << std::endl;
            for (unsigned int x = 0; x < subCats.size(); x++)
            {
                if (categoryNotExcluded(subCats[x]))
                    writeMenu(subCats[x], SUB_MENU, usedCats);
            }
            for (std::vector<DesktopFile*>::iterator it = dfiles.begin(); 
                    it < dfiles.end(); it++)
            {   
                if ((*it)->nodisplay) continue;
                if (useIcons && (*it)->icon != "\0") 
                    nameFormatted = '"' + (*it)->name + '"' + " " + 
                        (*it)->icon;
                else nameFormatted = '"' + (*it)->name + "\" -";
                execFormatted = (*it)->exec;
                for (int x = 0; x < depth; x++) std::cout << "    ";
                std::cout << "    prog " + nameFormatted + " " + execFormatted << 
                    std::endl;
            }
            for (int x = 0; x < depth; x++) std::cout << "    ";
            if (depth == 0) std::cout << "}\n" << std::endl;
            else std::cout << "}\n";
            break;
    }
}
