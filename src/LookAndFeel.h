#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

using namespace juce;

class ChickenKnobStyle : public LookAndFeel_V3
{
public:
    void drawRotarySlider(Graphics &g, int x, int y, int width, int height, float sliderPos,
                          const float rotaryStartAngle, const float rotaryEndAngle, Slider &slider)
    {
        const float minWidthHeight = jmin(width, height);
        const float radius = minWidthHeight * 0.4f;
        const float innerRadius = radius * 0.7;
        const float centreX = x + width * 0.5f;
        const float centreY = y + height * 0.5f;
        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        auto baseColour = Colours::red;

        const float offset = radius * 0.1;

        auto gradient = ColourGradient(
            baseColour.brighter(0.5), centreX + offset, centreY - offset, baseColour.darker(0.8f), centreX + innerRadius, centreY, true);

        const float dotRadius = minWidthHeight * 0.5f;
        for (int i = 0; i < 12; i++)
        {
            Path pAlso;
            const float angleAlso = rotaryStartAngle + (i / 11.0f) * (rotaryEndAngle - rotaryStartAngle);
            pAlso.addEllipse((-dotRadius * 0.1f) * 0.5f, -dotRadius, dotRadius * 0.1f, dotRadius * 0.1f);
            pAlso.applyTransform(AffineTransform::rotation(angleAlso).translated(centreX, centreY));
            g.setColour(Colour(0x66FFFFFF));
            g.fillPath(pAlso);
        }

        g.setGradientFill(gradient);
        g.fillEllipse(centreX - innerRadius, centreY - innerRadius, innerRadius * 2.0, innerRadius * 2.0);

        g.setColour(baseColour.brighter(0.5f));
        g.fillEllipse(centreX - innerRadius * 0.6, centreY - innerRadius * 0.6, innerRadius * 2.0 * 0.6, innerRadius * 2.0 * 0.6);

        Path p2;
        float p2Thing = radius * 2.0f;
        float x1 = p2Thing * 0.015;
        float y1 = p2Thing * -0.5;
        float x2 = p2Thing * 0.17;
        float y2 = p2Thing * 0.5;
        float x3 = p2Thing * -0.17;
        float y3 = p2Thing * 0.5;
        p2.clear();
        p2.startNewSubPath(x1, y1);
        p2.lineTo(x2, y2);

        float w = x2 - x3;
        float increment = w / 7.0;
        p2.lineTo(x2 - increment, y2 * 1.05);
        p2.lineTo(x2 - (increment * 2), y2 * 1.08);
        p2.lineTo(x2 - (increment * 3), y2 * 1.09);
        p2.lineTo(x2 - (increment * 4), y2 * 1.09);
        p2.lineTo(x2 - (increment * 5), y2 * 1.08);
        p2.lineTo(x2 - (increment * 6), y2 * 1.05);

        p2.lineTo(x3, y3);
        p2.lineTo(p2Thing * -0.015, p2Thing * -0.5);
        p2.closeSubPath();

        p2.applyTransform(AffineTransform::rotation(angle).translated(centreX, centreY));
        g.setColour(baseColour.brighter(0.4f));
        g.fillPath(p2);

        const float shadowStart = 0.30f;
        const float shadowEnd = 0.67f;

        Path p3;
        p3.clear();
        p3.startNewSubPath(x1 + ((x2 - x1) * shadowStart), y1 + ((y2 - y1) * shadowStart));
        p3.lineTo(x1 + ((x2 - x1) * shadowEnd), y1 + ((y2 - y1) * shadowEnd));
        p3.lineTo(x1 + ((x2 - x1) * shadowEnd) + 1, y1 + ((y2 - y1) * shadowEnd));
        p3.lineTo(x1 + ((x2 - x1) * shadowStart) + 1, y1 + ((y2 - y1) * shadowStart));
        p3.closeSubPath();
        p3.applyTransform(AffineTransform::rotation(angle).translated(centreX, centreY));
        g.setColour(Colour(0.0f, 0.0f, 0.0f, 0.3f));
        g.fillPath(p3);

        x1 = p2Thing * -0.015;
        y1 = p2Thing * -0.5;
        x2 = p2Thing * -0.17;
        y2 = p2Thing * 0.5;

        Path p4;
        p4.clear();
        p4.startNewSubPath(x1 + ((x2 - x1) * shadowStart), y1 + ((y2 - y1) * shadowStart));
        p4.lineTo(x1 + ((x2 - x1) * shadowEnd), y1 + ((y2 - y1) * shadowEnd));
        p4.lineTo(x1 + ((x2 - x1) * shadowEnd) - 1, y1 + ((y2 - y1) * shadowEnd));
        p4.lineTo(x1 + ((x2 - x1) * shadowStart) - 1, y1 + ((y2 - y1) * shadowStart));
        p4.closeSubPath();
        p4.applyTransform(AffineTransform::rotation(angle).translated(centreX, centreY));
        g.setColour(Colour(0.0f, 0.0f, 0.0f, 0.3f));
        g.fillPath(p4);

        Path p;
        const float pointerLength = radius * 0.33f;
        const float pointerThickness = 2.0f;
        p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform(AffineTransform::rotation(angle).translated(centreX, centreY));

        g.setColour(Colours::white);
        g.fillPath(p);
    }
};