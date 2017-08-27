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
 * along with this program.	If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "boost/filesystem.hpp"
#include "DesktopFile.h"
#include "MenuWriter.h"
#include "Category.h"

void usage()
{	cout << "mwmmenu - creates application menus for MWM and other window managers.\n\n"
		"Usage:\n"
		"	# Note: all options must be spaced.\n"
		"	mwmmenu [OPTIONS]\n"
		"Options:\n"
		"	-h, -help: show this dialogue\n\n"
		"	-n, -name: name used for the main menu - by default, use Applications\n\n"
		"	-i, -icons: use icons with menu entries, only compatible with some window managers\n\n"
		"	-icons_xdg_only: Exclude any non-xdg icons. Note that this will disable the -add_icon_paths option.\n\n"
		"	-xdg_icons_size: can be 16x16, 32x32 etc. Can also be scalable or all. Note that using all might significantly slow down the program. Also note that this cannot control sizes for non-xdg icons. Defaults to 16x16.\n\n"
		"	-no_custom_categories: do not add entries to or print non-standard categories, 'Other' will be used instead if required.\n\n"
		"	# Note:\n"
		"	* The following options accept a single string which can contain multiple parameters.\n"
		"	* Multiple parameters should be separated by commas: e.g. -option Param1,Param2,Param3\n"
		"	* If the option asks for a name, assume it means a name as specified in an entry file, e.g. Name=Firefox.\n"
		"	* If a parameter contains spaces, that parameter must be enclosed in quotes.\n\n"
		"	-exclude: do not add desktop entries that have the names specified.\n\n"
		"	-exclude_matching: do not add desktop entries where the entry's name contains one of the strings specified.\n\n"
		"	-exclude_categories: do not print category menus for the following category names.\n\n"
		"	-exclude_by_filename: exclude desktop entries based on their full paths.\n\n"
		"	-include: force entries with the following names to be included in menus even if their no display value is true.\n\n"
		"	-show_from_desktops: show entries from the specified desktops if OnlyShowIn is set. Can be values like GNOME or XFCE. Can also be none or all, The default is none.\n\n"
		"	-add_desktop_paths: add extra search paths for desktop entries.\n\n"
		"	-add_icon_paths: add extra search paths for icons.\n\n"
		"Menu format options:\n"
		"	# No format argument: produce menus for MWM or TWM\n"
		"	-fvwm: produce menus for FVWM\n"
		"	-fvwm_dynamic: produce dynamic menus for FVWM\n"
		"	-fluxbox: produce menus for Fluxbox or Blackbox\n"
		"	-openbox: produce menus for Openbox\n"
		"	-openbox_pipe: produce pipe menus for Openbox\n"
		"	-olvwm: produce menus for Olvwm\n"
		"	-windowmaker: produce menus for Windowmaker\n"
		"	-icewm: produce menus for IceWM\n";
}

