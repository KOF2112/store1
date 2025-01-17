// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                        -= Sky Fox / Exerizer =-

                driver by   Luca Elia (l.elia@tin.it)


                            [ 1 Background ]

    The stars in the background are not tile based (I think!) and
    their rendering is entirely guesswork for now..

    I draw a star for each horizontal line using 2 bytes in the
    background rom:

    - the first byte seems a color / shape info
    - the second byte seems a position info

    The rom holds 4 chunks of $2000 bytes. Most of the data does not
    change between chunks, while the remaining part (which is rendered
    to what seems a "milky way") pulsates in color and/or shape
    to simulate the shimmering of stars (?!) if we draw one chunk only
    and cycle through the four. Indeed, there's a register cycling
    through 4 values.

    Since the result kind of matches a screenshot we have, I feel the
    drawn result is not that far from reality. On the other hand we
    have a random arrangement of stars, so it's hard to tell for sure..

                            [ 256 Sprites ]

    Sprites are 8 planes deep and can be 8x8, 16x16 or 32x32 pixels
    in size. They are stored as 32x32x8 tiles in the ROMs.


***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "skyfox.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  There are three 256x4 palette PROMs (one per gun).
  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 110 ohm resistor  -- RED/GREEN/BLUE
        -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 680 ohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 1.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

static constexpr res_net_decode_info skyfox_decode_info =
{
	1,
	0, 255, // start/end
	// R,     G,     B,
	{  0,     0x100, 0x200, }, // offsets
	{  0,     0,     0,     }, // shifts
	{  0xf,   0xf,   0xf,   }  // masks
};

static constexpr res_net_info skyfox_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_TTL_OUT,
	{
		{ RES_NET_AMP_NONE, 0, 0, 4, { 1200, 680, 220, 110 } },
		{ RES_NET_AMP_NONE, 0, 0, 4, { 1200, 680, 220, 110 } },
		{ RES_NET_AMP_NONE, 0, 0, 4, { 1200, 680, 220, 110 } }
	}
};

void skyfox_state::skyfox_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	std::vector<rgb_t> rgb;

	compute_res_net_all(rgb, color_prom, skyfox_decode_info, skyfox_net_info);
	palette.set_pen_colors(0, rgb);
}


/***************************************************************************

                                Sprites Drawing

Offset:         Value:

03              Code: selects one of the 32x32 tiles in the ROMs.
                (Tiles $80-ff are bankswitched to cover $180 tiles)

02              Code + Attr

                    7654 ----   Code (low 4 bits)
                                8x8   sprites use bits 7654 (since there are 16 8x8  tiles in the 32x32 one)
                                16x16 sprites use bits --54 (since there are 4 16x16 tiles in the 32x32 one)
                                32x32 sprites use no bits   (since the 32x32 tile is already selected)

                    7--- 3---   Size
                                1--- 1--- : 32x32 sprites
                                0--- 1--- : 16x16 sprites
                                8x8 sprites otherwise

                    ---- -2--   Flip Y
                    ---- --1-   Flip X
                    ---- ---0   X Low Bit

00              Y

01              X (High 8 Bits)

***************************************************************************/

void skyfox_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	int width = m_screen->width();
	int height = m_screen->height();

	/* The 32x32 tiles in the 80-ff range are bankswitched */
	int shift = (m_bg_ctrl & 0x80) ? (4 - 1) : 4;

	for (int offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int xstart, ystart, xend, yend;
		int xinc, yinc, dx, dy;
		int low_code, high_code, n;

		int code = m_spriteram[offs + 3] << 8 | m_spriteram[offs + 2];
		int flipx = code & 0x2;
		int flipy = code & 0x4;
		int y = m_spriteram[offs + 0];
		int x = m_spriteram[offs + 1] << 1 | (code & 1);

		high_code = ((code >> 4) & 0x7f0) + ((code & 0x8000) >> shift);

		switch (code & 0x88)
		{
			case 0x88:  n = 4; low_code = 0; break;
			case 0x08:  n = 2; low_code = (code & 0x20) >> 2 | (code & 0x10) >> 3; break;
			default:    n = 1; low_code = (code >> 4) & 0xf; break;
		}

		if (m_bg_ctrl & 1) // flipscreen
		{
			x = width  - x - n * 8;
			y = height - y - n * 8;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (flipx)  { xstart = n - 1;  xend = -1;  xinc = -1; }
		else        { xstart = 0;      xend = n;   xinc = +1; }

		if (flipy)  { ystart = n - 1;  yend = -1;  yinc = -1; }
		else        { ystart = 0;      yend = n;   yinc = +1; }


		code = low_code + high_code;

		for (dy = ystart; dy != yend; dy += yinc)
		{
			for (dx = xstart; dx != xend; dx += xinc, code++)
			{
				gfx->transpen(bitmap, cliprect, code, 0, flipx, flipy, dx*8 + x, dy*8 + y, 0xff);

				// wraparound y - BTANB: large sprites exiting the screen sometimes reappear on the other edge
				gfx->transpen(bitmap, cliprect, code, 0, flipx, flipy, dx*8 + x, dy*8 + y - 256, 0xff);
			}

			if (n == 2)
				code += 2;
		}
	}
}



/***************************************************************************

                            Background Rendering

***************************************************************************/

void skyfox_state::draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t *rom = memregion("gfx2")->base();

	/* Blinking stops until the first star moves after turning on the power */
	bool blinking = (m_bg_ctrl & 0x8);
	/* Star pattern change. This will change at when all star clusters to out of screen or when player restart.
	   When it changes, will change color of star clusters. */
	int pattern = (m_bg_ctrl & 0x6) >> 1;

	for (int i = 0; i < 0x1000; i++)
	{
		/* contains the position of stars from 0xd4e0 in RAM */
		int ramoffset = 0xe0 + (i & 0xf) * 2;
		int pos = m_bgram[ramoffset + 1] * 2 + ((m_bgram[ramoffset] & 0x80) ? 1 : 0);

		/* ROM offset of star pattern */
		int offs = (i * 2) % 0x2000 + pattern * 0x2000;

		/* Adjusted with 1 pixel accuracy compared to PCB.
		   Confirmed that pixel and color pattern match in the "1UP START" screen. */
		int pen = rom[offs];
		int x = rom[offs + 1] * 2 + pos + 0x5b;
		int y = (i >> 4) + 1;

		/* When flipscreen is enabled, scroll direction is flipped by only in-game subroutine.
		   This PCB seems does not support background flip. */

		/* This looks perfect at first glance.
		   but when strict compared on "1UP START" screen,
		   it seems the blinking pattern in each star may be different. */
		if (((m_bg_ctrl >> 4) & 3) != (pen & 3) || !blinking)
			bitmap.pix(y % 256, x % 512) = pen;
	}
}


/***************************************************************************

                                Screen Drawing

***************************************************************************/

uint32_t skyfox_state::screen_update_skyfox(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0xff, cliprect); // the bg is black
	draw_background(bitmap, cliprect);
	draw_sprites(bitmap, cliprect);
	return 0;
}
