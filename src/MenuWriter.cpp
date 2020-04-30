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
    std::vector<DesktopFile*> dfiles; 
    if (cat != NULL) 
    {
        dfiles = cat->getEntries();
        sort(dfiles.begin(), dfiles.end(), myCompare<DesktopFile>);
    }

    std::vector<Category*> subCats;
    if (cat != NULL) 
    {
        subCats = cat->getSubcats();
        sort(subCats.begin(), subCats.end(), myCompare<Category>);
    }

    switch(windowmanager)
    {
        case mwm :
        {
            if (cat != NULL)
            {  
                for (unsigned int x = 0; x < subCats.size(); x++)
                {
                    if (categoryNotExcluded(subCats[x]))
                        writeMenu(subCats[x], SUB_MENU, usedCats);
                }  
                std::cout << "menu \"" << cat->name << '"' << std::endl << "{" << std::endl;
                std::cout << "    \"" << cat->name << "\" " << "f.title" << std::endl;
                writeMenu(NULL, SUB_MENU, subCats);
                for (std::vector<DesktopFile*>::iterator it = dfiles.begin(); 
                        it < dfiles.end(); it++)
                {
                    if ((*it)->nodisplay) continue;
                    std::cout << "    \"" << (*it)->name << "\" " << "f.exec " << 
                        "\"exec " << (*it)->exec << " &\"" << std::endl;
                }
                std::cout << "}" << std::endl << std::endl;
            }
            else
            {  
                if (catNumber == MAIN_MENU)
                { 
                    std::cout << "menu \"" << menuName << '"' << std::endl << "{" << std::endl;
                    std::cout << "    \"" << menuName << "\" " << "f.title" << std::endl;
                }
                for (unsigned int x = 0; x < usedCats.size(); x++)
                {  
                    if (catNumber == SUB_MENU)
                        if (!categoryNotExcluded(usedCats[x])) continue;
                    std::cout << "    \"" << usedCats[x]->name << "\" " << "f.menu " <<
                        '"' << usedCats[x]->name << '"' << std::endl;
                }
                if (catNumber == MAIN_MENU) std::cout << "}" << std::endl << std::endl;
            }
            break;
        }
        case fvwm :
        case fvwm_dynamic :
        {
            if (cat != NULL)
            {
                for (unsigned int x = 0; x < subCats.size(); x++)
                {
                    if (categoryNotExcluded(subCats[x]))
                        writeMenu(subCats[x], SUB_MENU, usedCats);
                }  
                if (windowmanager == fvwm)
                    std::cout << "DestroyMenu \"" << cat->name << '"' << std::endl;
                else
                    std::cout << "DestroyMenu recreate \"" << cat->name << '"' << std::endl;
                std::cout << "AddToMenu \"" << cat->name << "\" " << 
                    '"' << cat->name << "\" Title" << std::endl;
                writeMenu(NULL, SUB_MENU, subCats);
                for (std::vector<DesktopFile*>::iterator it = dfiles.begin(); 
                        it < dfiles.end(); it++)
                {   
                    if ((*it)->nodisplay) continue;
                    if (useIcons && (*it)->icon != "\0")
                    {
                        std::cout << "+ \"" << (*it)->name << " %" << 
                            (*it)->icon << "%\" Exec exec " << 
                            (*it)->exec << std::endl;
                    }
                    else
                    {
                        std::cout << "+ \"" << (*it)->name << "\" " << "Exec exec " << 
                            (*it)->exec << std::endl;
                    }
                }
                std::cout << std::endl;
            }
            else
            {   
                if (catNumber == MAIN_MENU)
                {
                    if (windowmanager == fvwm)
                        std::cout << "DestroyMenu \"" << menuName << '"' << std::endl;
                    else
                        std::cout << "DestroyMenu recreate \"" << menuName << '"' << std::endl;
                    std::cout << "AddToMenu \"" << menuName << "\" " << 
                        '"' << menuName << "\" Title" << std::endl;
                }
                for (unsigned int x = 0; x < usedCats.size(); x++)
                {   
                    if (catNumber == SUB_MENU && !categoryNotExcluded(usedCats[x])) continue;
                    if (useIcons && usedCats[x]->icon != "\0")
                    {
                        std::cout << "+ \"" << usedCats[x]->name << " %" << 
                            usedCats[x]->icon << "%\" Popup " << 
                            '"' + usedCats[x]->name + '"' << std::endl;
                    }
                    else
                    {
                        std::cout << "+ \"" << usedCats[x]->name << "\" " << "Popup " << 
                            '"' + usedCats[x]->name + '"' << std::endl;
                    }
                }
                if (catNumber == MAIN_MENU) std::cout << std::endl;
            }
            break;
        }
        case fluxbox :
        {
            if (catNumber == 0) 
                std::cout << "[submenu] (" << menuName << ')' << std::endl;
            for (int x = 0; x < cat->depth; x++) std::cout << "    ";
            if (useIcons && cat->icon != "\0")
            {
                std::cout << "    [submenu] (" << cat->name << ") <" << cat->icon 
                    << "> {}" << std::endl;
            }
            else
            {
                std::cout << "    [submenu] (" << cat->name << ") {}" << std::endl;
            }
            for (unsigned int x = 0; x < subCats.size(); x++)
            {
                if (categoryNotExcluded(subCats[x]))
                    writeMenu(subCats[x], SUB_MENU, usedCats);
            }
            for (std::vector<DesktopFile*>::iterator it = dfiles.begin(); 
                    it < dfiles.end(); it++)
            {   
                if ((*it)->nodisplay) continue;
                for (int x = 0; x < cat->depth; x++) std::cout << "    ";
                std::string theName = (*it)->name;
                //If a name has brackets, we need to escape the closing
                //bracket or it will be missed out
                boost::replace_all(theName, ")", "\\)");
                std::cout << "        [exec] (" << theName << ") " << 
                    "{" << (*it)->exec << "}";
                if (useIcons && (*it)->icon != "\0")
                    std::cout << " <" << (*it)->icon << ">" << std::endl;
                else
                    std::cout << std::endl;
            }
            for (int x = 0; x < cat->depth; x++) std::cout << "    ";
            std::cout << "    [end]" << std::endl;
            if (catNumber == maxCatNumber) std::cout << "[end]" << std::endl;
            break;
        }
        case openbox :
        case openbox_pipe :
        {
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
                if (useIcons)
                {
                    if (cat->icon != "\0")
                    {
                        if (windowmanager == openbox_pipe)
                           for (int x = 0; x < cat->depth; x++) std::cout << "    ";
                        std::cout << "<menu id=\"" << cat->name << "\" label=\"" << 
                            cat->name << "\" icon=" << '"' + cat->icon + '"' << 
                            ">" << std::endl;
                    }
                    else
                    {
                        if (windowmanager == openbox_pipe)
                            for (int x = 0; x < cat->depth; x++) std::cout << "    ";
                        std::cout << "<menu id=\"" << cat->name << "\" label=\"" << 
                            cat->name << "\">" << std::endl;
                    }
                }
                else 
                {
                    if (windowmanager == openbox_pipe)
                        for (int x = 0; x < cat->depth; x++) std::cout << "    ";
                    std::cout << "<menu id=\"" << cat->name << "\" label=\"" << 
                        cat->name << "\">" << std::endl;
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
                    if (windowmanager == openbox_pipe)
                        for (int x = 0; x < cat->depth; x++) std::cout << "    ";
                    if (useIcons && (*it)->icon != "\0")
                    {
                        std::cout << "    <item label=\"" << (*it)->name << "\" icon=\""
                           << (*it)->icon << "\">" << std::endl;
                    }
                    else
                    {
                        std::cout << "    <item label=\"" << (*it)->name << "\">" << std::endl;
                    }
                    if (windowmanager == openbox_pipe)
                        for (int x = 0; x < cat->depth; x++) std::cout << "    ";
                    std::cout << "        <action name=\"Execute\">" << std::endl;
                    if (windowmanager == openbox_pipe)
                        for (int x = 0; x < cat->depth; x++) std::cout << "    ";
                    std::cout << "            <execute>" << (*it)->exec << 
                        "</execute>" << std::endl;
                    if (windowmanager == openbox_pipe)
                        for (int x = 0; x < cat->depth; x++) std::cout << "    ";
                    std::cout << "        </action>" << std::endl;
                    if (windowmanager == openbox_pipe)
                        for (int x = 0; x < cat->depth; x++) std::cout << "    ";
                    std::cout << "    </item>" << std::endl;
                }
                if (windowmanager == openbox_pipe) 
                    for (int x = 0; x < cat->depth; x++) std::cout << "    ";
                if (windowmanager == openbox_pipe && cat->depth != 0)
                    std::cout << "</menu>" << std::endl;
                else std::cout << "</menu>" << std::endl << std::endl;
                if (windowmanager == openbox_pipe && catNumber == maxCatNumber)
                    std::cout << "</openbox_pipe_menu>" << std::endl << std::endl;
            }
            else
            {  
                if (catNumber == MAIN_MENU)
                { 
                    std::cout << "<menu id=\"" << menuName << "\" label=\"" << 
                        menuName << "\">" << std::endl;
                }
                for (unsigned int x = 0; x < usedCats.size(); x++)
                {   
                    if (catNumber == SUB_MENU && !categoryNotExcluded(usedCats[x])) continue;
                    if (useIcons && usedCats[x]->icon != "\0")
                    {
                        std::cout << "    <menu id=\"" << usedCats[x]->name << "\" icon=\""
                           << usedCats[x]->icon << "\"/>" << std::endl;
                    }
                    else
                    {
                        std::cout << "    <menu id=\"" << usedCats[x]->name << "\"/>" << std::endl;
                    }
                }
                if (catNumber == MAIN_MENU) std::cout << "</menu>" << std::endl << std::endl;
            }
            break;
        }
        case olvwm :
        {
            if (catNumber == 0) 
                std::cout << '"' << menuName << "\" MENU" << std::endl << std::endl;
            for (int x = 0; x < cat->depth; x++) std::cout << "    ";
            std::cout << '"' << cat->name << "\" MENU" << std::endl;
            for (unsigned int x = 0; x < subCats.size(); x++)
            {
                if (categoryNotExcluded(subCats[x]))
                    writeMenu(subCats[x], SUB_MENU, usedCats);
            }
            for (std::vector<DesktopFile*>::iterator it = dfiles.begin(); 
                    it < dfiles.end(); it++)
            {   
                if ((*it)->nodisplay) continue;
                for (int x = 0; x < cat->depth; x++) std::cout << "    ";
                std::cout << '"' << (*it)->name << "\" " << (*it)->exec << std::endl;
            }
            for (int x = 0; x < cat->depth; x++) std::cout << "    ";
            if (cat->depth == 0)
                std::cout << '"' << cat->name << "\" END PIN" << std::endl << std::endl;
            else
                std::cout << '"' << cat->name << "\" END PIN" << std::endl;
            if (catNumber == maxCatNumber) 
                std::cout << '"' << menuName << "\" END PIN" << std::endl;
            break;
        }
        case windowmaker : 
        {
            int numOfItems = 0;
            int realPos = 0;
            if (catNumber == 0 && cat->depth == 0) 
                std::cout << "(\n    \"" << menuName << "\"," << std::endl;
            for (int x = 0; x < cat->depth; x++) std::cout << "    ";
            std::cout << "    (" << std::endl;
            for (int x = 0; x < cat->depth; x++) std::cout << "    ";
            std::cout << "        \"" << cat->name << "\"," << std::endl;
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
                for (int x = 0; x < cat->depth; x++) std::cout << "    ";
                std::cout << "        (\"" << (*it)->name << "\", " << "EXEC, \"" << 
                    (*it)->exec << "\")";
                if (realPos < realMenuSize(dfiles))
                    std::cout << ',' << std::endl;
                else 
                    std::cout << std::endl;
            }
            if (catNumber != maxCatNumber) 
            {
                for (int x = 0; x < cat->depth; x++) std::cout << "    ";
                std::cout << "    )," << std::endl;
            }
            else 
            {
                if (cat->depth == 0) std::cout << "    )\n)" << std::endl;
                else
                {
                    for (int x = 0; x < cat->depth; x++) std::cout << "    ";
                    std::cout << "    )" << std::endl;
                }
            }
            break;
        }
        case icewm :
        {
            for (int x = 0; x < cat->depth; x++) std::cout << "    ";
            if (useIcons)
            {
                if (cat->icon != "\0")
                    std::cout << "menu \"" << cat->name << "\" " << cat->icon << " {" << std::endl;
                else
                    std::cout << "menu \"" << cat->name << "\" - {" << std::endl;
            }
            else
            {
                std::cout << "menu \"" << cat->name << "\" folder {" << std::endl;
            }
            for (unsigned int x = 0; x < subCats.size(); x++)
            {
                if (categoryNotExcluded(subCats[x]))
                    writeMenu(subCats[x], SUB_MENU, usedCats);
            }
            for (std::vector<DesktopFile*>::iterator it = dfiles.begin(); 
                    it < dfiles.end(); it++)
            {   
                if ((*it)->nodisplay) continue;
                for (int x = 0; x < cat->depth; x++) std::cout << "    ";
                if (useIcons && (*it)->icon != "\0")
                {
                    std::cout << "    prog \"" << (*it)->name << "\" " << 
                        (*it)->icon << " " << (*it)->exec << std::endl;
                }
                else
                {
                    std::cout << "    prog \"" + (*it)->name + "\" - " << 
                        (*it)->exec << std::endl;
                }
            }
            for (int x = 0; x < cat->depth; x++) std::cout << "    ";
            if (cat->depth == 0) std::cout << "}\n" << std::endl;
            else std::cout << "}\n";
            break;
        }
    }
}
