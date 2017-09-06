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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <set>
#include <string.h>
#include "DesktopFile.h"
#include "Category.h"

DesktopFile::DesktopFile() {}

DesktopFile::DesktopFile(const char *filename, vector<string> showFromDesktops, bool useIcons, vector<IconSpec>& iconpaths, vector<Category>& cats) 
{	this->filename = filename;
	this->nodisplay = false;
	dfile.open(filename);
	if (!dfile);
	else
	{	populate(showFromDesktops, useIcons, iconpaths, cats);
		dfile.close();
	}
}

DesktopFile::DesktopFile(const DesktopFile& df)
{	this->filename = df.filename;
	this->name = df.name;
	this->exec = df.exec;
	this->nodisplay = df.nodisplay;
	this->icon = df.icon;
}

DesktopFile& DesktopFile::operator=(const DesktopFile& df)
{	this->filename = df.filename;
	this->name = df.name;
	this->exec = df.exec;
	this->nodisplay = df.nodisplay;
	this->icon = df.icon;
	return *this;
}

bool DesktopFile::operator<(const DesktopFile& df)
{	string name_a = this->name;
	string name_b = df.name;
	for (unsigned int x = 0; x < name_a.size(); x++) name_a.at(x) = tolower(name_a.at(x));
	for (unsigned int x = 0; x < name_b.size(); x++) name_b.at(x) = tolower(name_b.at(x));
	if (name_a < name_b) return true;
	else return false;
}

/* This function fetches the required values (Name, Exec, Categories, NoDisplay etc)
 * and then assigns the results to the appropriate instance variables or passes the results
 * to the appropriate function */
void DesktopFile::populate(vector<string> showFromDesktops, bool useIcons, vector<IconSpec>& iconpaths, vector<Category>& cats)
{	string line;
	string iconDef;
	vector<string> onlyShowInDesktops;
	vector<string> foundCategories;
	bool started = false;

	while (!dfile.eof())
	{	getline(dfile, line);
		if (line.size() == 0) continue;
		string id = getID(line);
		/* .desktop files can contain more than just desktop entries. On getting
		 * the id [Desktop Entry] we know we've started looking at an entry */
		if (id == "[Desktop Entry]")
		{	started = true;
			continue;
		}
		/* If we get another line beginning with [, it probably means we've found
		 * a desktop action as opposed to a desktop entry. We should break here 
		 * to avoid the entry data being overwritten with action data */
		if (id[0] == '[' && started == true) break;
		if (id == "Name")
		{	name = getSingleValue(line);
			continue;
		}
		if (id == "Exec")
		{	exec = getSingleValue(line);
			continue;
		}
		if (id == "Categories")
		{	foundCategories = getMultiValue(line);
			continue;
		}
		if (id == "NoDisplay")
		{	string value = getSingleValue(line);
			if (value == "True" || value == "true")
				nodisplay = true;
			continue;
		}
		if (id == "OnlyShowIn")
		{	onlyShowInDesktops = getMultiValue(line);
			continue;
		}
		if (id == "Icon")
		{	iconDef = getSingleValue(line);
			continue;
		}
	}

	processCategories(cats, foundCategories);
	if (useIcons && iconDef != "\0") matchIcon(iconDef, iconpaths);
	if (!onlyShowInDesktops.empty()) processDesktops(showFromDesktops, onlyShowInDesktops);
}

/* This function is used to get the single value before the = sign.
 * Should be Name, Exec, Categories etc */
string DesktopFile::getID(string line)
{	vector<char> readChars;
	readChars.reserve(10);
	char c;
	unsigned int counter = 0;

	while (counter < line.size())
	{	c = line[counter];
		if (c == '=') break;
		readChars.push_back(c);
		counter++;
	}

	return string(readChars.begin(), readChars.end());
}

/* This function is used to get single values, for example: for the line 
 * Name=Firefox this function will return Firefox */
string DesktopFile::getSingleValue(string line)
{	vector<char> readChars;
	readChars.reserve(10);
	string value;
	char c;
	bool startFilling = false;
	unsigned int counter = 0;

	while (counter < line.size())
	{	c = line[counter];
		if (startFilling) readChars.push_back(c);
		if (c == '=') startFilling = true;
		counter++;
	}

	//Some names include a trailing space. For matching, it's best if we remove these
	if (readChars[readChars.size() - 1] == ' ') readChars.erase(readChars.end() - 1);
	value = string(readChars.begin(), readChars.end());
	//Throw away field codes like %F, most WMs don't appear to handle these
	string::iterator fieldCode = find(value.begin(), value.end(), '%');
	if (fieldCode != value.end()) value.erase(fieldCode - 1, value.end());

	return value;
}

