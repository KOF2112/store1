// license:BSD-3-Clause
// copyright-holders:BUT
/*

Chack'n Pop driver by BUT


Modified by Hau, Chack'n

Chack'n Pop
Taito 1983

PCB Layout
----------

Top board

J1000045A
K1000220A
 |------------------------|
 |                        |
 |             AO4_06.IC27|
 |                        |
 |                        |
 |                        |
 |                        |
 |                        |
 |                        |
 |                        |
 |                        |
|-|                       |
| |              Z80A     |
| |                       |
| | AO4_05.IC3   MSM2128  |
| |                       |
| |                       |
| |                       |
|-|                       |
 |                        |
 |------------------------|
Notes:
      AO4_06.IC23 - Motorola MC68705P5 Micro-controller. Clock 3.000MHz [18/6]
          MSM2128 - 2k x8 SRAM
              Z80 - Clock 3.000MHz [18/6]


Middle Board

J1000043A
K1000218A
M4200367A (sticker)
|-----------------------------------------------------|
| VOL  MB3731                                         |
|             MC14584                416  416        |-|
|                                    416  416        | |
|                    NE555           416  416        | |
|                                    416  416        | |
|    TD62003                         416  416        | |
|                                    416  416        | |
|2                                   416  416        | |
|2                                   416  416        |-|
|W                                                    |
|A                                                   |-|
|Y                                                   | |
|                        AO4_01.28                   | |
|    LM3900                                          | |
|         AY3-8910       AO4_02.27                   | |
|                    S                               | |
|         AY3-8910       AO4_03.26                   | |
|                                                    |-|
|DSWC  DSWA  DSWB        AO4_04.25                    |
|-----------------------------------------------------|
Notes:
           S - Flat cable connector joining to top PCB
         416 - NEC uPC416C 16k x1 DRAM
    AY3-8910 - Clock 1.500MHz [18/12]
       HSync - 15.1430kHz
       VSync - 59.1828Hz


Bottom Board

J1000044A
K1000219A
|-----------------------------------------------------|
|  AO4_07.IC15                                        |
|                                                    |-|
|  AO4_08.IC14                      2114             | |
|                 2114              2114             | |
|                                         AO4_09.IC98| |
|                 2114                               | |
|1                                        AO4_10.IC97| |
|8                                                   | |
|W                                        AO4-11.IC96|-|
|A                                        AO4-12.IC95 |
|Y                                                   |-|
|                                               18MHz| |
|                                                    | |
|                                                    | |
| HM2510     HM2510                                  | |
| HM2510     HM2510                                  | |
| HM2510     HM2510                                  | |
| HM2510     HM2510                                  |-|
| HM2510     HM2510                                   |
|-----------------------------------------------------|
Notes:
      HM2510 - Hitachi HM2510 1k x1 SRAM
        2114 - 1k x4 SRAM

*/

#include "emu.h"
#include "chaknpop.h"

#include "cpu/z80/z80.h"
#include "cpu/m6805/m6805.h"
#include "sound/ay8910.h"
#include "screen.h"
#include "speaker.h"


/***************************************************************************

  Memory Handler(s)

***************************************************************************/

uint8_t chaknpop_state::mcu_status_r()
{
	// bit 0 = when 1, MCU is ready to receive data from main CPU
	// bit 1 = when 1, MCU has sent data to the main CPU
	return
			((CLEAR_LINE == m_bmcu->host_semaphore_r()) ? 0x01 : 0x00) |
			((CLEAR_LINE != m_bmcu->mcu_semaphore_r()) ? 0x02 : 0x00);
}

void chaknpop_state::unknown_port_1_w(uint8_t data)
{
	//logerror("%s: write to unknown port 1: 0x%02x\n", machine().describe_context(), data);
}

void chaknpop_state::unknown_port_2_w(uint8_t data)
{
	//logerror("%s: write to unknown port 2: 0x%02x\n", machine().describe_context(), data);
}

void chaknpop_state::unknown_port_3_w(uint8_t data)
{
	//logerror("%s: write to unknown port 3: 0x%02x\n", machine().describe_context(), data);
}

void chaknpop_state::coinlock_w(uint8_t data)
{
	logerror("%04x: coin lock %sable\n", m_maincpu->pc(), data ? "dis" : "en");
}


/***************************************************************************

  Memory Map(s)

***************************************************************************/

