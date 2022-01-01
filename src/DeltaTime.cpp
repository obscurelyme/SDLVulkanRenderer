#include "DeltaTime.hpp"

void CoffeeMaker::DeltaTime::SetDelta() { _Delta = CurrentTime - PreviousTime; }

float CoffeeMaker::DeltaTime::Value() { return _Delta; }

float CoffeeMaker::DeltaTime::CurrentTime = 0.0f;
float CoffeeMaker::DeltaTime::PreviousTime = 0.0f;
float CoffeeMaker::DeltaTime::_Delta = 0.0f;