#ifndef _UNONE_REGISTRY_H_
#define _UNONE_REGISTRY_H_
#pragma once

#include <windows.h>
#include <string>
#include <vector>

namespace UNONE {
// Utility class to read, write and manipulate the Windows Registry.
// Registry vocabulary primer: a "key" is like a folder, in which there
// are "values", which are <name, data> pairs, with an associated data type.
//
// Note:
// ReadValue family of functions guarantee that the return arguments
// are not touched in case of failure.
class RegistryKey
{
public:
	RegistryKey();
	RegistryKey(HKEY rootkey, const char* subkey, REGSAM access);
	~RegistryKey();

	LONG Create(HKEY rootkey, const char* subkey, REGSAM access);

	LONG CreateWithDisposition(HKEY rootkey, const char* subkey, DWORD* disposition, REGSAM access);

	// Creates a subkey or open it if it already exists.
	LONG CreateKey(const char* name, REGSAM access);

	// Opens an existing reg key.
	LONG Open(HKEY rootkey, const char* subkey, REGSAM access);

	// Opens an existing reg key, given the relative key name.
	LONG OpenKey(const char* relative_key_name, REGSAM access);

	// Closes this reg key.
	void Close();

	// Returns false if this key does not have the specified value, of if an error
	// occurrs while attempting to access it.
	bool HasValue(const char* value_name) const;

	// Returns the number of values for this key, of 0 if the number cannot be
	// determined.
	DWORD GetValueCount() const;

	// Determine the nth value's name.
	LONG GetValueNameAt(int index, std::string& name) const;

	// True while the key is valid.
	bool Valid() const { return key_ != NULL; }

	// Kill a key and everything that live below it; please be careful when using
	// it.
	// delete subkeys only
	LONG DeleteKey(const char* name);

	// Kill a key and everything that live below it and itself
	// access is KEY_WOW64_32KEY or KEY_WOW64_64KEY
	// delete subkeys and self
	static LONG DeleteKey(HKEY rootkey, const char* subkey, REGSAM access = 0);

	// Deletes a single value within the key.
	LONG DeleteValue(const char* name);

	// Getters:

	// Returns an int32 value. If |name| is NULL or empty, returns the default
	// value, if any.
	LONG ReadValueDW(const char* name, DWORD* out_value) const;

	// Returns an int64 value. If |name| is NULL or empty, returns the default
	// value, if any.
	LONG ReadInt64(const char* name, __int64* out_value) const;

	// Returns a string value. If |name| is NULL or empty, returns the default
	// value, if any.
	LONG ReadValue(const char* name, std::string& out_value) const;

	// Reads a REG_MULTI_SZ registry field into a vector of strings. Clears
	// |values| initially and adds further strings to the list. Returns
	// ERROR_CANTREAD if type is not REG_MULTI_SZ.
	LONG ReadValues(const char* name, std::vector<std::string>& values);

	// Returns raw data. If |name| is NULL or empty, returns the default
	// value, if any.
	LONG ReadValue(const char* name, void* data, DWORD* dsize, DWORD* dtype) const;

	// Setters:

	// Sets an int32 value.
	LONG WriteValue(const char* name, DWORD in_value);

	// Sets a string value.
	LONG WriteValue(const char* name, const std::string& in_value) ;

	// Sets raw data, including type.
	LONG WriteValue(const char* name, const void* data, DWORD dsize, DWORD dtype);

	// Starts watching the key to see if any of its values have changed.
	// The key must have been opened with the KEY_NOTIFY access privilege.
	LONG StartWatching();

	// If StartWatching hasn't been called, always returns false.
	// Otherwise, returns true if anything under the key has changed.
	// This can't be const because the |watch_event_| may be refreshed.
	bool HasChanged();

	// Will automatically be called by destructor if not manually called
	// beforehand.  Returns true if it was watching, false otherwise.
	LONG StopWatching();

	inline bool IsWatching() const { return watch_event_ != 0; }
	HANDLE watch_event() const { return watch_event_; }
	HKEY Handle() const { return key_; }

private:
	HKEY key_;  // The registry key being iterated.
	HANDLE watch_event_;
};

// Iterates the entries found in a particular folder on the registry.
class RegistryValueIterator 
{
public:
	RegistryValueIterator(HKEY root_key, const char* folder_key);

	~RegistryValueIterator();

	DWORD ValueCount() const;

	// True while the iterator is valid.
	bool Valid() const;

	// Advances to the next registry entry.
	void operator++();

	const char* Name() const { return name_; }
	std::string Value() const { return value_; }
	DWORD ValueSize() const { return value_size_; }
	DWORD Type() const { return type_; }

	int Index() const { return index_; }

private:
	// Read in the current values.
	bool Read();

	// The registry key being iterated.
	HKEY key_;

	// Current index of the iteration.
	int index_;

	// Current values.
	char name_[MAX_PATH];
	std::string value_;
	DWORD value_size_;
	DWORD type_;
};

class RegistryKeyIterator {
public:
	RegistryKeyIterator(HKEY root_key, const char* folder_key);

	~RegistryKeyIterator();

	DWORD SubkeyCount() const;

	// True while the iterator is valid.
	bool Valid() const;

	// Advances to the next entry in the folder.
	void operator++();

	const char* Name() const { return name_; }

	int Index() const { return index_; }

private:
	// Read in the current values.
	bool Read();

	// The registry key being iterated.
	HKEY key_;

	// Current index of the iteration.
	int index_;

	char name_[MAX_PATH];
};
} // namespace UNONE
#endif  // BASE_WIN_REGISTRY_H_