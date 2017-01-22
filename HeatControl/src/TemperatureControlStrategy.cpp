#include "TemperatureControlStrategy.h"

#include <cmath>

TemperatureControlStrategy::~TemperatureControlStrategy()
{
}

LevelSituation TemperatureControlStrategy::evaluateTemperature(float t) const {
    if (abs(t - _targetTemperature) < _epsilon) {
        return LevelSituation::WITHIN_EPSILON;
    }
    if (t > _targetTemperature) {
        return LevelSituation::ABOVE_TEMPERATURE;
    }
    return LevelSituation::BELOW_TEMPERATURE;
}

int BinaryHeatControl::computeLevel(float temperature) {
    LevelSituation situation = evaluateTemperature(temperature);
    switch (situation) {
    case LevelSituation::ABOVE_TEMPERATURE: return 0;
    case LevelSituation::BELOW_TEMPERATURE: return 100;
    case LevelSituation::WITHIN_EPSILON: return 0;
    }
    return 0;
}

