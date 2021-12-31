#include "GPUUtilizationContainer.h"

#include <QHBoxLayout>

#include "GPUUtilizationWidget.h"

#include "core/SettingsManager.h"
#include "core/InfoProvider.h"

#define temp 1
#define power 2

GPUUtilizationContainer::GPUUtilizationContainer() {
    utilizationWidget = new GPUUtilizationWidget();
    build("GPU Utilization");

    for (int i = 0; i < SettingsManager::getGPUCount(); i++) {
        addInfoTitleLayout(i);

        auto infoLayout = new QHBoxLayout();
        infoLayout->addWidget(getInfoLabel(i, UtInfoLabelId));

        if (InfoProvider::isGPUTempSupported(i)) {
            infoLayout->addWidget(getInfoLabel(i, temp));
        }

        if (InfoProvider::isPowerSupported(i)) {
            infoLayout->addWidget(getInfoLabel(i, power));
        }

        getLayout()->addLayout(infoLayout);
    }

    updateData();

    setMouseTracking(true);
}

void GPUUtilizationContainer::onDataUpdated() {
    for (int i = 0; i < SettingsManager::getGPUCount(); i++) {
        const auto &data = utilizationWidget->worker->udata[i];

        auto infoLabel = [&](int gpuIndex, int index) {
            return findChild<QLabel*>(getInfoLabelName(gpuIndex, index));
        };

        infoLabel(i, 0)->setText(QString::asprintf("Utilization: %i%% (%i%% / %i%% / %i%%)",
                                                   data.level, data.avgLevel, data.minLevel, data.maxLevel));

        if (auto label = infoLabel(i, temp); label) {
            label->setText(QString::asprintf("Temperature: %i °C", InfoProvider::getGPUTemp(i)));
        }

        if (auto label = infoLabel(i, power); label) {
            infoLabel(i, power)->setText(QString::asprintf("Power: %.2f W", InfoProvider::getPower(i)));
        }
    }

    utilizationWidget->update();
}
