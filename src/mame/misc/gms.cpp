// license:BSD-3-Clause
// copyright-holders: Tomasz Slanina, David Haywood

/*
实战麻将王 (Shízhàn Májiàng Wáng) by 'Game Men System Co. Ltd.'

PCB Layout
----------

No.6899-B
|--------------------------------------------------------|
|UPC1241H          YM3014  YM2151    14.31818MHz         |
|     VOL       358                  89C51        B1     |
|          M6295                                         |
|                  S1      PAL                           |
|                                             A1         |
|                                                        |
|J                                           6116        |
|A                 P1                        6116        |
|M   DSW3                                                |
|M   DSW2                                                |
|A   DSW1   DSW4                                         |
|                               |-------|    6116        |
|                               |LATTICE|    6116  PAL   |
|               62256    62256  |1032E  |                |
|                               |       |    T1          |
|                    68000      |-------|                |
| 3.6V_BATT     |-------------|                          |
|               |        93C46|                          |
|               |             |                          |
|               |  *          |              6116        |
|               |             |  22MHz       6116        |
|---------------|PLASTIC COVER|--------------------------|
Notes:
      68000 clock - 11.000MHz [22/2]
      VSync       - 58Hz
      Hsync       - none (dead board, no signal)
      M6295 clock - 1.100MHz [22/20], sample rate = 1100000 / 165, chip is printed 'AD-65'
      YM2151 clock- 2.750MHz [22/8], chip is printed 'K-666'. YM3014 chip is printed 'K-664'
                * - Unpopulated position for PIC16F84
        3.6V_BATT - Purpose of battery unknown, does not appear to be used for backup of suicide RAM,
                    and there's no RTC on the board.
            93C46 - 128 x8 EEPROM. This chip was covered by a plastic cover. There's nothing else under
                    the cover, but there was an unpopulated position for a PIC16F84
            89C51 - Atmel 89C51 Microcontroller (protected)

      ROMs -
            P1 - Hitachi HN27C4096  (Main PRG)
            T1 - Macronix MX27C4000 (GFX)
            A1 - Atmel AT27C080     (GFX)
            B1 - Macronix MX261000  (GFX?? or PRG/data for 89C51?)
            S1 - Macronix MX27C2000 (OKI samples)

Keep pressed 9 and press reset to enter service mode.

TODO:
- correct decode for 1st layer in sc2in1 and magslot
- tilemap priorities for cots and ballch
- correct EEPROM hookup for all games
- oki banking
- hookup MCU and YM2151 sound for the mahjong games
- hookup PIC16F84 for rbspm once a CPU core is available
- emulate protection devices correctly instead of patching
- hookup lamps and do layouts
*/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class gms_2layers_state : public driver_device
{
public:
	gms_2layers_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_vidram(*this, "vidram%u", 1U)
		, m_maincpu(*this, "maincpu")
		, m_mcu(*this, "mcu")
		, m_eeprom(*this, "eeprom")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_ymsnd(*this, "ymsnd")
		, m_dsw(*this, "DSW%u", 1U)
	{
	}

	void rbmk(machine_config &config);
	void rbspm(machine_config &config);

	void super555(machine_config &config);

	void init_ballch();
	void init_cots();
	void init_rbspm();
	void init_super555();

protected:
	virtual void video_start() override;

	optional_shared_ptr_array<uint16_t, 3> m_vidram;

	required_device<cpu_device> m_maincpu;
	optional_device<at89c4051_device> m_mcu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<ym2151_device> m_ymsnd;
	optional_ioport_array<4> m_dsw;

	uint16_t m_tilebank = 0;
	tilemap_t *m_tilemap[3]{};

	void super555_mem(address_map &map);

	template <uint8_t Which> void vram_w(offs_t offset, u16 data, u16 mem_mask);

private:
	uint8_t m_mux_data = 0;
	uint16_t m_input_matrix = 0;
	//uint16_t m_prot_data = 0;

	void mcu_io(address_map &map);
	void mcu_mem(address_map &map);
	void rbmk_mem(address_map &map);
	void rbspm_mem(address_map &map);

	uint16_t unk_r();
	uint16_t input_matrix_r();
	void input_matrix_w(uint16_t data);
	void tilebank_w(uint16_t data);
	uint8_t mcu_io_r(offs_t offset);
	void mcu_io_w(offs_t offset, uint8_t data);
	void mcu_io_mux_w(uint8_t data);
	void eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	TILE_GET_INFO_MEMBER(get_tile0_info);
	TILE_GET_INFO_MEMBER(get_tile1_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

class gms_3layers_state : public gms_2layers_state
{
public:
	gms_3layers_state(const machine_config &mconfig, device_type type, const char *tag)
		: gms_2layers_state(mconfig, type, tag)
	{
	}

	void init_sc2in1();

	void magslot(machine_config &config);

protected:
	virtual void video_start() override;

private:
	void magslot_mem(address_map &map);

	TILE_GET_INFO_MEMBER(get_tile2_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

uint16_t gms_2layers_state::unk_r()
{
	return machine().rand();
}

uint16_t gms_2layers_state::input_matrix_r()
{
	uint16_t res = 0xffff;

	// TODO: & 0x00ff are the inputs for keyboard mode in rbmk
	if (m_input_matrix & 0x1000) res &= m_dsw[0]->read();
	if (m_input_matrix & 0x2000) res &= m_dsw[1].read_safe(0xffff);
	if (m_input_matrix & 0x4000) res &= m_dsw[2].read_safe(0xffff);
	if (m_input_matrix & 0x8000) res &= m_dsw[3].read_safe(0xffff);

	return res;
}

void gms_2layers_state::tilebank_w(uint16_t data)
{
	m_tilebank = data;
}

template <uint8_t Which>
void gms_2layers_state::vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vidram[Which][offset]);
	m_tilemap[Which]->mark_tile_dirty(offset);
}

void gms_2layers_state::input_matrix_w(uint16_t data)
{
	m_input_matrix = data;
}

void gms_2layers_state::eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// bad ?
	if (ACCESSING_BITS_0_7)
	{
		m_eeprom->di_write((data & 0x04) >> 2);
		m_eeprom->cs_write((data & 0x01) ? ASSERT_LINE : CLEAR_LINE);

		m_eeprom->clk_write((data & 0x02) ? ASSERT_LINE : CLEAR_LINE);
	}
}


void gms_2layers_state::rbmk_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom().nopw();
	map(0x100000, 0x10ffff).ram();
	map(0x500000, 0x50ffff).ram();
	map(0x900000, 0x900fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x940000, 0x940bff).ram();
	map(0x940c00, 0x940fff).ram().w(FUNC(gms_2layers_state::vram_w<0>)).share(m_vidram[0]);
	map(0x980300, 0x983fff).ram(); // 0x2048  words ???, byte access
	map(0x9c0000, 0x9c0fff).ram().w(FUNC(gms_2layers_state::vram_w<1>)).share(m_vidram[1]);
	map(0xb00000, 0xb00001).w(FUNC(gms_2layers_state::eeprom_w));
	map(0xc00000, 0xc00001).rw(FUNC(gms_2layers_state::input_matrix_r), FUNC(gms_2layers_state::input_matrix_w));
	map(0xc08000, 0xc08001).portr("IN1").w(FUNC(gms_2layers_state::tilebank_w));
	map(0xc10000, 0xc10001).portr("IN2");
	map(0xc18080, 0xc18081).r(FUNC(gms_2layers_state::unk_r));  // TODO: from MCU?
	map(0xc20000, 0xc20000).r("oki", FUNC(okim6295_device::read));
	//map(0xc20080, 0xc20081) // TODO: to MCU?
	map(0xc28000, 0xc28000).w("oki", FUNC(okim6295_device::write));
}