void chaknpop_state::chaknpop_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x8800).rw(m_bmcu, FUNC(taito68705_mcu_device::data_r), FUNC(taito68705_mcu_device::data_w));
	map(0x8801, 0x8801).r(FUNC(chaknpop_state::mcu_status_r));
	map(0x8802, 0x8802).w(FUNC(chaknpop_state::unknown_port_3_w));
	map(0x8804, 0x8805).rw("ay1", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	map(0x8806, 0x8807).rw("ay2", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	map(0x8808, 0x8808).portr("DSWC");
	map(0x8809, 0x8809).portr("P1");
	map(0x880a, 0x880a).portr("SYSTEM");
	map(0x880b, 0x880b).portr("P2");
	map(0x880c, 0x880c).rw(FUNC(chaknpop_state::gfxmode_r), FUNC(chaknpop_state::gfxmode_w));
	map(0x880d, 0x880d).w(FUNC(chaknpop_state::coinlock_w));                              // coin lock out
	map(0x9000, 0x93ff).ram().w(FUNC(chaknpop_state::txram_w)).share("tx_ram");          // TX tilemap
	map(0x9800, 0x983f).ram().w(FUNC(chaknpop_state::attrram_w)).share("attr_ram");      // Color attribute
	map(0x9840, 0x98ff).ram().share("spr_ram"); // sprite
	map(0xa000, 0xbfff).rom();
	map(0xc000, 0xffff).bankrw(m_vram_bank);                               // bitmap plane 1-4
}

/***************************************************************************

  Input Port(s)

***************************************************************************/

static INPUT_PORTS_START( chaknpop )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )  // LEFT COIN
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )  // RIGHT COIN
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_TILT )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Language ) )     PORT_DIPLOCATION("SWC:1")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWC:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Super Chack'n" )     PORT_DIPLOCATION("SWC:3")
	PORT_DIPSETTING(    0x04, "pi" )
	PORT_DIPSETTING(    0x00, "1st Chance" )
	PORT_DIPNAME( 0x08, 0x08, "Endless (Cheat)")        PORT_DIPLOCATION("SWC:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Credit Info" )       PORT_DIPLOCATION("SWC:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Show Year" )         PORT_DIPLOCATION("SWC:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Infinite (Cheat)")       PORT_DIPLOCATION("SWC:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SWC:8")
	PORT_DIPSETTING(    0x00, "1 Way" )
	PORT_DIPSETTING(    0x80, "2 Way" )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x00, "80k and every 100k" )
	PORT_DIPSETTING(    0x01, "60k and every 100k" )
	PORT_DIPSETTING(    0x02, "40k and every 100k" )
	PORT_DIPSETTING(    0x03, "20k and every 100k" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:4,5")
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPNAME( 0x20, 0x00, "Training/Difficulty" )   PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, "Off/Every 10 Min." )
	PORT_DIPSETTING(    0x00, "On/Every 7 Min." )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSWA")
	PORT_DIPNAME(0x0f,  0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SWA:1,2,3,4")
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME(0xf0,  0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SWA:5,6,7,8")
	PORT_DIPSETTING(    0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )
INPUT_PORTS_END


/***************************************************************************

  Machine Driver(s)

***************************************************************************/

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 characters */
	256,    /* 256 characters */
	2,  /* 2 bits per pixel */
	{ 0, 0x2000*8 },    /* the two bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 ,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8    /* every sprite takes 32 consecutive bytes */
};

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	1024,   /* 1024 characters */
	2,  /* 2 bits per pixel */
	{ 0, 0x2000*8 },    /* the two bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static GFXDECODE_START( gfx_chaknpop )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 0,  8 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,   32, 8 )
GFXDECODE_END


void chaknpop_state::machine_start()
{
	m_vram_bank->configure_entries(0, 2, m_vram, 0x4000);

	save_item(NAME(m_gfxmode));
	save_item(NAME(m_flip_x));
	save_item(NAME(m_flip_y));
}

void chaknpop_state::machine_reset()
{
	m_gfxmode = 0;
	m_flip_x = 0;
	m_flip_y = 0;
}

void chaknpop_state::chaknpop(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(18'000'000) / 6);    // Verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &chaknpop_state::chaknpop_map);
	m_maincpu->set_vblank_int("screen", FUNC(chaknpop_state::irq0_line_hold));

	TAITO68705_MCU(config, m_bmcu, XTAL(18'000'000) / 6);    // Verified on PCB
	config.set_maximum_quantum(attotime::from_hz(6000));  // 100 CPU slices per frame - a high value to ensure proper synchronization of the CPUs

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.1828);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(chaknpop_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_chaknpop);
	PALETTE(config, m_palette, FUNC(chaknpop_state::chaknpop_palette), 1024);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &ay1(AY8910(config, "ay1", XTAL(18'000'000) / 12));  // Verified on PCB
	ay1.port_a_read_callback().set_ioport("DSWA");
	ay1.port_b_read_callback().set_ioport("DSWB");
	ay1.add_route(ALL_OUTPUTS, "mono", 0.15);

	ay8910_device &ay2(AY8910(config, "ay2", XTAL(18'000'000) / 12));  // Verified on PCB
	ay2.port_a_write_callback().set(FUNC(chaknpop_state::unknown_port_1_w));   // ??
	ay2.port_b_write_callback().set(FUNC(chaknpop_state::unknown_port_2_w));   // ??
	ay2.add_route(ALL_OUTPUTS, "mono", 0.10);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( chaknpop )
	ROM_REGION( 0xc000, "maincpu", 0 ) // Main CPU
	ROM_LOAD( "ao4_01.ic28", 0x00000, 0x2000, CRC(386fe1c8) SHA1(cca24abfb8a7f439251e7936036475c694002561) )
	ROM_LOAD( "ao4_02.ic27", 0x02000, 0x2000, CRC(5562a6a7) SHA1(0c5d81f9aaf858f88007a6bca7f83dc3ef59c5b5) )
	ROM_LOAD( "ao4_03.ic26", 0x04000, 0x2000, CRC(3e2f0a9c) SHA1(f1cf87a4cb07f77104d4a4d369807dac522e052c) )
	ROM_LOAD( "ao4_04.ic25", 0x06000, 0x2000, CRC(5209c7d4) SHA1(dcba785a697df55d84d65735de38365869a1da9d) )
	ROM_LOAD( "ao4_05.ic3",  0x0a000, 0x2000, CRC(8720e024) SHA1(99e445c117d1501a245f9eb8d014abc4712b4963) )

	ROM_REGION( 0x0800, "bmcu:mcu", 0 ) // 2k for the Motorola MC68705P5 Micro-controller
	ROM_LOAD( "ao4_06.ic23", 0x0000, 0x0800, CRC(9c78c24c) SHA1(f74c7f3ee106e5c45c907e590ec09614a2bc6751) )

	ROM_REGION( 0x4000, "gfx1", 0 )     // Sprite
	ROM_LOAD( "ao4_08.ic14", 0x0000, 0x2000, CRC(5575a021) SHA1(c2fad53fe6a12c19cec69d27c13fce6aea2502f2) )
	ROM_LOAD( "ao4_07.ic15", 0x2000, 0x2000, CRC(ae687c18) SHA1(65b25263da88d30cbc0dad94511869596e5c975a) )

	ROM_REGION( 0x4000, "gfx2", 0 )     // Text
	ROM_LOAD( "ao4_09.ic98", 0x0000, 0x2000, CRC(757a723a) SHA1(62ab84d2aaa9bc1ea5aa9df8155aa3b5a1e93889) )
	ROM_LOAD( "ao4_10.ic97", 0x2000, 0x2000, CRC(3e3fd608) SHA1(053a8fbdb35bf1c142349f78a63e8cd1adb41ef6) )

	ROM_REGION( 0x0800, "proms", 0 )    // Palette
	ROM_LOAD( "ao4-11.ic96", 0x0000, 0x0400, CRC(9bf0e85f) SHA1(44f0a4712c99a715dec54060afb0b27dc48998b4) )
	ROM_LOAD( "ao4-12.ic95", 0x0400, 0x0400, CRC(954ce8fc) SHA1(e187f9e2cb754264d149c2896ca949dea3bcf2eb) )
ROM_END


//  ( YEAR  NAME      PARENT    MACHINE   INPUT     STATE           INIT        MONITOR  COMPANY              FULLNAME       FLAGS )
GAME( 1983, chaknpop, 0,        chaknpop, chaknpop, chaknpop_state, empty_init, ROT0,    "Taito Corporation", "Chack'n Pop", MACHINE_SUPPORTS_SAVE )
