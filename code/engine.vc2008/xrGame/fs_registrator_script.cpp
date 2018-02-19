#include "stdafx.h"
#include "fs_registrator.h"
#include "../xrcore/LocatorApi.h"

using namespace luabind;

const char* get_file_age_str(CLocatorAPI* fs, const char* nm);

CLocatorAPI* getFS()
{
	return &FS;
}


const char* update_path_script(CLocatorAPI* fs, const char* initial, const char* src)
{
	string_path			temp;
	shared_str			temp_2;
	fs->update_path(temp, initial, src);
	temp_2 = temp;
	return *temp_2;
}

class FS_file_list {
	xr_vector<LPSTR>*	m_p;
public:
	FS_file_list(xr_vector<LPSTR>* p) :m_p(p) { }
	u32			Size() { return m_p->size(); }
	const char*		GetAt(u32 idx) { return m_p->at(idx); }
	void		Free() { FS.file_list_close(m_p); };
};

struct FS_item
{
	string_path name;
	u32			size;
	u32			modif;
	string256	buff;

	const char*		NameShort() { return name; }
	const char*		NameFull() { return name; }
	u32			Size() { return size; }
	const char*		Modif()
	{
		struct tm*	newtime;
		time_t t = modif;
		newtime = localtime(&t);
		strcpy(buff, asctime(newtime));
		return		buff;
	}

	const char*		ModifDigitOnly()
	{
		struct tm*	newtime;
		time_t t = modif;
		newtime = localtime(&t);
		sprintf_s(buff, "%02d:%02d:%4d %02d:%02d",
			newtime->tm_mday,
			newtime->tm_mon + 1,
			newtime->tm_year + 1900,
			newtime->tm_hour,
			newtime->tm_min);
		return		buff;
	}
};

template<bool b>
bool sizeSorter(const FS_item& itm1, const FS_item& itm2) {
	if (b) return	(itm1.size<itm2.size);
	return			(itm2.size<itm1.size);
}
template<bool b>
bool modifSorter(const FS_item& itm1, const FS_item& itm2) {
	if (b) return	(itm1.modif<itm2.modif);
	return			(itm2.modif<itm1.modif);
}
template<bool b>
bool nameSorter(const FS_item& itm1, const FS_item& itm2) {
	if (b) return	(xr_strcmp(itm1.name, itm2.name)<0);
	return			(xr_strcmp(itm2.name, itm1.name)<0);
}

class FS_file_list_ex {
	xr_vector<FS_item>	m_file_items;
public:
	enum {
		eSortByNameUp = 0,
		eSortByNameDown,
		eSortBySizeUp,
		eSortBySizeDown,
		eSortByModifUp,
		eSortByModifDown
	};
	FS_file_list_ex(const char* path, u32 flags, const char* mask);

	u32			Size() { return m_file_items.size(); }
	FS_item		GetAt(u32 idx) { return m_file_items[idx]; }
	void		Sort(u32 flags);
};

FS_file_list_ex::FS_file_list_ex(const char* path, u32 flags, const char* mask)
{
	FS_Path* P = FS.get_path(path);
	P->m_Flags.set(FS_Path::flNeedRescan, TRUE);
	FS.m_Flags.set(CLocatorAPI::flNeedCheck, TRUE);
	FS.rescan_pathes();

	FS_FileSet		files;
	FS.file_list(files, path, flags, mask);

	for (const FS_File &it : files) 
	{
		m_file_items.push_back(FS_item());
		FS_item& itm = m_file_items.back();
		std::memset(itm.name, 0, sizeof(itm.name));
		strcat(itm.name, it.name.c_str());
		itm.modif = (u32)it.time_write;
		itm.size = it.size;
	}

	FS.m_Flags.set(CLocatorAPI::flNeedCheck, FALSE);
}

void FS_file_list_ex::Sort(u32 flags)
{
	if (flags == eSortByNameUp)		std::sort(m_file_items.begin(), m_file_items.end(), nameSorter<true>);
	else if (flags == eSortByNameDown)	std::sort(m_file_items.begin(), m_file_items.end(), nameSorter<false>);
	else if (flags == eSortBySizeUp)	std::sort(m_file_items.begin(), m_file_items.end(), sizeSorter<true>);
	else if (flags == eSortBySizeDown)	std::sort(m_file_items.begin(), m_file_items.end(), sizeSorter<false>);
	else if (flags == eSortByModifUp)	std::sort(m_file_items.begin(), m_file_items.end(), modifSorter<true>);
	else if (flags == eSortByModifDown)std::sort(m_file_items.begin(), m_file_items.end(), modifSorter<false>);
}

FS_file_list_ex file_list_open_ex(CLocatorAPI* fs, const char* path, u32 flags, const char* mask)
{
	return FS_file_list_ex(path, flags, mask);
}