void gms_2layers_state::rbspm_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x200000, 0x200001).w(FUNC(gms_2layers_state::eeprom_w)); // wrong
	map(0x300000, 0x300001).rw(FUNC(gms_2layers_state::input_matrix_r), FUNC(gms_2layers_state::input_matrix_w));
	map(0x308000, 0x308001).portr("IN1").w(FUNC(gms_2layers_state::tilebank_w)); // ok
	map(0x310000, 0x310001).portr("IN2");
	map(0x318080, 0x318081).r(FUNC(gms_2layers_state::unk_r));
	map(0x320000, 0x320000).r("oki", FUNC(okim6295_device::read));
	map(0x328000, 0x328000).w("oki", FUNC(okim6295_device::write));
	map(0x340002, 0x340003).nopw();
	map(0x500000, 0x50ffff).ram();
	map(0x900000, 0x900fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette"); // if removed fails gfx test?
	map(0x940000, 0x940bff).ram();
	map(0x940c00, 0x940fff).ram().w(FUNC(gms_2layers_state::vram_w<0>)).share(m_vidram[0]); // if removed fails palette test?
	map(0x980300, 0x983fff).ram(); // 0x2048  words ???, byte access, u25 and u26 according to test mode
	map(0x9c0000, 0x9c0fff).ram().w(FUNC(gms_2layers_state::vram_w<1>)).share(m_vidram[1]);
}

void gms_2layers_state::super555_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x600000, 0x600001).rw(FUNC(gms_2layers_state::input_matrix_r), FUNC(gms_2layers_state::input_matrix_w));
	map(0x608000, 0x608001).portr("IN1").w(FUNC(gms_2layers_state::tilebank_w)); // ok
	map(0x610000, 0x610001).portr("IN2");
	map(0x618080, 0x618081).nopr();//.lr16(NAME([this] () -> uint16_t { return m_prot_data; })); // reads something here from below, if these are hooked up booting stops with '0x09 U64 ERROR', like it's failing some checksum test
	map(0x620000, 0x620000).r("oki", FUNC(okim6295_device::read)); // TODO: Oki controlled through a GAL at 18C, should be banked, too
	// map(0x620080, 0x620081).lw16(NAME([this] (uint16_t data) { m_prot_data = data; })); // writes something here that expects to read above
	map(0x628000, 0x628000).w("oki", FUNC(okim6295_device::write));
	map(0x638000, 0x638001).nopw(); // lamps / outputs?
	map(0x900000, 0x900fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x940000, 0x940bff).ram();
	map(0x940c00, 0x940fff).ram().w(FUNC(gms_2layers_state::vram_w<0>)).share(m_vidram[0]);
	map(0x980000, 0x983fff).ram();
	map(0x9c0000, 0x9c0fff).ram().w(FUNC(gms_2layers_state::vram_w<1>)).share(m_vidram[1]);
	//map(0xf00000, 0xf00001).w(FUNC(gms_2layers_state::eeprom_w)); // wrong?
}

