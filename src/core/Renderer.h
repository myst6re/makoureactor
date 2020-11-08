/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2012 Arzel Jérôme <myst6re@gmail.com>
 ** Copyright (C) 2020 Julian Xhokaxhiu <https://julianxhokaxhiu.com>
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

#ifndef RENDERER_H
#define RENDERER_H

#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLDebugLogger>
#include <QOpenGLFunctions>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include "field/CaFile.h"
#include "field/IdFile.h"

struct RendererVertex {
	GLfloat position[4]{0.0f, 0.0f, 0.0f, 1.0f};
	GLfloat color[4]{1.0f, 1.0f, 1.0f, 1.0f};
	GLfloat texcoord[2]{0.0f, 0.0f};
};

enum RendererPrimitiveType {
	PT_POINTS = 0,
	PT_LINES,
	PT_LINE_LOOP,
	PT_LINE_STRIP,
	PT_TRIANGLES,
	PT_TRIANGLE_STRIP,
	PT_TRIANGLE_FAN,
	PT_QUADS,
	PT_QUAD_STRIP
};

class Renderer : public QObject
{
	Q_OBJECT
private:
	enum ShaderProgramAttributes {
		POSITION = 0,
		COLOR,
		TEXCOORD
	};

	QOpenGLWidget *mWidget;

	QOpenGLDebugLogger mLogger;

	QOpenGLFunctions mGL;

	QOpenGLShaderProgram mProgram;
	QOpenGLShader mVertexShader;
	QOpenGLShader mFragmentShader;

	QMatrix4x4 mModelMatrix;
	QMatrix4x4 mProjectionMatrix;
	QMatrix4x4 mViewMatrix;

	QOpenGLBuffer mVertex;
	QOpenGLBuffer mIndex;

	std::vector<RendererVertex> mVertexBuffer;
	std::vector<uint32_t> mIndexBuffer;

	QOpenGLTexture mTexture;

public:
	Renderer(QOpenGLWidget *_widget);

	void clear();
	void show();

	void draw(RendererPrimitiveType _type, float _pointSize = 1.0f);

	void setViewport(uint32_t _x, uint32_t _y, uint32_t _width, uint32_t _height);

	void bindModelMatrix(QMatrix4x4 _matrix);
	void bindProjectionMatrix(QMatrix4x4 _matrix);
	void bindViewMatrix(QMatrix4x4 _matrix);

	void bindVertex(const RendererVertex *_vertex, uint32_t _count = 1);
	void bindIndex(uint32_t *_index, uint32_t _count = 1);

	void bindTexture(QImage &_image, bool generateMipmaps = false);

	void bufferVertex(QVector3D _position, QRgba64 _color, QVector2D _texcoord);

protected slots:
	void messageLogged(const QOpenGLDebugMessage &msg);
};

#endif
