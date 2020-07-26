#include <Windows.h>
#include "reg-wrapper.h"
#include <shlwapi.h>
#include <algorithm>

#pragma comment(lib, "shlwapi.lib")

namespace UNONE {
// RegKey ----------------------------------------------------------------------

RegistryKey::RegistryKey()
	: key_(NULL)
	, watch_event_(NULL) 
{
}

RegistryKey::RegistryKey(HKEY rootkey, const char* subkey, REGSAM access)
	: key_(NULL)
	, watch_event_(NULL) 
{
	if (rootkey) 
	{
		if (access & (KEY_SET_VALUE | KEY_CREATE_SUB_KEY | KEY_CREATE_LINK))
			Create(rootkey, subkey, access);
		else
			Open(rootkey, subkey, access);
	}
}

RegistryKey::~RegistryKey() 
{
	Close();
}

LONG RegistryKey::Create( HKEY rootkey, const char* subkey, REGSAM access )
{
	DWORD disposition_value;
	return CreateWithDisposition(rootkey, subkey, &disposition_value, access);
}

LONG RegistryKey::CreateWithDisposition( HKEY rootkey, const char* subkey, DWORD* disposition, REGSAM access )
{
	Close();

	LONG result = RegCreateKeyExA(rootkey, subkey, 0, NULL,
		REG_OPTION_NON_VOLATILE, access, NULL, &key_,
		disposition);
	return result;
}

LONG RegistryKey::CreateKey( const char* name, REGSAM access )
{
	HKEY subkey = NULL;
	LONG result = RegCreateKeyExA(key_, name, 0, NULL, REG_OPTION_NON_VOLATILE,
		access, NULL, &subkey, NULL);
	Close();

	key_ = subkey;
	return result;
}

LONG RegistryKey::Open( HKEY rootkey, const char* subkey, REGSAM access )
{
	Close();
	LONG result = RegOpenKeyExA(rootkey, subkey, 0, access, &key_);
	return result;
}

LONG RegistryKey::OpenKey( const char* relative_key_name, REGSAM access )
{
	HKEY subkey = NULL;
	LONG result = RegOpenKeyExA(key_, relative_key_name, 0, access, &subkey);

	// We have to close the current opened key before replacing it with the new
	// one.
	Close();

	key_ = subkey;
	return result;
}

void RegistryKey::Close() 
{
	StopWatching();
	if (key_) 
	{
		::RegCloseKey(key_);
		key_ = NULL;
	}
}

bool RegistryKey::HasValue( const char* value_name ) const
{
	return RegQueryValueExA(key_, value_name, 0, NULL, NULL, NULL) == ERROR_SUCCESS;
}

DWORD RegistryKey::GetValueCount() const 
{
	DWORD count = 0;
	LONG result = RegQueryInfoKeyA(key_, NULL, 0, NULL, NULL, NULL, NULL, &count,
		NULL, NULL, NULL, NULL);
	return (result == ERROR_SUCCESS) ? count : 0;
}

LONG RegistryKey::GetValueNameAt( int index, std::string& name ) const
{
	char buf[256];
	DWORD bufsize = sizeof(buf) / sizeof(buf[0]);
	LONG r = ::RegEnumValueA(key_, index, buf, &bufsize, NULL, NULL, NULL, NULL);
	if (r == ERROR_SUCCESS)
		name = buf;

	return r;
}

LONG RegistryKey::DeleteKey( const char* name )
{
	LONG result = SHDeleteKeyA(key_, name);
	return result;
}

LONG RegistryKey::DeleteKey(HKEY rootkey, const char* subkey, REGSAM access /* = 0 */)
{
	LONG result = ERROR_SUCCESS;
	access &= (KEY_WOW64_32KEY | KEY_WOW64_64KEY);
	access |= DELETE;

	std::string subkey_str = subkey;
	std::string parent_key;
	std::string key_name;

	do 
	{
		RegistryKey reg;

		if (subkey == NULL)
		{
			result = ERROR_INVALID_PARAMETER;
			break;
		}

		// delete subkeys
		result = reg.Open(rootkey, subkey, access);
		if (result != ERROR_SUCCESS) break;
		result = reg.DeleteKey(NULL);
		if (result != ERROR_SUCCESS) break;
		reg.Close();

		if (subkey_str.find("\\") == std::string::npos)
		{
			result = RegDeleteKeyA(rootkey, subkey);
		}
		else
		{
			key_name = subkey_str.substr(subkey_str.rfind("\\")+1);
			parent_key = subkey_str.substr(0, subkey_str.rfind("\\"));

			result = reg.Open(rootkey, parent_key.c_str(), access);
			if (result != ERROR_SUCCESS) break;
			result = RegDeleteKeyA(reg.Handle(), key_name.c_str());
			reg.Close();
		}
	} while (0);

	return result;
}

LONG RegistryKey::DeleteValue( const char* name )
{
	LONG result = RegDeleteValueA(key_, name);
	return result;
}

LONG RegistryKey::ReadValueDW( const char* name, DWORD* out_value ) const
{
	DWORD type = REG_DWORD;
	DWORD size = sizeof(DWORD);
	DWORD local_value = 0;
	LONG result = ReadValue(name, &local_value, &size, &type);
	if (result == ERROR_SUCCESS) 
	{
		if ((type == REG_DWORD || type == REG_BINARY) && size == sizeof(DWORD))
			*out_value = local_value;
		else
			result = ERROR_CANTREAD;
	}

	return result;
}

LONG RegistryKey::ReadInt64( const char* name, __int64* out_value ) const
{
	DWORD type = REG_QWORD;
	__int64 local_value = 0;
	DWORD size = sizeof(local_value);
	LONG result = ReadValue(name, &local_value, &size, &type);
	if (result == ERROR_SUCCESS) 
	{
		if ((type == REG_QWORD || type == REG_BINARY) &&
			size == sizeof(local_value))
			*out_value = local_value;
		else
			result = ERROR_CANTREAD;
	}

	return result;
}

LONG RegistryKey::ReadValue(const char* name, std::basic_string<char>& out_value) const 
{
	const size_t kMaxStringLength = 1024;  // This is after expansion.
	// Use the one of the other forms of ReadValue if 1024 is too small for you.
	char raw_value[kMaxStringLength];
	DWORD type = REG_SZ, size = sizeof(raw_value);
	LONG result = ReadValue(name, raw_value, &size, &type);
	if (result == ERROR_SUCCESS) 
	{
		if (type == REG_SZ) 
		{
			out_value = raw_value;
		} else if (type == REG_EXPAND_SZ) {
			char expanded[kMaxStringLength];
			size = ExpandEnvironmentStringsA(raw_value, expanded, kMaxStringLength);
			// Success: returns the number of char's copied
			// Fail: buffer too small, returns the size required
			// Fail: other, returns 0
			if (size == 0 || size > kMaxStringLength) 
			{
				result = ERROR_MORE_DATA;
			} else {
				out_value = expanded;
			}
		} else {
			// Not a string. Oops.
			result = ERROR_CANTREAD;
		}
	}

	return result;
}

LONG RegistryKey::ReadValue(const char* name, void* data, DWORD* dsize, DWORD* dtype) const 
{
	LONG result = RegQueryValueExA(key_, name, 0, dtype, reinterpret_cast<LPBYTE>(data), dsize);
	return result;
}

LONG RegistryKey::ReadValues( const char* name, std::vector<std::basic_string<char>>& values )
{
	values.clear();

	DWORD type = REG_MULTI_SZ;
	DWORD size = 0;
	LONG result = ReadValue(name, NULL, &size, &type);
	if (FAILED(result) || size == 0)
		return result;

	if (type != REG_MULTI_SZ)
		return ERROR_CANTREAD;

	std::vector<char> buffer(size / sizeof(char));
	result = ReadValue(name, &buffer[0], &size, NULL);
	if (FAILED(result) || size == 0)
		return result;

	// Parse the double-null-terminated list of strings.
	// Note: This code is paranoid to not read outside of |buf|, in the case where
	// it may not be properly terminated.
	const char* entry = &buffer[0];
	const char* buffer_end = entry + (size / sizeof(char));
	while (entry < buffer_end && entry[0] != '\0') 
	{
		const char* entry_end = std::find(entry, buffer_end, '\0');
		values.push_back(std::basic_string<char>(entry, entry_end));
		entry = entry_end + 1;
	}
	return 0;
}

LONG RegistryKey::WriteValue(const char* name, DWORD in_value) 
{
	return WriteValue(name, &in_value, static_cast<DWORD>(sizeof(in_value)), REG_DWORD);
}

LONG RegistryKey::WriteValue(const char* name, const std::basic_string<char>& in_value) 
{
	return WriteValue(name, in_value.c_str(), static_cast<DWORD>(sizeof(char) * (in_value.size() + 1)), REG_SZ);
}

LONG RegistryKey::WriteValue(const char* name, const void* data, DWORD dsize, DWORD dtype) 
{
	LONG result = RegSetValueExA(key_, name, 0, dtype,
		reinterpret_cast<LPBYTE>(const_cast<void*>(data)), dsize);
	return result;
}

LONG RegistryKey::StartWatching()
{
	if (!watch_event_)
		watch_event_ = CreateEventA(NULL, TRUE, FALSE, NULL);

	DWORD filter = REG_NOTIFY_CHANGE_NAME |
		REG_NOTIFY_CHANGE_ATTRIBUTES |
		REG_NOTIFY_CHANGE_LAST_SET |
		REG_NOTIFY_CHANGE_SECURITY;

	// Watch the registry key for a change of value.
	LONG result = RegNotifyChangeKeyValue(key_, TRUE, filter, watch_event_, TRUE);
	if (result != ERROR_SUCCESS)
	{
		CloseHandle(watch_event_);
		watch_event_ = 0;
	}

	return result;
}

bool RegistryKey::HasChanged() 
{
	if (watch_event_) 
	{
		if (WaitForSingleObject(watch_event_, 0) == WAIT_OBJECT_0) 
		{
			StartWatching();
			return true;
		}
	}
	return false;
}

LONG RegistryKey::StopWatching()
{
	LONG result = ERROR_INVALID_HANDLE;
	if (watch_event_) 
	{
		CloseHandle(watch_event_);
		watch_event_ = 0;
		result = ERROR_SUCCESS;
	}
	return result;
}

// RegistryValueIterator ------------------------------------------------------

RegistryValueIterator::RegistryValueIterator(HKEY root_key, const char* folder_key)
{
	LONG result = RegOpenKeyExA(root_key, folder_key, 0, KEY_READ, &key_);
	if (result != ERROR_SUCCESS)
	{
		key_ = NULL;
	} else {
		DWORD count = 0;
		result = ::RegQueryInfoKeyA(key_, NULL, 0, NULL, NULL, NULL, NULL, &count,
			NULL, NULL, NULL, NULL);

		if (result != ERROR_SUCCESS) 
		{
			::RegCloseKey(key_);
			key_ = NULL;
		} else {
			index_ = count - 1;
		}
	}

	Read();
}

RegistryValueIterator::~RegistryValueIterator() 
{
	if (key_)
		::RegCloseKey(key_);
}

DWORD RegistryValueIterator::ValueCount() const 
{
	DWORD count = 0;
	LONG result = ::RegQueryInfoKeyA(key_, NULL, 0, NULL, NULL, NULL, NULL,
		&count, NULL, NULL, NULL, NULL);
	if (result != ERROR_SUCCESS)
		return 0;

	return count;
}

bool RegistryValueIterator::Valid() const 
{
	return key_ != NULL && index_ >= 0;
}

void RegistryValueIterator::operator++()
{
	--index_;
	Read();
}

bool RegistryValueIterator::Read()
{
	if (Valid()) 
	{
		DWORD ncount = sizeof(name_) / sizeof(name_[0]);
		value_size_ = 0;
		LONG r = ::RegEnumValueA(key_, index_, name_, &ncount, NULL, &type_, (BYTE*)value_.c_str(), &value_size_);
		if (ERROR_MORE_DATA == r)
		{
			ncount = sizeof(name_) / sizeof(name_[0]);
			value_.resize(value_size_);
			r = ::RegEnumValueA(key_, index_, name_, &ncount, NULL, &type_, (BYTE*)value_.c_str(), &value_size_);
			if (ERROR_SUCCESS == r)
			{
				return true;
			}
		}
	}
	name_[0] = '\0';
	value_.clear();
	value_size_ = 0;
	return false;
}

// RegistryKeyIterator --------------------------------------------------------

RegistryKeyIterator::RegistryKeyIterator(HKEY root_key, const char* folder_key)
{
	LONG result = RegOpenKeyExA(root_key, folder_key, 0, KEY_READ, &key_);
	if (result != ERROR_SUCCESS) 
	{
		key_ = NULL;
	} else {
		DWORD count = 0;
		LONG result = ::RegQueryInfoKeyA(key_, NULL, 0, NULL, &count, NULL, NULL,
			NULL, NULL, NULL, NULL, NULL);

		if (result != ERROR_SUCCESS) 
		{
			::RegCloseKey(key_);
			key_ = NULL;
		} else {
			index_ = count - 1;
		}
	}

	Read();
}

RegistryKeyIterator::~RegistryKeyIterator() 
{
	if (key_)
		::RegCloseKey(key_);
}

DWORD RegistryKeyIterator::SubkeyCount() const
{
	DWORD count = 0;
	LONG result = ::RegQueryInfoKeyA(key_, NULL, 0, NULL, &count, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL);
	if (result != ERROR_SUCCESS)
		return 0;

	return count;
}

bool RegistryKeyIterator::Valid() const 
{
	return key_ != NULL && index_ >= 0;
}

void RegistryKeyIterator::operator++() 
{
	--index_;
	Read();
}

bool RegistryKeyIterator::Read() 
{
	if (Valid())
	{
		DWORD ncount = sizeof(name_) / sizeof(name_[0]);
		FILETIME written;
		LONG r = ::RegEnumKeyExA(key_, index_, name_, &ncount, NULL, NULL,
			NULL, &written);
		if (ERROR_SUCCESS == r)
			return true;
	}

	name_[0] = '\0';
	return false;
}
} // namespace UNONE