void gms_3layers_state::magslot_mem(address_map &map)
{
	super555_mem(map);

	map(0x9e0000, 0x9e0fff).ram().w(FUNC(gms_3layers_state::vram_w<2>)).share(m_vidram[2]);
}

void gms_2layers_state::mcu_mem(address_map &map)
{
//  map(0x0000, 0x0fff).rom();
}

uint8_t gms_2layers_state::mcu_io_r(offs_t offset)
{
	if (m_mux_data & 8)
	{
		return m_ymsnd->read(offset & 1);
	}
	else if (m_mux_data & 4)
	{
		// printf("%02x R\n",offset);
		// ...
		return 0xff;
	}
	else
		printf("Warning: mux data R = %02x", m_mux_data);

	return 0xff;
}

void gms_2layers_state::mcu_io_w(offs_t offset, uint8_t data)
{
	if (m_mux_data & 8) { m_ymsnd->write(offset & 1, data); }
	else if (m_mux_data & 4)
	{
		//printf("%02x %02x W\n", offset, data);
		// ...
	}
	else
		printf("Warning: mux data W = %02x", m_mux_data);
}

void gms_2layers_state::mcu_io_mux_w(uint8_t data)
{
	m_mux_data = ~data;
}

void gms_2layers_state::mcu_io(address_map &map)
{
	map(0x0ff00, 0x0ffff).rw(FUNC(gms_2layers_state::mcu_io_r), FUNC(gms_2layers_state::mcu_io_w));
}

static INPUT_PORTS_START( rbmk )
	PORT_START("IN1")   // 16bit
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT  )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")   // 16bit
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_TOGGLE
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MEMORY_RESET ) PORT_TOGGLE
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)


	PORT_START("DSW1")   // 16bit, in test mode first 8 are recognised as dsw1, second 8 as dsw4
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Controls ) ) PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Joystick ) )
	PORT_DIPSETTING(      0x0000, "Keyboard" )

	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Version ) ) PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(      0x4000, "8.8" )
	PORT_DIPSETTING(      0x0000, "8.8-" )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW2")   // 16bit, in test mode first 8 are recognised as dsw2, second 8 as dsw5
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:1") // 1,2,3 should be coinage
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW3")      // 16bit, in test mode first 8 are recognised as dsw3, second 8 as dsw6
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW6:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( rbspm )
	PORT_INCLUDE( rbmk )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Version ) ) PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(      0x4000, "4.1" )
	PORT_DIPSETTING(      0x0000, "4.2" )
INPUT_PORTS_END


