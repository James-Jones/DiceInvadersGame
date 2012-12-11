#ifndef VEC2_H
#define VEC2_H

class Vec2
{
public:
    Vec2() : mX(0.0f), mY(0.0f) {}
    explicit Vec2(float x, float y) : mX(x), mY(y) {}

    Vec2& operator = (const Vec2& rhs)
    {
        mX = rhs.x();
        mY = rhs.y();
        return *this;
    }

    const Vec2 operator - (const Vec2& rhs)
    {
        return Vec2( mX - rhs.x(),
                     mY - rhs.y());
    }

    const Vec2 operator + (const Vec2& rhs)
    {
        return Vec2( mX + rhs.x(),
                     mY + rhs.y());
    }

    Vec2& operator += (const Vec2& rhs)
    {
        *this = *this + rhs;
        return *this;
    }

    void moveX(const float delta)
    {
        mX += delta;
    }
    void moveY(const float delta)
    {
        mY += delta;
    }

    float x() const {
        return mX;
    }
    float y() const {
        return mY;
    }
private:
    float mX;
    float mY;
};

#endif
