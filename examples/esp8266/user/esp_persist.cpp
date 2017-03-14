/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * esp_persist.cpp - section bases persistence implementation for ESP8266
 *
 * This file is part of µcuREST Library. http://hutorny.in.ua/projects/micurest
 *
 * The µcuREST Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License v2
 * as published by the Free Software Foundation;
 *
 * The µcuREST Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License v2
 * along with the COJSON Library; if not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 */

 /*
 * This implementation relays on presence of the following lines in ld script:
   _persistent_start = ABSOLUTE(.);
    *(.persistent)
    _persistent_end = ABSOLUTE(.);
 */
extern "C" {
#	include "c_types.h"
#	include "user.h"
#	include "spi_flash.h"
	extern void* _persistent_start;
	extern void* _persistent_end;
}

static inline constexpr uint32 persistent_size() noexcept {
	return (~3) & (
		reinterpret_cast<uint32>(&_persistent_end)  -
		reinterpret_cast<uint32>(&_persistent_start)+3);
}

static constexpr const uint32 sector_size = 0x1000;


static inline constexpr uint32 persistent_sectors() noexcept {
	return (persistent_size() +0xFFF)/sector_size;
}

static inline uint32 user_get_spare_flash_addr(void) noexcept {
	uint32 addr;
	switch( system_get_flash_size_map() ) {
	default:
	case FLASH_SIZE_2M:					addr = 0x040000; break;
	case FLASH_SIZE_4M_MAP_256_256:  	addr = 0x080000; break;
	case FLASH_SIZE_8M_MAP_512_512:		addr = 0x100000; break;
	case FLASH_SIZE_16M_MAP_512_512:
	case FLASH_SIZE_16M_MAP_1024_1024:  addr = 0x200000; break;
	case FLASH_SIZE_32M_MAP_512_512:
	case FLASH_SIZE_32M_MAP_1024_1024:	addr = 0x400000; break;
	}
	return addr - 0x4000 - 2 * sector_size * persistent_sectors();
}

static constexpr uint32 magic_word = 0x54535250;

struct persist_hdr {
	uint32 magic;
	uint32 hash;
};

template<typename T>
static inline constexpr T ror(T V, unsigned int N) noexcept {
  return (V >> N) | (V << (sizeof(T)*8 - N));
}

template<typename T>
static inline constexpr T poly(T V) noexcept {
	return
		ror(V,2) ^ ror(V,1)
		^(sizeof(T) >= 2 ? ror(V, 7) ^ ror(V, 5) : 0)
		^(sizeof(T) >= 4 ? ror(V, 12) ^ ror(V, 10) : 0);
}

template<typename T>
static inline constexpr T hash(T A, T V) noexcept {
	return poly(A^V);
}


template<typename T>
static inline T hash(const T* data, unsigned count) noexcept {
	T accu = magic_word;
	while(count--) { accu = hash(accu, *data++); }
	return accu;
}

static inline uint32 flashhash(uint32 addr, unsigned count) noexcept {
	uint32 accu = magic_word, data = {};
	while(count--) {
		spi_flash_read(addr, &data, sizeof(data));
		accu = hash(accu, data);
		addr+=4;
	}
	return accu;
}

static bool user_load_persistent(uint32 addr) {
	persist_hdr header;
	if( persistent_size() == 0 ) return true;
	if( SPI_FLASH_RESULT_OK != spi_flash_read(addr, (uint32*)&header, sizeof(header)) ) {
		return false;
	}
	if( header.magic != magic_word )
		return false;
	addr += sizeof(header);
	if( header.hash != flashhash(addr, persistent_size()/4) )
		return false;
	return SPI_FLASH_RESULT_OK ==
		spi_flash_read(addr, (uint32*)&_persistent_start, persistent_size());
}

static bool user_save_persistent(uint32 addr) {
	if( persistent_size() == 0 ) return true;
	persist_hdr header = { magic_word, hash((uint32*)&_persistent_start, persistent_size()/4) };
	if( SPI_FLASH_RESULT_OK != spi_flash_erase_sector(addr/sector_size) ) {
		ets_printf("Error erasing flash sector %X (%d)", addr/sector_size, addr/sector_size);
		return false;
	}
	if( SPI_FLASH_RESULT_OK !=
		spi_flash_write(addr, (uint32*)&header, sizeof(header)) ) {
		ets_printf("Error writing flash at addr 0x%X sector %x",
				addr, addr/sector_size);
		return false;
	}
	addr += sizeof(header);
	if( SPI_FLASH_RESULT_OK !=
		spi_flash_write(addr, (uint32*)&_persistent_start, persistent_size())){
		ets_printf("Error writing flash at addr 0x%X sector %x",
				addr, addr/sector_size);
		return false;
	}
	return true;
}

bool user_load_persistent(void) {
	uint32 addr = user_get_spare_flash_addr();
	return
		user_load_persistent(addr) ||
		user_load_persistent(addr+persistent_sectors()*sector_size);
}

bool user_save_persistent(void) {
	uint32 addr = user_get_spare_flash_addr();
	return
		user_save_persistent(addr) |
		user_save_persistent(addr+persistent_sectors()*sector_size);
}
