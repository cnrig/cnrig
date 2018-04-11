/* CNRig
 * Copyright 2018 mode0x13 <mode0x13@firemail.cc>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <fstream>

extern "C" {
	extern void* _binary_cacert_pem_start;
	extern void* _binary_cacert_pem_end;
	extern void* _binary_cacert_pem_size;
}

void writeCAcerts(const char* path) {
	char* data_start     = (char*) &_binary_cacert_pem_start;
	char* data_end       = (char*) &_binary_cacert_pem_end;
	size_t data_size     = data_end - data_start;

	std::ofstream of(path, std::ios::out | std::ios::binary);
	of.write(data_start, data_size);
	of.close();
}
