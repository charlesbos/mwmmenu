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
#include "MenuWriter.h"
#include "Category.h"

MenuWriter::MenuWriter(const vector<DesktopFile*>& files, 
        const string& menuName, int windowmanager, bool useIcons, 
        vector<string> exclude, vector<string> excludeMatching, 
        vector<string> excludeCategories, vector<string> include, 
        vector<string> excludedFilenames, const vector<Category*>& cats)
{   
    this->files = files;
    this->cats = cats;
    this->menuName = menuName;
    this->windowmanager = windowmanager;
    this->useIcons = useIcons;
    this->exclude = exclude;
    this->excludeMatching = excludeMatching;
    this->excludeCategories = excludeCategories;
    this->include = include;
    this->excludedFilenames = excludedFilenames;
    printHandler();
}

/* Handles the fetching of the desktop entries assigned to each category, 
 * the exclusion of categories or entries based on command 
 * line arguments and then the printing of the menus themselves */
void MenuWriter::printHandler()
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
    for (unsigned int x = 0; x < usedCats.size(); x++)
        writeMenu(x, usedCats);
    //For WMs which require a menu which sources the individual category menus
    if ((windowmanager == mwm ||
            windowmanager == fvwm ||
            windowmanager == fvwm_dynamic ||
            windowmanager == openbox) &&
            !usedCats.empty())
        writeMenu(-1, usedCats);
}

/* Function to filter out desktop entries specified from the command line
 * based on various criteria. The entries are excluded simply by setting the
 * nodisplay value to true */
void MenuWriter::entryDisplayHandler()
{   
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
}

/* A function to check whether a category is not present in the list of 
 * excluded categories and also whether a category has any non-hidden desktop 
 * files. If so, return true, otherwise return false */
bool MenuWriter::categoryNotExcluded(Category* c)
{   
    if (find(excludeCategories.begin(), excludeCategories.end(), c->name) != 
            excludeCategories.end())
        return false;
    bool visibleFound = false;
    for (unsigned int x = 0; x < c->incEntries.size(); x++)
    {
        if (c->incEntries[x]->nodisplay == false)
        {
            visibleFound = true;
            break;
        }
    }
    if (!visibleFound) return false;
    return true;
}

/* This function is called multiple times. Each time, it prints out the submenu
 * for a given category. Some WMs (MWM and FVWM) require a menu which sources
 * the individual category menus. Such a menu will be printed for those window 
 * managers if a negative category number is provided */
