#ifndef __Path_INCLUDE_H__
#define __Path_INCLUDE_H__

#include "CppPlugDefs.h"

#include <string>
#include <vector>

class CppPlug_API Path
{
public:
	typedef std::vector<std::string> StringVector;

private:
	Path();

public:
	Path(const std::string& path);

public:
	Path& operator +=(const Path & path);

	operator const char*() const;
	
public:
	bool Exists() const;
	Path GetParent() const;
	Path GetBasename() const;
	Path GetExtension() const;
	size_t GetFileSize() const;
	
	Path& Normalize();

	Path& MakeAbsolute();
	void Split(StringVector& parts) const;
	
	bool IsDirectory() const;
	bool IsFile() const;
	bool IsAbsolute() const;
	bool IsSymbolicLink() const;
	bool IsEmpty() const;

	const std::string& GetPath() const { return _path; }

public:
	static std::string GetCWD();
	static bool SetCWD(const std::string& path);

	static bool Exists(const std::string& path);
	static std::string GetParent(const std::string& path);
	static std::string GetBasename(const std::string& path);
	static std::string GetExtension(const std::string& path);
	static size_t GetFileSize(const std::string& path);
	static std::string Normalize(const std::string& path);
	static std::string MakeAbsolute(const std::string& path);
	static void Split(const std::string& path, StringVector& parts);
	static std::string Join(StringVector::iterator begin, StringVector::iterator end);
	static bool IsDirectory(const std::string& path);
	static bool IsFile(const std::string& path);
	static bool IsSymbolicLink(const std::string& path);
	static bool IsAbsolute(const std::string& path);
	
private:
	static const char* s_separator;

	std::string _path;
};

// Global operator  
static Path operator+(const Path & p1, const Path & p2)
{
	Path::StringVector sv;
	sv.push_back(std::string(p1));
	sv.push_back(std::string(p2));

	return Path::Join(sv.begin(), sv.end());
}

#endif //__Path_INCLUDE_H__