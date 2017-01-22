#pragma once

enum class LevelSituation {
    WITHIN_EPSILON,
    ABOVE_TEMPERATURE,
    BELOW_TEMPERATURE
};

//! Abstract base class for different temperature control strategies.
class TemperatureControlStrategy
{
public:
    ~TemperatureControlStrategy();

    TemperatureControlStrategy(float targetTemperature)
        :_targetTemperature(targetTemperature)
    {}

    virtual int computeLevel(float temperature) = 0;

protected:
    LevelSituation evaluateTemperature(float t) const;

private:
    const float _epsilon = 1.0;
    float _targetTemperature = 0.0;
};

//! Simple heat control: temperature too low -> open valve completely, otherwise close completely
//(Would normally go into its own.h and.cpp)
class BinaryHeatControl : public TemperatureControlStrategy {

public:
    using TemperatureControlStrategy::TemperatureControlStrategy;
    virtual int computeLevel(float temperature) override;
};



