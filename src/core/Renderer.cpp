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

#include "Renderer.h"

// Get the size of a vector in bytes
template<typename T>
size_t vectorSizeOf(const typename std::vector<T>& vec)
{
  return sizeof(T) * vec.size();
}

Renderer::Renderer(QOpenGLWidget *_widget) :
	mTexture(QOpenGLTexture::Target2D), mLogger(_widget), mProgram(_widget),
	mVertexShader(QOpenGLShader::Vertex, _widget), mFragmentShader(QOpenGLShader::Fragment, _widget),
	mVertex(QOpenGLBuffer::VertexBuffer), mIndex(QOpenGLBuffer::IndexBuffer)
{
	mWidget = _widget;

	mGL.initializeOpenGLFunctions();

	mLogger.initialize();
	connect(&mLogger, SIGNAL(messageLogged(QOpenGLDebugMessage)), this, SLOT(messageLogged(QOpenGLDebugMessage)));
	mLogger.startLogging();

	mVertexShader.compileSourceFile(":/shaders/main.vert");
	mFragmentShader.compileSourceFile(":/shaders/main.frag");

	mProgram.addShader(&mVertexShader);
	mProgram.addShader(&mFragmentShader);

	mProgram.bindAttributeLocation("a_position", ShaderProgramAttributes::POSITION);
	mProgram.bindAttributeLocation("a_color", ShaderProgramAttributes::COLOR);
	mProgram.bindAttributeLocation("a_texcoord", ShaderProgramAttributes::TEXCOORD);

	mProgram.link();
	mProgram.bind();

	mProgram.setUniformValue("tex", 0);

	mGL.glEnable(GL_PROGRAM_POINT_SIZE);

	mGL.glEnable(GL_DEPTH_TEST);
	mGL.glDepthFunc(GL_LEQUAL);

	mGL.glDisable(GL_CULL_FACE);

	mGL.glDisable(GL_BLEND);
	mGL.glDisable(GL_TEXTURE_2D);
}