static INPUT_PORTS_START( magslot )
	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("3 Lines")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("5 Lines")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_TOGGLE
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MEMORY_RESET ) PORT_TOGGLE
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // but recognized for password entering
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read) // TODO: verify

	// 3 8-dip banks on PCB
	PORT_START("DSW1") // Game setup is password protected, needs reverse engineering of the password
	PORT_DIPNAME( 0x0001, 0x0001, "DSW1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0001, 0x0001, "DSW2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0001, 0x0001, "DSW3" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( super555 )
	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE(0x02, IP_ACTIVE_LOW)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME( "Start / Take" )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME( "Hold 2 / Double Up" )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Paytable" )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME( "Hold 1 / Double Up / Big" )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME( "Hold 3 / Double Up / Small" )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	//PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read) // TODO: verify


	// There are 4 8-DIP banks on PCB but only 3 are shown in test mode.
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0003, 0x0000, "Main Game Rate" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0003, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0004, 0x0000, "Limit Over Score" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0000, "100.000" )
	PORT_DIPSETTING(      0x0004, "200.000" )
	PORT_DIPNAME( 0x0008, 0x0000, "Coin/Key In Over Score" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, "30.000" )
	PORT_DIPSETTING(      0x0008, "50.000" )
	PORT_DIPNAME( 0x0010, 0x0000, "W-Up Game" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, "W-Up Game Rate" ) PORT_DIPLOCATION("SW1:6") // only has effect if the above one is on.
	PORT_DIPSETTING(      0x0020, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x0040, 0x0000, "Auto Mode" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, "Good" )
	PORT_DIPSETTING(      0x0000, "Hits" )
	PORT_DIPNAME( 0x0080, 0x0000, "Five Bars" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0007, 0x0000, "Coin Rate" ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0001, "5" )
	PORT_DIPSETTING(      0x0002, "10" )
	PORT_DIPSETTING(      0x0003, "20" )
	PORT_DIPSETTING(      0x0004, "30" )
	PORT_DIPSETTING(      0x0000, "50" )
	PORT_DIPSETTING(      0x0005, "100" )
	PORT_DIPSETTING(      0x0006, "200" )
	PORT_DIPSETTING(      0x0007, "300" )
	PORT_DIPNAME( 0x0018, 0x0000, "Coin x Times Rate" ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0008, "5" )
	PORT_DIPSETTING(      0x0010, "10" )
	PORT_DIPSETTING(      0x0018, "20" )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, "Counter Jumping" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, "By Keyin Rate" )
	PORT_DIPSETTING(      0x0000, "By Coin Rate" )
	PORT_DIPNAME( 0x0080, 0x0000, "Cards Voice" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0003, 0x0000, "Min. Bet" ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(      0x0000, "10" )
	PORT_DIPSETTING(      0x0001, "20" )
	PORT_DIPSETTING(      0x0002, "30" )
	PORT_DIPSETTING(      0x0003, "50" )
	PORT_DIPNAME( 0x000c, 0x0000, "Max. Bet" ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(      0x0000, "50" )
	PORT_DIPSETTING(      0x0004, "100" )
	PORT_DIPSETTING(      0x0008, "150" )
	PORT_DIPSETTING(      0x000c, "200" )
	PORT_DIPNAME( 0x0010, 0x0000, "Connector" ) PORT_DIPLOCATION("SW3:5") // Hardcoded to JAMMA
	PORT_DIPSETTING(      0x0010, "JAMMA" )
	PORT_DIPSETTING(      0x0000, "JAMMA" )
	PORT_DIPNAME( 0x0020, 0x0000, "Card Choice" ) PORT_DIPLOCATION("SW3:6") // also changes title screen
	PORT_DIPSETTING(      0x0020, "Car" ) // city skyline title screen
	PORT_DIPSETTING(      0x0000, "Poker" ) // lady in red with card title screen
	PORT_DIPNAME( 0x0040, 0x0000, "Last Game Mode" ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(      0x0040, "Rechoice Card" )
	PORT_DIPSETTING(      0x0000, "Only 6 Card" )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:8") // not shown in test mode
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "SW4:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "SW4:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW4:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW4:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW4:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW4:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x00, "SW4:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW4:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( sc2in1 )
	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE(0x02, IP_ACTIVE_LOW)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME( "Start / Take" )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME( "Hold 2 / Double Up" )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Paytable" )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME( "Hold 1 / Double Up / Big" )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME( "Hold 3 / Double Up / Small" )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	//PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read) // TODO: verify


	// Only 1 8-DIP bank on PCB
	PORT_START("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0000, "SW1:1")
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0000, "SW1:2")
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0000, "SW1:3")
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0000, "SW1:4")
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0000, "SW1:5")
	PORT_DIPNAME(          0x0020, 0x0000, "Game Setup" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(       0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(       0x0020, "Power On" )
	PORT_DIPNAME(          0x0040, 0x0000, "Game Password" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(       0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(       0x0040, "Power On" )
	PORT_DIPNAME(          0x0080, 0x0000, "Connector" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(       0x0000, "Joystick" ) // hardcoded
	PORT_DIPSETTING(       0x0080, "Joystick" )
INPUT_PORTS_END

static INPUT_PORTS_START( ballch )
	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE(0x02, IP_ACTIVE_LOW)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME( "Start / Stop" )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) // play in test mode
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) // button A in test mode
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) // button B in test mode
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON4 ) // button C in test mode
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	//PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read) // TODO: verify


	// There are 3 8-DIP banks on PCB
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0007, 0x0000, "Main Game Rate" ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0001, "91" )
	PORT_DIPSETTING(      0x0002, "92" )
	PORT_DIPSETTING(      0x0003, "93" )
	PORT_DIPSETTING(      0x0004, "94" )
	PORT_DIPSETTING(      0x0005, "95" )
	PORT_DIPSETTING(      0x0000, "96" )
	PORT_DIPSETTING(      0x0006, "97" )
	PORT_DIPSETTING(      0x0007, "98" )
	PORT_DIPNAME( 0x0008, 0x0000, "Lamp Speed" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( High ) )
	PORT_DIPNAME( 0x0010, 0x0000, "Score Feature" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, "Play Score" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, "Title" ) PORT_DIPLOCATION("SW1:7") // enables / disables the title screen, if disabled attract is always running
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:8") // not shown in test mode
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0007, 0x0000, "Coin Rate" ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0001, "5" )
	PORT_DIPSETTING(      0x0002, "10" )
	PORT_DIPSETTING(      0x0003, "25" )
	PORT_DIPSETTING(      0x0004, "50" )
	PORT_DIPSETTING(      0x0005, "75" )
	PORT_DIPSETTING(      0x0006, "100" )
	PORT_DIPSETTING(      0x0007, "500" )
	PORT_DIPNAME( 0x0038, 0x0000, "Key In Rate" ) PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0008, "5" )
	PORT_DIPSETTING(      0x0010, "10" )
	PORT_DIPSETTING(      0x0018, "25" )
	PORT_DIPSETTING(      0x0020, "50" )
	PORT_DIPSETTING(      0x0028, "100" )
	PORT_DIPSETTING(      0x0030, "500" )
	PORT_DIPSETTING(      0x0038, "1000" )
	PORT_DIPNAME( 0x0040, 0x0000, "Key Out Meter" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, "Every 100" )
	PORT_DIPSETTING(      0x0000, "By Coin" )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8") // not shown in test mode
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0003, 0x0000, "Min. Bet" ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(      0x0001, "1" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPSETTING(      0x0002, "8" )
	PORT_DIPSETTING(      0x0003, "16" )
	PORT_DIPNAME( 0x000c, 0x0000, "Max. Bet" ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(      0x0000, "16" )
	PORT_DIPSETTING(      0x0004, "32" )
	PORT_DIPSETTING(      0x0008, "64" )
	PORT_DIPSETTING(      0x000c, "80" )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:5") // not shown in test mode
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:6") // not shown in test mode
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:7") // not shown in test mode
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:8") // not shown in test mode
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cots )
	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE(0x02, IP_ACTIVE_LOW)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_SLOT_STOP4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SLOT_STOP1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_SLOT_STOP2 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SLOT_STOP3 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	//PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read) // TODO: verify


	// There are 3 8-DIP banks on PCB, but settings seem to be selected via test mode?
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout rbmk32_layout =
{
	8,32,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 4,0,12,8,20,16,28,24},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, 8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32,
		16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32, 24*32, 25*32, 26*32, 27*32, 28*32, 29*32, 30*32, 31*32 },
	32*32
};

