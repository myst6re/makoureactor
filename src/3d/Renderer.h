/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2022 Arzel Jérôme <myst6re@gmail.com>
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

#pragma once

#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLDebugLogger>
#include <QOpenGLFunctions>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>

struct RendererVertex {
	GLfloat position[4]{0.0f, 0.0f, 0.0f, 1.0f};
	GLfloat color[4]{1.0f, 1.0f, 1.0f, 1.0f};
	GLfloat texcoord[2]{0.0f, 0.0f};
};

enum RendererPrimitiveType {
	PT_POINTS = GL_POINTS,
	PT_LINES = GL_LINES,
	PT_LINE_LOOP = GL_LINE_LOOP,
	PT_LINE_STRIP = GL_LINE_STRIP,
	PT_TRIANGLES = GL_TRIANGLES,
	PT_TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
	PT_TRIANGLE_FAN = GL_TRIANGLE_FAN,
	PT_QUADS = GL_QUADS,
	PT_QUAD_STRIP = GL_QUAD_STRIP,
	PT_POLYGON = GL_POLYGON
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

	QOpenGLFunctions mGL;

	QOpenGLShaderProgram mProgram;
	QOpenGLShader mVertexShader;
	QOpenGLShader mFragmentShader;

	QOpenGLVertexArrayObject mVAO;

	QOpenGLBuffer mVertex;
	QOpenGLBuffer mIndex;

	std::vector<RendererVertex> mVertexBuffer;
	std::vector<uint32_t> mIndexBuffer;

	QOpenGLTexture mTexture;
	bool _hasError;
#ifdef QT_DEBUG
	QOpenGLDebugLogger mLogger;
#endif

	QMatrix4x4 mModelMatrix;
	QMatrix4x4 mProjectionMatrix;
	QMatrix4x4 mViewMatrix;
public:
	Renderer(QOpenGLWidget *_widget);

	inline bool hasError() const {
		return _hasError;
	}
	void clear();
	void show();
	void reset();

	void draw(RendererPrimitiveType _type, float _pointSize = 1.0f);

	void setViewport(int32_t _x, int32_t _y, int32_t _width, int32_t _height);

	void bindModelMatrix(const QMatrix4x4 &_matrix);
	void bindProjectionMatrix(const QMatrix4x4 &_matrix);
	void bindViewMatrix(const QMatrix4x4 &_matrix);

	void bindVertex(const RendererVertex *_vertex, uint32_t _count = 1);
	void bindIndex(uint32_t *_index, uint32_t _count = 1);

	void bindTexture(QImage &_image, bool generateMipmaps = false);

	void bufferVertex(const QVector3D &_position, QRgba64 _color, const QVector2D &_texcoord);
#ifdef QT_DEBUG
protected slots:
	void messageLogged(const QOpenGLDebugMessage &msg);
#endif
};
