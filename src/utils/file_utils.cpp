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

#include <pangolin/utils/file_utils.h>

#ifdef _WIN_
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <Windows.h>
#  include <Shlobj.h>
#  ifdef UNICODE
#    include <codecvt>
#  endif
#else
#  include <dirent.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <sys/signal.h>
#  include <stdio.h>
#  include <string.h>
#  include <unistd.h>
#  include <fcntl.h>
#  include <errno.h>
#  include <poll.h>
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

// Make path seperator consistent for OS.
void PathOsNormaliseInplace(std::string& path)
{
#ifdef _WIN_
    std::replace(path.begin(), path.end(), '/', '\\');
#else
    std::replace(path.begin(), path.end(), '\\', '/');
#endif
}

std::string SanitizePath(const std::string& path)
{
    std::string path_copy(path.length(), '\0');

    int p_slash1 = -1;
    int p_slash0 = -1;
    int n_dots = 0;

    int dst = 0;
    for(int src=0; src < (int)path.length(); ++src) {
        if(path[src] == '/') {
            if(n_dots==1 && p_slash0 >=0) {
                dst = p_slash0;
                for(p_slash1=p_slash0-1; p_slash1>=0 && path_copy[p_slash1] != '/'; --p_slash1);
            }else if(n_dots==2) {
                if(p_slash1 >=0) {
                    dst = p_slash1;
                    p_slash0 = dst;
                    for(p_slash1=p_slash0-1; p_slash1>=0 && path_copy[p_slash1] != '/'; --p_slash1) {
                        if( path_copy[p_slash1] == '.' ) {
                            p_slash1=-1;
                            break;
                        }
                    }
                }else{
                    p_slash1 = -1;
                    p_slash0 = dst;
                }
            }else{
                p_slash1 = p_slash0;
                p_slash0 = dst;
            }
            n_dots = 0;
        }else if(path[src] == '.' ){
            if((dst-p_slash0) == n_dots+1) {
                ++n_dots;
            }
        }else{
            n_dots = 0;
        }
        path_copy[dst++] = path[src];
    }

    return path_copy.substr(0,dst);
}

// Return path 'levels' directories above 'path'
std::string PathParent(const std::string& path, int levels)
{
    std::string res = path;

    while (levels > 0) {
        if (res.length() == 0) {
            res = std::string();
            for (int l = 0; l < levels; ++l) {
#ifdef _WIN_
                res += std::string("..\\");
#else
                res += std::string("../");
#endif
            }
            return res;
        }else{
            const size_t nLastSlash = res.find_last_of("/\\");

            if (nLastSlash != std::string::npos) {
                res = path.substr(0, nLastSlash);
            } else{
                res = std::string();
            }

            --levels;
        }
    }

    return res;
}

std::string FindPath(const std::string& child_path, const std::string& signature_path)
{
    std::string path = PathExpand(child_path);
#ifdef _UNIX_
    char abs_path[PATH_MAX];
    if (realpath(path.c_str(), abs_path)) {
        path = abs_path;
    }
#endif
    std::string signature = signature_path;
    PathOsNormaliseInplace(path);
    PathOsNormaliseInplace(signature);

    while(!FileExists(path + signature)) {
        if (path.empty()) {
            return std::string();
        } else {
            path = PathParent(path);
        }
    }

    return path + signature;
}

std::string PathExpand(const std::string& sPath)
{
    if(sPath.length() >0 && sPath[0] == '~') {
#ifdef _WIN_
        std::string sHomeDir;
        WCHAR path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, path))) {
            std::wstring ws(path);
            sHomeDir = std::string(ws.begin(), ws.end());
        }
#else
        std::string sHomeDir = std::string(getenv("HOME"));
#endif
        return sHomeDir + sPath.substr(1,std::string::npos);
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

std::string MakeUniqueFilename(const std::string& filename)
{
    if( FileExists(filename) ) {
        const size_t dot = filename.find_last_of('.');

        std::string fn;
        std::string ext;

        if(dot == filename.npos) {
            fn = filename;
            ext = "";
        }else{
            fn = filename.substr(0, dot);
            ext = filename.substr(dot);
        }

        int id = 1;
        std::string new_file;
        do {
            id++;
            std::stringstream ss;
            ss << fn << "_" << id << ext;
            new_file = ss.str();
        }while( FileExists(new_file) );

        return new_file;
    }else{
        return filename;
    }
}

bool IsPipe(const std::string& file)
{
#ifdef _WIN_
    return false;
#else
    struct stat st;
    int err = stat(file.c_str(), &st);
    return (err == 0) && ((st.st_mode & S_IFMT) == S_IFIFO);
#endif // _WIN_
}

bool IsPipe(int fd)
{
#ifdef _WIN_
    return false;
#else
    struct stat st;
    int err = fstat(fd, &st);
    return (err == 0) && ((st.st_mode & S_IFMT) == S_IFIFO);
#endif
}