//Function that attempts to get the user icon theme from ~/.gtkrc-2.0
string getIconTheme(string homedir)
{	ifstream themefile;
	string path = homedir + "/.gtkrc-2.0";
	string id = "gtk-icon-theme-name";
	themefile.open(path.c_str());
	if (!themefile) return "\0";
	else
	{	string line;
		char c;
		unsigned int counter = 0;
		while (!themefile.eof())
		{	getline(themefile, line);
			vector<char> readChars;
			readChars.reserve(10);
			while (counter < line.size())
			{	c = line[counter];
				if (c == '=') break;
				readChars.push_back(c);
				counter++;
			}
			string read_id = string(readChars.begin(), readChars.end());
			read_id.erase(remove(read_id.begin(), read_id.end(), ' '), read_id.end());
			if (id == read_id)
			{	vector<char> themeNameChars;
				themeNameChars.reserve(10);
				while (counter < line.size())
				{	c = line[counter];
					if (c != '"' && c != '=') themeNameChars.push_back(c);
					counter ++;
				}
				themefile.close();
				string themename = string(themeNameChars.begin(), themeNameChars.end());
				themename.erase(remove(themename.begin(), themename.end(), ' '), themename.end());
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
vector<string> splitCommaArgs(string arg)
{	vector<string> splitArgs;
	vector<char> buffer;
	splitArgs.reserve(5);
	buffer.reserve(10);

	for (unsigned int x = 0; x < arg.size(); x++)
	{	if (arg[x] == ',') 
		{	splitArgs.push_back(string(buffer.begin(), buffer.end()));
			buffer.clear();
			continue;
		}
		buffer.push_back(arg[x]);
	}
	if (!buffer.empty()) splitArgs.push_back(string(buffer.begin(), buffer.end()));

	return splitArgs;
}

//A function to make sure we only add unique categories to the categories list
void addCategory(Category &c, vector<Category> &categories)
{	for (unsigned int x = 0; x < categories.size(); x++) {
		if (categories[x].name == c.name) return;
	}
	categories.push_back(c);
}

int main(int argc, char *argv[])
{	//Handle args
	string homedir = getenv("HOME");
	string menuName = "Applications";
	int windowmanager = mwm;
	bool useIcons = false;
	bool iconsXdgOnly = false;
	string xdgIconsSize = "16x16";
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
	{	if (strcmp(argv[x], "-h") == 0 || strcmp(argv[x], "-help") == 0)
		{	usage();
			return 0; 
		}
		if (strcmp(argv[x], "-n") == 0 || strcmp(argv[x], "-name") == 0) 
		{	if (x + 1 < argc) menuName = argv[x + 1];
			continue;
		}
		if (strcmp(argv[x], "-i") == 0 || strcmp(argv[x], "-icons") == 0)
		{	useIcons = true;
			continue;
		}
		if (strcmp(argv[x], "-icons_xdg_only") == 0) 
		{	iconsXdgOnly = true;
			continue;
		}
		if (strcmp(argv[x], "-xdg_icons_size") == 0) 
		{	if (x + 1 < argc) xdgIconsSize = argv[x + 1] ;
			continue;
		}
		if (strcmp(argv[x], "-fvwm") == 0) 
		{	windowmanager = fvwm;
			continue;
		}
		if (strcmp(argv[x], "-fvwm_dynamic") == 0) 
		{	windowmanager = fvwm_dynamic;
			continue;
		}
		if (strcmp(argv[x], "-fluxbox") == 0) 
		{	windowmanager = fluxbox;
			continue;
		}
		if (strcmp(argv[x], "-openbox") == 0) 
		{	windowmanager = openbox;
			continue;
		}
		if (strcmp(argv[x], "-openbox_pipe") == 0) 
		{	windowmanager = openbox_pipe;
			continue;
		}
		if (strcmp(argv[x], "-olvwm") == 0) 
		{	windowmanager = olvwm;
			continue;
		}
		if (strcmp(argv[x], "-windowmaker") == 0) 
		{	windowmanager = windowmaker;
			continue;
		}
		if (strcmp(argv[x], "-icewm") == 0) 
		{	windowmanager = icewm;
			continue;
		}
		if (strcmp(argv[x], "-exclude") == 0) 
		{	if (x + 1 < argc) exclude = argv[x + 1];
			continue;
		}
		if (strcmp(argv[x], "-exclude_matching") == 0) 
		{	if (x + 1 < argc) excludeMatching = argv[x + 1];
			continue;
		}
		if (strcmp(argv[x], "-exclude_categories") == 0) 
		{	if (x + 1 < argc) excludeCategories = argv[x + 1];
			continue;
		}
		if (strcmp(argv[x], "-exclude_by_filename") == 0)
		{	if (x + 1 < argc) excludedFilenames = argv[x + 1];
			continue;
		}
		if (strcmp(argv[x], "-include") == 0)
		{	if (x + 1 < argc) include = argv[x + 1];
			continue;
		}
		if (strcmp(argv[x], "-show_from_desktops") == 0)
		{	if (x + 1 < argc) showFromDesktops = argv[x + 1];
			continue;
		}
		if (strcmp(argv[x], "-add_desktop_paths") == 0) 
		{	if (x + 1 < argc) extraDesktopPaths = argv[x + 1];
			continue;
		}
		if (strcmp(argv[x], "-add_icon_paths") == 0) 
		{	if (x + 1 < argc) extraIconPaths = argv[x + 1];
			continue;
		}
		if (strcmp(argv[x], "-no_custom_categories") == 0)
		{	noCustomCats = true;
			continue;
		}
	}
	if (windowmanager == mwm || 
			windowmanager == olvwm ||
			windowmanager == windowmaker) 
		useIcons = false;
	if (xdgIconsSize == "all") xdgIconsSize = "/";

	//Get string vector of paths to .desktop files
	vector<string> paths;
	paths.reserve(300);
	vector<string> appdirs = {"/usr/share/applications/", "/usr/local/share/applications/"};
	if (homedir.c_str() != NULL) appdirs.push_back(homedir + "/.local/share/applications/");
	if (extraDesktopPaths != "\0")
	{	vector<string> newDPaths = splitCommaArgs(extraDesktopPaths);
		for (unsigned int x = 0; x < newDPaths.size(); x++)
			appdirs.push_back(newDPaths[x]);
	}
	for (unsigned int x = 0; x < appdirs.size(); x++)
	{	try
		{	for (boost::filesystem::recursive_directory_iterator i(appdirs[x]), end; i != end; ++i)
				if (!is_directory(i->path())) paths.push_back(i->path().string());
		}
		catch (boost::filesystem::filesystem_error) { continue; }
	}

	//Get string vector of paths to icons
	vector<string> iconpaths;
	if (useIcons)
	{	iconpaths.reserve(500);
		vector<string> icondirs = {"/usr/share/icons/hicolor"};
		if (!iconsXdgOnly) 
		{	icondirs.push_back("/usr/share/pixmaps");
			icondirs.push_back("/usr/local/share/pixmaps");
		}
		if (homedir.c_str() != NULL)
		{	string themename = getIconTheme(homedir); 
			icondirs.push_back("/usr/share/icons/" + themename);
			if (find(icondirs.begin(), icondirs.end(), "/usr/share/icons/gnome") != icondirs.end()) icondirs.push_back("/usr/share/icons/gnome");
			icondirs.push_back(homedir + "/.local/share/icons");
		}
		if (extraIconPaths != "\0" && !iconsXdgOnly)
		{	vector<string> newIPaths = splitCommaArgs(extraIconPaths);
			for (unsigned int x = 0; x < newIPaths.size(); x++)
				icondirs.push_back(newIPaths[x]);
		}
		for (unsigned int x = 0; x < icondirs.size(); x++)
		{	try
			{	for (boost::filesystem::recursive_directory_iterator i(icondirs[x]), end; i != end; ++i)
				{	if (!is_directory(i->path()))
					{	string ipath = i->path().string();
						//If icon directory is a XDG one, only add iconpath if conforms to the chosen size
						if (ipath.find("/usr/share/icons") != string::npos || ipath.find(".local/share/icons") != string::npos)
						{	if (ipath.find(xdgIconsSize) == string::npos) continue;
						}
						iconpaths.push_back(i->path().string());
					}
				}
			}
			catch (boost::filesystem::filesystem_error) { continue; }
		}
	}

	/* Create categories
	 * Note that for baseCategories we combine Audio, Video and AudioVideo into Multimedia. We also rename Network to Internet and
	 * Utility to Accessories as this is what is commonly done elsewhere. Otherwise, our categories are the same as the freedesktop.org
	 * base categories */
	vector<string> baseCategories = {"Accessories", "Development", "Education", "Game", "Graphics", "Multimedia", "Internet",
									"Office", "Other", "Science", "Settings", "System"};
	vector<string> catPaths;
	vector<string> menuPaths;
	if (!noCustomCats)
	{	vector<string> catDirs = {"/usr/share/desktop-directories"};
		vector<string> menuDirs = {"/etc/xdg/menus"}; 
		if (homedir.c_str() != NULL) 
		{	catDirs.push_back(homedir + "/.local/share/desktop-directories");
			menuDirs.push_back(homedir + "/.config/menus/applications-merged");
		}
		for (unsigned int x = 0; x < catDirs.size(); x++)
		{	try
			{	for (boost::filesystem::recursive_directory_iterator i(catDirs[x]), end; i != end; ++i)
					if (!is_directory(i->path())) catPaths.push_back(i->path().string());
			}
			catch (boost::filesystem::filesystem_error) { continue; }
		}
		for (unsigned int x = 0; x < menuDirs.size(); x++)
		{	try
			{	for (boost::filesystem::recursive_directory_iterator i(menuDirs[x]), end; i != end; ++i)
					if (!is_directory(i->path())) menuPaths.push_back(i->path().string());
			}
			catch (boost::filesystem::filesystem_error) { continue; }
		}
	}
	vector<Category> cats;
	//Create the base categories
	for (unsigned int x = 0; x < baseCategories.size(); x++)
	{	Category c = Category(baseCategories[x], useIcons, iconpaths);
		cats.push_back(c);
	}
	//Create the custom categories (if there are any)
	for (unsigned int x = 0; x < catPaths.size(); x++)
	{	Category c = Category(catPaths[x].c_str(), menuPaths, useIcons, iconpaths);
		if (c.name != "\0") addCategory(c, cats);
	}
	sort(cats.begin(), cats.end());

	//Create vector of DesktopFile, using each path in the paths vector
	vector<DesktopFile> files;
	for (vector<string>::iterator it = paths.begin(); it < paths.end(); it++)
	{	DesktopFile df = DesktopFile((*it).c_str(), splitCommaArgs(showFromDesktops), useIcons, iconpaths, cats);
		if (df.name != "\0" && df.exec != "\0") files.push_back(df);
	}
	sort(files.begin(), files.end());

	//Create a MenuWriter which will write the menu out to the console
	MenuWriter(files, 
		menuName, 
		windowmanager, 
		useIcons, 
		splitCommaArgs(exclude), 
		splitCommaArgs(excludeMatching), 
		splitCommaArgs(excludeCategories),
		splitCommaArgs(include),
		splitCommaArgs(excludedFilenames),
		cats);

	return 0;
}

