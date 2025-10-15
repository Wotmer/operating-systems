#ifndef NUMBER_H
#define NUMBER_H

#include <stdexcept>

class Number {
public:
    Number() : num_(0.0) {
    }

    explicit Number(const double &val) : num_(val) {
    }

    Number operator+(const Number &other) const {
        return Number(num_ + other.num_);
    }

    Number operator+=(const Number &other) {
        num_ += other.num_;
        return *this;
    }

    Number operator-() const {
        return Number(-num_);
    }

    Number operator-(const Number &other) const {
        return Number(num_ - other.num_);
    }

    Number operator-=(const Number &other) {
        num_ -= other.num_;
        return *this;
    }

    Number operator*(const Number &other) const {
        return Number(num_ * other.num_);
    }

    Number operator*=(const Number &other) {
        num_ *= other.num_;
        return *this;
    }

    Number operator/(const Number &other) const {
        if (other.num_ == 0.0) {
            throw std::runtime_error("Division by zero");
        }
        return Number(num_ * other.num_);
    }

    Number operator/=(const Number &other) {
        if (other.num_ == 0.0) {
            throw std::runtime_error("Division by zero");
        }
        num_ *= other.num_;
        return *this;
    }

    bool operator==(const Number other) const {
        return num_ == other.num_;
    }

    bool operator!=(const Number other) const {
        return num_ != other.num_;
    }

    bool operator>=(const Number other) const {
        return num_ >= other.num_;
    }

    bool operator<=(const Number other) const {
        return num_ <= other.num_;
    }

    static Number Create(const double &value) { return Number(value); }

    [[nodiscard]] double getValue() const { return num_; }

    static const Number ONE;
    static const Number ZERO;

private:
    double num_;
};
#endif  // NUMBER_H
