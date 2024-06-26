
#include "Precomp.h"
#include "File.h"
#include "UTF16.h"
#ifdef WIN32
#include <Windows.h>
#else
#include <libgen.h>
#include <fnmatch.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif
#ifdef EXTERN___PROGNAME
extern const char* __progname;
#endif
#endif
#include "Exception.h"
#include <string.h>
#include <sstream>
#include <iostream>

class FileImpl : public File
{
public:
	FileImpl(FILE* handle) : handle(handle)
	{
	}

	int64_t size() override
	{
		auto pos = ftell(handle);
		fseek(handle, 0, SEEK_END);
		auto s = ftell(handle);
		fseek(handle, pos, SEEK_SET);
		return s;
	}

	void read(void *data, size_t size) override
	{
		size_t result = fread(data, size, 1, handle);
		if (result != 1)
			Exception::Throw("fread failed");
	}

	void write(const void *data, size_t size) override
	{
		size_t result = fwrite(data, size, 1, handle);
		if (result != 1)
			Exception::Throw("fwrite failed");
	}

	void seek(int64_t offset, SeekPoint origin = SeekPoint::begin) override
	{
		int moveMethod = SEEK_SET;
		if (origin == SeekPoint::current) moveMethod = SEEK_CUR;
		else if (origin == SeekPoint::end) moveMethod = SEEK_END;
		int result = fseek(handle, offset, moveMethod);
		if (result != 0)
			Exception::Throw("fseek failed");
	}

	uint64_t tell() override
	{
		auto result = ftell(handle);
		if (result == -1)
			Exception::Throw("ftell failed");
		return result;
	}

	FILE* handle = nullptr;
};

std::shared_ptr<File> File::create_always(const std::string &filename)
{
	FILE* handle = fopen(filename.c_str(), "wb");
	if (handle == nullptr)
		Exception::Throw("Could not create " + filename);

	return std::make_shared<FileImpl>(handle);
}

std::shared_ptr<File> File::open_existing(const std::string &filename, int debug_index)
{
#ifdef DEBUG_INDEXING_ENABLED
	std::cout << "File::open_existing: " << filename << ", debug_index: " << debug_index << std::endl;
#endif
	FILE* handle = fopen(filename.c_str(), "rb");
	if (handle == nullptr)
		Exception::Throw("Could not open " + filename);

	return std::make_shared<FileImpl>(handle);
}

void File::write_all_bytes(const std::string& filename, const void* data, size_t size)
{
	auto file = create_always(filename);
	file->write(data, size);
}

void File::write_all_text(const std::string& filename, const std::string& text)
{
	auto file = create_always(filename);
	file->write(text.data(), text.size());
}

std::vector<uint8_t> File::read_all_bytes(const std::string& filename, int debug_index)
{
#ifdef DEBUG_INDEXING_ENABLED	
	std::cout << "File::read_all_bytes; debug_index: " << debug_index << std::endl;
#endif
	auto file = open_existing(filename, 1);
	std::vector<uint8_t> buffer(file->size());
	file->read(buffer.data(), buffer.size());
	return buffer;
}

std::string File::read_all_text(const std::string& filename)
{
	auto file = try_open_existing(filename);
	if (file == nullptr) return {};
	auto size = file->size();
	if (size == 0) return {};
	std::string buffer;
	buffer.resize(size);
	file->read(&buffer[0], buffer.size());
	return buffer;
}

std::vector<std::string> File::read_all_lines(const std::string& filename)
{
	std::string text = read_all_text(filename);

	if (text.empty())
		return {};

	std::vector<std::string> result;

	std::string buffer;

	std::stringstream ss(text);

	while (std::getline(ss, buffer)) // Default delimiter is '\n'
		result.push_back(buffer);

	return result;
}

/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32

