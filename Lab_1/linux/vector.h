#ifndef VECTOR_H
#define VECTOR_H

#include <cassert>
#include <cmath>
#include "number.h"

class Vector {
public:
    Vector() : x_(Number::ZERO), y_(Number::ZERO) {
    }

    Vector(const Number &x, const Number &y) : x_(x), y_(y) {
    }

    Vector(const Vector &v) = default;

    /// \brief Длина вектора (тета)
    [[nodiscard]] Number getTheta() const {
        return Number(std::atan2(y_.getValue(), x_.getValue()));
    }

    /// \brief Угол вектора в радианах (ро)
    [[nodiscard]] Number getRho() const {
        return Number(std::sqrt((x_ * x_ + y_ * y_).getValue()));
    }

    [[nodiscard]] Number dot(const Vector &v) const {
        return x_ * v.x_ + y_ * v.y_;
    }

    [[nodiscard]] Number cross(const Vector &v) const {
        return x_ * v.y_ - y_ * v.x_;
    }

    [[nodiscard]] Vector componentWiseMul(const Vector &v) const {
        return {x_ * v.x_, y_ * v.y_};
    }

    [[nodiscard]] Vector componentWiseDiv(const Vector &v) const {
        assert(v.x_ != Number::ZERO && "Vec2::componentWiseDiv cannot divide by 0");
        assert(v.y_ != Number::ZERO && "Vec2::componentWiseDiv cannot divide by 0");
        return {x_ / v.x_, y_ / v.y_};
    }

    [[nodiscard]] Number len_sq() const {
        return dot(*this);
    }

    [[nodiscard]] Vector rotate(const Number angle) const {
        const double c = std::cos(angle.getValue());
        const double s = std::sin(angle.getValue());
        return {x_ * Number(c) - y_ * Number(s), x_ * Number(s) + y_ * Number(c)};
    }

    [[nodiscard]] Vector operator-() const {
        return {-x_, -y_};
    }

    /// \brief Перегрузка бинарного оператора +
    ///
    /// \param v Правый операнд (a Vector)
    ///
    /// \return Почленное сложение параметров двух векторов
    [[nodiscard]] Vector operator+(const Vector &v) const {
        return {x_ + v.x_, y_ + v.y_};
    }

    Vector &operator+=(const Vector &v) {
        x_ += v.x_;
        y_ += v.y_;
        return *this;
    }

    Vector &operator-=(const Vector &v) {
        x_ -= v.x_;
        y_ -= v.y_;
        return *this;
    }

    Vector &operator*=(const Number &t) {
        x_ *= t;
        y_ *= t;
        return *this;
    }

    Vector &operator/=(const Number &t) {
        assert(t != Number::ZERO && "Vec2::operator/= cannot divide by 0");
        x_ /= t;
        y_ /= t;
        return *this;
    }

    [[nodiscard]] Vector operator-(const Vector &v) const {
        return {x_ - v.x_, y_ - v.y_};
    }

    [[nodiscard]] Vector operator*(const Number &t) const {
        return {x_ * t, y_ * t};
    }

    [[nodiscard]] Vector operator/(const Number &t) const {
        assert(t != Number::ZERO && "Vec2::operator/ cannot divide by 0");
        return {x_ / t, y_ / t};
    }

    [[nodiscard]] bool operator==(const Vector &v) const {
        return (x_ == v.x_) && (y_ == v.y_);
    }

    [[nodiscard]] bool operator!=(const Vector &v) const {
        return !(*this == v);
    }

    Vector &operator=(const Vector &other) {
        if (this != &other) {
            x_ = other.x_;
            y_ = other.y_;
        }
        return *this;
    }

    /// \brief Первая координата вектора
    [[nodiscard]] Number getX() const {
        return x_;
    }

    /// \brief Вторая координата вектора
    [[nodiscard]] Number getY() const {
        return x_;
    }

    static const Vector ZERO;
    static const Vector ONE;

private:
    Number x_;
    Number y_;
};

[[nodiscard]] inline Vector operator*(const Number &t, const Vector &u) {
    return u * t;
}
#endif //VECTOR_H