static const gfx_layout magslot16_layout = // TODO: not correct
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{8, 9,10, 11, 0, 1, 2, 3  },
	{ 0, 4, 16, 20, 32, 36, 48, 52,
	64+0,64+4,64+16,64+20,64+32,64+36,64+48,64+52},
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
	512+0*16,512+1*64,512+2*64,512+3*64,512+4*64,512+5*64,512+6*64,512+7*64},
	32*64
};


static GFXDECODE_START( gfx_rbmk )
	GFXDECODE_ENTRY( "gfx1", 0, rbmk32_layout,            0x0, 16  )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x4_packed_lsb,   0x100, 16  )
GFXDECODE_END

static GFXDECODE_START( gfx_magslot )
	GFXDECODE_ENTRY( "gfx1", 0, magslot16_layout,         0x000, 16  )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x4_packed_lsb,     0x100, 16  )
	GFXDECODE_ENTRY( "gfx3", 0, gfx_8x8x4_packed_lsb,     0x100, 16  ) // wrong colors
GFXDECODE_END

void gms_2layers_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gms_2layers_state::get_tile0_info)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gms_2layers_state::get_tile1_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_tilemap[1]->set_transparent_pen(0);

	save_item(NAME(m_tilebank));
	save_item(NAME(m_mux_data));
	save_item(NAME(m_input_matrix));
}

void gms_3layers_state::video_start()
{
	gms_2layers_state::video_start();

	m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gms_3layers_state::get_tile2_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_tilemap[2]->set_transparent_pen(0);
}

TILE_GET_INFO_MEMBER(gms_2layers_state::get_tile0_info)
{
	const int tile = m_vidram[0][tile_index];
	tileinfo.set(0, (tile & 0x0fff) + ((m_tilebank & 0x10) >> 4) * 0x1000, tile >> 12, 0);
}

TILE_GET_INFO_MEMBER(gms_2layers_state::get_tile1_info)
{
	const int tile = m_vidram[1][tile_index];
	tileinfo.set(1, (tile & 0x0fff) + ((m_tilebank >> 1) & 3) * 0x1000, tile >> 12, 0);
}

TILE_GET_INFO_MEMBER(gms_3layers_state::get_tile2_info)
{
	const int tile = m_vidram[2][tile_index];
	tileinfo.set(2, (tile & 0x0fff) + ((m_tilebank >> 9) & 3) * 0x1000, tile >> 12, 0);
}

// TODO:  ballch's and cots' title screens highlight a priority bug: the title and copyright are drawn behind the background
uint32_t gms_2layers_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap[0]->draw(screen, bitmap, cliprect);
	m_tilemap[1]->draw(screen, bitmap, cliprect);

	return 0;
}

uint32_t gms_3layers_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap[0]->draw(screen, bitmap, cliprect);
	m_tilemap[1]->draw(screen, bitmap, cliprect);
	m_tilemap[2]->draw(screen, bitmap, cliprect);

	return 0;
}