int WritablePipeFileDescriptor(const std::string& file)
{
#ifdef _WIN_
    return false;
#else
    // open(2) will return ENXIO when there is no reader on the other
    // side of the pipe, but only if O_WRONLY|O_NONBLOCK is specified.
    // The file descriptor can be adjusted to be a blocking file
    // descriptor if it is valid.
    return open(file.c_str(), O_WRONLY | O_NONBLOCK);
#endif // _WIN_
}

int ReadablePipeFileDescriptor(const std::string& file)
{
#ifdef _WIN_
    return -1;
#else
    return open(file.c_str(), O_RDONLY | O_NONBLOCK);
#endif
}

bool PipeHasDataToRead(int fd)
{
#ifdef _WIN_
    return false;
#else
    struct pollfd pfd;
    memset(&pfd, 0, sizeof(pfd));
    pfd.fd = fd;
    pfd.events = POLLIN;

    int err = poll(&pfd, 1, 0);

    // If err is 0, the file has no data. If err is negative, an error
    // occurred.
    return err == 1 && pfd.revents & POLLIN;
#endif
}

void FlushPipe(const std::string& file)
{
#ifndef _WIN_
    int fd = open(file.c_str(), O_RDONLY | O_NONBLOCK);
    char buf[65535];
    int n = 0;
    do
    {
        n = read(fd, buf, sizeof(buf));
    } while(n > 0);
    close(fd);
#endif
}

#ifdef _WIN_

#ifdef UNICODE
    typedef std::codecvt_utf8<wchar_t> WinStringConvert;
    typedef std::wstring WinString;

    WinString s2ws(const std::string& str) {
        std::wstring_convert<WinStringConvert, wchar_t> converter;
        return converter.from_bytes(str);
    }
    std::string ws2s(const WinString& wstr) {
        std::wstring_convert<WinStringConvert, wchar_t> converter;
        return converter.to_bytes(wstr);
    }
#else
    typedef std::string WinString;
    // No conversions necessary
#   define s2ws(X) (X)
#   define ws2s(X) (X)
#endif // UNICODE

bool FilesMatchingWildcard(const std::string& wildcard, std::vector<std::string>& file_vec)
{
	size_t nLastSlash = wildcard.find_last_of("/\\");
    
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
    
    WIN32_FIND_DATA wfd;
    HANDLE fh = FindFirstFile( s2ws(sPath + "\\" + sFileWc).c_str(), &wfd);

    std::vector<std::string> files;

    if (fh != INVALID_HANDLE_VALUE) {
        do {
            if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))  {
                files.push_back( sPath + "\\" + ws2s(wfd.cFileName) );
            }
        } while (FindNextFile (fh, &wfd));
        FindClose(fh);
    }

    // TOOD: Use different comparison to achieve better ordering.
    std::sort(files.begin(), files.end() );

    // Put file list at end of file_vec
    file_vec.insert(file_vec.end(), files.begin(), files.end() );

    return files.size() > 0;
}

bool FileExists(const std::string& filename)
{
    std::string search_filename = filename;
    if(filename.length() > 0 && filename[filename.length()-1] == '\\') {
        search_filename.resize(filename.length()-1);
    }
    WIN32_FIND_DATA wfd;
    HANDLE fh = FindFirstFile( s2ws(search_filename).c_str(), &wfd);
    const bool exists = fh != INVALID_HANDLE_VALUE;
    FindClose(fh);
    return exists;
}

#else // _WIN_

bool FilesMatchingWildcard(const std::string& in_wildcard, std::vector<std::string>& file_vec)
{
    const std::string wildcard = PathExpand(in_wildcard);
    const size_t first_wildcard = wildcard.find_first_of("?*");
    if(first_wildcard != std::string::npos) {
        const std::string root = PathParent(wildcard.substr(0,first_wildcard));
        struct dirent **namelist;
        int n = scandir(root.c_str(), &namelist, 0, alphasort );
        if (n >= 0) {
            const size_t next_slash = wildcard.find_first_of("/\\",first_wildcard+1);
            std::string dir_wildcard, rest;
            if(next_slash != std::string::npos) {
                dir_wildcard = wildcard.substr(root.size()+1, next_slash-root.size()-1);
                rest = wildcard.substr(next_slash);
            }else{
                dir_wildcard = wildcard.substr(root.size()+1);
            }

            while(n--) {
                const std::string file_name(namelist[n]->d_name);
                if( file_name != "." && file_name != ".." && MatchesWildcard(file_name, dir_wildcard) ) {
                    const std::string sub_wildcard = root + "/" + file_name + rest;
                    FilesMatchingWildcard(sub_wildcard, file_vec);
                    if(dir_wildcard == "**") {
                        const std::string sub_wildcard2 = root + "/" + file_name + "/**" + rest;
                        FilesMatchingWildcard(sub_wildcard2, file_vec);
                    }
                }
            }
        }
    }else if(FileExists(wildcard)) {
        file_vec.push_back(wildcard);
    }
    return file_vec.size() > 0;
}

bool FileExists(const std::string& filename)
{
    struct stat buf;
    return stat(filename.c_str(), &buf) != -1;
}

#endif //_WIN_

}
