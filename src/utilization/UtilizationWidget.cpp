#include "UtilizationWidget.h"

#include <QApplication>
#include <QPainterPath>

#include "core/SettingsManager.h"
#include "core/InfoProvider.h"
#include "core/Utils.h"

using namespace Utils;

UtilizationWidget::UtilizationWidget() {
    graphPoints.resize(InfoProvider::getGPUCount());
    utilizationData.resize(InfoProvider::getGPUCount());
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

UtilizationWidget::~UtilizationWidget() = default;

void UtilizationWidget::onDataUpdated() {
    if (lastTime == 0) {
        lastTime = getTime();
    }

    auto graphStep = (float) SettingsManager::getUpdateDelay() / (float) SettingsManager::getGraphLength();
    auto step = (float)(getTime() - lastTime) / (float) SettingsManager::getUpdateDelay() * graphStep;

    // g means gpu
    for (int g = 0; g < InfoProvider::getGPUCount(); g++) {
        auto &points = graphPoints[g];
        auto &data = utilizationData[g];

        for (PointF &point : points)
            point.x -= step;

        points.emplace_back(1.0f, (float) data.level / 100.0f);

        // deleteSuperfluousPoints(g)
        if (points.size() > 2 && points[0].x < 0 && points[1].x <= 0) {
            points.erase(points.begin());
        }

        // calculate average, min, max
        data.avgLevel = data.maxLevel = 0;
        data.minLevel = 100;

        for (auto &point : points) {
            data.avgLevel += point.y;

            if (data.maxLevel < point.y) {
                data.maxLevel = point.y;
            }

            if (data.minLevel > point.y) {
                data.minLevel = point.y;
            }
        }

        data.avgLevel /= static_cast<int>(points.size());
    }

    lastTime = getTime();
}

void UtilizationWidget::paintEvent(QPaintEvent *) {
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    drawGrid();
    drawGraph();

    painter.end();
}

void UtilizationWidget::drawGrid() {
    const int graphHeight = size().height();
    const int graphWidth = size().width();

    painter.setPen(QColor(100, 100, 100));

    for (int i = 0; i <= 4; ++i) {
        float k = (float) i / 4.0f;
        painter.drawLine(graphWidth * k, 0, graphWidth * k, graphHeight); // vertical
        painter.drawLine(0, graphHeight * k, graphWidth, graphHeight * k); // horizontal
    }
}

void UtilizationWidget::drawGraph() {
    QColor color;
    QPen pen;
    pen.setWidth(2);

    for (int g = 0; g < InfoProvider::getGPUCount(); g++) {
        color = SettingsManager::getGPUColor(g);
        pen.setColor(color);
        painter.setPen(pen);

        auto &points = graphPoints[g];
        QPainterPath filling;

        auto pointPos = [this](PointF point) -> QPoint {
            auto graphHeight = static_cast<float>(size().height());
            auto graphWidth = static_cast<float>(size().width());
            return {
                static_cast<int>(point.x * graphWidth),
                static_cast<int>(graphHeight - graphHeight * point.y)
            };
        };

        filling.moveTo(pointPos(points[0]));

        for (size_t i = 1; i < points.size(); i++)
            filling.lineTo(pointPos(points[i]));

        painter.drawPath(filling);

        filling.lineTo(pointPos({1.0f, 0.0f}));
        filling.lineTo(pointPos({points[0].x, 0.0f}));
        filling.lineTo(pointPos({points[0].x, points[0].y}));

        color.setAlpha(64);
        painter.fillPath(filling, QBrush(color));
    }
}

