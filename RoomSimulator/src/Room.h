#pragma once

#include <assert.h>

// simplistic model of a room with a valve and a constant loss of temperature
class Room
{
public:
    // advances the simulation one step
    void tick() {
        _currentTemperature += _temperatureGainPerSecond - _temperatureLossPerSecond;
    }

    float getTemperature() const {
        return _currentTemperature;
    }
    void setValve(int level) {
        assert(level <= 100 && level >= 0);
        // if valve is at 100%, temperature rises by _maximumTemperatureBoostPerSecond 
        float const _maximumTemperatureBoostPerSecond = 1.0;
        _temperatureGainPerSecond = _maximumTemperatureBoostPerSecond * level*0.01f;
    }
private:
    float _currentTemperature = 25.3f;
    float const _temperatureLossPerSecond = 0.1f;
    float _temperatureGainPerSecond = 0;
};

