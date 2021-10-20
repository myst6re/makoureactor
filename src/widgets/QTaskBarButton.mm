/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2021 Arzel Jérôme <myst6re@gmail.com>
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
/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "QTaskBarButton.h"

#ifdef QTASKBAR_APPLE

QTaskBarButton::QTaskBarButton(QWidget *parent) :
    QObject(parent), _minimum(0), _maximum(100),
    _value(0), _state(Invisible)
{
}

QTaskBarButton::~QTaskBarButton()
{
}

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
#import <AppKit/NSDockTile.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSImageView.h>
#import <AppKit/NSCIImageRep.h>
#import <AppKit/NSBezierPath.h>
#import <AppKit/NSColor.h>
#import <Foundation/NSString.h>

typedef NS_ENUM(NSUInteger, ProgressType) {
	Normal,
	Paused,
	Error
};

@interface ApplicationProgressView : NSView {
	int min;
	int max;
	int value;
	ProgressType type;
}

+ (ApplicationProgressView *)sharedProgressView;

- (void)setRangeMin:(int)v1 max:(int)v2;
- (void)setValue:(int)v;
- (void)updateBadge;

@end

static ApplicationProgressView *sharedProgressView = nil;

@implementation ApplicationProgressView

+ (ApplicationProgressView *)sharedProgressView
{
	if (sharedProgressView == nil) {
		sharedProgressView = [[ApplicationProgressView alloc] init];
	}
	return sharedProgressView;
}

- (void)setRangeMin:(int)v1 max:(int)v2
{
	min = v1;
	max = v2;
	[self updateBadge];
}

- (void)setValue:(int)v
{
	value = v;
	[self updateBadge];
}

- (void)setProgressType:(ProgressType)v
{
	type = v;
	[self updateBadge];
}

- (void)updateBadge
{
	[[NSApp dockTile] display];
}

- (void)drawRect:(NSRect)rect
{
	Q_UNUSED(rect)
	NSRect boundary = [self bounds];
	[[NSApp applicationIconImage] drawInRect:boundary
	                                         fromRect:NSZeroRect
	                                         operation:NSCompositingOperationCopy
	                                         fraction:1.0];
	NSRect progressBoundary = boundary;
	progressBoundary.size.height *= 0.13;
	progressBoundary.size.width *= 0.8;
	progressBoundary.origin.x = (NSWidth(boundary) - NSWidth(progressBoundary)) / 2.;
	progressBoundary.origin.y = NSHeight(boundary) * 0.13;
	
	double range = max - min;
	double percent = 0.50;
	if (range != 0.0) {
		percent = (value - min) / range;
	}
	if (percent > 1) {
		percent = 1;
	}
	else if (percent < 0) {
		percent = 0;
	}

	NSRect currentProgress = progressBoundary;
	currentProgress.size.width *= percent;
	[[NSColor blackColor] setFill];
	[NSBezierPath fillRect:progressBoundary];
	switch (type) {
	case ProgressType::Normal:
		[[NSColor lightGrayColor] setFill];
		break;
	case ProgressType::Paused:
		[[NSColor systemOrangeColor] setFill];
		break;
	case ProgressType::Error:
		[[NSColor systemRedColor] setFill];
		break;
	}
	[NSBezierPath fillRect:currentProgress];
	[[NSColor blackColor] setStroke];
	[NSBezierPath strokeRect:progressBoundary];
}

@end

void QTaskBarButton::setOverlayIcon(const QPixmap &pixmap, const QString &text)
{
	Q_UNUSED(pixmap)
	NSString *cocoaString = [[NSString alloc] initWithUTF8String:text.toUtf8().constData()];
	[[NSApp dockTile] setBadgeLabel:cocoaString];
	[cocoaString release];
}

void QTaskBarButton::setState(State state)
{
	if (state == Invisible) {
		[[NSApp dockTile] setContentView:nil];
	} else {
		[[NSApp dockTile] setContentView:[ApplicationProgressView sharedProgressView]];
		[[ApplicationProgressView sharedProgressView] setRangeMin:minimum() max:maximum()];
	}

	switch (state) {
	case Indeterminate:
		[[ApplicationProgressView sharedProgressView] setRangeMin:0 max:0];
		break;
	case Paused:
		[[ApplicationProgressView sharedProgressView] setProgressType:ProgressType::Paused];
		break;
	case Error:
		[[ApplicationProgressView sharedProgressView] setProgressType:ProgressType::Error];
		break;
	default:
		[[ApplicationProgressView sharedProgressView] setProgressType:ProgressType::Normal];
		break;
	}

	[[NSApp dockTile] display];
}

void QTaskBarButton::setValue(int value)
{
	[[ApplicationProgressView sharedProgressView] setRangeMin:minimum() max:maximum()];
	[[ApplicationProgressView sharedProgressView] setValue:value];
}

#else

void QTaskBarButton::setOverlayIcon(const QPixmap &pixmap, const QString &text)
{
	Q_UNUSED(pixmap)
	Q_UNUSED(text)
}

void QTaskBarButton::setState(State state)
{
	Q_UNUSED(state)
}

void QTaskBarButton::setValue(int value)
{
	Q_UNUSED(value)
}

#endif

#endif
