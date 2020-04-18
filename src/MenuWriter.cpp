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

MenuWriter::MenuWriter(const string& menuName, int windowmanager, 
        bool useIcons, vector<string> exclude, vector<string> excludeMatching,
        vector<string> excludeCategories, vector<string> include, 
        vector<string> excludedFilenames, const vector<Category*>& cats) :
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
    vector<Category*> usedCats;

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
        vector<DesktopFile*> files = cats[w]->getEntriesR();
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
                    if (files[x]->name.find(excludeMatching[y]) != string::npos)
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
            vector<Category*> subCats = cats[w]->getSubcats();
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
    vector<DesktopFile*> entries = c->getEntriesR();
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
int MenuWriter::realMenuSize(vector<DesktopFile*> entries)
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
        const vector<Category*>& usedCats, int maxCatNumber)
{   
    //Variable for the vector of desktop entries
    vector<DesktopFile*> dfiles; 
    if (cat != NULL) 
    {
        dfiles = cat->getEntries();
        sort(dfiles.begin(), dfiles.end(), myCompare<DesktopFile>);
    }
    //Variable for the category name
    string category; 
    if (cat != NULL) category = cat->name;
    //Variable for the category icon
    string catIcon;
    if (cat != NULL) catIcon = cat->icon;
    //Variable for category depth
    int depth = 0;
    if (cat != NULL) depth = cat->depth;
    //Variable for vector of subcategories
    vector<Category*> subCats;
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
    string nameFormatted;
    //Variable for a formatted version of the exec, e.g. quotes added
    string execFormatted;
    //Variable for a formatted version of the category name, e.g. quotes added
    string catFormatted;
    //Variable for a formatted version of the menu, e.g. quotes added
    string menuFormatted;

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
                cout << "menu " << catFormatted << endl << "{" << endl;
                cout << "    " << catFormatted << " " << "f.title" << endl;
                writeMenu(NULL, SUB_MENU, subCats);
                for (vector<DesktopFile*>::iterator it = dfiles.begin(); 
                        it < dfiles.end(); it++)
                {
                    if ((*it)->nodisplay) continue;
                    nameFormatted = '"' + (*it)->name + '"';
                    execFormatted = "\"exec " + (*it)->exec + " &" + '"';
                    cout << "    " << nameFormatted << " " << "f.exec " << 
                        execFormatted << endl;
                }
                cout << "}" << endl << endl;
            }
            else
            {  
                if (catNumber == MAIN_MENU)
                { 
                    menuFormatted = '"' + menuName + '"';
                    cout << "menu " << menuFormatted << endl << "{" << endl;
                    cout << "    " << menuFormatted << " " << "f.title" << endl;
                }
                for (unsigned int x = 0; x < usedCats.size(); x++)
                {  
                    if (catNumber == SUB_MENU)
                        if (!categoryNotExcluded(usedCats[x])) continue;
                    catFormatted = '"' + string(usedCats[x]->name) + '"';
                    cout << "    " << catFormatted << " " << "f.menu " <<
                        catFormatted << endl;
                }
                if (catNumber == MAIN_MENU) cout << "}" << endl << endl;
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
                    cout << "DestroyMenu " << catFormatted << endl;
                else
                    cout << "DestroyMenu recreate " << catFormatted << endl;
                cout << "AddToMenu " << catFormatted << " " << 
                    catFormatted << " Title" << endl;
                writeMenu(NULL, SUB_MENU, subCats);
                for (vector<DesktopFile*>::iterator it = dfiles.begin(); 
                        it < dfiles.end(); it++)
                {   
                    if ((*it)->nodisplay) continue;
                    if (useIcons && (*it)->icon != "\0") 
                        nameFormatted = '"' + (*it)->name + " %" + 
                            (*it)->icon + "%" + '"';
                    else nameFormatted = '"' + (*it)->name + '"';
                    execFormatted = (*it)->exec;
                    cout << "+ " << nameFormatted << " " << "Exec exec " << 
                        execFormatted << endl;
                }
                cout << endl;
            }
            else
            {   
                if (catNumber == MAIN_MENU)
                {
                    menuFormatted = '"' + menuName + '"';
                    if (windowmanager == fvwm)
                        cout << "DestroyMenu " << menuFormatted << endl;
                    else
                        cout << "DestroyMenu recreate " << menuFormatted << endl;
                    cout << "AddToMenu " << menuFormatted << " " << 
                        menuFormatted << " Title" << endl;
                }
                for (unsigned int x = 0; x < usedCats.size(); x++)
                {   
                    if (catNumber == SUB_MENU)
                        if (!categoryNotExcluded(usedCats[x])) continue;
                    if (useIcons)
                    {
                        catIcon = usedCats[x]->icon;
                        if (catIcon != "\0") 
                            catFormatted = '"' +  string(usedCats[x]->name) + 
                                " %" + catIcon + "%" + '"';
                        else 
                            catFormatted = '"' + string(usedCats[x]->name) + '"';
                    }
                    else 
                        catFormatted = '"' + string(usedCats[x]->name) + '"';
                    cout << "+ " << catFormatted << " " << "Popup " << 
                        '"' + usedCats[x]->name + '"' << endl;
                }
                if (catNumber == MAIN_MENU) cout << endl;
            }
            break;
        case fluxbox :
            if (catNumber == 0) 
                cout << "[submenu] (" << menuName << ')' << endl;
            if (useIcons)
            {   
                if (catIcon != "\0") 
                    catFormatted = '(' + category + ") <" + catIcon + '>';
                else 
                    catFormatted = '(' + category + ')';
            }
            else catFormatted = '(' + category + ')';
            for (int x = 0; x < depth; x++) cout << "    ";
            cout << "    [submenu] " + catFormatted + " {}" << endl;
            for (unsigned int x = 0; x < subCats.size(); x++)
            {
                if (categoryNotExcluded(subCats[x]))
                    writeMenu(subCats[x], SUB_MENU, usedCats);
            }
            for (vector<DesktopFile*>::iterator it = dfiles.begin(); 
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
                if (nameFormatted.find(string(")").c_str()) != string::npos)
                    boost::replace_all(nameFormatted, ")", "\\)");
                nameFormatted = '(' + nameFormatted + ')';
                for (int x = 0; x < depth; x++) cout << "    ";
                cout << "        [exec] " << nameFormatted << " " << 
                    execFormatted << endl;
            }
            for (int x = 0; x < depth; x++) cout << "    ";
            cout << "    [end]" << endl;
            if (catNumber == maxCatNumber) cout << "[end]" << endl;
            break;
        case openbox :
        case openbox_pipe :
            if (windowmanager == openbox_pipe && catNumber == 0) 
                cout << 
                    "<openbox_pipe_menu xmlns=\"http://openbox.org/3.4/menu\">"
                    << endl << endl;
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
                           for (int x = 0; x < depth; x++) cout << "    ";
                        cout << "<menu id=" << catFormatted << " label=" << 
                            catFormatted << " icon=" << '"' + catIcon + '"' << 
                            ">" << endl;
                    }
                    else
                    {
                        if (windowmanager == openbox_pipe)
                            for (int x = 0; x < depth; x++) cout << "    ";
                        cout << "<menu id=" << catFormatted << " label=" << 
                            catFormatted << ">" << endl;
                    }
                }
                else 
                {
                    if (windowmanager == openbox_pipe)
                        for (int x = 0; x < depth; x++) cout << "    ";
                    cout << "<menu id=" << catFormatted << " label=" << 
                        catFormatted << ">" << endl;
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
                for (vector<DesktopFile*>::iterator it = dfiles.begin(); 
                        it < dfiles.end(); it++)
                {   
                    if ((*it)->nodisplay) continue;
                    if (useIcons && (*it)->icon != "\0") 
                        nameFormatted = '"' + (*it)->name + '"' + 
                            " icon=\"" + (*it)->icon + "\">";
                    else nameFormatted = '"' + (*it)->name + "\">";
                    execFormatted = (*it)->exec;
                    if (windowmanager == openbox_pipe)
                        for (int x = 0; x < depth; x++) cout << "    ";
                    cout << "    <item label=" << nameFormatted << endl;
                    if (windowmanager == openbox_pipe)
                        for (int x = 0; x < depth; x++) cout << "    ";
                    cout << "        <action name=\"Execute\">" << endl;
                    if (windowmanager == openbox_pipe)
                        for (int x = 0; x < depth; x++) cout << "    ";
                    cout << "            <execute>" << execFormatted << 
                        "</execute>" << endl;
                    if (windowmanager == openbox_pipe)
                        for (int x = 0; x < depth; x++) cout << "    ";
                    cout << "        </action>" << endl;
                    if (windowmanager == openbox_pipe)
                        for (int x = 0; x < depth; x++) cout << "    ";
                    cout << "    </item>" << endl;
                }
                if (windowmanager == openbox_pipe) 
                    for (int x = 0; x < depth; x++) cout << "    ";
                if (windowmanager == openbox_pipe && depth != 0)
                    cout << "</menu>" << endl;
                else cout << "</menu>" << endl << endl;
                if (windowmanager == openbox_pipe && catNumber == maxCatNumber)
                    cout << "</openbox_pipe_menu>" << endl << endl;
            }
            else
            {  
                if (catNumber == MAIN_MENU)
                { 
                    menuFormatted = '"' + menuName + '"';
                    cout << "<menu id=" << menuFormatted << " label=" << 
                        menuFormatted << ">" << endl;
                }
                for (unsigned int x = 0; x < usedCats.size(); x++)
                {   
                    if (catNumber == SUB_MENU)
                        if (!categoryNotExcluded(usedCats[x])) continue;
                    if (useIcons)
                    {
                        catIcon = usedCats[x]->icon;
                        if (catIcon != "\0") 
                            catFormatted = '"' + string(usedCats[x]->name) + 
                                '"' + " icon=\"" + catIcon + "\"/>";
                        else 
                            catFormatted = '"' + string(usedCats[x]->name) + 
                                "\"/>";
                    }
                    else 
                        catFormatted = '"' + string(usedCats[x]->name) + "\"/>";
                    cout << "    <menu id=" << catFormatted << endl;
                }
                if (catNumber == MAIN_MENU) cout << "</menu>" << endl << endl;
            }
            break;
        case olvwm :
            if (catNumber == 0) 
                cout << '"' + menuName + '"' << " MENU" << endl << endl;
            catFormatted = '"' + category + '"';
            for (int x = 0; x < depth; x++) cout << "    ";
            cout << catFormatted << " MENU" << endl;
            for (unsigned int x = 0; x < subCats.size(); x++)
            {
                if (categoryNotExcluded(subCats[x]))
                    writeMenu(subCats[x], SUB_MENU, usedCats);
            }
            for (vector<DesktopFile*>::iterator it = dfiles.begin(); 
                    it < dfiles.end(); it++)
            {   
                if ((*it)->nodisplay) continue;
                nameFormatted = '"' + (*it)->name + '"';
                execFormatted = (*it)->exec;
                for (int x = 0; x < depth; x++) cout << "    ";
                cout << nameFormatted << " " << execFormatted << endl;
            }
            for (int x = 0; x < depth; x++) cout << "    ";
            if (depth == 0)
                cout << catFormatted << " END PIN" << endl << endl;
            else
                cout << catFormatted << " END PIN" << endl;
            if (catNumber == maxCatNumber) 
                cout << '"' + menuName + '"' << " END PIN" << endl;
            break;     
        case windowmaker :
            if (catNumber == 0 && depth == 0) 
                cout << "(\n    " << '"' << menuName << '"' << ',' << endl;
            catFormatted = '"' + category + '"';
            for (int x = 0; x < depth; x++) cout << "    ";
            cout << "    (" << endl;
            for (int x = 0; x < depth; x++) cout << "    ";
            cout << "        " << catFormatted << ',' << endl;
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
            for (vector<DesktopFile*>::iterator it = dfiles.begin(); 
                    it < dfiles.end(); it++)
            {   
                if ((*it)->nodisplay) continue;
                realPos++;
                nameFormatted = '"' + (*it)->name + '"';
                execFormatted = '"' + (*it)->exec + '"';
                for (int x = 0; x < depth; x++) cout << "    ";
                cout << "        (" << nameFormatted << ", " << "EXEC, " << 
                    execFormatted << ")";
                if (realPos < realMenuSize(dfiles))
                    cout << ',' << endl;
                else 
                    cout << endl;
            }
            if (catNumber != maxCatNumber) 
            {
                for (int x = 0; x < depth; x++) cout << "    ";
                cout << "    )," << endl;
            }
            else 
            {
                if (depth == 0) cout << "    )\n)" << endl;
                else
                {
                    for (int x = 0; x < depth; x++) cout << "    ";
                    cout << "    )" << endl;
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
            for (int x = 0; x < depth; x++) cout << "    ";
            cout << "menu " << catFormatted << " {" << endl;
            for (unsigned int x = 0; x < subCats.size(); x++)
            {
                if (categoryNotExcluded(subCats[x]))
                    writeMenu(subCats[x], SUB_MENU, usedCats);
            }
            for (vector<DesktopFile*>::iterator it = dfiles.begin(); 
                    it < dfiles.end(); it++)
            {   
                if ((*it)->nodisplay) continue;
                if (useIcons && (*it)->icon != "\0") 
                    nameFormatted = '"' + (*it)->name + '"' + " " + 
                        (*it)->icon;
                else nameFormatted = '"' + (*it)->name + "\" -";
                execFormatted = (*it)->exec;
                for (int x = 0; x < depth; x++) cout << "    ";
                cout << "    prog " + nameFormatted + " " + execFormatted << 
                    endl;
            }
            for (int x = 0; x < depth; x++) cout << "    ";
            if (depth == 0) cout << "}\n" << endl;
            else cout << "}\n";
            break;
    }
}
