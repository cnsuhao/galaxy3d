#include "GTFile.h"
#include <fstream>
#include "Debug.h"
#include "GTString.h"
#include "Application.h"

#ifdef WINPHONE
#include <wrl/client.h>
#include <ppl.h>
#include <ppltasks.h>
#endif

extern "C" char *file_read_all_bytes(const char *file, int *size)
{
	std::vector<char> bytes = Galaxy3D::GTFile::ReadAllBytes(file);
	if (!bytes.empty())
	{
		char *ret = (char *) malloc(bytes.size());
		memcpy(ret, &bytes[0], bytes.size());
		*size = bytes.size();
		return ret;
	}

	*size = 0;
	return 0;
}

extern "C" int file_exist(const char *file)
{
	return Galaxy3D::GTFile::Exist(file) ? 1 : 0;
}

#ifdef WINPHONE
template<class T>
static void wait_for_async(T ^A)
{
	while (A->Status == Windows::Foundation::AsyncStatus::Started)
	{
		//Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(Windows::UI::Core::CoreProcessEventsOption::ProcessAllIfPresent);
	}

	Windows::Foundation::AsyncStatus S = A->Status;
}
#endif

namespace Galaxy3D
{
#ifdef WINPHONE
	bool GTFile::Exist(const std::string &path)
	{
		using namespace Windows::Storage;
		using namespace Concurrency;

		std::vector<char> ret;

		GTString path_relative(path);
		path_relative = path_relative.Replace("/", "\\");
		GTString data_path = Application::GetDataPath();
		data_path = data_path.Replace("/", "\\");
		if (path_relative.StartsWith(data_path.str))
		{
			path_relative = path_relative.str.substr(data_path.str.size() + 1);
		}

		wchar_t buffer[MAX_PATH];
		int size = MultiByteToWideChar(CP_ACP, 0, path_relative.str.c_str(), path_relative.str.size(), buffer, MAX_PATH);
		buffer[size] = 0;
		auto wpath = ref new Platform::String(buffer);

		try
		{
			auto find_async = Windows::ApplicationModel::Package::Current->InstalledLocation->GetFileAsync(wpath);
			wait_for_async(find_async);

			auto file = find_async->GetResults();
			return true;
		}
		catch (Platform::InvalidArgumentException ^e)
		{
			return false;
		}
		catch (Platform::COMException ^e)
		{
			if (e->HResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
			{
				Debug::Log("GTFile::Exist Platform::COMException ERROR_FILE_NOT_FOUND");
				return false;
			}
			else
			{
				throw(e);
			}
		}
		
		return false;
	}
#else
	bool GTFile::Exist(const std::string &path)
	{
		std::ifstream is(path.c_str(), std::ios::in | std::ios::binary);
		if(is)
		{
			is.close();
			return true;
		}

		return false;
	}
#endif

#ifdef WINPHONE
	std::vector<char> GTFile::ReadAllBytes(const std::string &path)
	{
		using namespace Windows::Storage;
		using namespace Concurrency;

		std::vector<char> ret;

		GTString path_relative(path);
		path_relative = path_relative.Replace("/", "\\");
		GTString data_path = Application::GetDataPath();
		data_path = data_path.Replace("/", "\\");
		if (path_relative.StartsWith(data_path.str))
		{
			path_relative = path_relative.str.substr(data_path.str.size() + 1);
		}

		wchar_t buffer[MAX_PATH];
		int size = MultiByteToWideChar(CP_ACP, 0, path_relative.str.c_str(), path_relative.str.size(), buffer, MAX_PATH);
		buffer[size] = 0;
		auto wpath = ref new Platform::String(buffer);

		try
		{
			auto find_async = Windows::ApplicationModel::Package::Current->InstalledLocation->GetFileAsync(wpath);
			wait_for_async(find_async);

			auto file = find_async->GetResults();
			auto open_async = file->OpenReadAsync();
			wait_for_async(open_async);

			auto stream = open_async->GetResults();
			int bufferSize = (int)stream->Size;
			auto fileBuffer = ref new Streams::Buffer(bufferSize);
			auto read_async = stream->ReadAsync(fileBuffer, bufferSize, Streams::InputStreamOptions::None);
			wait_for_async(read_async);

			auto fileData = ref new Platform::Array<byte>(fileBuffer->Length);
			Streams::DataReader::FromBuffer(fileBuffer)->ReadBytes(fileData);

			ret.resize(fileData->Length);
			memcpy(&ret[0], fileData->Data, ret.size());

			return ret;
		}
		catch (Platform::InvalidArgumentException ^e)
		{
		}
		catch (Platform::COMException ^e)
		{
			if (e->HResult != HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
			{
				throw(e);
			}
		}

		Debug::Log("File %s does not exist!!!", path.c_str());

		return ret;
	}
#else
	std::vector<char> GTFile::ReadAllBytes(const std::string &path)
	{
		std::vector<char> ret;

		std::ifstream is(path.c_str(), std::ios::in | std::ios::binary);
		if (is)
		{
			is.seekg(0, std::ios::end);
			int size = (int)is.tellg();
			is.seekg(0, std::ios::beg);

			ret.resize(size);
			if(size > 0)
			{
				is.read(&ret[0], size);
			}
			is.close();
		}
		else
		{
			Debug::Log("File %s does not exist!!!", path.c_str());
		}

		return ret;
	}
#endif

	std::string GTFile::ReadAllText(const std::string &path)
	{
		std::string ret;

		std::vector<char> buffer = ReadAllBytes(path);
		int size = (int)buffer.size();
		if (size > 0)
		{
			ret.resize(size);
			memcpy(&ret[0], &buffer[0], size);
		}

		return ret;
	}

	void GTFile::WriteAllBytes(const std::string &path, void *data, int size)
	{
		std::ofstream os(path.c_str(), std::ios::out | std::ios::binary);
		if (os)
		{
			os.write((const char *)data, size);
			os.close();
		}
	}
}