void gms_2layers_state::rbmk(machine_config &config)
{
	M68000(config, m_maincpu, 22_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &gms_2layers_state::rbmk_mem);
	m_maincpu->set_vblank_int("screen", FUNC(gms_2layers_state::irq1_line_hold));

	AT89C4051(config, m_mcu, 22_MHz_XTAL / 4); // frequency isn't right
	m_mcu->set_addrmap(AS_PROGRAM, &gms_2layers_state::mcu_mem);
	m_mcu->set_addrmap(AS_IO, &gms_2layers_state::mcu_io);
	m_mcu->port_out_cb<3>().set(FUNC(gms_2layers_state::mcu_io_mux_w));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_rbmk);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(58);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(gms_2layers_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x800);

	EEPROM_93C46_16BIT(config, m_eeprom);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	okim6295_device &oki(OKIM6295(config, "oki", 22_MHz_XTAL / 20, okim6295_device::PIN7_HIGH)); // pin 7 not verified
	oki.add_route(ALL_OUTPUTS, "lspeaker", 0.47);
	oki.add_route(ALL_OUTPUTS, "rspeaker", 0.47);

	YM2151(config, m_ymsnd, 22_MHz_XTAL / 8);
	m_ymsnd->add_route(0, "lspeaker", 0.60);
	m_ymsnd->add_route(1, "rspeaker", 0.60);
}

void gms_2layers_state::rbspm(machine_config &config)
{
	rbmk(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &gms_2layers_state::rbspm_mem);

	// PIC16F84 but no CPU core available
}

void gms_2layers_state::super555(machine_config &config)
{
	rbmk(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &gms_2layers_state::super555_mem);

	config.device_remove("mcu");
	config.device_remove("ymsnd");
}

void gms_3layers_state::magslot(machine_config &config)
{
	super555(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &gms_3layers_state::magslot_mem);

	subdevice<screen_device>("screen")->set_screen_update(FUNC(gms_3layers_state::screen_update));

	m_gfxdecode->set_info(gfx_magslot);
}


