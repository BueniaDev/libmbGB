/*
    This file is part of libmbGB.
    Copyright (C) 2022 BueniaDev.

    libmbGB is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libmbGB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libmbGB.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "utils.h"
using namespace gb;
using namespace std;

namespace gb
{
    mbGBSavestate::mbGBSavestate(VecFile &file, bool save)
    {
	const char* magic = "MBGB";

	state_file = file;
	error = false;

	if (save)
	{
	    issaving = true;
	    
	    vermajor = 1;
	    verminor = 1;

	    vfwrite((void*)magic, 4, 1, state_file);
	    vfwrite(&vermajor, 2, 1, state_file);
	    vfwrite(&verminor, 2, 1, state_file);
	    vfseek(state_file, 8, SEEK_CUR);
	}
	else
	{
	    issaving = false;
	    if (state_file.data.empty())
	    {
		cout << "Error: could not open savestate" << endl;
		error = true;
		return;
	    }

	    uint32_t len;
	    vfseek(state_file, 0, SEEK_END);
	    len = static_cast<uint32_t>(state_file.loc_pos);
	    vfseek(state_file, 0, SEEK_SET);

	    uint32_t buf = 0;
	    vfread(&buf, 4, 1, state_file);
	    if (buf != ((uint32_t*)magic)[0])
	    {
		cout << "Error: Invaldi magic bytes" << endl;
		error = true;
		return;
	    }

	    vermajor = 0;
	    verminor = 0;

	    vfread(&vermajor, 2, 1, state_file);

	    if (vermajor != 1)
	    {
		cout << "Error: Bad major version" << endl;
		error = true;
		return;
	    }

	    vfread(&verminor, 2, 1, state_file);

	    if (verminor != 1)
	    {
		cout << "Error: Bad minor version" << endl;
		error = true;
		return;
	    }

	    buf = 0;
	    vfread(&buf, 4, 1, state_file);

	    if (buf != len)
	    {
		cout << "Error: Bad length" << endl;
		error = true;
		return;
	    }

	    vfseek(state_file, 4, SEEK_CUR);
	}

	cursection = -1;
    }

    mbGBSavestate::~mbGBSavestate()
    {

    }

    VecFile mbGBSavestate::get_savestate_file()
    {
	if (issaving)
	{
	    if (cursection != -1)
	    {
		uint32_t pos = static_cast<uint32_t>(state_file.loc_pos);
		vfseek(state_file, (cursection + 4), SEEK_SET);

		uint32_t len = (pos - cursection);
		vfwrite(&len, 4, 1, state_file);

		vfseek(state_file, pos, SEEK_SET);
	    }

	    vfseek(state_file, 0, SEEK_END);
	    uint32_t len = static_cast<uint32_t>(state_file.loc_pos);
	    vfseek(state_file, 8, SEEK_SET);
	    vfwrite(&len, 4, 1, state_file);
	}

	return state_file;
    }

    void mbGBSavestate::section(const char* magic)
    {
	if (error)
	{
	    return;
	}

	if (issaving)
	{
	    if (cursection != -1)
	    {
		uint32_t pos = static_cast<uint32_t>(state_file.loc_pos);
		vfseek(state_file, (cursection + 4), SEEK_SET);
		
		uint32_t len = (pos - cursection);
		vfwrite(&len, 4, 1, state_file);

		vfseek(state_file, pos, SEEK_SET);
	    }

	    cursection = static_cast<uint32_t>(state_file.loc_pos);

	    vfwrite((void*)magic, 4, 1, state_file);
	    vfseek(state_file, 12, SEEK_CUR);
	}
	else
	{
	    vfseek(state_file, 0x10, SEEK_SET);

	    for (;;)
	    {
		uint32_t buf = 0;

		vfread(&buf, 4, 1, state_file);

		if (buf != ((uint32_t*)magic)[0])
		{
		    if (buf == 0)
		    {
			cout << "Warning: section of " << string(magic) << " not found." << endl;
			return;
		    }

		    buf = 0;
		    vfread(&buf, 4, 1, state_file);
		    vfseek(state_file, (buf - 8), SEEK_CUR);
		    continue;
		}

		vfseek(state_file, 12, SEEK_CUR);
		break;
	    }
	}
    }

    void mbGBSavestate::var8(uint8_t *var)
    {
	if (error)
	{
	    return;
	}

	if (issaving)
	{
	    vfwrite(var, 1, 1, state_file);
	}
	else
	{
	    vfread(var, 1, 1, state_file);
	}
    }

    void mbGBSavestate::var16(uint16_t *var)
    {
	if (error)
	{
	    return;
	}

	if (issaving)
	{
	    vfwrite(var, 2, 1, state_file);
	}
	else
	{
	    vfread(var, 2, 1, state_file);
	}
    }

    void mbGBSavestate::var32(uint32_t *var)
    {
	if (error)
	{
	    return;
	}

	if (issaving)
	{
	    vfwrite(var, 4, 1, state_file);
	}
	else
	{
	    vfread(var, 4, 1, state_file);
	}
    }

    void mbGBSavestate::var64(uint64_t *var)
    {
	if (error)
	{
	    return;
	}

	if (issaving)
	{
	    vfwrite(var, 8, 1, state_file);
	}
	else
	{
	    vfread(var, 8, 1, state_file);
	}
    }

    void mbGBSavestate::varint(int *var)
    {
	var32(reinterpret_cast<uint32_t*>(var));
    }

    void mbGBSavestate::bool32(bool *var)
    {
	if (issaving)
	{
	    uint32_t val = *var;
	    var32(&val);
	}
	else
	{
	    uint32_t val = 0;
	    var32(&val);
	    *var = (val != 0);
	}
    }

    void mbGBSavestate::vararray(void *data, uint32_t len)
    {
	if (error)
	{
	    return;
	}

	if (issaving)
	{
	    vfwrite(data, len, 1, state_file);
	}
	else
	{
	    vfread(data, len, 1, state_file);
	}
    }
};