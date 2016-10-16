#include "console_window.h"
#include <halley/core/graphics/sprite/sprite.h>
#include <halley/core/graphics/text/text_renderer.h>
#include <halley/core/graphics/material/material.h>
#include <halley/core/graphics/material/material_definition.h>
#include <halley/core/graphics/material/material_parameter.h>

using namespace Halley;

ConsoleWindow::ConsoleWindow(Resources& resources)
{
	backgroundMaterial = std::make_shared<Material>(resources.get<MaterialDefinition>("distance_field_sprite"));
	auto& mat = *backgroundMaterial;
	mat["tex0"] = resources.get<Texture>("round_rect.png");
	mat["u_smoothness"] = 1.0f / 16.0f;
	mat["u_outline"] = 0.5f;
	mat["u_outlineColour"] = Colour(0.47f, 0.47f, 0.47f);

	font = resources.get<Font>("Inconsolata.font");

	printLn("Welcome to the Halley Game Engine Editor.");
	Logger::addSink(*this);
}

ConsoleWindow::~ConsoleWindow()
{
	Logger::removeSink(*this);
}

void ConsoleWindow::log(LoggerLevel level, const String& msg)
{
	printLn(msg);
}

void ConsoleWindow::update(InputKeyboard& keyboard)
{
	for (int next; (next = keyboard.getNextLetter()) != 0;) {
		if (next == '\b') {
			input.setSize(std::max(0, int(input.size()) - 1));
		} else if (next == '\n') {
			submit();
		} else {
			input.appendCharacter(next);
		}
	}
}

void ConsoleWindow::draw(Painter& painter, Rect4f bounds) const
{
	Rect4f innerBounds = bounds.shrink(12);
	Rect4f outerBounds = bounds.grow(8);

	// Background
	Sprite()
		.setMaterial(backgroundMaterial)
		.setPos(outerBounds.getTopLeft())
		.setSize(Vector2f(64, 64))
		.setTexRect(Rect4f(0, 0, 1, 1))
		.setColour(Colour4f(0.0f, 0.0f, 0.0f, 0.4f))
		.drawSliced(painter, outerBounds.getSize(), Vector4i(28, 28, 28, 28));

	const float size = 18;
	float lineH = font->getLineHeightAtSize(size);
	int nLines = int(innerBounds.getHeight() / lineH) - 1;
	Vector2f cursor = innerBounds.getBottomLeft();

	TextRenderer text;
	text.setFont(font).setSize(size).setOffset(Vector2f(0, 1)).setColour(Colour(1, 1, 1));

	// Draw command
	text.setText("> " + input).draw(painter, cursor);
	cursor += Vector2f(0, -lineH);

	// Draw buffer
	int last = int(buffer.size());
	int nDrawn = 0;
	for (int i = last; --i >= 0 && nDrawn < nLines;) {
		int nLinesHere = 1;
		size_t pos = 0;
		while ((pos = buffer[i].find('\n', pos)) != size_t(-1)) {
			nLinesHere++;
			pos++;
		}
		text.setText(buffer[i]);
		//auto extents = text.getExtents();
		text.draw(painter, cursor);
		cursor += Vector2f(0, -lineH * nLinesHere);
		nDrawn += nLinesHere;
	}
}

void ConsoleWindow::submit()
{
	printLn("> " + input);
	history.push_back(input);
	input = "";
}

void ConsoleWindow::printLn(const String& line)
{
	buffer.push_back(line);
}