/* This function is used to get multiple semi-colon seperated values.
 * For instance, the line Categories=System;Settings; will return a string
 * vector with the items System and Settings */
vector<string> DesktopFile::getMultiValue(string line)
{	vector<string> values;
	vector<char> readChars;
	values.reserve(5);
	readChars.reserve(10);
	char c;
	bool startFilling = false;
	unsigned int counter = 0;

	while (counter < line.size())
	{	c = line[counter];
		if (startFilling && c != ';') readChars.push_back(c);
		if (c == '=') startFilling = true;
		if (c == ';')
		{	values.push_back(string(readChars.begin(), readChars.end()));
			readChars.clear();
		}
		counter++;
	}

	//Ensure we get the last category in line if it isn't terminated with a
	//semi-colon
	if (!readChars.empty())
		values.push_back(string(readChars.begin(), readChars.end()));

	return values;
}

/* Add the desktop entry to the appropriate categories, based on what was read from the
 * file. If we can't find a category, add the entry to the Other category which is the catchall */
void DesktopFile::processCategories(vector<Category>& cats, vector<string> foundCategories)
{	bool hasCategory = false;
	vector<string>::iterator it = foundCategories.begin();

	//Convert some base categories to more commonly used categories
	while (it < foundCategories.end())
	{	if (*it == "AudioVideo" || *it == "Audio" || *it == "Video") *it = "Multimedia";
		if (*it == "Network") *it = "Internet";
		if (*it == "Utility") *it = "Accessories";
		it++;
	}

	//Loop through our category objects, adding the desktop entry name to the category if appropriate
	for (unsigned int x = 0; x < cats.size(); x++)
	{	//Add to category if foundCategories contains the category name
		if (find(foundCategories.begin(), foundCategories.end(), cats[x].name) != foundCategories.end()) 
		{	cats[x].incEntries.push_back(name);
			hasCategory = true;
			continue;
		}
		//Add to category if the category specifies a particular desktop file by filename
		string baseFilename = filename.substr(filename.find_last_of("/") + 1, filename.size() - filename.find_last_of("/") - 1);
		if (find(cats[x].incEntryFiles.begin(), cats[x].incEntryFiles.end(), baseFilename) != cats[x].incEntryFiles.end())
		{	cats[x].incEntries.push_back(name);
			hasCategory = true;
		}
	}

	//If an entry ends up with no categories, give the entry the catchall category
	if (!hasCategory)
	{	for (unsigned int x = 0; x < cats.size(); x++)
		{	if (cats[x].name == "Other")
			{	cats[x].incEntries.push_back(name);
				break;
			}
		}
	}
}

/* Function which attempts to find the full path for a desktop entry by going
 * through a list of icons, attempting to match the icon entry in the entry
 * against each icon path */
void DesktopFile::matchIcon(string iconDef, vector<IconSpec>& iconpaths)
{	//This is a kludge. If the iconDef is a path then just use that and return
	if (iconDef.find("/") != string::npos)
	{	icon = iconDef;
		return;
	}
	/* Here we search through the icon locations provided, trying to match the definition to a
	 * full path. Note that the first matching icon found will be the one that is chosen */
	for (unsigned int x = 0; x < iconpaths.size(); x++)
	{	if (iconDef == iconpaths[x].def || iconDef == iconpaths[x].id)
		{	icon = iconpaths[x].path;
			break;
		}
	}
}

/* This function handles desktop entries that specify they should only be displayed in certain
 * desktops. If the user specifies that OnlyShowIn entries from a matching desktop should be displayed
 * then the function will return and the nodisplay value will be left untouched. If not, it will be set
 * to true */
void DesktopFile::processDesktops(vector<string> showInDesktops, vector<string> onlyShowInDesktops)
{	//First check for all or none
	for (unsigned int x = 0; x < showInDesktops.size(); x++)
	{	if (showInDesktops[x] == "all") return;
		if (showInDesktops[x] == "none" && !onlyShowInDesktops.empty())
		{	nodisplay = true;
			return;
		}
	}
	//Now loop through the entry OnlyShowIn desktops
	for (unsigned int x = 0; x < onlyShowInDesktops.size(); x++)
	{	if (find(showInDesktops.begin(), showInDesktops.end(), onlyShowInDesktops[x]) != showInDesktops.end())
			return;
	}
	nodisplay = true;
}
