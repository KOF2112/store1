// license:BSD-3-Clause
// copyright-holders:Allard van der Bas
/***************************************************************************

    Pooyan

    Original driver by Allard van der Bas

    This hardware is very similar to Time Pilot.

***************************************************************************/

#include "emu.h"
#include "pooyan.h"
#include "konamipt.h"
#include "timeplt_a.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/watchdog.h"
#include "screen.h"
#include "speaker.h"


#define MASTER_CLOCK        XTAL(18'432'000)


/*************************************
 *
 *  Interrupts
 *
 *************************************/

WRITE_LINE_MEMBER(pooyan_state::vblank_irq)
{
	if (state && m_irq_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}


WRITE_LINE_MEMBER(pooyan_state::irq_enable_w)
{
	m_irq_enable = state;
	if (!m_irq_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}


WRITE_LINE_MEMBER(pooyan_state::coin_counter_1_w)
{
	machine().bookkeeping().coin_counter_w(0, state);
}


WRITE_LINE_MEMBER(pooyan_state::coin_counter_2_w)
{
	machine().bookkeeping().coin_counter_w(1, state);
}


/*************************************
 *
 *  Memory maps
 *
 *************************************/

void pooyan_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x83ff).ram().w(FUNC(pooyan_state::colorram_w)).share("colorram");
	map(0x8400, 0x87ff).ram().w(FUNC(pooyan_state::videoram_w)).share("videoram");
	map(0x8800, 0x8fff).ram();
	map(0x9000, 0x90ff).mirror(0x0b00).ram().share("spriteram");
	map(0x9400, 0x94ff).mirror(0x0b00).ram().share("spriteram2");
	map(0xa000, 0xa000).mirror(0x5e7f).portr("DSW1");
	map(0xa080, 0xa080).mirror(0x5e1f).portr("IN0");
	map(0xa0a0, 0xa0a0).mirror(0x5e1f).portr("IN1");
	map(0xa0c0, 0xa0c0).mirror(0x5e1f).portr("IN2");
	map(0xa0e0, 0xa0e0).mirror(0x5e1f).portr("DSW0");
	map(0xa000, 0xa000).mirror(0x5e7f).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0xa100, 0xa100).mirror(0x5e7f).w("timeplt_audio", FUNC(timeplt_audio_device::sound_data_w));
	map(0xa180, 0xa187).mirror(0x5e78).w("mainlatch", FUNC(ls259_device::write_d0));
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( pooyan )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW0")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Invalid", SW1)
	/* Invalid = both coin slots disabled */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "255 (Cheat)" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "50K 80K+" )
	PORT_DIPSETTING(    0x00, "30K 70K+" )
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:5,6,7")
	PORT_DIPSETTING(    0x70, "1 (Easy)" )
	PORT_DIPSETTING(    0x60, "2" )
	PORT_DIPSETTING(    0x50, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x30, "5" )
	PORT_DIPSETTING(    0x20, "6" )
	PORT_DIPSETTING(    0x10, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{ STEP4(0,1), STEP4(8*8,1) },
	{ STEP8(0,8) },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{ STEP4(0,1), STEP4(8*8,1), STEP4(16*8,1), STEP4(24*8,1) },
	{ STEP8(0,8), STEP8(32*8,8) },
	64*8
};


static GFXDECODE_START( gfx_pooyan )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,       0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 16*16, 16 )
GFXDECODE_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void pooyan_state::machine_start()
{
	save_item(NAME(m_irq_enable));
}