FS_file_list file_list_open_script(CLocatorAPI* fs, const char* initial, u32 flags)
{
	return FS_file_list(fs->file_list_open(initial, flags));
}

FS_file_list file_list_open_script_2(CLocatorAPI* fs, const char* initial, const char* folder, u32 flags)
{
	return FS_file_list(fs->file_list_open(initial, folder, flags));
}

void dir_delete_script_2(CLocatorAPI* fs, const char* path, const char* nm, int remove_files)
{
	fs->dir_delete(path, nm, remove_files);
}

void dir_delete_script(CLocatorAPI* fs, const char* full_path, int remove_files)
{
	fs->dir_delete(full_path, remove_files);
}

const char* get_file_age_str(CLocatorAPI* fs, const char* nm)
{
	time_t t = fs->get_file_age(nm);
	struct tm *newtime;
	newtime = localtime(&t);
	return asctime(newtime);
}


////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// SCRIPT C++17 FILESYSTEM - START ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
/*
KRodin: ���� � ���, ��� ��������� FS �� ����� ������������ ����� � ������� ���-��� ������.
��������, ����� � �������, ����� � ��� ��������� ������� ����� ������ (� ���� ������ ���������� �����).
���� � � ���� ������ ��� �������� ����� ��� ���������� (� �� ������ �����������),
�� � ���� ������������ ����� �������������� � ������� �++17 Filesystem, � ������� ����� ������� ���.
�������, ��� �������� ����� ���� ������ � � ������� �������� ���� LFS, �� KRodin �� ���� ����� �����!
//
TODO: ��� �������������, ���������������� ��������� ������� �������� �������, ������� ��� � ��������� .cpp
*/
#include <iomanip>
#include <sstream>
#include <filesystem>
namespace stdfs = std::experimental::filesystem;

//���� �� ����� � �������
decltype(auto) get_engine_dir()
{
	return stdfs::current_path().string();
}

//������� ������ � �����, ����������� �� �����������.
void directory_iterator(const char* dir, const luabind::functor<void> &iterator_func)
{
	for (const stdfs::path& file : stdfs::directory_iterator(dir))
		if (stdfs::is_regular_file(file)) //����� �� ���������
			iterator_func(file);
}

//������� ������ � ����� ������� �����������.
void recursive_directory_iterator(const char* dir, const luabind::functor<void> &iterator_func)
{
	for (const stdfs::path& file : stdfs::recursive_directory_iterator(dir))
		if (stdfs::is_regular_file(file)) //����� �� ���������
			iterator_func(file);
}

//������ ���� �� ����� � �����������.
inline decltype(auto) get_full_path(const stdfs::path& file)
{
	return file.string();
}

//��� ����� ��� ����, �� � �����������.
inline decltype(auto) get_full_filename(const stdfs::path& file)
{
	return file.filename().string();
}

//��� ����� ��� ����, � ��� ����������.
inline decltype(auto) get_short_filename(const stdfs::path& file)
{
	return file.stem().string();
}

//���������� �����.
inline decltype(auto) get_extension(const stdfs::path& file)
{
	return file.extension().string();
}

//����� ���������� ��������� ����� ( �������� � ��������, �� � �� ������ )
inline long long get_last_write_time(const stdfs::path& file)
{
	const auto ftime = stdfs::last_write_time(file);
	long long cftime = decltype(ftime)::clock::to_time_t(ftime);
	return cftime;
}

//����� ���������� ��������� ����� � ������� [������� 02 ��� 2018 14:03:32]
//inline decltype(auto) get_last_write_time_string(const stdfs::path& file)
//{
//	const auto ftime = stdfs::last_write_time(file);
//	const auto cftime = decltype(ftime)::clock::to_time_t(ftime);
//	std::stringstream ss;
//	ss.imbue(std::locale("")); //������������� ��������� ������ ������, ���� �����/���� ������ ���� �� ��������� �����.
//	ss << std::put_time(std::localtime(&cftime), "[%A %d %b %Y %T]");
//	return ss.str();
//}