// 实战麻将王 (Shízhàn Májiàng Wáng)
ROM_START( rbmk )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD( "p1.u64", 0x00000, 0x80000, CRC(83b3c505) SHA1(b943d7312dacdf46d4a55f9dc3cf92e291c40ce7) )

	ROM_REGION( 0x1000, "mcu", 0 ) // protected MCU?
	ROM_LOAD( "89c51.bin", 0x0, 0x1000, CRC(c6d58031) SHA1(5c61ce4eef1ef29bd870d0678bdba24e5aa43eae) )

	ROM_REGION( 0x20000, "user1", 0 ) // ??? MCU data / code
	ROM_LOAD( "b1.u72", 0x00000, 0x20000,  CRC(1a4991ac) SHA1(523b58caa21b4a073c664c076d2d7bb07a4253cd) )

	ROM_REGION( 0x040000, "oki", 0 )
	ROM_LOAD( "s1.u83", 0x00000, 0x40000, CRC(44b20e47) SHA1(54691af73aa5d20f9a9afe145447ef1cf34c9a0c) )

	ROM_REGION( 0x100000, "gfx1", 0 ) // 8x32 tiles, lots of girls etc.
	ROM_LOAD( "a1.u41", 0x00000, 0x100000,  CRC(1924de6b) SHA1(1a72ee2fd0abca51893f0985a591573bfd429389) )

	ROM_REGION( 0x80000, "gfx2", 0 ) // 8x8 tiles? cards etc
	ROM_LOAD( "t1.u39", 0x00000, 0x80000, CRC(adf67429) SHA1(ab03c7f68403545f9e86a069581dc3fc3fa6b9c4) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c46.u51", 0x00, 0x080, CRC(4ca6ff01) SHA1(66c456eac5b0d1176ef9130baf2e746efdf30152) )
ROM_END

/*
实战頂凰麻雀 (Shízhàn Dǐng Huáng Máquè)
Gameplay videos:
http://youtu.be/pPk-6N1wXoE
http://youtu.be/VGbrR7GfDck
*/

ROM_START( rbspm )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD( "mj-dfmj-p1.bin", 0x00000, 0x80000, CRC(8f81f154) SHA1(50a9a373dec96b0265907f053d068d636bdabd61) )

	ROM_REGION( 0x1000, "mcu", 0 ) // protected MCU
	ROM_LOAD( "mj-dfmj_at89c51.bin", 0x0000, 0x1000, CRC(c6c48161) SHA1(c3ecf998820d758286b18896ff7860221dd0cf43) ) // decapped

	ROM_REGION( 0x880, "pic", 0 ) // pic was populated on this board
	ROM_LOAD( "c016_pic16f84_code.bin", 0x000, 0x800, CRC(1eb5cd2b) SHA1(9e747235e39eaea337f9325fa55fbfec1c03168d) )
	ROM_LOAD( "c016_pic16f84_data.bin", 0x800, 0x080, CRC(ee882e11) SHA1(aa5852a95a89b17270bb6f315dfa036f9f8155cf) )

	ROM_REGION( 0x20000, "user1", 0 ) // ??? MCU data / code
	ROM_LOAD( "mj-dfmj-2.2-xx.bin", 0x00000, 0x20000,  CRC(58a9eea2) SHA1(1a251e9b049bc8dafbc0728b3d876fdd5a1c8dd9) )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "mj-dfmj-2.2-s1.bin", 0x00000, 0x80000, CRC(2410bb61) SHA1(54e258e4af089841a63e45f25aad70310a28d76b) )  // 1st and 2nd half identical

	ROM_REGION( 0x80000, "gfx1", 0 ) // 8x32 tiles, lots of girls etc.
	ROM_LOAD( "mj-dfmj-4.2-a1.bin", 0x00000, 0x80000,  CRC(b0a3a866) SHA1(cc950532160a066fc6ce427f6df9d58ee4589821) )

	ROM_REGION( 0x80000, "gfx2", 0 ) // 8x8 tiles? cards etc
	ROM_LOAD( "mj-dfmj-4.8-t1.bin", 0x00000, 0x80000, CRC(2b8b689d) SHA1(65ab643fac1e734af8b3a86caa06b532baafa0fe) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c46.u51", 0x00, 0x080, NO_DUMP )
ROM_END


ROM_START( super555 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD( "super555-v1.5e-0d9b.u64", 0x00000, 0x80000, CRC(9a9c16cc) SHA1(95609dbd45feb591190a2b62dee8846cdcec3462) )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "pk-s1-s06.u83", 0x00000, 0x80000, CRC(e329b9ce) SHA1(9fc31daaacc7b3a1a1cf99ab30035021b7cbb78f) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "pk-a1-a09.u41", 0x00000, 0x80000, CRC(f48e74bd) SHA1(68e2a0384964e04c526e4002ffae5fa4f2835d66) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "super555-t1-e67d.u39", 0x00000, 0x80000,  CRC(ee092a9c) SHA1(4123d45d21ca60b0d38f36f59353c56d4fdfcddf) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c46.u138", 0x00, 0x080, CRC(60407223) SHA1(10f766b5431709ab11b16bf5ad7adbfdced0e7ac) )
ROM_END


ROM_START( sc2in1 ) // Basically same PCB as magslot, but with only 1 dip bank. Most labels have been covered with other labels with 'TETRIS' hand-written
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD( "u64", 0x00000, 0x80000, CRC(c0ad5df0) SHA1(a51f30e76493ea9fb5313c0064dac9a2a4f70cc3) )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "u83", 0x00000, 0x80000, CRC(d7ff589b) SHA1(38e61dd7509862dec1299708da8785d1df713fe9) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "u178", 0x000000, 0x200000,  CRC(eaceb446) SHA1(db312f555e060eea6450f506cbbdca8874a05d58) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "u41", 0x00000, 0x40000, CRC(9ea462f7) SHA1(8cec497691f0121693a482b452ddf7a7dcedaf87) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "u169", 0x00000, 0x80000, CRC(f442fa70) SHA1(d06a84080e0196e1917b6f942adc29f97314be58) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "is93c46.u136", 0x00, 0x080, CRC(f0552ce8) SHA1(2dae746d9808d8a37f4f928dedda500063efdcfe) )
ROM_END


// the PCB is slightly different from the others, both layout-wise and component-wise, but it's mostly compatible. It seems to use one more GFX layer and not to have the 89C51.
ROM_START( magslot ) // All labels have SLOT canceled with a black pen. No sum matches the one on label.
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD( "magic 1.0c _ _ _ _.u64", 0x00000, 0x80000, CRC(84544dd7) SHA1(cf10ad3373c2f35f5fa7986be0865f760a454c28) ) // no sum on label, 1xxxxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "magic s1.0c ba8d.u83", 0x00000, 0x80000, CRC(46df3564) SHA1(6b740ca1fd839f7e7e35f097457e87d1260a6aaf) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "magic a1.0c _ _ _ _.u178", 0x000000, 0x200000,  CRC(11028627) SHA1(80b38acab1cd12462d8fc36a9cdce5e5e76f6403) ) // no sum on label, 1xxxxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "magic t1.0c ec43.u41", 0x00000, 0x80000, CRC(18df608d) SHA1(753b8090e8fd89e50131a22259ef3280d7e6b282) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "magic u1.0c f7f6.u169", 0x00000, 0x40000, CRC(582631d3) SHA1(92d1b767bc7ef15eed6dad599392c17620210678) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "is93c46.u136", 0x00, 0x080, CRC(47ef702d) SHA1(269f3aff70cbf5144795b77953eb582d8c4da22a) )
ROM_END

/*
Creatures of the Sea
(c) 2005 ECM

GMS based 8-liner PCB

Major components:

  CPU: MC68HC00F16
Sound: OKI 6295
  OSC: 22.00MHz
EEPOM: ISSI 93C46
  DSW: 3 x 8-position switches
  BAT: 3.6v Varta battery

GMS branded chips:
 GMS-A201
 GMS-A202
 GMS-A89

GMS protection device labeled F.M.  200 (other COTS boards labeled FISHING M)

ROMs not labeled
*/

ROM_START( cots )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "3.u64", 0x00000, 0x80000, CRC(5a1a70d8) SHA1(356d93edd6af4bef72c3d613059a6658c9342d28) )

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "cos_s1_.u83", 0x00000, 0x80000, CRC(e236a02d) SHA1(21361739c2d9b62249dfccc176638a6f375c313c) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "1_a1_.u41", 0x000000, 0x100000, CRC(0ca98ccd) SHA1(45f4c8a93d387f2790fee46c05597628ff238c2d) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "2_t1_.u39", 0x00000, 0x80000,  CRC(8c85dbc7) SHA1(c860949e5a61a4426b1409cefde9651c1d3a2765) )
ROM_END

