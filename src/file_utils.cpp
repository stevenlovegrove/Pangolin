/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2013 Steven Lovegrove
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <pangolin/platform.h>
#include <pangolin/file_utils.h>

#ifndef _WIN_
#include <dirent.h>
#include <sys/stat.h>
#endif // _WIN_

#include <algorithm>
#include <sstream>
#include <list>

namespace pangolin
{

std::vector<std::string>& Split(const std::string& s, char delim, std::vector<std::string>& elements) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elements.push_back(item);
    }
    return elements;
}

std::vector<std::string> Split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    return Split(s, delim, elems);
}

std::vector<std::string> Expand(const std::string &s, char open, char close, char delim)
{
    const size_t no = s.find_first_of(open);
    if(no != std::string::npos) {
        const size_t nc = s.find_first_of(close, no);
        if(no != std::string::npos) {
            const std::string pre  = s.substr(0, no);
            const std::string mid  = s.substr(no+1, nc-no-1);
            const std::string post = s.substr(nc+1, std::string::npos);
            const std::vector<std::string> options = Split(mid, delim);
            std::vector<std::string> expansion;
            
            for(std::vector<std::string>::const_iterator iop = options.begin(); iop != options.end(); ++iop)
            {
                std::string full = pre + *iop + post;
                expansion.push_back(full);
            }
            return expansion;
        }
        // Open but no close is unusual. Leave it for caller to see if valid
    }
    
    std::vector<std::string> ret;
    ret.push_back(s);
    return ret;
}

std::string PathParent(const std::string& path)
{
    const size_t nLastSlash = path.find_last_of('/');
    
    if(nLastSlash != std::string::npos) {
        return path.substr(0, nLastSlash);
    }else{
        return std::string();
    }
}

bool FileExists(const std::string& filename)
{
#ifndef _WIN_
    struct stat buf;
    return stat(filename.c_str(), &buf) != -1;
#else
	throw std::runtime_error("Not implemented");
#endif //_WIN_
}

std::string PathExpand(const std::string& sPath)
{
    if(sPath.length() >0 && sPath[0] == '~') {
        const char* sHomeDir = getenv("HOME");
        return std::string(sHomeDir) + sPath.substr(1,std::string::npos);
    }else{
        return sPath;
    }
}

// Based on http://www.codeproject.com/Articles/188256/A-Simple-Wildcard-Matching-Function
bool MatchesWildcard(const std::string& str, const std::string& wildcard)
{
    const char* psQuery = str.c_str();
    const char* psWildcard = wildcard.c_str();
    
    while(*psWildcard)
    {
        if(*psWildcard=='?')
        {
            if(!*psQuery)
                return false;

            ++psQuery;
            ++psWildcard;
        }
        else if(*psWildcard=='*')
        {
            if(MatchesWildcard(psQuery,psWildcard+1))
                return true;

            if(*psQuery && MatchesWildcard(psQuery+1,psWildcard))
                return true;

            return false;
        }
        else
        {
            if(*psQuery++ != *psWildcard++ )
                return false;
        }
    }

    return !*psQuery && !*psWildcard;
}

bool FilesMatchingWildcard(const std::string& wildcard, std::vector<std::string>& file_vec)
{
#ifndef _WIN_
	size_t nLastSlash = wildcard.find_last_of('/');
    
    std::string sPath;
    std::string sFileWc;
    
    if(nLastSlash != std::string::npos) {
        sPath =   wildcard.substr(0, nLastSlash);                  
        sFileWc = wildcard.substr(nLastSlash+1, std::string::npos);
    }else{
        sPath = ".";
        sFileWc = wildcard;
    }
    
    sPath = PathExpand(sPath);
        
    struct dirent **namelist;
    int n = scandir(sPath.c_str(), &namelist, 0, alphasort ); // sort alpha-numeric
//    int n = scandir(sPath.c_str(), &namelist, 0, versionsort ); // sort aa1 < aa10 < aa100 etc.
    if (n >= 0){
        std::list<std::string> file_list;
        while( n-- ){
            const std::string sName(namelist[n]->d_name);
            if( sName != "." && sName != ".." && MatchesWildcard(sName, sFileWc) ) {
                const std::string sFullName = sPath + "/" + sName;
                file_list.push_front( sFullName );
            }
            free(namelist[n]);
        }
        free(namelist);
        
        file_vec.reserve(file_list.size());
        file_vec.insert(file_vec.begin(), file_list.begin(), file_list.end());
        return file_vec.size() > 0;
    }
    return false;
#else
	throw std::runtime_error("Not implemented");
#endif //_WIN_
}

}
