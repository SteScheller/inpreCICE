#pragma once

#include <array>
#include <GL/gl3w.h>

#include "util.hpp"

namespace util
{
    namespace geometry
    {
        class Shape
        {
            public:
            Shape();
            Shape(const Shape& other) = delete;
            Shape& operator=(const Shape& other) = delete;
            Shape(Shape&& other);
            Shape& operator=(Shape&& other);
            virtual ~Shape();

            virtual void draw() const = 0;

            protected:
            GLuint m_vertexArrayObject;

            void bind() const;
            void unbind() const;
        };

        class CubeFrame : Shape
        {
            public:
            CubeFrame(bool oglAvailable);
            CubeFrame(const CubeFrame& other) = delete;
            CubeFrame& operator=(const CubeFrame& other) = delete;
            CubeFrame(CubeFrame&& other) : Shape(std::move(other)) {};
            CubeFrame& operator=(CubeFrame&& other);
            ~CubeFrame();

            void draw() const;
        };

        class Cube : Shape
        {
            public:
            Cube(bool oglAvailable);
            Cube(const Cube& other) = delete;
            Cube& operator=(const Cube& other) = delete;
            Cube(Cube&& other) : Shape(std::move(other)) {};
            Cube& operator=(Cube&& other);
            ~Cube();

            void draw() const;
        };

        class Quad : Shape
        {
            public:
            Quad(bool oglAvailable);
            Quad(const Quad& other) = delete;
            Quad& operator=(const Quad& other) = delete;
            Quad(Quad&& other) : Shape(std::move(other)) {};
            Quad& operator=(Quad&& other);
            ~Quad();

            void draw() const;
        };

        class Point2D : Shape
        {
            public:
            Point2D(bool oglAvailable);
            Point2D(const Point2D& other) = delete;
            Point2D& operator=(const Point2D& other) = delete;
            Point2D(Point2D&& other) : Shape(std::move(other)) {};
            Point2D& operator=(Point2D&& other);
            ~Point2D();

            void draw() const;
        };

        class Line2D : Shape
        {
            public:
            Line2D(
                bool oglAvailable,
                std::array<float, 4> vertices = {0.f, 0.f, 1.f, 1.f});
            Line2D(const Line2D& other) = delete;
            Line2D& operator=(const Line2D& other) = delete;
            Line2D(Line2D&& other) : Shape(std::move(other)) {};
            Line2D& operator=(Line2D&& other);
            ~Line2D();

            void draw() const;
        };

    }
}