void pooyan_state::pooyan(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK/3/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &pooyan_state::main_map);

	ls259_device &mainlatch(LS259(config, "mainlatch")); // B2
	mainlatch.q_out_cb<0>().set(FUNC(pooyan_state::irq_enable_w));
	mainlatch.q_out_cb<1>().set("timeplt_audio", FUNC(timeplt_audio_device::sh_irqtrigger_w));
	mainlatch.q_out_cb<2>().set("timeplt_audio", FUNC(timeplt_audio_device::mute_w));
	mainlatch.q_out_cb<3>().set(FUNC(pooyan_state::coin_counter_1_w));
	mainlatch.q_out_cb<4>().set(FUNC(pooyan_state::coin_counter_2_w));
	mainlatch.q_out_cb<5>().set_nop(); // PAY OUT - not used
	mainlatch.q_out_cb<7>().set(FUNC(pooyan_state::flipscreen_w));

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(pooyan_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(pooyan_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pooyan);
	PALETTE(config, m_palette, FUNC(pooyan_state::pooyan_palette), 16*16+16*16, 32);

	/* sound hardware */
	TIMEPLT_AUDIO(config, "timeplt_audio");
}


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( pooyan )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.4a",         0x0000, 0x2000, CRC(bb319c63) SHA1(5401b8ef586127c8cf5a431e5c44e38be2254a98) )
	ROM_LOAD( "2.5a",         0x2000, 0x2000, CRC(a1463d98) SHA1(b23cc7e61276c61a78e80fe08c7f0c8adadf2ffe) )
	ROM_LOAD( "3.6a",         0x4000, 0x2000, CRC(fe1a9e08) SHA1(5206893760f188ac71a5e6bd42561cf25fcc3d49) )
	ROM_LOAD( "4.7a",         0x6000, 0x2000, CRC(9e0f9bcc) SHA1(4d9707423ad531ac535db432e329b3d52cbb4559) )

	ROM_REGION( 0x10000, "timeplt_audio:tpsound", 0 )
	ROM_LOAD( "xx.7a",        0x0000, 0x1000, CRC(fbe2b368) SHA1(5689a84ef110bdc0039ad1a6c5778e0b8eccfce0) )
	ROM_LOAD( "xx.8a",        0x1000, 0x1000, CRC(e1795b3d) SHA1(9ab4e5362f9f7d9b46b750e14b1d9d71c57be40f) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "8.10g",        0x0000, 0x1000, CRC(931b29eb) SHA1(0325c1c1fdb44e0044b82b7c79b5eeabf5c11ce7) )
	ROM_LOAD( "7.9g",         0x1000, 0x1000, CRC(bbe6d6e4) SHA1(de5447d59a99c4c08c4f40c0b7dd3c3c609c11d4) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "6.9a",         0x0000, 0x1000, CRC(b2d8c121) SHA1(189ad488869f34d7a38b82ef70eb805acfe04312) )
	ROM_LOAD( "5.8a",         0x1000, 0x1000, CRC(1097c2b6) SHA1(c815f0d27593efd23923511bdd13835456ef7f76) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "pooyan.pr1",   0x0000, 0x0020, CRC(a06a6d0e) SHA1(ae131320b66d76d4bc9108da6708f6f874b2e123) ) /* palette */
	ROM_LOAD( "pooyan.pr3",   0x0020, 0x0100, CRC(8cd4cd60) SHA1(e0188ecd5b53a8e6e28c1de80def676740772334) ) /* characters */
	ROM_LOAD( "pooyan.pr2",   0x0120, 0x0100, CRC(82748c0b) SHA1(9ce8eb92e482eba5a9077e9db99841d65b011346) ) /* sprites */
ROM_END

ROM_START( pooyans )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic22_a4.cpu",  0x0000, 0x2000, CRC(916ae7d7) SHA1(e96eba381e6ad228acf4b74240d618f9d0bae39d) )
	ROM_LOAD( "ic23_a5.cpu",  0x2000, 0x2000, CRC(8fe38c61) SHA1(4588f9f80a5884e056a1d429785c7331e92d5654) )
	ROM_LOAD( "ic24_a6.cpu",  0x4000, 0x2000, CRC(2660218a) SHA1(606b10a4bab2432e20471440105e04d15d384570) )
	ROM_LOAD( "ic25_a7.cpu",  0x6000, 0x2000, CRC(3d2a10ad) SHA1(962c621a19e9797b8f3d12c150aa0b90958c9498) )

	ROM_REGION( 0x10000, "timeplt_audio:tpsound", 0 )
	ROM_LOAD( "xx.7a",        0x0000, 0x1000, CRC(fbe2b368) SHA1(5689a84ef110bdc0039ad1a6c5778e0b8eccfce0) )
	ROM_LOAD( "xx.8a",        0x1000, 0x1000, CRC(e1795b3d) SHA1(9ab4e5362f9f7d9b46b750e14b1d9d71c57be40f) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "ic13_g10.cpu", 0x0000, 0x1000, CRC(7433aea9) SHA1(a5ad6311f097fefb6e7b747ebe9d01d72d7755d0) )
	ROM_LOAD( "ic14_g9.cpu",  0x1000, 0x1000, CRC(87c1789e) SHA1(7637a9604a3ad4f9a27105d87252de3d923672aa) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "6.9a",         0x0000, 0x1000, CRC(b2d8c121) SHA1(189ad488869f34d7a38b82ef70eb805acfe04312) )
	ROM_LOAD( "5.8a",         0x1000, 0x1000, CRC(1097c2b6) SHA1(c815f0d27593efd23923511bdd13835456ef7f76) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "pooyan.pr1",   0x0000, 0x0020, CRC(a06a6d0e) SHA1(ae131320b66d76d4bc9108da6708f6f874b2e123) ) /* palette */
	ROM_LOAD( "pooyan.pr3",   0x0020, 0x0100, CRC(8cd4cd60) SHA1(e0188ecd5b53a8e6e28c1de80def676740772334) ) /* characters */
	ROM_LOAD( "pooyan.pr2",   0x0120, 0x0100, CRC(82748c0b) SHA1(9ce8eb92e482eba5a9077e9db99841d65b011346) ) /* sprites */