void Renderer::clear()
{
	mGL.glClearColor(0, 0, 0, 0);
	mGL.glClearDepthf(1.0f);
	mGL.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::show()
{
	mWidget->update();
}

void Renderer::reset()
{
	mProjectionMatrix.setToIdentity();
	mViewMatrix.setToIdentity();
	mModelMatrix.setToIdentity();
}

void Renderer::draw(RendererPrimitiveType _type, float _pointSize)
{
	// --- Before Draw ---

	// Vertex Buffer
	if (!mVertex.isCreated()) mVertex.create();

	mVertex.bind();
	mVertex.allocate(mVertexBuffer.data(), vectorSizeOf(mVertexBuffer));

	mProgram.enableAttributeArray(ShaderProgramAttributes::POSITION);
	mProgram.enableAttributeArray(ShaderProgramAttributes::COLOR);
	mProgram.enableAttributeArray(ShaderProgramAttributes::TEXCOORD);

	int vertexStride = sizeof(RendererVertex);

	mProgram.setAttributeBuffer(ShaderProgramAttributes::POSITION, GL_FLOAT, 0, 4, vertexStride);
	mProgram.setAttributeBuffer(ShaderProgramAttributes::COLOR, GL_FLOAT, 4 * sizeof(GLfloat), 4, vertexStride);
	mProgram.setAttributeBuffer(ShaderProgramAttributes::TEXCOORD, GL_FLOAT, (4 * sizeof(GLfloat)) + (4 * sizeof(GLfloat)), 2, vertexStride);

	// Index Buffer
	if (mIndexBuffer.empty())
	{
		for (uint32_t idx = 0; idx < mVertexBuffer.size(); idx++)
			mIndexBuffer.push_back(idx);
	}

	if (!mIndex.isCreated()) mIndex.create();

	mIndex.bind();
	mIndex.allocate(mIndexBuffer.data(), vectorSizeOf(mIndexBuffer));

	// Set Point Size
	mProgram.setUniformValue("pointSize", _pointSize);

	// Bind matrices
	mProgram.setUniformValue("modelMatrix", mModelMatrix);
	mProgram.setUniformValue("projectionMatrix", mProjectionMatrix);
	mProgram.setUniformValue("viewMatrix", mViewMatrix);

	// --- Draw ---
	mGL.glDrawElements(_type, mIndexBuffer.size(), GL_UNSIGNED_INT, NULL);

	// --- After Draw ---
	mVertex.release();
	mIndex.release();
	mVertexBuffer.clear();
	mIndexBuffer.clear();

	if (mTexture.isCreated() && mTexture.isBound()) mTexture.release();
	mGL.glDisable(GL_BLEND);
	mGL.glDisable(GL_TEXTURE_2D);
}

void Renderer::setViewport(uint32_t _x, uint32_t _y, uint32_t _width, uint32_t _height)
{
	mGL.glViewport(_x, _y, _width, _height);
}

void Renderer::bindModelMatrix(QMatrix4x4 _matrix)
{
	mModelMatrix = _matrix;
}

void Renderer::bindProjectionMatrix(QMatrix4x4 _matrix)
{
	mProjectionMatrix = _matrix;
}

void Renderer::bindViewMatrix(QMatrix4x4 _matrix)
{
	mViewMatrix = _matrix;
}

void Renderer::bindVertex(const RendererVertex *_vertex, uint32_t _count)
{
	uint32_t currentOffset = mVertexBuffer.size();

	for (uint32_t idx = 0; idx < _count; idx++)
	{
		mVertexBuffer.push_back(RendererVertex());

		mVertexBuffer[currentOffset + idx] = _vertex[idx];
	}
}

void Renderer::bindIndex(uint32_t *_index, uint32_t _count)
{
	uint32_t currentOffset = mIndexBuffer.size();

	for (uint32_t idx = 0; idx < _count; idx++)
	{
			mIndexBuffer.push_back(_index[idx]);
	}
}

void Renderer::bindTexture(QImage &_image, bool generateMipmaps)
{
	if (mTexture.isCreated()) mTexture.destroy();

	mTexture.create();
	mTexture.setMinificationFilter(QOpenGLTexture::NearestMipMapLinear);
	mTexture.setMagnificationFilter(QOpenGLTexture::Nearest);

	mTexture.setData(
		_image,
		generateMipmaps ? QOpenGLTexture::GenerateMipMaps
	                  : QOpenGLTexture::DontGenerateMipMaps
	);

	mTexture.bind();

	mGL.glEnable(GL_TEXTURE_2D);

	mGL.glEnable(GL_BLEND);
	mGL.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Renderer::bufferVertex(QVector3D _position, QRgba64 _color, QVector2D _texcoord)
{
	mVertexBuffer.push_back(
		RendererVertex {
			{ _position.x(), _position.y(), _position.z(), 1.0f},
			{ _color.red8() / float(UINT8_MAX), _color.green8() / float(UINT8_MAX), _color.blue8() / float(UINT8_MAX), _color.alpha8() / float(UINT8_MAX)},
			{ _texcoord.x(), _texcoord.y() }
		}
	);
}

void Renderer::messageLogged(const QOpenGLDebugMessage &msg)
{
  QString error;

  // Format based on severity
  switch (msg.severity())
  {
  case QOpenGLDebugMessage::NotificationSeverity:
    error += "NOTIFICATION";
    break;
  case QOpenGLDebugMessage::HighSeverity:
    error += "HIGH";
    break;
  case QOpenGLDebugMessage::MediumSeverity:
    error += "MEDIUM";
    break;
  case QOpenGLDebugMessage::LowSeverity:
    error += "LOW";
    break;
  }

  error += "[";

  // Format based on source
#define CASE(c) case QOpenGLDebugMessage::c: error += #c; break
  switch (msg.source())
  {
    CASE(APISource);
    CASE(WindowSystemSource);
    CASE(ShaderCompilerSource);
    CASE(ThirdPartySource);
    CASE(ApplicationSource);
    CASE(OtherSource);
    CASE(InvalidSource);
  }
#undef CASE

  error += "][";

  // Format based on type
#define CASE(c) case QOpenGLDebugMessage::c: error += #c; break
  switch (msg.type())
  {
    CASE(ErrorType);
    CASE(DeprecatedBehaviorType);
    CASE(UndefinedBehaviorType);
    CASE(PortabilityType);
    CASE(PerformanceType);
    CASE(OtherType);
    CASE(MarkerType);
    CASE(GroupPushType);
    CASE(GroupPopType);
  }
#undef CASE

  error += "]";

  qDebug() << qPrintable(error) << qPrintable(msg.message());
}
