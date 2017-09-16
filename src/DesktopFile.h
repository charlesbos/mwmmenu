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

#ifndef _DESKTOP_FILE_H_
#define _DESKTOP_FILE_H_

#include <string>
#include <fstream>
#include <vector>

using namespace std;

struct IconSpec
{	string path;
	string def;
	string id;
};

class Category;

class DesktopFile
{
	public:
		DesktopFile();
		DesktopFile(const char *filename, vector<string> showFromDesktops, bool useIcons, const vector<IconSpec>& iconpaths, vector<Category>& cats);
		DesktopFile(const DesktopFile& df);

		DesktopFile& operator=(const DesktopFile& df);
		bool operator<(const DesktopFile& df);

		string filename;
		string name;
		string exec;
		bool nodisplay;
		string icon;

		static string getID(const string& line);
		static string getSingleValue(const string& line);
		static vector<string> getMultiValue(const string& line);
 
	private:
		ifstream dfile;

		void populate(const vector<string>& showFromDesktops, bool useIcons, const vector<IconSpec>& iconpaths, vector<Category>& cats);
		void matchIcon(const string& iconDef, const vector<IconSpec>& iconpaths);
		void processCategories(vector<Category>& cats, vector<string>& foundCategories);
		void processDesktops(const vector<string>& showFromDesktops, const vector<string>& onlyShowInDesktops);
};

#endif