ROM_END

ROM_START( pootan )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "poo_ic22.bin", 0x0000, 0x2000, CRC(41b23a24) SHA1(366efcc45613391c1ab1514654ecac1ae3d39d0e) )
	ROM_LOAD( "poo_ic23.bin", 0x2000, 0x2000, CRC(c9d94661) SHA1(af1e818335adb4398ea0dc41be0d6399999f3946) )
	ROM_LOAD( "3.6a",         0x4000, 0x2000, CRC(fe1a9e08) SHA1(5206893760f188ac71a5e6bd42561cf25fcc3d49) )
	ROM_LOAD( "poo_ic25.bin", 0x6000, 0x2000, CRC(8ae459ef) SHA1(995eba204bbb82da20063b965bf79a64441a907a) )

	ROM_REGION( 0x10000, "timeplt_audio:tpsound", 0 )
	ROM_LOAD( "xx.7a",        0x0000, 0x1000, CRC(fbe2b368) SHA1(5689a84ef110bdc0039ad1a6c5778e0b8eccfce0) )
	ROM_LOAD( "xx.8a",        0x1000, 0x1000, CRC(e1795b3d) SHA1(9ab4e5362f9f7d9b46b750e14b1d9d71c57be40f) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "poo_ic13.bin", 0x0000, 0x1000, CRC(0be802e4) SHA1(07adc17bcb7332ddc00b7c71bf4919eda80b0bdb) )
	ROM_LOAD( "poo_ic14.bin", 0x1000, 0x1000, CRC(cba29096) SHA1(b5a4cf75089cf04f7361e00074816facd57452b2) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "6.9a",         0x0000, 0x1000, CRC(b2d8c121) SHA1(189ad488869f34d7a38b82ef70eb805acfe04312) )
	ROM_LOAD( "5.8a",         0x1000, 0x1000, CRC(1097c2b6) SHA1(c815f0d27593efd23923511bdd13835456ef7f76) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "pooyan.pr1",   0x0000, 0x0020, CRC(a06a6d0e) SHA1(ae131320b66d76d4bc9108da6708f6f874b2e123) ) /* palette */
	ROM_LOAD( "pooyan.pr3",   0x0020, 0x0100, CRC(8cd4cd60) SHA1(e0188ecd5b53a8e6e28c1de80def676740772334) ) /* characters */
	ROM_LOAD( "pooyan.pr2",   0x0120, 0x0100, CRC(82748c0b) SHA1(9ce8eb92e482eba5a9077e9db99841d65b011346) ) /* sprites */
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

//    YEAR, NAME,    PARENT, MACHINE,INPUT,  INIT,MONITOR,COMPANY,FULLNAME,FLAGS
GAME( 1982, pooyan,  0,      pooyan, pooyan, pooyan_state, empty_init, ROT90,  "Konami", "Pooyan", MACHINE_SUPPORTS_SAVE )
GAME( 1982, pooyans, pooyan, pooyan, pooyan, pooyan_state, empty_init, ROT90,  "Konami (Stern Electronics license)", "Pooyan (Stern Electronics)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, pootan,  pooyan, pooyan, pooyan, pooyan_state, empty_init, ROT90,  "bootleg", "Pootan", MACHINE_SUPPORTS_SAVE )
