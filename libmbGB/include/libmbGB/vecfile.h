#ifndef VECFILE_H
#define VECFILE_H

#include <vector>
#include <cstdint>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
using namespace std;

struct VecFile
{
    vector<uint8_t> data;
    int loc_pos = 0;
    bool read = false;
    bool write = false;

    vector<uint8_t> get_data()
    {
	return data;
    }

    string to_string()
    {
	return string(data.begin(), data.end());
    }
};

inline VecFile vfopen(void *data, size_t size)
{
    VecFile file;

    char* buf = static_cast<char*>(data);
    vector<uint8_t> file_data(buf, (buf + size));
    file.data = vector<uint8_t>(file_data.begin(), file_data.end());
    return file;
}

inline int vfclose(VecFile &fp)
{
    fp.read = false;
    fp.write = false;
    fp.loc_pos = 0;
    fp.data.clear();
    return 0;
}

inline int vfseek(VecFile &fp, long int offs, int whence)
{
    switch (whence)
    {
	case SEEK_SET:
	{
	    if (offs >= static_cast<long int>(fp.data.size()))
	    {
		return -1;
	    }

	    fp.loc_pos = offs;
	}
	break;
	case SEEK_CUR:
	{
	    if ((offs + fp.loc_pos) >= static_cast<long int>(fp.data.size()))
	    {
		return -1;
	    }

	    fp.loc_pos += offs;
	}
	break;
	case SEEK_END:
	{
	    if (offs > 0)
	    {
		return -1;
	    }

	    fp.loc_pos = static_cast<int>((fp.data.size() + offs));
	}
	break;
    }

    return 0;
}

inline size_t vfwrite(void *ptr, size_t size, size_t nmemb, VecFile &fp)
{
    int total = (nmemb * size);

    if (fp.loc_pos >= static_cast<int>(fp.data.size()))
    {
	return 0;
    }

    if ((fp.loc_pos + total) > static_cast<int>(fp.data.size()))
    {
	cout << "Overwriting..." << endl;
	int64_t ak = (fp.data.size() - fp.loc_pos);

	memcpy(&fp.data[fp.loc_pos], static_cast<uint8_t*>(ptr), ak);

	fp.loc_pos = fp.data.size();
	
	return (ak / size);
    }

    memcpy(&fp.data[fp.loc_pos], static_cast<uint8_t*>(ptr), total);

    fp.loc_pos += total;
    return nmemb;
}

inline size_t vfread(void *ptr, size_t size, size_t nmemb, VecFile &fp)
{
    int total = static_cast<int>((nmemb * size));

    if (fp.loc_pos >= static_cast<int>(fp.data.size()))
    {
	return 0;
    }

    if ((fp.loc_pos + total) > static_cast<int>(fp.data.size()))
    {
	int64_t ak = (fp.data.size() - fp.loc_pos);

	memcpy(static_cast<uint8_t*>(ptr), &fp.data[fp.loc_pos], ak);

	fp.loc_pos = fp.data.size();
	
	return (ak / size);
    }

    memcpy(static_cast<uint8_t*>(ptr), &fp.data[fp.loc_pos], total);

    fp.loc_pos += total;
    return nmemb;
}


#endif // VECFILE_H