#ifndef _coffeemaker_deltatime_hpp
#define _coffeemaker_deltatime_hpp

namespace CoffeeMaker {
  class DeltaTime {
    public:
    static void SetDelta();
    static float Value();
    static float CurrentTime;
    static float PreviousTime;

    private:
    static float _Delta;
  };
};  // namespace CoffeeMaker

#endif