#pragma optimize("s",on)
void script_register_stdfs(lua_State *L)
{
	module(L, "stdfs")
		[
		def("directory_iterator", &directory_iterator),
		def("recursive_directory_iterator", &recursive_directory_iterator),

		///#TODO: Fix Luabind later
//		class_<stdfs::path>("path")
//			.property("full_path_name", &get_full_path)
//			.property("full_filename", &get_full_filename)
//			.property("short_filename", &get_short_filename)
//			.property("extension", &get_extension)
//			.property("last_write_time", &get_last_write_time)
//			.property("last_write_time_string", &get_last_write_time_string)
//			.property("get_engine_dir", &get_engine_dir)
//		];
}
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// SCRIPT C++17 FILESYSTEM - END ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

#pragma optimize("s",on)
void fs_registrator::script_register(lua_State *L)
{
	script_register_stdfs(L);
	module(L)
	[
		class_<FS_item>("FS_item")
			.def("NameFull",							&FS_item::NameFull)
			.def("NameShort",							&FS_item::NameShort)
			.def("Size",								&FS_item::Size)
			.def("ModifDigitOnly",						&FS_item::ModifDigitOnly)
			.def("Modif",								&FS_item::Modif),

		class_<FS_file_list_ex>("FS_file_list_ex")
			.def("Size",								&FS_file_list_ex::Size)
			.def("GetAt",								&FS_file_list_ex::GetAt)
			.def("Sort",								&FS_file_list_ex::Sort),

		class_<FS_file_list>("FS_file_list")
			.def("Size",								&FS_file_list::Size)
			.def("GetAt",								&FS_file_list::GetAt)
			.def("Free",								&FS_file_list::Free),

/*		class_<FS_Path>("FS_Path")
			.def_readonly("m_Path",						&FS_Path::m_Path)
			.def_readonly("m_Root",						&FS_Path::m_Root)
			.def_readonly("m_Add",						&FS_Path::m_Add)
			.def_readonly("m_DefExt",					&FS_Path::m_DefExt)
			.def_readonly("m_FilterCaption",			&FS_Path::m_FilterCaption),
*/
		class_<CLocatorAPI::file>("fs_file")
			.def_readonly("name",						&CLocatorAPI::file::name)
			.def_readonly("vfs",						&CLocatorAPI::file::vfs)
			.def_readonly("ptr",						&CLocatorAPI::file::ptr)
			.def_readonly("size_real",					&CLocatorAPI::file::size_real)
			.def_readonly("size_compressed",			&CLocatorAPI::file::size_compressed)
			.def_readonly("modif",						&CLocatorAPI::file::modif),


		class_<CLocatorAPI>("FS")
			.enum_("FS_sort_mode")
			[
				value("FS_sort_by_name_up",				int(FS_file_list_ex::eSortByNameUp)),
				value("FS_sort_by_name_down",			int(FS_file_list_ex::eSortByNameDown)),
				value("FS_sort_by_size_up",				int(FS_file_list_ex::eSortBySizeUp)),
				value("FS_sort_by_size_down",			int(FS_file_list_ex::eSortBySizeDown)),
				value("FS_sort_by_modif_up",			int(FS_file_list_ex::eSortByModifUp)),
				value("FS_sort_by_modif_down",			int(FS_file_list_ex::eSortByModifDown))
			]
			.enum_("FS_List")
			[
				value("FS_ListFiles",					int(FS_ListFiles)),
				value("FS_ListFolders",					int(FS_ListFolders)),
				value("FS_ClampExt",					int(FS_ClampExt)),
				value("FS_RootOnly",					int(FS_RootOnly))
			]
			.def("path_exist",							&CLocatorAPI::path_exist)
			.def("update_path",							&update_path_script)
			.def("get_path",							&CLocatorAPI::get_path)
			.def("append_path",							&CLocatorAPI::append_path)
			
			.def("file_delete",							(void	(CLocatorAPI::*)(const char*,const char*)) (&CLocatorAPI::file_delete))
			.def("file_delete",							(void	(CLocatorAPI::*)(const char*)) (&CLocatorAPI::file_delete))

			.def("dir_delete",							&dir_delete_script)
			.def("dir_delete",							&dir_delete_script_2)

			.def("file_rename",							&CLocatorAPI::file_rename)
			.def("file_length",							&CLocatorAPI::file_length)
			.def("file_copy",							&CLocatorAPI::file_copy)

			.def("exist",								(const CLocatorAPI::file*	(CLocatorAPI::*)(const char*)) (&CLocatorAPI::exist))
			.def("exist",								(const CLocatorAPI::file*	(CLocatorAPI::*)(const char*, const char*)) (&CLocatorAPI::exist))

			.def("get_file_age",						&CLocatorAPI::get_file_age)
			.def("get_file_age_str",					&get_file_age_str)
			.def("r_open",								(IReader*	(CLocatorAPI::*)(const char*,const char*)) (&CLocatorAPI::r_open))
			.def("r_open",								(IReader*	(CLocatorAPI::*)(const char*)) (&CLocatorAPI::r_open))
			.def("r_close",								(void (CLocatorAPI::*)(IReader *&))(&CLocatorAPI::r_close))

			.def("w_open",								(IWriter*	(CLocatorAPI::*)(const char*,const char*)) (&CLocatorAPI::w_open))
			.def("w_open",								(IWriter*	(CLocatorAPI::*)(const char*)) (&CLocatorAPI::w_close))
			.def("w_close",								&CLocatorAPI::w_close)

			.def("file_list_open",						&file_list_open_script)
			.def("file_list_open",						&file_list_open_script_2)
			.def("file_list_open_ex",					&file_list_open_ex),

		def("getFS",									getFS)
	];
}
