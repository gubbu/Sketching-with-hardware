#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <ranges>
#include <tuple>
#include <chrono>
#include <numeric>
#include <functional>

using namespace std::chrono_literals;

namespace sf {

static Font font_;

//------ rotateVector ------
inline Vector2f rotateVector(const Vector2f& vector, float angleDegrees) {
  Transform rotation;
  rotation.rotate(angleDegrees);
  return rotation.transformPoint(vector);
}

//------ normalizeVector ------
inline Vector2f normalizeVector(const Vector2f& vector) {
  float magnitude = std::sqrt(vector.x * vector.x + vector.y * vector.y);
  if (magnitude == 0) {
    return Vector2f(0.f, 0.f);
  }
  return Vector2f(vector.x / magnitude, vector.y / magnitude);
}

//--------------------------------------------------------------------------------------------------------------
// Object
//--------------------------------------------------------------------------------------------------------------

struct Object : Drawable {
public:
  bool isMouseOver(RenderWindow* pWindow);
  Object* isClicked(RenderWindow* pWindow);

  virtual Object* invoke() { return nullptr; }

  std::shared_ptr<Shape> shape() { return pShape_; }

  virtual void move2(Vector2f pos) { pShape_->setPosition(pos); }
  virtual void move(Vector2f direct) { pShape_->setPosition(pShape_->getPosition() + direct); }
  virtual void turn(bool right) {
    front = right ? rotateVector(front, turnSpeed_) : rotateVector(front, -turnSpeed_);
    front = normalizeVector(front);
  }
  virtual void turn90(bool right) {
    front = right ? rotateVector(front, 90) : rotateVector(front, -90);
    front = normalizeVector(front);
  }

  void keyBoardMove(std::vector<std::shared_ptr<Drawable>>* pDrawables);

  void draw(RenderTarget& target, RenderStates states) const override { target.draw(*pShape_, states); }

  virtual bool collides(FloatRect rect) { return pShape_->getGlobalBounds().intersects(rect); }

  float x() const { return pShape_->getPosition().x; }
  float y() const { return pShape_->getPosition().y; }

  Vector2f front{0, -1};

protected:
  std::shared_ptr<Shape> pShape_{nullptr};
  float turnSpeed_{0.03f};
  float speed_{0.1f};
};

//--------------------------------------------------------------------------------------------------------------
// Clickable
//--------------------------------------------------------------------------------------------------------------

struct Clickable : Object {
  Clickable(std::function<Object*(Object*)> onClick) : onClick_(onClick) {}

  Object* invoke() override;
  virtual bool delay() const { return false; }

private:
  std::function<Object*(Object*)> onClick_;
};

//--------------------------------------------------------------------------------------------------------------
// Wall
//--------------------------------------------------------------------------------------------------------------

struct Wall : public Clickable {
  Wall(Vector2f pos, bool horizontal, std::function<Object*(Object*)> lambda) : Clickable(lambda) {
    if (horizontal)
      Clickable::pShape_ = std::make_shared<RectangleShape>(Vector2f{20, 200});
    else
      Clickable::pShape_ = std::make_shared<RectangleShape>(Vector2f{200, 20});

    Clickable::pShape_->setFillColor(Color::Black);
    Clickable::pShape_->setOrigin(Clickable::pShape_->getLocalBounds().width / 2.0f,
      Clickable::pShape_->getLocalBounds().height / 2.0f);
    Clickable::pShape_->setPosition(pos);
  }
};

//--------------------------------------------------------------------------------------------------------------
// GenerateDrawable
//--------------------------------------------------------------------------------------------------------------

struct GenerateDrawable : public Clickable {
  GenerateDrawable(Vector2f pos, Vector2f size, std::function<Object*(Object*)> lambda, const std::string& text);

  void draw(RenderTarget& target, RenderStates states) const override {
    Object::draw(target, states);
    target.draw(text_);
  }

  bool delay() const override;

private:
  Text text_;
};

//--------------------------------------------------------------------------------------------------------------
// DistanceSensor
//--------------------------------------------------------------------------------------------------------------

struct DistanceSensor : public Object {
  DistanceSensor() = default;

  DistanceSensor(Vector2f direct, Vector2f pos, int radius, std::vector<std::shared_ptr<Drawable>>* pWalls);

  DistanceSensor& operator = (const DistanceSensor& rhs);

  float measureDistance() const;

  void draw(RenderTarget& target, RenderStates states) const override { Object::draw(target, states); }

  std::optional<float> rectangleDistance(const RectangleShape& rectangle) const;
private:

  std::vector<std::shared_ptr<Drawable>>* pWalls_;
};

//--------------------------------------------------------------------------------------------------------------
// Car
//--------------------------------------------------------------------------------------------------------------

struct Car : public Clickable {
  Car(Vector2f pos, std::function<Object*(Object*)> lambda, std::vector<std::shared_ptr<Drawable>>* pWalls);

  void update(std::vector<std::shared_ptr<Drawable>>* pWalls);

  void draw(RenderTarget& target, RenderStates states) const override;

  void move2(Vector2f pos) override { Object::move2(pos); updateSensorPositions(); }
  void move(Vector2f direct) override {
    Object::move(direct);
    travelledDistance += std::sqrt(direct.x * direct.x + direct.y * direct.y);
    for (auto& sensor : sensors_) sensor.move(direct);
  }

  bool collides(FloatRect rect) override;

  std::vector<Vector2f> edges();

  void turn(bool right) override;
  void turn90(bool right) override;

  const DistanceSensor& distanceSensor(int pos)         { return sensors_[pos]; }
  int getTravelledDistance() { int ret = travelledDistance; travelledDistance = 0; return ret; }

private:
  void updateSensorPositions();
  std::vector<Text> sensorsText_;
  std::vector<DistanceSensor> sensors_; // topRight, topLeft, bottomRight, bottomLeft, topMiddle

  int radiusSensors = 10;
  float travelledDistance{0};
};

//--------------------------------------------------------------------------------------------------------------
// Simulator
//--------------------------------------------------------------------------------------------------------------

struct Simulator {
  Simulator();

  RenderWindow window;

  std::shared_ptr<Drawable> car;

  std::vector<std::shared_ptr<Drawable>> clickables;
  std::vector<std::shared_ptr<Drawable>> walls;
};

} // end of namespace sf