std::vector<std::string> Directory::files(const std::string& filename)
{
	std::vector<std::string> files;

	WIN32_FIND_DATA fileinfo;
	HANDLE handle = FindFirstFile(to_utf16(filename).c_str(), &fileinfo);
	if (handle == INVALID_HANDLE_VALUE)
		return {};

	try
	{
		do
		{
			bool is_directory = !!(fileinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
			if (!is_directory)
				files.push_back(from_utf16(fileinfo.cFileName));
		} while (FindNextFile(handle, &fileinfo) == TRUE);
		FindClose(handle);
	}
	catch (...)
	{
		FindClose(handle);
		throw;
	}

	return files;
}

void Directory::make_directory(const std::string& dirname)
{
	if (!CreateDirectory(to_utf16(dirname).c_str(), NULL))
	{
		if (GetLastError() != ERROR_ALREADY_EXISTS)
			Exception::Throw("Could not create directory " + dirname);
	}
}

#else

std::vector<std::string> Directory::files(const std::string& filename)
{
	std::vector<std::string> files;

	std::string filter = FilePath::last_component(filename);
	std::string path = FilePath::remove_last_component(filename);

	DIR *dir = opendir(path.c_str());
	if (!dir)
	{
		//Exception::Throw("Could not open folder: " + path);
		//printf("Could not open folder: %s\n", path.c_str());
		return {};
	}
	
	while (true)
	{
		dirent *entry = readdir(dir);
		if (!entry)
			break;

		std::string name = entry->d_name;
		if (fnmatch(filter.c_str(), name.c_str(), FNM_PATHNAME) == 0)
		{
			struct stat statbuf;
			if (stat(FilePath::combine(path, name).c_str(), &statbuf) == -1)
				memset(&statbuf, 0, sizeof(statbuf));

			bool is_directory = (statbuf.st_mode & S_IFDIR) != 0;
			if (!is_directory)
				files.push_back(name);
		}
	}

	closedir(dir);
	return files;
}

void Directory::make_directory(const std::string& dirname)
{
	if (mkdir(dirname.c_str(), 0777) < 0)
	{
		if (errno != EEXIST)
			Exception::Throw("Could not create directory " + dirname);
	}
}

#endif

std::string OS::executable_path()
{
#if defined(WIN32)
	WCHAR exe_filename[_MAX_PATH];
	DWORD len = GetModuleFileName(nullptr, exe_filename, _MAX_PATH);
	if (len == 0 || len == _MAX_PATH)
		Exception::Throw("GetModuleFileName failed!");

	WCHAR drive[_MAX_DRIVE], dir[_MAX_DIR];
	_wsplitpath_s(exe_filename, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0);

	return from_utf16(std::wstring(drive) + dir);
#elif defined(__APPLE__)
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	if (mainBundle)
	{
		CFURLRef mainURL = CFBundleCopyBundleURL(mainBundle);

		if (mainURL)
		{
			char exe_file[PATH_MAX];
			int ok = CFURLGetFileSystemRepresentation(mainURL, (Boolean)true, (UInt8*)exe_file, PATH_MAX);
			if (ok)
			{
				return std::string(exe_file) + "/";
			}
		}
	}

	Exception::Throw("get_exe_path failed");
#else
	#ifndef PROC_EXE_PATH
	#define PROC_EXE_PATH "/proc/self/exe"
	#endif

	char exe_file[PATH_MAX];
	int size;
	struct stat sb;
	if (lstat(PROC_EXE_PATH, &sb) < 0)
	{
		#ifdef EXTERN___PROGNAME
			char* pathenv, * name, * end;
			char fname[PATH_MAX];
			char cwd[PATH_MAX];
			struct stat sba;

			exe_file[0] = '\0';
			if ((pathenv = getenv("PATH")) != nullptr)
			{
				for (name = pathenv; name; name = end)
				{
					if ((end = strchr(name, ':')))
						*end++ = '\0';
					snprintf(fname, sizeof(fname),
						"%s/%s", name, (char*)__progname);
					if (stat(fname, &sba) == 0) {
						snprintf(exe_file, sizeof(exe_file),
							"%s/", name);
						break;
					}
				}
			}
			// if getenv failed or path still not found
			// try current directory as last resort
			if (!exe_file[0])
			{
				if (getcwd(cwd, sizeof(cwd)) != nullptr)
				{
					snprintf(fname, sizeof(fname),
						"%s/%s", cwd, (char*)__progname);
					if (stat(fname, &sba) == 0)
						snprintf(exe_file, sizeof(exe_file),
							"%s/", cwd);
				}
			}
			if (!exe_file[0])
				Exception::Throw("get_exe_path: could not find path");
			else
				return std::string(exe_file);
		#else
			Exception::Throw("get_exe_path: proc file system not accesible");
		#endif
	}
	else
	{
		size = readlink(PROC_EXE_PATH, exe_file, PATH_MAX);
		if (size < 0)
		{
			Exception::Throw(strerror(errno));
		}
		else
		{
			exe_file[size] = '\0';
			return std::string(dirname(exe_file)) + "/";
		}
	}

	#endif
}

std::string OS::get_default_font_name()
{
#ifdef WIN32
	return "segoeui.ttf";
#elif defined(APPLE)
	return "SFUIDisplay-Regular.otf"; // Guess this is the default on Apple?
#else
	return "DejaVuSans.ttf";
#endif
}

std::string OS::find_truetype_font(const std::string& font_name_and_extension)
{
#ifdef WIN32
	const std::vector<std::string> possible_fonts_folders = {
		"C:\\Windows\\Fonts"
	};
#elif defined(APPLE)
	// Based on: https://superuser.com/a/407449
	// Start from user Fonts folder, and go up from there
	const std::vector<std::string> possible_fonts_folders = {
		"~/Library/Fonts",
		"/Library/Fonts",
		"/System/Library/Fonts"
	};
#else
	// Linux is Linux and as always, how the fonts stored can be wholly different
	// So here we try to account for possible paths
	// ...is there a better way to do this?
	const std::vector<std::string> possible_fonts_folders = {
		"/usr/share/fonts",
		"/usr/share/fonts/truetype",
		"/usr/share/fonts/TTF"
	};
#endif

	for (auto& current_font_folder : possible_fonts_folders) {
		std::string final_path = FilePath::combine(current_font_folder, font_name_and_extension);

		if (File::try_open_existing(final_path))
			return final_path;
	}

	// We couldn't find the font in any of the folders, bail out
	return "";
}

/////////////////////////////////////////////////////////////////////////////

bool FilePath::has_extension(const std::string &filename, const char *checkext)
{
	auto fileext = extension(filename);
#ifdef WIN32
	return _stricmp(fileext.c_str(), checkext) == 0;
#else
	return strcasecmp(fileext.c_str(), checkext) == 0;
#endif
}

std::string FilePath::extension(const std::string &filename)
{
	std::string file = last_component(filename);
	std::string::size_type pos = file.find_last_of('.');
	if (pos == std::string::npos)
		return std::string();

#ifndef WIN32
	// Files beginning with a dot is not a filename extension in Unix.
	// This is different from Windows where it is considered the extension.
	if (pos == 0)
		return std::string();
#endif

	return file.substr(pos + 1);

}

std::string FilePath::remove_extension(const std::string &filename)
{
	std::string file = last_component(filename);
	std::string::size_type pos = file.find_last_of('.');
	if (pos == std::string::npos)
		return filename;
	else
		return filename.substr(0, filename.length() - file.length() + pos);
}

std::string FilePath::first_component(const std::string& path)
{
	auto path_conv = convert_path_delimiters(path);
#ifdef WIN32
	auto first_slash = path_conv.find_first_of("/\\");
#else
	auto first_slash = path_conv.find_first_of('/');
#endif
	if (first_slash != std::string::npos)
		return path_conv.substr(0, first_slash);
	else
		return path_conv;
}

std::string FilePath::remove_first_component(const std::string& path)
{
	auto path_conv = convert_path_delimiters(path);
#ifdef WIN32
	auto first_slash = path_conv.find_first_of("/\\");
#else
	auto first_slash = path_conv.find_first_of('/');
#endif
	if (first_slash != std::string::npos)
		return path_conv.substr(first_slash + 1);
	else
		return std::string();
}

std::string FilePath::last_component(const std::string &path)
{
	auto path_conv = convert_path_delimiters(path);
#ifdef WIN32
	auto last_slash = path_conv.find_last_of("/\\");
#else
	auto last_slash = path_conv.find_last_of('/');
#endif
	if (last_slash != std::string::npos)
		return path_conv.substr(last_slash + 1);
	else
		return path_conv;
}

std::string FilePath::remove_last_component(const std::string &path)
{
	auto path_conv = convert_path_delimiters(path);
#ifdef WIN32
	auto last_slash = path_conv.find_last_of("/\\");
#else
	auto last_slash = path_conv.find_last_of('/');
#endif
	if (last_slash != std::string::npos)
		return path_conv.substr(0, last_slash);
	else
		return std::string();
}

std::string FilePath::combine(const std::string &path1, const std::string &path2)
{
	auto path1_conv = convert_path_delimiters(path1);
	auto path2_conv = convert_path_delimiters(path2);
#ifdef WIN32
	if (path1_conv.empty())
		return path2_conv;
	else if (path2_conv.empty())
		return path1_conv;
	else if (path2_conv.front() == '/' || path2_conv.front() == '\\')
		return path2_conv;
	else if (path1_conv.back() != '/' && path1_conv.back() != '\\')
		return path1_conv + "\\" + path2_conv;
	else
		return path1_conv + path2_conv;
#else
	if (path1_conv.empty())
		return path2_conv;
	else if (path2_conv.empty())
		return path1_conv;
	else if (path2_conv.front() == '/')
		return path2_conv;
	else if (path1_conv.back() != '/')
		return path1_conv + "/" + path2_conv;
	else {
		auto value = path1_conv + path2_conv;
		std::cout << "combine result: "<< value  << std::endl;
		return value;
	}
#endif
}

std::string FilePath::convert_path_delimiters(const std::string &path)
{
	std::string result = path;
#ifdef WIN32
	auto pos = result.find("/");
	while (pos != std::string::npos)
	{
		result.replace(result.find("/"), 1, "\\");
		pos = result.find("/");
	}
#else
	auto pos = result.find("\\");
	while (pos != std::string::npos)
	{
		result.replace(result.find("\\"), 1, "/");
		pos = result.find("\\");
	}
#endif
	return result;
}

std::string FilePath::relative_to_absolute_from_system(std::string game_system_path, std::string relative_path)
{
	auto first_component = FilePath::first_component(relative_path);

	while (first_component == ".." || first_component == ".")
	{
		if (first_component == "..")
		{
			// "Go one directory up" as many as the amount of ".."s in the current_path
			game_system_path = FilePath::remove_last_component(game_system_path);
		}

		relative_path = FilePath::remove_first_component(relative_path);
		first_component = FilePath::first_component(relative_path);
	}

	// Combine everything
	return FilePath::combine(game_system_path, relative_path);
}