/*
BALL CHALLENGE
(c) 2002 TVE

GMS based 8-liner PCB

Major components:

  CPU: MC68HC00F16
Sound: OKI 6295
  OSC: 22.00MHz
EEPOM: ISSI 93C46
  DSW: 3 x 8-position switches
  BAT: 3.6v Varta battery

GMS branded chips:
 GMS-A201
 GMS-A202
 GMS-A89

GMS protection device labeled B.CHALLENGE

ROMs labeled as:

ST 27C4002 @ U64:
 B.CHALLENGE
 0607 C757

ST 27C4001 @ U39:
 B.CHALLENGE
 T1 F4CB

ST 27C801 @ U41:
 B.CHALLENGE
 A1 0179

ST 27C4001 @ U83:
 B.CHALLENGE
 S1 15EF
*/

ROM_START( ballch )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "b.challenge_0607_c757.u64", 0x00000, 0x80000, CRC(d7c507e0) SHA1(482c06afb1ffeae99d43a1f4f50cbcd5f231c9bf) ) // 1xxxxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "b.challenge_s1_15ef.u83", 0x00000, 0x80000, CRC(39c3bc0f) SHA1(1a0299f7774f7c95ee43858cf4f12b22eb652f02) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "b.challenge_a1_0179.u41", 0x000000, 0x100000, CRC(b3c49a74) SHA1(a828fd007443ee08ece0c4cad80bd4f84471bb49) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "b.challenge_t1_f4cb.u39", 0x00000, 0x80000,  CRC(a401072a) SHA1(f80ed4ef873393c36bb0446445bfb3a45e3efb97) )
ROM_END


	// the following inits patch out protection (?) checks to allow for testing
	// unfortunately the various U errors shown don't correspond to correct PCB locations

void gms_2layers_state::init_rbspm()
{
	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();

	rom[0x00520 / 2] = 0x600a;
	rom[0x00772 / 2] = 0x4e71;
	rom[0x00774 / 2] = 0x4e71;
	rom[0x1f1fc / 2] = 0x6000;
}

void gms_3layers_state::init_sc2in1()
{
	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();

	rom[0x45f48 / 2] = 0xff84;
}

void gms_2layers_state::init_super555()
{
	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();

	rom[0x46f54 / 2] = 0x6000; // loops endlessly after ROM / RAM test
	rom[0x474b4 / 2] = 0x4e71; // 0x09 U64 ERROR
	rom[0x4782e / 2] = 0x6000; // 0x0A U135 ERROR
}

void gms_2layers_state::init_ballch()
{
	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();

	rom[0x12f0c / 2] = 0x4e71; // U135 ERROR
	rom[0x13212 / 2] = 0x4e71; // U64 U136 ERROR
}

void gms_2layers_state::init_cots()
{
	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();

	rom[0x186c8 / 2] = 0x6000; // U64 U136 ERROR
	rom[0x198f6 / 2] = 0x62fe; // "
	rom[0x19566 / 2] = 0x62fe; // A88 ERROR U135 ERROR

	// the password to enter test mode is all Start
}

} // anonymous namespace


// mahjong
GAME( 1998, rbmk,     0, rbmk,     rbmk,     gms_2layers_state, empty_init,    ROT0,  "GMS", "Shizhan Majiang Wang (Version 8.8)",        MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // misses YM2151 hookup, Oki hookup may be imperfect
GAME( 1998, rbspm,    0, rbspm,    rbspm,    gms_2layers_state, init_rbspm,    ROT0,  "GMS", "Shizhan Ding Huang Maque (Version 4.1)",    MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // stops during boot, patched for now

// card games
GAME( 1999, super555, 0, super555, super555, gms_2layers_state, init_super555, ROT0,  "GMS", "Super 555 (English version V1.5)",          MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // stops during boot, patched for now
GAME( 2001, sc2in1,   0, magslot,  sc2in1,   gms_3layers_state, init_sc2in1,   ROT0,  "GMS", "Super Card 2 in 1 (English version 03.23)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // stops during boot, patched for now

// slot, on slightly different PCB
GAME( 2003, magslot,  0, magslot,  magslot,  gms_3layers_state, empty_init,    ROT0,  "GMS", "Magic Slot (normal 1.0C)",                  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // needs implementing of 3rd GFX layer, correct GFX decode for 1st layer, inputs

// train games
GAME( 2002, ballch,   0, super555, ballch,   gms_2layers_state, init_ballch,   ROT0,  "TVE", "Ball Challenge (20020607 1.0 OVERSEA)",     MACHINE_IMPERFECT_GRAPHICS | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // stops during boot, patched for now
GAME( 2005, cots,     0, super555, cots,     gms_2layers_state, init_cots,     ROT0,  "ECM", "Creatures of the Sea (20050328 USA 6.3)",   MACHINE_IMPERFECT_GRAPHICS | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // stops during boot, patched for now
