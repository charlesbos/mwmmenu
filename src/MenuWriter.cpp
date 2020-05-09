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

//------------------------------------------------------------------------------

MenuWriter::MenuWriter(WRITER_CONSTRUCT) :
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
    entryDisplayHandler();

    //Get the used categories
    for (unsigned int x = 0; x < cats.size(); x++)
    { 
        if (categoryNotExcluded(cats[x])) 
        {
            usedCats.push_back(cats[x]);
        }
    }
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
int MenuWriter::realNumEntries(std::vector<DesktopFile*> entries)
{
    int size = 0;
    for (unsigned int x = 0; x < entries.size(); x++)
        if (!entries[x]->nodisplay) size++;
    return size;
}

/* This function returns the number of visible subcategories per category */
int MenuWriter::realNumCats(std::vector<Category*> cats)
{
    int size = 0;
    for (unsigned int x = 0; x < cats.size(); x++)
        if (categoryNotExcluded(cats[x])) size++;
    return size;
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

MwmMenuWriter::MwmMenuWriter(WRITER_CONSTRUCT) : MenuWriter(WRITER_PARAMS)
{
    for (unsigned int x = 0; x < usedCats.size(); x++) 
        writeMenu(usedCats[x], x, usedCats.size() - 1);
    if (!usedCats.empty()) writeMainMenu();
}

void MwmMenuWriter::writeMenu(Category *cat, int catNumber, int maxCatNumber)
{
    std::vector<DesktopFile*> dfiles = cat->getEntries();
    std::vector<Category*> subCats = cat->getSubcats();
    for (unsigned int x = 0; x < subCats.size(); x++)
        if (categoryNotExcluded(subCats[x])) writeMenu(subCats[x]);
    std::cout << "menu \"" << cat->name << '"' << std::endl << "{" << std::endl;
    std::cout << "    \"" << cat->name << "\" " << "f.title" << std::endl;
    for (unsigned int x = 0; x < subCats.size(); x++)
    {
        if (categoryNotExcluded(subCats[x]))
            std::cout << "    \"" << subCats[x]->name << "\" " << "f.menu " <<
                    '"' << subCats[x]->name << '"' << std::endl;
    }
    for (std::vector<DesktopFile*>::iterator it = dfiles.begin(); it < dfiles.end(); it++)
    {
        if ((*it)->nodisplay) continue;
        std::cout << "    \"" << (*it)->name << "\" " << "f.exec " << 
            "\"exec " << (*it)->exec << " &\"" << std::endl;
    }
    std::cout << "}" << std::endl << std::endl;
}

void MwmMenuWriter::writeMainMenu()
{
    std::cout << "menu \"" << menuName << '"' << std::endl << "{" << std::endl;
    std::cout << "    \"" << menuName << "\" " << "f.title" << std::endl;
    for (unsigned int x = 0; x < usedCats.size(); x++)
    {  
        std::cout << "    \"" << usedCats[x]->name << "\" " << "f.menu " <<
            '"' << usedCats[x]->name << '"' << std::endl;
    }
    std::cout << "}" << std::endl << std::endl;
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

FvwmMenuWriter::FvwmMenuWriter(WRITER_CONSTRUCT) : MenuWriter(WRITER_PARAMS)
{
    for (unsigned int x = 0; x < usedCats.size(); x++) 
        writeMenu(usedCats[x], x, usedCats.size() - 1);
    if (!usedCats.empty()) writeMainMenu();
}

void FvwmMenuWriter::writeMenu(Category *cat, int catNumber, int maxCatNumber)
{
    std::vector<DesktopFile*> dfiles = cat->getEntries();
    std::vector<Category*> subCats = cat->getSubcats();
    for (unsigned int x = 0; x < subCats.size(); x++)
        if (categoryNotExcluded(subCats[x])) writeMenu(subCats[x]);
    if (windowmanager == fvwm)
        std::cout << "DestroyMenu \"" << cat->name << '"' << std::endl;
    else
        std::cout << "DestroyMenu recreate \"" << cat->name << '"' << std::endl;
    std::cout << "AddToMenu \"" << cat->name << "\" " << 
        '"' << cat->name << "\" Title" << std::endl;
    for (unsigned int x = 0; x < subCats.size(); x++)
    {   
        if (categoryNotExcluded(subCats[x]))
        {
            if (useIcons && subCats[x]->icon != "\0")
            {
                std::cout << "+ \"" << subCats[x]->name << " %" << 
                    subCats[x]->icon << "%\" Popup " << 
                    '"' + subCats[x]->name + '"' << std::endl;
            }
            else
            {
                std::cout << "+ \"" << subCats[x]->name << "\" " << "Popup " << 
                    '"' + subCats[x]->name + '"' << std::endl;
            }
        }
    }
    for (std::vector<DesktopFile*>::iterator it = dfiles.begin(); it < dfiles.end(); it++)
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

void FvwmMenuWriter::writeMainMenu()
{
    if (windowmanager == fvwm)
        std::cout << "DestroyMenu \"" << menuName << '"' << std::endl;
    else
        std::cout << "DestroyMenu recreate \"" << menuName << '"' << std::endl;
    std::cout << "AddToMenu \"" << menuName << "\" " << 
        '"' << menuName << "\" Title" << std::endl;
    for (unsigned int x = 0; x < usedCats.size(); x++)
    {   
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
    std::cout << std::endl;
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

FluxboxMenuWriter::FluxboxMenuWriter(WRITER_CONSTRUCT) : MenuWriter(WRITER_PARAMS)
{
    for (unsigned int x = 0; x < usedCats.size(); x++) 
        writeMenu(usedCats[x], x, usedCats.size() - 1);
}

void FluxboxMenuWriter::writeMenu(Category *cat, int catNumber, int maxCatNumber)
{
    std::vector<DesktopFile*> dfiles = cat->getEntries();
    std::vector<Category*> subCats = cat->getSubcats();
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
        if (categoryNotExcluded(subCats[x])) writeMenu(subCats[x]);
    for (std::vector<DesktopFile*>::iterator it = dfiles.begin(); it < dfiles.end(); it++)
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
    if (catNumber >= 0 && catNumber == maxCatNumber) std::cout << "[end]" << std::endl;
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

OpenboxMenuWriter::OpenboxMenuWriter(WRITER_CONSTRUCT) : MenuWriter(WRITER_PARAMS)
{
    for (unsigned int x = 0; x < usedCats.size(); x++) 
        writeMenu(usedCats[x], x, usedCats.size() - 1);
    if (!usedCats.empty() && windowmanager == openbox) writeMainMenu();
}

void OpenboxMenuWriter::writeMenu(Category *cat, int catNumber, int maxCatNumber)
{
    std::vector<DesktopFile*> dfiles = cat->getEntries();
    std::vector<Category*> subCats = cat->getSubcats();
    if (windowmanager == openbox_pipe && catNumber == 0) 
        std::cout << 
            "<openbox_pipe_menu xmlns=\"http://openbox.org/3.4/menu\">"
            << std::endl << std::endl;
    if (windowmanager == openbox)
    {
        for (unsigned int x = 0; x < subCats.size(); x++)
            if (categoryNotExcluded(subCats[x])) writeMenu(subCats[x]);
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
    if (windowmanager == openbox)
    {
        for (unsigned int x = 0; x < subCats.size(); x++)
        {   
            if (categoryNotExcluded(subCats[x]))
            {
                if (useIcons && subCats[x]->icon != "\0")
                {
                    std::cout << "    <menu id=\"" << subCats[x]->name << "\" icon=\""
                       << subCats[x]->icon << "\"/>" << std::endl;
                }
                else
                {
                    std::cout << "    <menu id=\"" << subCats[x]->name << "\"/>" << std::endl;
                }
            }
        }
    }
    if (windowmanager == openbox_pipe)
    {
        for (unsigned int x = 0; x < subCats.size(); x++)
            if (categoryNotExcluded(subCats[x])) writeMenu(subCats[x]);
    } 
    for (std::vector<DesktopFile*>::iterator it = dfiles.begin(); it < dfiles.end(); it++)
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
    if (windowmanager == openbox_pipe && catNumber >= 0 && catNumber == maxCatNumber)
        std::cout << "</openbox_pipe_menu>" << std::endl << std::endl;
}

void OpenboxMenuWriter::writeMainMenu()
{
    std::cout << "<menu id=\"" << menuName << "\" label=\"" << menuName << "\">" << std::endl;
    for (unsigned int x = 0; x < usedCats.size(); x++)
    {   
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
    std::cout << "</menu>" << std::endl << std::endl;
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

OlvwmMenuWriter::OlvwmMenuWriter(WRITER_CONSTRUCT) : MenuWriter(WRITER_PARAMS)
{
    for (unsigned int x = 0; x < usedCats.size(); x++) 
        writeMenu(usedCats[x], x, usedCats.size() - 1);
}

void OlvwmMenuWriter::writeMenu(Category *cat, int catNumber, int maxCatNumber)
{
    std::vector<DesktopFile*> dfiles = cat->getEntries();
    std::vector<Category*> subCats = cat->getSubcats();
    if (catNumber == 0) 
        std::cout << '"' << menuName << "\" MENU" << std::endl << std::endl;
    for (int x = 0; x < cat->depth; x++) std::cout << "    ";
    std::cout << '"' << cat->name << "\" MENU" << std::endl;
    for (unsigned int x = 0; x < subCats.size(); x++)
        if (categoryNotExcluded(subCats[x])) writeMenu(subCats[x]);
    for (std::vector<DesktopFile*>::iterator it = dfiles.begin(); it < dfiles.end(); it++)
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
    if (catNumber >= 0 && catNumber == maxCatNumber) 
        std::cout << '"' << menuName << "\" END PIN" << std::endl;
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

WmakerMenuWriter::WmakerMenuWriter(WRITER_CONSTRUCT) : MenuWriter(WRITER_PARAMS)
{
    for (unsigned int x = 0; x < usedCats.size(); x++) 
        writeMenu(usedCats[x], x, usedCats.size() - 1);
}

void WmakerMenuWriter::writeMenu(Category *cat, int catNumber, int maxCatNumber)
{
    std::vector<DesktopFile*> dfiles = cat->getEntries();
    std::vector<Category*> subCats = cat->getSubcats();
    int numOfItems = 0;
    int realPos = 0;
    if (catNumber == 0 && cat->depth == 0) 
        std::cout << "(\n    \"" << menuName << "\"," << std::endl;
    for (int x = 0; x < cat->depth; x++) std::cout << "    ";
    std::cout << "    (" << std::endl;
    for (int x = 0; x < cat->depth; x++) std::cout << "    ";
    std::cout << "        \"" << cat->name << "\"," << std::endl;
    //For Windowmaker we have to exactly how many items there are
    //in menu (submenus + desktop entries) because we have to
    //terminate each entry other than the final one with a comma
    if (subCats.size() > 0) numOfItems = realNumEntries(dfiles) + realNumCats(subCats) - 1;
    for (unsigned int x = 0; x < subCats.size(); x++)
        if (categoryNotExcluded(subCats[x])) writeMenu(subCats[x], x, numOfItems);
    realPos = 0;
    for (std::vector<DesktopFile*>::iterator it = dfiles.begin(); it < dfiles.end(); it++)
    {   
        if ((*it)->nodisplay) continue;
        realPos++;
        for (int x = 0; x < cat->depth; x++) std::cout << "    ";
        std::cout << "        (\"" << (*it)->name << "\", " << "EXEC, \"" << 
            (*it)->exec << "\")";
        if (realPos < realNumEntries(dfiles))
            std::cout << ',' << std::endl;
        else 
            std::cout << std::endl;
    }
    if (catNumber >= 0 && catNumber != maxCatNumber) 
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
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

IcewmMenuWriter::IcewmMenuWriter(WRITER_CONSTRUCT) : MenuWriter(WRITER_PARAMS)
{
    for (unsigned int x = 0; x < usedCats.size(); x++) 
        writeMenu(usedCats[x], x, usedCats.size() - 1);
}

void IcewmMenuWriter::writeMenu(Category *cat, int catNumber, int maxCatNumber)
{
    std::vector<DesktopFile*> dfiles = cat->getEntries();
    std::vector<Category*> subCats = cat->getSubcats();
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
        if (categoryNotExcluded(subCats[x])) writeMenu(subCats[x]);
    for (std::vector<DesktopFile*>::iterator it = dfiles.begin(); it < dfiles.end(); it++)
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
}

//------------------------------------------------------------------------------