void MenuWriter::writeMenu(int catNumber, const vector<Category*>& usedCats)
{   
    //Variable for the vector of desktop entries
    vector<DesktopFile*> dfiles; 
    if (catNumber >= 0) dfiles = usedCats[catNumber]->incEntries; 
    //Variable for the category name
    string category; 
    if (catNumber >= 0) category = usedCats[catNumber]->name;
    //Variable for the category icon
    string catIcon;
    if (catNumber >= 0) catIcon = usedCats[catNumber]->icon;
    //Variable for knowing when the last category has been reached
    int maxCatNumber = usedCats.size() - 1;
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
            if (catNumber >= 0)
            {  
                catFormatted = '"' + category + '"';
                cout << "menu " << catFormatted << endl << "{" << endl;
                cout << "    " << catFormatted << " " << "f.title" << endl;
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
                menuFormatted = '"' + menuName + '"';
                cout << "menu " << menuFormatted << endl << "{" << endl;
                cout << "    " << menuFormatted << " " << "f.title" << endl;
                for (unsigned int x = 0; x < usedCats.size(); x++)
                {   
                    catFormatted = '"' + string(usedCats[x]->name) + '"';
                    cout << "    " << catFormatted << " " << "f.menu " <<
                        catFormatted << endl;
                }
                cout << "}" << endl << endl;
            }
            break;
        case fvwm :
        case fvwm_dynamic :
            if (catNumber >= 0)
            {   
                catFormatted = '"' + category + '"';
                if (windowmanager == fvwm)
                    cout << "DestroyMenu " << catFormatted << endl;
                else
                    cout << "DestroyMenu recreate " << catFormatted << endl;
                cout << "AddToMenu " << catFormatted << " " << 
                    catFormatted << " Title" << endl;
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
                menuFormatted = '"' + menuName + '"';
                if (windowmanager == fvwm)
                    cout << "DestroyMenu " << menuFormatted << endl;
                else
                    cout << "DestroyMenu recreate " << menuFormatted << endl;
                cout << "AddToMenu " << menuFormatted << " " << 
                    menuFormatted << " Title" << endl;
                for (unsigned int x = 0; x < usedCats.size(); x++)
                {   
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
                cout << endl;
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
            cout << "    [submenu] " + catFormatted + " {}" << endl;
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
                    nameFormatted.insert(static_cast<int>(
                        nameFormatted.find_last_of(')')), string("\\").c_str());
                nameFormatted = '(' + nameFormatted + ')';
                cout << "        [exec] " << nameFormatted << " " << 
                    execFormatted << endl;
            }
            cout << "    [end]" << endl;
            if (catNumber == maxCatNumber) cout << "[end]" << endl;
            break;
        case openbox :
        case openbox_pipe :
            if (windowmanager == openbox_pipe && catNumber == 0) 
                cout << 
                    "<openbox_pipe_menu xmlns=\"http://openbox.org/3.4/menu\">"
                    << endl << endl;
            if (catNumber >= 0)
            {   
                catFormatted = '"' + category + '"';
                if (useIcons)
                {
                    if (catIcon != "\0") 
                        cout << "<menu id=" << catFormatted << " label=" << 
                            catFormatted << " icon=" << '"' + catIcon + '"' << 
                            ">" << endl;
                    else 
                        cout << "<menu id=" << catFormatted << " label=" << 
                            catFormatted << ">" << endl;
                }
                else 
                    cout << "<menu id=" << catFormatted << " label=" << 
                        catFormatted << ">" << endl;
                for (vector<DesktopFile*>::iterator it = dfiles.begin(); 
                        it < dfiles.end(); it++)
                {   
                    if ((*it)->nodisplay) continue;
                    if (useIcons && (*it)->icon != "\0") 
                        nameFormatted = '"' + (*it)->name + '"' + 
                            " icon=\"" + (*it)->icon + "\">";
                    else nameFormatted = '"' + (*it)->name + "\">";
                    execFormatted = (*it)->exec;
                    cout << "    <item label=" << nameFormatted << endl;
                    cout << "        <action name=\"Execute\">" << endl;
                    cout << "            <execute>" << execFormatted << 
                        "</execute>" << endl;
                    cout << "        </action>" << endl;
                    cout << "    </item>" << endl;
                }
                cout << "</menu>" << endl << endl;
                if (windowmanager == openbox_pipe && catNumber == maxCatNumber)
                    cout << "</openbox_pipe_menu>" << endl << endl;
            }
            else
            {   
                menuFormatted = '"' + menuName + '"';
                cout << "<menu id=" << menuFormatted << " label=" << 
                    menuFormatted << ">" << endl;
                for (unsigned int x = 0; x < usedCats.size(); x++)
                {   
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
                cout << "</menu>" << endl << endl;
            }
            break;
        case olvwm :
            if (catNumber == 0) 
                cout << '"' + menuName + '"' << " MENU" << endl << endl;
            catFormatted = '"' + category + '"';
            cout << catFormatted << " MENU" << endl;
            for (vector<DesktopFile*>::iterator it = dfiles.begin(); 
                    it < dfiles.end(); it++)
            {   
                if ((*it)->nodisplay) continue;
                nameFormatted = '"' + (*it)->name + '"';
                execFormatted = (*it)->exec;
                cout << nameFormatted << " " << execFormatted << endl;
            }
            cout << catFormatted << " END PIN" << endl << endl;
            if (catNumber == maxCatNumber) 
                cout << '"' + menuName + '"' << " END PIN" << endl;
            break;     
        case windowmaker :
            if (catNumber == 0) 
                cout << "(\n    " << '"' << menuName << '"' << ',' << endl;
            catFormatted = '"' + category + '"';
            cout << "    (\n        " << catFormatted << ',' << endl;
            for (vector<DesktopFile*>::iterator it = dfiles.begin(); 
                    it < dfiles.end(); it++)
            {   
                if ((*it)->nodisplay) continue;
                nameFormatted = '"' + (*it)->name + '"';
                execFormatted = '"' + (*it)->exec + '"';
                cout << "        (" << nameFormatted << ", " << "EXEC, " << 
                    execFormatted << ")";
                if ((it - dfiles.begin()) != 
                        (dfiles.end() - dfiles.begin() - 1)) 
                    cout << ',' << endl;
                else cout << endl;
            }
            if (catNumber != maxCatNumber) cout << "    )," << endl;
            else cout << "    )\n)" << endl;
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
            cout << "menu " << catFormatted << " {" << endl;
            for (vector<DesktopFile*>::iterator it = dfiles.begin(); 
                    it < dfiles.end(); it++)
            {   
                if ((*it)->nodisplay) continue;
                if (useIcons && (*it)->icon != "\0") 
                    nameFormatted = '"' + (*it)->name + '"' + " " + 
                        (*it)->icon;
                else nameFormatted = '"' + (*it)->name + "\" -";
                execFormatted = (*it)->exec;
                cout << "    prog " + nameFormatted + " " + execFormatted << 
                    endl;
            }
            cout << "}\n" << endl;
            break;
    }
}
