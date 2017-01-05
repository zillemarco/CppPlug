#include "Path.h"

#include <iterator>

#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#include <windows.h>

const char* Path::s_separator = "\\";
#else
#include <fstream>

const char* Path::s_separator = "/";
#endif

#define FILE_DIR		0
#define FILE_FILE		1
#define FILE_SYM_LINK	2

static bool GetInfo(const std::string& path, struct stat& info)
{
	if (path.empty())
		return false;

	return stat(path.c_str(), &info) == 0;
}

static int GetType(const std::string& path)
{
	struct stat fileInfo;

	if (!GetInfo(path, fileInfo))
		return -1;

	if (fileInfo.st_mode & S_IFDIR)
		return FILE_DIR;
	else if (fileInfo.st_mode & S_IFREG)
		return FILE_FILE;
#ifndef WIN32
	else if (fileInfo.st_mode & S_IFLNK)
		return FILE_SYM_LINK;
#endif

	return -1;
}

Path::Path(const std::string& path)
	: _path(path)
{ }

std::string Path::GetCWD()
{
	char cwd[8192];
	memset(cwd, 0, 8192);

#ifdef WIN32
	::GetCurrentDirectoryA(8192, cwd);
#else
	char * res = ::getcwd(cwd, 8192);
#endif

	return std::string(cwd);
}

bool Path::SetCWD(const std::string& path)
{
	int res = 0;

#ifdef WIN32
	res = ::SetCurrentDirectoryA(path.c_str()) ? 0 : -1;
#else
	res = ::chdir(path.c_str());
#endif

	return res == 0;
}

bool Path::Exists(const std::string& path)
{
	if (path.empty())
		return false;

	struct stat fileInfo;
	return GetInfo(path, fileInfo);
}

bool Path::IsFile(const std::string& path)
{
	return GetType(path) == FILE_FILE;
}

bool Path::IsDirectory(const std::string& path)
{
	return GetType(path) == FILE_DIR;
}

bool Path::IsSymbolicLink(const std::string& path)
{
	return GetType(path) == FILE_SYM_LINK;
}

bool Path::IsAbsolute(const std::string& path)
{
	if (path.empty())
		return false;

#ifdef WIN32
	if (path.size() < 2)
		return false;
	else
		return path[1] == ':';
#else
	return path[0] == '/';
#endif
}

std::string Path::GetParent(const std::string& path)
{
	Path::StringVector sv;
	Path::Split(path, sv);

	std::string root;
	
	if (path[0] == '/')
		root = "/";
	
	return root + Path::Join(sv.begin(), sv.end() - 1);
}

std::string Path::GetBasename(const std::string& path)
{
	std::string::size_type index = path.find_last_of(Path::s_separator);

	if (index == std::string::npos)
		return path;

	return std::string(path.c_str() + index + 1, index);
}

std::string Path::GetExtension(const std::string& path)
{
	std::string filename = Path::GetBasename(path);
	std::string::size_type index = filename.find_last_of('.');

	// If it has a regular or hidden filename with no extension return an empty string
	if (index == std::string::npos ||	// regular filename with no ext 
		index == 0 ||					// hidden file (starts with a '.')
		index == path.size() - 1)       // filename ends with a dot
		return "";

	// Don't include the dot, just the extension itself
	return filename.substr(index + 1);
}

size_t Path::GetFileSize(const std::string& path)
{
	struct stat fileInfo;
	if (!GetInfo(path, fileInfo) || !(fileInfo.st_mode & S_IFREG))
		return 0;
	
	return (size_t)fileInfo.st_size;
}

std::string Path::Normalize(const std::string& path)
{
	return path;
}

std::string Path::MakeAbsolute(const std::string& path)
{
	if (Path::IsAbsolute(path))
		return path;

	std::string cwd = GetCWD();

	// If its already absolute just return the original path
	if (::strncmp(cwd.c_str(), path.c_str(), cwd.length()) == 0)
		return path;

	// Get rid of trailing separators if any
	if (path.find_last_of(Path::s_separator) == path.length() - 1)
		cwd = std::string(cwd.c_str(), cwd.length() - 1);

	// join the cwd to the path and return it (handle duplicate separators) 
	std::string result = cwd;
	if (path.find_first_of(Path::s_separator) == 0)
		return cwd + path;
	else
		return cwd + Path::s_separator + path;

	return "";
}

void Path::Split(const std::string& path, StringVector& parts)
{
	char* localString = new char[path.length() + 1];
	memset(localString, 0, path.length() + 1);

	strcpy(localString, path.c_str());

	char *p = strtok(localString, s_separator);

	while (p)
	{
		parts.push_back(p);
		p = strtok(nullptr, s_separator);
	}

	delete[] localString;
}

std::string Path::Join(StringVector::iterator begin, StringVector::iterator end)
{
	if (begin == end)
		return "";

	std::string path(*begin++);

	while (begin != end)
	{
		path += Path::s_separator;
		path += *begin++;
	};

	return path;
}

Path::operator const char *() const
{
	return _path.c_str();
}

Path& Path::operator+=(const Path& path)
{
	Path::StringVector sv;
	sv.push_back(std::string(_path));
	sv.push_back(std::string(path._path));

	_path = Path::Join(sv.begin(), sv.end());

	return *this;
}

Path Path::GetParent() const
{
	return Path::GetParent(_path);
}

Path Path::GetBasename() const
{
	return Path::GetBasename(_path);
}

Path Path::GetExtension() const
{
	return Path::GetExtension(_path);
}

size_t Path::GetFileSize() const
{
	return Path::GetFileSize(_path);
}

Path& Path::Normalize()
{
	_path = Path::Normalize(_path);
	return *this;
}

Path& Path::MakeAbsolute()
{
	if (!IsAbsolute())
		_path = Path::MakeAbsolute(_path);
	return *this;
}

void Path::Split(StringVector& parts) const
{
	Path::Split(_path, parts);
}

bool Path::IsDirectory() const
{
	return Path::IsDirectory(_path);
}

bool Path::IsFile() const
{
	return Path::IsFile(_path);
}

bool Path::IsSymbolicLink() const
{
	return Path::IsSymbolicLink(_path);
}

bool Path::IsAbsolute() const
{
	return Path::IsAbsolute(_path);
}

bool Path::Exists() const
{
	return Path::Exists(_path);
}

bool Path::IsEmpty() const
{
	return _path.empty();
}