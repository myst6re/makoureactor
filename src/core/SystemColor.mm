/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2022 Arzel Jérôme <myst6re@gmail.com>
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include "SystemColor.h"

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
#import <AppKit/NSColor.h>

#define nsToQColor(name) \
	NSColor *color = [NSColor name]; \
	NSColor *tmpColor = [color colorUsingColorSpaceName:NSCalibratedRGBColorSpace]; \
	QColor ret; \
	CGFloat red = 0, green = 0, blue = 0, alpha = 0; \
	[tmpColor getRed:&red green:&green blue:&blue alpha:&alpha]; \
	ret.setRgbF(float(red), float(green), float(blue), float(alpha)); \
	return ret;

QColor SystemColor::red()
{
	nsToQColor(systemRedColor)
}

QColor SystemColor::blue()
{
	nsToQColor(systemBlueColor)
}

QColor SystemColor::green()
{
	nsToQColor(systemGreenColor)
}

QColor SystemColor::orange()
{
	nsToQColor(systemOrangeColor)
}

QColor SystemColor::purple()
{
	nsToQColor(systemPurpleColor)
}

QColor SystemColor::grey()
{
	nsToQColor(systemGrayColor)
}

#endif
